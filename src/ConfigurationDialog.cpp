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

#include "ConfigurationDialog.h"
#include "GridLayout.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "TreeViewConfiguration.h"
#include "Application.h"
#include "OptionBrowsedLineEditor.h"
#include "OptionCheckBox.h"
#include "OptionSpinBox.h"
#include "OptionListBox.h"
#include "Options.h"
#include "MainWindow.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellCheckConfiguration.h"
#endif

// forward declaration
void installDefaultOptions( void );
void installSystemOptions( void );

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
  QLabel* label;

  // attachment editors
  QWidget* page = &addPage( "Attachments" );
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
  
  box = new QGroupBox( "Misc", page );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);
  page->layout()->addWidget( box );
  
  box->layout()->addWidget( checkbox = new OptionCheckBox( "Check attached file", box, "CHECK_ATTACHMENT" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Check if attached file exists at start-up" );

  // window sizes
  page = &addPage( "Window sizes" );
  box = new QGroupBox( "Window sizes", page );
  page->layout()->addWidget( box );

  grid_layout = new GridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(3);
  box->setLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "Width", box ), 0, 1 );
  grid_layout->addWidget( new QLabel( "Height", box ), 0, 2 );

  grid_layout->addWidget( label = new QLabel( "Attachment: ", box ), 3, 0 );
  label->setToolTip( "Attachment window size (width x height)" );
  
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "ATC_FRAME_WIDTH" ), 3, 1 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  addOptionWidget( spinbox );

  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "ATC_FRAME_HEIGHT" ), 3, 2 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  addOptionWidget( spinbox );

  box = new QGroupBox( "Main window layout", page );
  box->setLayout( new QVBoxLayout() );
  box->layout()->setMargin(5);
  box->layout()->setSpacing(5);
  page->layout()->addWidget( box );
  
  box->layout()->addWidget( checkbox = new OptionCheckBox( "Show keywords toolbar", box, "KEYWORD_TOOLBAR" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "show keywords toolbar in main window" );
  
  box->layout()->addWidget( checkbox = new OptionCheckBox( "Show entries toolbar", box, "ENTRY_TOOLBAR" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "show entries toolbar in main window" );
  
  box->layout()->addWidget( checkbox = new OptionCheckBox( "Show search panel", box, "SHOW_SEARCHPANEL" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "show search panel in main window" );

  // listview configuration
  page = &addPage( "List configuration" );
  TreeViewConfiguration *listview_config = new TreeViewConfiguration( 
    page, 
    &static_cast<Application*>(qApp)->mainWindow().logEntryList(), 
    "Logbook entries display list" );
  
  page->layout()->addWidget( listview_config );
  
  // toolbars
  page = &addPage( "Toolbars" );
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
  page = &addPage( "Colors" ); 
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
  page = &addPage( "Backup" );
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
  page = &addPage( "Spell checking" );
  SpellCheckConfiguration* spell_config = new SpellCheckConfiguration( page );
  page->layout()->addWidget( spell_config );
  addOptionWidget( spell_config );
  #endif

  // edition
  textEditConfiguration();

  // misc
  page = &addPage( "Misc" );
  
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

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Splash screen", box, "SPLASH_SCREEN" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Show splash screen at startup" );

  box->layout()->addWidget( checkbox = new OptionCheckBox( "Transparent splash screen", box, "TRANSPARENT_SPLASH_SCREEN" ) );
  addOptionWidget( checkbox );
  checkbox->setToolTip( "Show transparent splash screen at startup" );
  
  // misc
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
      
  // connect buttons
  connect( this, SIGNAL( apply() ), listview_config, SLOT( update() ) );
  connect( this, SIGNAL( ok() ), listview_config, SLOT( update() ) );
  connect( this, SIGNAL( cancel() ), listview_config, SLOT( restore() ) );
  
  // load initial configuration
  _read();
  adjustSize();
  
}

//________________________________________________________________________________
void ConfigurationDialog::_restoreDefaults( void )
{
  
  Debug::Throw( "ConfigurationDialog::restoreDefaults.\n" );
  
  // reset options
  XmlOptions::get() = Options();
  
  // reinstall default options
  installDefaultOptions();
  installSystemOptions();
  
  // read everything in dialog
  _read();
  
}
