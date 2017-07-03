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
#include "ResourceMigration.h"
#include "Util.h"
#include "XmlOptions.h"

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
    // TODO use command-line arguments
    if( argc < 2 )
    {
        Debug::Throw(0) << "usage: uncompress-logbook <input file>" << endl;
        return 0;
    }

    // load argument
    File input( argv[1] );

    // load options
    QString user( Util::user( ) );
    QString host( Util::host() );
    XmlOptions::get().set( "USER", Option( user+"@"+host, Option::Flag::None ) );

    // install default options
    installDefaultOptions();

    // user options
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

    // the core application is needed to have locale, fonts, etc. set properly, notably for QSting
    // not having it might result in lost accents and special characters.
    QCoreApplication application( argc, argv );

    // install error handler
    ErrorHandler::get().disableMessage( "qUncompress: Z_DATA_ERROR: Input data is corrupted file" );
    ErrorHandler::initialize();

    // try open input logbook
    Debug::Throw(0) << "uncompress-logbook - reading from: " << input << endl;
    Logbook logbook;
    logbook.setFile( input.expanded() );
    if( !logbook.read() )
    {
        Debug::Throw(0) << "uncompress-logbook - error reading logbook" << endl;
        return 0;
    }

    // debug
    Debug::Throw(0) << "uncompress-logbook - number of files: " << logbook.children().size() << endl;
    Debug::Throw(0) << "uncompress-logbook - number of entries: " << logbook.entries().size() << endl;

    // perform copy
    Debug::Throw(0) << "uncompress-logbook - uncompressing" << endl;
    logbook.setUseCompression( false );
    logbook.setModifiedRecursive( true );

    // copy logbook to ouput
    if( !logbook.write() )
    { Debug::Throw(0) << "uncompress-logbook - error writing to file " << input << endl; }

    return 0;
}

//_____________________________________________
void interrupt( int sig )
{
    Debug::Throw() << "uncompress-logbook::interrupt - Recieved signal " << sig << endl;
    exit(0);
}
