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
#include "Application.h"
#include "XmlOptions.h"

#include "CompositeEngine.h"

using namespace std;

//_______________________________
//! to handle keyboard interruptions
void interrupt( int sig ); 
 
//__________________________________________
//! main function
int main (int argc, char *argv[])
{
  
  // Ensure proper cleaning at exit
  signal(SIGINT,  interrupt);
  signal(SIGTERM, interrupt);
  
  // install error handler
  qInstallMsgHandler( ErrorHandler::Throw );
  
  // load possible command file
  ArgList args( argc, argv );
  if( args.find( "--help" ) )
  {
    Application::usage();
    return 0;
  }
  
  // install default options
  installDefaultOptions();
  installSystemOptions();
  XmlOptions::read( XmlOptions::get().raw( "RC_FILE" ) ); 
  
  // set debug level
  Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
  if( Debug::level() ) cout << XmlOptions::get() << endl;
  
  // initialize main frame and run loop
  Q_INIT_RESOURCE( basePixmaps );
  Q_INIT_RESOURCE( pixmaps );
  Q_INIT_RESOURCE( baseSvg );
  
  // create Application
  Application* application( 0 );
  
  TRANSPARENCY::CompositeEngine::get().initialize();
  #ifdef Q_WS_X11
  if( TRANSPARENCY::CompositeEngine::get().isAvailable() )
  { 
    application = new Application( 
      TRANSPARENCY::CompositeEngine::get().display(), 
      argc, argv, 
      TRANSPARENCY::CompositeEngine::get().visual(), 
      TRANSPARENCY::CompositeEngine::get().colormap() );
  } else application = new Application( argc, argv );
  #else
  application = new Application( argc, argv );
  #endif

  application->initApplicationManager();
  application->exec();
  delete application;
  
  return 0;

}

//_____________________________________________
void interrupt( int sig )
{ 
  Debug::Throw() << "interrupt - Recieved signal " << sig << endl;
  qApp->quit();
}
