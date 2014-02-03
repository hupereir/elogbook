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

#include "Application.h"
#include "Application.moc"

#include "AttachmentWindow.h"
#include "AttachmentType.h"
#include "ConfigurationDialog.h"
#include "Debug.h"
#include "EditionWindow.h"
#include "File.h"
#include "InformationDialog.h"
#include "Logbook.h"
#include "MainWindow.h"
#include "Menu.h"
#include "RecentFilesMenu.h"
#include "ScratchFileMonitor.h"
#include "QtUtil.h"
#include "QuestionDialog.h"
#include "XmlFileList.h"
#include "XmlOptions.h"

#include <QMessageBox>

//____________________________________________
Application::Application( CommandLineArguments arguments ) :
    BaseApplication( 0, arguments ),
    Counter( "Application" ),
    recentFiles_( 0 ),
    attachmentWindow_( 0 ),
    mainWindow_( 0 )
{}

//____________________________________________
Application::~Application( void )
{
    Debug::Throw( "Application::~Application.\n" );
    if( mainWindow_ ) delete mainWindow_;
    if( recentFiles_ ) delete recentFiles_;

}

//____________________________________________
bool Application::initApplicationManager( void )
{
    Debug::Throw( "Application::initApplicationManager.\n" );

    // retrieve files from arguments and expand if needed
    CommandLineParser parser( commandLineParser( _arguments() ) );
    QStringList& orphans( parser.orphans() );
    for( QStringList::iterator iter = orphans.begin(); iter != orphans.end(); ++iter )
    { if( !iter->isEmpty() ) (*iter) = File( *iter ).expand(); }

    // replace arguments
    _setArguments( parser.arguments() );

    // base class initialization
    return BaseApplication::initApplicationManager();

}

//____________________________________________
bool Application::realizeWidget( void )
{
    Debug::Throw( "Application::realizeWidget.\n" );

    // check if the method has already been called.
    if( !BaseApplication::realizeWidget() ) return false;

    // need to redirect closeAction to proper exit
    closeAction().disconnect();
    connect( &closeAction(), SIGNAL(triggered()), SLOT(_exit()) );

    // recent files
    recentFiles_ = new XmlFileList();
    recentFiles_->setCheck( true );

    // create attachment window
    attachmentWindow_ = new AttachmentWindow();
    attachmentWindow().centerOnDesktop();

    // create selection frame
    mainWindow_ = new MainWindow();
    connect( &attachmentWindow(), SIGNAL(entrySelected(LogEntry*)), mainWindow_, SLOT(selectEntry(LogEntry*)) );

    // update configuration
    emit configurationChanged();

    mainWindow_->centerOnDesktop();
    mainWindow_->show();

    // scratch files
    scratchFileMonitor_ = new ScratchFileMonitor( this );
    connect( qApp, SIGNAL(aboutToQuit()), scratchFileMonitor_, SLOT(deleteScratchFiles()) );
    connect( mainWindow_, SIGNAL(scratchFileCreated(File)), scratchFileMonitor_, SLOT(add(File)) );

    // update
    qApp->processEvents();

    // load file from arguments or recent files
    QStringList filenames( commandLineParser( _arguments() ).orphans() );
    const File file = filenames.empty() ?
        recentFiles_->lastValidFile().file():
        File( filenames.front() );

    if( mainWindow_->setLogbook( file ) ) mainWindow_->checkLogbookBackup();
    else {

        // show information dialog, provided that fileList is not empty
        if( !file.isEmpty() )
        {
            const QString buffer = QString( tr( "Unable to open file '%1'." ) ).arg( file );
            InformationDialog( mainWindow_, buffer ).exec();
        }

        // create a default, empty logbook
        mainWindow_->createDefaultLogbook();

    }

    return true;

}

//____________________________________________
void Application::usage( void ) const
{
    _usage( "elogbook", tr( "[options] [file]" ) );
    commandLineParser().usage();
    return;
}

//_________________________________________________
void Application::_configuration( void )
{

    Debug::Throw( "Application::_configuration" );
    emit saveConfiguration();
    ConfigurationDialog dialog;
    connect( &dialog, SIGNAL(configurationChanged()), SIGNAL(configurationChanged()) );
    dialog.centerOnWidget( qApp->activeWindow() );
    dialog.exec();

}

//_________________________________________________
void Application::_exit( void )
{

    Debug::Throw( "Application::_exit.\n" );

    // ensure everything is saved properly
    if( mainWindow_ )
    {
        // check if editable EditionWindows needs save
        BASE::KeySet<EditionWindow> windows( mainWindow_ );
        foreach( EditionWindow* window, windows )
        {
            if( !( window->isReadOnly() || window->isClosed() ) && window->modified() && window->askForSave() == AskForSaveDialog::CANCEL )
            { return; }
        }

        Debug::Throw( "EditionWindows saved.\n" );

        // check if current logbook is modified
        if(
            mainWindow_->logbook() &&
            mainWindow_->logbook()->modified() &&
            mainWindow_->askForSave() == AskForSaveDialog::CANCEL )
        { return; }
    }

    qApp->quit();

}

//________________________________________________
bool Application::_processCommand( SERVER::ServerCommand command )
{

    Debug::Throw( "Application::_processCommand.\n" );
    if( BaseApplication::_processCommand( command ) ) return true;
    if( command.command() == SERVER::ServerCommand::Raise )
    {
        if( mainWindow_ ) mainWindow_->uniconifyAction().trigger();
        QStringList filenames( commandLineParser( command.arguments() ).orphans() );
        if( !filenames.isEmpty() )
        {

            const QString buffer = QString( tr( "Accept request for file '%1'?" ) ).arg( filenames.front() );
            if( QuestionDialog( mainWindow_, buffer ).centerOnParent().exec() )
            { mainWindow_->setLogbook( File( filenames.front() ) ); }

        }

        return true;
    } else return false;

}
