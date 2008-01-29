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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        
* for more details.                    
*                         
* You should have received a copy of the GNU General Public License along with 
* software; if not, write to the Free Software Foundation, Inc., 59 Temple    
* Place, Suite 330, Boston, MA  02111-1307 USA                          
*                        
*                        
*******************************************************************************/

/*!
  \file SearchPanel.cpp
  \brief selects entries from keyword/title/text/...
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLayout>

#include "CustomLineEdit.h"
#include "Debug.h"
#include "SearchPanel.h"

using namespace std;

//___________________________________________________________
SearchPanel::SearchPanel( QWidget* parent ):
    QWidget( parent ),
    Counter( "SearchPanel" )
{
  Debug::Throw( "SearchPanel::SearchPanel.\n" );
  
  // layout
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setSpacing( 2 );
  layout->setMargin( 0 );
  setLayout( layout );
  
  // find selection button
  QPushButton* button;
  layout->addWidget( button = new QPushButton( "&Find", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _selectionRequest() ) ); 
  button->setToolTip( "Find logbook entries matching selected text" );
  
  // selection text
  layout->addWidget( selection_ = new CustomLineEdit( this ), 1 ); 
  connect( selection_, SIGNAL( returnPressed() ), SLOT( _selectionRequest() ) );
  selection_->setToolTip( "Text to be found in logbook" );
  
  layout->addWidget( new QLabel( " in ", this ) );
  
  layout->addWidget( checkboxes_[TITLE] = new QCheckBox( "&Title", this ) );
  layout->addWidget( checkboxes_[KEYWORD] = new QCheckBox( "&Keyword", this ) );
  layout->addWidget( checkboxes_[TEXT]  = new QCheckBox( "&Text", this ) );
  layout->addWidget( checkboxes_[ATTACHMENT] = new QCheckBox( "&Attachment", this ) );
  layout->addWidget( checkboxes_[COLOR] = new QCheckBox( "&Color", this ) );
  checkboxes_[TEXT]->setChecked( true );

  // show_all button
  layout->addWidget( button = new QPushButton( "&Show All", this ) );
  connect( button, SIGNAL( clicked() ), this, SIGNAL( showAllEntries() ) ); 
  button->setToolTip( "Show all logbook entries" );
  
}

//___________________________________________________________
void SearchPanel::_selectionRequest( void )
{
  
  Debug::Throw( "SearchPanel::_selectionRequest.\n" );
  
  // build mode
  unsigned int mode = NONE;
  for( std::map<SearchMode,QCheckBox*>::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); iter++ )
  { if( iter->second->isChecked() ) mode |= iter->first; }
  
  // text selection
  emit selectEntries( selection_->text(), mode );

}
