
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
   \file DeleteAttachmentDialog.cpp
   \brief Delete attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

#include "Debug.h"
#include "DeleteAttachmentDialog.h"

using namespace std;

//_____________________________________________________
DeleteAttachmentDialog::DeleteAttachmentDialog( QWidget* parent, const Attachment& attachment ):
  CustomDialog( parent )
{
  
  Debug::Throw( "DeleteAttachmentDialog::DeleteAttachmentDialog.\n" );
      
  // radio buttons
  QButtonGroup* group = new QButtonGroup( this );
  group->setExclusive( true );

  mainLayout().addWidget( new QLabel( "Delete attachment ?", this ) );
  
  QGroupBox *group_box = new QGroupBox( this );
  mainLayout().addWidget( group_box );
  group_box->setLayout( new QVBoxLayout() );
  group_box->layout()->setMargin(5);
  group_box->layout()->setSpacing(5);
  
  group_box->layout()->addWidget( from_disk_radio_button_ = new QRadioButton( "From disk", group_box ) );
  from_disk_radio_button_->setChecked( true );
  from_disk_radio_button_->setToolTip( "Select this button to remove attachment file from disk and logbook." );
  group->addButton( from_disk_radio_button_ );
  
  group_box->layout()->addWidget( from_logbook_radio_button_ = new QRadioButton( "From frame", group_box ) );
  from_logbook_radio_button_->setToolTip( "Select this button to remove attachment file from logbook only (attachment is kept on disk)." );
  group->addButton( from_logbook_radio_button_ );

  if( attachment.type() == AttachmentType::URL )
  { group_box->setEnabled( false ); }
  
  adjustSize();
  
} 

//______________________________________________________
DeleteAttachmentDialog::Action DeleteAttachmentDialog::action( void ) const
{ return from_disk_radio_button_->isChecked() ? FROM_DISK:FROM_LOGBOOK; }
