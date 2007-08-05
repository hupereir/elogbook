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
  \file ConfigurationDialog.cc
  \brief xMaze configuration dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QLayout>

#include "ConfigurationDialog.h"
#include "CustomGridLayout.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "ListViewConfiguration.h"
#include "MainFrame.h"
#include "OptionBrowsedLineEdit.h"
#include "OptionCheckBox.h"
#include "OptionSpinBox.h"
#include "OptionListBox.h"
#include "Options.h"
#include "SelectionFrame.h"

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
  QLabel* label;

  // attachment editors
  QWidget* page = &addPage( "Attachments" );
  page->layout()->addWidget( box = new QGroupBox( "Editors", page ));

  CustomGridLayout* grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount( 2 );
  box->setLayout( grid_layout );

  OptionBrowsedLineEdit *editor;
  
  grid_layout->addWidget( new QLabel( "Default: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_UNKNOWN_ATC" ) );
  addOptionWidget( editor );
  
  grid_layout->addWidget( new QLabel( "HTML: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_HTML_ATC" ) );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "URL: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_URL_ATC" ) );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "Text: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_PLAIN_TEXT_ATC" ) );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "Postscript: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_POSTSCRIPT_ATC" ) );
  addOptionWidget( editor );

  grid_layout->addWidget( new QLabel( "Image: ", box ) );
  grid_layout->addWidget( editor =  new OptionBrowsedLineEdit( box, "EDIT_IMAGE_ATC" ) );
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

  grid_layout = new CustomGridLayout();
  grid_layout->setSpacing(5);
  grid_layout->setMargin(5);
  grid_layout->setMaxCount(3);
  box->setLayout( grid_layout );

  grid_layout->addWidget( new QLabel( "Width", box ), 0, 1 );
  grid_layout->addWidget( new QLabel( "Height", box ), 0, 2 );

  grid_layout->addWidget( label = new QLabel( "Main window: ", box ) );
  label->setToolTip( "Main window size (width x height)" );

  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "SELECTION_FRAME_WIDTH" ) );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  addOptionWidget( spinbox );

  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "SELECTION_FRAME_HEIGHT" ) );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  addOptionWidget( spinbox );

  grid_layout->addWidget( label = new QLabel( "Editor: ", box ) );
  label->setToolTip( "Editor window size (width x height)" );
  
  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "EDIT_FRAME_WIDTH" ), 2, 1 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  addOptionWidget( spinbox );

  grid_layout->addWidget( spinbox = new OptionSpinBox( box, "EDIT_FRAME_HEIGHT" ), 2, 2 );
  spinbox->setMinimum( 5 );
  spinbox->setMaximum( 2048 );
  addOptionWidget( spinbox );

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

  // listview configuration
  ListViewConfiguration *listview_config = new ListViewConfiguration( 
    &addPage( "List configuration" ), 
    &static_cast<MainFrame*>(qApp)->selectionFrame().logEntryList(), 
    "Main window logbook entries display list" );

  // toolbars
  page = &addPage( "Toolbars" );
  box = new QGroupBox( "Toolbars", page );
  page->layout()->addWidget( box );

  grid_layout = new CustomGridLayout();
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

  grid_layout = new CustomGridLayout();
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
 
  grid_layout = new CustomGridLayout();
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

  // misc
  page = &addPage( "Misc" );
  textEditConfiguration( page );
  
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
  
  // connect buttons
  connect( this, SIGNAL( apply() ), listview_config, SLOT( update() ) );
  connect( this, SIGNAL( ok() ), listview_config, SLOT( update() ) );
  connect( this, SIGNAL( cancel() ), listview_config, SLOT( restore() ) );

  // load initial configuration
  _read();

}
