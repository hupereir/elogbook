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

#include "Application.h"
#include "Debug.h"
#include "DefaultOptions.h"
#include "ErrorHandler.h"
#include "ResourceMigration.h"
#include "SystemOptions.h"
#include "Singleton.h"
#include "XmlFileRecord.h"
#include "XmlOptions.h"

#include <QApplication>

//__________________________________________
//! main function
int main (int argc, char *argv[])
{

    // error handler
    ErrorHandler::initialize();
    ErrorHandler::get().disableMessage( "qUncompress: Z_DATA_ERROR: Input data is corrupted file" );

    // options
    installDefaultOptions();
    installSystemOptions();

    // migrate old rc files
    File oldRCFile( XmlOptions::get().raw( "OLD_RC_FILE" ) );
    File rcFile( XmlOptions::get().raw( "RC_FILE" ) );
    ResourceMigration( oldRCFile ).migrate( rcFile );

    // assign and read
    XmlOptions::setFile( rcFile );
    XmlOptions::read();

    // debug level
    Debug::setLevel( XmlOptions::get().get<int>( "DEBUG_LEVEL" ) );
    if( Debug::level() ) Debug::Throw() << XmlOptions::get() << endl;

    // resources
    Q_INIT_RESOURCE( basePixmaps );
    Q_INIT_RESOURCE( pixmaps );

    // create Application
    QApplication application( argc, argv );

    Application singleton( CommandLineArguments( argc, argv ) );
    Base::Singleton::get().setApplication( &singleton );

    // initialize and run
    if( singleton.initApplicationManager() )
    {  application.exec(); }

    return 0;

}
