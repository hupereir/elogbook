/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "LogbookPrintHelper.h"
#include "LogEntryPrintHelper.h"
#include "QtUtil.h"
#include "Util.h"

#include <QList>
#include <QPair>

#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>
#include <QTextTableFormat>

#include <cmath>

//__________________________________________________________________________________
void LogbookPrintHelper::print( QPrinter* printer )
{
    Debug::Throw( QStringLiteral("LogbookPrintHelper::print.\n") );

    // check logbook
    Q_CHECK_PTR( logbook_ );

    // do nothing if mask is empty
    if( !mask_ ) return;

    // reset abort flag
    setAborted( false );

    // setup
    setupPage( printer );
    setFile( logbook_->file() );

    // create painter on printer
    QPainter painter;
    painter.begin(printer);

    _newPage( printer, &painter );

    // progress dialog
    int maxValue(0);
    if( mask_ & Logbook::TableOfContentMask ) maxValue += entries_.size();
    if( (mask_&Logbook::ContentMask) && (entryMask_&LogEntry::All) ) maxValue += entries_.size();

    if( maxValue )
    {
        progress_ = 0;
        progressDialog_ = new QProgressDialog( tr( "Generating print output..." ), tr( "Cancel" ), 0, maxValue );
        progressDialog_.data()->setWindowTitle( Util::windowTitle( tr( "Print Logbook" ) ) );
        progressDialog_.data()->setWindowModality(Qt::WindowModal);
    }

    // print everything
    QPointF offset( 0, 0 );
    _printHeader( printer, &painter, offset );
    if( !isAborted() ) _printTable( printer, &painter, offset );
    if( !isAborted() ) _printEntries( printer, &painter, offset );

    painter.end();

    if( progressDialog_ )
    {
        progressDialog_.data()->setValue( maxValue );
        progressDialog_.clear();
    }
}

//__________________________________________________________________________________
void LogbookPrintHelper::_printHeader( QPrinter* printer, QPainter* painter, QPointF& offset )
{

    Debug::Throw( QStringLiteral("LogbookPrintHelper::_printHeader.\n") );

    // check mask
    if( !( mask_ & Logbook::HeaderMask ) ) return;

    // create document
    QTextDocument document;
    const QRect pageRect( _pageRect() );
    document.setPageSize( _pageRect().size() );
    document.setDocumentMargin(0);
    document.documentLayout()->setPaintDevice( printer );

    const QFont font( document.defaultFont() );
    const QFontMetrics metrics( font, printer );

    // get table cells
    using StringPair=QPair<QString, QString>;
    using StringList=QList<StringPair>;
    StringList values;
    if( mask_&Logbook::TitleMask ) values.append( StringPair( tr( "Title:" ), logbook_->title() ) );
    if( mask_&Logbook::CommentsMask && !logbook_->comments().isEmpty() ) values.append( StringPair( tr( "Comments:" ), logbook_->comments() ) );
    if( mask_&Logbook::AuthorMasks ) values.append( StringPair( tr( "Author:" ), logbook_->author() ) );

    if( mask_&Logbook::FileMask ) values.append( StringPair( tr( "File:" ), logbook_->file() ) );
    if( mask_&Logbook::DirectoryMask && !logbook_->directory().isEmpty() )
    {
        const QString buffer = logbook_->checkDirectory() ? QString(logbook_->directory()) : tr( "%1 (not found)" ).arg( logbook_->directory() );
        values.append( StringPair( tr( "Attachments directory:" ), buffer ) );
    }

    if( mask_&Logbook::CreationMask ) values.append( StringPair( tr( "Created:" ), logbook_->creation().toString() ) );
    if( mask_&Logbook::ModificationMask ) values.append( StringPair( tr( "Modified:" ), logbook_->modification().toString() ) );
    if( mask_&Logbook::BackupMask ) values.append( StringPair( tr( "Backup:" ), logbook_->backup().toString() ) );
    const int nRows( values.size() );

    // create table
    QTextCursor cursor( &document );
    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_None );

    const int margin( metrics.averageCharWidth() );
    tableFormat.setMargin(margin);
    tableFormat.setCellPadding(0);
    tableFormat.setCellSpacing(0);
    QTextTable* table = cursor.insertTable( nRows, 2, tableFormat );

    // populate cells
    for( int row=0; row < nRows; ++row )
    {
        table->cellAt(row,0).firstCursorPosition().insertText( values[row].first );
        table->cellAt(row,1).firstCursorPosition().insertText( values[row].second );
    }

    // check for new page
    QRectF boundingRect( document.documentLayout()->frameBoundingRect( table ) );
    boundingRect.setWidth( pageRect.width()-1 );

    if( offset.y() + boundingRect.height() > pageRect.height() )
    {
        offset.setY(0);
        _newPage( printer, painter );
    }

    // setup painter
    painter->save();
    painter->translate( pageRect.topLeft() + offset );

    // render background frame
    painter->setPen( QColor( 136, 136, 136 ) );
    painter->drawRect( QRectF( QPointF(0,0), boundingRect.size() ) );

    // render contents
    document.drawContents( painter );
    painter->restore();

    // update offset
    offset.setY( offset.y() + boundingRect.height() + 2*margin );

}


