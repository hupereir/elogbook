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

    // create painter on printer
    QPainter painter;
    painter.begin(printer);

    // print everything
    QPointF offset( 0, 0 );
    printEntry( printer, &painter, offset );
    painter.end();

}

//__________________________________________________________________________________
void LogEntryPrintHelper::printEntry( QPrinter* printer, QPainter* painter, QPointF& offset ) const
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
void LogEntryPrintHelper::_printHeader( QPrinter* printer, QPainter* painter, QPointF& offset ) const
{

    Debug::Throw( "LogEntryPrintHelper::_printHeader.\n" );

    // check mask
    if( !( mask_ & ENTRY_HEADER ) ) return;

    // create document
    QTextDocument document;
    const QRect pageRect( printer->pageRect() );
    document.setPageSize( printer->pageRect().size() );
    document.setDocumentMargin(0);
    document.documentLayout()->setPaintDevice( printer );

    const QFont font( document.defaultFont() );
    const QFontMetrics metrics( font, printer );

    // get table cells
    typedef QPair<QString, QString> StringPair;
    typedef QList<StringPair> StringList;
    StringList values;
    if( mask_&ENTRY_KEYWORD ) values.push_back( StringPair( "Keyword: ", entry_->keyword().get() ) );
    if( mask_&ENTRY_TITLE ) values.push_back( StringPair( "Title: ", entry_->title() ) );
    if( mask_&ENTRY_AUTHOR ) values.push_back( StringPair( "Author: ", entry_->author() ) );
    if( mask_&ENTRY_CREATION ) values.push_back( StringPair( "Created: ", entry_->creation().toString() ) );
    if( mask_&ENTRY_MODIFICATION ) values.push_back( StringPair( "Modified: ", entry_->modification().toString() ) );
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
    boundingRect.setWidth( pageRect.width()-5 );

    if( offset.y() + boundingRect.height() > pageRect.bottom() )
    {
        offset.setY(0);
        printer->newPage();
    }

    // render
    painter->save();
    painter->translate( offset );

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
void LogEntryPrintHelper::_printBody( QPrinter* printer, QPainter* painter, QPointF& offset ) const
{
    Debug::Throw( "LogEntryPrintHelper::_printBody.\n" );

    // check mask
    if( !(mask_&ENTRY_TEXT ) ) return;

    // create document
    QTextDocument document;
    const QRect pageRect( printer->pageRect() );
    document.setPageSize( pageRect.size() );
    document.setDocumentMargin(0);
    document.documentLayout()->setPaintDevice( printer );

    // layout on document (using proper formats)
    document.setPlainText( entry_->text() );
    QTextCursor cursor( &document );
    cursor.beginEditBlock();

    const FORMAT::TextFormatBlock::List& formatList( entry_->formats() );
    for( FORMAT::TextFormatBlock::List::const_iterator iter = formatList.begin(); iter != formatList.end(); ++iter )
    {

        // check if paragraphs are set to 0 or not. If non 0, need to convert to absolute index
        TextPosition begin( iter->parBegin(), iter->begin() );
        int indexBegin = iter->parBegin() ? begin.index( &document ) : iter->begin();

        TextPosition end( iter->parEnd(), iter->end() );
        int indexEnd = iter->parEnd() ? end.index( &document ) : iter->end();

        // define cursor
        cursor.setPosition( indexBegin, QTextCursor::MoveAnchor );
        cursor.setPosition( indexEnd, QTextCursor::KeepAnchor );

        // define format
        QTextCharFormat textFormat;
        textFormat.setFontWeight( iter->format() & FORMAT::BOLD ? QFont::Bold : QFont::Normal );
        textFormat.setFontItalic( iter->format() & FORMAT::ITALIC );
        textFormat.setFontUnderline( iter->format() & FORMAT::UNDERLINE );
        textFormat.setFontStrikeOut( iter->format() & FORMAT::STRIKE );
        textFormat.setFontOverline( iter->format() & FORMAT::OVERLINE );

        // load color
        if( iter->color() != ColorMenu::NONE )
        { textFormat.setForeground( QColor( iter->color() ) ); }

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
        if( (offset.y() + textLayoutHeight ) > pageRect.bottom() )
        {
            offset.setY(0);
            printer->newPage();

        }

        // render
        textLayout.draw( painter, offset );

        // update position
        offset.setY( offset.y() + textLayoutHeight );

    }

}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printAttachments( QPrinter* printer, QPainter* painter, QPointF& offset ) const
{
    Debug::Throw( "LogEntryPrintHelper::_printAttachments.\n" );

    // check mask
    if( !(mask_&ENTRY_ATTACHMENTS) ) return;

    // check attachments
    BASE::KeySet<Attachment> attachments( entry_ );
    if( attachments.empty() ) return;

    // create document
    QTextDocument document;
    const QRect pageRect( printer->pageRect() );
    document.setPageSize( printer->pageRect().size() );
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
    QTextTable* table = cursor.insertTable( attachments.size()+1, 3, tableFormat );

    int row(0);
    table->cellAt( row, 0 ).firstCursorPosition().insertText( "Attachments: " );
    row++;

    // loop over attachments
    for( BASE::KeySet<Attachment>::const_iterator iter = attachments.begin(); iter != attachments.end(); ++iter, ++row )
    {
        const Attachment& attachment( **iter );

        // filename
        table->cellAt( row, 0 ).firstCursorPosition().insertText( attachment.shortFile() );

        // type
        QString buffer;
        QTextStream( &buffer ) << "  (" << attachment.type().name() << ")  ";
        table->cellAt( row, 1 ).firstCursorPosition().insertText( buffer );

        // comments
        table->cellAt( row, 2 ).firstCursorPosition().insertText( attachment.comments() );
    }

    // check for new page
    QRectF boundingRect( document.documentLayout()->frameBoundingRect( table ) );
    if( offset.y() + boundingRect.height() > pageRect.bottom() )
    {
        offset.setY(0);
        printer->newPage();
    }

    // render
    painter->save();
    painter->translate( offset );

    // render contents
    document.drawContents( painter );
    painter->restore();

    // update offset
    offset.setY( offset.y() + boundingRect.height() + 2*margin );
}
