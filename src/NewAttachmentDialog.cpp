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

/*!
   \file NewAttachmentDialog.cpp
   \brief new attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLayout>
#include <QLabel>

#include "AnimatedLineEditor.h"
#include "GridLayout.h"
#include "Debug.h"
#include "NewAttachmentDialog.h"

using namespace std;

//_____________________________________________________
NewAttachmentDialog::NewAttachmentDialog( QWidget* parent ):
  CustomDialog( parent )
{

  Debug::Throw( "NewAttachmentDialog::NewAttachmentDialog.\n" );
  setWindowTitle( "New Attachment - Elogbook" );

  GridLayout* gridLayout = new GridLayout();
  gridLayout->setMargin(0);
  gridLayout->setSpacing(5);
  gridLayout->setMaxCount(2);
  gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
  mainLayout().addLayout( gridLayout, 0 );

  // source file
  gridLayout->addWidget( new QLabel( "Source file:", this ) );
  gridLayout->addWidget( fileEditor_ = new BrowsedLineEditor( this ) );
  fileEditor_->setFileMode( QFileDialog::ExistingFile );
  fileEditor_->setToolTip( "Attachment source file (or URL) to be stored in logbook." );

  gridLayout->addWidget( new QLabel( "Destination directory:", this ) );
  gridLayout->addWidget( destinationDirectoryEditor_ = new BrowsedLineEditor( this ) );
  destinationDirectoryEditor_->setFileMode( QFileDialog::DirectoryOnly );
  destinationDirectoryEditor_->setToolTip( "Attachment directory where attached file is stored (either copied or linked)." );

  gridLayout->addWidget( new QLabel( "Type:", this ) );
  gridLayout->addWidget( fileTypeComboBox_ = new QComboBox( this ) );
  for(
    AttachmentType::Map::const_iterator iter = AttachmentType::types().begin();
    iter != AttachmentType::types().end();
    iter ++ )
  { fileTypeComboBox_->addItem( iter->second.name() ); }
  fileTypeComboBox_->setToolTip( "Attachment type. Defines the default application used to display the attachment." );
  connect( fileTypeComboBox_, SIGNAL( activated( int ) ), SLOT( _attachmentTypeChanged( int ) ) );

  // action
  gridLayout->addWidget( new QLabel( "Action:", this ) );
  gridLayout->addWidget( actionComboBox_ = new QComboBox( this ) );
  const char *actions[] = { "Copy", "Link", 0 };
  for( int i=0; actions[i]; i++ )
  actionComboBox_->addItem( actions[i] );
  actionComboBox_->setCurrentIndex( 0 );
  actionComboBox_->setToolTip( "Action to perform in order to save the attachment." );

  gridLayout->setColumnStretch( 1, 1 );

  // comments
  mainLayout().addWidget( new QLabel( "Comments:", this ), 0 );
  mainLayout().addWidget( commentsEditor_ = new TextEditor( this ), 1 );
  commentsEditor_->setToolTip( "Attachment comments." );

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
  return type() == AttachmentType::URL ? out : out.expand();
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
    iter++ )
  { if( iter->second.name() == type ) return iter->second; }
  return AttachmentType::UNKNOWN;

}

//____________________________________________________
void NewAttachmentDialog::setAction( const Attachment::Command& command )
{
  Debug::Throw( "NewAttachmentDialog::SetAction.\n" );
  QString action_string = ( command == Attachment::COPY_VERSION ) ? "Copy":"Link";
  actionComboBox_->setCurrentIndex( actionComboBox_->findText( action_string ) );
}

//____________________________________________________
Attachment::Command NewAttachmentDialog::action( void ) const
{ return actionComboBox_->currentText() == "Copy" ? Attachment::COPY_VERSION: Attachment::LINK_VERSION; }

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
void NewAttachmentDialog::_attachmentTypeChanged( int index )
{
  bool enabled = !( type() == AttachmentType::URL );
  destinationDirectoryEditor_->setEnabled( enabled );
  actionComboBox_->setEnabled( enabled );
  return;
}
