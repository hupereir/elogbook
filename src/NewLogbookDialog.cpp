
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

#include "Debug.h"
#include "NewLogbookDialog.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>

//_______________________________________________________
NewLogbookDialog::NewLogbookDialog( QWidget* parent ):
CustomDialog( parent )
{

    Debug::Throw( "NewLogbookDialog::NewLogbookDialog.\n" );
    setWindowTitle( "New Logbook - Elogbook" );
    QGridLayout *gridLayout( new QGridLayout() );
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    mainLayout().addLayout( gridLayout, 0 );

    // title
    QLabel* label;
    gridLayout->addWidget( label = new QLabel( "Title:", this ), 0, 0 );
    gridLayout->addWidget( title_ = new AnimatedLineEditor( this ), 0, 1 );
    title_->setToolTip( "Logbook title" );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    label->setBuddy( title_ );

    // logbook author
    gridLayout->addWidget( label = new QLabel( "Author:", this ), 1, 0 );
    gridLayout->addWidget( author_ = new AnimatedLineEditor( this ), 1, 1 );
    author_->setToolTip( "Logbook author." );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    label->setBuddy( author_ );

    // attachment directory
    gridLayout->addWidget( label = new QLabel( "Attachment directory:", this ), 3, 0 );
    gridLayout->addWidget( attachmentDirectory_ = new BrowsedLineEditor( this ), 3, 1 );
    attachmentDirectory_->setFileMode( QFileDialog::DirectoryOnly );
    attachmentDirectory_->setAcceptMode( QFileDialog::AcceptSave );
    attachmentDirectory_->setToolTip( "Default directory where attached files are stored (either copied or linked)." );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    label->setBuddy( attachmentDirectory_ );

    gridLayout->setColumnStretch( 1, 1 );

    // separator
    QFrame* frame;
    mainLayout().addWidget( frame = new QFrame() );
    frame->setFrameStyle( QFrame::HLine );

    // comments
    mainLayout().addWidget( label = new QLabel( "Comments:", this ), 0 );
    mainLayout().addWidget( comments_ = new TextEditor( this ), 1 );
    comments_->setToolTip( "Logbook comments." );
    Debug::Throw( "NewLogbookDialog::NewLogbookDialog - done.\n" );
    label->setBuddy( comments_ );

    adjustSize();

}
