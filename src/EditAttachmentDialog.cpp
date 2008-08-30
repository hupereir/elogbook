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
   \file EditAttachmentDialog.cpp
   \brief open attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLayout>
#include <QLabel>

#include "GridLayout.h"
#include "LineEditor.h"
#include "Debug.h"
#include "EditAttachmentDialog.h"
#include "File.h"

using namespace std;

//_____________________________________________________
EditAttachmentDialog::EditAttachmentDialog( QWidget* parent, const Attachment& attachment ):
  CustomDialog( parent )
{
  
  Debug::Throw( "EditAttachmentDialog::EditAttachmentDialog.\n" );

  // file name
  mainLayout().addWidget( new QLabel( "File:", this ) );
  LineEditor *file_line_edit( new LineEditor( this ) );
  file_line_edit->setReadOnly( true );
  file_line_edit->setToolTip( "Attachment file/URL. (read-only)" );
  mainLayout().addWidget( file_line_edit );
  
  AttachmentType type( attachment.type() );
  File fullname( ( type == AttachmentType::URL ) ? attachment.file():attachment.file().expand() );  
  file_line_edit->setText( fullname.c_str() );

  GridLayout* grid_layout = new GridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  grid_layout->setMaxCount(2);
  mainLayout().addLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "Type:", this ) );
  grid_layout->addWidget( file_type_combo_box_ = new QComboBox( this ) );
  for( 
    std::map<string, AttachmentType>::const_iterator iter = AttachmentType::types().begin(); 
    iter != AttachmentType::types().end();
    iter ++ )
  {
    
    if( type == AttachmentType::URL && !( iter->second == AttachmentType::URL ) ) continue;
    if( !( type == AttachmentType::URL ) && iter->second == AttachmentType::URL ) continue;
    file_type_combo_box_->addItem( iter->second.name().c_str() );
    
  }
  file_type_combo_box_->setCurrentIndex( file_type_combo_box_->findText( type.name().c_str() ) );
  file_type_combo_box_->setToolTip( "Attachment type. Defines the default application used to display the attachment." );

  grid_layout->setColumnStretch( 1, 1 );

  QVBoxLayout* box_layout = new QVBoxLayout();
  box_layout->setMargin(0);
  box_layout->setSpacing(5);
  mainLayout().addLayout( box_layout, 1 );
  
  box_layout->addWidget( new QLabel( "Comments:", this ), 0 );  
  box_layout->addWidget( comments_text_edit_ = new TextEditor( this ), 1 );
  comments_text_edit_->setPlainText( attachment.comments().c_str() );
  comments_text_edit_->setToolTip( "Attachment comments." );
  
  //adjustSize();
  
}
  
//____________________________________________
AttachmentType EditAttachmentDialog::type( void ) const
{

  Debug::Throw( "EditAttachmentDialog::GetType.\n" );
  string type_string( qPrintable( file_type_combo_box_->currentText() ) );
  for( 
    std::map<string, AttachmentType>::const_iterator iter = AttachmentType::types().begin(); 
    iter != AttachmentType::types().end();
    iter++ ) 
  { if( iter->second.name() == type_string ) return iter->second; }
  return AttachmentType::UNKNOWN;
  
}
  

//____________________________________________________
string EditAttachmentDialog::comments( void ) const
{
  Debug::Throw( "EditAttachmentDialog::comments.\n" );
  return qPrintable( comments_text_edit_->toPlainText() );
}
