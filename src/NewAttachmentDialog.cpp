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

#include "GridLayout.h"
#include "LineEditor.h"
#include "Debug.h"
#include "NewAttachmentDialog.h"

using namespace std;

//_____________________________________________________
NewAttachmentDialog::NewAttachmentDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "NewAttachmentDialog::NewAttachmentDialog.\n" );
  setWindowTitle( "eLogbook - new attachment" );
  
  // source file
  mainLayout().addWidget( new QLabel( "Source file:", this ) );
  mainLayout().addWidget( file_line_edit_ = new BrowsedLineEditor( this ) );
  file_line_edit_->setFileMode( QFileDialog::ExistingFile );
  file_line_edit_->setToolTip( "Attachment source file (or URL) to be stored in logbook." );
  
  // destination directory
  directory_layout_ = new QVBoxLayout();
  directory_layout_->setSpacing( 5 );
  directory_layout_->setMargin( 0 );
  mainLayout().addLayout( directory_layout_ );
  
  directory_layout_->addWidget( new QLabel( "Destination directory:", this ) );
  directory_layout_->addWidget( dest_dir_line_edit_ = new BrowsedLineEditor( this ) );
  dest_dir_line_edit_->setFileMode( QFileDialog::DirectoryOnly );
  dest_dir_line_edit_->setToolTip( "Attachment directory where attached file is stored (either copied or linked)." );
    
  GridLayout* grid_layout = new GridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  grid_layout->setMaxCount(2);
  mainLayout().addLayout( grid_layout, 0 );

  grid_layout->addWidget( new QLabel( "Type:", this ) );
  grid_layout->addWidget( file_type_combo_box_ = new QComboBox( this ) );
  for( 
    AttachmentType::Map::const_iterator iter = AttachmentType::types().begin(); 
    iter != AttachmentType::types().end();
    iter ++ )
  { file_type_combo_box_->addItem( iter->second.name() ); }
  file_type_combo_box_->setToolTip( "Attachment type. Defines the default application used to display the attachment." );
  connect( file_type_combo_box_, SIGNAL( activated( int ) ), SLOT( _attachmentTypeChanged( int ) ) );
        
  // action
  grid_layout->addWidget( new QLabel( "Action:", this ) );
  grid_layout->addWidget( action_combo_box_ = new QComboBox( this ) );
  const char *actions[] = { "Copy", "Link", 0 };
  for( int i=0; actions[i]; i++ )
  action_combo_box_->addItem( actions[i] );
  action_combo_box_->setCurrentIndex( 0 );
  action_combo_box_->setToolTip( "Action to perform in order to save the attachment." );

  grid_layout->setColumnStretch( 1, 1 );
  
  // comments
  mainLayout().addWidget( new QLabel( "Comments:", this ), 0 );  
  mainLayout().addWidget( comments_text_edit_ = new TextEditor( this ), 1 );
  comments_text_edit_->setToolTip( "Attachment comments." );
  
  adjustSize();
  
} 

//____________________________________________________
void NewAttachmentDialog::setFile( const File& file )
{
  
  Debug::Throw( "NewAttachmentDialog::SetFile.\n" );
  file_line_edit_->setFile( file );
  
}

//____________________________________________________
File NewAttachmentDialog::file( void ) const
{ 
  File out( file_line_edit_->editor().text() );
  return type() == AttachmentType::URL ? out : out.expand(); 
}

//____________________________________________________
void NewAttachmentDialog::setDestinationDirectory( const File& file )
{
  Debug::Throw( "NewAttachmentDialog::SetDestinationDirectory.\n" );
  dest_dir_line_edit_->setFile( file );
}

//____________________________________________________
File NewAttachmentDialog::destinationDirectory( void ) const
{ return File( dest_dir_line_edit_->editor().text() ).expand(); }

//____________________________________________________
void NewAttachmentDialog::setType( const AttachmentType& type )
{
  Debug::Throw( "NewAttachmentDialog::setType.\n" );
  file_type_combo_box_->setCurrentIndex( file_type_combo_box_->findText( type.name() ) );
}

//____________________________________________________
AttachmentType NewAttachmentDialog::type( void ) const
{

  Debug::Throw( "NewAttachmentDialog::GetType.\n" );
  QString type( file_type_combo_box_->currentText() );
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
  action_combo_box_->setCurrentIndex( action_combo_box_->findText( action_string ) );
}

//____________________________________________________
Attachment::Command NewAttachmentDialog::action( void ) const
{ return action_combo_box_->currentText() == "Copy" ? Attachment::COPY_VERSION: Attachment::LINK_VERSION; }

//____________________________________________________
void NewAttachmentDialog::setComments( const QString& comments )
{ 
  Debug::Throw( "NewAttachmentDialog::SetComments.\n" );
  comments_text_edit_->setPlainText( comments );
}

//____________________________________________________
QString NewAttachmentDialog::comments( void ) const
{
  Debug::Throw( "NewAttachmentDialog::GetComments.\n" );
  return comments_text_edit_->toPlainText();
}

//____________________________________________________
void NewAttachmentDialog::_attachmentTypeChanged( int index )
{
  bool enabled = !( type() == AttachmentType::URL );
  directory_layout_->setEnabled( enabled );
  action_combo_box_->setEnabled( enabled );
  return;
}
