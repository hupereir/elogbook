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
#include <iostream>
#include <string>

#include "Application.h"
#include "AttachmentWindow.h"
#include "AttachmentType.h"
#include "Config.h"
#include "ConfigurationDialog.h"
#include "Debug.h"
#include "EditionWindow.h"
#include "ErrorHandler.h"
#include "File.h"
#include "FlatStyle.h"
#include "IconEngine.h"
#include "Icons.h"
#include "Logbook.h"
#include "MainWindow.h"
#include "Menu.h"
#include "XmlOptions.h"
#include "RecentFilesMenu.h"
#include "QtUtil.h"
#include "SplashScreen.h"
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
  std::cout << "usage : eLogbook [options] [file]" << std::endl;
  std::cout << std::endl;
  std::cout << "Options : " << std::endl;
  std::cout << "  --help\t\tdisplays this help and exit" << std::endl;
  SERVER::ApplicationManager::usage();
  return;
}

//____________________________________________
Application::Application( int argc, char*argv[] ) :
  QApplication( argc, argv ),
  Counter( "Application" ),
  args_( argc, argv ),
  application_manager_( 0 ),
  recent_files_( 0 ),
  attachment_window_( 0 ),
  main_window_( 0 ),
  realized_( false )
{ 
  Debug::Throw( "Application::Application.\n" ); 
  if( XmlOptions::get().get<bool>( "USE_FLAT_THEME" ) ) setStyle( new FlatStyle() );
} 

//____________________________________________
Application::~Application( void ) 
{
  Debug::Throw( "Application::~Application.\n" );
  
  if( main_window_ ) delete main_window_;
  if( application_manager_ ) delete application_manager_; 
  if( recent_files_ ) delete recent_files_;
  
  XmlOptions::write();
  
  // error handler
  ErrorHandler::exit();

} 

//____________________________________________
void Application::initApplicationManager(  void )
{
  Debug::Throw( "Application::InitApplicationManager. Done.\n" ); 

  // disable server mode from option
  if( args_.find( "--no-server" ) ) 
  {
    realizeWidget();
    return;
  }
  
  if( application_manager_ ) return;

  // create application manager
  application_manager_ = new SERVER::ApplicationManager( this );
  application_manager_->setApplicationName( "ELOGBOOK" );
  connect( 
    application_manager_, SIGNAL( stateChanged( SERVER::ApplicationManager::State ) ),
    SLOT( _applicationManagerStateChanged( SERVER::ApplicationManager::State ) ) );
    
  connect( application_manager_, SIGNAL( serverRequest( const ArgList& ) ), SLOT( _processRequest( const ArgList& ) ) );
    
  // initialize application manager  
  application_manager_->init( args_ );
}
  
//____________________________________________
void Application::realizeWidget( void )
{
  Debug::Throw( "Application::realizeWidget.\n" );
 
  //! check if the method has already been called.
  if( realized_ ) return;
  realized_ = true;

  // actions
  about_action_ = new QAction( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).c_str() ), "About &eLogbook", this );
  connect( about_action_, SIGNAL( triggered() ), SLOT( _about() ) );

  aboutqt_action_ = new QAction( IconEngine::get( ICONS::ABOUT_QT ), "About &Qt", this );
  connect( aboutqt_action_, SIGNAL( triggered() ), SLOT( aboutQt() ) ); 

  close_action_ = new QAction( IconEngine::get( ICONS::EXIT ), "E&xit", this );
  close_action_->setShortcut( CTRL+Key_Q );
  connect( close_action_, SIGNAL( triggered() ), SLOT( _exit() ) );
  
  configuration_action_ = new QAction( IconEngine::get( ICONS::CONFIGURE ), "Default &Configuration", this );
  connect( configuration_action_, SIGNAL( triggered() ), SLOT( _configuration() ) );  
  
  // recent files
  recent_files_ = new XmlFileList();
  recent_files_->setCheck( true );
  
  // create attachment window
  attachment_window_ = new AttachmentWindow();
  
  // create selection frame
  main_window_ = new MainWindow();

  // update configuration
  _updateConfiguration();

  // splashscreen
  ::SplashScreen *splash_screen = new ::SplashScreen( main_window_ );

  // connections
  connect( main_window_, SIGNAL( messageAvailable( const QString& ) ), splash_screen, SLOT( displayMessage( const QString& ) ) );
  connect( main_window_, SIGNAL( ready() ), splash_screen, SLOT( close() ) );

  QtUtil::centerOnDesktop( splash_screen );
  if( XmlOptions::get().get<bool>("SPLASH_SCREEN") )
  { splash_screen->show(); }
  
  QtUtil::centerOnDesktop( main_window_ );
  mainWindow().show();
    
  // update
  processEvents();
    
  // try open file from argument
  File file( args_.last() );
  if( file.size() ) mainWindow().setLogbook( file );
  else if( !mainWindow().setLogbook( recentFiles().lastValidFile().file() ) )
  { 
    splash_screen->close();
    mainWindow().newLogbookAction().trigger();
  }
  
}

