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
  \file    eLogbook.cpp
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
#include "SystemOptions.h"
#include "ErrorHandler.h"
#include "MainFrame.h"
#include "XmlOptions.h"

using namespace std;

//_______________________________
//! to handle keyboard interruptions
void interrupt( int sig ); 
 
//__________________________________________
//! main function
int main (int argc, char *argv[])
{
  try {
              
    // Ensure proper cleaning at exit
    signal(SIGINT,  interrupt);
    signal(SIGTERM, interrupt);
        
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
   
    // install default options
    installDefaultOptions();
    installSystemOptions();
    XmlOptions::read( XmlOptions::get().raw( "RC_FILE" ) ); 
               
    // set debug level
    Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    if( Debug::level() ) XmlOptions::get().dump();
    
    // initialize main frame and run loop
    Q_INIT_RESOURCE( pixmaps );
    MainFrame main_frame(argc, argv);
    main_frame.initApplicationManager();
    main_frame.exec();

  } catch ( exception& e ) { cout << e.what() << endl; }
  return 0;
}

//_____________________________________________
void interrupt( int sig )
{ 
  Debug::Throw() << "interrupt - Recieved signal " << sig << endl;
  qApp->quit();
}