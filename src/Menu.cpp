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

#include <QMessageBox>


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

using namespace std;
using namespace Qt;

//_______________________________________________
Menu::Menu( QWidget* parent, MainWindow* mainwindow ):
    QMenuBar( parent ),
    Counter( "Menu" ),
    recent_entriesMenu_(0),
    editorMenu_(0),
    preferenceMenu_(0),
    recentFilesMenu_(0)
{

    Debug::Throw( "Menu::Menu.\n" );

    // try cast parent to EditionWindow
    EditionWindow* editionwindow( qobject_cast<EditionWindow*>( parent ) );

    // generic menu/action
    QMenu *menu;

    // file menu
    menu = addMenu( "&File" );
    menu->addAction( &mainwindow->newLogbookAction() );

    if( editionwindow )
    {
        menu->addAction( &editionwindow->newEntryAction() );
        menu->addAction( &editionwindow->splitViewHorizontalAction() );
    }

    menu->addAction( &mainwindow->openAction() );

    // file menu
    recentFilesMenu_ = new RecentFilesMenu( this, Singleton::get().application<Application>()->recentFiles() );
    connect( recentFilesMenu_, SIGNAL( fileSelected( FileRecord ) ), mainwindow, SLOT( open( FileRecord ) ) );
    menu->addMenu( recentFilesMenu_ );

    menu->addAction( &mainwindow->synchronizeAction() );
    menu->addAction( &mainwindow->reorganizeAction() );

    menu->addSeparator();
    if( editionwindow ) menu->addAction( &editionwindow->saveAction() );
    else menu->addAction( &mainwindow->saveAction() );

    menu->addAction( &mainwindow->saveAsAction() );
    menu->addAction( &mainwindow->saveBackupAction() );
    menu->addAction( &mainwindow->revertToSaveAction() );
    menu->addSeparator();

    if( editionwindow ) menu->addAction( &editionwindow->printAction() );
    else menu->addAction( &mainwindow->printAction() );

    if( editionwindow ) menu->addAction( &editionwindow->closeAction() );

    Application& application( *Singleton::get().application<Application>() );
    menu->addAction( &application.closeAction() );

    // edition menu
    if( parent == mainwindow )
    {
        menu = addMenu( "&Edit" );
        menu->addAction( &mainwindow->findEntriesAction() );
        menu->addSeparator();
        menu->addAction( &mainwindow->newKeywordAction() );
        menu->addAction( &mainwindow->editKeywordAction() );
        menu->addAction( &mainwindow->deleteKeywordAction() );
        menu->addSeparator();
        menu->addAction( &mainwindow->newEntryAction() );
        menu->addAction( &mainwindow->editEntryAction() );
        menu->addAction( &mainwindow->deleteEntryAction() );

        menu->addMenu( recent_entriesMenu_ = new QMenu( "Recent Entries" ) );
        connect( recent_entriesMenu_, SIGNAL( aboutToShow() ), this, SLOT( _updateRecentEntriesMenu() ) );
        connect( recent_entriesMenu_, SIGNAL( triggered( QAction* ) ), SLOT( _selectEntry( QAction* ) ) );

    }

    // Settings
    preferenceMenu_ = addMenu( "&Settings" );
    connect( preferenceMenu_, SIGNAL( aboutToShow() ), this, SLOT( _updatePreferenceMenu() ) );

    // windows menu
    editorMenu_ = addMenu( "&Window" );
    connect( editorMenu_, SIGNAL( aboutToShow() ), SLOT( _updateEditorMenu() ) );

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
    #ifdef DEBUG
    menu->addSeparator();
    DebugMenu *debug_menu( new DebugMenu( this ) );
    debug_menu->setTitle( "&Debug" );
    menu->addMenu( debug_menu );
    debug_menu->addAction( &mainwindow->saveForcedAction() );
    debug_menu->addAction( &mainwindow->showDuplicatesAction() );
    debug_menu->addAction( &help->dumpAction() );
    debug_menu->addAction( &mainwindow->monitoredFilesAction() );
    #endif

}

//_______________________________________________
void Menu::_updateRecentEntriesMenu( void )
{
    Debug::Throw( "Menu::_updateRecentEntriesMenu.\n" );

    assert( recent_entriesMenu_ );
    recent_entriesMenu_->clear();
    actions_.clear();

    MainWindow &mainwindow( Singleton::get().application<Application>()->mainWindow() );
    if( !mainwindow.logbook() ) return;

    std::vector<LogEntry*> entries( mainwindow.logbook()->recentEntries() );
    for( std::vector<LogEntry*>::const_iterator iter = entries.begin(); iter != entries.end(); iter++ )
    {
        QString buffer;
        QTextStream( &buffer ) << (*iter)->title() << " (" << (*iter)->keyword() << ")";
        QAction* action = recent_entriesMenu_->addAction( buffer );
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

    editorMenu_->clear();
    MainWindow &mainwindow( Singleton::get().application<Application>()->mainWindow() );
    AttachmentWindow &attachment_window( Singleton::get().application<Application>()->attachmentWindow() );

    // retrieve parent editFream if any
    EditionWindow* editionwindow = qobject_cast<EditionWindow*>( parentWidget() );

    // editor attachments and logbook information
    editorMenu_->addAction( &mainwindow.uniconifyAction() );

    editorMenu_->addAction( &attachment_window.uniconifyAction() );
    editorMenu_->addAction( &mainwindow.logbookStatisticsAction() );
    editorMenu_->addAction( &mainwindow.logbookInformationsAction() );

    if( editionwindow ) { editorMenu_->addAction( &editionwindow->entryInfoAction() ); }

    BASE::KeySet<EditionWindow> frames( mainwindow );
    bool has_alive_frame( find_if( frames.begin(), frames.end(), EditionWindow::aliveFTor() ) != frames.end() );
    if( has_alive_frame )
    {

        editorMenu_->addSeparator();
        for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
        {

            // ignore if frame is to be deleted
            if( (*iter)->isClosed() ) continue;

            // add menu entry for this frame
            QString title( (*iter)->windowTitle() );
            QAction* action = editorMenu_->addAction( IconEngine::get( ICONS::EDIT ), title, &(*iter)->uniconifyAction(), SLOT( trigger() ) );
            action->setCheckable( true );
            action->setChecked( editionwindow && ( editionwindow == (*iter) ) );

        }

        editorMenu_->addAction( &mainwindow.closeFramesAction() );

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
    MainWindow *mainwindow = qobject_cast<MainWindow*>( parentWidget() );
    if( mainwindow )
    {
        preferenceMenu_->addSeparator();
        preferenceMenu_->addAction( &mainwindow->keywordToolBar().visibilityAction() );
        preferenceMenu_->addAction( &mainwindow->entryToolBar().visibilityAction() );
        preferenceMenu_->addAction( &mainwindow->searchPanel().visibilityAction() );
    }

    // additional Settings in case parent is an edition frame
    EditionWindow *editionwindow = qobject_cast<EditionWindow*>( parentWidget() );
    if( editionwindow )
    {
        preferenceMenu_->addSeparator();
        preferenceMenu_->addAction( &editionwindow->attachmentFrame().visibilityAction() );
        preferenceMenu_->addAction( &editionwindow->activeEditor().showLineNumberAction() );
        preferenceMenu_->addAction( &editionwindow->activeEditor().wrapModeAction() );
        preferenceMenu_->addAction( &editionwindow->activeEditor().blockHighlightAction() );
    }

    preferenceMenu_->addSeparator();
    preferenceMenu_->addAction( &application.configurationAction() );

}
