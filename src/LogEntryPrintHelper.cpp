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

#include "LogEntryPrintHelper.h"

#include "Attachment.h"
#include "Color.h"
#include "ColorMenu.h"
#include "LogEntry.h"
#include "Logbook.h"
#include "TextFormat.h"
#include "TextPosition.h"

#include <QtCore/QList>
#include <QtCore/QPair>

#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QTextTable>
#include <QtGui/QTextTableFormat>

//__________________________________________________________________________________
void LogEntryPrintHelper::print( QPrinter* printer )
{
    Debug::Throw( "LogEntryPrintHelper::print.\n" );

    // check entry
    assert( entry_ );

    // do nothing if mask is empty
    if( !mask_ ) return;

    // setup
    setupPage( printer );

    // get associated logbook
    BASE::KeySet<Logbook> logbooks( entry_ );
    if( !logbooks.empty() ) setFile( (*logbooks.begin())->file() );

    // create painter on printer
    QPainter painter;
    painter.begin(printer);

    // page
    _newPage( printer, &painter );

    // print everything
    QPointF offset( 0, 0 );
    printEntry( printer, &painter, offset );
    painter.end();

}

//__________________________________________________________________________________
void LogEntryPrintHelper::printEntry( QPrinter* printer, QPainter* painter, QPointF& offset )
{

    Debug::Throw( "LogEntryPrintHelper::printEntry.\n" );

    // check entry
    assert( entry_ );

    // do nothing if mask is empty
    if( !mask_ ) return;

    // print everything
    _printHeader( printer, painter, offset );
    _printBody( printer, painter, offset );
    _printAttachments( printer, painter, offset );

}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printHeader( QPrinter* printer, QPainter* painter, QPointF& offset )
{

    Debug::Throw( "LogEntryPrintHelper::_printHeader.\n" );

    // check mask
    if( !( mask_ & LogEntry::ENTRY_HEADER ) ) return;

    // create document
    QTextDocument document;
    const QRect pageRect( _pageRect() );
    document.setPageSize( pageRect.size() );
    document.setDocumentMargin(0);
    document.documentLayout()->setPaintDevice( printer );

    const QFont font( document.defaultFont() );
    const QFontMetrics metrics( font, printer );

    // get table cells
    typedef QPair<QString, QString> StringPair;
    typedef QList<StringPair> StringList;
    StringList values;
    if( mask_&LogEntry::ENTRY_KEYWORD ) values.push_back( StringPair( "Keyword: ", entry_->keyword().get() ) );
    if( mask_&LogEntry::ENTRY_TITLE ) values.push_back( StringPair( "Title: ", entry_->title() ) );
    if( mask_&LogEntry::ENTRY_AUTHOR ) values.push_back( StringPair( "Author: ", entry_->author() ) );
    if( mask_&LogEntry::ENTRY_CREATION ) values.push_back( StringPair( "Created: ", entry_->creation().toString() ) );
    if( mask_&LogEntry::ENTRY_MODIFICATION ) values.push_back( StringPair( "Last Modified: ", entry_->modification().toString() ) );
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
    QColor color( BASE::Color( entry_->color() ) );
    if( !color.isValid() ) color = QColor( "#888888" );
    painter->setPen( color );
    painter->drawRect( QRectF( QPointF(0,0), boundingRect.size() ) );

    // render contents
    document.drawContents( painter );
    painter->restore();

    // update offset
    offset.setY( offset.y() + boundingRect.height() + 2*margin );

}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printBody( QPrinter* printer, QPainter* painter, QPointF& offset )
{
    Debug::Throw( "LogEntryPrintHelper::_printBody.\n" );

    // check mask
    if( !(mask_&LogEntry::ENTRY_TEXT ) ) return;

    // create document
    QTextDocument document;
    const QRect pageRect( _pageRect() );
    document.setPageSize( pageRect.size() );
    document.setDocumentMargin(0);
    document.documentLayout()->setPaintDevice( printer );

    // layout on document (using proper formats)
    document.setPlainText( entry_->text() );
    QTextCursor cursor( &document );
    cursor.beginEditBlock();

    foreach( const FORMAT::TextFormatBlock& format, entry_->formats() )
    {

        // define cursor
        cursor.setPosition( format.begin(), QTextCursor::MoveAnchor );
        cursor.setPosition( format.end(), QTextCursor::KeepAnchor );

        // define format
        QTextCharFormat textFormat;
        textFormat.setFontWeight( format.format() & FORMAT::BOLD ? QFont::Bold : QFont::Normal );
        textFormat.setFontItalic( format.format() & FORMAT::ITALIC );
        textFormat.setFontUnderline( format.format() & FORMAT::UNDERLINE );
        textFormat.setFontStrikeOut( format.format() & FORMAT::STRIKE );
        textFormat.setFontOverline( format.format() & FORMAT::OVERLINE );

        // load color
        if( format.color() != ColorMenu::NONE )
        { textFormat.setForeground( QColor( format.color() ) ); }

        cursor.setCharFormat( textFormat );

    }

    cursor.endEditBlock();

    // rendering
    const QFont font( document.defaultFont() );
    const QFontMetrics metrics( font, printer );
    const int leading( metrics.leading() );

    // get list of blocks from document
    for( QTextBlock block( document.begin() ); block.isValid(); block = block.next() )
    {

        // construct text layout
        QTextLayout textLayout( block.text(), font, printer );

        // layout text
        textLayout.beginLayout();
        qreal height(0);
        while( true )
        {
            QTextLine line = textLayout.createLine();
            if (!line.isValid()) break;

            line.setLineWidth( pageRect.width() );
            height += leading;
            line.setPosition(QPointF(0, height));
            height += line.height();
        }

        // create ranges
        QList<QTextLayout::FormatRange> formatRanges;

        // iterator over text fragments
        for( QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it)
        {
            QTextFragment fragment = it.fragment();
            if( !fragment.isValid() ) continue;

            // create corresponding FormatRange and store
            QTextLayout::FormatRange formatRange;
            formatRange.start = fragment.position() - block.position();
            formatRange.length = fragment.length();
            formatRange.format = fragment.charFormat();
            formatRanges.push_back( formatRange );

        }

        // assign to layout
        textLayout.setAdditionalFormats( formatRanges );
        textLayout.endLayout();

        // increase page
        int textLayoutHeight( textLayout.boundingRect().height() );
        if( (offset.y() + textLayoutHeight ) > pageRect.height() )
        {
            offset.setY(0);
            _newPage( printer, painter );

        }

        // render
        textLayout.draw( painter, pageRect.topLeft() + offset );

        // update position
        offset.setY( offset.y() + textLayoutHeight );

    }

}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printAttachments( QPrinter* printer, QPainter* painter, QPointF& offset )
{
    Debug::Throw( "LogEntryPrintHelper::_printAttachments.\n" );

    // check mask
    if( !(mask_&LogEntry::ENTRY_ATTACHMENTS) ) return;

    // check attachments
    BASE::KeySet<Attachment> attachments( entry_ );
    if( attachments.empty() ) return;

    // create document
    QTextDocument document;
    const QRect pageRect( _pageRect() );
    document.setPageSize( _pageRect().size() );
    document.setDocumentMargin(0);
    document.documentLayout()->setPaintDevice( printer );

    const QFont font( document.defaultFont() );
    const QFontMetrics metrics( font, printer );

    // create table
    QTextCursor cursor( &document );
    QTextTableFormat tableFormat;
    tableFormat.setBorderStyle( QTextFrameFormat::BorderStyle_None );

    const int margin( metrics.averageCharWidth() );
    tableFormat.setMargin(0);
    tableFormat.setPadding(0);
    tableFormat.setBorder(0);
    tableFormat.setCellPadding(0);
    tableFormat.setCellSpacing(0);
    tableFormat.setWidth( QTextLength( QTextLength::PercentageLength, 100 ) );
    QTextTable* table = cursor.insertTable( attachments.size()+2, 3, tableFormat );

    int row(0);
    QTextTableCellFormat cellFormat;
    cellFormat.setFontWeight( QFont::Bold );
    QTextTableCell cell;
    ( cell = table->cellAt( row, 0 ) ).setFormat( cellFormat );
    cell.firstCursorPosition().insertText( "Attachments: " );
    row++;

    ( cell = table->cellAt( row, 0 ) ).setFormat( cellFormat );
    cell.firstCursorPosition().insertText( "Location " );

    ( cell = table->cellAt( row, 1 ) ).setFormat( cellFormat );
    cell.firstCursorPosition().insertText( "Type " );

    ( cell = table->cellAt( row, 2 ) ).setFormat( cellFormat );
    cell.firstCursorPosition().insertText( "Comments " );
    row++;

    // loop over attachments
    foreach( Attachment* attachment, attachments )
    {
        table->cellAt( row, 0 ).firstCursorPosition().insertText( attachment->file() );
        table->cellAt( row, 1 ).firstCursorPosition().insertText( attachment->type().name() );
        table->cellAt( row, 2 ).firstCursorPosition().insertText( attachment->comments() );
    }

    // check for new page
    QRectF boundingRect( document.documentLayout()->frameBoundingRect( table ) );
    if( offset.y() + boundingRect.height() > pageRect.height() )
    {
        offset.setY(0);
        _newPage( printer, painter );
    }

    // setup painter
    painter->save();
    painter->translate( pageRect.topLeft() + offset );

    // render contents
    document.drawContents( painter );
    painter->restore();

    // update offset
    offset.setY( offset.y() + boundingRect.height() + 2*margin );
}
