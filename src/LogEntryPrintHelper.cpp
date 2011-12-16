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
#include "LogEntry.h"

#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QTextTable>
#include <QtGui/QTextTableFormat>

//__________________________________________________________________________________
void LogEntryPrintHelper::print( QPrinter* )
{}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printHeader( QPrinter* printer, QPointF offset ) const
{

    // create document
    QTextDocument document;
    document.setPageSize( printer->pageRect().size() );
    document.documentLayout()->setPaintDevice( printer );

    // create table
    QTextCursor cursor( &document );
    QTextTable* table = cursor.insertTable( 4, 2, QTextTableFormat() );

    // populate the cells
    table->cellAt(0,0).firstCursorPosition().insertText( "Keyword: " );
    table->cellAt(1,0).firstCursorPosition().insertText( "Title: " );
    table->cellAt(2,0).firstCursorPosition().insertText( "Created: " );
    table->cellAt(2,0).firstCursorPosition().insertText( "Modified: " );
}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printBody( QPrinter*, QPointF ) const
{}

//__________________________________________________________________________________
void LogEntryPrintHelper::_printAttachments( QPrinter*, QPointF ) const
{}
