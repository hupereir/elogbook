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

#include "Menu.h"

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
#include "IconNames.h"
#include "Logbook.h"
#include "MainWindow.h"
#include "RecentFilesMenu.h"
#include "QtUtil.h"
#include "Singleton.h"
#include "Util.h"
#include "XmlOptions.h"

//_______________________________________________
Menu::Menu( QWidget* parent, MainWindow* mainWindow ):
    QMenuBar( parent ),
    Counter( "Menu" )
{

    Debug::Throw( "Menu::Menu.\n" );

    // try cast parent to EditionWindow
    EditionWindow* editionWindow( qobject_cast<EditionWindow*>( parent ) );

    // generic menu/action
    QMenu *menu;

    // file menu
    menu = addMenu( tr( "File" ) );
    menu->addAction( &mainWindow->newLogbookAction() );

    if( editionWindow ) menu->addAction( &editionWindow->newEntryAction() );

    menu->addAction( &mainWindow->openAction() );

    // file menu
    recentFilesMenu_ = new RecentFilesMenu( this, Base::Singleton::get().application<Application>()->recentFiles() );
    connect( recentFilesMenu_, SIGNAL(fileSelected(FileRecord)), mainWindow, SLOT(open(FileRecord)) );
    menu->addMenu( recentFilesMenu_ );

    menu->addAction( &mainWindow->synchronizeAction() );
    menu->addAction( &mainWindow->reorganizeAction() );

    menu->addSeparator();
    if( editionWindow ) menu->addAction( &editionWindow->saveAction() );
    else menu->addAction( &mainWindow->saveAction() );

    menu->addAction( &mainWindow->saveAsAction() );
    menu->addAction( &mainWindow->saveBackupAction() );
    menu->addAction( &mainWindow->backupManagerAction() );
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

    auto application( Base::Singleton::get().application<Application>() );
    menu->addAction( &application->closeAction() );

    // edition menu
    if( parent == mainWindow )
    {
        menu = addMenu( tr( "Edit" ) );
        menu->addAction( &mainWindow->findEntriesAction() );
        menu->addSeparator();
        menu->addAction( &mainWindow->newKeywordAction() );
        menu->addAction( &mainWindow->editKeywordAction() );
        menu->addAction( &mainWindow->deleteKeywordAction() );
        menu->addSeparator();
        menu->addAction( &mainWindow->newEntryAction() );
        menu->addAction( &mainWindow->editEntryAction() );
        menu->addAction( &mainWindow->deleteEntryAction() );

        menu->addMenu( recentEntriesMenu_ = new QMenu( tr( "Recent Entries" ) ) );
        connect( recentEntriesMenu_, SIGNAL(aboutToShow()), SLOT(_updateRecentEntriesMenu()) );
        connect( recentEntriesMenu_, SIGNAL(triggered(QAction*)), SLOT(_selectEntry(QAction*)) );

    }

    // windows menu

    actionGroup_ = new QActionGroup( this );
    actionGroup_->setExclusive( true );

    windowsMenu_ = addMenu( tr( "Windows" ) );
    connect( windowsMenu_, SIGNAL(aboutToShow()), SLOT(_updateEditorMenu()) );

    // Settings
    preferenceMenu_ = addMenu( tr( "Settings" ) );
    connect( preferenceMenu_, SIGNAL(aboutToShow()), SLOT(_updatePreferenceMenu()) );

    // help manager
    Base::HelpManager* help( new Base::HelpManager( this ) );
    help->setWindowTitle( tr( "Elogbook Handbook" ) );
    help->install( helpText );
    help->install( Base::helpText, false );

    // help menu
    menu = addMenu( tr( "Help" ) );
    menu->addAction( &help->displayAction() );
    menu->addSeparator();
    menu->addAction( &application->aboutAction() );
    menu->addAction( &application->aboutQtAction() );

    // debug menu
    menu->addSeparator();
    DebugMenu *debugMenu( new DebugMenu( menu ) );
    menu->addAction( debugMenu->menuAction() );
    debugMenu->menuAction()->setVisible( false );

    debugMenu->addAction( &mainWindow->saveForcedAction() );
    debugMenu->addAction( &mainWindow->showDuplicatesAction() );
    debugMenu->addAction( &mainWindow->monitoredFilesAction() );

}

