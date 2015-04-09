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

#include "LogbookStatisticsDialog.h"

#include "Debug.h"
#include "GridLayout.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "GridLayoutItem.h"
#include "TreeView.h"
#include "Util.h"

#include <QHeaderView>
#include <QLayout>
#include <QLabel>
#include <QPushButton>

//_________________________________________________________
LogbookStatisticsDialog::LogbookStatisticsDialog( QWidget* parent, Logbook* logbook ):
    CustomDialog( parent, CloseButton )
{
    Debug::Throw( "LogbookStatisticsDialog::LogbookStatisticsDialog.\n" );

    setWindowTitle( tr( "Logbook Statistics - Elogbook" ) );
    setOptionName( "LOGBOOK_STATISTICS_DIALOG" );

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setMargin(5);
    mainLayout().addLayout( hLayout );

    QLabel* label = new QLabel(this);
    label->setPixmap( IconEngine::get( IconNames::DialogInformation ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignTop );
    hLayout->addStretch();

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);
    hLayout->addLayout( gridLayout );
    hLayout->addStretch();

    // generic item
    GridLayoutItem* item;

    if( !logbook->file().isEmpty() )
    {
        // file
        item = new GridLayoutItem( this, gridLayout, GridLayoutItem::All );
        item->setKey( tr( "File name:" ) );
        item->setText( logbook->file().localName() );

        // path
        item = new GridLayoutItem( this, gridLayout, GridLayoutItem::Selectable|GridLayoutItem::Elide );
        item->setKey( tr( "Path:" ) );
        item->setText( logbook->file().path() );
    }

    // creation
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Created:" ) );
    item->setText( logbook->creation().isValid() ? logbook->creation().toString():QString() );

    // modification
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Modified:" ) );
    item->setText( logbook->modification().isValid() ? logbook->modification().toString():QString() );

    // backup
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Last backup:" ) );
    item->setText( logbook->backup().isValid() ? logbook->backup().toString():QString() );

    // entries
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Entries:" ) );
    item->setText( QString::number( logbook->entries().size() ) );

    // attachments
    item = new GridLayoutItem( this, gridLayout );
    item->setKey( tr( "Attachments:" ) );
    item->setText( QString::number( logbook->attachments().size() ) );

    gridLayout->setColumnStretch( 1, 1 );

    // detail
    TreeView *listView( new TreeView( this ) );
    listView->setModel( &model_ );
    listView->setSortingEnabled( false );
    listView->setOptionName( "LOGBOOK_STATISTICS" );
    mainLayout().addWidget( listView, 1 );

    #if QT_VERSION >= 0x050000
    listView->header()->setSectionResizeMode( LogbookModel::Created, QHeaderView::Stretch);
    listView->header()->setSectionResizeMode( LogbookModel::Modified, QHeaderView::Stretch);
    #else
    listView->header()->setResizeMode( LogbookModel::Created, QHeaderView::Stretch);
    listView->header()->setResizeMode( LogbookModel::Modified, QHeaderView::Stretch);
    #endif

    Logbook::List all( logbook->children() );
    all.prepend( logbook );
    model_.set( all );

    listView->resizeColumns();

}
