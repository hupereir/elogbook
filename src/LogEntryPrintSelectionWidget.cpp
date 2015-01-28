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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/


#include "LogEntryPrintSelectionWidget.h"
#include "XmlOptions.h"

#include <QLayout>
#include <QButtonGroup>

//_________________________________________________________________
LogEntryPrintSelectionWidget::LogEntryPrintSelectionWidget( QWidget* parent ):
    QWidget( parent ),
    OptionWidget( "LOGENTRY_PRINT_SELECTION", this )
{

    setWindowTitle( "Logbook Entry Selection" );

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout( layout );

    QButtonGroup* group = new QButtonGroup( this );

    // insert checkboxes
    radioButtons_.insert( AllEntries, new QRadioButton( tr( "All entries" ), this ) );
    radioButtons_.insert( VisibleEntries, new QRadioButton( tr( "Visible entries" ), this ) );
    radioButtons_.insert( SelectedEntries, new QRadioButton( tr( "Selected entries" ), this ) );

    // insert in layout
    for( RadioButtonMap::const_iterator iter = radioButtons_.begin(); iter != radioButtons_.end(); iter++ )
    {
        layout->addWidget( iter.value() );
        group->addButton( iter.value() );
    }

    connect( group, SIGNAL(buttonClicked(int)), SLOT(_updateMode()) );

    layout->addStretch(1);

}

//_________________________________________________________________
void LogEntryPrintSelectionWidget::read( const Options& options )
{

    unsigned int mode( options.get<unsigned int>( optionName() ) );
    for( RadioButtonMap::const_iterator iter = radioButtons_.begin(); iter != radioButtons_.end(); iter++ )
    { iter.value()->setChecked( iter.key() == mode ); }

    _setConnected();

}

//_________________________________________________________________
void LogEntryPrintSelectionWidget::write( Options& options ) const
{ options.set<Mode>( optionName(), mode() ); }

//_________________________________________________________________
LogEntryPrintSelectionWidget::Mode LogEntryPrintSelectionWidget::mode( void ) const
{
    for( RadioButtonMap::const_iterator iter = radioButtons_.begin(); iter != radioButtons_.end(); iter++ )
    { if( iter.value()->isChecked() ) return iter.key(); }

    return AllEntries;
}
