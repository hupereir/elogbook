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

#include "MainWindow.h"

#include "Application.h"
#include "AttachmentWindow.h"
#include "BackupManagerDialog.h"
#include "BackupManagerWidget.h"
#include "BaseIconNames.h"
#include "ColorMenu.h"
#include "Command.h"
#include "ContextMenu.h"
#include "ToolBar.h"
#include "Debug.h"
#include "DeleteKeywordDialog.h"
#include "EditionWindow.h"
#include "EditKeywordDialog.h"
#include "FileDialog.h"
#include "FileCheckDialog.h"
#include "FileList.h"
#include "HtmlDialog.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "InformationDialog.h"
#include "LineEditor.h"
#include "Logbook.h"
#include "LogbookHtmlHelper.h"
#include "LogbookInformationDialog.h"
#include "LogbookModifiedDialog.h"
#include "LogbookStatisticsDialog.h"
#include "LogbookPrintOptionWidget.h"
#include "LogbookPrintHelper.h"
#include "LogEntryInformationDialog.h"
#include "LogEntryPrintOptionWidget.h"
#include "LogEntryPrintSelectionDialog.h"
#include "LogEntryPrintSelectionWidget.h"
#include "MenuBar.h"
#include "NewLogbookDialog.h"
#include "PrinterOptionWidget.h"
#include "PrintPreviewDialog.h"
#include "ProgressBar.h"
#include "ProgressStatusBar.h"
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "RecentFilesMenu.h"
#include "SearchWidget.h"
#include "Singleton.h"
#include "TextEditionDelegate.h"
#include "Util.h"
#include "WarningDialog.h"
#include "XmlOptions.h"

#include <QHeaderView>
#include <QMenu>
#include <QPrintDialog>
#include <QSplitter>

//_____________________________________________
MainWindow::MainWindow( QWidget *parent ):
    BaseMainWindow( parent ),
    Counter( QStringLiteral("MainWindow") ),
    workingDirectory_( Util::workingDirectory() )
{
    Debug::Throw( QStringLiteral("MainWindow::MainWindow.\n") );
    setOptionName( QStringLiteral("MAIN_WINDOW") );
    updateWindowTitle();

    // file checker
    fileCheck_ = new FileCheck( this );
    connect( fileCheck_, &FileCheck::filesModified, this, &MainWindow::_filesModified );

    // main widget
    auto main = new QWidget( this );
    setCentralWidget( main );

    // local layout
    auto layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(2);
    main->setLayout( layout );

    // splitter for KeywordList/LogEntryList
    auto splitter( new QSplitter( main ) );
    layout->addWidget( splitter, 1 );
    splitter->setOrientation( Qt::Horizontal );

    // search widget
    layout->addWidget( searchWidget_ = new SearchWidget( main ) );
    searchWidget_->hide();

    connect( searchWidget_, &SearchWidget::selectEntries, this, &MainWindow::selectEntries );
    connect( searchWidget_, &SearchWidget::showAllEntries, this, &MainWindow::showAllEntries );

    // status bar
    setStatusBar( statusbar_ = new ProgressStatusBar( this ) );
    statusbar_->setProgressBar( new ProgressBar );
    statusbar_->addClock();
    connect( this, &MainWindow::messageAvailable, &statusbar_->label(), &StatusBarLabel::setTextAndUpdate );
    connect( this, &MainWindow::messageAvailable, static_cast<ProgressBar*>(&statusbar_->progressBar()), &ProgressBar::setText );

    // global scope actions
    _installActions();

    // aditional actions from application
    auto application( Base::Singleton::get().application<Application>() );
    addAction( &application->closeAction() );

    // Keyword container
    keywordContainer_ = new QWidget;

    // set layout
    auto vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    keywordContainer_->setLayout( vLayout );

    keywordToolBar_ = new ToolBar( tr( "Keywords" ), keywordContainer_, QStringLiteral("KEYWORD_TOOLBAR") );
    keywordToolBar_->setTransparent( true );
    keywordToolBar_->setAppearsInMenu( true );
    vLayout->addWidget( keywordToolBar_ );

    // keyword actions
    keywordToolBar_->addAction( newKeywordAction_ );
    keywordToolBar_->addAction( deleteKeywordAction_ );
    keywordToolBar_->addAction( editKeywordAction_ );
    Debug::Throw() << "MainWindow::MainWindow - keyword toolbar created." << endl;

    // create keyword list
    vLayout->addWidget( keywordList_ = new KeywordList( keywordContainer_ ), 1 );
    keywordList_->setFindEnabled( false );
    keywordList_->setModel( &keywordModel_ );
    keywordList_->setRootIsDecorated( true );
    keywordList_->setSortingEnabled( true );
    keywordList_->setDragEnabled(true);
    keywordList_->setAcceptDrops(true);
    keywordList_->setDropIndicatorShown(true);
    keywordList_->setOptionName( QStringLiteral("KEYWORD_LIST") );

    // default width from options, if found
    if( XmlOptions::get().contains( QStringLiteral("KEYWORD_LIST_WIDTH") ) )
    { keywordList_->setDefaultWidth( XmlOptions::get().get<int>( QStringLiteral("KEYWORD_LIST_WIDTH") ) ); }

    // replace item delegate
    if( keywordList_->itemDelegate() ) keywordList_->itemDelegate()->deleteLater();
    keywordList_->setItemDelegate( new TextEditionDelegate( this ) );

    // update LogEntryList when keyword selection change
    connect( keywordList_->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::_keywordSelectionChanged );
    connect( keywordList_->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::_updateKeywordActions );
    connect( keywordList_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::_updateKeywordActions );
    _updateKeywordActions();

    // rename selected entries when KeywordChanged is emitted with a single argument.
    // this correspond to drag and drop action from the logEntryList in the KeywordList
    connect( &keywordModel_, &KeywordModel::entryKeywordChangeRequest, this, &MainWindow::_confirmRenameEntryKeyword );

    // rename all entries matching first keyword the second. This correspond to
    // drag and drop inside the keyword list, or to direct edition of a keyword list item.
    connect( &keywordModel_, &KeywordModel::keywordChangeRequest, this, &MainWindow::_confirmRenameKeyword );
    connect( &keywordModel_, &KeywordModel::keywordChanged, this, QOverload<const Keyword&, const Keyword&>::of( &MainWindow::_renameKeyword ) );

    {
        // popup menu for keyword list
        auto menu = new ContextMenu( keywordList_ );
        menu->addAction( newEntryAction_ );
        menu->addAction( newKeywordAction_ );
        menu->addSeparator();
        menu->addAction( deleteKeywordAction_ );
        menu->addAction( editKeywordAction_ );
        menu->setHideDisabledActions( true );
    }

    /*
    add the deleteKeywordAction to the keyword list,
    so that the corresponding shortcut gets activated whenever it is pressed
    while the list has focus
    */
    keywordList_->addAction( deleteKeywordAction_ );
    keywordList_->addAction( editKeywordAction_ );

    // right box for entries and buttons
    auto right = new QWidget;

    vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    right->setLayout( vLayout );

    entryToolBar_ = new ToolBar( tr( "Entries" ), right, QStringLiteral("ENTRY_TOOLBAR") );
    entryToolBar_->setTransparent( true );
    entryToolBar_->setAppearsInMenu( true );
    vLayout->addWidget( entryToolBar_ );

    // entry actions
    entryToolBar_->addAction( newEntryAction_ );
    entryToolBar_->addAction( editEntryAction_ );

    // need to use a button to be able to set the popup mode
    entryColorButton_ = new QToolButton;
    entryColorButton_->setText( tr( "Entry Color" ) );
    entryColorButton_->setIcon( IconEngine::get( IconNames::Color ) );
    entryColorButton_->setPopupMode( QToolButton::InstantPopup );
    entryColorButton_->setMenu( colorMenu_ );
    entryToolBar_->addWidget( entryColorButton_ );

    entryToolBar_->addAction( deleteEntryAction_ );
    entryToolBar_->addAction( saveAction_ );
    entryToolBar_->addAction( printAction_ );

    // create logEntry list
    vLayout->addWidget( entryList_ = new LogEntryList( right ), 1 );
    entryList_->setFindEnabled( false );
    entryList_->setModel( &entryModel_ );
    entryList_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    entryList_->setDragEnabled(true);
    entryList_->setOptionName( QStringLiteral("ENTRY_LIST") );
    entryList_->setColumnHidden( LogEntryModel::Key, true );
    entryList_->lockColumnVisibility( LogEntryModel::Key );
    entryList_->setColumnHidden( LogEntryModel::Title, false );
    entryList_->lockColumnVisibility( LogEntryModel::Title );

    entryList_->header()->setSectionResizeMode(LogEntryModel::Creation, QHeaderView::Stretch);
    entryList_->header()->setSectionResizeMode(LogEntryModel::Modification, QHeaderView::Stretch);

    // replace item delegate
    if( entryList_->itemDelegate() ) entryList_->itemDelegate()->deleteLater();
    entryList_->setItemDelegate( new TextEditionDelegate( this ) );

    connect( entryList_->header(), &QHeaderView::sortIndicatorChanged, this, QOverload<int, Qt::SortOrder>::of( &MainWindow::_storeSortMethod ) );
    connect( entryList_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainWindow::_updateEntryActions );
    connect( entryList_, &QAbstractItemView::activated, this, &MainWindow::_entryItemActivated );
    connect( entryList_, &QAbstractItemView::clicked, this, &MainWindow::_entryItemClicked );
    _updateEntryActions();

    connect( &entryModel_, &LogEntryModel::layoutChanged, entryList_, QOverload<>::of( &LogEntryList::resizeColumns ) );
    connect( &entryModel_, &LogEntryModel::dataChanged, this, &MainWindow::_entryDataChanged );

    /*
    add the deleteEntryAction to the list,
    so that the corresponding shortcut gets activated whenever it is pressed
    while the list has focus
    */
    entryList_->addAction( deleteEntryAction_ );
    entryList_->addAction( editEntryTitleAction_ );

    {
        // popup menu for list
        auto menu = new ContextMenu( entryList_ );
        menu->addAction( newEntryAction_ );
        menu->addSeparator();
        menu->addAction( editEntryTitleAction_ );
        menu->addAction( editEntryAction_ );
        menu->addAction( entryKeywordAction_ );
        menu->addAction( deleteEntryAction_ );
        menu->addAction( entryColorAction_ );
        menu->addSeparator();
        menu->addAction( entryInformationAction_ );
        menu->setHideDisabledActions( true );
    }

    // add widgets to splitter
    splitter->addWidget( keywordContainer_ );
    splitter->addWidget( right );

    // assign stretch factors
    splitter->setStretchFactor( 0, 0 );
    splitter->setStretchFactor( 1, 1 );

    connect( splitter, &QSplitter::splitterMoved, this, &MainWindow::_splitterMoved );

    // main menu
    menuBar_ = new MenuBar( this , this );
    setMenuBar( menuBar_ );
    connect( menuBar_, &MenuBar::entrySelected, this, QOverload<LogEntry*>::of( &MainWindow::selectEntry ) );
    connect( menuBar_, &MenuBar::entrySelected, this, &MainWindow::_displayEntry );

    // configuration
    connect( application, &Application::configurationChanged, this, &MainWindow::_updateConfiguration );
    _updateConfiguration();
    _updateKeywordActions();
    _updateEntryActions();
    _updateReadOnlyState();
}

