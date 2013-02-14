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

#include "AnimatedLineEditor.h"
#include "GridLayout.h"
#include "TextEditor.h"
#include "Debug.h"
#include "ElidedLabel.h"
#include "File.h"
#include "OpenAttachmentDialog.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QTextStream>

//_____________________________________________________
OpenAttachmentDialog::OpenAttachmentDialog( QWidget* parent, const Attachment& attachment ):
CustomDialog( parent, OkButton|CancelButton|Separator )
{

    Debug::Throw( "OpenAttachmentDialog::OpenAttachmentDialog.\n" );
    setOptionName( "OPEN_ATTACHMENT_DIALOG" );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
    mainLayout().addLayout( gridLayout, 0 );

    // attachment full name
    File fullname( ( attachment.type() == AttachmentType::URL ) ? attachment.file() : attachment.file().expand() );

    // file name
    QLabel* label;
    gridLayout->addWidget( label = new QLabel( "File:", this ) );
    gridLayout->addWidget( label = new ElidedLabel( fullname, this ) );
    label->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );

    // attachment type
    gridLayout->addWidget(new QLabel( "Type:", this ) );
    gridLayout->addWidget( new QLabel( attachment.type().name(), this ) );

    // creation
    gridLayout->addWidget( new QLabel( "Created:", this ) );
    gridLayout->addWidget( new QLabel( attachment.creation().isValid() ? attachment.creation().toString():"-", this ) );

    // modification
    if( attachment.modification().isValid() )
    {
        gridLayout->addWidget( new QLabel( "Modified:", this ) );
        gridLayout->addWidget( new QLabel( attachment.modification().toString(), this ) );
    }

    // horizontal separator
    QFrame* frame;
    mainLayout().addWidget( frame = new QFrame( this ) );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    // radio buttons
    QButtonGroup* group = new QButtonGroup( this );
    group->setExclusive( true );

    gridLayout = new GridLayout();
    mainLayout().addLayout( gridLayout );
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);

    gridLayout->addWidget( openRadioButton_ = new QRadioButton( "Open using:", this ) );
    openRadioButton_->setToolTip( "Select this button to open attachment using the selected application." );
    group->addButton( openRadioButton_ );

    gridLayout->addWidget( commandEditor_ = new BrowsedLineEditor( this ) );
    commandEditor_->setFile( attachment.type().editCommand() );
    commandEditor_->setToolTip( "Application to be used to display the attachment." );

    gridLayout->addWidget( saveRadioButton_ = new QRadioButton( "save to disk ", this ) );
    saveRadioButton_->setToolTip( "Select this button to save a copy of the attachment on disk." );
    group->addButton( saveRadioButton_ );

    if( attachment.type() == AttachmentType::URL )
    {

        openRadioButton_->setChecked( true );
        saveRadioButton_->setChecked( false );
        saveRadioButton_->setEnabled( false );

    } else {

        openRadioButton_->setChecked( true );
        saveRadioButton_->setChecked( false );
        saveRadioButton_->setEnabled( true );
    }

    // comments
    if( !( attachment.comments().isEmpty() || attachment.comments()== Attachment::NO_COMMENTS ) )
    {
        mainLayout().addWidget( new QLabel( "Comments:", this ), 0 );
        TextEditor* comments_text_edit = new TextEditor( this );
        mainLayout().addWidget( comments_text_edit, 1 );
        comments_text_edit->setReadOnly( true );
        comments_text_edit->setPlainText( attachment.comments() );
        comments_text_edit->setToolTip( "Attachment comments. (read-only)" );
    }

    adjustSize();

}

//______________________________________________________
QString OpenAttachmentDialog::command( void ) const
{ return commandEditor_->editor().text(); }

//______________________________________________________
OpenAttachmentDialog::Action OpenAttachmentDialog::action( void ) const
{ return openRadioButton_->isChecked() ? OPEN:SAVE_AS; }
