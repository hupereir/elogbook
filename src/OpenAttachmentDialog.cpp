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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "AnimatedLineEditor.h"
#include "GridLayout.h"
#include "TextEditor.h"
#include "Debug.h"
#include "ElidedLabel.h"
#include "File.h"
#include "OpenAttachmentDialog.h"
#include "OpenAttachmentDialog.moc"

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
    File fullname( ( attachment.type() == AttachmentType::Url ) ? attachment.file() : attachment.file().expand() );

    // file name
    QLabel* label;
    gridLayout->addWidget( label = new QLabel( tr( "File:" ), this ) );
    gridLayout->addWidget( label = new ElidedLabel( fullname, this ) );
    label->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );

    // attachment type
    gridLayout->addWidget(new QLabel( tr( "Type:" ), this ) );
    gridLayout->addWidget( new QLabel( attachment.type().name(), this ) );

    // creation
    gridLayout->addWidget( new QLabel( tr( "Created:" ), this ) );
    gridLayout->addWidget( new QLabel( attachment.creation().isValid() ? attachment.creation().toString():"-", this ) );

    // modification
    if( attachment.modification().isValid() )
    {
        gridLayout->addWidget( new QLabel( tr( "Modified:" ), this ) );
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

    gridLayout->addWidget( openRadioButton_ = new QRadioButton( tr( "Open using:" ), this ) );
    openRadioButton_->setToolTip( tr( "Select this button to open attachment using the selected application" ) );
    group->addButton( openRadioButton_ );

    gridLayout->addWidget( commandEditor_ = new BrowsedLineEditor( this ) );
    commandEditor_->setFile( attachment.type().editCommand() );
    commandEditor_->setToolTip( tr( "Application to be used to display the attachment" ) );

    gridLayout->addWidget( saveRadioButton_ = new QRadioButton( tr( "Save to disk" ), this ) );
    saveRadioButton_->setToolTip( tr( "Select this button to save a copy of the attachment on disk" ) );
    group->addButton( saveRadioButton_ );

    if( attachment.type() == AttachmentType::Url )
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
    if( !( attachment.comments().isEmpty() || attachment.comments()== Attachment::NoComments ) )
    {
        mainLayout().addWidget( new QLabel( tr( "Comments:" ), this ), 0 );
        TextEditor* commentsTextEdit = new TextEditor( this );
        mainLayout().addWidget( commentsTextEdit, 1 );
        commentsTextEdit->setReadOnly( true );
        commentsTextEdit->setPlainText( attachment.comments() );
        commentsTextEdit->setToolTip( tr( "Attachment comments. (read-only)" ) );
    }

    adjustSize();

}

//______________________________________________________
QString OpenAttachmentDialog::command( void ) const
{ return commandEditor_->editor().text(); }

//______________________________________________________
OpenAttachmentDialog::Action OpenAttachmentDialog::action( void ) const
{ return openRadioButton_->isChecked() ? Open:SaveAs; }
