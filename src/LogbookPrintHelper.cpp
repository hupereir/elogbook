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

#include "Logbook.h"

#include <QtCore/QList>
#include <QtCore/QPair>

#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QTextTable>
#include <QtGui/QTextTableFormat>

//__________________________________________________________________________________
void LogbookPrintHelper::print( QPrinter* printer )
{
    Debug::Throw( "LogbookPrintHelper::print.\n" );

    // check logbook
    assert( logbook_ );

    // do nothing if mask is empty
    if( !mask_ ) return;

    // create painter on printer
    QPainter painter;
    painter.begin(printer);

    // print everything
    QPointF offset( 0, 0 );

    // print everything
    _printHeader( printer, &painter, offset );
    _printTable( printer, &painter, offset );
    _printEntries( printer, &painter, offset );

    painter.end();

}

//__________________________________________________________________________________
void LogbookPrintHelper::_printHeader( QPrinter* printer, QPainter* painter, QPointF& offset ) const
{

    Debug::Throw( "LogbookPrintHelper::_printHeader.\n" );

    // check mask
    if( !( mask_ & LOGBOOK_HEADER ) ) return;

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
    if( mask_&LOGBOOK_TITLE ) values.push_back( StringPair( "Title: ", logbook_->title() ) );
    if( mask_&LOGBOOK_COMMENTS && !logbook_->comments().isEmpty() ) values.push_back( StringPair( "Comments: ", logbook_->comments() ) );
    if( mask_&LOGBOOK_AUTHOR ) values.push_back( StringPair( "Author: ", logbook_->author() ) );

    if( mask_&LOGBOOK_FILE ) values.push_back( StringPair( "File: ", logbook_->file() ) );
    if( mask_&LOGBOOK_DIRECTORY && !logbook_->directory().isEmpty() )
    {
        QString buffer;
        QTextStream what( &buffer );
        what << logbook_->directory();
        if( !logbook_->checkDirectory() ) what << " (not found)";

        values.push_back( StringPair( "Attachments directory: ", buffer ) );
    }

    if( mask_&LOGBOOK_CREATION ) values.push_back( StringPair( "Created: ", logbook_->creation().toString() ) );
    if( mask_&LOGBOOK_MODIFICATION ) values.push_back( StringPair( "Modified: ", logbook_->modification().toString() ) );
    if( mask_&LOGBOOK_BACKUP ) values.push_back( StringPair( "Last backup: ", logbook_->backup().toString() ) );
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
    painter->setPen( QColor( "#888888" ) );
    painter->drawRect( QRectF( QPointF(0,0), boundingRect.size() ) );

    // render contents
    document.drawContents( painter );
    painter->restore();

    // update offset
    offset.setY( offset.y() + boundingRect.height() + 2*margin );

}


//__________________________________________________________________________________
void LogbookPrintHelper::_printTable( QPrinter* printer, QPainter* painter, QPointF& offset ) const
{

    Debug::Throw( "LogbookPrintHelper::_printTable.\n" );

    // check entries
    if( entries_.empty() ) return;

    // check mask
    if( !( mask_ & LOGBOOK_TABLE ) ) return;
}


//__________________________________________________________________________________
void LogbookPrintHelper::_printEntries( QPrinter* printer, QPainter* painter, QPointF& offset ) const
{

    Debug::Throw( "LogbookPrintHelper::_printEntries.\n" );

    // check entries
    if( entries_.empty() ) return;

    // check mask
    if( !( mask_ & LOGBOOK_CONTENT ) ) return;
}
