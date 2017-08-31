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

#include "LogEntryInformationDialog.h"

#include "GridLayoutItem.h"
#include "Debug.h"
#include "GridLayout.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "Options.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

//_________________________________________________________
LogEntryInformationDialog::LogEntryInformationDialog( QWidget* parent, LogEntry* entry ):
    CustomDialog( parent, OkButton| Separator )
{
    Debug::Throw( "LogEntryInformationDialog::LogEntryInformationDialog.\n" );

    setOptionName( "ENTRY_INFORMATION_DIALOG" );

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(10);
    hLayout->setMargin(5);
    mainLayout().addLayout( hLayout );

    auto label = new QLabel(this);
    label->setPixmap( IconEngine::get( IconNames::DialogInformation ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignTop );

    auto gridLayout = new GridLayout;
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    gridLayout->setMaxCount(2);

    hLayout->addStretch();
    hLayout->addLayout( gridLayout );
    hLayout->addStretch();

    GridLayoutItem* item;

    // title
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Title:" ) );
    item->setText( entry->title() );

    // keyword
    if( entry->keywords().size() == 1 )
    {
            item = new GridLayoutItem( this, gridLayout );
            item->setKey( tr( "Keyword:" ) );
            item->setText( entry->keywords().begin()->get() );

    } else {

        bool first = true;
        for( const auto& keyword:entry->keywords() )
        {
            item = new GridLayoutItem( this, gridLayout );
            item->setKey( first ? tr( "Keywords:" ):QString() );
            item->setText( keyword.get() );
            first = false;
        }

    }

    // author
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Author:" ) );
    item->setText( entry->author() );

    // creation
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Created:" ) );
    item->setText( entry->creation().toString() );

    // modified
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Modified:" ) );
    item->setText( entry->modification().toString() );

    // retrieve associated logbook
    Base::KeySet<Logbook> logbooks( entry );
    if( !logbooks.empty() )
    {

        item = new GridLayoutItem( this, gridLayout );
        item->setKey( tr( "File:" ) );
        item->setText( File( (*logbooks.begin())->file() ).localName() );

    }

    mainLayout().addStretch();

}
