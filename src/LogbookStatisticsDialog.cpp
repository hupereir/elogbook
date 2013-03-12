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

#include "LogbookStatisticsDialog.h"

#include "AnimatedLineEditor.h"
#include "BaseFileInformationDialog.h"
#include "Debug.h"
#include "GridLayout.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "TreeView.h"
#include "Util.h"

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
    hLayout->setSpacing(10);
    hLayout->setMargin(5);
    mainLayout().addLayout( hLayout );

    QLabel* label = new QLabel(this);
    label->setPixmap( IconEngine::get( ICONS::INFORMATION ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignTop );
    hLayout->addStretch();

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    gridLayout->setMaxCount(2);
    hLayout->addLayout( gridLayout );
    hLayout->addStretch();

    // generic item
    BaseFileInformationDialog::Item* item;

    if( !logbook->file().isEmpty() )
    {
        // file
        item = new BaseFileInformationDialog::Item( this, gridLayout, BaseFileInformationDialog::All );
        item->setKey( tr( "File name" ) );
        item->setValue( logbook->file().localName() );

        // path
        item = new BaseFileInformationDialog::Item( this, gridLayout, BaseFileInformationDialog::Selectable|BaseFileInformationDialog::Elide );
        item->setKey( tr( "Path" ) );
        item->setValue( logbook->file().path() );
    }

    // creation
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( tr( "Created:" ) );
    item->setValue( logbook->creation().isValid() ? logbook->creation().toString():QString() );

    // modification
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( tr( "Modified:" ) );
    item->setValue( logbook->modification().isValid() ? logbook->modification().toString():QString() );

    // backup
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( tr( "Last backup:" ) );
    item->setValue( logbook->backup().isValid() ? logbook->backup().toString():QString() );

    // entries
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( tr( "Entries:" ) );
    item->setValue( QString().setNum( logbook->entries().size() ) );

    // attachments
    item = new BaseFileInformationDialog::Item( this, gridLayout );
    item->setKey( tr( "Attachments:" ) );
    item->setValue( QString().setNum( logbook->attachments().size() ) );

    gridLayout->setColumnStretch( 1, 1 );

    // detail
    TreeView *listView( new TreeView( this ) );
    listView->setModel( &model_ );
    listView->setSortingEnabled( false );
    mainLayout().addWidget( listView, 1 );

    Logbook::List all( logbook->children() );
    all.prepend( logbook );
    model_.set( all );

    listView->resizeColumns();

}


//_______________________________________________
const QString LogbookStatisticsDialog::Model::columnTitles_[ LogbookStatisticsDialog::Model::nColumns ] =
{
    tr( "File" ),
    tr( "Entries" ),
    tr( "Created" ),
    tr( "Modified" )
};


//_______________________________________________________________________________________
QVariant LogbookStatisticsDialog::Model::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    // retrieve associated file info
    Logbook& logbook( *get()[index.row()] );

    // return text associated to file and column
    if( role == Qt::DisplayRole )
    {

        switch( index.column() )
        {

            case FILE: return logbook.file().localName();
            case ENTRIES: return int(BASE::KeySet<LogEntry>(&logbook).size());
            case CREATED: return logbook.creation().toString();
            case MODIFIED: return logbook.modification().toString();
            default: return QVariant();
        }
    }

    return QVariant();

}
