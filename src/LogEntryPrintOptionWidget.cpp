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


#include "LogEntryPrintOptionWidget.h"
#include "LogEntry.h"
#include "XmlOptions.h"

#include <QLayout>

//_________________________________________________________________
LogEntryPrintOptionWidget::LogEntryPrintOptionWidget( QWidget* parent ):
    QWidget( parent ),
    OptionWidget( "LOGENTRY_PRINT_OPTION_MASK" )
{

    setWindowTitle( tr( "Logbook Entry Configuration - Elogbook" ) );

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout( layout );

    // insert checkboxes
    checkBoxes_.insert( LogEntry::ENTRY_KEYWORD, new QCheckBox( tr( "Keyword" ), this ) );
    checkBoxes_.insert( LogEntry::ENTRY_TITLE, new QCheckBox( tr( "Title" ), this ) );
    checkBoxes_.insert( LogEntry::ENTRY_AUTHOR, new QCheckBox( tr( "Author" ), this ) );
    checkBoxes_.insert( LogEntry::ENTRY_CREATION, new QCheckBox( tr( "Creation time" ), this ) );
    checkBoxes_.insert( LogEntry::ENTRY_MODIFICATION, new QCheckBox( tr( "Last modificaion time" ), this ) );
    checkBoxes_.insert( LogEntry::ENTRY_TEXT, new QCheckBox( tr( "Contents" ), this ) );
    checkBoxes_.insert( LogEntry::ENTRY_ATTACHMENTS, new QCheckBox( tr( "Attachments" ), this ) );

    // insert in layout
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    {
        connect( iter.value(), SIGNAL( toggled( bool ) ), SLOT( _updateMask( void ) ) );
        layout->addWidget( iter.value() );
    }

    layout->addStretch(1);

}

//_________________________________________________________________
void LogEntryPrintOptionWidget::read()
{
    unsigned int mask( XmlOptions::get().get<unsigned int>( optionName() ) );
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { iter.value()->setChecked( mask&iter.key() ); }
}

//_________________________________________________________________
void LogEntryPrintOptionWidget::write( void ) const
{ XmlOptions::get().set<unsigned int>( optionName(), mask() ); }

//_________________________________________________________________
unsigned int LogEntryPrintOptionWidget::mask( void ) const
{
    unsigned int out(0);
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { if( iter.value()->isChecked() ) out |= iter.key(); }

    return out;

}