//___________________________________________________________
void MainWindow::createDefaultLogbook()
{

    Debug::Throw( QStringLiteral("MainWindow::_newLogbook.\n") );

    // check current logbook
    if( logbookIsModified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create a new logbook, with no file
    setLogbook( File() );

    logbook_->setTitle(  Logbook::NoTitle );
    logbook_->setAuthor( XmlOptions::get().raw( QStringLiteral("USER") ) );
    logbook_->setDirectory( workingDirectory() );

    logbook_->setComments( tr( "Default logbook created automatically on %1" ).arg( TimeStamp::now().toString( TimeStamp::Format::Long ) ) );

}

//_______________________________________________
bool MainWindow::setLogbook( const File &file )
{

    Debug::Throw() << "MainWindow::SetLogbook - logbook: \"" << file << "\"" << endl;

    // reset current logbook
    if( logbook_ ) reset();

    // clear file checker
    fileCheck_->clear();

    // create new logbook
    logbook_.reset( new Logbook );
    logbook_->setUseCompression( XmlOptions::get().get<bool>( QStringLiteral("USE_COMPRESSION") ) );

    // if filename is empty, return
    if( file.isEmpty() )
    {

        // update listView with new entries
        _resetKeywordList();
        _resetLogEntryList();
        emit ready();
        return false;

    } else {

        // save in menu
        menuBar_->recentFilesMenu().setCurrentFile( file );

    }

    // set file
    logbook_->setFile( file );
    if( !file.exists() )
    {
        // update listView with new entries
        _resetKeywordList();
        _resetLogEntryList();
        emit ready();
        return false;
    }

    connect( logbook_.get(), &Logbook::maximumProgressAvailable, statusbar_, &ProgressStatusBar::showProgressBar );
    connect( logbook_.get(), &Logbook::maximumProgressAvailable, &statusbar_->progressBar(), &QProgressBar::setMaximum );
    connect( logbook_.get(), &Logbook::progressAvailable, static_cast<ProgressBar*>(&statusbar_->progressBar()), &ProgressBar::addToProgress );
    connect( logbook_.get(), &Logbook::messageAvailable, this, &MainWindow::messageAvailable );

    connect( logbook_.get(), &Logbook::readOnlyChanged, this, &MainWindow::_updateEntryActions );
    connect( logbook_.get(), &Logbook::readOnlyChanged, this, &MainWindow::_updateKeywordActions );
    connect( logbook_.get(), &Logbook::readOnlyChanged, this, &MainWindow::_updateReadOnlyState );

    // one need to disable everything in the window
    // to prevent user to interact with the application while loading
    _setEnabled( false );
    logbook_->read();
    _setEnabled( true );

    Debug::Throw( QStringLiteral("MainWindow::setLogbook - finished reading.\n") );

    // update listView with new entries
    _resetKeywordList();
    _resetLogEntryList();
    _loadColors();

    Debug::Throw( QStringLiteral("MainWindow::setLogbook - lists set.\n") );

    // change sorting
    Qt::SortOrder sortOrder( (Qt::SortOrder) logbook_->sortOrder() );
    Debug::Throw( QStringLiteral("MainWindow::setLogbook - got sort order.\n") );

    switch( logbook_->sortMethod() )
    {
        case Logbook::SortMethod::SortColor: entryList_->sortByColumn( LogEntryModel::Color, sortOrder ); break;
        case Logbook::SortMethod::SortTitle: entryList_->sortByColumn( LogEntryModel::Title, sortOrder ); break;
        case Logbook::SortMethod::SortCreation: entryList_->sortByColumn( LogEntryModel::Creation, sortOrder ); break;
        case Logbook::SortMethod::SortModification: entryList_->sortByColumn( LogEntryModel::Modification , sortOrder); break;
        case Logbook::SortMethod::SortAuthor: entryList_->sortByColumn( LogEntryModel::Author, sortOrder ); break;
        default: break;
    }

    Debug::Throw( QStringLiteral("MainWindow::setLogbook - lists sorted.\n") );

    // update attachment frame
    resetAttachmentWindow();
    Debug::Throw( QStringLiteral("MainWindow::setLogbook - attachment frame reset.\n") );

    // retrieve last modified entry
    Base::KeySet<LogEntry> entries( logbook_->entries() );
    if( !entries.empty() )
    { selectEntry( *std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() ) ); }

    entryList_->setFocus();

    Debug::Throw( QStringLiteral("MainWindow::setLogbook - entry selected.\n") );

    // see if logbook has parent file
    if( !logbook_->parentFile().isEmpty() )
    {
        const QString buffer = tr( "Warning: this logbook should be oppened via '%1' only." ).arg( logbook_->parentFile() );
        WarningDialog( this, buffer ).exec();
    }

    // see if logbook is read-only
    if( logbook_->isBackup() )
    {

        auto buffer(
            tr( "Warning: this logbook is a backup and is therefore read-only.\n"
            "All editing will be disabled until it is marked as writable again "
            "in the Logbook Information dialog." ) );

        WarningDialog( this, buffer ).exec();

    } else if( logbook_->isReadOnly() ) {

        auto buffer(
            tr( "Warning: this logbook is read-only.\n"
            "All editing will be disabled until it is marked as writable again "
            "in the Logbook Information dialog." ) );

        WarningDialog( this, buffer ).exec();

    }

    // cleanup
    // make sure top-level logbook has no associated entries
    entries = Base::KeySet<LogEntry>(logbook_.get());
    if( !entries.empty() )
    {

        Debug::Throw(0) << "MainWindow::setLogbook - moving " << entries.size() << " entries" << endl;
        for( const auto& entry : entries )
        {
            // dissassociate
            Base::Key::disassociate( entry, logbook_.get() );

            // reassociate to latest child
            auto child = logbook_->latestChild();
            child->setModified(true);
            Base::Key::associate( entry, child.get() );
        }

        logbook_->setModified(true);
        if( !logbook_->file().isEmpty() ) save();
    }


    // store logbook directory for next open, save comment
    workingDirectory_ = File( logbook_->file() ).path();
    statusbar_->label().clear();
    statusbar_->showLabel();

    // register logbook to fileCheck
    fileCheck_->registerLogbook( logbook_.get() );

    emit ready();

    // check errors
    auto errors( logbook_->xmlErrors() );
    if( errors.size() )
    {
        QString buffer( errors.size() > 1 ? tr( "Errors occured while parsing files.\n" ):tr("An error occured while parsing files.\n") );
        buffer += XmlError::toString( errors );
        InformationDialog( nullptr, buffer ).exec();
    }

    // add opened file to OpenPrevious mennu.
    if( !logbook_->file().isEmpty() )
    { Base::Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expanded() ); }

    ignoreWarnings_ = false;

    _updateKeywordActions();
    _updateEntryActions();
    _updateReadOnlyState();

    updateWindowTitle();
    return true;
}

//_____________________________________________
void MainWindow::checkLogbookBackup()
{
    Debug::Throw( QStringLiteral("MainWindow::checkLogbookBackup.\n") );

    // check logbook makes sense
    if( !logbook_ ) return;

    // check if oppened logbook needs backup
    if(
        XmlOptions::get().get<bool>( QStringLiteral("AUTO_BACKUP") ) &&
        !logbook_->file().isEmpty() &&
        !logbook_->isReadOnly() &&
        logbook_->needsBackup() )
    {

        // ask if backup needs to be saved; save if yes
        if( QuestionDialog( this, tr( "Current logbook needs backup. Make one?" ) ).exec() )
        { _saveBackup(); }

    }

    return;
}

//_____________________________________________
void MainWindow::reset()
{

    Debug::Throw( QStringLiteral("MainWindow::reset.\n") );
    if( logbook_ ) logbook_.reset();

    // clear list of entries
    keywordModel_.clear();
    entryModel_.clear();

    // clear the AttachmentWindow
    Base::Singleton::get().application<Application>()->attachmentWindow().frame().clear();

    // make all EditionWindows for deletion
    for( const auto& window:Base::KeySet<EditionWindow>( this ) )
    {
        window->setIsClosed( true );
        window->hide();
    }

    return;

}

//____________________________________________
AskForSaveDialog::ReturnCode MainWindow::checkModifiedEntries()
{ return _checkModifiedEntries( Base::KeySet<EditionWindow>(this) ); }

//____________________________________________
AskForSaveDialog::ReturnCode MainWindow::askForSave( bool enableCancel )
{

    Debug::Throw( QStringLiteral("MainWindow::askForSave.\n") );

    // create dialog
    AskForSaveDialog::ReturnCodes buttons( AskForSaveDialog::Yes | AskForSaveDialog::No );
    if( enableCancel ) buttons |= AskForSaveDialog::Cancel;

    // exec and check return code
    auto reply = AskForSaveDialog( this, tr( "Logbook has been modified. Save ?" ), buttons ).centerOnParent().exec();
    if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    return AskForSaveDialog::ReturnCode(reply);

}

//_______________________________________________
void MainWindow::clearSelection()
{ entryList_->clearSelection(); }

//_______________________________________________
void MainWindow::selectEntry( LogEntry* entry )
{
    Debug::Throw(QStringLiteral("MainWindow::selectEntry.\n") );

    if( !entry ) return;

    // select entry keyword
    if( !entry->keywords().contains( currentKeyword() ) )
    {
        const auto index = keywordModel_.index( entry->hasKeywords() ?  *entry->keywords().begin():Keyword::Default );
        keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->scrollTo( index );
    }

    const auto index = entryModel_.index( entry );
    entryList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    entryList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    entryList_->scrollTo( index );
    return;

}

//_______________________________________________
void MainWindow::selectEntry( const Keyword& keyword, LogEntry* entry )
{
    Debug::Throw(QStringLiteral("MainWindow::selectEntry.\n") );

    if( !entry ) return;
    if( !entry->keywords().contains( keyword ) )
    { return selectEntry( entry ); }

    // select entry keyword
    if( !(keyword == currentKeyword() ) )
    {
        const auto index = keywordModel_.index( keyword );
        keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->scrollTo( index );
    }

    const auto index = entryModel_.index( entry );
    entryList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    entryList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    entryList_->scrollTo( index );
    return;

}

//_______________________________________________
void MainWindow::updateEntry( const Keyword &keyword, LogEntry* entry, bool updateSelection )
{

    Debug::Throw( QStringLiteral("MainWindow::updateEntry.\n") );

    // make sure keyword model contains all entry keywords
    keywordModel_.add( Base::makeT<KeywordModel::List>(entry->keywords()) );

    // update keyword model if needed
    if( keyword != currentKeyword() )
    {

        // update keyword model
        QModelIndex index = keywordModel_.index( keyword );
        keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );

    }

    // umdate logEntry model
    entryModel_.add( entry );

    // select
    if( updateSelection )
    {
        QModelIndex index( entryModel_.index( entry ) );
        entryList_->selectionModel()->select( index, QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        entryList_->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current|QItemSelectionModel::Rows );
    }

}


//_______________________________________________
void MainWindow::deleteEntry( LogEntry* entry, bool save )
{
    Debug::Throw( QStringLiteral("MainWindow::deleteEntry.\n") );

    // check entry
    if( !entry ) return;

    // keep track whether entry is new
    const bool entryIsNew( Base::KeySet<Logbook>( entry ).empty() );

    // get associated attachments and delete
    for( const auto& attachment:Base::KeySet<Attachment>(entry) )
    {

        // retrieve associated attachment frames
        for( const auto& frame:Base::KeySet<AttachmentFrame>(attachment) )
        { frame->remove( *attachment ); }

        // delete attachment
        delete attachment;

    };

    /*
    hide associated EditionWindows
    they will get deleted next time
    MainWindow::_displayEntry() is called
    */
    for( const auto& window:Base::KeySet<EditionWindow>( entry ) )
    {
        window->setIsClosed( true );
        window->hide();
    }

    if( !entryIsNew )
    {

        // remove from model
        entryModel_.remove( entry );

        // set associated logbooks as modified
        for( const auto& logbook : Base::KeySet<Logbook>( entry ) )
        { logbook->setModified( true ); }

        // delete entry
        /* this is not called for new entries, because already handled in editionWindow->setIsClosed */
        delete entry;

        // save
        if( save && !logbook_->file().isEmpty() )
        { this->save(); }

    }

    return;

}

//_______________________________________________
bool MainWindow::lockEntry( LogEntry* entry, EditionWindow* editionWindow )
{
    Debug::Throw( QStringLiteral("MainWindow::lockEntry.\n") );
    if( !entry ) return true;

    // check whether there are modified editors around and ask for save
    Base::KeySet<EditionWindow> windows( entry );
    auto reply = _checkModifiedEntries( windows );
    if( reply == AskForSaveDialog::Cancel ) return false;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();

    // mark modified editors as read only
    for( const auto& window:windows )
    {  if( window != editionWindow ) window->setReadOnly( true ); }

    return true;
}

