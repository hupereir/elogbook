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
   \file OpenAttachmentDialog.cc
   \brief open attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QGroupBox>
#include <QLayout>

#include "BrowsedLineEdit.h"
#include "CustomGridLayout.h"
#include "CustomLineEdit.h"
#include "CustomTextEdit.h"
#include "Debug.h"
#include "File.h"
#include "OpenAttachmentDialog.h"
#include "QtUtil.h"

using namespace std;

//_____________________________________________________
OpenAttachmentDialog::OpenAttachmentDialog( QWidget* parent, const Attachment& attachment ):
  CustomDialog( parent )
{
  
  Debug::Throw( "OpenAttachmentDialog::OpenAttachmentDialog.\n" );

  CustomGridLayout* grid_layout = new CustomGridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  grid_layout->setMaxCount(2);
  mainLayout().addLayout( grid_layout, 0 );

  // file name
  grid_layout->addWidget(new QLabel( "File: ", this ) );
  CustomLineEdit* file_line_edit = new CustomLineEdit( this );
  grid_layout->addWidget( file_line_edit );
  file_line_edit->setReadOnly( true );
  file_line_edit->setToolTip( "Attachment file/URL. (read-only)" );

  File fullname( ( attachment.type() == AttachmentType::URL ) ? attachment.file() : attachment.file().expand() );
  file_line_edit->setText( fullname.c_str() );

  // attachment type
  grid_layout->addWidget(new QLabel( "Type: ", this ) );
  ostringstream what;
  what << "<B>" << attachment.type().name() << "</B>";
  grid_layout->addWidget( new QLabel( what.str().c_str(), this ) );
    
  // radio buttons
  QButtonGroup* group = new QButtonGroup( this );
  group->setExclusive( true );
  
  QGroupBox *group_box = new QGroupBox( this );
  mainLayout().addWidget( group_box, 0 );
  group_box->setLayout( new QVBoxLayout() );
  group_box->layout()->setMargin(5);
  group_box->layout()->setSpacing(5);

  group_box->layout()->addWidget( open_radio_button_ = new QRadioButton( "open using: ", group_box ) );
  open_radio_button_->setToolTip( "Select this button to open attachment using the selected application." );
  group->addButton( open_radio_button_ );
  
  group_box->layout()->addWidget( command_line_edit_ = new BrowsedLineEdit( group_box ) );
  command_line_edit_->setFile( attachment.type().editCommand() );
  QtUtil::expand( &command_line_edit_->editor() );
  command_line_edit_->setToolTip( "Application to be used to display the attachment." );

  group_box->layout()->addWidget( save_radio_button_ = new QRadioButton( "save to disk ", group_box ) );
  save_radio_button_->setToolTip( "Select this button to save a copy of the attachment on disk." );
  group->addButton( save_radio_button_ );

  if( attachment.type() == AttachmentType::URL )
  {
    
    open_radio_button_->setChecked( true );
    save_radio_button_->setChecked( false );
    save_radio_button_->setEnabled( false );
    
  } else {
    
    open_radio_button_->setChecked( true );
    save_radio_button_->setChecked( false );
    save_radio_button_->setEnabled( true );
  }    
      
  // comments
  if( !( attachment.comments().empty() || attachment.comments()== Attachment::NO_COMMENTS ) )
  {
    mainLayout().addWidget( new QLabel( "Comments:", this ), 0 );  
    CustomTextEdit* comments_text_edit = new CustomTextEdit( this );
    mainLayout().addWidget( comments_text_edit, 1 );  
    comments_text_edit->setReadOnly( true );
    comments_text_edit->setPlainText( attachment.comments().c_str() );
    comments_text_edit->setToolTip( "Attachment comments. (read-only)" );
  }
  
} 

//______________________________________________________
string OpenAttachmentDialog::command( void ) const
{ return qPrintable( command_line_edit_->editor().text() ); }

//______________________________________________________
OpenAttachmentDialog::Action OpenAttachmentDialog::action( void ) const
{ return open_radio_button_->isChecked() ? OPEN:SAVE_AS; }
