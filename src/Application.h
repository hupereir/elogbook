#ifndef Application_h
#define Application_h

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

#include "AttachmentWindow.h"
#include "BaseApplication.h"
#include "CommandLineArguments.h"
#include "CommandLineParser.h"
#include "Config.h"
#include "Counter.h"
#include "FileList.h"
#include "IconEngine.h"
#include "ScratchFileMonitor.h"
#include "MainWindow.h"

#include <memory>

//* application
class Application: public BaseApplication, private Base::Counter<Application>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit Application( CommandLineArguments );

    //* application manager
    virtual bool initApplicationManager( void );

    //* create all widgets
    virtual bool realizeWidget( void );

    //* file list
    FileList& recentFiles( void ) const
    { return *recentFiles_; }

    //* retrieve AttachmentWindow singleton
    AttachmentWindow & attachmentWindow( void ) const
    { return *attachmentWindow_; }

    //* retrieve MainWindow singleton
    MainWindow& mainWindow( void ) const
    { return *mainWindow_; }

    //* scratch files
    ScratchFileMonitor& scratchFileMonitor( void ) const
    { return *scratchFileMonitor_; }

    //*@name application information
    //@{

    //* command line help
    void usage( void ) const;

    //* application name
    virtual QString applicationName( void ) const
    { return "Elogbook"; }

    //* application name
    virtual QIcon applicationIcon( void ) const
    { return IconEngine::get( ":/elogbook.png" ); }

    // application version
    virtual QString applicationVersion( void ) const
    { return VERSION; }

    //@}

    protected Q_SLOTS:

    //* configuration
    virtual void _configuration( void );

    //* exit safely
    void _exit( void );

    //* process request from application manager
    virtual bool _processCommand( Server::ServerCommand );

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration( void );

    private:

    //* recent files
    std::unique_ptr<FileList> recentFiles_;

    //* toplevel attachment frame
    std::unique_ptr<AttachmentWindow> attachmentWindow_;

    //* main window entry selection frame
    std::unique_ptr<MainWindow> mainWindow_;

    //* scratch files
    std::unique_ptr<ScratchFileMonitor> scratchFileMonitor_;

};

#endif
