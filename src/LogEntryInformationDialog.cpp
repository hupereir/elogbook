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

/*!
\file LogEntryInformationDialog.cpp
\brief  logbook entry informations
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "PixmapEngine.h"
#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "LogEntryInformationDialog.h"
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

    QHBoxLayout* h_layout = new QHBoxLayout();
    h_layout->setSpacing(10);
    h_layout->setMargin(5);
    mainLayout().addLayout( h_layout );

    //! try load Question icon
    QPixmap pixmap( PixmapEngine::get( ICONS::INFORMATION ) );

    QLabel* label = new QLabel(this);
    label->setPixmap( pixmap );
    h_layout->addWidget( label, 0, Qt::AlignHCenter );

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(2);
    h_layout->addLayout( gridLayout, 1 );

    // title
    gridLayout->addWidget( label = new QLabel( "Title: ", this ), 0, 0 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    gridLayout->addWidget( label = new QLabel( entry->title(), this ), 0, 1 );
    QFont font( label->font() );
    font.setWeight( QFont::Bold );
    label->setFont( font );

    // keyword
    gridLayout->addWidget( label = new QLabel( "Keyword: ", this ), 1, 0 );
    gridLayout->addWidget( new QLabel( entry->keyword().get(), this ), 1, 1 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    gridLayout->addWidget( label = new QLabel( "Author: ", this ), 2, 0 );
    gridLayout->addWidget( new QLabel( entry->author(), this ), 2, 1 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    gridLayout->addWidget( label = new QLabel( "Creation: ", this ), 3, 0 );
    gridLayout->addWidget( new QLabel( entry->creation().toString(), this ), 3, 1 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    gridLayout->addWidget( label = new QLabel( "Modification: ", this ), 4, 0 );
    gridLayout->addWidget( new QLabel( entry->modification().toString(), this ), 4, 1 );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    // retrieve associated logbook
    int i=5;
    BASE::KeySet<Logbook> logbooks( entry );
    foreach( Logbook* logbook, logbooks )
    {

        gridLayout->addWidget( label = new QLabel( "File: ", this ), i, 0 );
        gridLayout->addWidget( new QLabel( File( logbook->file() ).localName(), this ), i, 1);
        label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );

    }

}
