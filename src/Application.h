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
    bool initApplicationManager() override;

    //* create all widgets
    bool realizeWidget() override;

    //* file list
    FileList& recentFiles() const
    { return *recentFiles_; }

    //* retrieve AttachmentWindow singleton
    AttachmentWindow & attachmentWindow() const
    { return *attachmentWindow_; }

    //* retrieve MainWindow singleton
    MainWindow& mainWindow() const
    { return *mainWindow_; }

    //* scratch files
    ScratchFileMonitor& scratchFileMonitor() const
    { return *scratchFileMonitor_; }

    //*@name application information
    //@{

    //* command line help
    void usage() const override;

    //* application name
    QString applicationName() const override
    { return "Elogbook"; }

    //* application icon
    QIcon applicationIcon() const override
    { return IconEngine::get( ":/elogbook.png" ); }

    // application version
    QString applicationVersion() const override
    { return VERSION; }

    //@}

    protected Q_SLOTS:

    //* configuration
    void _configuration() override;

    //* exit safely
    void _exit();

    //* process request from application manager
    bool _processCommand( Server::ServerCommand ) override;

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration();

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
