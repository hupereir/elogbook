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
#include "AnimatedLineEditor.h"
#include "Debug.h"
#include "EditAttachmentDialog.h"
#include "File.h"

using namespace std;

//_____________________________________________________
EditAttachmentDialog::EditAttachmentDialog( QWidget* parent, const Attachment& attachment ):
  CustomDialog( parent )
{

  Debug::Throw( "EditAttachmentDialog::EditAttachmentDialog.\n" );

  GridLayout* grid_layout = new GridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  grid_layout->setMaxCount(2);
  grid_layout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
  mainLayout().addLayout( grid_layout );

  // file name
  grid_layout->addWidget( new QLabel( "File:", this ) );
  AnimatedLineEditor *file_line_edit( new AnimatedLineEditor( this ) );
  file_line_edit->setReadOnly( true );
  file_line_edit->setToolTip( "Attachment file/URL. (read-only)" );
  grid_layout->addWidget( file_line_edit );

  AttachmentType type( attachment.type() );
  File fullname( ( type == AttachmentType::URL ) ? attachment.file():attachment.file().expand() );
  file_line_edit->setText( fullname );

  // type
  grid_layout->addWidget( new QLabel( "Type:", this ) );
  grid_layout->addWidget( fileTypeComboBox_ = new QComboBox( this ) );
  for(
    AttachmentType::Map::const_iterator iter = AttachmentType::types().begin();
    iter != AttachmentType::types().end();
    iter ++ )
  {

    if( type == AttachmentType::URL && !( iter->second == AttachmentType::URL ) ) continue;
    if( !( type == AttachmentType::URL ) && iter->second == AttachmentType::URL ) continue;
    fileTypeComboBox_->addItem( iter->second.name() );

  }
  fileTypeComboBox_->setCurrentIndex( fileTypeComboBox_->findText( type.name() ) );
  fileTypeComboBox_->setToolTip( "Attachment type. Defines the default application used to display the attachment." );

  // creation
  grid_layout->addWidget( new QLabel( "Created: ", this ) );
  grid_layout->addWidget( new QLabel( attachment.creation().isValid() ? attachment.creation().toString():"-", this ) );

  // modification
  grid_layout->addWidget( new QLabel( "Modified: ", this ) );
  grid_layout->addWidget( new QLabel( attachment.modification().isValid() ? attachment.modification().toString():"-", this ) );

  grid_layout->setColumnStretch( 1, 1 );

  QVBoxLayout* box_layout = new QVBoxLayout();
  box_layout->setMargin(0);
  box_layout->setSpacing(5);
  mainLayout().addLayout( box_layout, 1 );

  box_layout->addWidget( new QLabel( "Comments:", this ), 0 );
  box_layout->addWidget( commentsEditor_ = new TextEditor( this ), 1 );
  commentsEditor_->setPlainText( attachment.comments() );
  commentsEditor_->setToolTip( "Attachment comments." );

}

//____________________________________________
AttachmentType EditAttachmentDialog::type( void ) const
{

  Debug::Throw( "EditAttachmentDialog::GetType.\n" );
  QString type_string( fileTypeComboBox_->currentText() );
  for(
    AttachmentType::Map::const_iterator iter = AttachmentType::types().begin();
    iter != AttachmentType::types().end();
    iter++ )
  { if( iter->second.name() == type_string ) return iter->second; }
  return AttachmentType::UNKNOWN;

}


//____________________________________________________
QString EditAttachmentDialog::comments( void ) const
{
  Debug::Throw( "EditAttachmentDialog::comments.\n" );
  return commentsEditor_->toPlainText();
}
