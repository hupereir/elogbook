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
#include "Debug.h"
#include "NewAttachmentDialog.h"
#include "NewAttachmentDialog.moc"

#include <QLayout>
#include <QLabel>

//_____________________________________________________
NewAttachmentDialog::NewAttachmentDialog( QWidget* parent ):
CustomDialog( parent )
{

    Debug::Throw( "NewAttachmentDialog::NewAttachmentDialog.\n" );
    setWindowTitle( tr( "New Attachment - Elogbook" ) );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing(5);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
    mainLayout().addLayout( gridLayout, 0 );

    // source file
    gridLayout->addWidget( new QLabel( tr( "Source file:" ), this ) );
    gridLayout->addWidget( fileEditor_ = new BrowsedLineEditor( this ) );
    fileEditor_->setFileMode( QFileDialog::ExistingFile );
    fileEditor_->setToolTip( tr( "Attachment source file (or URL) to be stored in logbook" ) );

    gridLayout->addWidget( new QLabel( tr( "Destination directory:" ), this ) );
    gridLayout->addWidget( destinationDirectoryEditor_ = new BrowsedLineEditor( this ) );
    destinationDirectoryEditor_->setFileMode( QFileDialog::DirectoryOnly );
    destinationDirectoryEditor_->setToolTip( tr( "Attachment directory where attached file is stored (either copied or linked)." ) );

    gridLayout->addWidget( new QLabel( tr( "Type:" ), this ) );
    gridLayout->addWidget( fileTypeComboBox_ = new QComboBox( this ) );
    for(
        AttachmentType::Map::const_iterator iter = AttachmentType::types().begin();
    iter != AttachmentType::types().end();
    iter ++ )
    { fileTypeComboBox_->addItem( iter.value().name() ); }
    fileTypeComboBox_->setToolTip( tr( "Attachment type. Defines the default application used to display the attachment" ) );
    connect( fileTypeComboBox_, SIGNAL(activated(int)), SLOT(_attachmentTypeChanged(int)) );

    // action
    gridLayout->addWidget( new QLabel( tr( "Action:" ), this ) );
    gridLayout->addWidget( actionComboBox_ = new QComboBox( this ) );
    QString actions[] = { tr( "Copy" ), tr( "Link" ), QString() };
    for( int i=0; !actions[i].isEmpty(); i++ )
    { actionComboBox_->addItem( actions[i] ); }

    actionComboBox_->setCurrentIndex( 0 );
    actionComboBox_->setToolTip( tr( "Action to perform in order to save the attachment." ) );

    gridLayout->setColumnStretch( 1, 1 );

    // comments
    mainLayout().addWidget( new QLabel( tr( "Comments:" ), this ), 0 );
    mainLayout().addWidget( commentsEditor_ = new TextEditor( this ), 1 );
    commentsEditor_->setToolTip( tr( "Attachment comments." ) );

    adjustSize();

}

//____________________________________________________
void NewAttachmentDialog::setFile( const File& file )
{

    Debug::Throw( "NewAttachmentDialog::SetFile.\n" );
    fileEditor_->setFile( file );

}

//____________________________________________________
File NewAttachmentDialog::file( void ) const
{
    File out( fileEditor_->editor().text() );
    return type() == AttachmentType::Url ? out : out.expand();
}

//____________________________________________________
void NewAttachmentDialog::setDestinationDirectory( const File& file )
{
    Debug::Throw( "NewAttachmentDialog::SetDestinationDirectory.\n" );
    destinationDirectoryEditor_->setFile( file );
}

//____________________________________________________
File NewAttachmentDialog::destinationDirectory( void ) const
{ return File( destinationDirectoryEditor_->editor().text() ).expand(); }

//____________________________________________________
void NewAttachmentDialog::setType( const AttachmentType& type )
{
    Debug::Throw( "NewAttachmentDialog::setType.\n" );
    fileTypeComboBox_->setCurrentIndex( fileTypeComboBox_->findText( type.name() ) );
}

//____________________________________________________
AttachmentType NewAttachmentDialog::type( void ) const
{

    Debug::Throw( "NewAttachmentDialog::GetType.\n" );
    QString type( fileTypeComboBox_->currentText() );
    for(
        AttachmentType::Map::const_iterator iter = AttachmentType::types().begin();
    iter != AttachmentType::types().end();
    ++iter )
    { if( iter.value().name() == type ) return iter.value(); }
    return AttachmentType::Unknown;

}

//____________________________________________________
void NewAttachmentDialog::setAction( const Attachment::Command& command )
{
    Debug::Throw( "NewAttachmentDialog::SetAction.\n" );
    QString actionString = ( command == Attachment::CopyVersion ) ? tr( "Copy" ) : tr( "Link" );
    actionComboBox_->setCurrentIndex( actionComboBox_->findText( actionString ) );
}

//____________________________________________________
Attachment::Command NewAttachmentDialog::action( void ) const
{ return actionComboBox_->currentText() == tr( "Copy" ) ? Attachment::CopyVersion: Attachment::LinkVersion; }

//____________________________________________________
void NewAttachmentDialog::setComments( const QString& comments )
{
    Debug::Throw( "NewAttachmentDialog::SetComments.\n" );
    commentsEditor_->setPlainText( comments );
}

//____________________________________________________
QString NewAttachmentDialog::comments( void ) const
{
    Debug::Throw( "NewAttachmentDialog::GetComments.\n" );
    return commentsEditor_->toPlainText();
}

//____________________________________________________
void NewAttachmentDialog::_attachmentTypeChanged( int )
{
    bool enabled = !( type() == AttachmentType::Url );
    destinationDirectoryEditor_->setEnabled( enabled );
    actionComboBox_->setEnabled( enabled );
    return;
}
