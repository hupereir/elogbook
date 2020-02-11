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
        Debug::Throw(0) << "usage: synchronize-logbook <first logbook> <second logbook>" << endl;
        return 0;
    }

    // load argument
    File first( argv[1] );
    File second( argv[2] );

    // load options
    QString user( Util::user( ) );
    QString host( Util::host() );
    XmlOptions::get().set( QStringLiteral("USER"), Option( user+"@"+host, Option::Flag::None ) );

    // options
    installDefaultOptions();

    // migrate old rc files
    File oldRCFile( XmlOptions::get().raw( QStringLiteral("OLD_RC_FILE") ) );
    File rcFile( XmlOptions::get().raw( QStringLiteral("RC_FILE") ) );
    ResourceMigration( oldRCFile ).migrate( rcFile );

    // assign and read
    XmlOptions::setFile( rcFile );
    XmlOptions::read();

    // force debug level to 0
    XmlOptions::get().set<int>(QStringLiteral("DEBUG_LEVEL"), 0 );

    // debug level
    Debug::setLevel( XmlOptions::get().get<int>( QStringLiteral("DEBUG_LEVEL") ) );
    if( Debug::level() ) Debug::Throw() << XmlOptions::get() << endl;

    // compression
    bool useCompression( XmlOptions::get().get<bool>( QStringLiteral("USE_COMPRESSION") ) );

    // the core application is needed to have locale, fonts, etc. set properly, notably for QSting
    // not having it might result in lost accents and special characters.
    QCoreApplication application( argc, argv );

    // install error handler
    ErrorHandler::get().disableMessage( QStringLiteral("qUncompress: Z_DATA_ERROR: Input data is corrupted file") );
    ErrorHandler::initialize();

    // try open first Logbook
    Debug::Throw(0) << "synchronize-logbook - reading first logbook from: " << first << endl;
    Logbook firstLogbook;
    firstLogbook.setFile( first.expanded() );
    firstLogbook.setUseCompression( useCompression );
    if( !firstLogbook.read() )
    {
        Debug::Throw(0) << "synchronize-logbook - error reading first logbook" << endl;
        return 0;
    }

    // debug
    Debug::Throw(0) << "synchronize-logbook - number of files in first logbook: " << firstLogbook.children().size() << endl;
    Debug::Throw(0) << "synchronize-logbook - number of entries in first logbook: " << firstLogbook.entries().size() << endl;

    // try open second logbook
    Debug::Throw(0) << "synchronize-logbook - reading second logbook from: " << second << endl;
    Logbook secondLogbook;
    secondLogbook.setFile( second.expanded() );
    secondLogbook.setUseCompression( useCompression );
    if( !secondLogbook.read() )
    {
        Debug::Throw(0) << "synchronize-logbook - error reading second logbook" << endl;
        return 0;
    }

    // debug
    Debug::Throw(0) << "synchronize-logbook - number of files in second logbook: " << secondLogbook.children().size() << endl;
    Debug::Throw(0) << "synchronize-logbook - number of entries in second logbook: " << secondLogbook.entries().size() << endl;

    // check whether first logbook is read-only
    if( firstLogbook.isReadOnly() )
    {

        Debug::Throw(0) << "synchronize-logbook - first logbook is read-only. It will not be synchronized to the second logbook." << endl;

    } else {

        Debug::Throw(0) << "synchronize-logbook - updating first logbook from second" << endl;
        const int nDuplicated( firstLogbook.synchronize( secondLogbook ).size() );
        Debug::Throw(0) << "synchronize-logbook - number of duplicated entries: " << nDuplicated << endl;

        if( !firstLogbook.write() )
        {
            Debug::Throw(0) << "synchronize-logbook - error writing first logbook" << endl;
            return 0;
        }

    }

    // check whether second logbook is read-only
    if( secondLogbook.isReadOnly() )
    {

        Debug::Throw(0) << "synchronize-logbook - second logbook is read-only. It will not be synchronized to the first logbook." << endl;

    } else {

        Debug::Throw(0) << "synchronize-logbook - updating second logbook from first" << endl;
        const int nDuplicated( secondLogbook.synchronize( firstLogbook ).size() );

        Debug::Throw(0) << "synchronize-logbook - number of duplicated entries: " << nDuplicated << endl;

        if( !secondLogbook.write() )
        {
            Debug::Throw(0) << "error writting to second logbook" << endl;
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
