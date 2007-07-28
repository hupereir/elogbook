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
  \file ConfigDialog.cc
  \brief xMaze configuration dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QToolTip.h>

#include "ConfigDialog.h"

#include "Debug.h"
#include "ListViewConfig.h"
#include "MainFrame.h"
#include "OptionBrowsedLineEdit.h"
#include "OptionCheckBox.h"
#include "OptionSpinBox.h"
#include "OptionListBox.h"
#include "Options.h"
#include "SelectionFrame.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellConfig.h"
#endif

using namespace std;

//_________________________________________________________
ConfigDialog::ConfigDialog( QWidget* parent, const string& name ):
  ConfigDialogBase( parent, name )
{


  Debug::Throw( "ConfigDialog::ConfigDialog.\n" );

  baseConfiguration();

  // generic objects
  QGroupBox *box;
  OptionCheckBox *checkbox;
  OptionSpinBox* spinbox;
  QLabel* label;

  // attachment editors
  QWidget* page = &addPage( "attachments" );
  box = new QGroupBox( "editors", vbox );
  page->layout()->addWidget( box );

  QGridLayout* grid_layout = new QGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  box->setLayout( grid_layout );

  OptionBrowsedLineEdit *editor;
  
  grid_layout->addWidget( new QLabel( "default: ", box ), 0, 0 );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_UNKNOWN_ATC" ), 0, 1 );
  addOptionWidget( editor );
  
  grid_layout->addWidget( new QLabel( "HTML: ", box ), 1, 0 );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_HTML_ATC" ), 1, 1 );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "URL: ", box ), 2, 0 );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_URL_ATC" ), 2, 1 );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "text: ", box ), 3, 0 );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_PLAIN_TEXT_ATC" ), 3, 1 );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "postscript: ", box ), 4, 0 );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_POSTSCRIPT_ATC" ), 4, 1 );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "image: ", box ), 5, 0 );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_IMAGE_ATC" ), 5, 1 );
  addOptionWidget( editor );
  
  grid_layout->setColumnStretch( 1, 1 );
  
  box = new QGroupBox( "misc", vbox );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);
  page->layout()->addWidget( box );
  
  box->layout()->addWidget( checkbox = new OptionCheckBox( "check attached file", box, "CHECK_ATTACHMENT" ) );
  AddOptionWidget( checkbox );
  checkbox->setToolTip( "check if attached file exists at start-up" );

  // window sizes
  box = new QVGroupBox( "window size", &addPage( "window size" ) );
  page->layout()->addWidget( box );

  QGridLayout* grid_layout = new QGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  box->setLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "width", box ), 0, 1 );
  grid_layout->addWidget( new QLabel( "height", box ), 0, 2 );

  grid_layout->addWidget( label = new QLabel( "main window: ", box ), 1, 0 );
  label->setToolTip( "main window size (width x height)" );

  grid_layout->addWidget( spinbox = new OptionSpinBox( grid, "SELECTION_FRAME_WIDTH" ), 1, 1 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  AddOptionWidget( spinbox );

  grid_layout->addWidget( spinbox = new OptionSpinBox( grid, "SELECTION_FRAME_HEIGHT" ), 1, 2 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  AddOptionWidget( spinbox );

  grid_layout->addWidget( label = new QLabel( "editor: ", box ), 2, 0 );
  label->setToolTip( "editor window size (width x height)" );
  
  grid_layout->addWidget( spinbox = new OptionSpinBox( grid, "EDIT_FRAME_WIDTH" ), 2, 1 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  AddOptionWidget( spinbox );

  grid_layout->addWidget( spinbox = new OptionSpinBox( grid, "EDIT_FRAME_HEIGHT" ), 2, 2 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  AddOptionWidget( spinbox );

  grid_layout->addWidget( label = new QLabel( "attachment: ", box ), 3, 0 );
  label->setToolTip( "attachment window size (width x height)" );
  
  grid_layout->addWidget( spinbox = new OptionSpinBox( grid, "ATC_FRAME_WIDTH" ), 3, 1 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  AddOptionWidget( spinbox );

  grid_layout->addWidget( spinbox = new OptionSpinBox( grid, "ATC_FRAME_HEIGHT" ), 3, 2 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  AddOptionWidget( spinbox );

  // listview configuration
  ListViewConfig *listview_config = new ListViewConfig( &addPage( "list configuration" ), &static_cast<MainFrame*>(qApp)->selectionFrame().logEntryList() );

//   // toolbars
//   box = new QVGroupBox( "toolbars", &addPage( "toolbars" ) );
// 
//   box = new QGrid( 2, box );
//   new QLabel( "visibility", box );
//   new QLabel( "location", box );
// 
//   AddOptionWidget( new OptionCheckBox( "entry toolbar", grid, "ENTRY_TOOLBAR" ));
//   AddOptionWidget( new CustomToolBar::LocationComboBox( grid, "ENTRY_TOOLBAR_LOCATION" ) );
// 
//   AddOptionWidget( new OptionCheckBox( "edition toolbar", grid, "EDITION_TOOLBAR" ));
//   AddOptionWidget( new CustomToolBar::LocationComboBox( grid, "EDITION_TOOLBAR_LOCATION" ));
// 
//   AddOptionWidget( new OptionCheckBox( "format toolbar", grid, "FORMAT_TOOLBAR" ));
//   AddOptionWidget( new CustomToolBar::LocationComboBox( grid, "FORMAT_TOOLBAR_LOCATION" ));
// 
//   AddOptionWidget( new OptionCheckBox( "extra toolbar (1)", grid, "EXTRA_TOOLBAR_1" ));
//   AddOptionWidget( new CustomToolBar::LocationComboBox( grid, "EXTRA_TOOLBAR_1_LOCATION" ));
// 
//   AddOptionWidget( new OptionCheckBox( "extra toolbar (2)", grid, "EXTRA_TOOLBAR_2" ));
//   AddOptionWidget( new CustomToolBar::LocationComboBox( grid, "EXTRA_TOOLBAR_2_LOCATION" ));

  // colors
  vbox = &addPage( "colors" ); 
  box = new QGroupBox( "logbook entry colors", vbox );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);
  page->layout()->addWidget( box );
 
  OptionListBox* listbox;
  box->layout()->addWidget( lisbox = new OptionListBox( box, "COLOR" ) );
  listbox->setToolTip( "colors used for logbook entry display" );
  AddOptionWidget( listbox );

  box = new QVGroupBox( "text colors", vbox );
  box->layout()->addWidget( listbox = new OptionListBox( box, "TEXT_COLOR" ) );
  listbox->setToolTip( "colors used for text formatting" );
  AddOptionWidget( listbox );

  // auto save
  vbox = &addPage( "backup" );
  box = new QGroupBox( "auto save", vbox );
  page->layout()->addWidget( box );

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing(5);
  box->setLayout( layout );

  layout->addWidget( checkbox = new OptionCheckBox( "auto save", box, "AUTO_SAVE" );
  AddOptionWidget( checkbox );
  checkbox->setToolTip( "automaticaly save logbook at fixed time interval" );

  QGridLayout* grid_layout = new QGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  layout->addLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "auto save interval (seconds): ", box ), 0, 0 );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "AUTO_SAVE_ITV" ), 0, 1 );
  spinbox->setMinimum( 0 );
  spinbox->setMaximum( 3600 );
  spinbox->setToolTip( "time interval between two automatic save" );
  AddOptionWidget( spinbox );

  // auto backup
  box = new QVGroupBox( "auto backup", vbox );
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing(5);
  box->setLayout( layout );
  page->layout()->addWidget( box );

  layout->addWidget( checkbox = new OptionCheckBox( "auto backup", box, "AUTO_BACKUP" ) );
  checkbox->setToolTip( "make a backup of the logbook at fixed time schedule" );
  AddOptionWidget( checkbox );
 
  QGridLayout* grid_layout = new QGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  layout->addLayout( grid_layout );
  
  grid_layout->addWidget( new QLabel( "backup interval (days): ", box ), 0, 0 );
  grid_layout->addWidget( spinbox = new OptionSpinBox( grid, "BACKUP_ITV" ), 0, 1 );
  spinbox->setMinimum( 0 );
  spinbox->setMaximum( 365 );
  spinbox->setToolTip( "time interval between two backups" );
  AddOptionWidget( spinbox );

  // spelling
  #if WITH_ASPELL
  vbox = &addPage( "spell checking", true );
  SpellConfig* spell_config = new SpellConfig( page );
  page->layout()->addWidget( spell_config );
  addOptionWidget( spell_config );
  #endif

  // misc
  vbox = &addPage( "misc", true );
  TabConfig( vbox );
  
  box = new QVGroupBox( "misc", vbox );
 
  checkbox = new OptionCheckBox( "show menu", box, "SHOW_EDITFRAME_MENU" );
  AddOptionWidget( checkbox );
  QToolTip::add( checkbox, "show/hide menu in editor window" );
  
  checkbox = new OptionCheckBox( "show keyword", box, "SHOW_KEYWORD" );
  AddOptionWidget( checkbox );
  QToolTip::add( checkbox, "show/hide keyword line in editor window" );

  checkbox = new OptionCheckBox( "wrap text", box, "WRAP_TEXT" ); 
  AddOptionWidget( checkbox ); 
  QToolTip::add( checkbox, "wrap text in entry edition window" ); 

  checkbox = new OptionCheckBox( "case sensitive", box, "CASE_SENSITIVE" );
  AddOptionWidget( checkbox );
  QToolTip::add( checkbox, "toggle case sensitive text search" );

  checkbox = new OptionCheckBox( "splash screen", box, "SPLASH_SCREEN" );
  AddOptionWidget( checkbox );
  QToolTip::add( checkbox, "show splash screen at startup" );
  
  new QVBox( vbox );
  
  // connect buttons
  connect( &GetApplyButton(), SIGNAL( clicked() ), listview_config, SLOT( Update() ) );
  connect( &GetOkButton(), SIGNAL( clicked() ), listview_config, SLOT( Update() ) );
  connect( &GetCancelButton(), SIGNAL( clicked() ), listview_config, SLOT( Restore() ) );

  // load initial configuration
  _ReadConfig();

}
