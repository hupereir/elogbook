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
  \file MainFrame.cc
  \brief application Main Window singleton object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <iostream>
#include <string>

#include "AttachmentFrame.h"
#include "AttachmentType.h"
#include "Config.h"
#include "ConfigurationDialog.h"
#include "Debug.h"
#include "EditFrame.h"
#include "ErrorHandler.h"
#include "File.h"
#include "FlatStyle.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "MainFrame.h"
#include "Menu.h"
#include "XmlOptions.h"
#include "OpenPreviousMenu.h"
#include "QtUtil.h"
#include "SelectionFrame.h"
#include "SplashScreen.h"

using namespace std;
using namespace BASE;
using namespace SERVER;

//____________________________________________
const QString MainFrame::MAIN_TITLE_MODIFIED = "elogbook (modified)";
const QString MainFrame::MAIN_TITLE = "elogbook";
const QString MainFrame::ATTACHMENT_TITLE = "elogbook - attachments";

//____________________________________________
void MainFrame::usage( void )
{
  std::cout << "usage : eLogbook [options] [file]" << std::endl;
  std::cout << std::endl;
  std::cout << "Options : " << std::endl;
  std::cout << "  --help\t\tdisplays this help and exit" << std::endl;
  ApplicationManager::usage();
  return;
}

//____________________________________________
MainFrame::MainFrame( int argc, char*argv[] ) :
  QApplication( argc, argv ),
  Counter( "MainFrame" ),
  args_( argc, argv ),
  application_manager_( 0 ),
  attachment_frame_( 0 ),
  selection_frame_( 0 ),
  realized_( false )
{ 
  Debug::Throw( "MainFrame::MainFrame.\n" ); 
  setStyle( new FlatStyle() );
} 

//____________________________________________
MainFrame::~MainFrame( void ) 
{
  Debug::Throw( "MainFrame::~MainFrame.\n" );
  XmlOptions::write();
  
  if( application_manager_ ) {
    delete application_manager_; 
    application_manager_ = 0;
  }

  // error handler
  ErrorHandler::exit();

} 

//____________________________________________
void MainFrame::initApplicationManager(  void )
{
  Debug::Throw( "MainFrame::InitApplicationManager. Done.\n" ); 

  // disable server mode from option
  if( args_.find( "--no-server" ) ) {
    realizeWidget();
    return;
  }
  
  if( application_manager_ ) return;

  // create application manager
  application_manager_ = new ApplicationManager( this );
  application_manager_->setApplicationName( "ELOGBOOK" );
  connect( 
    application_manager_, SIGNAL( stateChanged( SERVER::ApplicationManager::State ) ),
    SLOT( _applicationManagerStateChanged( SERVER::ApplicationManager::State ) ) );
    
  connect( application_manager_, SIGNAL( serverRequest( const ArgList& ) ), SLOT( _processRequest( const ArgList& ) ) );
    
  // initialize application manager  
  application_manager_->init( args_ );
}
  
//____________________________________________
void MainFrame::realizeWidget( void )
{
  Debug::Throw( "MainFrame::realizeWidget.\n" );
 
  //! check if the method has already been called.
  if( realized_ ) return;
  realized_ = true;

  // create attachment window
  attachment_frame_ = new AttachmentFrame( 0 );
  
  // create selection frame
  selection_frame_ = new SelectionFrame( 0 );

  // update configuration
  updateConfiguration();

  // splashscreen
  QPixmap pixmap( (File( XmlOptions::get().raw( "ICON_PIXMAP" ) )).expand().c_str() );
  
  ostringstream what;
  what << "<B>eLogbook</B><BR> version " << VERSION;
  SplashScreen *splash_screen = new SplashScreen( selection_frame_, what.str() );
  splash_screen->setIcon( pixmap );
  splash_screen->setOpacity( 0.7 );
  splash_screen->setMinimumSize( QSize( 350, 150 ) );
  splash_screen->realizeWidget();

  // connections
  connect( selection_frame_, SIGNAL( messageAvailable( const QString& ) ), splash_screen, SLOT( displayMessage( const QString& ) ) );
  connect( selection_frame_, SIGNAL( ready() ), splash_screen, SLOT( close() ) );

  QtUtil::centerOnDesktop( splash_screen );
  if( XmlOptions::get().get<bool>("SPLASH_SCREEN") )
  { splash_screen->show(); }
  
  QtUtil::centerOnDesktop( selection_frame_ );
  selection_frame_->show();
    
  // update
  processEvents();
    
  // try open file from argument
  File file( args_.last() );
  if( file.size() ) selection_frame_->setLogbook( file );
  else if( !selection_frame_->menu().openPreviousMenu().openLastValidFile() )
  { 
    splash_screen->close();
    selection_frame_->newLogbookAction().trigger();
  }
  
}

