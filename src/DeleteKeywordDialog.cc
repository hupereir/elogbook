
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
   \file DeleteKeywordDialog.cc
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
#include "DeleteKeywordDialog.h"

using namespace std;

//_____________________________________________________
DeleteKeywordDialog::DeleteKeywordDialog( QWidget* parent, const string& keyword, const bool& has_entries ):
  CustomDialog( parent )
{
  
  Debug::Throw( "DeleteKeywordDialog::DeleteKeywordDialog.\n" );

  setWindowTitle( "delete keyword" );
  
  // radio buttons
  QButtonGroup* group = new QButtonGroup( this );
  group->set( true );

  ostringstream what;
  what << "Delete keyword " << keyword << " ?";
  mainLayout().addWidget( new QLabel( what.str().c_str(), this ) );
  
  QGroupBox *group_box = new QGroupBox( this );
  mainLayout().addWidget( group_box );
  group_box->setLayout( new QVBoxLayout() );
  group_box->layout()->setMargin(5);
  group_box->layout()->setSpacing(5);
        
  group_box->layout()->addWidget( move_radio_button_ = new QRadioButton( "Move entries to parent keyword", this ) );
  move_radio_button_->setToolTip( "Select this button to move entries associated to this keyword to the parent keyword." );
  group->addButton( move_radio_button_ );
  
  group_box->layout()->addWidget( delete_radio_button_ = new QRadioButton( "Delete entries", this ) );
  delete_radio_button_->setToolTip( "Select this button to delete entries associated to this keyword." );
  group->addButton( move_radio_button_ );
  move_radio_button_->setChecked( true );
  
  if( !has_entries ) group_box->setEnabled( false );
}
