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

#include "EditAttachmentDialog.h"

#include "AnimatedLineEditor.h"
#include "Debug.h"
#include "File.h"
#include "GridLayout.h"

#include <QLayout>
#include <QLabel>

//_____________________________________________________
EditAttachmentDialog::EditAttachmentDialog( QWidget* parent, const Attachment& attachment ):
    CustomDialog( parent )
{

    Debug::Throw( "EditAttachmentDialog::EditAttachmentDialog.\n" );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
    mainLayout().addLayout( gridLayout );

    // file name
    QLabel* label;
    gridLayout->addWidget( label = new QLabel( tr( "File:" ), this ) );
    AnimatedLineEditor *fileLineEdit( new AnimatedLineEditor( this ) );
    fileLineEdit->setReadOnly( true );
    fileLineEdit->setToolTip( tr( "Attachment file/URL. (read-only)" ) );
    gridLayout->addWidget( fileLineEdit );
    label->setBuddy( fileLineEdit );

    AttachmentType type( attachment.type() );
    File fullname( ( type == AttachmentType::Url ) ? attachment.file():attachment.file().expand() );
    fileLineEdit->setText( fullname );

    // type
    gridLayout->addWidget( label = new QLabel( tr( "Type:" ), this ) );
    gridLayout->addWidget( fileTypeComboBox_ = new QComboBox( this ) );
    for(
        AttachmentType::Map::const_iterator iter = AttachmentType::types().begin();
        iter != AttachmentType::types().end();
        iter ++ )
    {

        if( type == AttachmentType::Url && !( iter.value() == AttachmentType::Url ) ) continue;
        if( !( type == AttachmentType::Url ) && iter.value() == AttachmentType::Url ) continue;
        fileTypeComboBox_->addItem( iter.value().name() );

    }
    fileTypeComboBox_->setCurrentIndex( fileTypeComboBox_->findText( type.name() ) );
    fileTypeComboBox_->setToolTip( tr( "Attachment type. Defines the default application used to display the attachment" ) );
    label->setBuddy( fileTypeComboBox_ );

    // creation
    if( attachment.creation().isValid() )
    {
        gridLayout->addWidget( new QLabel( tr( "Created:" ), this ) );
        gridLayout->addWidget( new QLabel( attachment.creation().toString(), this ) );
    }

    // modification
    if( attachment.modification().isValid() )
    {
        gridLayout->addWidget( new QLabel( tr( "Modified:" ), this ) );
        gridLayout->addWidget( new QLabel( attachment.modification().toString(), this ) );
    }

    gridLayout->setColumnStretch( 1, 1 );

    QVBoxLayout* boxLayout = new QVBoxLayout();
    boxLayout->setMargin(0);
    boxLayout->setSpacing(5);
    mainLayout().addLayout( boxLayout, 1 );

    boxLayout->addWidget( label = new QLabel( tr( "Comments:" ), this ), 0 );
    boxLayout->addWidget( commentsEditor_ = new TextEditor( this ), 1 );
    commentsEditor_->setPlainText( attachment.comments() );
    commentsEditor_->setToolTip( tr( "Attachment comments" ) );
    label->setBuddy( commentsEditor_ );
}

//____________________________________________
AttachmentType EditAttachmentDialog::type( void ) const
{

    Debug::Throw( "EditAttachmentDialog::GetType.\n" );
    QString type_string( fileTypeComboBox_->currentText() );
    for(
        AttachmentType::Map::const_iterator iter = AttachmentType::types().begin();
        iter != AttachmentType::types().end();
        ++iter )
    { if( iter.value().name() == type_string ) return iter.value(); }
    return AttachmentType::Unknown;

}


//____________________________________________________
QString EditAttachmentDialog::comments( void ) const
{
    Debug::Throw( "EditAttachmentDialog::comments.\n" );
    return commentsEditor_->toPlainText();
}
