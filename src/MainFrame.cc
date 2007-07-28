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
#include "Debug.h"
#include "EditFrame.h"
#include "File.h"
#include "Logbook.h"
#include "MainFrame.h"
#include "Menu.h"
#include "XmlOptions.h"
#include "OpenPreviousMenu.h"
#include "QtUtil.h"
#include "SelectionFrame.h"
#include "StateFrame.h"
#include "SplashScreen.h"

using namespace std;
using namespace BASE;
using namespace SERVER;

//____________________________________________
const string MainFrame::MAIN_TITLE_MODIFIED = "Electronic logbook (modified)";
const string MainFrame::MAIN_TITLE = "Electronic logbook ";

//____________________________________________
void MainFrame::Usage( void )
{
  std::cout << "Usage : eLogbook [options] [file]" << std::endl;
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
    
  // about to quit connection
  connect( this, SIGNAL( aboutToQuit() ), this, SLOT( _AboutToQuit() ) );
  
} 

//____________________________________________
MainFrame::~MainFrame( void ) 
{} 

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
    application_manager_, SIGNAL( stateChanged( const int & ) ),
    this, SLOT( _applicationManagerStateChanged( const int & ) ) );
    
  connect( application_manager_, SIGNAL( serverRequest( const ArgList& ) ), this, SLOT( _processRequest( const ArgList& ) ) );
    
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
  attachment_frame_ = new AttachmentFrame( 0, "attachment_frame" );
  attachment_frame_->setWindowTitle( MAIN_TITLE );
  
  // create selection frame
  selection_frame_ = new TreeSelectionFrame( 0 );
  selection_frame_->setWindowTitle( MAIN_TITLE );

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
  
  QtUtil::centerOnDesktop( selection_frame_, false );
  selection_frame_->show();
    
  // update
  processEvents();
    
  // try open file from argument
  File file( args_.Last() );
  if( file.size() ) selection_frame_->setLogbook( file );
  else if( !selection_frame_->menu().openPreviousMenu().openLastValidFile() )
  { 
    splash_screen->close();
    selection_frame_->newLogbook();
  }
  
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

  emit configurationChanged();
}

//_______________________________________________
void MainFrame::showSplashScreen( void )
{

  QPixmap pixmap( (File( XmlOptions::get().getRaw( "ICON_PIXMAP" ) )).expand().c_str() );

  ostringstream what;
  what << "<B>eLogbook</B><BR> version " << VERSION;
  SplashScreen *splash_screen = new SplashScreen( this, what.str() );
  splash_screen->SetOpacity( 0.7 );
  splash_screen->SetMinimumSize( QSize( 350, 150 ) );
  splash_screen->setIcon( pixmap );
  splash_screen->RealizeWidget();
  splash_screen->show();
  splash_screen->displayMessage( "click on the window to close" );
  QtUtil::CenterOnParent( splash_screen );

  processEvents();

}

//_________________________________________________
void MainFrame:exit( void )
{
  
  Debug::Throw( "MainFrame::exit.\n" );
      
  // ensure everything is saved properly
  if( selection_frame_ )
  {
    // check if editable EditFrames needs save 
    KeySet<EditFrame> frames( selection_frame_ );
    for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    {
      if( (!(*iter)->isReadOnly()) && (*iter)->Modified() && (*iter)->AskForSave() == AskForSaveDialog::CANCEL ) 
      { return; }
    }
    
    Debug::Throw( "EditFrames saved.\n" );
    
    // check if current logbook is modified
    if( 
      selection_frame_->GetLogbook() &&
      selection_frame_->GetLogbook()->GetModified() &&
      selection_frame_->AskForSave() == AskForSaveDialog::CANCEL ) 
    return;
  }
  
  quit();
  
}
 
//________________________________________________
void MainFrame::_processRequest( const ArgList& args )
{

  Debug::Throw() << "MainFrame::_processRequest - " << args << endl;
  
  if( selection_frame_ ) selection_frame_->uniconify();

  // check argument. Last argument, if starting with a "-" is possibly a filename
  string filename( args.Last() );
  if( filename.size() ) {
    ostringstream what;
    what << "Accept request for file \"" << filename << "\" ?";
    if( QtUtil::questionDialog( selection_frame_, what.str(), QtUtil::CENTER_ON_PARENT ) )
    { selection_frame_->menu().openPreviousMenu().select( filename ); }
  }

}

//________________________________________________
void MainFrame::_applicationManagerStateChanged( const int & state )
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

//_______________________________________________
void MainFrame::_aboutToQuit( void )
{ 
  Debug::Throw( "MainFrame::_AboutToQuit.\n" );
  
  if( selection_frame_ ) selection_frame_->menu().openPreviousMenu().write();
  if( selection_frame_ )
  {
    
    // write list view config
    XmlOptions::get().set<unsigned int>( "ENTRYLIST_MASK", selection_frame_->logEntryList().mask() ); 
    XmlOptions::write();
    
  }
  
  if( application_manager_ ) {
    delete application_manager_; 
    application_manager_ = 0;
  }
    
}
  