//_________________________________________________
void MainFrame::configuration( void )
{
  
  Debug::Throw( "MainFrame::configuration" );
  ConfigurationDialog dialog(0);
  connect( &dialog, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );
  QtUtil::centerOnWidget( &dialog, activeWindow() );
  dialog.exec();

}

//_________________________________________________
void MainFrame::updateConfiguration( void )
{
  
  Debug::Throw( "MainFrame::UpdateConfiguration.\n" );

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
  
  // clear IconEngine cache (in case of icon_path_list that changed)
  IconEngine::get().clear();

  emit configurationChanged();
}

//_______________________________________________
void MainFrame::about( void )
{

  Debug::Throw( "MainFrame::about.\n" );
  ostringstream what;
  what << "<b>eLogbook</b> version " << VERSION << " (" << BUILD_TIMESTAMP << ")<br>";
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

//_______________________________________________
void MainFrame::showSplashScreen( void )
{

  QPixmap pixmap( (File( XmlOptions::get().raw( "ICON_PIXMAP" ) )).expand().c_str() );

  ostringstream what;
  what << "<B>eLogbook</B><BR> version " << VERSION;
  SplashScreen *splash_screen = new SplashScreen( 0, what.str() );
  splash_screen->setOpacity( 0.3 );
  splash_screen->setMinimumSize( QSize( 350, 150 ) );
  splash_screen->setIcon( pixmap );
  splash_screen->realizeWidget();
  splash_screen->show();
  splash_screen->displayMessage( "click on the window to close" );
  QtUtil::centerOnParent( splash_screen );

  processEvents();

}

//_________________________________________________
void MainFrame::exit( void )
{
  
  Debug::Throw( "MainFrame::exit.\n" );
      
  // ensure everything is saved properly
  if( selection_frame_ )
  {
    // check if editable EditFrames needs save 
    KeySet<EditFrame> frames( selection_frame_ );
    for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    {
      if( (!(*iter)->isReadOnly()) && (*iter)->modified() && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) 
      { return; }
    }
    
    Debug::Throw( "EditFrames saved.\n" );
    
    // check if current logbook is modified
    if( 
      selection_frame_->logbook() &&
      selection_frame_->logbook()->modified() &&
      selection_frame_->askForSave() == AskForSaveDialog::CANCEL ) 
    return;
  }
  
  quit();
  
}
 
//________________________________________________
void MainFrame::_processRequest( const ArgList& args )
{

  Debug::Throw() << "MainFrame::_processRequest - " << args << endl;
  
  if( selection_frame_ ) selection_frame_->uniconifyAction().trigger();

  // check argument. Last argument, if starting with a "-" is possibly a filename
  string filename( args.last() );
  
  if( !filename.empty() ) 
  {
    ostringstream what;
    what << "Accept request for file \"" << filename << "\" ?";
    if( QtUtil::questionDialog( selection_frame_, what.str(), QtUtil::CENTER_ON_PARENT ) )
    { selection_frame_->menu().openPreviousMenu().select( filename ); }
  }

}

//________________________________________________
void MainFrame::_applicationManagerStateChanged( SERVER::ApplicationManager::State state )
{

  Debug::Throw() << "MainFrame::_applicationManagerStateChanged - state=" << state << endl;

  switch ( state ) {
    case ApplicationManager::ALIVE:
    realizeWidget();
    break;
    
    case ApplicationManager::DEAD:
    quit();
    break;
    
    default:
    break;
  }
  
  return;
  
}
