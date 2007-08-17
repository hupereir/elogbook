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
  \file    eLogbook.cc
  \brief  eLogbook main file
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>

#include "ArgList.h"
#include "Debug.h"
#include "DefaultOptions.h"
#include "ErrorHandler.h"
#include "File.h"
#include "FlatStyle.h"
#include "MainFrame.h"
#include "XmlOptions.h"
#include "Util.h"

using namespace std;

//_______________________________
//! to handle keyboard interuptions
void interupt( int sig ); 
 
//__________________________________________
//! main function
int main (int argc, char *argv[])
{
  try {
              
    // Ensure proper cleaning at exit
    signal(SIGINT,  interupt);
    signal(SIGTERM, interupt);
        
    // install error handler
    qInstallMsgHandler( ErrorHandler::Throw );
    ErrorHandler::disableMessage( "QServerSocket: failed to bind or listen to the socket" );
    ErrorHandler::disableMessage( "QPixmap::resize: TODO: resize alpha data" );
  
    // load possible command file
    ArgList args( argc, argv );
    if( args.find( "--help" ) )
    {
      MainFrame::usage();
      return 0;
    }
      
    // add user name as non recordable option
    string user( Util::user( ) );
    string host( Util::host() );
    Option option( "USER", user+"@"+host );  
    option.setRecordable( false );
    XmlOptions::get().add( option );
   
    // install default options
    installDefaultOptions();

    // add help file
    string helpfile = Util::env( "HOME", "." ) + "/.eLogbook_help";
    option = Option( "HELP_FILE", helpfile );  
    option.setRecordable( false );
    XmlOptions::get().add( option );

    // add help file
    string dbfile = Util::env( "HOME", "." ) + "/.eLogbook_db";
    option = Option( "DB_FILE", dbfile );  
    option.setRecordable( false );
    XmlOptions::get().add( option );

    // load user resource file
    string rcfile = Util::env( "HOME", "." ) + "/.eLogbookrc";
    XmlOptions::read( rcfile ); 
               
    // set debug level
    Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    if( Debug::level() ) XmlOptions::get().dump();
    
    // initialize main frame and run loop
    Q_INIT_RESOURCE( pixmaps );
    MainFrame main_frame(argc, argv);
    main_frame.setStyle( new FlatStyle() );
    main_frame.initApplicationManager();
    main_frame.exec();

  } catch ( exception& e ) { cout << e.what() << endl; }
  return 0;
}

//_____________________________________________
void interupt( int sig )
{ 
  Debug::Throw() << "interupt - Recieved signal " << sig << endl;
  qApp->quit();
}
