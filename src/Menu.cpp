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

#include "Application.h"
#include "AttachmentWindow.h"
#include "CustomToolBar.h"
#include "DebugMenu.h"
#include "EditionWindow.h"
#include "File.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "IconEngine.h"
#include "Icons.h"
#include "Logbook.h"
#include "MainWindow.h"
#include "Menu.h"
#include "RecentFilesMenu.h"
#include "QtUtil.h"
#include "SearchPanel.h"
#include "Str.h"
#include "Util.h"
#include "XmlOptions.h"

using namespace std;
using namespace Qt;

//_______________________________________________
Menu::Menu( QWidget* parent, MainWindow* mainwindow ):
  QMenuBar( parent ),
  Counter( "Menu" )
{
  
  Debug::Throw( "Menu::Menu.\n" );
  
  // try cast parent to EditionWindow
  EditionWindow* editionwindow( dynamic_cast<EditionWindow*>( parent ) );
  
  // generic menu/action
  QMenu *menu;
  
  // file menu
  menu = addMenu( "&File" );
  menu->addAction( &mainwindow->newLogbookAction() );
  
  if( editionwindow )
  {
    menu->addAction( &editionwindow->newEntryAction() );
    menu->addAction( &editionwindow->splitViewHorizontalAction() );
  }
  
  menu->addAction( &mainwindow->openAction() );

  // file menu
  RecentFilesMenu* recent_files_menu( new RecentFilesMenu( this, static_cast<Application*>(qApp)->recentFiles() ) );
  connect( recent_files_menu, SIGNAL( fileSelected( FileRecord ) ), mainwindow, SLOT( open( FileRecord ) ) );  
  menu->addMenu( recent_files_menu );

  menu->addAction( &mainwindow->synchronizeAction() );
  menu->addAction( &mainwindow->reorganizeAction() );

  menu->addSeparator();
  if( editionwindow ) menu->addAction( &editionwindow->saveAction() );
  else menu->addAction( &mainwindow->saveAction() );
  
  menu->addAction( &mainwindow->saveAsAction() );
  menu->addAction( &mainwindow->saveBackupAction() );
  menu->addAction( &mainwindow->revertToSaveAction() );
  menu->addSeparator();
  
  if( editionwindow ) menu->addAction( &editionwindow->viewHtmlAction() );
  else menu->addAction( &mainwindow->viewHtmlAction() );
  
  if( editionwindow ) menu->addAction( &editionwindow->closeAction() );
 
  Application& application( *static_cast<Application*>(qApp) );
  menu->addAction( &application.closeAction() );

  // edition menu
  if( parent == mainwindow )
  {
    menu = addMenu( "&Edit" );
    menu->addAction( &mainwindow->newKeywordAction() );
    menu->addAction( &mainwindow->editKeywordAction() );
    menu->addAction( &mainwindow->deleteKeywordAction() );
    menu->addSeparator();
    menu->addAction( &mainwindow->newEntryAction() );
    menu->addAction( &mainwindow->editEntryAction() );
    menu->addAction( &mainwindow->deleteEntryAction() );
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
  menu->addAction( &application.aboutAction() );
  menu->addAction( &application.aboutQtAction() );
  menu->addSeparator();
  
  // debug menu
  DebugMenu *debug_menu( new DebugMenu( this ) );
  debug_menu->setTitle( "&Debug" );
  menu->addMenu( debug_menu );
  debug_menu->addAction( &mainwindow->saveForcedAction() );
  debug_menu->addAction( &mainwindow->showDuplicatesAction() );
  debug_menu->addAction( "&Show splash screen", qApp, SLOT( showSplashScreen() ) );
  debug_menu->addAction( &help->dumpAction() );

}

//_______________________________________________
void Menu::_updateEditorMenu( void )
{
  Debug::Throw( "Menu::_UpdateEditorMenu.\n" );
  
  editor_menu_->clear();

  MainWindow &mainwindow( static_cast<Application*>(qApp)->mainWindow() );
  AttachmentWindow &attachment_frame( static_cast<Application*>(qApp)->attachmentFrame() );
  
  // retrieve parent editFream if any
  EditionWindow* editionwindow = dynamic_cast<EditionWindow*>( parentWidget() );
  
  // editor attachments and logbook information
  editor_menu_->addAction( &mainwindow.uniconifyAction() );
  
  editor_menu_->addAction( &attachment_frame.uniconifyAction() );
  editor_menu_->addAction( &mainwindow.logbookStatisticsAction() );
  editor_menu_->addAction( &mainwindow.logbookInformationsAction() );

  BASE::KeySet<EditionWindow> frames( mainwindow );
  bool has_alive_frame( find_if( frames.begin(), frames.end(), EditionWindow::aliveFTor() ) != frames.end() );
  if( has_alive_frame )
  {
    
    editor_menu_->addSeparator();
    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    {
      
      // ignore if frame is to be deleted
      if( (*iter)->isClosed() ) continue;

      // add menu entry for this frame
      string title( (*iter)->windowTitle() );
      QAction* action = editor_menu_->addAction( IconEngine::get( ICONS::EDIT ), title.c_str(), &(*iter)->uniconifyAction(), SLOT( trigger() ) );
      action->setCheckable( true );
      action->setChecked( editionwindow && ( editionwindow == (*iter) ) );
      
    }

    editor_menu_->addAction( &mainwindow.closeFramesAction() );
  
  }
  
  return;
}


//_______________________________________________
void Menu::_updatePreferenceMenu( void )
{
  
  Debug::Throw( "Menu::_updatePreferenceMenu.\n" );
  
  // preferences menu
  Application& application( *static_cast<Application*>(qApp) );
  preference_menu_->addAction( &application.configurationAction() );
  
  // additional preferences in case parent is a selection frame
  MainWindow *mainwindow = dynamic_cast<MainWindow*>( parentWidget() );
  if( mainwindow )
  {
    preference_menu_->addSeparator();
    preference_menu_->addAction( &mainwindow->keywordToolBar().visibilityAction() );
    preference_menu_->addAction( &mainwindow->entryToolBar().visibilityAction() );
    preference_menu_->addAction( &mainwindow->searchPanel().visibilityAction() );
  }
  
  // additional preferences in case parent is an edition frame
  EditionWindow *editionwindow = dynamic_cast<EditionWindow*>( parentWidget() );
  if( editionwindow )
  {
    preference_menu_->addSeparator();
    preference_menu_->addAction( &editionwindow->activeEditor().showLineNumberAction() );
    preference_menu_->addAction( &editionwindow->activeEditor().wrapModeAction() );
    preference_menu_->addAction( &editionwindow->activeEditor().blockHighlightAction() );
  }
  
}
