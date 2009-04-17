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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License        
* for more details.                    
*                         
* You should have received a copy of the GNU General Public License along with 
* software; if not, write to the Free Software Foundation, Inc., 59 Temple    
* Place, Suite 330, Boston, MA  02111-1307 USA                          
*                        
*                        
*******************************************************************************/

/*!
  \file Application.cpp
  \brief application Main Window singleton object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QMessageBox>



#include "Application.h"
#include "AttachmentWindow.h"
#include "AttachmentType.h"
#include "Config.h"
#include "ConfigurationDialog.h"
#include "Debug.h"
#include "EditionWindow.h"
#include "File.h"
#include "IconEngine.h"
#include "Icons.h"
#include "Logbook.h"
#include "MainWindow.h"
#include "Menu.h"
#include "XmlOptions.h"
#include "RecentFilesMenu.h"
#include "QtUtil.h"
#include "QuestionDialog.h"
#include "XmlFileList.h"

using namespace std;
using namespace Qt;

//____________________________________________
const QString Application::MAIN_TITLE_MODIFIED = "elogbook (modified)";
const QString Application::MAIN_TITLE = "elogbook";
const QString Application::ATTACHMENT_TITLE = "elogbook - attachments";

//____________________________________________
void Application::usage( void )
{
  Debug::Throw(0) << "usage : elogbook [options] [file]" << endl;
  SERVER::ApplicationManager::commandLineParser().usage();
  return;
}

//____________________________________________
Application::Application( CommandLineArguments arguments ) :
  BaseApplication( 0, arguments ),
  Counter( "Application" ),
  recent_files_( 0 ),
  attachment_window_( 0 ),
  main_window_( 0 )
{} 

//____________________________________________
Application::~Application( void ) 
{
  Debug::Throw( "Application::~Application.\n" );  
  if( main_window_ ) delete main_window_;
  if( recent_files_ ) delete recent_files_;

} 

//____________________________________________
void Application::initApplicationManager( void )
{
  Debug::Throw( "Application::initApplicationManager.\n" );

  // retrieve files from arguments and expand if needed
  CommandLineParser parser( SERVER::ApplicationManager::commandLineParser( _arguments() ) );
  QStringList& orphans( parser.orphans() );
  for( QStringList::iterator iter = orphans.begin(); iter != orphans.end(); iter++ )
  { if( !iter->isEmpty() ) (*iter) = File( *iter ).expand(); }

  // replace arguments
  _setArguments( parser.arguments() );

  // base class initialization
  BaseApplication::initApplicationManager();

}

//____________________________________________
bool Application::realizeWidget( void )
{
  Debug::Throw( "Application::realizeWidget.\n" );
 
  // check if the method has already been called.
  if( !BaseApplication::realizeWidget() ) return false;

  // rename about action
  aboutAction().setText( "About &elogbook" );

  // need to redirect closeAction to proper exit
  closeAction().disconnect();
  connect( &closeAction(), SIGNAL( triggered() ), SLOT( _exit() ) );
    
  // recent files
  recent_files_ = new XmlFileList();
  recent_files_->setCheck( true );
  
  // create attachment window
  attachment_window_ = new AttachmentWindow();
  attachmentWindow().centerOnDesktop();
  
  // create selection frame
  main_window_ = new MainWindow();

  connect( &attachmentWindow(), SIGNAL( entrySelected( LogEntry* ) ), &mainWindow(), SLOT( selectEntry( LogEntry* ) ) );
  
  // update configuration
  emit configurationChanged();
  
  mainWindow().centerOnDesktop();
  mainWindow().show();
    
  // update
  qApp->processEvents();
  
  // load file from arguments or recent files
  QStringList filenames( SERVER::ApplicationManager::commandLineParser( _arguments() ).orphans() );
  if( !filenames.isEmpty() ) mainWindow().setLogbook( File( filenames.front() ).expand() ); 
  else if( !mainWindow().setLogbook( recentFiles().lastValidFile().file() ) ) 
  {
    Debug::Throw() << "Application::realizeWidget - openning last valid logbook failed." << endl;
    mainWindow().newLogbookAction().trigger();
  }
  
  return true;
  
}

//_________________________________________________
void Application::_configuration( void )
{
  
  Debug::Throw( "Application::_configuration" );
  emit saveConfiguration();
  ConfigurationDialog dialog;
  connect( &dialog, SIGNAL( configurationChanged() ), SIGNAL( configurationChanged() ) );
  dialog.centerOnWidget( qApp->activeWindow() );
  dialog.exec();

}

//_________________________________________________
void Application::_exit( void )
{
  
  Debug::Throw( "Application::_exit.\n" );
      
  // ensure everything is saved properly
  if( main_window_ )
  {
    // check if editable EditionWindows needs save 
    BASE::KeySet<EditionWindow> frames( main_window_ );
    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    {
      if( !( (*iter)->isReadOnly() || (*iter)->isClosed() ) && (*iter)->modified() && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) 
      { return; }
    }
    
    Debug::Throw( "EditionWindows saved.\n" );
    
    // check if current logbook is modified
    if( 
      mainWindow().logbook() &&
      mainWindow().logbook()->modified() &&
      mainWindow().askForSave() == AskForSaveDialog::CANCEL ) 
    return;
  }
  
  qApp->quit();
  
}

//________________________________________________
void Application::_processRequest( const CommandLineArguments& arguments )
{

  Debug::Throw() << "Application::_processRequest - arguments = " << arguments.join( " " ) << endl;
  if( main_window_ ) mainWindow().uniconifyAction().trigger();

  QStringList filenames( SERVER::ApplicationManager::commandLineParser( arguments ).orphans() );
  if( filenames.isEmpty() ) return;

  QString buffer;
  QTextStream( &buffer ) << "Accept request for file \"" << filenames.front() << "\" ?";
  if( QuestionDialog( main_window_, buffer ).centerOnParent().exec() )
  { mainWindow().setLogbook( File( filenames.front() ) ); }
  
}
