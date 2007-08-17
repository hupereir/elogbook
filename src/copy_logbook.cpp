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
  \file copy_logbook.cc
  \brief comand line copy of a logbook
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/


#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>

#include "Debug.h"
#include "DefaultOptions.h"
#include "Logbook.h"
#include "XmlOptions.h"
#include "Util.h"

using namespace std;

//_______________________________
//! to handle keyboard interuptions
void interrupt( int sig ); 

//__________________________________________
//! main function
int main (int argc, char *argv[])
{
  try {
            
    // Ensure proper cleaning at exit
    signal(SIGINT,  interrupt);
    signal(SIGTERM, interrupt);
     
    // read argument
    if( argc < 3 ) 
    {
      cout << "usage: copy_logbook <input file> <output file>" << endl;
      return 0;
    }
    
    // load argument
    string input( argv[1] );
    string output( argv[2] );
    
    // load options
    string user( Util::user( ) );
    string host( Util::host() );
    Option option( "USER", user+"@"+host );  
    option.setRecordable( false );
    XmlOptions::get().add( option );

    // install default options
    installDefaultOptions();
    
    // load user resource file
    string rcfile = Util::env( "HOME", "." ) + "/.eLogbookrc";
    XmlOptions::read( rcfile ); 

    // force debug level to 0
    XmlOptions::get().set<int>("DEBUG_LEVEL", 0 );

    // set debug level
    int debug_level( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    Debug::setLevel( debug_level );
    if( debug_level ) XmlOptions::get().dump();
    
    // try open input logbook   
    cout << "copy_logbook - reading from: " << input << endl;
    Logbook logbook;
    logbook.setFile( File(input).expand() );
    logbook.read();
    
    // debug
    cout << "copy_logbook - number of files: " << logbook.children().size() << endl;
    cout << "copy_logbook - number of entries: " << logbook.entries().size() << endl;
  
    // perform copy
    cout << "copy_logbook - writing to: " << output << endl;
    logbook.setFile( File( output ).expand() );
    logbook.setModifiedRecursive( true );

    // check logbook filename is writable
    File fullname = File( logbook.file() ).expand();
    if( fullname.exists() ) {

      // check file is not a directory
      if( fullname.isDirectory() ) 
      {
        cout << "copy_logbook - selected file is a directory. <Save Logbook> canceled." << endl;
        return 0;
      }

      // check file is writable
      if( !fullname.isWritable() ) {
        cout << "copy_logbook - selected file is not writable. <Save Logbook> canceled." << endl;
        return 0;
      }
      
    } else {
      
      File path( fullname.path() );
      if( !path.isDirectory() ) {
        cout << "copy_logbook - selected path is not valid. <Save Logbook> canceled." << endl;
        return 0;
      }
      
    }
    
    // copy logbook to ouput
    if( !logbook.write() )
    { cout << "copy_logbook - error writing to file " << output << endl; }
    
  }catch ( exception& e ) { cout << e.what() << endl; }
  
  return 0;
}

//_____________________________________________
void interrupt( int sig )
{ 
  Debug::Throw() << "interrupt - Recieved signal " << sig << endl;
  exit(0);
}