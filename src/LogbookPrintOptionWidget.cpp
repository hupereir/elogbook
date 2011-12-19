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


#include "LogbookPrintOptionWidget.h"
#include "LogbookPrintHelper.h"
#include "XmlOptions.h"

#include <QtGui/QLayout>

//_________________________________________________________________
LogbookPrintOptionWidget::LogbookPrintOptionWidget( QWidget* parent ):
    QWidget( parent ),
    OptionWidget( "LOGBOOK_PRINT_OPTION_MASK" )
{

    setWindowTitle( "Logbook Configuration" );

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout( layout );

    // insert checkboxes
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_TITLE, new QCheckBox( "Title", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_COMMENTS, new QCheckBox( "Comments", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_AUTHOR, new QCheckBox( "Author", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_FILE, new QCheckBox( "File name", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_DIRECTORY, new QCheckBox( "Attachments directory name", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_CREATION, new QCheckBox( "Creation time", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_MODIFICATION, new QCheckBox( "Last modificaion time", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_BACKUP, new QCheckBox( "Last backup time", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_TABLE, new QCheckBox( "Table of contents", this ) );
    checkBoxes_.insert( LogbookPrintHelper::LOGBOOK_CONTENT, new QCheckBox( "Selected logbook entries", this ) );

    // insert in layout
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { layout->addWidget( iter.value() ); }

    layout->addStretch(1);

}

//_________________________________________________________________
void LogbookPrintOptionWidget::read()
{
    unsigned int mask( XmlOptions::get().get<unsigned int>( optionName() ) );
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { iter.value()->setChecked( mask&iter.key() ); }
}

//_________________________________________________________________
void LogbookPrintOptionWidget::write( void ) const
{ XmlOptions::get().set<unsigned int>( optionName(), mask() ); }

//_________________________________________________________________
unsigned int LogbookPrintOptionWidget::mask( void ) const
{
    unsigned int out(0);
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { if( iter.value()->isChecked() ) out |= iter.key(); }

    return out;

}
