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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "LogEntryInformationDialog.h"

#include "BaseFileInformationDialog.h"
#include "Debug.h"
#include "GridLayout.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "Options.h"
#include "QtUtil.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

//_________________________________________________________
LogEntryInformationDialog::LogEntryInformationDialog( QWidget* parent, LogEntry* entry ):
    CustomDialog( parent, OkButton| Separator )
{
    Debug::Throw( "LogEntryInformationDialog::LogEntryInformationDialog.\n" );

    setOptionName( "ENTRY_INFORMATION_DIALOG" );

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setSpacing(10);
    hLayout->setMargin(5);
    mainLayout().addLayout( hLayout );

    QLabel* label = new QLabel(this);
    label->setPixmap( IconEngine::get( ICONS::INFORMATION ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignTop );

    GridLayout *gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    gridLayout->setMaxCount(2);

    hLayout->addStretch();
    hLayout->addLayout( gridLayout );
    hLayout->addStretch();

    BaseFileInformationDialog::Item* item;

    // title
    item = new BaseFileInformationDialog::Item( this, gridLayout, BaseFileInformationDialog::Bold );
    item->setKey( "Title:" );
    item->setValue( entry->title() );

    // keyword
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( "Keyword:" );
    item->setValue( entry->keyword().get() );

    // author
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( "Author:" );
    item->setValue( entry->author() );

    // creation
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( "Created:" );
    item->setValue( entry->creation().toString() );

    // modified
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( "Modified:" );
    item->setValue( entry->modification().toString() );

    // retrieve associated logbook
    BASE::KeySet<Logbook> logbooks( entry );
    if( !logbooks.empty() )
    {

        item = new BaseFileInformationDialog::Item( this, gridLayout );
        item->setKey( "File:" );
        item->setValue( File( (*logbooks.begin())->file() ).localName() );

    }

    mainLayout().addStretch();

}