//_______________________________________________
void Application::showSplashScreen( void )
{

  QPixmap pixmap( (File( XmlOptions::get().raw( "ICON_PIXMAP" ) )).expand().c_str() );

  ostringstream what;
  what << "<B>eLogbook</B><BR> version " << VERSION;
  ::SplashScreen *splash_screen = new ::SplashScreen();
  QtUtil::centerOnDesktop( splash_screen );
  splash_screen->show();
  splash_screen->displayMessage( "click on the window to close" );

  processEvents();

}

 
//_______________________________________________
void Application::_about( void )
{

  Debug::Throw( "Application::_about.\n" );
  ostringstream what;
  what << "<b>eLogbook</b> version " << VERSION << " (" << BUILD_TIMESTAMP << ")";
  what 
    << "<p>This application was written for personal use only. "
    << "It is not meant to be bug free, although all efforts "
    << "are made so that it remains/becomes so. "
    
    << "<p>Suggestions, comments and bug reports are welcome. "
    << "Please use the following e-mail address:"

    << "<p><a href=\"mailto:hugo.pereira@free.fr\">hugo.pereira@free.fr</a>";

  QMessageBox dialog;
  dialog.setWindowIcon( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).expand().c_str() ) );
  dialog.setIconPixmap( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).expand().c_str() ) );
  dialog.setText( what.str().c_str() );
  dialog.adjustSize();
  QtUtil::centerOnWidget( &dialog, activeWindow() );
  dialog.exec();

}

//_________________________________________________
void Application::_configuration( void )
{
  
  Debug::Throw( "Application::_configuration" );
  emit saveConfiguration();
  ConfigurationDialog dialog(0);
  connect( &dialog, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  QtUtil::centerOnWidget( &dialog, activeWindow() );
  dialog.exec();

}

//_________________________________________________
void Application::_updateConfiguration( void )
{
  
  Debug::Throw( "Application::_updateConfiguration.\n" );

  // set fonts
  QFont font;
  font.fromString( XmlOptions::get().raw( "FONT_NAME" ).c_str() );
  setFont( font );
  
  font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ).c_str() );
  setFont( font, "QLineEdit" ); 
  setFont( font, "QTextEdit" ); 
    
  // debug
  Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );

  // window icon
  setWindowIcon( QPixmap( File( XmlOptions::get().raw( "ICON_PIXMAP" ) ).expand().c_str() ) );
  
  // reload IconEngine cache (in case of icon_path_list that changed)
  IconEngine::get().reload();

  emit configurationChanged();
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
      if( (!(*iter)->isReadOnly()) && (*iter)->modified() && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) 
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
  
  quit();
  
}
//________________________________________________
void Application::_processRequest( const ArgList& args )
{

  Debug::Throw() << "Application::_processRequest - " << args << endl;
  
  if( main_window_ ) mainWindow().uniconifyAction().trigger();

  // check argument. Last argument, if starting with a "-" is possibly a filename
  string filename( args.last() );
  
  if( !filename.empty() ) 
  {
    ostringstream what;
    what << "Accept request for file \"" << filename << "\" ?";
    if( QtUtil::questionDialog( main_window_, what.str(), QtUtil::CENTER_ON_PARENT ) )
    { mainWindow().setLogbook( recentFiles().add( filename ).file() ); }
    
  }

}

//________________________________________________
void Application::_applicationManagerStateChanged( SERVER::ApplicationManager::State state )
{

  Debug::Throw() << "Application::_applicationManagerStateChanged - state=" << state << endl;

  switch ( state ) {
    case SERVER::ApplicationManager::ALIVE:
    realizeWidget();
    break;
    
    case SERVER::ApplicationManager::DEAD:
    quit();
    break;
    
    default:
    break;
  }
  
  return;
  
}