//_______________________________________________
LogEntry* MainWindow::previousEntry( LogEntry* entry, bool updateSelection )
{

    Debug::Throw( QStringLiteral("MainWindow::previousEntry.\n") );
    QModelIndex index( entryModel_.index( entry ) );
    if( !( index.isValid() && index.row() > 0 ) ) return nullptr;

    QModelIndex previousIndex( entryModel_.index( index.row()-1, index.column() ) );
    if( updateSelection )
    {
        entryList_->selectionModel()->select( previousIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        entryList_->selectionModel()->setCurrentIndex( previousIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }

    return entryModel_.get( previousIndex );

}

//_______________________________________________
LogEntry* MainWindow::nextEntry( LogEntry* entry, bool updateSelection )
{

    Debug::Throw( QStringLiteral("MainWindow::nextEntry.\n") );
    QModelIndex index( entryModel_.index( entry ) );
    if( !( index.isValid() && index.row()+1 < entryModel_.rowCount() ) ) return nullptr;

    QModelIndex nextIndex( entryModel_.index( index.row()+1, index.column() ) );
    if( updateSelection )
    {
        entryList_->selectionModel()->select( nextIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        entryList_->selectionModel()->setCurrentIndex( nextIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }

    return entryModel_.get( nextIndex );

}

//_______________________________________________
void MainWindow::resetAttachmentWindow() const
{

    Debug::Throw( QStringLiteral("MainWindow::resetAttachmentWindow.\n") );

    // clear the AttachmentWindow
    auto& attachmentWindow( Base::Singleton::get().application<Application>()->attachmentWindow() );
    attachmentWindow.frame().clear();

    // check current logbook
    if( !logbook_ ) return;

    // retrieve logbook attachments, adds to AttachmentWindow
    attachmentWindow.frame().add( Base::makeT<AttachmentModel::List>(logbook_->attachments()) );

    return;

}

//_______________________________________________
Keyword MainWindow::currentKeyword() const
{
    Debug::Throw( QStringLiteral("MainWindow::currentKeyword.\n") );
    QModelIndex index( keywordList_->selectionModel()->currentIndex() );
    return index.isValid() ? keywordModel_.get( index ) : Keyword();
}

//_______________________________________________
void MainWindow::saveUnchecked()
{

    Debug::Throw( QStringLiteral("MainWindow::saveUnchecked.\n") );
    // check logbook
    if( !logbook_ )
    {
        InformationDialog( this, tr( "No Logbook opened. <Save> canceled." ) ).exec();
        return;
    }

        // check logbook filename, go to Save As if no file is given and redirect is true
    if( logbook_->file().isEmpty() ) {
        _saveAs();
        return;
    }

    // check logbook filename is writable
    auto fullname = logbook_->file().expanded();
    if( fullname.exists() ) {

        // check file is not a directory
        if( fullname.isDirectory() ) {
            InformationDialog( this, tr( "Selected file is a directory. <Save Logbook> canceled." ) ).exec();
            return;
        }

        // check file is writable
        if( !fullname.isWritable() ) {
            InformationDialog( this, tr( "Selected file is not writable. <Save Logbook> canceled." ) ).exec();
            return;
        }

    } else {

        auto path( fullname.path() );
        if( !path.isDirectory() ) {
            InformationDialog( this, tr( "Selected path is not vallid. <Save Logbook> canceled." ) ).exec();
            return;
        }

    }

    // write logbook to file, retrieve result
    Base::Singleton::get().application<Application>()->busy();
    _setEnabled( false );

    logbook_->truncateRecentEntriesList( maxRecentEntries_ );

    logbook_->write();
    Base::Singleton::get().application<Application>()->idle();
    _setEnabled( true );

    updateWindowTitle();

    // update StateFrame
    statusbar_->label().clear();
    statusbar_->showLabel();

    // add new file to openPreviousMenu
    if( !logbook_->file().isEmpty() ) Base::Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expanded() );

    // reset ignore_warning flag
    ignoreWarnings_ = false;

    return;

}

//_______________________________________________
void MainWindow::save()
{

    Debug::Throw( QStringLiteral("MainWindow::save.\n") );

    // check logbook
    if( !logbook_ )
    {
        InformationDialog( this, tr( "No Logbook opened. <Save> canceled." ) ).exec();
        return;
    }

    if( checkModifiedEntries() == AskForSaveDialog::Cancel ) return;

    saveUnchecked();

}

//_______________________________________________
void MainWindow::selectEntries( const QString &selection, SearchWidget::SearchModes mode )
{
    Debug::Throw() << "MainWindow::selectEntries - selection: " << selection << " mode:" << mode << endl;

    // check logbook
    if( !logbook_ ) return;

    // check selection text
    if( selection.isEmpty() )
    {
        showAllEntries();
        return;
    }

    // retrieve selection source
    if( mode == SearchWidget::None )
    {
        InformationDialog( this, tr( "At least one search field must be selected." ) ).centerOnParent().exec();
        return;
    }

    // number of found items
    int found( 0 );
    int total( 0 );

    // keep track of the last visible entry
    LogEntry *lastVisibleEntry( nullptr );

    // keep track of the current selected entry
    auto currentIndex( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( currentIndex.isValid() ? entryModel_.get( currentIndex ):nullptr );

    // check is selection is a valid color when Color search is requested.
    bool colorValid = ( mode&SearchWidget::Color && QColor( selection ).isValid() );

    // retrieve all logbook entries
    Base::KeySet<LogEntry> turnedOffEntries;
    for( const auto& entry:logbook_->entries() )
    {

        total++;

        // if entry is already hidder, skipp
        if( !entry->isFindSelected() ) continue;

        // check entry
        bool accept( false );
        if( (mode&SearchWidget::Title ) && entry->matchTitle( selection ) ) accept = true;
        if( (mode&SearchWidget::Keyword ) && entry->matchKeyword( selection ) ) accept = true;
        if( (mode&SearchWidget::Text ) && entry->matchText( selection ) ) accept = true;
        if( (mode&SearchWidget::Attachment ) && entry->matchAttachment( selection ) ) accept = true;
        if( colorValid && entry->matchColor( selection ) ) accept = true;

        if( accept )
        {

            found++;
            if( entry->isKeywordSelected() || !(lastVisibleEntry && lastVisibleEntry->isKeywordSelected()) )
            { lastVisibleEntry = entry; }

        } else {

            turnedOffEntries.insert( entry );
            entry->setFindSelected( false );

        }

    }

    // if no entries are found, restore the disabled entries and abort
    if( !found )
    {

        searchWidget_->noMatchFound();
        statusbar_->label().setText( tr( "No match found" ) );

        // reset flag for the turned off entries to true
        for( const auto& entry:turnedOffEntries )
        { entry->setFindSelected( true ); }

    } else {

        searchWidget_->matchFound();

        // reinitialize logEntry list
        _resetKeywordList();
        _resetLogEntryList();

        // if EditionWindow current entry is visible, select it;
        if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
        else if( lastVisibleEntry ) selectEntry( lastVisibleEntry );

        const QString buffer = QString( found > 1 ? tr( "%1 out of %2 entries selected" ):tr( "%1 out of %2 entry selected" ) ).arg( found ).arg( total );
        statusbar_->label().setText( buffer );

    }

    entryList_->setFocus();
    return;

}

//_______________________________________________
void MainWindow::showAllEntries()
{
    Debug::Throw( QStringLiteral("MainWindow::showAllEntries.\n") );

    // keep track of the current selected entry
    auto currentIndex( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( currentIndex.isValid() ? entryModel_.get( currentIndex ):nullptr );

    // set all logbook entries to find_visible
    for( const auto& entry:logbook_->entries() )
    { entry->setFindSelected( true ); }

    // reinitialize logEntry list
    _resetKeywordList();
    _resetLogEntryList();

    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( !entryModel_.empty() ) selectEntry( entryModel_.get( entryModel_.index( entryModel_.rowCount()-1, 0 ) ) );

    statusbar_->label().clear();

    entryList_->setFocus();
    return;

}

//____________________________________
void MainWindow::closeEvent( QCloseEvent *event )
{
    Debug::Throw( QStringLiteral("MainWindow::closeEvent.\n") );

    auto reply = checkModifiedEntries();
    if( reply == AskForSaveDialog::Cancel )
    {

        event->ignore();
        return;

    } else if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    else if( logbookIsModified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // quit application
    qApp->quit();
}

//_______________________________________________________
void MainWindow::timerEvent( QTimerEvent* event )
{
    if( event->timerId() == resizeTimer_.timerId() )
    {
        resizeTimer_.stop();
        XmlOptions::get().set<int>( QStringLiteral("KEYWORD_LIST_WIDTH"), keywordList_->width() );
    } else if(event->timerId() == editionTimer_.timerId() ) {
        editionTimer_.stop();
        auto index( entryList_->currentIndex() );
        if( index.isValid() && index == entryModel_.editionIndex() )
        { _startEntryEdition(); }
    } else if(event->timerId() == autosaveTimer_.timerId() ) {
        _autoSave();
    } else BaseMainWindow::timerEvent( event );
}

//________________________________________________
void MainWindow::contextMenuEvent( QContextMenuEvent* event )
{
    Debug::Throw( QStringLiteral("MainWindow::contextMenuEvent.\n") );
    BaseMainWindow::contextMenuEvent( event );
    if( event->isAccepted() ) return;

    // if event was rejected it means it is outer of one of the
    // relevant window area. However here we want it to also be accepted
    // in the 'customized' keyword and entry toolbars.
    Debug::Throw( QStringLiteral("MainWindow::contextMenuEvent - event rejected.\n") );

    // get child under widget
    bool accepted( false );
    for( auto child = childAt(event->pos()); child && child != this; child = child->parentWidget() )
    {
        if( child == keywordToolBar_ || child == entryToolBar_ )
        {
            accepted = true;
            break;
        }
    }

    if( !accepted ) return;
    auto menu = createPopupMenu();
    menu->exec( event->globalPos() );
    menu->deleteLater();
    event->accept();
    return;
}

//_______________________________________________
void MainWindow::_installActions()
{

    Debug::Throw( QStringLiteral("MainWindow::_installActions.\n") );
    uniconifyAction_ = new QAction( IconEngine::get( IconNames::Home ), tr( "Main Window" ), this );
    uniconifyAction_->setToolTip( tr( "Raise application main window" ) );
    connect( uniconifyAction_, &QAction::triggered, this, &BaseMainWindow::uniconify );

    newKeywordAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New Keyword" ), this );
    newKeywordAction_->setToolTip( tr( "Create a new keyword" ) );
    connect( newKeywordAction_, &QAction::triggered, this, &MainWindow::_newKeyword );

    addAction( editKeywordAction_ = new QAction( IconEngine::get( IconNames::Rename ), tr( "Rename Keyword..." ), this ) );
    editKeywordAction_->setToolTip( tr( "Rename selected keyword" ) );
    editKeywordAction_->setShortcut( Qt::Key_F2 );
    editKeywordAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( editKeywordAction_, &QAction::triggered, this, QOverload<>::of( &MainWindow::_renameKeyword ) );

    /*
    delete keyword action
    it is associated to the Qt::Key_Delete shortcut
    but the later is enabled only if the KeywordList has focus.
    */
    deleteKeywordAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete Keyword" ), this );
    deleteKeywordAction_->setToolTip( tr( "Delete selected keyword" ) );
    deleteKeywordAction_->setShortcut( QKeySequence::Delete );
    deleteKeywordAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( deleteKeywordAction_, &QAction::triggered, this, &MainWindow::_deleteKeyword );

    findEntriesAction_ = new QAction( IconEngine::get( IconNames::Find ), tr( "Find" ), this );
    findEntriesAction_->setShortcut( QKeySequence::Find );
    findEntriesAction_->setToolTip( tr( "Find entries matching specific criteria" ) );
    connect( findEntriesAction_, &QAction::triggered, this, &MainWindow::_findEntries );

    addAction( newEntryAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New Entry..." ), this ) );
    newEntryAction_->setToolTip( tr( "Create a new entry" ) );
    newEntryAction_->setShortcut( QKeySequence::New );
    connect( newEntryAction_, &QAction::triggered, this, &MainWindow::_newEntry );

    editEntryAction_ = new QAction( IconEngine::get( IconNames::Edit ), tr( "Edit Entries..." ), this );
    editEntryAction_->setToolTip( tr( "Edit selected entries" ) );
    connect( editEntryAction_, &QAction::triggered, this, &MainWindow::_editEntries );

    editEntryTitleAction_ = new QAction( IconEngine::get( IconNames::Rename ), tr( "Rename Entry..." ), this );
    editEntryTitleAction_->setToolTip( tr( "Edit selected entry title" ) );
    editEntryTitleAction_->setShortcut( Qt::Key_F2 );
    editEntryTitleAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( editEntryTitleAction_, &QAction::triggered, this, &MainWindow::_startEntryEdition );

    /*
    delete entry action
    it is associated to the Qt::Key_Delete shortcut
    but the later is enabled only if the KeywordList has focus.
    */
    deleteEntryAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete Entries" ), this );
    deleteEntryAction_->setToolTip( tr( "Delete selected entries" ) );
    deleteEntryAction_->setShortcut( QKeySequence::Delete );
    deleteEntryAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( deleteEntryAction_, &QAction::triggered, this, &MainWindow::_deleteEntries );

    // color menu
    colorMenu_ = new ColorMenu( this );
    colorMenu_->setTitle( tr( "Change entry color" ) );
    connect( colorMenu_, &ColorMenu::selected, this, &MainWindow::_changeEntryColor );

    entryColorAction_ = new QAction( IconEngine::get( IconNames::Color ), tr( "Entry Color" ), this );
    entryColorAction_->setToolTip( tr( "Change selected entries color" ) );
    entryColorAction_->setMenu( colorMenu_ );

    entryKeywordAction_ = new QAction( IconEngine::get( IconNames::Edit ), tr( "Change Keyword..." ), this );
    entryKeywordAction_->setToolTip( tr( "Edit selected entries keyword" ) );
    connect( entryKeywordAction_, &QAction::triggered, this, QOverload<>::of( &MainWindow::_renameEntryKeyword) );

    // entry information
    addAction( entryInformationAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Entry Properties..." ), this ) );
    entryInformationAction_->setToolTip( tr( "Show current entry properties" ) );
    connect( entryInformationAction_, &QAction::triggered, this, &MainWindow::_entryInformation );

    newLogbookAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New Logbook..." ), this );
    newLogbookAction_->setToolTip( tr( "Create a new logbook" ) );
    connect( newLogbookAction_, &QAction::triggered, this, &MainWindow::_newLogbook );

    openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open..." ), this );
    openAction_->setToolTip( tr( "Open an existsing logbook" ) );
    openAction_->setShortcut( QKeySequence::Open );
    connect( openAction_, &QAction::triggered, this, QOverload<>::of( &MainWindow::open ) );

    synchronizeAction_ = new QAction( IconEngine::get( IconNames::Merge ), tr( "Synchronize..." ), this );
    synchronizeAction_->setToolTip( tr( "Synchronize current logbook with remote" ) );
    connect( synchronizeAction_, &QAction::triggered, this, &MainWindow::_synchronize );

    reorganizeAction_ = new QAction( tr( "Reorganize" ), this );
    reorganizeAction_->setToolTip( tr( "Reoganize logbook entries in files" ) );
    connect( reorganizeAction_, &QAction::triggered, this, &MainWindow::_reorganize );

    saveAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save" ), this );
    saveAction_->setToolTip( tr( "Save all edited entries" ) );
    connect( saveAction_, &QAction::triggered, this, &MainWindow::save );

    saveForcedAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save (forced)" ), this );
    saveForcedAction_->setToolTip( tr( "Save all entries" ) );
    connect( saveForcedAction_, &QAction::triggered, this, &MainWindow::_saveForced );

    saveAsAction_ = new QAction( IconEngine::get( IconNames::SaveAs ), tr( "Save As..." ), this );
    saveAsAction_->setToolTip( tr( "Save logbook with a different name" ) );
    connect( saveAsAction_, &QAction::triggered, this, QOverload<>::of( &MainWindow::_saveAs ) );

    saveBackupAction_ = new QAction( IconEngine::get( IconNames::SaveAs ), tr( "Save Backup..." ), this );
    saveBackupAction_->setToolTip( tr( "Save logbook backup" ) );
    connect( saveBackupAction_, &QAction::triggered, this, &MainWindow::_saveBackup );

    backupManagerAction_ = new QAction( IconEngine::get( IconNames::ConfigureBackups ), tr( "Manage Backups..." ), this );
    backupManagerAction_->setToolTip( tr( "Save logbook backup" ) );
    connect( backupManagerAction_, &QAction::triggered, this, &MainWindow::_manageBackups );

    // reload
    reloadAction_ = new QAction( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this );
    reloadAction_->setToolTip( tr( "Restore saved logbook" ) );
    reloadAction_->setShortcut( QKeySequence::Refresh );
    connect( reloadAction_, &QAction::triggered, this, &MainWindow::_revertToSaved );

    // print
    printAction_ = new QAction( IconEngine::get( IconNames::Print ), tr( "Print..." ), this );
    printAction_->setShortcut( QKeySequence::Print );
    connect( printAction_, &QAction::triggered, this, QOverload<>::of( &MainWindow::_print ) );

    // print preview
    addAction( printPreviewAction_ = new QAction( IconEngine::get( IconNames::PrintPreview ), tr( "Print Preview..." ), this ) );
    printPreviewAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_P );
    connect( printPreviewAction_, &QAction::triggered, this, &MainWindow::_printPreview );

    // export to HTML
    htmlAction_ = new QAction( IconEngine::get( IconNames::Html ), tr( "Export to HTML..." ), this );
    connect( htmlAction_, &QAction::triggered, this, &MainWindow::_toHtml );

    logbookStatisticsAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Logbook Statistics..." ), this );
    logbookStatisticsAction_->setToolTip( tr( "View logbook statistics" ) );
    connect( logbookStatisticsAction_, &QAction::triggered, this, &MainWindow::_viewLogbookStatistics );

    logbookInformationsAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Logbook Properties..." ), this );
    logbookInformationsAction_->setToolTip( tr( "Edit logbook properties" ) );
    connect( logbookInformationsAction_, &QAction::triggered, this, &MainWindow::_editLogbookInformations );

    closeFramesAction_ = new QAction( IconEngine::get( IconNames::Close ), tr( "Close Editors" ), this );
    closeFramesAction_->setToolTip( tr( "Close all entry editors" ) );
    connect( closeFramesAction_, &QAction::triggered, this, &MainWindow::_closeEditionWindows );

    // show duplicated entries
    showDuplicatesAction_ = new QAction( tr( "Show Duplicated Entries..." ), this );
    showDuplicatesAction_->setToolTip( tr( "Show duplicated entries in logbook" ) );
    connect( showDuplicatesAction_, &QAction::triggered, this, &MainWindow::_showDuplicatedEntries );

    // view monitored files
    monitoredFilesAction_ = new QAction( tr( "Show Monitored Files..." ), this );
    monitoredFilesAction_->setToolTip( tr( "Show monitored files" ) );
    connect( monitoredFilesAction_, &QAction::triggered, this, &MainWindow::_showMonitoredFiles );

    // tree mode
    treeModeAction_ = new QAction( tr( "Use Tree to Display Entries and Keywords" ), this );
    treeModeAction_->setCheckable( true );
    treeModeAction_->setChecked( true );
    connect( treeModeAction_, &QAction::toggled, this, &MainWindow::_toggleTreeMode );

    // menu actions
    QAction* action;
    keywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::Move ), tr( "Move Here" ), this ) );
    keywordChangedMenuActions_.append( action = new QAction( this ) );
    action->setSeparator( true );
    keywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::DialogCancel ), tr( "Cancel" ), this ) );
    action->setShortcut( Qt::Key_Escape );

    entryKeywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::Move ), tr( "Move Here" ), this ) );
    entryKeywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::Copy ), tr( "Copy Here" ), this ) );
    entryKeywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::Link ), tr( "Link Here" ), this ) );
    entryKeywordChangedMenuActions_.append( action = new QAction( this ) );
    action->setSeparator( true );
    entryKeywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::DialogCancel ), tr( "Cancel" ), this ) );
    action->setShortcut( Qt::Key_Escape );

}

