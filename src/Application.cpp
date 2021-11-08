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
#include "AttachmentWindow.h"
#include "ConfigurationDialog.h"
#include "Debug.h"
#include "EditionWindow.h"
#include "File.h"
#include "InformationDialog.h"
#include "Logbook.h"
#include "MainWindow.h"
#include "MenuBar.h"
#include "QtUtil.h"
#include "QuestionDialog.h"
#include "RecentFilesMenu.h"
#include "ScratchFileMonitor.h"
#include "XmlFileList.h"
#include "XmlOptions.h"


#include <QMessageBox>

//____________________________________________
Application::Application( const CommandLineArguments &arguments ) :
    BaseApplication( nullptr, arguments ),
    Counter( QStringLiteral("Application") )
{}

//____________________________________________
bool Application::initApplicationManager()
{
    Debug::Throw( QStringLiteral("Application::initApplicationManager.\n") );

    // retrieve files from arguments and expand if needed
    auto parser( commandLineParser( _arguments() ) );

    auto&& orphans( parser.orphans() );
    for( auto&& orphan:orphans )
    { if( !orphan.isEmpty() ) File::expand( orphan ); }

    // replace arguments
    _setArguments( parser.arguments() );

    // base class initialization
    return BaseApplication::initApplicationManager();

}

//____________________________________________
bool Application::realizeWidget()
{
    Debug::Throw( QStringLiteral("Application::realizeWidget.\n") );

    // check if the method has already been called.
    if( !BaseApplication::realizeWidget() ) return false;

    // need to redirect closeAction to proper exit
    closeAction().disconnect();
    connect( &closeAction(), &QAction::triggered, this, &Application::_exit );

    // recent files
    recentFiles_.reset( new XmlFileList );
    recentFiles_->setCheck( true );

    // create attachment window
    attachmentWindow_.reset( new AttachmentWindow );
    attachmentWindow_->centerOnDesktop();

    // create selection frame
    mainWindow_.reset( new MainWindow );
    connect( attachmentWindow_.get(), &AttachmentWindow::entrySelected, mainWindow_.get(), QOverload<LogEntry*>::of( &MainWindow::selectEntry ) );

    // update configuration
    connect( this, &Application::configurationChanged, this, &Application::_updateConfiguration );
    emit configurationChanged();

    mainWindow_->centerOnDesktop();
    mainWindow_->show();

    // scratch files
    scratchFileMonitor_.reset( new ScratchFileMonitor );
    connect( qApp, &QCoreApplication::aboutToQuit, scratchFileMonitor_.get(), &ScratchFileMonitor::deleteScratchFiles );
    connect( mainWindow_.get(), &MainWindow::scratchFileCreated, scratchFileMonitor_.get(), &ScratchFileMonitor::add );

    // update
    qApp->processEvents();

    // load file from arguments or recent files
    const auto filenames( commandLineParser( _arguments() ).orphans() );
    const File file = filenames.empty() ?
        recentFiles_->lastValidFile().file():
        File( filenames.front() );

    if( mainWindow_->setLogbook( file ) ) mainWindow_->checkLogbookBackup();
    else {

        // show information dialog, provided that fileList is not empty
        if( !file.isEmpty() )
        {
            const auto buffer = tr( "Unable to open file '%1'." ).arg( file );
            InformationDialog( mainWindow_.get(), buffer ).exec();
        }

        // create a default, empty logbook
        mainWindow_->createDefaultLogbook();

    }

    return true;

}

//____________________________________________
void Application::usage() const
{
    _usage( QStringLiteral("elogbook"), tr( "[options] [file]" ) );
    commandLineParser().usage();
    return;
}

//_________________________________________________
void Application::_configuration()
{

    Debug::Throw( QStringLiteral("Application::_configuration") );
    emit saveConfiguration();
    ConfigurationDialog dialog;
    connect( &dialog, &BaseConfigurationDialog::configurationChanged, this, &BaseCoreApplication::configurationChanged );
    dialog.centerOnWidget( qApp->activeWindow() );
    dialog.exec();

}

//_________________________________________________
void Application::_updateConfiguration()
{
    Debug::Throw( QStringLiteral("Application::_updateConfiguration.\n") );
    static_cast<XmlFileList*>(recentFiles_.get())->setDBFile( File( XmlOptions::get().raw( QStringLiteral("RC_FILE") ) ) );
    recentFiles_->setMaxSize( XmlOptions::get().get<int>( QStringLiteral("DB_SIZE") ) );
}

//_________________________________________________
void Application::_exit()
{

    Debug::Throw( QStringLiteral("Application::_exit.\n") );

    // ensure everything is saved properly
    if( mainWindow_ )
    {
        auto reply = mainWindow_->checkModifiedEntries();
        if( reply == AskForSaveDialog::Cancel ) return;
        else if( reply == AskForSaveDialog::Yes ) mainWindow_->saveUnchecked();
        else if( mainWindow_->logbookIsModified() && mainWindow_->askForSave() == AskForSaveDialog::Cancel ) return;
    }

    qApp->quit();

}

//________________________________________________
bool Application::_processCommand( const Server::ServerCommand& command )
{

    Debug::Throw( QStringLiteral("Application::_processCommand.\n") );
    if( BaseApplication::_processCommand( command ) ) return true;
    if( command.command() == Server::ServerCommand::CommandType::Raise )
    {

        if( mainWindow_ ) mainWindow_->uniconifyAction().trigger();
        auto filenames( commandLineParser( command.arguments() ).orphans() );
        if( !filenames.isEmpty() )
        {

            auto buffer = tr( "Accept request for file '%1'?" ).arg( filenames.front() );
            if( QuestionDialog( mainWindow_.get(), buffer ).centerOnParent().exec() )
            { mainWindow_->setLogbook( File( filenames.front() ) ); }

        }

        return true;

    } else return false;

}
