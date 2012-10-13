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
#include "Options.h"
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
        Debug::Throw(0) << "usage: synchronize-logbook <local file> <remote file>" << endl;
        return 0;
    }

    // load argument
    QString local( argv[1] );
    QString remote( argv[2] );

    // load options
    QString user( Util::user( ) );
    QString host( Util::host() );
    XmlOptions::get().set( "USER", Option( user+"@"+host, Option::None ) );

    // install default options
    installDefaultOptions();

    // load user resource file
    QString rcfile = Util::env( "HOME", "." ) + "/.eLogbookrc";
    XmlOptions::read( rcfile );

    // force debug level to 0
    XmlOptions::get().set<int>("DEBUG_LEVEL", 0 );

    // set debug level
    int debug_level( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    Debug::setLevel( debug_level );
    if( debug_level ) XmlOptions::get().print();

    // the core application is needed to have locale, fonts, etc. set properly, notably for QSting
    // not having it might result in lost accents and special characters.
    QCoreApplication application( argc, argv );

    // try open local_logbook
    Debug::Throw(0) << "synchronize-logbook - reading local logbook from: " << local << endl;
    Logbook local_logbook;
    local_logbook.setFile( File( local ).expand() );
    if( !local_logbook.read() )
    {
        Debug::Throw(0) << "synchronize-logbook - error reading local logbook" << endl;
        return 0;
    }

    // debug
    Debug::Throw(0) << "synchronize-logbook - number of local files: " << local_logbook.children().size() << endl;
    Debug::Throw(0) << "synchronize-logbook - number of local entries: " << local_logbook.entries().size() << endl;

    // try open local_logbook
    Debug::Throw(0) << "synchronize-logbook - reading remote logbook from: " << remote << endl;
    Logbook remote_logbook;
    remote_logbook.setFile( File( remote ).expand() );
    if( !remote_logbook.read() )
    {
        Debug::Throw(0) << "synchronize-logbook - error reading remote logbook" << endl;
        return 0;
    }

    // debug
    Debug::Throw(0) << "synchronize-logbook - number of remote files: " << remote_logbook.children().size() << endl;
    Debug::Throw(0) << "synchronize-logbook - number of remote entries: " << remote_logbook.entries().size() << endl;

    Debug::Throw(0) << "synchronize-logbook - updating local from remote" << endl;
    int n_duplicated = local_logbook.synchronize( remote_logbook ).size();

    Debug::Throw(0) << "synchronize-logbook - number of duplicated entries: " << n_duplicated << endl;

    if( !local_logbook.write() )
    {
        Debug::Throw(0) << "synchronize-logbook - error writing local logbook" << endl;
        return 0;
    }

    Debug::Throw(0) << "synchronize-logbook - updating remote from local" << endl;
    n_duplicated = remote_logbook.synchronize( local_logbook ).size();

    Debug::Throw(0) << "synchronize-logbook - number of duplicated entries: " << n_duplicated << endl;

    if( !remote_logbook.write() )
    {
        Debug::Throw(0) << "error writting to remote logbook" << endl;
        return 0;
    }

    return 0;

}

//_____________________________________________
void interrupt( int sig )
{
    Debug::Throw() << "interrupt - Recieved signal " << sig << endl;
    exit(0);
}