//_______________________________________________
void MainWindow::_resetKeywordList()
{

    Debug::Throw( QStringLiteral("MainWindow::_resetKeywordList.\n") );

    // retrieve new list of keywords (from logbook)
    Keyword::OrderedSet newKeywords;
    Keyword root;
    for( const auto& entry:logbook_->entries() )
    {
        if( entry->isFindSelected() )
        {
            for( auto keyword:entry->keywords() )
            {
                for( ; keyword != root; keyword = keyword.parent() )
                { newKeywords.insert( keyword ); }
            }
        }
    }
    keywordModel_.set( Base::makeT<KeywordModel::List>(newKeywords) );
}

//_______________________________________________
void MainWindow::_resetLogEntryList()
{

    Debug::Throw( QStringLiteral("MainWindow::_resetLogEntryList.\n") );

    // clear list of entries
    entryModel_.clear();

    if( logbook_ )
    {

        LogEntryModel::List modelEntries;
        auto entries( logbook_->entries() );
        std::copy_if( entries.begin(), entries.end(), std::back_inserter(modelEntries),
            [this]( LogEntry* entry )
            { return (!treeModeAction_->isChecked() && entry->isFindSelected()) || entry->isSelected(); } );

        entryModel_.add( modelEntries );

    }

    // loop over associated editionwindows
    // update navigation buttons
    for( const auto& window:Base::KeySet<EditionWindow>( this ) )
    {

        // skip closed editors
        if( window->isClosed() ) continue;

        // get associated entry and see if selected
        auto entry( window->entry() );
        window->previousEntryAction().setEnabled( entry && entry->isSelected() && previousEntry(entry, false) );
        window->nextEntryAction().setEnabled( entry && entry->isSelected() && nextEntry(entry, false) );

    }

    return;

}

//_______________________________________________
void MainWindow::_loadColors()
{

    Debug::Throw( QStringLiteral("MainWindow::_loadColors.\n") );

    if( !logbook_ ) return;

    // retrieve all entries
    for( const auto& entry:logbook_->entries() )
    { colorMenu_->add( entry->color() ); }

}

//_______________________________________________
void MainWindow::_setEnabled( bool value )
{

    // main widget
    centralWidget()->setEnabled( value );

    // menu
    menuBar_->setEnabled( value );

    // toolbars
    for( const auto& toolbar:findChildren<QToolBar*>() )
    { toolbar->setEnabled( value ); }

}


//__________________________________________________________________
bool MainWindow::_hasModifiedEntries() const
{
    Base::KeySet<EditionWindow> frames( this );
    return std::any_of( frames.begin(), frames.end(), EditionWindow::ModifiedFTor() );
}

//_______________________________________________
void MainWindow::_autoSave()
{

    if( logbook_ && !logbook_->file().isEmpty() )
    {

        statusbar_->label().setText( tr( "Saving" ) );

        // retrieve non read only editors; perform save
        for( const auto& window:Base::KeySet<EditionWindow>( this ) )
        {
            if( window->isReadOnly() || window->isClosed() ) continue;
            window->saveAction().trigger();
        }

        save();

    } else {

        statusbar_->label().setText( tr( "no logbook filename. <Autosave> skipped" ) );

    }

}

//__________________________________________________________________
AskForSaveDialog::ReturnCode MainWindow::_checkModifiedEntries( Base::KeySet<EditionWindow> windows )
{
    Debug::Throw( QStringLiteral("_MainWindow::checkModifiedEntries.\n") );

    // check if editable EditionWindows needs save
    // cancel if required
    int count = std::count_if( windows.begin(), windows.end(),
        []( EditionWindow* window )
        { return !( window->isReadOnly() || window->isClosed()) && window->modified(); } );

    // do nothing if no window needs saving
    if( !count ) return AskForSaveDialog::No;

    // loop over windows and ask for confirmation
    bool confirm = true;
    bool modified = false;
    for( const auto& window:windows )
    {
        // skip unmodified windows
        if( window->isReadOnly() || window->isClosed() || !window->modified() ) continue;

        if( !confirm ) window->writeEntryToLogbook( false );
        else {

            AskForSaveDialog::ReturnCodes buttons( AskForSaveDialog::Yes | AskForSaveDialog::No  | AskForSaveDialog::Cancel );
            if( count > 1 ) buttons |= AskForSaveDialog::All;

            QString message = tr( "Entry '%1' has been modified. Save ?" ).arg( window->entryTitle() );
            AskForSaveDialog dialog( this, message, buttons );
            auto reply = dialog.centerOnParent().exec();
            if( reply == AskForSaveDialog::Yes )
            {
                modified = true;
                window->writeEntryToLogbook( false );

            } else if( reply == AskForSaveDialog::All ) {

                modified = true;
                confirm = false;
                window->writeEntryToLogbook( false );

            } else if( reply == AskForSaveDialog::Cancel ) return AskForSaveDialog::Cancel;

            count--;

        }

    }

    return  modified ? AskForSaveDialog::Yes:AskForSaveDialog::No;

}

//_______________________________________________
void MainWindow::_updateEntryFrames( LogEntry* entry, Mask mask )
{
    Debug::Throw( QStringLiteral("MainWindow::_updateEntryFrames.\n") );

    if( !mask ) return;

    // update associated EditionWindows
    for( const auto& window:Base::KeySet<EditionWindow>( entry ) )
    {

        // keep track of already modified EditionWindows
        bool windowModified( window->modified() && !window->isReadOnly() );

        // update EditionWindow
        if( mask&TitleMask ) window->displayTitle();
        if( mask&KeywordMask ) window->displayKeyword();

        // save if needed [title/keyword changes are discarded since saved here anyway]
        if( windowModified ) window->askForSave();
        else window->setModified( false );

    }

}

//_____________________________________________
void MainWindow::_filesModified( const FileCheck::DataSet &files )
{

    Debug::Throw( QStringLiteral("MainWindow::_filesModified.\n") );

    if( ignoreWarnings_ ) return;
    if( files.empty() ) return;

    // check if there is already a dialog open, and update list of files if yes
    auto dialog = findChild<LogbookModifiedDialog*>();
    if( dialog ) dialog->addFiles( files );
    else {

        // create dialog and take action accordingly
        auto state = static_cast<LogbookModifiedDialog::ReturnCode>( LogbookModifiedDialog( this, files ).exec() );
        if( state == LogbookModifiedDialog::ReturnCode::SaveAgain ) { save(); }
        else if( state == LogbookModifiedDialog::ReturnCode::SaveAs ) { _saveAs(); }
        else if( state == LogbookModifiedDialog::ReturnCode::Reload )
        {

            logbook_->setModifiedRecursive( false );
            _revertToSaved();

        } else if( state == LogbookModifiedDialog::ReturnCode::Ignore ) { ignoreWarnings_ = true; }

    }

    return;
}

//________________________________________________________
void MainWindow::_splitterMoved()
{
    Debug::Throw( QStringLiteral("MainWindow::_splitterMoved.\n") );
    resizeTimer_.start( 200, this );
}

//_______________________________________________
void MainWindow::_newLogbook()
{
    Debug::Throw( QStringLiteral("MainWindow::_newLogbook.\n") );

    // check current logbook
    auto reply = checkModifiedEntries();
    if( reply == AskForSaveDialog::Cancel ) return;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    else if( logbookIsModified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // new logbook
    NewLogbookDialog dialog( this );
    dialog.setTitle( Logbook::NoTitle );
    dialog.setAuthor( XmlOptions::get().raw( QStringLiteral("USER") ) );
    dialog.setAttachmentDirectory( workingDirectory() );
    if( !dialog.centerOnParent().exec() ) return;

    // create a new logbook, with no file
    setLogbook( File() );

    logbook_->setTitle( dialog.title() );
    logbook_->setAuthor( dialog.author() );
    logbook_->setComments( dialog.comments() );

    // attachment directory
    File directory( dialog.attachmentDirectory() );

    // check if fulldir is not a non directory existsing file
    if( directory.exists() && !directory.isDirectory() )
    {

        const QString buffer = tr( "File '%1' is not a directory." ).arg( directory );
        InformationDialog( this, buffer ).exec();

    } else logbook_->setDirectory( directory );

}

//_______________________________________________
void MainWindow::updateWindowTitle()
{

    Debug::Throw( QStringLiteral("MainWindow::updateWindowTitle.\n") );

    if( logbook_ )
    {

        if( logbook_->file().isEmpty() )
        {

            if( logbook_->isReadOnly() ) QMainWindow::setWindowTitle( tr( "ELogbook (read-only)" ) );
            else if( logbook_->modified() ) QMainWindow::setWindowTitle( tr( "ELogbook (modified)" ) );
            else QMainWindow::setWindowTitle( QStringLiteral("Elogbook") );

        } else  {

            if( logbook_->isReadOnly() ) setWindowTitle( tr( "%1 (read-only)" ).arg( logbook_->file() ) );
            else if( logbook_->modified() ) setWindowTitle( tr( "%1 (modified)" ).arg( logbook_->file() ) );
            else setWindowTitle( logbook_->file() );

        }

    } else QMainWindow::setWindowTitle( QStringLiteral("Elogbook") );

}

//_______________________________________________
void MainWindow::open( FileRecord record )
{

    Debug::Throw( QStringLiteral("MainWindow::open.\n") );

    // check if current logbook needs save
    auto reply = checkModifiedEntries();
    if( reply == AskForSaveDialog::Cancel ) return;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    else if( logbookIsModified()  && askForSave() == AskForSaveDialog::Cancel ) return;

    // open file from dialog if not set as argument
    if( record.file().isEmpty() )
    {

        auto file( FileDialog(this).selectFile( workingDirectory() ).getFile() );
        if( file.isEmpty() ) return;
        else record = FileRecord( file );

    }

    // create logbook from file
    Base::Singleton::get().application<Application>()->busy();
    setLogbook( record.file() );
    Base::Singleton::get().application<Application>()->idle();

    // check if backup is needed
    // no need to do that for read-only logbooks
    checkLogbookBackup();

    return;
}

//_______________________________________________
bool MainWindow::_saveAs( File defaultFile, bool registerLogbook )
{
    Debug::Throw( QStringLiteral("MainWindow::_saveAs.\n"));

    // check current logbook
    if( !logbook_ )
    {
        InformationDialog( this, tr( "No logbook opened. <Save Logbook> canceled." ) ).exec();
        return false;
    }

    // check default filename
    if( defaultFile.isEmpty() ) defaultFile = logbook_->file();
    if( defaultFile.isEmpty() ) defaultFile = File( "log.xml" ).addPath( workingDirectory() );

    // create file dialog
    FileDialog dialog( this );
    dialog.setAcceptMode( QFileDialog::AcceptSave );
    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.selectFile( defaultFile );

    // get file
    File fullname( dialog.getFile() );
    if( fullname.isEmpty() ) return false;
    else fullname.expand();

    // update working directory
    workingDirectory_ = fullname.path();

    // change logbook filename and save
    logbook_->setFile( fullname );
    logbook_->setModifiedRecursive( true );
    save();

    // update current file in menu
    menuBar_->recentFilesMenu().setCurrentFile( fullname );

    /*
    force logbook state to unmodified since
    some children state may not have been reset properly
    */
    logbook_->setModifiedRecursive( false );

    // add new file to openPreviousMenu
    if( !logbook_->file().isEmpty() )
    { Base::Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expanded() ); }

    // redo file check registration
    if( registerLogbook )
    {
        fileCheck_->clear();
        fileCheck_->registerLogbook( logbook_.get() );
    }

    // reset ignore_warning flag
    ignoreWarnings_ = false;

    return true;
}


//_____________________________________________
void MainWindow::_saveForced()
{
    Debug::Throw( QStringLiteral("MainWindow::_saveForced.\n") );

    // retrieve/check MainWindow/Logbook
    if( !logbook_ )
    {
        InformationDialog( this, tr( "No Logbook opened. <Save> canceled." ) ).exec();
        return;
    }

    // set all logbooks as modified
    logbook_->setModifiedRecursive( true );
    save();

}

//_______________________________________________
void MainWindow::_saveBackup()
{
    Debug::Throw( QStringLiteral("MainWindow::_saveBackup.\n"));

    // check current logbook
    if( !logbook_ ) {
        InformationDialog( this, tr( "No logbook opened. <Save Backup> canceled." ) ).exec();
        return;
    }

    // generate backup fileName
    auto filename( logbook_->backupFileName( ) );
    if( filename.isEmpty() ) {
        InformationDialog( this, tr( "No valid filename. Use <Save As> first." ) ).exec();
        return;
    }

    // stores current logbook filename
    auto currentFileName( logbook_->file() );
    const bool readOnlyState( logbook_->isReadOnly() );
    const bool backupState( logbook_->isBackup() );

    // save logbook as backup.
    // mark backups as read-only
    logbook_->setReadOnly( true );
    logbook_->setIsBackup( true );
    bool saved( _saveAs( filename, false ) );

    // restore old read-only state
    logbook_->setReadOnly( readOnlyState );
    logbook_->setIsBackup( backupState );

    // remove the "backup" filename from the openPrevious list
    // to avoid confusion
    Base::Singleton::get().application<Application>()->recentFiles().remove( File(filename).expanded() );

    // restore initial filename
    logbook_->setFile( currentFileName, true );

    if( saved )
    {

        logbook_->addBackup( filename );
        logbook_->setModified( true );
        updateWindowTitle();

        // Save logbook if needed (to make sure the backup stamp is updated)
        if( !logbook_->file().isEmpty() ) save();
    }

}

