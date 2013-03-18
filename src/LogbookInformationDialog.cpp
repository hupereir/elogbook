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

#include "LogbookInformationDialog.h"

#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "TextEditor.h"
#include "Util.h"

#include <QLayout>
#include <QLabel>

//_________________________________________________________
LogbookInformationDialog::LogbookInformationDialog( QWidget* parent, Logbook* logbook ):
    CustomDialog( parent )
{
    Debug::Throw( "LogbookInformationDialog::LogbookInformationDialog.\n" );

    setWindowTitle( tr( "Logbook Informations - Elogbook" ) );
    setOptionName( "LOGBOOK_INFORMATION_DIALOG" );

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setSpacing(10);
    hLayout->setMargin(5);
    mainLayout().addLayout( hLayout );

    QLabel* label = new QLabel(this);
    label->setPixmap( IconEngine::get( ICONS::INFORMATION ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignTop );

    QGridLayout *gridLayout( new QGridLayout() );
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    hLayout->addLayout( gridLayout, 0 );

    gridLayout->addWidget( label = new QLabel( tr( "Title:" ), this ), 0, 0 );
    gridLayout->addWidget( title_ = new AnimatedLineEditor( this ), 0, 1 );
    title_->setText( logbook->title().isEmpty() ?   Logbook::LOGBOOK_NO_TITLE:logbook->title()  );
    title_->setToolTip( tr( "Logbook title" ) );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    label->setBuddy( title_ );

    // logbook author
    gridLayout->addWidget( label = new QLabel( tr( "Author:" ), this ), 1, 0 );
    gridLayout->addWidget( author_ = new AnimatedLineEditor( this ), 1, 1 );
    author_->setText( logbook->author().isEmpty() ? Logbook::LOGBOOK_NO_AUTHOR:logbook->author() );
    author_->setToolTip( tr( "Logbook author" ) );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    label->setBuddy( author_ );

    // attachment directory
    gridLayout->addWidget( label = new QLabel( tr( "Attachment Directory:" ), this ), 2, 0 );
    gridLayout->addWidget( attachmentDirectory_ = new BrowsedLineEditor( this ), 2, 1 );
    attachmentDirectory_->setFile( logbook->directory().isEmpty() ? File(Util::workingDirectory()) : logbook->directory() );
    attachmentDirectory_->setFileMode( QFileDialog::DirectoryOnly );
    attachmentDirectory_->setToolTip( tr( "Default directory where attached files are stored (either copied or linked)." ) );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    label->setBuddy( attachmentDirectory_ );

    // readonly
    gridLayout->addWidget( readOnlyCheckBox_ = new QCheckBox( tr( "Read-only" ), this ), 3, 1 );
    readOnlyCheckBox_->setToolTip( tr( "Mark logbook as read-only. Further write to the logbook will be forbidden." ) );

    gridLayout->setColumnStretch( 1, 1 );

    // comments
    mainLayout().addWidget( label = new QLabel( tr( "Comments:" ), this ), 0 );
    mainLayout().addWidget( comments_ = new TextEditor( this ), 1 );
    comments_->setPlainText( logbook->comments() );
    comments_->setToolTip( tr( "Logbook comments." ) );
    label->setBuddy( comments_ );

    // connections
    connect( readOnlyCheckBox_, SIGNAL( toggled( bool ) ), title_, SLOT( setDisabled( bool ) ) );
    connect( readOnlyCheckBox_, SIGNAL( toggled( bool ) ), author_, SLOT( setDisabled( bool ) ) );
    connect( readOnlyCheckBox_, SIGNAL( toggled( bool ) ), attachmentDirectory_, SLOT( setDisabled( bool ) ) );
    connect( readOnlyCheckBox_, SIGNAL( toggled( bool ) ), comments_, SLOT( setDisabled( bool ) ) );
    readOnlyCheckBox_->setChecked( logbook->readOnly() );

}
