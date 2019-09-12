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

#include "OpenAttachmentDialog.h"

#include "BaseIconNames.h"
#include "Debug.h"
#include "ElidedLabel.h"
#include "File.h"
#include "GridLayout.h"
#include "IconEngine.h"
#include "OpenWithComboBox.h"
#include "TextEditor.h"
#include "XmlOptions.h"

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>

//_____________________________________________________
OpenAttachmentDialog::OpenAttachmentDialog( QWidget* parent, const Attachment& attachment ):
CustomDialog( parent, OkButton|CancelButton|Separator )
{

    Debug::Throw( "OpenAttachmentDialog::OpenAttachmentDialog.\n" );
    setOptionName( "OPEN_ATTACHMENT_DIALOG" );

    // try load Question icon
    auto hLayout( new QHBoxLayout );
    hLayout->setSpacing(10);
    hLayout->setMargin(0);
    mainLayout().addLayout( hLayout );
    {
        // icon
        auto label = new QLabel( this );
        auto icon = IconEngine::get( IconNames::DialogWarning );
        label->setPixmap( icon.pixmap( iconSize() ) );
        hLayout->addWidget( label, 0 );

    }

    auto vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    hLayout->addLayout( vLayout, 1 );

    auto gridLayout = new GridLayout;
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
    vLayout->addStretch();
    vLayout->addLayout( gridLayout );
    vLayout->addStretch();

    // attachment full name
    File fullname( ( attachment.isUrl() ) ? attachment.file() : attachment.file().expanded() );

    // file name
    QLabel* label;
    gridLayout->addWidget( label = new QLabel( tr( "File:" ), this ) );
    gridLayout->addWidget( label = new ElidedLabel( fullname, this ) );
    static_cast<ElidedLabel*>(label)->setElideMode( Qt::ElideMiddle );
    label->setTextInteractionFlags( Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard );

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
    auto group = new QButtonGroup( this );
    group->setExclusive( true );

    gridLayout = new GridLayout;
    mainLayout().addLayout( gridLayout );
    gridLayout->setMargin(0);
    gridLayout->setMaxCount(2);

    gridLayout->addWidget( openRadioButton_ = new QRadioButton( tr( "Open using:" ), this ) );
    openRadioButton_->setToolTip( tr( "Select this button to open attachment using the selected application" ) );
    group->addButton( openRadioButton_ );

    gridLayout->addWidget( comboBox_ = new OpenWithComboBox( this ) );
    comboBox_->setToolTip( tr( "Application to be used to display the attachment" ) );

    // retrieve applications from options
    const auto applications( XmlOptions::get().specialOptions( "OPEN_ATTACHMENT_APPLICATIONS" ) );
    for( const auto& option:applications )
    { comboBox_->addItem( File( option.raw() ) ); }

    // sytem default
    comboBox_->insertItem(0, tr( "System Default" ) );
    comboBox_->setCurrentIndex( 0 );

    gridLayout->setColumnStretch( 1, 1 );

    gridLayout->addWidget( saveRadioButton_ = new QRadioButton( tr( "Save to disk" ), this ) );
    saveRadioButton_->setToolTip( tr( "Select this button to save a copy of the attachment on disk" ) );
    group->addButton( saveRadioButton_ );

    if( attachment.isUrl() )
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

    // connection
    connect( &okButton(), SIGNAL(clicked()), SLOT(_saveCommands()) );

}

//____________________________________________________________________________
bool OpenAttachmentDialog::isCommandValid() const
{ return comboBox_->isItemValid(); }

//____________________________________________________________________________
bool OpenAttachmentDialog::isCommandDefault() const
{ return comboBox_->currentIndex() == 0; }

//______________________________________________________
QString OpenAttachmentDialog::command() const
{ return comboBox_->command(); }

//______________________________________________________
OpenAttachmentDialog::Action OpenAttachmentDialog::action() const
{ return openRadioButton_->isChecked() ? Action::Open : Action::SaveAs; }

//____________________________________________________________________________
void OpenAttachmentDialog::_saveCommands()
{

    const QString optionName = "OPEN_ATTACHMENT_APPLICATIONS";
    for( const auto& command:comboBox_->newItems() )
    { XmlOptions::get().add( optionName, Option( command, Option::Flag::Recordable|Option::Flag::Current ) ); }

}