//_______________________________________________
void MainWindow::_manageBackups()
{
    Debug::Throw( QStringLiteral("MainWindow::_manageBackups.\n"));

    BackupManagerDialog dialog( this );
    Base::Key::associate( &dialog.managerWidget(), logbook_.get() );
    dialog.managerWidget().updateBackups();

    // connections
    connect( &dialog.managerWidget(), &BackupManagerWidget::saveLogbookRequested, this, &MainWindow::save );
    connect( &dialog.managerWidget(), &BackupManagerWidget::backupRequested, this, &MainWindow::_saveBackup );
    connect( &dialog.managerWidget(), &BackupManagerWidget::removeBackupRequested, this, &MainWindow::_removeBackup );
    connect( &dialog.managerWidget(), &BackupManagerWidget::removeBackupsRequested, this, &MainWindow::_removeBackups );
    connect( &dialog.managerWidget(), &BackupManagerWidget::restoreBackupRequested, this, &MainWindow::_restoreBackup );
    connect( &dialog.managerWidget(), &BackupManagerWidget::mergeBackupRequested, this, &MainWindow::_mergeBackup );

    dialog.exec();
}

//_____________________________________________
void MainWindow::_revertToSaved()
{
    Debug::Throw( QStringLiteral("MainWindow::_revertToSaved.\n") );

    // check logbook
    if( !logbook_ )
    {
        InformationDialog( this, tr( "No logbook opened. <Reload> canceled." ) ).exec();
        return;
    }

    // ask for confirmation
    auto buffer = tr( "Discard changes to '%1'?" ).arg( logbook_->file().localName());
    if( ( _hasModifiedEntries() || logbook_->modified() ) && !QuestionDialog( this, buffer ).exec() )
    { return; }

    // reinit MainWindow
    Base::Singleton::get().application<Application>()->busy();
    setLogbook( logbook_->file() );
    Base::Singleton::get().application<Application>()->idle();

    // check if backup is needed
    // no need to do that for read-only logbooks
    checkLogbookBackup();
    ignoreWarnings_ = false;

}

//___________________________________________________________
void MainWindow::_print()
{

    // save EditionWindows
    // check if current logbook needs save
    auto reply = checkModifiedEntries();
    if( reply == AskForSaveDialog::Cancel ) return;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    else if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // get entry selection
    LogEntryPrintSelectionDialog selectionDialog( this );
    selectionDialog.okButton().setText( tr( "Print ..." ) );
    selectionDialog.okButton().setIcon( IconEngine::get( IconNames::Print ) );
    if( !selectionDialog.exec() ) return;

    // create helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook_.get() );

    // assign entries and keyword
    auto selectionMode( selectionDialog.mode() );
    helper.setEntries( _entries( selectionMode ) );

    if( treeModeAction_->isChecked() &&
        ( selectionMode == LogEntryPrintSelectionWidget::Mode::VisibleEntries ||
        selectionMode == LogEntryPrintSelectionWidget::Mode::SelectedEntries ) )
    { helper.setCurrentKeyword( currentKeyword() ); }

    // print
    _print( helper );

}

//___________________________________________________________
void MainWindow::_print( LogbookPrintHelper& helper )
{
    Debug::Throw( QStringLiteral("MainWindow::_print.\n") );

    // create printer
    QPrinter printer( QPrinter::HighResolution );

    // generate document name
    printer.setDocName( QStringLiteral( "elogbook_%1_%2_%3" ).arg( Util::user(), TimeStamp::now().unixTime(), Util::pid() ) );

    // create options widget
    auto optionWidget = new PrinterOptionWidget;
    optionWidget->setHelper( &helper );
    connect( optionWidget, &PrinterOptionWidget::orientationChanged, &helper, &BasePrintHelper::setOrientation );
    connect( optionWidget, &PrinterOptionWidget::pageModeChanged, &helper, &BasePrintHelper::setPageMode );

    auto logbookOptionWidget = new LogbookPrintOptionWidget;
    logbookOptionWidget->setWindowTitle( tr( "Logbook Configuration" ) );
    connect( logbookOptionWidget, &LogbookPrintOptionWidget::maskChanged, &helper, &LogbookPrintHelper::setMask );
    logbookOptionWidget->read( XmlOptions::get() );

    auto logEntryOptionWidget = new LogEntryPrintOptionWidget;
    logEntryOptionWidget->setWindowTitle( tr( "Logbook Entry Configuration" ) );
    connect( logEntryOptionWidget, &LogEntryPrintOptionWidget::maskChanged, &helper, &LogbookPrintHelper::setEntryMask );
    logEntryOptionWidget->read( XmlOptions::get() );

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setWindowTitle( Util::windowTitle( tr( "Print Logbook" ) ) );
    dialog.setOptionTabs( { optionWidget, logbookOptionWidget, logEntryOptionWidget });

    if( dialog.exec() == QDialog::Rejected ) return;

    // add output file to scratch files, if any
    if( !printer.outputFileName().isEmpty() )
    { emit scratchFileCreated( File( printer.outputFileName() ) ); }

    // write options
    logbookOptionWidget->write( XmlOptions::get() );
    logEntryOptionWidget->write( XmlOptions::get() );

    // retrieve mask and assign
    helper.setMask( logbookOptionWidget->mask() );
    helper.setEntryMask( logEntryOptionWidget->mask() );

    // print
    helper.print( &printer );

    // reset status bar
    statusbar_->label().clear();
    statusbar_->showLabel();

    return;

}

//___________________________________________________________
void MainWindow::_printPreview()
{
    Debug::Throw( QStringLiteral("MainWindow::_printPreview.\n") );

    // save EditionWindows and logbook
    auto reply = checkModifiedEntries();
    if( reply == AskForSaveDialog::Cancel ) return;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    else if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // get entry selection
    LogEntryPrintSelectionDialog selectionDialog( this );
    selectionDialog.okButton().setText( tr( "Preview ..." ) );
    selectionDialog.okButton().setIcon( IconEngine::get( IconNames::PrintPreview ) );
    if( !selectionDialog.exec() ) return;

    // create helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook_.get() );

    auto selectionMode( selectionDialog.mode() );

    if( treeModeAction_->isChecked() &&
        ( selectionMode == LogEntryPrintSelectionWidget::Mode::VisibleEntries ||
        selectionMode == LogEntryPrintSelectionWidget::Mode::SelectedEntries ) )
    { helper.setCurrentKeyword( currentKeyword() ); }

    switch( selectionMode )
    {

        case LogEntryPrintSelectionWidget::Mode::AllEntries:
        helper.setEntries( Base::makeT<LogEntryModel::List>( Base::KeySet<LogEntry>( logbook_->entries() ) ) );
        break;

        default:
        case LogEntryPrintSelectionWidget::Mode::VisibleEntries:
        helper.setEntries( entryModel_.get() );
        break;

        case LogEntryPrintSelectionWidget::Mode::SelectedEntries:
        helper.setEntries( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
        break;

    }

    // masks
    helper.setMask( (Logbook::Mask) XmlOptions::get().get<int>( QStringLiteral("LOGBOOK_PRINT_OPTION_MASK") ) );
    helper.setEntryMask( (LogEntry::Mask) XmlOptions::get().get<int>( QStringLiteral("LOGENTRY_PRINT_OPTION_MASK") ) );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this, CustomDialog::OkButton|CustomDialog::CancelButton );
    dialog.setWindowTitle( tr( "Print Preview" ) );
    dialog.setHelper( &helper );

    // print
    if( dialog.exec() ) _print( helper );
    else {
        statusbar_->label().clear();
        statusbar_->showLabel();
    }

}

//___________________________________________________________
void MainWindow::_toHtml()
{
    Debug::Throw( QStringLiteral("MainWindow::_toHtml.\n") );

    // save EditionWindows
    auto reply = checkModifiedEntries();
    if( reply == AskForSaveDialog::Cancel ) return;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    else if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create options widget
    auto logbookOptionWidget = new LogbookPrintOptionWidget;
    logbookOptionWidget->read( XmlOptions::get() );

    auto logEntrySelectionWidget = new LogEntryPrintSelectionWidget;
    logEntrySelectionWidget->read( XmlOptions::get() );

    auto logEntryOptionWidget = new LogEntryPrintOptionWidget;
    logEntryOptionWidget->read( XmlOptions::get() );

    // create dialog
    HtmlDialog dialog( this );
    dialog.setOptionWidgets( { logEntrySelectionWidget, logbookOptionWidget, logEntryOptionWidget } );

    // generate file name
    QString buffer = QStringLiteral( "eLogbook_%1_%2_%3.html" ).arg( Util::user(), TimeStamp::now().unixTime(), Util::pid() );
    dialog.setFile( File( buffer ).addPath( Util::tmp() ) );

    // execute dialog
    if( !dialog.exec() ) return;

    // retrieve/check file
    File file( dialog.file() );
    if( file.isEmpty() ) {
        InformationDialog(this, tr( "No output file specified. <View HTML> canceled." ) ).exec();
        return;
    }

    QFile out( file );
    if( !out.open( QIODevice::WriteOnly ) )
    {
        const QString buffer = tr( "Cannot write to file '%1'. <View HTML> canceled." ).arg( file );
        InformationDialog( this, buffer ).exec();
        return;
    }

    // add as scratch file
    emit scratchFileCreated( file );

    // write options
    logbookOptionWidget->write( XmlOptions::get() );
    logEntrySelectionWidget->write( XmlOptions::get() );
    logEntryOptionWidget->write( XmlOptions::get() );

    // create print helper
    LogbookHtmlHelper helper( this );
    helper.setLogbook( logbook_.get() );

    // assign entries and keyword
    auto selectionMode( logEntrySelectionWidget->mode() );
    helper.setEntries( _entries( selectionMode ) );

    if( treeModeAction_->isChecked() &&
        ( selectionMode == LogEntryPrintSelectionWidget::Mode::VisibleEntries ||
        selectionMode == LogEntryPrintSelectionWidget::Mode::SelectedEntries ) )
    { helper.setCurrentKeyword( currentKeyword() ); }

    // retrieve mask and assign
    helper.setMask( logbookOptionWidget->mask() );
    helper.setEntryMask( logEntryOptionWidget->mask() );

    // print
    helper.print( &out );
    out.close();

    // get command and execute
    QString command( dialog.command() );
    if( !command.isEmpty() )
    { ( Base::Command( command ) << file ).run(); }

}

//_______________________________________________
void MainWindow::_synchronize()
{
    Debug::Throw( QStringLiteral("MainWindow::_synchronize.\n") );

    // check current logbook is valid
    if( !logbook_ ) {
        InformationDialog( this, tr( "No logbook opened. <Merge> canceled." ) ).exec();
        return;
    }

    // save EditionWindows
    auto reply = checkModifiedEntries();
    if( reply == AskForSaveDialog::Cancel ) return;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    else if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create file dialog
    File remoteFile( FileDialog(this).getFile() );
    if( remoteFile.isEmpty() ) return;

    // debug
    Debug::Throw() << "MainWindow::_synchronize - number of local files: " << logbook_->children().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - number of local entries: " << logbook_->entries().size() << endl;

    // set busy flag
    Base::Singleton::get().application<Application>()->busy();
    statusbar_->label().setText( QStringLiteral("Reading remote logbook ... ") );

    // opens file in remote logbook
    Debug::Throw() << "MainWindow::_synchronize - reading remote logbook from file: " << remoteFile << endl;

    Logbook remoteLogbook;
    connect( &remoteLogbook, &Logbook::messageAvailable, this, &MainWindow::messageAvailable );
    remoteLogbook.setFile( remoteFile );
    remoteLogbook.read();

    // check if logbook is valid
    XmlError::List errors( remoteLogbook.xmlErrors() );
    if( errors.size() )
    {

        QString buffer = QString( errors.size() > 1 ? tr( "Errors occured while parsing files.\n"):tr("An error occured while parsing files.\n") );
        buffer += XmlError::toString( errors );
        InformationDialog( nullptr, buffer ).exec();

        Base::Singleton::get().application<Application>()->idle();
        return;

    }

    // debug
    Debug::Throw() << "MainWindow::_synchronize - number of remote files: " << remoteLogbook.children().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - number of remote entries: " << remoteLogbook.entries().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - updating local from remote" << endl;

    // synchronize local with remote
    // retrieve map of duplicated entries
    auto duplicates( logbook_->synchronize( remoteLogbook ) );
    Debug::Throw() << "MainWindow::_synchronize - number of duplicated entries: " << duplicates.size() << endl;

    // update possible EditionWindows when duplicated entries are found
    // delete the local duplicated entries
    for( auto&& iter = duplicates.begin(); iter != duplicates.end(); ++iter )
    {

        // display the new entry in all matching edit frames
        for( const auto& window:Base::KeySet<EditionWindow>( iter.key() ) )
        { window->displayEntry( iter.value() ); }

        delete iter.key();

    }

    // reinitialize lists
    _resetKeywordList();

    // reset selected keyword
    _keywordSelectionChanged( keywordList_->selectionModel()->currentIndex() );

    // reset attachment window
    resetAttachmentWindow();

    // retrieve last modified entry
    Base::KeySet<LogEntry> entries( logbook_->entries() );
    selectEntry( *std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() ) );
    entryList_->setFocus();

    // write local logbook
    if( !logbook_->file().isEmpty() ) save();

    // synchronize remove with local
    Debug::Throw() << "MainWindow::_synchronize - updating remote from local" << endl;
    int nDuplicated = remoteLogbook.synchronize( *logbook_ ).size();
    Debug::Throw() << "MainWindow::_synchronize - number of duplicated entries: " << nDuplicated << endl;

    // save remote logbook
    statusbar_->label().setText( tr( "Saving remote logbook..." ) );
    remoteLogbook.write();

    // idle
    Base::Singleton::get().application<Application>()->idle();
    statusbar_->label().clear();

    return;

}

//_______________________________________________
void MainWindow::_removeBackup( const Backup &backup )
{ _removeBackups( { backup } ); }

//_______________________________________________
void MainWindow::_removeBackups( const Backup::List &backups )
{

    Debug::Throw( QStringLiteral("MainWindow::_removeBackups.\n") );

    Base::Singleton::get().application<Application>()->busy();

    // store logbook backups
    auto logbookBackups( logbook_->backupFiles() );

    File::List invalidFiles;
    bool modified( false );
    for( const auto& backup:backups )
    {

        if( !backup.file().exists() )
        {
            invalidFiles.append( backup.file() );
            continue;
        }

        // read backup
        Logbook backupLogbook;
        backupLogbook.setFile( backup.file() );
        backupLogbook.read();

        // remove main file
        emit messageAvailable( tr( "Removing '%1'" ).arg( backupLogbook.file() ) );
        backupLogbook.file().remove();

        // remove children files
        for( const auto& logbook:backupLogbook.children() )
        {
            emit messageAvailable( tr( "Removing '%1'" ).arg( logbook->file() ) );
            logbook->file().remove();
        }

        // remove from logbook backups
        if( logbookBackups.removeAll( backup ) ) modified = true;
    }

    Base::Singleton::get().application<Application>()->idle();

    if( modified )
    {
        // assign new list to logbook and save
        logbook_->setBackupFiles( logbookBackups );
        if( !logbook_->file().isEmpty() ) save();
    }

    if( invalidFiles.size() == 1 )
    {

        const QString buffer = tr( "Unable to open file named '%1'. <Remove Backup> canceled." ).arg( invalidFiles.front() );
        InformationDialog( this, buffer ).exec();

    } else if( invalidFiles.size() > 1 ) {

        const QString buffer = tr( "%i files could not be opened. <Remove Backup> canceled." ).arg( invalidFiles.size() );
        QString details;
        for( const auto& file:invalidFiles )
        { details += file + "\n"; }

        InformationDialog dialog( this, buffer );
        dialog.setDetails( details );
        dialog.exec();

    }

}

