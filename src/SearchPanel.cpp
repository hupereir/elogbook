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

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLayout>

#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "LineEditor.h"
#include "SearchPanel.h"
#include "XmlOptions.h"

using namespace std;

//___________________________________________________________
SearchPanel::SearchPanel( const QString& title, QWidget* parent, const std::string& option_name ):
  CustomToolBar( title, parent, option_name )
{
  Debug::Throw( "SearchPanel::SearchPanel.\n" );
    
  // find selection button
  QPushButton* button;
  addWidget( button = new QPushButton( IconEngine::get( ICONS::FIND ), "&Find", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _selectionRequest() ) ); 
  button->setToolTip( "Find logbook entries matching selected text" );
  
  // selection text
  editor_ = new CustomComboBox( this );
  _editor().setAutoCompletion( true );
  _editor().setEditable( true );
  _editor().setToolTip( "Text to be found in logbook" );
  _editor().setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  addWidget( editor_ ); 
  connect( &_editor(), SIGNAL( activated( const QString& ) ), SLOT( _selectionRequest() ) );
  
  addWidget( new QLabel( " in ", this ) );
  
  addWidget( checkboxes_[TITLE] = new QCheckBox( "&Title", this ) );
  addWidget( checkboxes_[KEYWORD] = new QCheckBox( "&Keyword", this ) );
  addWidget( checkboxes_[TEXT]  = new QCheckBox( "&Text", this ) );
  addWidget( checkboxes_[ATTACHMENT] = new QCheckBox( "&Attachment", this ) );
  addWidget( checkboxes_[COLOR] = new QCheckBox( "&Color", this ) );
  checkboxes_[TEXT]->setChecked( true );

  // show_all button
  addWidget( button = new QPushButton( "&Show All", this ) );
  connect( button, SIGNAL( clicked() ), this, SIGNAL( showAllEntries() ) ); 
  button->setToolTip( "Show all logbook entries" );
    
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( saveConfiguration() ), SLOT( _saveConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();
  
}

//_______________________________________________________________
void SearchPanel::_toggleVisibility( bool state )
{

  Debug::Throw( "SearchPanel::_toggleVisibility.\n" );
  CustomToolBar::_toggleVisibility( state );
  if( state ) _editor().setFocus();

}
  
//___________________________________________________________
void SearchPanel::_updateConfiguration( void )
{

  Debug::Throw( "SearchPanel::_updateConfiguration.\n" );
  
  // load mask
  if( XmlOptions::get().find( "SEARCHPANEL_MASK" ) )
  {
    unsigned int mask( XmlOptions::get().get<unsigned int>( "SEARCHPANEL_MASK" ) );
    for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); iter++ )
    { iter->second->setChecked( mask & iter->first ); }
  }
  
}
  
//___________________________________________________________
void SearchPanel::_saveConfiguration( void )
{

  Debug::Throw( "SearchPanel::_saveConfiguration.\n" );
  // XmlOptions::get().set<bool>( "SHOW_SEARCHPANEL", visibilityAction().isChecked() );

  // store mask
  unsigned int mask(0);
  for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); iter++ )
  { if( iter->second->isChecked() ) mask |= iter->first; }
  
  XmlOptions::get().set<unsigned int>( "SEARCHPANEL_MASK", mask );

}

//___________________________________________________________
void SearchPanel::_selectionRequest( void )
{  
  Debug::Throw( "SearchPanel::_selectionRequest.\n" );
  
  // build mode
  unsigned int mode = NONE;
  for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); iter++ )
  { if( iter->second->isChecked() ) mode |= iter->first; }
  
  // text selection
  emit selectEntries( _editor().currentText(), mode );

}
