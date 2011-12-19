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
 * ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * software; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA   02111-1307 USA
 *
 *
 *******************************************************************************/


#include "LogEntryPrintSelectionWidget.h"
#include "XmlOptions.h"

#include <QtGui/QLayout>
#include <QtGui/QButtonGroup>

//_________________________________________________________________
LogEntryPrintSelectionWidget::LogEntryPrintSelectionWidget( QWidget* parent ):
    QWidget( parent ),
    OptionWidget( "LOGENTRY_PRINT_SELECTION" )
{

    setWindowTitle( "Logbook Entry Selection" );

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout( layout );

    QButtonGroup* group = new QButtonGroup( this );

    // insert checkboxes
    radioButtons_.insert( ALL_ENTRIES, new QRadioButton( "All entries", this ) );
    radioButtons_.insert( VISIBLE_ENTRIES, new QRadioButton( "Visible entries", this ) );
    radioButtons_.insert( SELECTED_ENTRIES, new QRadioButton( "Selected entries", this ) );

    // insert in layout
    for( RadioButtonMap::const_iterator iter = radioButtons_.begin(); iter != radioButtons_.end(); iter++ )
    {
        layout->addWidget( iter.value() );
        group->addButton( iter.value() );
    }

    layout->addStretch(1);

}

//_________________________________________________________________
void LogEntryPrintSelectionWidget::read()
{
    unsigned int mode( XmlOptions::get().get<unsigned int>( optionName() ) );
    for( RadioButtonMap::const_iterator iter = radioButtons_.begin(); iter != radioButtons_.end(); iter++ )
    { iter.value()->setChecked( iter.key() == mode ); }
}

//_________________________________________________________________
void LogEntryPrintSelectionWidget::write( void ) const
{ XmlOptions::get().set<Mode>( optionName(), mode() ); }

//_________________________________________________________________
LogEntryPrintSelectionWidget::Mode LogEntryPrintSelectionWidget::mode( void ) const
{
    for( RadioButtonMap::const_iterator iter = radioButtons_.begin(); iter != radioButtons_.end(); iter++ )
    { if( iter.value()->isChecked() ) return iter.key(); }

    return ALL_ENTRIES;
}
