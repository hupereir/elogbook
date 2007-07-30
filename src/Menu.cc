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
  \file Menu.cc
  \brief  main menu
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qmessagebox.h>
#include <sstream>

#include "AttachmentFrame.h"
#include "Config.h"
#include "ConfigDialog.h"
#include "DebugMenu.h"
#include "EditFrame.h"
#include "Exception.h"
#include "File.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "Logbook.h"
#include "LogEntryList.h"
#include "MainFrame.h"
#include "Menu.h"
#include "OpenPreviousMenu.h"
#include "QtUtil.h"
#include "SelectionFrame.h"
#include "Str.h"
#include "Util.h"
#include "XmlOptions.h"

using namespace std;
using namespace BASE;
using namespace Qt;
using namespace HELP;

//_______________________________________________
Menu::Menu( QWidget* parent, SelectionFrame* selection_frame ):
  QMenuBar( parent ),
  Counter( "Menu" )
{
  
  Debug::Throw( "Menu::Menu.\n" );

  // generic menu
  QMenu *menu;

  // file menu
  menu = addMenu( "&File" );
  menu->addAction( "&New",  selection_frame, SLOT( newLogbook() ), CTRL+Key_N );
  menu->addAction( "&Open", selection_frame, SLOT( open() ), CTRL+Key_O );

  // file menu
  open_previous_menu_ = new OpenPreviousMenu( this );
  open_previous_menu_->setTitle( "Open pre&vious" );
  open_previous_menu_->setCheck( true );
  connect( open_previous_menu_, SIGNAL( fileSelected( FileRecord ) ), selection_frame, SLOT( open( FileRecord ) ) );  
  menu->addMenu( open_previous_menu_ );

  menu->addAction( "&Synchronize", selection_frame, SLOT( synchronize() ) )->setToolTip( "synchronize current logbook with remove logbook" );
  menu->addAction( "&Reorganize", selection_frame, SLOT( reorganize() ) )->setToolTip( "reorganize entries in sublogbook to minimize number of files" );

  menu->addSeparator();
  menu->addAction( "&Save", this, SIGNAL( save() ), CTRL+Key_S );
  menu->addAction( "Save &As", selection_frame, SLOT( saveAs() ) );
  menu->addAction( "Save &Backup", selection_frame, SLOT( saveBackup() ) );
  menu->addAction( "&Revert to Saved", selection_frame, SLOT( revertToSaved() ) );
  menu->addSeparator();
  menu->addAction( "&View HTML", this, SIGNAL( viewHtml() ), 0 );
  menu->addSeparator();
  menu->addAction( "&Close", this, SIGNAL( closeWindow() ), CTRL+Key_W );
  menu->addAction( "E&xit", qApp, SLOT( exit() ), CTRL+Key_Q );

  // preferences menu
  menu = addMenu( "&Preferences" );
  menu->addAction( "&Configuration", qApp, SLOT( configuration() ) );

  // windows menu
  editor_menu_ = addMenu( "&Windows" );
  connect( editor_menu_, SIGNAL( aboutToShow() ), SLOT( _updateEditorMenu() ) );

  // help menu
  menu = addMenu( "&Help" );
  menu->addAction( "&Reference Manuel", &HelpManager::get(), SLOT( display() ) );
  menu->addSeparator();
  menu->addAction( "About &Qt", qApp, SLOT( aboutQt() ), 0 );
  menu->addAction( "About &eLogbook", qApp, SLOT( about() ), 0 );
  menu->addSeparator();

  // install help
  File help_file( XmlOptions::get().get<File>( "HELP_FILE" ) );
  if( help_file.exist() ) HelpManager::get().install( help_file );
  else
  {
    HelpManager::get().setFile( help_file );
    HelpManager::get().install( HelpText );
  }
  
  // debug menu
  DebugMenu *debug_menu( new DebugMenu( this ) );
  debug_menu->setTitle( "&Debug" );
  menu->addMenu( debug_menu );
  debug_menu->addAction( "&Show duplicates", selection_frame, SLOT( showDuplicatedEntries() ) );
  debug_menu->addAction( "&Show splash screen", qApp, SLOT( showSplashScreen() ) );

}

//_______________________________________________
void Menu::_updateEditorMenu( void )
{
  Debug::Throw( "Menu::_UpdateEditorMenu.\n" );
  editor_menu_->clear();

  SelectionFrame *selection_frame( &static_cast<MainFrame*>(qApp)->selectionFrame() );
  AttachmentFrame *attachment_frame( &static_cast<MainFrame*>(qApp)->attachmentFrame() );
  
  // editor attachments and logbook information
  editor_menu_->addAction( "&Attachments", attachment_frame, SLOT( uniconify() ), CTRL+Key_T );
  editor_menu_->addAction( "&Logbook informations", selection_frame, SLOT( editLogbookInformations() ) );
  editor_menu_->addAction( "&Logbook statistics", selection_frame, SLOT( viewLogbookStatistics() ) );

  bool found ( false );
  KeySet<EditFrame> frames( selection_frame );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  {
    // hidden EditFrames will be deleted. Do not allow for opening
    if( (*iter)->isHidden() ) continue;
    if( !found ) editor_menu_->addSeparator();
    found = true;
    
    string title( (*iter)->windowTitle() );
    editor_menu_->addAction( title.c_str(), *iter, SLOT( uniconify() ) );

  }
  
  if( found )
  {
    editor_menu_->addSeparator();
    editor_menu_->addAction( "&Close editors", &static_cast<MainFrame*>(qApp)->selectionFrame(), SLOT( closeEditFrames() ) );
  }
  
  return;
}
