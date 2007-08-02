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
#include "CustomPixmap.h"
#include "DebugMenu.h"
#include "EditFrame.h"
#include "Exception.h"
#include "File.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "IconEngine.h"
#include "Icons.h"
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
  
  // path list for menu
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  // generic menu/action
  QMenu *menu;
  QAction *action;
  
  // file menu
  menu = addMenu( "&File" );
  menu->addAction( selection_frame->newLogbookAction() );
  menu->addAction( selection_frame->openAction() );

  // file menu
  open_previous_menu_ = new OpenPreviousMenu( this );
  open_previous_menu_->setTitle( "Open pre&vious" );
  open_previous_menu_->setCheck( true );
  open_previous_menu_->updateConfiguration();
  connect( open_previous_menu_, SIGNAL( fileSelected( FileRecord ) ), selection_frame, SLOT( open( FileRecord ) ) );  
  menu->addMenu( open_previous_menu_ );

  menu->addAction( selection_frame->synchronizeAction() );
  menu->addAction( selection_frame->reorganizeAction() );

  menu->addSeparator();
  action = menu->addAction( "&Save", this, SIGNAL( save() ), CTRL+Key_S );
  action->setIcon( IconEngine::get( ICONS::SAVE, path_list ) );
  
  menu->addAction( selection_frame->saveAsAction() );
  menu->addAction( selection_frame->saveBackupAction() );
  menu->addAction( selection_frame->revertToSaveAction() );
  menu->addSeparator();
  
  action = menu->addAction( "&View HTML", this, SIGNAL( viewHtml() ) );
  action->setIcon( IconEngine::get( ICONS::HTML, path_list ) );
  menu->addSeparator();
  
  menu->addAction( "&Close", this, SIGNAL( closeWindow() ), CTRL+Key_W );
  action = menu->addAction( "E&xit", qApp, SLOT( exit() ), CTRL+Key_Q );
  action->setIcon( IconEngine::get( ICONS::EXIT, path_list ) );

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
  debug_menu->addAction( selection_frame->saveForcedAction() );
  debug_menu->addAction( selection_frame->showDuplicatesAction() );
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
  editor_menu_->addAction( selection_frame->uniconifyAction() );

  KeySet<EditFrame> frames( selection_frame );
  bool has_edit_frames = find_if( frames.begin(), frames.end(), EditFrame::IsVisibleFTor() ) != frames.end();
  if( has_edit_frames )
  {
    QMenu *menu = editor_menu_->addMenu( "&Editors" );
    for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    {
      // hidden EditFrames will be deleted. Do not allow for opening
      if( (*iter)->isHidden() ) continue;
      
      string title( (*iter)->windowTitle() );
      menu->addAction( title.c_str(), *iter, SLOT( uniconify() ) );
    }
    menu->addSeparator();
    menu->addAction( selection_frame->closeFramesAction() );
  }
  
  editor_menu_->addAction( attachment_frame->uniconifyAction() );
  editor_menu_->addAction( selection_frame->logbookStatisticsAction() );
  editor_menu_->addAction( selection_frame->logbookInformationsAction() );
  
  return;
}
