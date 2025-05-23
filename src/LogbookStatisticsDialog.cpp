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
#include "GridLayoutItem.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "LogEntry.h"
#include "Logbook.h"
#include "QtUtil.h"
#include "TreeView.h"
#include "Util.h"


#include <QHeaderView>
#include <QLayout>
#include <QLabel>
#include <QPushButton>

//_________________________________________________________
LogbookStatisticsDialog::LogbookStatisticsDialog( QWidget* parent, Logbook* logbook ):
    Dialog( parent, CloseButton )
{
    Debug::Throw( QStringLiteral("LogbookStatisticsDialog::LogbookStatisticsDialog.\n") );

    setWindowTitle( tr( "Logbook Statistics" ) );
    setOptionName( QStringLiteral("LOGBOOK_STATISTICS_DIALOG") );

    layout()->setSpacing(0);
    QtUtil::setMargin(layout(), 0);
    QtUtil::setMargin(&buttonLayout(), defaultMargin());

    auto hLayout = new QHBoxLayout;
    QtUtil::setMargin(hLayout, defaultMargin());
    mainLayout().addLayout( hLayout );

    auto label = new QLabel(this);
    label->setPixmap( IconEngine::get( IconNames::DialogInformation ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignTop );
    hLayout->addStretch();

    auto gridLayout = new GridLayout;
    QtUtil::setMargin(gridLayout, 0);
    gridLayout->setMaxCount(2);
    hLayout->addLayout( gridLayout );
    hLayout->addStretch();

    // generic item
    GridLayoutItem* item;

    if( !logbook->file().isEmpty() )
    {
        // file
        item = new GridLayoutItem( this, gridLayout );
        item->setKey( tr( "File name:" ) );
        item->setText( logbook->file().localName() );

        // path
        item = new GridLayoutItem( this, gridLayout, GridLayoutItem::Flag::Selectable|GridLayoutItem::Flag::Elide );
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
    auto listView( new TreeView( this ) );
    listView->setModel( &model_ );
    listView->setSortingEnabled( false );
    listView->setOptionName( QStringLiteral("LOGBOOK_STATISTICS") );
    mainLayout().addWidget( listView, 1 );

    QtUtil::setWidgetSides(listView, Qt::TopEdge|Qt::BottomEdge);

    listView->header()->setSectionResizeMode( LogbookModel::Created, QHeaderView::Stretch);
    listView->header()->setSectionResizeMode( LogbookModel::Modified, QHeaderView::Stretch);

    LogbookModel::List all;
    all.append( logbook );

    const auto children( logbook->children() );
    std::transform( children.begin(), children.end(), std::back_inserter( all ), []( const Logbook::LogbookPtr& logbook ) { return logbook.get(); } );

    model_.set( all );

    listView->resizeColumns();

}