//_______________________________________________
void MainWindow::_restoreBackup( const Backup &backup )
{
    Debug::Throw( QStringLiteral("MainWindow::_restoreBackup.\n") );
    if( !backup.file().exists() )
    {
        const QString buffer = tr( "Unable to open file named '%1'. <Restore Backup> canceled." ).arg( backup.file() );
        InformationDialog( this, buffer ).exec();
        return;
    }

    // store old filename
    File oldName( logbook_->file() );

    // store old backups
    Backup::List backups( logbook_->backupFiles() );

    // store associated backup manager Widget
    Base::KeySet<BackupManagerWidget> widgets( logbook_.get() );

    // replace logbook with backup
    setLogbook( backup.file() );

    // remove the "backup" filename from the openPrevious list
    // to avoid confusion
    Base::Singleton::get().application<Application>()->recentFiles().remove( backup.file().expanded() );

    // change filename
    logbook_->setFile( oldName );

    // reassign backups
    logbook_->setBackupFiles( backups );

    // re-associate
    for( const auto& widget:widgets )
    { Base::Key::associate( widget, logbook_.get() ); }

    // and save
    if( !logbook_->file().isEmpty() )
    {
        save();
        Base::Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expanded() );
    }

}

//_______________________________________________
void MainWindow::_mergeBackup( const Backup &backup )
{
    Debug::Throw( QStringLiteral("MainWindow::_mergeBackup.\n") );

    // check current logbook is valid
    if( !logbook_ ) {
        InformationDialog( this, tr( "No logbook opened. <Merge> canceled." ) ).exec();
        return;
    }

    if( !backup.file().exists() ) return;

    // save EditionWindows
    auto reply = checkModifiedEntries();
    if( reply == AskForSaveDialog::Cancel ) return;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();
    else if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // debug
    Debug::Throw() << "MainWindow::_mergeBackup - number of local files: " << logbook_->children().size() << endl;
    Debug::Throw() << "MainWindow::_mergeBackup - number of local entries: " << logbook_->entries().size() << endl;

    // set busy flag
    Base::Singleton::get().application<Application>()->busy();
    statusbar_->label().setText( tr( "Reading remote logbook..." ) );

    // opens file in remote logbook
    Debug::Throw() << "MainWindow::_mergeBackup - reading remote logbook from file: " << backup.file() << endl;

    Logbook backupLogbook;
    connect( &backupLogbook, &Logbook::messageAvailable, this, &MainWindow::messageAvailable );
    backupLogbook.setFile( backup.file() );
    backupLogbook.read();

    // check if logbook is valid
    XmlError::List errors( backupLogbook.xmlErrors() );
    if( errors.size() )
    {

        QString buffer = QString( errors.size() > 1 ? tr( "Errors occured while parsing files.\n"):tr("An error occured while parsing files.\n") );
        buffer += XmlError::toString( errors );
        InformationDialog( nullptr, buffer ).exec();

        Base::Singleton::get().application<Application>()->idle();
        return;

    }

    // debug
    Debug::Throw() << "MainWindow::_mergeBackup - number of remote files: " << backupLogbook.children().size() << endl;
    Debug::Throw() << "MainWindow::_mergeBackup - number of remote entries: " << backupLogbook.entries().size() << endl;
    Debug::Throw() << "MainWindow::_mergeBackup - updating local from remote" << endl;

    // synchronize local with remote
    // retrieve map of duplicated entries
    auto duplicates( logbook_->synchronize( backupLogbook ) );
    Debug::Throw() << "MainWindow::_mergeBackup - number of duplicated entries: " << duplicates.size() << endl;

    // update possible EditionWindows when duplicated entries are found
    // delete the local duplicated entries
    for( auto&& iter = duplicates.begin(); iter != duplicates.end(); ++iter )
    {

        // display the new entry in all matching edit frames
        for( const auto& window:Base::KeySet<EditionWindow>( iter.key() ) )
        { window->displayEntry( iter.value() ); }

        delete iter.key();

    }

    // reinitialize lists
    _resetKeywordList();

    // reset selected keyword
    _keywordSelectionChanged( keywordList_->selectionModel()->currentIndex() );

    // reset attachments
    resetAttachmentWindow();

    // retrieve last modified entry
    auto entries( logbook_->entries() );
    auto iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    entryList_->setFocus();

    // write local logbook
    if( !logbook_->file().isEmpty() ) save();

    // idle
    Base::Singleton::get().application<Application>()->idle();
    statusbar_->label().clear();

    return;

}

//_______________________________________________
void MainWindow::_reorganize()
{
    Debug::Throw( QStringLiteral("MainWindow::_reorganize.\n") );

    if( !logbook_ )
    {
        InformationDialog( this, tr( "No valid logbook. <Reorganize> canceled." ) ).exec();
        return;
    }

    // retrieve all entries
    auto entries( logbook_->entries() );

    // clear all logbook-to-entry associations
    logbook_->removeAssociatedKeys<LogEntry>();
    for( const auto& logbook:logbook_->children() )
    { logbook->removeAssociatedKeys<LogEntry>(); }

    // put entry set into a list and sort by creation time.
    // First entry must the oldest
    QList<LogEntry*> entryList( entries.toList() );
    std::sort( entryList.begin(), entryList.end(), LogEntry::FirstCreatedFTor() );

    // put entries in logbook
    for( const auto& entry:entryList )
    {
        auto logbook( logbook_->latestChild() );
        if( entry->isAssociated( logbook.get() ) )
        {

            // redo the association to the logbook, but do not mark logbook as modified
            Base::Key::associate( entry, logbook.get() );

        } else {

            // mark original logbooks as modified
            for( const auto& logbook:Base::KeySet<Logbook>(entry) )
            { logbook->setModified( true ); }

            // clear association to old logbook
            entry->clearAssociations<Logbook>();

            // associate to this logbook
            Base::Key::associate( entry, logbook.get() );

            // mark logbook as modified
            logbook->setModified( true );

        }

    }

    // remove empty logbooks
    logbook_->removeEmptyChildren();

    // redo fileChecker registration
    fileCheck_->clear();
    fileCheck_->registerLogbook( logbook_.get() );

    // save
    logbook_->setModified( true );
    if( !logbook_->file().isEmpty() ) save();

}

//_______________________________________________
void MainWindow::_showDuplicatedEntries()
{
    Debug::Throw( QStringLiteral("MainWindow::_showDuplicatedEntries.\n") );

    // keep track of the last visible entry
    LogEntry *lastVisibleEntry( nullptr );

    // keep track of current index
    QModelIndex currentIndex( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( currentIndex.isValid() ? entryModel_.get( currentIndex ):nullptr );

    // keep track of found entries
    int found( 0 );

    // retrieve all logbook entries
    Base::KeySet<LogEntry> entries( logbook_->entries() );
    Base::KeySet<LogEntry> turnedOffEntries;
    for( const auto& entry:entries )
    {

        // if entry is already hidden, skipp
        if( !entry->isSelected() ) continue;

        // check duplicated entries
        int duplicates( std::count_if( entries.begin(), entries.end(), LogEntry::SameCreationFTor( entry->creation() ) ) );
        if( duplicates < 2 ) {

            entry->setFindSelected( false );
            turnedOffEntries.insert( entry );

        } else {

            found++;
            lastVisibleEntry = entry;

        }

    }

    if( !found )
    {

        InformationDialog( this, tr( "No duplicated entries found." ) ).centerOnParent().exec();

        // reset flag for the turned off entries to true
        for( const auto& entry:turnedOffEntries )
        { entry->setFindSelected( true ); }

        return;
    }

    // reinitialize logEntry list
    _resetKeywordList();
    _resetLogEntryList();

    // if EditionWindow current entry is visible, select it;
    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( lastVisibleEntry ) selectEntry( lastVisibleEntry );

    return;
}

//_______________________________________________
void MainWindow::_viewLogbookStatistics()
{
    Debug::Throw( QStringLiteral("MainWindow::_viewLogbookStatistics.\n") );

    if( !logbook_ )
    {
        InformationDialog( this, tr( "No logbook opened. <View Logbook Statistics> canceled." ) ).exec();
        return;
    }

    LogbookStatisticsDialog( this, logbook_.get() ).centerOnWidget( qApp->activeWindow() ).exec();

}

//_______________________________________________
void MainWindow::_editLogbookInformations()
{
    Debug::Throw( QStringLiteral("MainWindow::_editLogbookInformations.\n") );

    if( !logbook_ )
    {
        InformationDialog( this, tr( "No logbook opened. <Edit Logbook Informations> canceled." ) ).exec();
        return;
    }

    // create dialog
    LogbookInformationDialog dialog( this, logbook_.get() );
    if( !dialog.centerOnWidget( qApp->activeWindow() ).exec() ) return;

    // keep track of logbook modifications
    bool modified( false );

    modified |= logbook_->setTitle( dialog.title() );
    modified |= logbook_->setAuthor( dialog.author() );
    modified |= logbook_->setComments( dialog.comments() );
    modified |= logbook_->setReadOnly( dialog.readOnly() );

    // retrieve logbook directory
    File directory = dialog.attachmentDirectory();

    // check if fulldir is not a non directory existsing file
    if( directory.exists() &&  !directory.isDirectory() )
    {

        const QString buffer = tr( "File '%1' is not a directory." ).arg( directory );
        InformationDialog( this, buffer ).exec();

    } else modified |= logbook_->setDirectory( directory );


    // save Logbook, if needed
    if( modified ) logbook_->setModified( true );
    if( !logbook_->file().isEmpty() ) save();

    updateWindowTitle();

}

//_______________________________________________
void MainWindow::_closeEditionWindows( bool askForSave )
{
    Debug::Throw( QStringLiteral("MainWindow::_closeEditionWindows.\n") );

    // get all EditionWindows from MainWindow
    Base::KeySet<EditionWindow> windows( this );
    auto reply = _checkModifiedEntries( Base::KeySet<EditionWindow>( this ) );
    if( reply == AskForSaveDialog::Cancel ) return;
    else if( reply == AskForSaveDialog::Yes ) saveUnchecked();

    for( const auto& window:windows )
    { window->deleteLater(); }

    return;

}

//____________________________________________
void MainWindow::_findEntries() const
{

    Debug::Throw( QStringLiteral("MainWindow::_findEntries.\n") );

    // try set from current selection
    QString text;
    if( !( text = qApp->clipboard()->text( QClipboard::Selection ) ).isEmpty() )
    {
        const int maxLength( 1024 );
        text.truncate( maxLength );
        searchWidget_->setText( text );
    }

    // check panel visibility
    if( !searchWidget_->isVisible() ) searchWidget_->show();

    // change focus
    searchWidget_->editor().lineEdit()->selectAll();
    searchWidget_->editor().setFocus();

}

//____________________________________________
void MainWindow::_newEntry()
{

    Debug::Throw( QStringLiteral("MainWindow::_NewEntry.\n") );

    // retrieve associated EditionWindows, check if one matches the selected entry
    EditionWindow* editionWindow( nullptr );
    Base::KeySet<EditionWindow> frames( this );
    Base::KeySetIterator<EditionWindow> iterator( frames.get() );
    iterator.toBack();
    while( iterator.hasPrevious() )
    {
        EditionWindow* current( iterator.previous() );

        // skip closed editors
        if( !current->isClosed() ) continue;
        editionWindow = current;
        editionWindow->setIsClosed( false );
        editionWindow->setReadOnly( false );
        break;
    }

    if( !editionWindow )
    {
        // create new EditionWindow
        editionWindow = new EditionWindow( nullptr, false );
        editionWindow->setColorMenu( colorMenu_ );
        Base::Key::associate( this, editionWindow );
        connect( editionWindow, &EditionWindow::scratchFileCreated, this, &MainWindow::scratchFileCreated );
        connect( logbook_.get(), &Logbook::readOnlyChanged, editionWindow, &EditionWindow::updateReadOnlyState );
    }

    // force editionWindow show keyword flag
    editionWindow->setForceShowKeyword( !treeModeAction_->isChecked() );

    // call NewEntry for the selected frame
    editionWindow->newEntryAction().trigger();

    // show frame
    editionWindow->centerOnWidget( this );
    editionWindow->show();

}

//____________________________________________
void MainWindow::_editEntries()
{
    Debug::Throw( QStringLiteral("MainWindow::_EditEntries .\n") );

    // retrieve selected items; make sure they do not include the navigator
    auto selection( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No entry selected. <Edit Entries> canceled." ) ).exec();
        return;
    }

    // retrieve associated entry
    for( const auto& entry:selection )
    { _displayEntry( entry ); }

    return;

}

//____________________________________________
void MainWindow::_deleteEntries()
{
    Debug::Throw( QStringLiteral("MainWindow::_DeleteEntries .\n") );

    // retrieve selected rows;
    auto selectedIndexes( entryList_->selectionModel()->selectedRows() );

    // convert into LogEntry list
    LogEntryModel::List selection;
    for( const auto& index:selectedIndexes )
    {
        // check if index is not being edited
        if( entryModel_.editionEnabled() && index ==  entryModel_.editionIndex() )
        {

            InformationDialog( this, tr( "Software limitation: cannot delete an entry that is being edited. <Delete Entries> canceled." ) ).exec();
            return;

        } else selection.append( entryModel_.get( index ) );

    }

    // check selection size
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No entry selected. <Delete Entries> canceled." ) ).exec();
        return;
    }

    // ask confirmation
    const QString buffer = QString( selection.size() == 1 ? tr( "Delete selected entry" ):tr( "Delete selected entries" ) );
    QuestionDialog dialog( this, buffer );
    dialog.okButton().setText( tr( "Delete" ) );
    dialog.okButton().setIcon( IconEngine::get( IconNames::Delete ) );
    if( !dialog.exec() ) return;

    // retrieve associated entry
    const auto currentKeyword( this->currentKeyword() );
    for( const auto& entry:selection )
    {
        if( treeModeAction_->isChecked() )
        {

            entry->removeKeyword( currentKeyword );
            if( !entry->hasKeywords() ) deleteEntry( entry, false );
            else {

                // remove from entry model
                entryModel_.remove( entry );

                // set associated logbooks as modified
                for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
                { logbook->setModified( true ); }

            }

        } else deleteEntry( entry, false );
    }

    // Save logbook if needed
    if( !logbook_->file().isEmpty() ) save();

    return;

}

