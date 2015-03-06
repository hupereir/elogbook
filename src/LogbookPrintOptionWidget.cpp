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


#include "LogbookPrintOptionWidget.h"
#include "Logbook.h"
#include "XmlOptions.h"

#include <QLayout>

//_________________________________________________________________
LogbookPrintOptionWidget::LogbookPrintOptionWidget( QWidget* parent ):
    QWidget( parent ),
    OptionWidget( "LOGBOOK_PRINT_OPTION_MASK" )
{

    setWindowTitle( "Logbook Configuration - Elogbook" );

    QVBoxLayout* layout = new QVBoxLayout();
    setLayout( layout );

    // insert checkboxes
    checkBoxes_.insert( Logbook::TitleMask, new QCheckBox( tr( "Title" ), this ) );
    checkBoxes_.insert( Logbook::CommentsMask, new QCheckBox( tr( "Comments" ), this ) );
    checkBoxes_.insert( Logbook::AuthorMasks, new QCheckBox( tr( "Author" ), this ) );
    checkBoxes_.insert( Logbook::FileMask, new QCheckBox( tr( "File name" ), this ) );
    checkBoxes_.insert( Logbook::DirectoryMask, new QCheckBox( tr( "Attachments directory name" ), this ) );
    checkBoxes_.insert( Logbook::CreationMask, new QCheckBox( tr( "Creation time" ), this ) );
    checkBoxes_.insert( Logbook::ModificationMask, new QCheckBox( tr( "Modificaion time" ), this ) );
    checkBoxes_.insert( Logbook::BackupMask, new QCheckBox( tr( "Backup time" ), this ) );
    checkBoxes_.insert( Logbook::TableOfContentMask, new QCheckBox( tr( "Table of contents" ), this ) );
    checkBoxes_.insert( Logbook::ContentMask, new QCheckBox( tr( "Selected logbook entries" ), this ) );

    // insert in layout
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    {
        connect( iter.value(), SIGNAL(toggled(bool)), SLOT(_updateMask()) );
        layout->addWidget( iter.value() );
    }

    layout->addStretch(1);

}

//_________________________________________________________________
void LogbookPrintOptionWidget::read( const Options& options )
{

    unsigned int mask( options.get<unsigned int>( optionName() ) );
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { iter.value()->setChecked( mask&iter.key() ); }

    _setConnected();

}

//_________________________________________________________________
void LogbookPrintOptionWidget::write( Options& options ) const
{ options.set<unsigned int>( optionName(), mask() ); }

//_________________________________________________________________
Logbook::Mask LogbookPrintOptionWidget::mask( void ) const
{
    Logbook::Mask out(0);
    for( CheckBoxMap::const_iterator iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { if( iter.value()->isChecked() ) out |= iter.key(); }

    return out;

}
