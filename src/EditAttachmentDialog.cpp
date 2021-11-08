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
#include "Debug.h"
#include "File.h"
#include "GridLayout.h"
#include "LineEditor.h"
#include "QtUtil.h"


#include <QLayout>
#include <QLabel>

//_____________________________________________________
EditAttachmentDialog::EditAttachmentDialog( QWidget* parent, const Attachment& attachment ):
    Dialog( parent )
{

    Debug::Throw( QStringLiteral("EditAttachmentDialog::EditAttachmentDialog.\n") );

    GridLayout* gridLayout = new GridLayout;
    QtUtil::setMargin(gridLayout, 0);
    gridLayout->setSpacing(5);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
    mainLayout().addLayout( gridLayout );

    // file name
    QLabel* label;
    gridLayout->addWidget( label = new QLabel( tr( "File:" ), this ) );
    LineEditor *fileLineEdit( new LineEditor( this ) );
    fileLineEdit->setReadOnly( true );
    fileLineEdit->setToolTip( tr( "Attachment file/URL. (read-only)" ) );
    gridLayout->addWidget( fileLineEdit );
    label->setBuddy( fileLineEdit );

    File fullname( ( attachment.isUrl() ) ? attachment.file():attachment.file().expanded() );
    fileLineEdit->setText( fullname );

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

    QVBoxLayout* boxLayout = new QVBoxLayout;
    QtUtil::setMargin(boxLayout, 0);
    boxLayout->setSpacing(5);
    mainLayout().addLayout( boxLayout, 1 );

    boxLayout->addWidget( label = new QLabel( tr( "Comments:" ), this ), 0 );
    boxLayout->addWidget( commentsEditor_ = new TextEditor( this ), 1 );
    commentsEditor_->setPlainText( attachment.comments() );
    commentsEditor_->setToolTip( tr( "Attachment comments" ) );
    label->setBuddy( commentsEditor_ );
}

//____________________________________________________
QString EditAttachmentDialog::comments() const
{
    Debug::Throw( QStringLiteral("EditAttachmentDialog::comments.\n") );
    return commentsEditor_->toPlainText();
}