//_______________________________________________
void MainWindow::_displayEntry( LogEntry* entry )
{

    Debug::Throw( QStringLiteral("MainWindow::_displayEntry.\n") );

    // retrieve associated EditionWindows, check if one matches the selected entry
    EditionWindow *editionWindow( nullptr );
    Base::KeySet<EditionWindow> windows( this );
    for( const auto& window:windows )
    {

        // skip closed editors
        if( window->isClosed() ) continue;

        // check if EditionWindow is editable and match editor
        if( !window->isReadOnly() && window->entry() == entry )
        {
            editionWindow = window;
            editionWindow->uniconifyAction().trigger();
            return;
        }

    }

    // if no editionWindow is found, try re-used a closed editor
    if( !editionWindow )
    {

        // the order is reversed to start from latest
        Base::KeySetIterator<EditionWindow> iterator( windows.get() );
        iterator.toBack();
        while( iterator.hasPrevious() )
        {
            auto current( iterator.previous() );

            // skip closed editors
            if( !current->isClosed() ) continue;

            editionWindow = current;
            editionWindow->setIsClosed( false );
            editionWindow->setReadOnly( false );

            // also clear modification state
            editionWindow->setModified( false );

            // need to display entry before deleting sub views.
            if( treeModeAction_->isChecked() ) editionWindow->displayEntry( currentKeyword(), entry );
            else if( entry->hasKeywords() ) editionWindow->displayEntry( *entry->keywords().begin(), entry );
            else editionWindow->displayEntry( Keyword::Default, entry );

            break;
        }

    }

    // if no editionWindow is found create a new one
    if( !editionWindow )
    {
        editionWindow = new EditionWindow( nullptr, false );
        editionWindow->setColorMenu( colorMenu_ );
        Base::Key::associate( this, editionWindow );

        // need to display entry before deleting sub views.
        if( treeModeAction_->isChecked() ) editionWindow->displayEntry( currentKeyword(), entry );
        else if( entry->hasKeywords() ) editionWindow->displayEntry( *entry->keywords().begin(), entry );
        else editionWindow->displayEntry( Keyword::Default, entry );

        connect( editionWindow, &EditionWindow::scratchFileCreated, this, &MainWindow::scratchFileCreated );
        connect( logbook_.get(), &Logbook::readOnlyChanged, editionWindow, &EditionWindow::updateReadOnlyState );

    }

    editionWindow->setForceShowKeyword( !treeModeAction_->isChecked() );
    editionWindow->centerOnWidget( this );
    editionWindow->show();

}

//_______________________________________________
void MainWindow::_changeEntryTitle( LogEntry* entry, const QString &newTitle )
{
    Debug::Throw( QStringLiteral("MainWindow::_changeEntryTitle.\n") );

    // make sure that title was changed
    if( newTitle == entry->title() ) return;

    // update entry title
    entry->setTitle( newTitle );

    // update associated entries
    _updateEntryFrames( entry, TitleMask );

    // set logbooks as modified
    Base::KeySet<Logbook> logbooks( entry );
    for( const auto& logbook:logbooks ) logbook->setModified( true );

    // save Logbook
    if( logbook_ && !logbook_->file().isEmpty() ) save();

}

//_______________________________________________
void MainWindow::_changeEntryColor( QColor color )
{
    Debug::Throw( QStringLiteral("MainWindow::_changeEntryColor.\n") );

    // retrieve current selection
    auto selection( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No entry selected. <Change Entry Color> canceled." ) ).exec();
        return;
    }

    // retrieve associated entry
    for( const auto& entry:selection )
    {

        entry->setColor( color );
        entry->setModification( TimeStamp( entry->modification().unixTime()+1 ) );

        // update EditionWindow color
        for( const auto& window:Base::KeySet<EditionWindow>( entry ) )
        { if( !window->isClosed() ) window->displayColor(); }

        // set logbooks as modified
        Base::KeySet<Logbook> logbooks( entry );
        for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
        { logbook->setModified( true ); }

    }

    // update model
    entryModel_.add( selection );

    // save Logbook
    if( !logbook_->file().isEmpty() ) save();

}

