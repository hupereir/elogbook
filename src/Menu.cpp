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
  \file Menu.cpp
  \brief  main menu
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qmessagebox.h>
#include <sstream>

#include "AttachmentFrame.h"
#include "CustomToolBar.h"
#include "DebugMenu.h"
#include "EditFrame.h"
#include "File.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "IconEngine.h"
#include "Icons.h"
#include "Logbook.h"
#include "MainFrame.h"
#include "Menu.h"
#include "OpenPreviousMenu.h"
#include "QtUtil.h"
#include "SelectionFrame.h"
#include "SearchPanel.h"
#include "Str.h"
#include "Util.h"
#include "XmlOptions.h"

using namespace std;
using namespace Qt;

//_______________________________________________
Menu::Menu( QWidget* parent, SelectionFrame* selectionframe ):
  QMenuBar( parent ),
  Counter( "Menu" )
{
  
  Debug::Throw( "Menu::Menu.\n" );
  
  // try cast parent to EditFrame
  EditFrame* editframe( dynamic_cast<EditFrame*>( parent ) );
  
  // generic menu/action
  QMenu *menu;
  
  // file menu
  menu = addMenu( "&File" );
  menu->addAction( &selectionframe->newLogbookAction() );
  
  if( editframe )
  {
    menu->addAction( &editframe->newEntryAction() );
    menu->addAction( &editframe->splitViewHorizontalAction() );
  }
  
  menu->addAction( &selectionframe->openAction() );

  // file menu
  open_previous_menu_ = new OpenPreviousMenu( this );
  open_previous_menu_->setCheck( true );
  connect( open_previous_menu_, SIGNAL( fileSelected( FileRecord ) ), selectionframe, SLOT( open( FileRecord ) ) );  
  menu->addMenu( open_previous_menu_ );

  menu->addAction( &selectionframe->synchronizeAction() );
  menu->addAction( &selectionframe->reorganizeAction() );

  menu->addSeparator();
  if( editframe ) menu->addAction( &editframe->saveAction() );
  else menu->addAction( &selectionframe->saveAction() );
  
  menu->addAction( &selectionframe->saveAsAction() );
  menu->addAction( &selectionframe->saveBackupAction() );
  menu->addAction( &selectionframe->revertToSaveAction() );
  menu->addSeparator();
  
  if( editframe ) menu->addAction( &editframe->viewHtmlAction() );
  else menu->addAction( &selectionframe->viewHtmlAction() );
  
  if( editframe ) menu->addAction( &editframe->closeAction() );
 
  MainFrame& mainframe( *static_cast<MainFrame*>(qApp) );
  menu->addAction( &mainframe.closeAction() );

  // edition menu
  if( parent == selectionframe )
  {
    menu = addMenu( "&Edit" );
    menu->addAction( &selectionframe->newKeywordAction() );
    menu->addAction( &selectionframe->editKeywordAction() );
    menu->addAction( &selectionframe->deleteKeywordAction() );
    menu->addSeparator();
    menu->addAction( &selectionframe->newEntryAction() );
    menu->addAction( &selectionframe->editEntryAction() );
    menu->addAction( &selectionframe->deleteEntryAction() );
  }
  
  // preferences
  preference_menu_ = addMenu( "&Preferences" );
  connect( preference_menu_, SIGNAL( aboutToShow() ), this, SLOT( _updatePreferenceMenu() ) );
  
  // windows menu
  editor_menu_ = addMenu( "&Windows" );
  connect( editor_menu_, SIGNAL( aboutToShow() ), SLOT( _updateEditorMenu() ) );

  // help manager
  BASE::HelpManager* help( new BASE::HelpManager( this ) );
  File help_file( XmlOptions::get().raw( "HELP_FILE" ) );
  if( help_file.exists() )   BASE::HelpManager::install( help_file );
  else {
    BASE::HelpManager::setFile( help_file );
    BASE::HelpManager::install( HelpText );
  }  
  
  // help menu
  menu = addMenu( "&Help" );
  menu->addAction( &help->displayAction() );
  menu->addSeparator();
  menu->addAction( &mainframe.aboutAction() );
  menu->addAction( &mainframe.aboutQtAction() );
  menu->addSeparator();
  
  // debug menu
  DebugMenu *debug_menu( new DebugMenu( this ) );
  debug_menu->setTitle( "&Debug" );
  menu->addMenu( debug_menu );
  debug_menu->addAction( &selectionframe->saveForcedAction() );
  debug_menu->addAction( &selectionframe->showDuplicatesAction() );
  debug_menu->addAction( "&Show splash screen", qApp, SLOT( showSplashScreen() ) );
  debug_menu->addAction( &help->dumpAction() );

}

//_______________________________________________
void Menu::_updateEditorMenu( void )
{
  Debug::Throw( "Menu::_UpdateEditorMenu.\n" );
  
  editor_menu_->clear();

  SelectionFrame &selectionframe( static_cast<MainFrame*>(qApp)->selectionFrame() );
  AttachmentFrame &attachment_frame( static_cast<MainFrame*>(qApp)->attachmentFrame() );
  
  // editor attachments and logbook information
  editor_menu_->addAction( &selectionframe.uniconifyAction() );

  BASE::KeySet<EditFrame> frames( selectionframe );
  bool has_alive_frame( find_if( frames.begin(), frames.end(), EditFrame::aliveFTor() ) != frames.end() );
  if( has_alive_frame )
  {
    
    for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    {
      
      // ignore if frame is to be deleted
      if( (*iter)->isClosed() ) continue;

      // add menu entry for this frame
      string title( (*iter)->windowTitle() );
      editor_menu_->addAction( IconEngine::get( ICONS::EDIT ), title.c_str(), &(*iter)->uniconifyAction(), SLOT( trigger() ) );
      
    }
  }
  
  editor_menu_->addAction( &attachment_frame.uniconifyAction() );
  editor_menu_->addAction( &selectionframe.logbookStatisticsAction() );
  editor_menu_->addAction( &selectionframe.logbookInformationsAction() );

  if( has_alive_frame )
  {
    editor_menu_->addSeparator();
    editor_menu_->addAction( &selectionframe.closeFramesAction() );
  }
  
  return;
}


//_______________________________________________
void Menu::_updatePreferenceMenu( void )
{
  
  Debug::Throw( "Menu::_updatePreferenceMenu.\n" );
  
  // preferences menu
  MainFrame& mainframe( *static_cast<MainFrame*>(qApp) );
  preference_menu_->addAction( &mainframe.configurationAction() );
  
  // additional preferences in case parent is a selection frame
  SelectionFrame *selectionframe = dynamic_cast<SelectionFrame*>( parentWidget() );
  if( selectionframe )
  {
    preference_menu_->addSeparator();
    preference_menu_->addAction( &selectionframe->keywordToolBar().visibilityAction() );
    preference_menu_->addAction( &selectionframe->entryToolBar().visibilityAction() );
    preference_menu_->addAction( &selectionframe->searchPanel().visibilityAction() );
  }
  
  // additional preferences in case parent is an edition frame
  EditFrame *editframe = dynamic_cast<EditFrame*>( parentWidget() );
  if( editframe )
  {
    preference_menu_->addSeparator();
    preference_menu_->addAction( &editframe->activeEditor().showLineNumberAction() );
    preference_menu_->addAction( &editframe->activeEditor().wrapModeAction() );
    preference_menu_->addAction( &editframe->activeEditor().blockHighlightAction() );
  }
  
}
