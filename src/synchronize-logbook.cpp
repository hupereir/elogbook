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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Debug.h"
#include "DefaultOptions.h"
#include "ErrorHandler.h"
#include "Logbook.h"
#include "Options.h"
#include "ResourceMigration.h"
#include "Util.h"

#include <QCoreApplication>
#include <QString>
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
    File local( argv[1] );
    File remote( argv[2] );

    // load options
    QString user( Util::user( ) );
    QString host( Util::host() );
    XmlOptions::get().set( "USER", Option( user+"@"+host, Option::Flag::None ) );

    // options
    installDefaultOptions();

    // migrate old rc files
    File oldRCFile( XmlOptions::get().raw( "OLD_RC_FILE" ) );
    File rcFile( XmlOptions::get().raw( "RC_FILE" ) );
    ResourceMigration( oldRCFile ).migrate( rcFile );

    // assign and read
    XmlOptions::setFile( rcFile );
    XmlOptions::read();

    // force debug level to 0
    XmlOptions::get().set<int>("DEBUG_LEVEL", 0 );

    // debug level
    Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    if( Debug::level() ) Debug::Throw() << XmlOptions::get() << endl;

    // compression
    bool useCompression( XmlOptions::get().get<bool>( "USE_COMPRESSION" ) );

    // the core application is needed to have locale, fonts, etc. set properly, notably for QSting
    // not having it might result in lost accents and special characters.
    QCoreApplication application( argc, argv );

    // install error handler
    ErrorHandler::get().disableMessage( "qUncompress: Z_DATA_ERROR: Input data is corrupted file" );
    ErrorHandler::initialize();

    // try open local Logbook
    Debug::Throw(0) << "synchronize-logbook - reading local logbook from: " << local << endl;
    Logbook localLogbook;
    localLogbook.setFile( local.expand() );
    localLogbook.setUseCompression( useCompression );
    if( !localLogbook.read() )
    {
        Debug::Throw(0) << "synchronize-logbook - error reading local logbook" << endl;
        return 0;
    }

    // debug
    Debug::Throw(0) << "synchronize-logbook - number of local files: " << localLogbook.children().size() << endl;
    Debug::Throw(0) << "synchronize-logbook - number of local entries: " << localLogbook.entries().size() << endl;

    // try open localLogbook
    Debug::Throw(0) << "synchronize-logbook - reading remote logbook from: " << remote << endl;
    Logbook remoteLogbook;
    remoteLogbook.setFile( remote.expand() );
    remoteLogbook.setUseCompression( useCompression );
    if( !remoteLogbook.read() )
    {
        Debug::Throw(0) << "synchronize-logbook - error reading remote logbook" << endl;
        return 0;
    }

    // debug
    Debug::Throw(0) << "synchronize-logbook - number of remote files: " << remoteLogbook.children().size() << endl;
    Debug::Throw(0) << "synchronize-logbook - number of remote entries: " << remoteLogbook.entries().size() << endl;

    // check whether local logbook is read-only
    if( localLogbook.isReadOnly() )
    {

        Debug::Throw(0) << "synchronize-logbook - local logbook is read-only. It will not be synchronized to the remote logbook." << endl;

    } else {

        Debug::Throw(0) << "synchronize-logbook - updating local from remote" << endl;
        const int nDuplicated( localLogbook.synchronize( remoteLogbook ).size() );
        Debug::Throw(0) << "synchronize-logbook - number of duplicated entries: " << nDuplicated << endl;

        if( !localLogbook.write() )
        {
            Debug::Throw(0) << "synchronize-logbook - error writing local logbook" << endl;
            return 0;
        }

    }

    // check whether remote logbook is read-only
    if( remoteLogbook.isReadOnly() )
    {

        Debug::Throw(0) << "synchronize-logbook - remote logbook is read-only. It will not be synchronized to the local logbook." << endl;

    } else {

        Debug::Throw(0) << "synchronize-logbook - updating remote from local" << endl;
        const int nDuplicated( remoteLogbook.synchronize( localLogbook ).size() );

        Debug::Throw(0) << "synchronize-logbook - number of duplicated entries: " << nDuplicated << endl;

        if( !remoteLogbook.write() )
        {
            Debug::Throw(0) << "error writting to remote logbook" << endl;
            return 0;
        }

    }
    return 0;

}

//_____________________________________________
void interrupt( int sig )
{
    Debug::Throw() << "interrupt - Recieved signal " << sig << endl;
    exit(0);
}