//____________________________________________
void MainWindow::_entryInformation()
{
    // retrieve current selection
    auto selection( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
    if( selection.size() != 1 ) return;

    // create dialog
    LogEntryInformationDialog( this, selection.front() ).centerOnParent().exec();

}

//____________________________________________
void MainWindow::_newKeyword()
{

    Debug::Throw( QStringLiteral("MainWindow::_newKeyword.\n") );

    // create dialog
    EditKeywordDialog dialog( this );
    dialog.setWindowTitle( tr( "New Keyword" ) );

    for( const auto& keyword:keywordModel_.children() )
    { dialog.add( keyword ); }

    dialog.setKeyword( currentKeyword() );

    // map dialog
    if( !dialog.centerOnParent().exec() ) return;

    // retrieve keyword from line_edit
    Keyword keyword( dialog.keyword() );
    if( keyword != Keyword() )
    {
        keywordModel_.add( keyword );
        QModelIndex index( keywordModel_.index( keyword ) );
        keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }
}


//____________________________________________
void MainWindow::_deleteKeyword()
{
    Debug::Throw(QStringLiteral("MainWindow::_deleteKeyword.\n") );

    // check that keywordlist has selected item
    auto selectedIndexes( keywordList_->selectionModel()->selectedRows() );
    if( selectedIndexes.empty() )
    {
        InformationDialog( this, tr( "No keyword selected. <Delete Keyword> canceled." ) ).exec();
        return;
    }

    // store corresponding list of keywords
    KeywordModel::List keywords;
    for( const auto& index:selectedIndexes )
    { if( index.isValid() ) keywords.append( keywordModel_.get( index ) ); }

    // retrieve associated entries
    Base::KeySet<LogEntry> entries( logbook_->entries() );
    Base::KeySet<LogEntry> associatedEntries;
    for( const auto& keyword:keywords )
    {
        for( const auto& entry:entries )
        {
            for( const auto& entryKeyword:entry->keywords() )
            {
                if( entryKeyword.inherits( keyword ) )
                {
                    associatedEntries.insert( entry );
                    break;
                }
            }

        }

    }

    // create dialog
    DeleteKeywordDialog dialog( this, keywords, !associatedEntries.empty() );
    if( !dialog.centerOnParent().exec() ) return;

    if( dialog.moveEntries() && associatedEntries.size() )
    {

        Debug::Throw( QStringLiteral("MainWindow::_deleteKeyword - moving entries.\n") );
        for( const auto& keyword:keywords )
        { _renameKeyword( keyword, keyword.parent(), false ); }

    } else if( dialog.deleteEntries() ) {

        Debug::Throw( QStringLiteral("MainWindow::_deleteKeyword - deleting entries.\n") );

        for( const auto& entry:associatedEntries )
        {
            for( const auto& keyword:keywords )
            {
                auto entryKeywords( entry->keywords() );
                for( const auto& entryKeyword:entryKeywords )
                {
                    if( entryKeyword.inherits( keyword ) )
                    { entry->removeKeyword( entryKeyword ); }
                }

            }

            if( !entry->hasKeywords() ) deleteEntry( entry, false );
            else {

                // make sure entry is not selected any more
                // (this will be re-updated later, if needed when selecting updated keyword)
                entry->setKeywordSelected( false );

                // set associated logbooks as modified
                for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
                { logbook->setModified( true ); }

            }

        }

    }

    // reset keywords
    _resetKeywordList();

    // select last valid keyword parent
    KeywordModel::ListIterator iter( keywords );
    iter.toBack();
    while( iter.hasPrevious() )
    {
        const auto& keyword( iter.previous() );

        // retrieve index associated to parent keyword
        // if valid, select and break
        auto index( keywordModel_.index( keyword.parent() ) );
        if( index.isValid() )
        {
            keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            break;
        }

    }

    // reset log entry list
    _resetLogEntryList();

    // Save logbook
    if( !logbook_->file().isEmpty() ) save();

    return;

}

//____________________________________________
void MainWindow::_renameKeyword()
{
    Debug::Throw(QStringLiteral("MainWindow::_renameKeyword.\n") );

    // check that keywordlist has selected item
    auto current( keywordList_->selectionModel()->currentIndex() );
    if( !current.isValid() )
    {
        InformationDialog( this, tr( "No keyword selected. <Rename Keyword> canceled." ) ).exec();
        return;
    }

    keywordList_->edit( current );
    return;

}

//____________________________________________
void MainWindow::_confirmRenameKeyword( const Keyword& keyword, const Keyword& newKeyword )
{
    Debug::Throw(QStringLiteral("MainWindow::_confirmRenameKeyword.\n") );

    // check keywords are different
    if( keyword == newKeyword ) return;

    QMenu menu( this );
    menu.addActions( keywordChangedMenuActions_ );
    menu.ensurePolished();
    auto action( menu.exec( QCursor::pos() ) );
    if( action == keywordChangedMenuActions_[0] ) _renameKeyword( keyword, newKeyword, true );
    else return;

}


//____________________________________________
void MainWindow::_renameKeyword( const Keyword &keyword, const Keyword &newKeyword, bool updateSelection )
{

    Debug::Throw(QStringLiteral("MainWindow::_renameKeyword.\n") );

    // check keywords are different
    if( keyword == newKeyword ) return;

    // get entries matching the oldKeyword, change the keyword
    for( const auto& entry:logbook_->entries() )
    {

        /*
        if keyword to modify is a leading subset of current entry keyword,
        update entry with new keyword
        */
        bool modified = false;
        const auto keywords( entry->keywords() );
        for( auto entryKeyword:keywords )
        {
            if( entryKeyword.inherits( keyword ) )
            {
                entry->replaceKeyword( entryKeyword, Keyword( QString( entryKeyword.get() ).replace( keyword.get(), newKeyword.get() ) ) );
                modified = true;
            }
        }

        if( modified )
        {

            /* this is a kludge: add 1 second to the entry modification timeStamp to avoid loosing the
            keyword change when synchronizing logbooks, without having all entries modification time
            set to now() */
            entry->setModification( TimeStamp( entry->modification().unixTime()+1 ) );

            // update frames
            _updateEntryFrames( entry, KeywordMask );

            // set associated logbooks as modified
            for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
            { logbook->setModified( true ); }

        }

    }

    // reset lists
    _resetKeywordList();
    if( updateSelection )
    {

        // make sure parent keyword index is expanded
        auto parentIndex( keywordModel_.index( newKeyword.parent() ) );
        if( parentIndex.isValid() ) keywordList_->setExpanded( parentIndex, true );

        // retrieve current index, and select
        auto index( keywordModel_.index( newKeyword ) );
        keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->scrollTo( index );
    }

    _resetLogEntryList();

    // Save logbook if needed
    if( !logbook_->file().isEmpty() ) save();

}

//____________________________________________
void MainWindow::_renameEntryKeyword()
{
    Debug::Throw(QStringLiteral("MainWindow::_renameEntryKeyword.\n") );

    // retrieve current selection
    auto selection( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No entry selected. <Rename Entry Keyword> canceled." ) ).exec();
        return;
    }

    // check that current keyword make sense
    if( !keywordList_->selectionModel()->currentIndex().isValid() )
    {
        InformationDialog( this, tr( "No keyword selected. <Rename Entry Keyword> canceled." ) ).exec();
        return;
    }

    // get current selected keyword
    Keyword keyword( keywordModel_.get( keywordList_->selectionModel()->currentIndex() ) );

    // create dialog
    EditKeywordDialog dialog( this );
    dialog.setWindowTitle( tr( "Edit Keyword" ) );

    for( const auto& keyword:keywordModel_.children() )
    { dialog.add( keyword ); }

    dialog.setKeyword( keyword );

    // map dialog
    if( !dialog.centerOnParent().exec() ) return;

    // check if keyword was changed
    Keyword newKeyword( dialog.keyword() );
    if( keyword == newKeyword ) return;

    // change keyword for all entries that match the old one
    _renameEntryKeyword( newKeyword );

}

//_______________________________________________
void MainWindow::_confirmRenameEntryKeyword( const Keyword &newKeyword )
{

    Debug::Throw() << "MainWindow::_confirmRenameEntryKeyword - newKeyword: " << newKeyword << endl;

    QMenu menu( this );
    menu.addActions( entryKeywordChangedMenuActions_ );
    menu.ensurePolished();
    QAction* action( menu.exec( QCursor::pos() ) );
    if( action == entryKeywordChangedMenuActions_[0] ) _renameEntryKeyword( newKeyword );
    else if( action == entryKeywordChangedMenuActions_[1] ) _copyEntryKeyword( newKeyword );
    else if( action == entryKeywordChangedMenuActions_[2] ) _linkEntryKeyword( newKeyword );
    else return;

}

//_______________________________________________
void MainWindow::_renameEntryKeyword( const Keyword &newKeyword )
{

    Debug::Throw() << "MainWindow::_renameEntryKeyword - newKeyword: " << newKeyword << endl;

    const auto currentKeyword( this->currentKeyword() );
    if( treeModeAction_->isChecked() && newKeyword == currentKeyword ) return;

    // keep track of modified entries
    Base::KeySet<LogEntry> entries;

    // retrieve current selection
    for( const auto& entry:entryModel_.get( entryList_->selectionModel()->selectedRows() ) )
    {

        // change keyword and set as modified
        if( treeModeAction_->isChecked() ) entry->replaceKeyword( currentKeyword, newKeyword );
        else {

            // check if entry keyword has changed
            if( entry->hasKeywords() )
            {

                if( *entry->keywords().begin() == newKeyword ) continue;
                entry->replaceKeyword( *entry->keywords().begin(), newKeyword );

            } else entry->addKeyword( newKeyword );

        }

        /* this is a kludge: add 1 second to the entry modification timeStamp to avoid loosing the
        keyword change when synchronizing logbooks, without having all entries modification time
        set to now() */
        entry->setModification( TimeStamp( entry->modification().unixTime()+1 ) );

        // keep track of modified entries
        entries.insert( entry );

        // update frames
        _updateEntryFrames( entry, KeywordMask );

        // set associated logbooks as modified
        for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
        { logbook->setModified( true ); }

    }

    // update selection
    _updateSelection( newKeyword, entries );

    // Save logbook if needed
    if( !logbook_->file().isEmpty() ) save();

    return;

}

//_______________________________________________
void MainWindow::_copyEntryKeyword( const Keyword &newKeyword )
{

    Debug::Throw() << "MainWindow::_copyEntryKeyword - newKeyword: " << newKeyword << endl;

    const auto currentKeyword( this->currentKeyword() );
    if( treeModeAction_->isChecked() && newKeyword == currentKeyword ) return;

    // keep track of modified entries
    Base::KeySet<LogEntry> entries;

    // retrieve current selection
    for( const auto& entry:entryModel_.get( entryList_->selectionModel()->selectedRows() ) )
    {

        // change keyword and set as modified
        if( entry->keywords().contains( newKeyword ) ) continue;

        //* copy entry to new one
        auto newEntry = entry->copy();

        // assign creation and modification to now
        newEntry->setCreation( TimeStamp::now() );
        newEntry->setModification( TimeStamp::now() );

        //* set keyword
        newEntry->clearKeywords();
        newEntry->addKeyword( newKeyword );

        //* associate to logbook
        auto logbook( logbook_->latestChild() );
        logbook->setModified( true );
        Base::Key::associate( newEntry, logbook.get() );

        // keep track of modified entries
        entries.insert( newEntry );

    }

    _updateSelection( newKeyword, entries );

    // Save logbook if needed
    if( !logbook_->file().isEmpty() ) save();

    return;

}

//_______________________________________________
void MainWindow::_linkEntryKeyword( const Keyword &newKeyword )
{

    Debug::Throw() << "MainWindow::_linkEntryKeyword - newKeyword: " << newKeyword << endl;

    const auto currentKeyword( this->currentKeyword() );
    if( treeModeAction_->isChecked() && newKeyword == currentKeyword ) return;

    // keep track of modified entries
    Base::KeySet<LogEntry> entries;

    // retrieve current selection
    for( const auto& entry:entryModel_.get( entryList_->selectionModel()->selectedRows() ) )
    {

        // change keyword and set as modified
        if( entry->keywords().contains( newKeyword ) ) continue;
        else entry->addKeyword( newKeyword );

        /* this is a kludge: add 1 second to the entry modification timeStamp to avoid loosing the
        keyword change when synchronizing logbooks, without having all entries modification time
        set to now() */
        entry->setModification( TimeStamp( entry->modification().unixTime() + 1 ) );

        // keep track of modified entries
        entries.insert( entry );

        // set associated logbooks as modified
        for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
        { logbook->setModified( true ); }

    }

    _updateSelection( newKeyword, entries );

    // Save logbook if needed
    if( !logbook_->file().isEmpty() ) save();

    return;

}

//_______________________________________________
void MainWindow::_updateSelection( const Keyword &newKeyword, const Base::KeySet<LogEntry> &entries )
{

    // check if at least one entry is selected
    if( entries.empty() ) return;

    // reset lists
    _resetKeywordList();

    // make sure parent keyword index is expanded
    auto parentIndex( keywordModel_.index( newKeyword.parent() ) );
    if( parentIndex.isValid() ) keywordList_->setExpanded( parentIndex, true );

    // retrieve current index, and select
    auto index( keywordModel_.index( newKeyword ) );
    keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    keywordList_->scrollTo( index );

    // update entry selection
    _resetLogEntryList();

    // clear current selection
    entryList_->clearSelection();

    // select all modified entries
    QModelIndex lastIndex;
    for( const auto& entry:entries )
    {
        auto index( entryModel_.index( entry ) );
        if( index.isValid() )
        {
            lastIndex = index;
            entryList_->selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
        }
    }

    // update current index
    if( lastIndex.isValid() )
    {
        entryList_->selectionModel()->setCurrentIndex( lastIndex,  QItemSelectionModel::Select|QItemSelectionModel::Rows );
        entryList_->scrollTo( lastIndex );
    }

    return;

}

//_______________________________________________
void MainWindow::_keywordSelectionChanged( const QModelIndex& index )
{

    Debug::Throw( QStringLiteral("MainWindow::_keywordSelectionChanged.\n") );
    if( !logbook_ ) return;
    if( !index.isValid() ) return;

    Keyword keyword( keywordModel_.get( index ) );
    Debug::Throw() << "MainWindow::_keywordSelectionChanged - keyword: " << keyword << endl;

    if( treeModeAction_->isChecked() ) entryModel_.setCurrentKeyword( keyword );

    // keep track of the current selected entry
    auto currentIndex( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( currentIndex.isValid() ? entryModel_.get( currentIndex ):nullptr );

    // retrieve all logbook entries
    Base::KeySet<LogEntry> turnedOffEntries;
    for( const auto& entry:logbook_->entries() )
    { entry->setKeywordSelected( entry->keywords().contains( keyword ) ); }

    // reinitialize logEntry list
    _resetLogEntryList();

    // if EditionWindow current entry is visible, select it
    // otherwise, select last entry in model
    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( !entryModel_.empty() )  { selectEntry( entryModel_.get( entryModel_.index( entryModel_.rowCount()-1, 0 ) ) ); }

    return;
}

//_____________________________________________
void MainWindow::_updateKeywordActions()
{
    Debug::Throw( QStringLiteral("MainWindow::_updateKeywordActions.\n") );

    const bool readOnly( logbook_ && logbook_->isReadOnly() );
    newKeywordAction_->setEnabled( !readOnly );
    editKeywordAction_->setEnabled( !readOnly && keywordList_->selectionModel()->currentIndex().isValid() );
    deleteKeywordAction_->setEnabled( !( readOnly || keywordList_->selectionModel()->selectedRows().empty() ) );

    return;
}

//_____________________________________________
void MainWindow::_updateEntryActions()
{
    Debug::Throw( QStringLiteral("MainWindow::_updateEntryActions.\n") );
    const bool readOnly( logbook_ && logbook_->isReadOnly() );
    const int selectedEntries( entryList_->selectionModel()->selectedRows().size() );
    const bool hasSelection( selectedEntries > 0 );

    if( selectedEntries > 1 )
    {

        editEntryAction_->setText( tr( "Edit Entries" ) );
        deleteEntryAction_->setText( tr( "Delete Entries" ) );

    } else {

        editEntryAction_->setText( tr( "Edit Entry" ) );
        deleteEntryAction_->setText( tr( "Delete Entry" ) );

    }

    editEntryAction_->setEnabled( hasSelection );
    deleteEntryAction_->setEnabled( !readOnly && hasSelection );
    entryColorAction_->setEnabled( !readOnly && hasSelection );
    entryColorButton_->setEnabled( !readOnly && hasSelection );
    entryKeywordAction_->setEnabled( !readOnly && hasSelection );
    editEntryTitleAction_->setEnabled( !readOnly && hasSelection );

    entryInformationAction_->setEnabled( selectedEntries == 1 );

    newEntryAction_->setEnabled( !readOnly );


    return;
}

//_____________________________________________
void MainWindow::_updateReadOnlyState()
{
    Debug::Throw( QStringLiteral("MainWindow::_updateReadOnlyState.\n") );

    const bool readOnly( logbook_ && logbook_->isReadOnly() );
    synchronizeAction_->setEnabled( !readOnly );
    reorganizeAction_->setEnabled( !readOnly );
    saveAction_->setEnabled( !readOnly );
    saveForcedAction_->setEnabled( !readOnly );

    // clear the AttachmentWindow
    AttachmentWindow &attachmentWindow( Base::Singleton::get().application<Application>()->attachmentWindow() );
    attachmentWindow.frame().setReadOnly( readOnly );

}

//_______________________________________________
void MainWindow::_storeSortMethod( int column, Qt::SortOrder order  )
{

    Debug::Throw()
        << "MainWindow::_storeSortMethod -"
        << " column: " << column
        << " order: " << order
        << endl ;

    if( !logbook_ ) return;

    bool changed( false );
    switch( column ) {

        case LogEntryModel::Color: changed = logbook_->setSortMethod( Logbook::SortMethod::SortColor ); break;
        case LogEntryModel::Title: changed = logbook_->setSortMethod( Logbook::SortMethod::SortTitle ); break;
        case LogEntryModel::Creation: changed = logbook_->setSortMethod( Logbook::SortMethod::SortCreation ); break;
        case LogEntryModel::Modification: changed = logbook_->setSortMethod( Logbook::SortMethod::SortModification ); break;
        case LogEntryModel::Author: changed = logbook_->setSortMethod( Logbook::SortMethod::SortAuthor ); break;
        default: return;

    }

    // Save logbook if needed
    changed |= logbook_->setSortOrder( int( order ) );
    if( changed && !logbook_->file().isEmpty() ) save();

}


//____________________________________________________________
void MainWindow::_entryItemActivated( const QModelIndex& index )
{
    // stop edition timer
    entryModel_.setEditionIndex( QModelIndex() );
    editionTimer_.stop();
    if( index.isValid() ) _displayEntry( entryModel_.get( index ) );
}

//____________________________________________________________
void MainWindow::_entryItemClicked( const QModelIndex& index )
{

    // do nothing if index do not correspond to an entry title
    if( !index.isValid() ) return;

    // do nothing if index is not already selected
    if( !entryList_->selectionModel()->isSelected( index ) ) return;

    if( !( index.column() == LogEntryModel::Title ||  index.column() == LogEntryModel::Key ) )
    { return; }

    // compare to model edition index
    if( index == entryModel_.editionIndex() ) editionTimer_.start( editionDelay_, this );
    else entryModel_.setEditionIndex( index );

}

//_______________________________________________
void MainWindow::_entryDataChanged( const QModelIndex& index )
{
    Debug::Throw( QStringLiteral("MainWindow::_entryDataChanged.\n") );

    if( !index.isValid() ) return;
    auto entry( entryModel_.get( index ) );

    Mask mask;
    if( index.column() == LogEntryModel::Title ) mask |= TitleMask;
    else if( index.column() == LogEntryModel::Key ) mask |= KeywordMask;

    // update associated EditionWindows
    _updateEntryFrames( entry, mask );

    // set logbooks as modified
    for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
    { logbook->setModified( true ); }

    // save Logbook
    if( logbook_ && !logbook_->file().isEmpty() ) save();

}

//________________________________________
void MainWindow::_startEntryEdition()
{

    Debug::Throw( QStringLiteral("MainWindow::_startEntryEdition\n") );

    // get current index and check validity
    const auto& index = entryList_->currentIndex();
    if( !index.isValid() ) return;

    // make sure 'title' index is selected
    // index = entryModel_.index( index.row(), LogEntryModel::Title );
    if( !( index.column() == LogEntryModel::Key || index.column() == LogEntryModel::Title ) )
    { return; }

    // enable model edition
    entryModel_.setEditionIndex( index );
    entryModel_.setEditionEnabled( true );

    // edit item
    entryList_->edit( index );

}

//_____________________________________________
void MainWindow::_showMonitoredFiles()
{

    Debug::Throw( QStringLiteral("MainWindow::_showMonitoredFiles.\n") );
    FileCheckDialog dialog( qApp->activeWindow() );
    dialog.setFiles( fileCheck_->fileSystemWatcher().files() );
    dialog.exec();

}

//_____________________________________________
void MainWindow::_toggleTreeMode( bool value )
{

    Debug::Throw() << "MainWindow::_toggleTreeMode - " << ( value ? "true":"false" ) << endl;

    // show/hide keyword list
    keywordContainer_->setVisible( value );

    // change drag mode enability
    entryList_->setDragEnabled( value );

    // get current entry
    LogEntry* currentEntry( nullptr );
    auto index( entryList_->selectionModel()->currentIndex() );
    if( index.isValid() ) currentEntry = entryModel_.get( index );

    // update keyword list
    if( value ) _resetKeywordList();
    else entryModel_.setCurrentKeyword( Keyword() );

    // update log entry list
    _resetLogEntryList();

    // change keyword column visibility
    entryList_->setColumnHidden( LogEntryModel::Key, value );
    entryList_->resizeColumns();

    // keyword toolbar visibility action
    keywordToolBar_->visibilityAction().setEnabled( value );

    // if hiding keyword, first need ask for save
    auto windows( Base::KeySet<EditionWindow>( this ) );
    if( value )
    {
        _checkModifiedEntries( windows );
        saveUnchecked();
    }

    for( const auto& window:windows )
    { window->setForceShowKeyword( !value ); }

    // make sure entry is visible
    if( currentEntry )
    {
        if( value && currentEntry->hasKeywords() )
        {
            // select keyword
            auto index( keywordModel_.index( *currentEntry->keywords().begin() ) );
            keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            keywordList_->scrollTo( index );
        }

        // select entry
        auto index( entryModel_.index( currentEntry ) );
        entryList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        entryList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        entryList_->scrollTo( index );

    }


    // save to options
    XmlOptions::get().set<bool>( QStringLiteral("USE_TREE"), value );

}

//_______________________________________________
void MainWindow::_updateConfiguration()
{

    Debug::Throw( QStringLiteral("MainWindow::_updateConfiguration.\n") );

    resize( sizeHint() );

    // compression
    if( logbook_ ) logbook_->setUseCompression( XmlOptions::get().get<bool>( QStringLiteral("USE_COMPRESSION") ) );

    // autoSave
    autoSaveDelay_ = 1000*XmlOptions::get().get<int>( QStringLiteral("AUTO_SAVE_ITV") );
    bool autosave( XmlOptions::get().get<bool>( QStringLiteral("AUTO_SAVE") ) );
    if( autosave ) autosaveTimer_.start( autoSaveDelay_, this );
    else autosaveTimer_.stop();

    // colors
    colorMenu_->reset();
    for( const auto& color:XmlOptions::get().specialOptions( QStringLiteral("COLOR") ) )
    { colorMenu_->add( color.get<Base::Color>() ); }

    // max number of recent entries
    maxRecentEntries_ = XmlOptions::get().get<int>( QStringLiteral("MAX_RECENT_ENTRIES") );

    // tree mode
    treeModeAction_->setChecked( XmlOptions::get().get<bool>( QStringLiteral("USE_TREE") ) );

}

//_______________________________________________
LogEntryModel::List MainWindow::_entries( LogEntryPrintSelectionWidget::Mode mode )
{

    switch( mode )
    {
        default:
        case LogEntryPrintSelectionWidget::Mode::AllEntries:
        return Base::makeT<LogEntryModel::List>( logbook_->entries() );

        case LogEntryPrintSelectionWidget::Mode::VisibleEntries:
        return entryModel_.get();

        case LogEntryPrintSelectionWidget::Mode::SelectedEntries:
        return entryModel_.get( entryList_->selectionModel()->selectedRows() );

    }

}
