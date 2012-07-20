// $Id$
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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
****************************************************************************/

#include "LogbookPrintHelper.h"

#include "LogEntryPrintHelper.h"

#include <QtCore/QList>
#include <QtCore/QPair>

#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QTextTable>
#include <QtGui/QTextTableFormat>

#include <cmath>

//__________________________________________________________________________________
void LogbookPrintHelper::print( QPrinter* printer )
{
    Debug::Throw( "LogbookPrintHelper::print.\n" );

    // check logbook
    assert( logbook_ );

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
    if( mask_ & Logbook::LOGBOOK_TABLE ) maxValue += entries_.size();
    if( (mask_&Logbook::LOGBOOK_CONTENT) && (entryMask_&LogEntry::ENTRY_ALL) ) maxValue += entries_.size();

    if( maxValue )
    {
        progress_ = 0;
        progressDialog_ = new QProgressDialog( "Generating print output...", "Cancel", 0, maxValue );
        progressDialog_->setWindowTitle( "Print Logbook - elogbook" );
        progressDialog_->setWindowModality(Qt::WindowModal);
    }

    // print everything
    QPointF offset( 0, 0 );
    _printHeader( printer, &painter, offset );
    if( !isAborted() ) _printTable( printer, &painter, offset );
    if( !isAborted() ) _printEntries( printer, &painter, offset );

    painter.end();

    if( progressDialog_ )
    {
        progressDialog_->setValue( maxValue );
        progressDialog_->deleteLater();
        progressDialog_ = 0;
    }
}

//__________________________________________________________________________________
void LogbookPrintHelper::_updateEntries( void )
{

    Debug::Throw( "LogbookPrintHelper::_updateEntries.\n" );
    switch( selectionMode_ )
    {
        case LogEntryPrintSelectionWidget::ALL_ENTRIES:
        entries_ = allEntries_;
        break;

        default:
        case LogEntryPrintSelectionWidget::VISIBLE_ENTRIES:
        entries_ = visibleEntries_;
        break;

        case LogEntryPrintSelectionWidget::SELECTED_ENTRIES:
        entries_ = selectedEntries_;
        break;
    }

}


//__________________________________________________________________________________
void LogbookPrintHelper::_printHeader( QPrinter* printer, QPainter* painter, QPointF& offset )
{

    Debug::Throw( "LogbookPrintHelper::_printHeader.\n" );

    // check mask
    if( !( mask_ & Logbook::LOGBOOK_HEADER ) ) return;

    // create document
    QTextDocument document;
    const QRect pageRect( _pageRect() );
    document.setPageSize( _pageRect().size() );
    document.setDocumentMargin(0);
    document.documentLayout()->setPaintDevice( printer );

    const QFont font( document.defaultFont() );
    const QFontMetrics metrics( font, printer );

    // get table cells
    typedef QPair<QString, QString> StringPair;
    typedef QList<StringPair> StringList;
    StringList values;
    if( mask_&Logbook::LOGBOOK_TITLE ) values.push_back( StringPair( "Title: ", logbook_->title() ) );
    if( mask_&Logbook::LOGBOOK_COMMENTS && !logbook_->comments().isEmpty() ) values.push_back( StringPair( "Comments: ", logbook_->comments() ) );
    if( mask_&Logbook::LOGBOOK_AUTHOR ) values.push_back( StringPair( "Author: ", logbook_->author() ) );

    if( mask_&Logbook::LOGBOOK_FILE ) values.push_back( StringPair( "File: ", logbook_->file() ) );
    if( mask_&Logbook::LOGBOOK_DIRECTORY && !logbook_->directory().isEmpty() )
    {
        QString buffer;
        QTextStream what( &buffer );
        what << logbook_->directory();
        if( !logbook_->checkDirectory() ) what << " (not found)";

        values.push_back( StringPair( "Attachments directory: ", buffer ) );
    }

    if( mask_&Logbook::LOGBOOK_CREATION ) values.push_back( StringPair( "Created: ", logbook_->creation().toString() ) );
    if( mask_&Logbook::LOGBOOK_MODIFICATION ) values.push_back( StringPair( "Last Modified: ", logbook_->modification().toString() ) );
    if( mask_&Logbook::LOGBOOK_BACKUP ) values.push_back( StringPair( "Last backup: ", logbook_->backup().toString() ) );
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
    painter->setPen( QColor( "#888888" ) );
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

    Debug::Throw( "LogbookPrintHelper::_printTable.\n" );

    // check entries
    if( entries_.empty() ) return;

    // check mask
    if( !( mask_ & Logbook::LOGBOOK_TABLE ) ) return;

    // calculate number of entries per page
    const QFont font( QTextDocument().defaultFont() );
    const QFontMetrics metrics( font, printer );
    const QRect pageRect( _pageRect() );

    LogEntryModel::List::const_iterator iter = entries_.begin();
    while( iter != entries_.end() )
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
        QTextTable* table = cursor.insertTable( 1, 4, tableFormat );

        // header
        QTextTableCellFormat cellFormat;
        cellFormat.setFontWeight( QFont::Bold );
        QTextTableCell cell;
        (cell = table->cellAt(0,0)).setFormat( cellFormat ); cell.firstCursorPosition().insertText( "Keyword" );
        (cell = table->cellAt(0,1)).setFormat( cellFormat ); cell.firstCursorPosition().insertText( "Title" );
        (cell = table->cellAt(0,2)).setFormat( cellFormat ); cell.firstCursorPosition().insertText( "Created" );
        (cell = table->cellAt(0,3)).setFormat( cellFormat ); cell.firstCursorPosition().insertText( "Last modified" );

        int row(1);
        for( ;iter != entries_.end() && row < nRows; ++iter, ++row )
        {

            table->appendRows( 1 );

            const LogEntry& entry( **iter );
            table->cellAt(row,0).firstCursorPosition().insertText( entry.keyword().get() + "  " );
            table->cellAt(row,1).firstCursorPosition().insertText( entry.title() + "  " );
            table->cellAt(row,2).firstCursorPosition().insertText( entry.creation().toString() + "  " );
            table->cellAt(row,3).firstCursorPosition().insertText( entry.modification().toString() + "  " );

            // update progress
            if( progressDialog_ )
            {
                progressDialog_->setValue( ++progress_ );
                if( progressDialog_->wasCanceled() )
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

    Debug::Throw( "LogbookPrintHelper::_printEntries.\n" );

    // check entries
    if( entries_.empty() ) return;

    // check entries
    if( !( entryMask_ & LogEntry::ENTRY_ALL ) ) return;

    // check mask
    if( !( mask_ & Logbook::LOGBOOK_CONTENT ) ) return;

    LogEntryPrintHelper helper;
    helper.setOrientation( orientation() );
    helper.setPageMode( pageMode() );
    helper.setupPage( printer );

    helper.setFile( logbook_->file() );
    helper.setPageNumber( pageNumber() );
    helper.setSheetNumber( sheetNumber() );

    connect( &helper, SIGNAL( pageCountChanged( int ) ), this, SIGNAL( pageCountChanged( int ) ) );
    foreach( LogEntry* entry, entries_ )
    {

        helper.setEntry( entry );
        helper.setMask( entryMask_ );
        helper.printEntry( printer, painter, offset );

        // update progress
        if( progressDialog_ )
        {
            progressDialog_->setValue( ++progress_ );
            if( progressDialog_->wasCanceled() )
            {
                abort();
                return;
            }
        }

    }


}
