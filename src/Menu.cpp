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

/*!
\file Menu.cpp
\brief  main menu
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "Application.h"
#include "AttachmentWindow.h"
#include "CustomToolBar.h"
#include "DebugMenu.h"
#include "DefaultHelpText.h"
#include "EditionWindow.h"
#include "File.h"
#include "HelpManager.h"
#include "HelpText.h"
#include "IconEngine.h"
#include "Icons.h"
#include "Logbook.h"
#include "MainWindow.h"
#include "Menu.h"
#include "RecentFilesMenu.h"
#include "QtUtil.h"
#include "SearchPanel.h"
#include "Singleton.h"
#include "Str.h"
#include "Util.h"
#include "XmlOptions.h"

#include <QtGui/QMessageBox>

//_______________________________________________
Menu::Menu( QWidget* parent, MainWindow* mainWindow ):
    QMenuBar( parent ),
    Counter( "Menu" ),
    recentEntriesMenu_(0),
    windowsMenu_(0),
    preferenceMenu_(0),
    recentFilesMenu_(0)
{

    Debug::Throw( "Menu::Menu.\n" );

    // try cast parent to EditionWindow
    EditionWindow* editionWindow( qobject_cast<EditionWindow*>( parent ) );

    // generic menu/action
    QMenu *menu;

    // file menu
    menu = addMenu( "File" );
    menu->addAction( &mainWindow->newLogbookAction() );

    if( editionWindow )
    {
        menu->addAction( &editionWindow->newEntryAction() );
        menu->addAction( &editionWindow->splitViewHorizontalAction() );
    }

    menu->addAction( &mainWindow->openAction() );

    // file menu
    recentFilesMenu_ = new RecentFilesMenu( this, Singleton::get().application<Application>()->recentFiles() );
    connect( recentFilesMenu_, SIGNAL( fileSelected( FileRecord ) ), mainWindow, SLOT( open( FileRecord ) ) );
    menu->addMenu( recentFilesMenu_ );

    menu->addAction( &mainWindow->synchronizeAction() );
    menu->addAction( &mainWindow->reorganizeAction() );

    menu->addSeparator();
    if( editionWindow ) menu->addAction( &editionWindow->saveAction() );
    else menu->addAction( &mainWindow->saveAction() );

    menu->addAction( &mainWindow->saveAsAction() );
    menu->addAction( &mainWindow->saveBackupAction() );
    menu->addAction( &mainWindow->revertToSaveAction() );
    menu->addSeparator();

    if( editionWindow )
    {

        menu->addAction( &editionWindow->printAction() );
        menu->addAction( &editionWindow->printPreviewAction() );
        menu->addAction( &editionWindow->htmlAction() );

    } else {

        menu->addAction( &mainWindow->printAction() );
        menu->addAction( &mainWindow->printPreviewAction() );
        menu->addAction( &mainWindow->htmlAction() );

    }

    menu->addSeparator();
    if( editionWindow ) menu->addAction( &editionWindow->closeAction() );

    Application& application( *Singleton::get().application<Application>() );
    menu->addAction( &application.closeAction() );

    // edition menu
    if( parent == mainWindow )
    {
        menu = addMenu( "Edit" );
        menu->addAction( &mainWindow->findEntriesAction() );
        menu->addSeparator();
        menu->addAction( &mainWindow->newKeywordAction() );
        menu->addAction( &mainWindow->editKeywordAction() );
        menu->addAction( &mainWindow->deleteKeywordAction() );
        menu->addSeparator();
        menu->addAction( &mainWindow->newEntryAction() );
        menu->addAction( &mainWindow->editEntryAction() );
        menu->addAction( &mainWindow->deleteEntryAction() );

        menu->addMenu( recentEntriesMenu_ = new QMenu( "Recent Entries" ) );
        connect( recentEntriesMenu_, SIGNAL( aboutToShow() ), this, SLOT( _updateRecentEntriesMenu() ) );
        connect( recentEntriesMenu_, SIGNAL( triggered( QAction* ) ), SLOT( _selectEntry( QAction* ) ) );

    }

    // windows menu
    windowsMenu_ = addMenu( "Windows" );
    connect( windowsMenu_, SIGNAL( aboutToShow() ), SLOT( _updateEditorMenu() ) );

    // Settings
    preferenceMenu_ = addMenu( "Settings" );
    connect( preferenceMenu_, SIGNAL( aboutToShow() ), this, SLOT( _updatePreferenceMenu() ) );

    // help manager
    BASE::HelpManager* help( new BASE::HelpManager( this ) );
    help->setWindowTitle( "Elogbook Handbook" );
    File help_file( XmlOptions::get().raw( "HELP_FILE" ) );
    if( help_file.exists() ) help->install( help_file );
    else {
        help->setFile( help_file );
        help->install( helpText );
        help->install( BASE::helpText, false );
    }

    // help menu
    menu = addMenu( "&Help" );
    menu->addAction( &help->displayAction() );
    menu->addSeparator();
    menu->addAction( &application.aboutAction() );
    menu->addAction( &application.aboutQtAction() );

    // debug menu
    //#ifdef DEBUG
    menu->addSeparator();
    DebugMenu *debug_menu( new DebugMenu( this ) );
    debug_menu->setTitle( "Debug" );
    menu->addMenu( debug_menu );
    debug_menu->addAction( &mainWindow->saveForcedAction() );
    debug_menu->addAction( &mainWindow->showDuplicatesAction() );
    debug_menu->addAction( &help->dumpAction() );
    debug_menu->addAction( &mainWindow->monitoredFilesAction() );
    //#endif

}

//_______________________________________________
void Menu::_updateRecentEntriesMenu( void )
{
    Debug::Throw( "Menu::_updateRecentEntriesMenu.\n" );

    assert( recentEntriesMenu_ );
    recentEntriesMenu_->clear();
    actions_.clear();

    MainWindow &mainWindow( Singleton::get().application<Application>()->mainWindow() );
    if( !mainWindow.logbook() ) return;

    std::vector<LogEntry*> entries( mainWindow.logbook()->recentEntries() );
    for( std::vector<LogEntry*>::const_iterator iter = entries.begin(); iter != entries.end(); ++iter )
    {
        QString buffer;
        QTextStream( &buffer ) << (*iter)->title() << " (" << (*iter)->keyword() << ")";
        QAction* action = recentEntriesMenu_->addAction( buffer );
        actions_.insert( std::make_pair( action, (*iter) ) );
    }

}

//_______________________________________________
void Menu::_selectEntry( QAction* action )
{
    Debug::Throw( "Menu::_selectEntry.\n" );
    ActionMap::iterator iter( actions_.find( action ) );
    assert( iter != actions_.end() );
    iter->second->setFindSelected( true );
    emit entrySelected( iter->second );
}

//_______________________________________________
void Menu::_updateEditorMenu( void )
{
    Debug::Throw( "Menu::_UpdateEditorMenu.\n" );

    windowsMenu_->clear();
    // retrieve parent editFream if any
    EditionWindow* editionWindow = qobject_cast<EditionWindow*>( parentWidget() );

    MainWindow &mainWindow( Singleton::get().application<Application>()->mainWindow() );
    AttachmentWindow &attachmentWindow( Singleton::get().application<Application>()->attachmentWindow() );

    // editor attachments and logbook information
    if( editionWindow ) { windowsMenu_->addAction( &mainWindow.uniconifyAction() ); }

    windowsMenu_->addAction( &attachmentWindow.uniconifyAction() );
    windowsMenu_->addAction( &mainWindow.logbookStatisticsAction() );
    windowsMenu_->addAction( &mainWindow.logbookInformationsAction() );

    if( editionWindow ) { windowsMenu_->addAction( &editionWindow->entryInfoAction() ); }

    BASE::KeySet<EditionWindow> frames( mainWindow );
    bool hasAliveFrame( find_if( frames.begin(), frames.end(), EditionWindow::aliveFTor() ) != frames.end() );
    if( hasAliveFrame )
    {

        windowsMenu_->addSeparator();
        for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); ++iter )
        {

            // ignore if frame is to be deleted
            if( (*iter)->isClosed() ) continue;

            // add menu entry for this frame
            QString title( (*iter)->windowTitle() );
            QAction* action = windowsMenu_->addAction( IconEngine::get( ICONS::EDIT ), title, &(*iter)->uniconifyAction(), SLOT( trigger() ) );
            action->setCheckable( true );
            action->setChecked( editionWindow && ( editionWindow == (*iter) ) );

        }

        windowsMenu_->addAction( &mainWindow.closeFramesAction() );

    }

    return;
}


//_______________________________________________
void Menu::_updatePreferenceMenu( void )
{

    Debug::Throw( "Menu::_updatePreferenceMenu.\n" );

    preferenceMenu_->clear();

    // Settings menu
    Application& application( *Singleton::get().application<Application>() );

    // additional Settings in case parent is a selection frame
    MainWindow *mainWindow = qobject_cast<MainWindow*>( parentWidget() );
    if( mainWindow )
    {
        preferenceMenu_->addAction( &mainWindow->treeModeAction() );
        preferenceMenu_->addSeparator();
        preferenceMenu_->addAction( &mainWindow->keywordToolBar().visibilityAction() );
        preferenceMenu_->addAction( &mainWindow->entryToolBar().visibilityAction() );
        preferenceMenu_->addAction( &mainWindow->searchPanel().visibilityAction() );
    }

    // additional Settings in case parent is an edition frame
    EditionWindow *editionWindow = qobject_cast<EditionWindow*>( parentWidget() );
    if( editionWindow )
    {
        preferenceMenu_->addSeparator();
        preferenceMenu_->addAction( &editionWindow->showKeywordAction() );
        preferenceMenu_->addAction( &editionWindow->attachmentFrame().visibilityAction() );
        preferenceMenu_->addAction( &editionWindow->activeEditor().showLineNumberAction() );
        preferenceMenu_->addAction( &editionWindow->activeEditor().wrapModeAction() );
        preferenceMenu_->addAction( &editionWindow->activeEditor().blockHighlightAction() );
    }

    preferenceMenu_->addSeparator();
    preferenceMenu_->addAction( &application.configurationAction() );

}
