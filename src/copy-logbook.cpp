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

#include "Debug.h"
#include "DefaultOptions.h"
#include "Logbook.h"
#include "XmlOptions.h"
#include "Util.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QString>

#include <signal.h>
#include <unistd.h>

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

    // read argument
    if( argc < 3 )
    {
        Debug::Throw(0) << "usage: copy-logbook <input file> <output file>" << endl;
        return 0;
    }

    // load argument
    QString input( argv[1] );
    QString output( argv[2] );

    // load options
    QString user( Util::user( ) );
    QString host( Util::host() );
    XmlOptions::get().set( "USER", Option( user+"@"+host, Option::None ) );

    // install default options
    installDefaultOptions();

    // user options
    const QString rcfile( Util::env( "HOME", "." ) + "/.eLogbookrc" );
    XmlOptions::setFile( rcfile );
    XmlOptions::read();

    // force debug level to 0
    XmlOptions::get().set<int>("DEBUG_LEVEL", 0 );

    // debug level
    Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    if( Debug::level() ) Debug::Throw() << XmlOptions::get() << endl;

    // the core application is needed to have locale, fonts, etc. set properly, notably for QSting
    // not having it might result in lost accents and special characters.
    QCoreApplication application( argc, argv );

    // try open input logbook
    Debug::Throw(0) << "copy-logbook - reading from: " << input << endl;
    Logbook logbook;
    logbook.setFile( File(input).expand() );
    logbook.read();

    // debug
    Debug::Throw(0) << "copy-logbook - number of files: " << logbook.children().size() << endl;
    Debug::Throw(0) << "copy-logbook - number of entries: " << logbook.entries().size() << endl;

    // perform copy
    Debug::Throw(0) << "copy-logbook - writing to: " << output << endl;
    logbook.setFile( File( output ).expand() );
    logbook.setModifiedRecursive( true );

    // check logbook filename is writable
    File fullname = File( logbook.file() ).expand();
    if( fullname.exists() ) {

        // check file is not a directory
        if( fullname.isDirectory() )
        {
            Debug::Throw(0) << "copy-logbook - selected file is a directory. <Save Logbook> canceled." << endl;
            return 0;
        }

        // check file is writable
        if( !fullname.isWritable() ) {
            Debug::Throw(0) << "copy-logbook - selected file is not writable. <Save Logbook> canceled." << endl;
            return 0;
        }

    } else {

        File path( fullname.path() );
        if( !path.isDirectory() ) {
            Debug::Throw(0) << "copy-logbook - selected path is not valid. <Save Logbook> canceled." << endl;
            return 0;
        }

    }

    // copy logbook to ouput
    if( !logbook.write() )
    { Debug::Throw(0) << "copy-logbook - error writing to file " << output << endl; }

    return 0;
}

//_____________________________________________
void interrupt( int sig )
{
    Debug::Throw() << "copy-logbook::interrupt - Recieved signal " << sig << endl;
    exit(0);
}
