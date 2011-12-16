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

#include "ColorMenu.h"
#include "LogEntry.h"
#include "TextFormat.h"
#include "TextPosition.h"

#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QTextTable>
#include <QtGui/QTextTableFormat>

//__________________________________________________________________________________
void LogEntryPrintHelper::print( QPrinter* printer )
{

    Debug::Throw( "LogEntryPrintHelper::print.\n" );

    // create painter on printer
    QPainter painter;
    painter.begin(printer);

    QPointF offset( 0, 0 );
    _printHeader( printer, &painter, offset );

    painter.end();

}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printHeader( QPrinter* printer, QPainter* painter, QPointF& offset ) const
{

    Debug::Throw( "LogEntryPrintHelper::_printHeader.\n" );

    // create document
    QTextDocument document;
    document.setPageSize( printer->pageRect().size() );
    document.documentLayout()->setPaintDevice( printer );

    // create table
    QTextCursor cursor( &document );
    QTextTable* table = cursor.insertTable( 5, 2, QTextTableFormat() );

    // populate the cells
    table->cellAt(0,0).firstCursorPosition().insertText( "Keyword: " );
    table->cellAt(0,1).firstCursorPosition().insertText( entry_->keyword().get() );

    table->cellAt(1,0).firstCursorPosition().insertText( "Title: " );
    table->cellAt(1,1).firstCursorPosition().insertText( entry_->title() );

    table->cellAt(2,0).firstCursorPosition().insertText( "Author: " );
    table->cellAt(2,1).firstCursorPosition().insertText( entry_->author() );

    table->cellAt(3,0).firstCursorPosition().insertText( "Created: " );
    table->cellAt(3,1).firstCursorPosition().insertText( entry_->creation().toString() );

    table->cellAt(4,0).firstCursorPosition().insertText( "Modified: " );
    table->cellAt(4,1).firstCursorPosition().insertText( entry_->modification().toString() );

    // render
    painter->save();
    painter->translate( offset );
    document.drawContents( painter );
    painter->restore();

    // update offset
    offset += document.documentLayout()->frameBoundingRect( table ).bottomRight();

}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printBody( QPrinter* printer, QPainter* painter, QPointF& offset ) const
{
    Debug::Throw( "LogEntryPrintHelper::_printBody.\n" );
    // create document
    QTextDocument document;
    document.setPageSize( printer->pageRect().size() );
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

    // print document block by block

}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printAttachments( QPrinter*, QPainter* , QPointF& ) const
{ Debug::Throw( "LogEntryPrintHelper::_printAttachments.\n" ); }
