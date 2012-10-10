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

#include "Icons.h"
#include "IconEngine.h"
#include "AnimatedLineEditor.h"
#include "Debug.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "TreeView.h"
#include "Util.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>

//_________________________________________________________
LogbookStatisticsDialog::LogbookStatisticsDialog( QWidget* parent, Logbook* logbook ):
    CustomDialog( parent, CloseButton )
{
    Debug::Throw( "LogbookStatisticsDialog::LogbookStatisticsDialog.\n" );

    setWindowTitle( "Logbook Statistics - Elogbook" );
    setOptionName( "LOGBOOK_STATISTICS_DIALOG" );

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setSpacing(10);
    hLayout->setMargin(5);
    mainLayout().addLayout( hLayout );

    QLabel* label = new QLabel(this);
    label->setPixmap( IconEngine::get( ICONS::INFORMATION ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignTop );

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(2);
    hLayout->addLayout( gridLayout, 0 );

    // file
    gridLayout->addWidget( label = new QLabel( "File: ", this ), 0, 0 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    gridLayout->addWidget( label = new QLabel( logbook->file().expand(), this ), 0, 1 );
    QFont font( label->font() );
    font.setWeight( QFont::Bold );
    label->setFont( font );
    label->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );

    // creation time
    if( logbook->creation().isValid() )
    {
        gridLayout->addWidget( label = new QLabel( "Created: ", this ), 1, 0 );
        gridLayout->addWidget( new QLabel( logbook->creation().toString(), this ), 1, 1 );
        label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    }

    // modification time
    if( logbook->modification().isValid() )
    {
        gridLayout->addWidget( label = new QLabel( "Last modified: ", this ), 2, 0 );
        gridLayout->addWidget( new QLabel( logbook->modification().toString(), this ), 2, 1 );
        label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    }

    // backup time
    if( logbook->backup().isValid() )
    {
        gridLayout->addWidget( label = new QLabel( "Last backup: ", this ), 3, 0 );
        gridLayout->addWidget( new QLabel( logbook->backup().toString(), this ), 3, 1 );
        label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    }

    // stores all children

    // total number of entries
    gridLayout->addWidget( label = new QLabel( "Entries: ", this ), 4, 0 );
    gridLayout->addWidget( new QLabel( Str().assign<unsigned int>(logbook->entries().size()), this ), 4, 1 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    // total number of attachments
    gridLayout->addWidget( label = new QLabel( "Attachments: ", this ), 5, 0 );
    gridLayout->addWidget( new QLabel( Str().assign<unsigned int>( logbook->attachments().size() ), this ), 5, 1 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    gridLayout->setColumnStretch( 1, 1 );


    // detail
    TreeView *listView( new TreeView( this ) );
    listView->setModel( &model_ );
    listView->setSortingEnabled( false );
    mainLayout().addWidget( listView, 1 );

    Logbook::List all( logbook->children() );
    all.push_front( logbook );
    model_.add( all );

    listView->resizeColumns();

}


//_______________________________________________
const QString LogbookStatisticsDialog::Model::columnTitles_[ LogbookStatisticsDialog::Model::nColumns ] =
{
    "File",
    "Entries",
    "Created",
    "Modified"
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