//_______________________________________________
void Menu::_updateRecentEntriesMenu()
{
    Debug::Throw( "Menu::_updateRecentEntriesMenu.\n" );

    Q_CHECK_PTR( recentEntriesMenu_ );
    recentEntriesMenu_->clear();
    actions_.clear();

    MainWindow &mainWindow( Base::Singleton::get().application<Application>()->mainWindow() );
    if( !mainWindow.logbook() ) return;

    for( const auto& entry:mainWindow.logbook()->recentEntries() )
    {
        actions_.insert(  entry->hasKeywords() ?
            recentEntriesMenu_->addAction( QString( "%1 (%2)" ).arg( entry->title(), entry->keywords().begin()->get() ) ):
            recentEntriesMenu_->addAction( QString( "%1 (%2)" ).arg( entry->title(), Keyword::Default.get() ) ),
            entry );
    }

}

//_______________________________________________
void Menu::_selectEntry( QAction* action )
{
    Debug::Throw( "Menu::_selectEntry.\n" );
    ActionMap::iterator iter( actions_.find( action ) );
    Q_ASSERT( iter != actions_.end() );
    iter.value()->setFindSelected( true );
    emit entrySelected( iter.value() );
}

//_______________________________________________
void Menu::_updateEditorMenu()
{
    Debug::Throw( "Menu::_UpdateEditorMenu.\n" );

    windowsMenu_->clear();
    // retrieve parent editFream if any
    EditionWindow* editionWindow = qobject_cast<EditionWindow*>( parentWidget() );

    MainWindow &mainWindow( Base::Singleton::get().application<Application>()->mainWindow() );
    AttachmentWindow &attachmentWindow( Base::Singleton::get().application<Application>()->attachmentWindow() );

    // editor attachments and logbook information
    if( editionWindow ) { windowsMenu_->addAction( &mainWindow.uniconifyAction() ); }

    windowsMenu_->addAction( &attachmentWindow.uniconifyAction() );
    windowsMenu_->addAction( &mainWindow.logbookStatisticsAction() );
    windowsMenu_->addAction( &mainWindow.logbookInformationsAction() );

    if( editionWindow ) { windowsMenu_->addAction( &editionWindow->entryInformationAction() ); }

    Base::KeySet<EditionWindow> windows( mainWindow );
    bool hasAliveFrame( std::any_of( windows.begin(), windows.end(), EditionWindow::AliveFTor() ) );
    if( hasAliveFrame )
    {

        windowsMenu_->addSeparator();
        for( const auto& window:windows )
        {

            // ignore if frame is to be deleted
            if( window->isClosed() ) continue;

            // add menu entry for this frame
            QString title( window->windowTitle() );
            QAction* action = windowsMenu_->addAction( IconEngine::get( IconNames::Edit ), title, &window->uniconifyAction(), SLOT(trigger()) );

            if( editionWindow )
            {
                action->setCheckable( true );
                action->setChecked( editionWindow && ( editionWindow == window ) );
            }

            actionGroup_->addAction( action );

        }

        windowsMenu_->addSeparator();
        windowsMenu_->addAction( &mainWindow.closeFramesAction() );

    }

    return;
}


//_______________________________________________
void Menu::_updatePreferenceMenu()
{

    Debug::Throw( "Menu::_updatePreferenceMenu.\n" );

    preferenceMenu_->clear();

    // Settings menu
    auto application( Base::Singleton::get().application<Application>() );

    // additional Settings in case parent is a selection frame
    auto mainWindow = qobject_cast<MainWindow*>( parentWidget() );
    if( mainWindow ) preferenceMenu_->addAction( &mainWindow->treeModeAction() );

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
    preferenceMenu_->addAction( &application->configurationAction() );

}
