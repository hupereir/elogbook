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
  \file ConfigurationDialog.cpp
  \brief configuration dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QLayout>
#include <QGroupBox>

#include "Application.h"
#include "AttachmentWindow.h"
#include "ConfigurationDialog.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "GridLayout.h"
#include "MainWindow.h"
#include "OptionBrowsedLineEditor.h"
#include "OptionCheckBox.h"
#include "OptionSpinBox.h"
#include "OptionListBox.h"
#include "Options.h"
#include "ServerConfiguration.h"
#include "Singleton.h"
#include "SvgConfiguration.h"
#include "TreeViewConfiguration.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellCheckConfiguration.h"
#endif

using namespace std;

//_________________________________________________________
ConfigurationDialog::ConfigurationDialog( QWidget* parent ):
  BaseConfigurationDialog( parent )
{


  Debug::Throw( "ConfigurationDialog::ConfigurationDialog.\n" );

  baseConfiguration();
  
  // generic objects
  QGroupBox *box;
  OptionCheckBox *checkbox;
  OptionSpinBox* spinbox;

  // attachment editors
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog - attachments.\n" );
  QWidget* page = &addPage( "Attachments", "System dependent commands used to edit attachments" );
  page->layout()->addWidget( box = new QGroupBox( "Editors", page ));

  GridLayout* grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount( 2 );
  box->setLayout( grid_layout );

  OptionBrowsedLineEditor *editor;
  
  grid_layout->addWidget( new QLabel( "Default: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_UNKNOWN_ATC" ) );
  addOptionWidget( editor );
  
  grid_layout->addWidget( new QLabel( "HTML: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_HTML_ATC" ) );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "URL: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_URL_ATC" ) );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "Text: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_PLAIN_TEXT_ATC" ) );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "Postscript: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_POSTSCRIPT_ATC" ) );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "Image: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_IMAGE_ATC" ) );
  addOptionWidget( editor );
  
  grid_layout->setColumnStretch( 1, 1 );
  
  grid_layout->addWidget( new QLabel( "Icon size: ", box ) );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "ATTACHMENT_LIST_ICON_SIZE" ) );
  spinbox->setMinimum( 8 );
  spinbox->setMaximum( 96 );
  spinbox->setToolTip( "Icon size in attachment lists" );
  addOptionWidget( spinbox );  

  // listview configuration
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog - lists.\n" );
  page = &addPage( "List configuration", "Visible columns in logbook keywords and entries lists" );  
  TreeViewConfiguration *listview_config = new TreeViewConfiguration( 
    page, 
    &Singleton::get().application<Application>()->mainWindow().logEntryList(), 
    Singleton::get().application<Application>()->mainWindow().logEntryList().maskOptionName() );
  
  listview_config->setTitle( "Logbook entries list" );
  addOptionWidget( listview_config );
  page->layout()->addWidget( listview_config );

  // attachment configuration
  listview_config = new TreeViewConfiguration( 
    page, 
    &Singleton::get().application<Application>()->attachmentWindow().frame().list(), 
    Singleton::get().application<Application>()->attachmentWindow().frame().list().maskOptionName() );
  listview_config->setTitle( "Attachments list" );
  addOptionWidget( listview_config );
  page->layout()->addWidget( listview_config );

  // toolbars
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog - toolbars.\n" );
  page = &addPage( "Toolbars", "Toolbars visibility and location" );
  box = new QGroupBox( "Toolbars", page );
  page->layout()->addWidget( box );

  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "Visibility", box ) );
  grid_layout->addWidget( new QLabel( "Location", box ) );

  OptionComboBox* combobox;
 
  grid_layout->addWidget( checkbox = new OptionCheckBox( "Main toolbar", box, "MAIN_TOOLBAR" )); 
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "MAIN_TOOLBAR_LOCATION" ) ); 
  addOptionWidget( checkbox );
  addOptionWidget( combobox );
  
  grid_layout->addWidget( checkbox = new OptionCheckBox( "Undo history toolbar", box, "EDITION_TOOLBAR" )); 
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EDITION_TOOLBAR_LOCATION" )); 
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Format toolbar", box, "FORMAT_TOOLBAR" )); 
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "FORMAT_TOOLBAR_LOCATION" )); 
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Tools", box, "EXTRA_TOOLBAR" )); 
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EXTRA_TOOLBAR_LOCATION" )); 
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Navigation toolbar", box, "NAVIGATION_TOOLBAR" )); 
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "NAVIGATION_TOOLBAR_LOCATION" )); 
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  grid_layout->addWidget( checkbox = new OptionCheckBox( "Navigation toolbar", box, "MULTIPLE_VIEW_TOOLBAR" )); 
  grid_layout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "MULTIPLE_VIEW_TOOLBAR_LOCATION" )); 
  addOptionWidget( checkbox );
  addOptionWidget( combobox );

  box = new QGroupBox( page );
  page->layout()->addWidget( box );
  
  box->setLayout( new QHBoxLayout() );
  box->layout()->setSpacing( 5 );
  box->layout()->setMargin( 5 );
  
  box->layout()->addWidget( new QLabel( "Lock position: ", box )); 
  box->layout()->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "LOCK_TOOLBAR_LOCATION" ) ); 
  addOptionWidget( combobox );

  // colors
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog - colors.\n" );
  page = &addPage( "Colors", "Color settings for entry tagging and text highlighting" ); 
  box = new QGroupBox( "Logbook entry colors", page );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);
  page->layout()->addWidget( box );
 
  OptionListBox* listbox;
  box->layout()->addWidget( listbox = new OptionListBox( box, "COLOR" ) );
  listbox->setToolTip( "Colors used for logbook entry display" );
  addOptionWidget( listbox );

  box = new QGroupBox( "Text colors", page );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);
  page->layout()->addWidget( box );

  box->layout()->addWidget( listbox = new OptionListBox( box, "TEXT_COLOR" ) );
  listbox->setToolTip( "Colors used for text formatting" );
  addOptionWidget( listbox );

  // auto save
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog - backup.\n" );
  page = &addPage( "Backup", "Logbook backup configuration" );
  box = new QGroupBox( "Auto save", page );
  page->layout()->addWidget( box );

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing(5);
  box->setLayout( layout );

  layout->addWidget( checkbox = new OptionCheckBox( "Auto save", box, "AUTO_SAVE" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Automatically save logbook at fixed time interval" );

  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  layout->addLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "Auto save interval (seconds): ", box ), 0, 0 );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "AUTO_SAVE_ITV" ), 0, 1 );
  spinbox->setMinimum( 0 );
  spinbox->setMaximum( 3600 );
  spinbox->setToolTip( "Time interval between two automatic save" );
  addOptionWidget( spinbox );

  // auto backup
  box = new QGroupBox( "Auto backup", page );
  layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing(5);
  box->setLayout( layout );
  page->layout()->addWidget( box );

  layout->addWidget( checkbox = new OptionCheckBox( "Auto backup", box, "AUTO_BACKUP" ) );
  checkbox->setToolTip( "Make a backup of the logbook at fixed time schedule" );
  addOptionWidget( checkbox );
 
  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  layout->addLayout( grid_layout );
  
  grid_layout->addWidget( new QLabel( "Backup interval (days): ", box ), 0, 0 );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "BACKUP_ITV" ), 0, 1 );
  spinbox->setMinimum( 0 );
  spinbox->setMaximum( 365 );
  spinbox->setToolTip( "Time interval between two backups" );
  addOptionWidget( spinbox );

  // spelling
  #if WITH_ASPELL
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog - spellcheck.\n" );
  page = &addPage( "Spell checking", "Spell checking configuration" );
  SpellCheckConfiguration* spell_config = new SpellCheckConfiguration( page );
  page->layout()->addWidget( spell_config );
  addOptionWidget( spell_config );
  #endif

  // edition
  textEditConfiguration();

  // splash screen
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog - splash screen.\n" );
  page = &addPage( "Splash screen", "Splash screen appearance" );
  
  box = new QGroupBox( "", page );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setSpacing(5);
  box->layout()->setMargin(5);
  page->layout()->addWidget( box );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Enable splash screen", box, "SPLASH_SCREEN" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Show splash screen at startup" );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Enable transparency", box, "TRANSPARENT" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Enable startup splash screen transparency" );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Round corners", box, "ROUND_SPLASH_SCREEN" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Round startup splash screen corners (using mask)" );

  // svg background
  SVG::SvgConfiguration* svg_configuration( new SVG::SvgConfiguration( page ) );
  page->layout()->addWidget( svg_configuration);
  addOptionWidget( svg_configuration );

  // misc
  Debug::Throw( "ConfigurationDialog::ConfigurationDialog - misc.\n" );
  page = &addPage( "Misc", "Additional unsorted settings" );

  // server configuration
  SERVER::ServerConfiguration* server_configuration;
  page->layout()->addWidget( server_configuration = new SERVER::ServerConfiguration( page, "Server configuration" ));
  addOptionWidget( server_configuration );
  
  page->layout()->addWidget( box = new QGroupBox( "Recent files", page ) );  
   
  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(2);
  box->setLayout( grid_layout );

  // previous file history size
  grid_layout->addWidget( new QLabel( "recent files history size", box ) );
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "DB_SIZE" ) );
  spinbox->setMinimum( 0 );
  spinbox->setMaximum( 100 );
  addOptionWidget( spinbox );
  spinbox->setToolTip( "number of previously opened files to appear in the Open Previous menu" );

  // sort previous files by date
  grid_layout->addWidget( checkbox = new OptionCheckBox( "sort recent files by date", box, "SORT_FILES_BY_DATE" ), 3, 0, 1, 2 );
  checkbox->setToolTip( "Sort files by date rather than name in Open Previous menu." );
  addOptionWidget( checkbox );
  new QWidget( box );
  
  box = new QGroupBox( "Misc", page );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setSpacing(5);
  box->layout()->setMargin(5);
  page->layout()->addWidget( box );
  
  box->layout()->addWidget( checkbox = new OptionCheckBox( "Show menu", box, "SHOW_EDITFRAME_MENU" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Show/hide menu in editor window" );
  
  box->layout()->addWidget( checkbox = new OptionCheckBox( "Case sensitive", box, "CASE_SENSITIVE" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Toggle case sensitive text search" );

  // load initial configuration
  _read();
  
}