//__________________________________________________________________________________
void LogbookPrintHelper::_printTable( QPrinter* printer, QPainter* painter, QPointF& offset )
{

    Debug::Throw( QStringLiteral("LogbookPrintHelper::_printTable.\n") );

    // check entries
    if( entries_.empty() ) return;

    // check mask
    if( !( mask_ & Logbook::TableOfContentMask ) ) return;

    // calculate number of entries per page
    const QFont font( QTextDocument().defaultFont() );
    const QFontMetrics metrics( font, printer );
    const QRect pageRect( _pageRect() );

    for( auto iter = entries_.begin(); iter != entries_.end(); )
    {

        // calculate number of rows
        const int nRows = std::floor( qreal(pageRect.height() - offset.y() ) / (1+metrics.lineSpacing()) );

        // create document
        QTextDocument document;
        document.setPageSize( _pageRect().size() );
        document.setDocumentMargin(0);
        document.documentLayout()->setPaintDevice( printer );

        QTextCursor cursor( &document );
        QTextTableFormat tableFormat;
        tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_None );
        tableFormat.setMargin(0);
        tableFormat.setPadding(0);
        tableFormat.setBorder(0);
        tableFormat.setCellPadding(0);
        tableFormat.setCellSpacing(0);
        tableFormat.setWidth( QTextLength( QTextLength::PercentageLength, 100 ) );
        auto table = cursor.insertTable( 1, 4, tableFormat );

        // header
        QTextTableCellFormat cellFormat;
        cellFormat.setFontWeight( QFont::Bold );
        QTextTableCell cell;
        (cell = table->cellAt(0,0)).setFormat( cellFormat ); cell.firstCursorPosition().insertText( tr( "Title" ) );
        (cell = table->cellAt(0,1)).setFormat( cellFormat ); cell.firstCursorPosition().insertText( tr( "Keyword" ) );
        (cell = table->cellAt(0,2)).setFormat( cellFormat ); cell.firstCursorPosition().insertText( tr( "Created" ) );
        (cell = table->cellAt(0,3)).setFormat( cellFormat ); cell.firstCursorPosition().insertText( tr( "Modified" ) );

        int row(1);
        for( ;iter != entries_.end() && row < nRows; ++iter, row++ )
        {

            auto entry( *iter );

            // make sure there is room to print the full entry
            if( currentKeyword_.get().isEmpty() && entry->keywords().size() > nRows )
            { break; }

            // add new row
            table->appendRows( 1 );

            // assign title, creation and modification time
            table->cellAt(row,0).firstCursorPosition().insertText( entry->title() + "  " );
            table->cellAt(row,2).firstCursorPosition().insertText( entry->creation().toString() + "  " );
            table->cellAt(row,3).firstCursorPosition().insertText( entry->modification().toString() + "  " );

            // assign keyword
            if( entry->keywords().contains( currentKeyword_ ) )
            {

                table->cellAt(row,1).firstCursorPosition().insertText( currentKeyword_.get() + "  " );

            } else if( entry->hasKeywords() ) {

                int i=0;
                const int keywordCount( entry->keywords().size() );
                for( const auto& keyword:entry->keywords() )
                {
                    table->cellAt(row,1).firstCursorPosition().insertText( keyword.get() + "  " );
                    if( i < keywordCount-1 )
                    {
                        table->appendRows( 1 );
                        row++;
                    }

                    // increment counter
                    ++i;
                }

            }

            // update progress
            if( progressDialog_ )
            {
                progressDialog_.data()->setValue( ++progress_ );
                if( progressDialog_.data()->wasCanceled() )
                {
                    abort();
                    return;
                }
            }
        }

        // check for new page
        QRectF boundingRect( document.documentLayout()->frameBoundingRect( table ) );
        boundingRect.setWidth( pageRect.width()-5 );

        // setup painter
        painter->save();
        painter->translate( pageRect.topLeft() + offset );

        // render contents
        document.drawContents( painter );
        painter->restore();

        // new page
        if( iter == entries_.end() )
        {

            const int margin( metrics.averageCharWidth() );
            offset.setY( offset.y() + boundingRect.height() + 2*margin );

        } else {

            offset.setY(0);
            _newPage( printer, painter );
        }

    }

}


//__________________________________________________________________________________
void LogbookPrintHelper::_printEntries( QPrinter* printer, QPainter* painter, QPointF& offset )
{

    Debug::Throw( QStringLiteral("LogbookPrintHelper::_printEntries.\n") );

    // check entries
    if( entries_.empty() ) return;

    // check entries
    if( !( entryMask_ & LogEntry::All ) ) return;

    // check mask
    if( !( mask_ & Logbook::ContentMask ) ) return;

    LogEntryPrintHelper helper;
    helper.setOrientation( orientation() );
    helper.setPageMode( pageMode() );
    helper.setupPage( printer );

    helper.setFile( logbook_->file() );
    helper.setPageNumber( pageNumber() );
    helper.setSheetNumber( sheetNumber() );

    connect( &helper, &BasePrintHelper::pageCountChanged, this, &BasePrintHelper::pageCountChanged );
    for( const auto& entry:entries_ )
    {

        helper.setEntry( entry );
        helper.setMask( entryMask_ );
        helper.printEntry( printer, painter, offset );

        // update progress
        if( progressDialog_ )
        {
            progressDialog_.data()->setValue( ++progress_ );
            if( progressDialog_.data()->wasCanceled() )
            {
                abort();
                return;
            }
        }

    }


}
