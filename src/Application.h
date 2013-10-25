#ifndef Application_h
#define Application_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "BaseApplication.h"
#include "CommandLineArguments.h"
#include "CommandLineParser.h"
#include "Config.h"
#include "Counter.h"

class AttachmentWindow;
class FileList;
class LogEntry;
class MainWindow;
class ScratchFileMonitor;

/*!
\class  Application
\brief  Main Window singleton object
*/

class Application: public BaseApplication, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! command line help
    static void usage( void );

    //! constructor
    Application( CommandLineArguments );

    //! destructor
    ~Application( void );

    //! application manager
    virtual void initApplicationManager( void );

    //! create all widgets
    virtual bool realizeWidget( void );

    //! file list
    FileList& recentFiles( void ) const
    { return *recentFiles_; }

    //! retrieve AttachmentWindow singleton
    AttachmentWindow & attachmentWindow( void ) const
    { return *attachmentWindow_; }

    //! retrieve MainWindow singleton
    MainWindow& mainWindow( void ) const
    { return *mainWindow_; }

    //! scratch files
    ScratchFileMonitor& scratchFileMonitor( void ) const
    { return *scratchFileMonitor_; }

    protected Q_SLOTS:

    //! about eLogbook
    virtual void _about( void )
    { BaseApplication::_about( "Elogbook", VERSION, BUILD_TIMESTAMP ); }

    //! configuration
    virtual void _configuration( void );

    //! exit safely
    void _exit( void );

    //! process request from application manager
    virtual bool _processCommand( SERVER::ServerCommand );

    private:

    //! recent files
    FileList* recentFiles_;

    //! toplevel attachment frame
    AttachmentWindow* attachmentWindow_;

    //! main window entry selection frame
    MainWindow* mainWindow_;

    //! scratch files
    ScratchFileMonitor* scratchFileMonitor_;


};

#endif
