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
#include "CustomToolBar.h"
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
#include "Menu.h"
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
    Counter( "MainWindow" ),
    workingDirectory_( Util::workingDirectory() )
{
    Debug::Throw( "MainWindow::MainWindow.\n" );
    setOptionName( "MAIN_WINDOW" );
    updateWindowTitle();

    // file checker
    fileCheck_ = new FileCheck( this );
    connect( &fileCheck(), SIGNAL(filesModified(FileCheck::DataSet)), SLOT(_filesModified(FileCheck::DataSet)) );

    // main widget
    QWidget* main = new QWidget( this );
    setCentralWidget( main );

    // local layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(2);
    main->setLayout( layout );

    // splitter for KeywordList/LogEntryList
    QSplitter* splitter( new QSplitter( main ) );
    layout->addWidget( splitter, 1 );
    splitter->setOrientation( Qt::Horizontal );

    // search widget
    layout->addWidget( searchWidget_ = new SearchWidget( main ) );
    searchWidget_->hide();

    connect( searchWidget_, SIGNAL(selectEntries(QString,SearchWidget::SearchModes)), SLOT(selectEntries(QString,SearchWidget::SearchModes)) );
    connect( searchWidget_, SIGNAL(showAllEntries()), SLOT(showAllEntries()) );

    // status bar
    setStatusBar( statusbar_ = new ProgressStatusBar( this ) );
    statusbar_->setProgressBar( new ProgressBar() );
    statusbar_->addClock();
    connect( this, SIGNAL(messageAvailable(QString)), &statusbar_->label(), SLOT(setTextAndUpdate(QString)) );
    connect( this, SIGNAL(messageAvailable(QString)), &statusbar_->progressBar(), SLOT(setText(QString)) );

    // global scope actions
    _installActions();

    // aditional actions from application
    auto application( Singleton::get().application<Application>() );
    addAction( &application->closeAction() );

    // Keyword container
    keywordContainer_ = new QWidget();

    // set layout
    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing( 5 );
    keywordContainer_->setLayout( vLayout );

    keywordToolBar_ = new CustomToolBar( tr( "Keywords" ), keywordContainer_, "KEYWORD_TOOLBAR" );
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
    keywordList_->setOptionName( "KEYWORD_LIST" );

    // default width from options, if found
    if( XmlOptions::get().contains( "KEYWORD_LIST_WIDTH" ) )
    { keywordList_->setDefaultWidth( XmlOptions::get().get<int>( "KEYWORD_LIST_WIDTH" ) ); }

    // replace item delegate
    if( keywordList_->itemDelegate() ) keywordList_->itemDelegate()->deleteLater();
    keywordList_->setItemDelegate( new TextEditionDelegate( this ) );

    // update LogEntryList when keyword selection change
    connect( keywordList_->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(_keywordSelectionChanged(QModelIndex)) );
    connect( keywordList_->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(_updateKeywordActions()) );
    _updateKeywordActions();

    // rename selected entries when KeywordChanged is emitted with a single argument.
    // this correspond to drag and drop action from the logEntryList in the KeywordList
    connect( &keywordModel_, SIGNAL(entryKeywordChangeRequest(Keyword)), SLOT(_confirmRenameEntryKeyword(Keyword)) );

    // rename all entries matching first keyword the second. This correspond to
    // drag and drop inside the keyword list, or to direct edition of a keyword list item.
    connect( &keywordModel_, SIGNAL(keywordChangeRequest(Keyword,Keyword)), SLOT(_confirmRenameKeyword(Keyword,Keyword)) );
    connect( &keywordModel_, SIGNAL(keywordChanged(Keyword,Keyword)), SLOT(_renameKeyword(Keyword,Keyword)) );

    {
        // popup menu for keyword list
        ContextMenu* menu = new ContextMenu( keywordList_ );
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
    QWidget* right = new QWidget();

    vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing( 5 );
    right->setLayout( vLayout );

    entryToolBar_ = new CustomToolBar( tr( "Entries" ), right, "ENTRY_TOOLBAR" );
    entryToolBar_->setTransparent( true );
    entryToolBar_->setAppearsInMenu( true );
    vLayout->addWidget( entryToolBar_ );

    // entry actions
    entryToolBar_->addAction( newEntryAction_ );
    entryToolBar_->addAction( editEntryAction_ );

    // need to use a button to be able to set the popup mode
    entryColorButton_ = new QToolButton(0);
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
    entryList_->setOptionName( "ENTRY_LIST" );
    entryList_->setColumnHidden( LogEntryModel::Key, true );
    entryList_->lockColumnVisibility( LogEntryModel::Key );
    entryList_->setColumnHidden( LogEntryModel::Title, false );
    entryList_->lockColumnVisibility( LogEntryModel::Title );

    #if QT_VERSION >= 0x050000
    entryList_->header()->setSectionResizeMode(LogEntryModel::Creation, QHeaderView::Stretch);
    entryList_->header()->setSectionResizeMode(LogEntryModel::Modification, QHeaderView::Stretch);
    #else
    entryList_->header()->setResizeMode(LogEntryModel::Creation, QHeaderView::Stretch);
    entryList_->header()->setResizeMode(LogEntryModel::Modification, QHeaderView::Stretch);
    #endif

    // replace item delegate
    if( entryList_->itemDelegate() ) entryList_->itemDelegate()->deleteLater();
    entryList_->setItemDelegate( new TextEditionDelegate( this ) );

    connect( entryList_->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), SLOT(_storeSortMethod(int,Qt::SortOrder)) );
    connect( entryList_->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(_updateEntryActions()) );
    connect( entryList_, SIGNAL(activated(QModelIndex)), SLOT(_entryItemActivated(QModelIndex)) );
    connect( entryList_, SIGNAL(clicked(QModelIndex)), SLOT(_entryItemClicked(QModelIndex)) );
    _updateEntryActions();

    connect( &entryModel_, SIGNAL(layoutChanged()), entryList_, SLOT(resizeColumns()) );
    connect( &entryModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(_entryDataChanged(QModelIndex)) );

    /*
    add the deleteEntryAction to the list,
    so that the corresponding shortcut gets activated whenever it is pressed
    while the list has focus
    */
    entryList_->addAction( deleteEntryAction_ );
    entryList_->addAction( editEntryTitleAction_ );

    {
        // popup menu for list
        ContextMenu* menu = new ContextMenu( entryList_ );
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

    connect( splitter, SIGNAL(splitterMoved(int,int)), SLOT(_splitterMoved()) );

    // main menu
    menu_ = new Menu( this , this );
    setMenuBar( menu_ );
    connect( menu_, SIGNAL(entrySelected(LogEntry*)), SLOT(selectEntry(LogEntry*)) );
    connect( menu_, SIGNAL(entrySelected(LogEntry*)), SLOT(_displayEntry(LogEntry*)) );

    // configuration
    connect( application, SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    _updateConfiguration();
    _updateKeywordActions();
    _updateEntryActions();
    _updateReadOnlyState();
}

//___________________________________________________________
MainWindow::~MainWindow( void )
{
    Debug::Throw( "MainWindow::~MainWindow.\n" );
    if( logbook_ ) delete logbook_;
}

//___________________________________________________________
void MainWindow::createDefaultLogbook( void )
{

    Debug::Throw( "MainWindow::_newLogbook.\n" );

    // check current logbook
    if( logbook_ && logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create a new logbook, with no file
    setLogbook( File() );
    Q_CHECK_PTR( logbook_ );

    logbook_->setTitle(  Logbook::NoTitle );
    logbook_->setAuthor( XmlOptions::get().raw( "USER" ) );
    logbook_->setDirectory( workingDirectory() );

    logbook_->setComments( QString( tr( "Default logbook created automatically on %1" ) ).arg( TimeStamp::now().toString( TimeStamp::Long ) ) );

}

//_______________________________________________
bool MainWindow::setLogbook( File file )
{

    Debug::Throw() << "MainWindow::SetLogbook - logbook: \"" << file << "\"" << endl;

    // reset current logbook
    if( logbook_ ) reset();

    // clear file checker
    fileCheck().clear();

    // create new logbook
    logbook_ = new Logbook();
    logbook_->setUseCompression( XmlOptions::get().get<bool>( "USE_COMPRESSION" ) );

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
        menu().recentFilesMenu().setCurrentFile( file );

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

    connect( logbook_, SIGNAL(maximumProgressAvailable(int)), statusbar_, SLOT(showProgressBar()) );
    connect( logbook_, SIGNAL(maximumProgressAvailable(int)), &statusbar_->progressBar(), SLOT(setMaximum(int)) );
    connect( logbook_, SIGNAL(progressAvailable(int)), &statusbar_->progressBar(), SLOT(addToProgress(int)) );
    connect( logbook_, SIGNAL(messageAvailable(QString)), SIGNAL(messageAvailable(QString)) );

    connect( logbook_, SIGNAL(readOnlyChanged(bool)), SLOT(_updateEntryActions()) );
    connect( logbook_, SIGNAL(readOnlyChanged(bool)), SLOT(_updateKeywordActions()) );
    connect( logbook_, SIGNAL(readOnlyChanged(bool)), SLOT(_updateReadOnlyState()) );

    // one need to disable everything in the window
    // to prevent user to interact with the application while loading
    _setEnabled( false );
    logbook_->read();
    _setEnabled( true );

    Debug::Throw( "MainWindow::setLogbook - finished reading.\n" );

    // update listView with new entries
    _resetKeywordList();
    _resetLogEntryList();
    _loadColors();

    Debug::Throw( "MainWindow::setLogbook - lists set.\n" );

    // change sorting
    Qt::SortOrder sort_order( (Qt::SortOrder) logbook_->sortOrder() );
    Debug::Throw( "MainWindow::setLogbook - got sort order.\n" );

    switch( logbook_->sortMethod() )
    {
        case Logbook::SortColor: entryList_->sortByColumn( LogEntryModel::Color, sort_order ); break;
        case Logbook::SortTitle: entryList_->sortByColumn( LogEntryModel::Title, sort_order ); break;
        case Logbook::SortCreation: entryList_->sortByColumn( LogEntryModel::Creation, sort_order ); break;
        case Logbook::SortModification: entryList_->sortByColumn( LogEntryModel::Modification , sort_order); break;
        case Logbook::SortAuthor: entryList_->sortByColumn( LogEntryModel::Author, sort_order ); break;
        default: break;
    }

    Debug::Throw( "MainWindow::setLogbook - lists sorted.\n" );

    // update attachment frame
    resetAttachmentWindow();
    Debug::Throw( "MainWindow::setLogbook - attachment frame reset.\n" );

    // retrieve last modified entry
    Base::KeySet<LogEntry> entries( logbook_->entries() );
    auto iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    entryList_->setFocus();

    Debug::Throw( "MainWindow::setLogbook - entry selected.\n" );

    // see if logbook has parent file
    if( logbook_->parentFile().size() )
    {
        const QString buffer = QString( tr( "Warning: this logbook should be oppened via '%1' only." ) ).arg( logbook_->parentFile() );
        WarningDialog( this, buffer ).exec();
    }

    // see if logbook is read-only
    if( logbook_->isBackup() )
    {

        const QString buffer = QString(
            tr( "Warning: this logbook is a backup and is therefore read-only.\n"
            "All editing will be disabled until it is marked as writable again "
            "in the Logbook Information dialog." ) );

        WarningDialog( this, buffer ).exec();

    } else if( logbook_->isReadOnly() ) {

        const QString buffer = QString(
            tr( "Warning: this logbook is read-only.\n"
            "All editing will be disabled until it is marked as writable again "
            "in the Logbook Information dialog." ) );

        WarningDialog( this, buffer ).exec();

    }

    // store logbook directory for next open, save comment
    workingDirectory_ = File( logbook_->file() ).path();
    statusbar_->label().clear();
    statusbar_->showLabel();

    // register logbook to fileCheck
    fileCheck().registerLogbook( logbook_ );

    emit ready();

    // check errors
    auto errors( logbook_->xmlErrors() );
    if( errors.size() )
    {
        QString buffer( errors.size() > 1 ? tr( "Errors occured while parsing files.\n" ):tr("An error occured while parsing files.\n") );
        buffer += errors.toString();
        InformationDialog( 0, buffer ).exec();
    }

    // add opened file to OpenPrevious mennu.
    if( !logbook_->file().isEmpty() )
    { Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expand() ); }

    ignoreWarnings_ = false;

    _updateKeywordActions();
    _updateEntryActions();
    _updateReadOnlyState();

    updateWindowTitle();
    return true;
}

//_____________________________________________
void MainWindow::checkLogbookBackup( void )
{
    Debug::Throw( "MainWindow::checkLogbookBackup.\n" );

    // check logbook makes sense
    if( !logbook_ ) return;

    // check if oppened logbook needs backup
    if(
        XmlOptions::get().get<bool>( "AUTO_BACKUP" ) &&
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
void MainWindow::reset( void )
{

    Debug::Throw( "MainWindow::reset.\n" );
    if( logbook_ ) {

        // delete the logbook, all corresponding entries
        delete logbook_;
        logbook_ = 0;

    }

    // clear list of entries
    keywordModel_.clear();
    entryModel_.clear();

    // clear the AttachmentWindow
    Singleton::get().application<Application>()->attachmentWindow().frame().clear();

    // make all EditionWindows for deletion
    for( const auto& window:Base::KeySet<EditionWindow>( this ) )
    {
        window->setIsClosed( true );
        window->hide();
    }

    return;

}

//____________________________________________
AskForSaveDialog::ReturnCode MainWindow::askForSave( bool enableCancel )
{

    Debug::Throw( "MainWindow::askForSave.\n" );

    // create dialog
    AskForSaveDialog::ReturnCodes buttons( AskForSaveDialog::Yes | AskForSaveDialog::No );
    if( enableCancel ) buttons |= AskForSaveDialog::Cancel;

    // exec and check return code
    int state = AskForSaveDialog( this, tr( "Logbook has been modified. Save ?" ), buttons ).centerOnParent().exec();
    if( state == AskForSaveDialog::Yes ) save();
    return AskForSaveDialog::ReturnCode(state);
}

//_______________________________________________
void MainWindow::clearSelection( void )
{ entryList_->clearSelection(); }

//_______________________________________________
void MainWindow::selectEntry( LogEntry* entry )
{
    Debug::Throw("MainWindow::selectEntry.\n" );

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
    Debug::Throw("MainWindow::selectEntry.\n" );

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
void MainWindow::updateEntry( Keyword keyword, LogEntry* entry, bool updateSelection )
{

    Debug::Throw( "MainWindow::updateEntry.\n" );

    // make sure keyword model contains all entry keywords
    for( const auto& keyword:entry->keywords() )
    { keywordModel_.add( keyword ); }

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
    Debug::Throw( "MainWindow::deleteEntry.\n" );

    Q_CHECK_PTR( entry );

    // get associated attachments
    for( const auto& attachment:Base::KeySet<Attachment>(entry) )
    {

        // retrieve/delete associated attachment frames
        for( const auto& frame:Base::KeySet<AttachmentFrame>(attachment) )
        { frame->remove( *attachment ); }

        // delete attachment
        delete attachment;

    };

    // remove from model
    entryModel_.remove( entry );

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

    // set logbooks as modified
    for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
    { logbook->setModified( true ); }

    // delete entry
    delete entry;

    // save
    if( save && !logbook_->file().isEmpty() )
    { this->save(); }

    return;

}

//_______________________________________________
bool MainWindow::lockEntry( LogEntry* entry ) const
{
    Debug::Throw( "MainWindow::lockEntry.\n" );
    if( !entry ) return true;

    // check whether there are modified editors around and ask for save
    Base::KeySet<EditionWindow> windows( entry );
    if( _checkModifiedEntries( windows, true ) == AskForSaveDialog::Cancel ) return false;

    // mark modified editors as read only
    for( const auto& window:windows ) window->setReadOnly( true );

    return true;
}

//_______________________________________________
LogEntry* MainWindow::previousEntry( LogEntry* entry, bool updateSelection )
{

    Debug::Throw( "MainWindow::previousEntry.\n" );
    QModelIndex index( entryModel_.index( entry ) );
    if( !( index.isValid() && index.row() > 0 ) ) return 0;

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

    Debug::Throw( "MainWindow::nextEntry.\n" );
    QModelIndex index( entryModel_.index( entry ) );
    if( !( index.isValid() && index.row()+1 < entryModel_.rowCount() ) ) return 0;

    QModelIndex nextIndex( entryModel_.index( index.row()+1, index.column() ) );
    if( updateSelection )
    {
        entryList_->selectionModel()->select( nextIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        entryList_->selectionModel()->setCurrentIndex( nextIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }

    return entryModel_.get( nextIndex );

}

//_______________________________________________
void MainWindow::resetAttachmentWindow( void ) const
{

    Debug::Throw( "MainWindow::resetAttachmentWindow.\n" );

    // clear the AttachmentWindow
    AttachmentWindow &attachmentWindow( Singleton::get().application<Application>()->attachmentWindow() );
    attachmentWindow.frame().clear();

    // check current logbook
    if( !logbook_ ) return;

    // retrieve logbook attachments, adds to AttachmentWindow
    Base::KeySet<Attachment> attachments( logbook_->attachments() );
    attachmentWindow.frame().add( attachments.toList() );

    return;

}

//_______________________________________________
Keyword MainWindow::currentKeyword( void ) const
{
    Debug::Throw( "MainWindow::currentKeyword.\n" );
    QModelIndex index( keywordList_->selectionModel()->currentIndex() );
    return index.isValid() ? keywordModel_.get( index ) : Keyword();
}

//_______________________________________________
void MainWindow::saveUnchecked( void )
{

    Debug::Throw( "MainWindow::save.\n" );
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
    File fullname = logbook_->file().expand();
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

        File path( fullname.path() );
        if( !path.isDirectory() ) {
            InformationDialog( this, tr( "Selected path is not vallid. <Save Logbook> canceled." ) ).exec();
            return;
        }

    }

    // write logbook to file, retrieve result
    Singleton::get().application<Application>()->busy();
    _setEnabled( false );

    logbook_->truncateRecentEntriesList( maxRecentEntries_ );

    logbook_->write();
    Singleton::get().application<Application>()->idle();
    _setEnabled( true );

    updateWindowTitle();

    // update StateFrame
    statusbar_->label().clear();
    statusbar_->showLabel();

    // add new file to openPreviousMenu
    if( !logbook_->file().isEmpty() ) Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expand() );

    // reset ignore_warning flag
    ignoreWarnings_ = false;

    return;

}

//_______________________________________________
void MainWindow::save( bool confirmEntries )
{

    Debug::Throw( "MainWindow::save.\n" );

    // check logbook
    if( !logbook_ )
    {
        InformationDialog( this, tr( "No Logbook opened. <Save> canceled." ) ).exec();
        return;
    }

    if( !confirmEntries ) confirmEntries_ = false;
    if( _checkModifiedEntries( Base::KeySet<EditionWindow>( this ), confirmEntries_ ) == AskForSaveDialog::Cancel ) return;

    saveUnchecked();
    confirmEntries_ = true;

}

//_______________________________________________
void MainWindow::selectEntries( QString selection, SearchWidget::SearchModes mode )
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
        InformationDialog( this, tr( "At least one search field must be selected" ) ).centerOnParent().exec();
        return;
    }

    // number of found items
    unsigned int found( 0 );
    unsigned int total( 0 );

    // keep track of the last visible entry
    LogEntry *lastVisibleEntry( 0 );

    // keep track of the current selected entry
    QModelIndex currentIndex( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( currentIndex.isValid() ? entryModel_.get( currentIndex ):0 );

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
void MainWindow::showAllEntries( void )
{
    Debug::Throw( "MainWindow::showAllEntries.\n" );

    // keep track of the current selected entry
    QModelIndex currentIndex( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( currentIndex.isValid() ? entryModel_.get( currentIndex ):0 );

    // set all logbook entries to find_visible
    for( const auto& entry:logbook_->entries() )
    { entry->setFindSelected( true ); }

    // reinitialize logEntry list
    _resetKeywordList();
    _resetLogEntryList();

    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( entryModel_.rowCount() ) selectEntry( entryModel_.get( entryModel_.index( entryModel_.rowCount()-1, 0 ) ) );

    statusbar_->label().setText( "" );

    entryList_->setFocus();
    return;

}

//____________________________________
void MainWindow::closeEvent( QCloseEvent *event )
{
    Debug::Throw( "MainWindow::closeEvent.\n" );
    event->accept();
    Singleton::get().application<Application>()->closeAction().trigger();
}

//_______________________________________________________
void MainWindow::timerEvent( QTimerEvent* event )
{

    if( event->timerId() == resizeTimer_.timerId() )
    {

        // stop timer and save time
        resizeTimer_.stop();
        XmlOptions::get().set<int>( "KEYWORD_LIST_WIDTH", keywordList_->width() );

    } else if(event->timerId() == editionTimer_.timerId() ) {

        editionTimer_.stop();

        // check if current index is valid and was 'double-clicked'
        QModelIndex index( entryList_->currentIndex() );
        if( index.isValid() && index == entryModel_.editionIndex() )
        { _startEntryEdition(); }

    } else if(event->timerId() == autosaveTimer_.timerId() ) {

        _autoSave();

    } else return BaseMainWindow::timerEvent( event );

}

//________________________________________________
void MainWindow::contextMenuEvent( QContextMenuEvent* event )
{

    Debug::Throw( "MainWindow::contextMenuEvent.\n" );
    BaseMainWindow::contextMenuEvent( event );
    if( event->isAccepted() ) return;

    // if event was rejected it means it is outer of one of the
    // relevant window area. However here we want it to also be accepted
    // in the 'customized' keyword and entry toolbars.
    Debug::Throw( "MainWindow::contextMenuEvent - event rejected.\n" );

    // get child under widget
    bool accepted( false );
    auto child = childAt(event->pos());
    while (child && child != this)
    {
        if( child == keywordToolBar_ || child == entryToolBar_ )
        {
            accepted = true;
            break;
        }

        child = child->parentWidget();
    }

    if( !accepted ) return;
    auto menu = createPopupMenu();
    menu->exec( event->globalPos() );
    menu->deleteLater();
    event->accept();

    return;

}

//_______________________________________________
void MainWindow::_installActions( void )
{

    Debug::Throw( "MainWindow::_installActions.\n" );
    uniconifyAction_ = new QAction( IconEngine::get( IconNames::Home ), tr( "Main Window" ), this );
    uniconifyAction_->setToolTip( tr( "Raise application main window" ) );
    connect( uniconifyAction_, SIGNAL(triggered()), SLOT(uniconify()) );

    newKeywordAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New Keyword" ), this );
    newKeywordAction_->setToolTip( tr( "Create a new keyword" ) );
    connect( newKeywordAction_, SIGNAL(triggered()), SLOT(_newKeyword()) );

    addAction( editKeywordAction_ = new QAction( IconEngine::get( IconNames::Rename ), tr( "Rename Keyword..." ), this ) );
    editKeywordAction_->setToolTip( tr( "Rename selected keyword" ) );
    editKeywordAction_->setShortcut( Qt::Key_F2 );
    editKeywordAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( editKeywordAction_, SIGNAL(triggered()), SLOT(_renameKeyword()) );

    /*
    delete keyword action
    it is associated to the Qt::Key_Delete shortcut
    but the later is enabled only if the KeywordList has focus.
    */
    deleteKeywordAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete Keyword" ), this );
    deleteKeywordAction_->setToolTip( tr( "Delete selected keyword" ) );
    deleteKeywordAction_->setShortcut( QKeySequence::Delete );
    deleteKeywordAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( deleteKeywordAction_, SIGNAL(triggered()), SLOT(_deleteKeyword()) );

    findEntriesAction_ = new QAction( IconEngine::get( IconNames::Find ), tr( "Find" ), this );
    findEntriesAction_->setShortcut( QKeySequence::Find );
    findEntriesAction_->setToolTip( tr( "Find entries matching specific criteria" ) );
    connect( findEntriesAction_, SIGNAL(triggered()), SLOT(_findEntries()) );

    newEntryAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New Entry..." ), this );
    newEntryAction_->setToolTip( tr( "Create a new entry" ) );
    newEntryAction_->setShortcut( QKeySequence::New );
    connect( newEntryAction_, SIGNAL(triggered()), SLOT(_newEntry()) );

    editEntryAction_ = new QAction( IconEngine::get( IconNames::Edit ), tr( "Edit Entries..." ), this );
    editEntryAction_->setToolTip( tr( "Edit selected entries" ) );
    connect( editEntryAction_, SIGNAL(triggered()), SLOT(_editEntries()) );

    editEntryTitleAction_ = new QAction( IconEngine::get( IconNames::Rename ), tr( "Rename Entry..." ), this );
    editEntryTitleAction_->setToolTip( tr( "Edit selected entry title" ) );
    editEntryTitleAction_->setShortcut( Qt::Key_F2 );
    editEntryTitleAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( editEntryTitleAction_, SIGNAL(triggered()), SLOT(_startEntryEdition()) );

    /*
    delete entry action
    it is associated to the Qt::Key_Delete shortcut
    but the later is enabled only if the KeywordList has focus.
    */
    deleteEntryAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete Entries" ), this );
    deleteEntryAction_->setToolTip( tr( "Delete selected entries" ) );
    deleteEntryAction_->setShortcut( QKeySequence::Delete );
    deleteEntryAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( deleteEntryAction_, SIGNAL(triggered()), SLOT(_deleteEntries()) );

    // color menu
    colorMenu_ = new ColorMenu( this );
    colorMenu_->setTitle( tr( "Change entry color" ) );
    connect( colorMenu_, SIGNAL(selected(QColor)), SLOT(_changeEntryColor(QColor)) );

    entryColorAction_ = new QAction( IconEngine::get( IconNames::Color ), tr( "Entry Color" ), this );
    entryColorAction_->setToolTip( tr( "Change selected entries color" ) );
    entryColorAction_->setMenu( colorMenu_ );

    entryKeywordAction_ = new QAction( IconEngine::get( IconNames::Edit ), tr( "Change Keyword..." ), this );
    entryKeywordAction_->setToolTip( tr( "Edit selected entries keyword" ) );
    connect( entryKeywordAction_, SIGNAL(triggered()), SLOT(_renameEntryKeyword()) );

    // entry information
    addAction( entryInformationAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Entry Properties..." ), this ) );
    entryInformationAction_->setToolTip( tr( "Show current entry properties" ) );
    connect( entryInformationAction_, SIGNAL(triggered()), SLOT(_entryInformation()) );

    newLogbookAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New Logbook..." ), this );
    newLogbookAction_->setToolTip( tr( "Create a new logbook" ) );
    connect( newLogbookAction_, SIGNAL(triggered()), SLOT(_newLogbook()) );

    openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open..." ), this );
    openAction_->setToolTip( tr( "Open an existsing logbook" ) );
    openAction_->setShortcut( QKeySequence::Open );
    connect( openAction_, SIGNAL(triggered()), SLOT(open()) );

    synchronizeAction_ = new QAction( IconEngine::get( IconNames::Merge ), tr( "Synchronize..." ), this );
    synchronizeAction_->setToolTip( tr( "Synchronize current logbook with remote" ) );
    connect( synchronizeAction_, SIGNAL(triggered()), SLOT(_synchronize()) );

    reorganizeAction_ = new QAction( tr( "Reorganize" ), this );
    reorganizeAction_->setToolTip( tr( "Reoganize logbook entries in files" ) );
    connect( reorganizeAction_, SIGNAL(triggered()), SLOT(_reorganize()) );

    saveAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save" ), this );
    saveAction_->setToolTip( tr( "Save all edited entries" ) );
    connect( saveAction_, SIGNAL(triggered()), SLOT(save()) );

    saveForcedAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save (forced)" ), this );
    saveForcedAction_->setToolTip( tr( "Save all entries" ) );
    connect( saveForcedAction_, SIGNAL(triggered()), SLOT(_saveForced()) );

    saveAsAction_ = new QAction( IconEngine::get( IconNames::SaveAs ), tr( "Save As..." ), this );
    saveAsAction_->setToolTip( tr( "Save logbook with a different name" ) );
    connect( saveAsAction_, SIGNAL(triggered()), SLOT(_saveAs()) );

    saveBackupAction_ = new QAction( IconEngine::get( IconNames::SaveAs ), tr( "Save Backup..." ), this );
    saveBackupAction_->setToolTip( tr( "Save logbook backup" ) );
    connect( saveBackupAction_, SIGNAL(triggered()), SLOT(_saveBackup()) );

    backupManagerAction_ = new QAction( IconEngine::get( IconNames::ConfigureBackups ), tr( "Manage Backups..." ), this );
    backupManagerAction_->setToolTip( tr( "Save logbook backup" ) );
    connect( backupManagerAction_, SIGNAL(triggered()), SLOT(_manageBackups()) );

    revertToSaveAction_ = new QAction( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this );
    revertToSaveAction_->setToolTip( tr( "Restore saved logbook" ) );
    revertToSaveAction_->setShortcut( QKeySequence::Refresh );
    connect( revertToSaveAction_, SIGNAL(triggered()), SLOT(_revertToSaved()) );

    // print
    printAction_ = new QAction( IconEngine::get( IconNames::Print ), tr( "Print..." ), this );
    printAction_->setShortcut( QKeySequence::Print );
    connect( printAction_, SIGNAL(triggered()), SLOT(_print()) );

    // print preview
    addAction( printPreviewAction_ = new QAction( IconEngine::get( IconNames::PrintPreview ), tr( "Print Preview..." ), this ) );
    printPreviewAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_P );
    connect( printPreviewAction_, SIGNAL(triggered()), SLOT(_printPreview()) );

    // export to HTML
    htmlAction_ = new QAction( IconEngine::get( IconNames::Html ), tr( "Export to HTML..." ), this );
    connect( htmlAction_, SIGNAL(triggered()), SLOT(_toHtml()) );

    logbookStatisticsAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Logbook Statistics..." ), this );
    logbookStatisticsAction_->setToolTip( tr( "View logbook statistics" ) );
    connect( logbookStatisticsAction_, SIGNAL(triggered()), SLOT(_viewLogbookStatistics()) );

    logbookInformationsAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Logbook Properties..." ), this );
    logbookInformationsAction_->setToolTip( tr( "Edit logbook properties" ) );
    connect( logbookInformationsAction_, SIGNAL(triggered()), SLOT(_editLogbookInformations()) );

    closeFramesAction_ = new QAction( IconEngine::get( IconNames::Close ), tr( "Close Editors" ), this );
    closeFramesAction_->setToolTip( tr( "Close all entry editors" ) );
    connect( closeFramesAction_, SIGNAL(triggered()), SLOT(_closeEditionWindows()) );

    // show duplicated entries
    showDuplicatesAction_ = new QAction( tr( "Show Duplicated Entries..." ), this );
    showDuplicatesAction_->setToolTip( tr( "Show duplicated entries in logbook" ) );
    connect( showDuplicatesAction_, SIGNAL(triggered()), SLOT(_showDuplicatedEntries()) );

    // view monitored files
    monitoredFilesAction_ = new QAction( tr( "Show Monitored Files..." ), this );
    monitoredFilesAction_->setToolTip( tr( "Show monitored files" ) );
    connect( monitoredFilesAction_, SIGNAL(triggered()), SLOT(_showMonitoredFiles()) );

    // tree mode
    treeModeAction_ = new QAction( tr( "Use Tree to Display Entries and Keywords" ), this );
    treeModeAction_->setCheckable( true );
    treeModeAction_->setChecked( true );
    connect( treeModeAction_, SIGNAL(toggled(bool)), SLOT(_toggleTreeMode(bool)) );

    // menu actions
    QAction* action;
    keywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::Move ), tr( "Move Here" ), this ) );
    keywordChangedMenuActions_.append( action = new QAction( this ) );
    action->setSeparator( true );
    keywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::DialogCancel ), tr( "Cancel" ), this ) );
    action->setShortcut( Qt::Key_Escape );

    // keywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::Copy ), tr( "Copy Here" ), this ) );
    entryKeywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::Move ), tr( "Move Here" ), this ) );
    entryKeywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::Link ), tr( "Link Here" ), this ) );
    entryKeywordChangedMenuActions_.append( action = new QAction( this ) );
    action->setSeparator( true );
    entryKeywordChangedMenuActions_.append( action = new QAction( IconEngine::get( IconNames::DialogCancel ), tr( "Cancel" ), this ) );
    action->setShortcut( Qt::Key_Escape );

}

//_______________________________________________
void MainWindow::_resetKeywordList( void )
{

    Debug::Throw( "MainWindow::_resetKeywordList.\n" );
    Q_CHECK_PTR( logbook_ );

    // retrieve new list of keywords (from logbook)
    Keyword::Set newKeywords;
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

    keywordModel_.set( newKeywords.toList() );

}

//_______________________________________________
void MainWindow::_resetLogEntryList( void )
{

    Debug::Throw( "MainWindow::_resetLogEntryList.\n" );

    // clear list of entries
    entryModel_.clear();

    if( logbook_ )
    {

        LogEntryModel::List modelEntries;
        for( const auto& entry:logbook_->entries() )
        {
            if( (!treeModeAction_->isChecked() && entry->isFindSelected()) || entry->isSelected() )
            { modelEntries << entry; }
        }

        entryModel_.add( modelEntries );

    }

    // loop over associated editionwindows
    // update navigation buttons
    for( const auto& window:Base::KeySet<EditionWindow>( this ) )
    {

        // skip closed editors
        if( window->isClosed() ) continue;

        // get associated entry and see if selected
        LogEntry* entry( window->entry() );
        window->previousEntryAction().setEnabled( entry && entry->isSelected() && previousEntry(entry, false) );
        window->nextEntryAction().setEnabled( entry && entry->isSelected() && nextEntry(entry, false) );

    }

    return;

}

//_______________________________________________
void MainWindow::_loadColors( void )
{

    Debug::Throw( "MainWindow::_loadColors.\n" );

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
    menu().setEnabled( value );

    // toolbars
    for( const auto& toolbar:findChildren<QToolBar*>() )
    { toolbar->setEnabled( value ); }

}


//__________________________________________________________________
bool MainWindow::_hasModifiedEntries( void ) const
{
    Base::KeySet<EditionWindow> frames( this );
    return std::find_if( frames.begin(), frames.end(), EditionWindow::ModifiedFTor() ) != frames.end();
}

//_______________________________________________
void MainWindow::_autoSave( void )
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
AskForSaveDialog::ReturnCode MainWindow::_checkModifiedEntries( Base::KeySet<EditionWindow> windows, bool confirmEntries ) const
{
    Debug::Throw( "_MainWindow::checkModifiedEntries.\n" );

    // check if editable EditionWindows needs save
    // cancel if required
    for( const auto& window:windows )
    {
        if( !(window->isReadOnly() || window->isClosed()) && window->modified() )
        {
            if( !confirmEntries ) { window->saveAction().trigger(); }
            else if( window->askForSave() == AskForSaveDialog::Cancel ) return AskForSaveDialog::Cancel;
        }
    }

    return  AskForSaveDialog::Yes;
}

//_______________________________________________
void MainWindow::_updateEntryFrames( LogEntry* entry, Mask mask )
{
    Debug::Throw( "MainWindow::_updateEntryFrames.\n" );

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
        if( windowModified ) window->askForSave( false );
        else window->setModified( false );

    }

}

//_____________________________________________
void MainWindow::_filesModified( FileCheck::DataSet files )
{

    Debug::Throw( "MainWindow::_filesModified.\n" );

    if( ignoreWarnings_ ) return;
    if( files.empty() ) return;

    // check if there is already a dialog open, and update list of files if yes
    auto dialog = findChild<LogbookModifiedDialog*>();
    if( dialog ) dialog->addFiles( files );
    else {

        // create dialog and take action accordingly
        int state = LogbookModifiedDialog( this, files ).exec();
        if( state == LogbookModifiedDialog::SaveAgain ) { save(); }
        else if( state == LogbookModifiedDialog::SaveAs ) { _saveAs(); }
        else if( state == LogbookModifiedDialog::Reload )
        {

            logbook_->setModifiedRecursive( false );
            _revertToSaved();

        } else if( state == LogbookModifiedDialog::Ignore ) { ignoreWarnings_ = true; }

    }

    return;
}

//________________________________________________________
void MainWindow::_splitterMoved( void )
{
    Debug::Throw( "MainWindow::_splitterMoved.\n" );
    resizeTimer_.start( 200, this );
}

//_______________________________________________
void MainWindow::_newLogbook( void )
{
    Debug::Throw( "MainWindow::_newLogbook.\n" );

    // check current logbook
    if( logbook_ && logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // new logbook
    NewLogbookDialog dialog( this );
    dialog.setTitle( Logbook::NoTitle );
    dialog.setAuthor( XmlOptions::get().raw( "USER" ) );
    dialog.setAttachmentDirectory( workingDirectory() );
    if( !dialog.centerOnParent().exec() ) return;

    // create a new logbook, with no file
    setLogbook( File() );
    Q_CHECK_PTR( logbook_ );

    logbook_->setTitle( dialog.title() );
    logbook_->setAuthor( dialog.author() );
    logbook_->setComments( dialog.comments() );

    // attachment directory
    File directory( dialog.attachmentDirectory() );

    // check if fulldir is not a non directory existsing file
    if( directory.exists() && !directory.isDirectory() )
    {

        const QString buffer = QString( tr( "File '%1' is not a directory" ) ).arg( directory );
        InformationDialog( this, buffer ).exec();

    } else logbook_->setDirectory( directory );

}

//_______________________________________________
void MainWindow::updateWindowTitle( void )
{

    Debug::Throw( "MainWindow::updateWindowTitle.\n" );

    if( logbook_ )
    {

        if( !logbook_->file().isEmpty() )
        {

            if( logbook_->isReadOnly() ) setWindowTitle( QString( tr( "%1 (read-only) - Elogbook" ) ).arg( logbook_->file() ) );
            else if( logbook_->modified() )setWindowTitle( QString( tr( "%1 (modified) - Elogbook" ) ).arg( logbook_->file() ) );
            else setWindowTitle( QString( tr( "%1 - Elogbook" ) ).arg( logbook_->file() ) );

        } else  {

            if( logbook_->isReadOnly() ) setWindowTitle( tr( "ELogbook (read-only)" ) );
            else if( logbook_->modified() ) setWindowTitle( tr( "ELogbook (modified)" ) );
            else setWindowTitle( "Elogbook" );

        }

    } else setWindowTitle( "Elogbook" );

}

//_______________________________________________
void MainWindow::open( FileRecord record )
{

    Debug::Throw( "MainWindow::open.\n" );

    // check if current logbook needs save
    if( _checkModifiedEntries( Base::KeySet<EditionWindow>( this ), confirmEntries_ ) == AskForSaveDialog::Cancel ) return;
    if( logbook_ && logbook_->modified()  && askForSave() == AskForSaveDialog::Cancel ) return;

    // open file from dialog if not set as argument
    if( record.file().isEmpty() )
    {

        const QString file( FileDialog(this).selectFile( workingDirectory() ).getFile() );
        if( file.isNull() ) return;
        else record = FileRecord( file );

    }

    // create logbook from file
    Singleton::get().application<Application>()->busy();
    setLogbook( record.file() );
    Singleton::get().application<Application>()->idle();

    // check if backup is needed
    // no need to do that for read-only logbooks
    checkLogbookBackup();

    return;
}

//_______________________________________________
bool MainWindow::_saveAs( File defaultFile, bool registerLogbook )
{
    Debug::Throw( "MainWindow::_saveAs.\n");

    // check current logbook
    if( !logbook_ ) {
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
    if( fullname.isNull() ) return false;
    else  fullname = fullname.expand();

    // update working directory
    workingDirectory_ = fullname.path();

    // change logbook filename and save
    logbook_->setFile( fullname );
    logbook_->setModifiedRecursive( true );
    save();

    // update current file in menu
    menu().recentFilesMenu().setCurrentFile( fullname );

    /*
    force logbook state to unmodified since
    some children state may not have been reset properly
    */
    logbook_->setModifiedRecursive( false );

    // add new file to openPreviousMenu
    if( !logbook_->file().isEmpty() )
    { Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expand() ); }

    // redo file check registration
    if( registerLogbook )
    {
        fileCheck().clear();
        fileCheck().registerLogbook( logbook_ );
    }

    // reset ignore_warning flag
    ignoreWarnings_ = false;

    return true;
}


//_____________________________________________
void MainWindow::_saveForced( void )
{
    Debug::Throw( "MainWindow::_saveForced.\n" );

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
void MainWindow::_saveBackup( void )
{
    Debug::Throw( "MainWindow::_saveBackup.\n");

    // check current logbook
    if( !logbook_ ) {
        InformationDialog( this, tr( "No logbook opened. <Save Backup> canceled." ) ).exec();
        return;
    }

    // generate backup fileName
    auto filename( logbook_->backupFilename( ) );
    if( filename.isEmpty() ) {
        InformationDialog( this, tr( "No valid filename. Use <Save As> first." ) ).exec();
        return;
    }

    // stores current logbook filename
    auto currentFilename( logbook_->file() );
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
    Singleton::get().application<Application>()->recentFiles().remove( File(filename).expand() );

    // restore initial filename
    logbook_->setFile( currentFilename, true );

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
void MainWindow::_manageBackups( void )
{
    Debug::Throw( "MainWindow::_manageBackups.\n");

    BackupManagerDialog dialog( this );
    Base::Key::associate( &dialog.managerWidget(), logbook_ );
    dialog.managerWidget().updateBackups();

    // connections
    connect( &dialog.managerWidget(), SIGNAL(saveLogbookRequested()), SLOT(save()) );
    connect( &dialog.managerWidget(), SIGNAL(backupRequested()), SLOT(_saveBackup()) );
    connect( &dialog.managerWidget(), SIGNAL(removeBackupRequested(Backup)), SLOT(_removeBackup(Backup)) );
    connect( &dialog.managerWidget(), SIGNAL(removeBackupsRequested(Backup::List)), SLOT(_removeBackups(Backup::List)) );
    connect( &dialog.managerWidget(), SIGNAL(restoreBackupRequested(Backup)), SLOT(_restoreBackup(Backup)) );
    connect( &dialog.managerWidget(), SIGNAL(mergeBackupRequested(Backup)), SLOT(_mergeBackup(Backup)) );

    dialog.exec();
}

//_____________________________________________
void MainWindow::_revertToSaved( void )
{
    Debug::Throw( "MainWindow::_revertToSaved.\n" );

    // check logbook
    if( !logbook_ ){
        InformationDialog( this, tr( "No logbook opened. <Reload> canceled." ) ).exec();
        return;
    }

    // ask for confirmation
    const QString buffer = QString( tr( "Discard changes to '%1'?" ) ).arg( logbook_->file().localName());
    if( ( _hasModifiedEntries() || logbook_->modified() ) && !QuestionDialog( this, buffer ).exec() )
    { return; }

    // reinit MainWindow
    Singleton::get().application<Application>()->busy();
    setLogbook( logbook_->file() );
    Singleton::get().application<Application>()->idle();

    // check if backup is needed
    // no need to do that for read-only logbooks
    checkLogbookBackup();
    ignoreWarnings_ = false;

}

//___________________________________________________________
void MainWindow::_print( void )
{

    // save EditionWindows
    if( _checkModifiedEntries( Base::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::Cancel ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // get entry selection
    LogEntryPrintSelectionDialog selectionDialog( this );
    selectionDialog.okButton().setText( tr( "Print ..." ) );
    selectionDialog.okButton().setIcon( IconEngine::get( IconNames::Print ) );
    if( !selectionDialog.exec() ) return;

    // create helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook_ );

    // assign entries and keyword
    auto selectionMode( selectionDialog.mode() );
    helper.setEntries( _entries( selectionMode ) );

    if( treeModeAction_->isChecked() &&
        ( selectionMode == LogEntryPrintSelectionWidget::VisibleEntries ||
        selectionMode == LogEntryPrintSelectionWidget::SelectedEntries ) )
    { helper.setCurrentKeyword( currentKeyword() ); }

    // print
    _print( helper );

}

//___________________________________________________________
void MainWindow::_print( LogbookPrintHelper& helper )
{
    Debug::Throw( "MainWindow::_print.\n" );

    // create printer
    QPrinter printer( QPrinter::HighResolution );

    // generate document name
    QString buffer;
    QTextStream( &buffer )  << "elogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid();
    printer.setDocName( buffer );

    // create options widget
    PrinterOptionWidget* optionWidget( new PrinterOptionWidget() );
    optionWidget->setHelper( &helper );
    connect( optionWidget, SIGNAL(orientationChanged(QPrinter::Orientation)), &helper, SLOT(setOrientation(QPrinter::Orientation)) );
    connect( optionWidget, SIGNAL(pageModeChanged(BasePrintHelper::PageMode)), &helper, SLOT(setPageMode(BasePrintHelper::PageMode)) );

    LogbookPrintOptionWidget* logbookOptionWidget = new LogbookPrintOptionWidget();
    logbookOptionWidget->setWindowTitle( "Logbook Configuration" );
    connect( logbookOptionWidget, SIGNAL(maskChanged(Logbook::Mask)), &helper, SLOT(setMask(Logbook::Mask)) );
    logbookOptionWidget->read( XmlOptions::get() );

    LogEntryPrintOptionWidget* logEntryOptionWidget = new LogEntryPrintOptionWidget();
    logEntryOptionWidget->setWindowTitle( "Logbook Entry Configuration" );
    connect( logEntryOptionWidget, SIGNAL(maskChanged(LogEntry::Mask)), &helper, SLOT(setEntryMask(LogEntry::Mask)) );
    logEntryOptionWidget->read( XmlOptions::get() );

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setOptionTabs( QList<QWidget *>()
        << optionWidget
        << logbookOptionWidget
        << logEntryOptionWidget );

    dialog.setWindowTitle( tr( "Print Logbook - Elogbook" ) );
    if( dialog.exec() == QDialog::Rejected ) return;

    // add output file to scratch files, if any
    if( !printer.outputFileName().isEmpty() )
    { emit scratchFileCreated( printer.outputFileName() ); }

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
void MainWindow::_printPreview( void )
{
    Debug::Throw( "MainWindow::_printPreview.\n" );

    // save EditionWindows
    if( _checkModifiedEntries( Base::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::Cancel ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // get entry selection
    LogEntryPrintSelectionDialog selectionDialog( this );
    selectionDialog.okButton().setText( tr( "Preview ..." ) );
    selectionDialog.okButton().setIcon( IconEngine::get( IconNames::PrintPreview ) );
    if( !selectionDialog.exec() ) return;

    // create helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook_ );

    auto selectionMode( selectionDialog.mode() );

    if( treeModeAction_->isChecked() &&
        ( selectionMode == LogEntryPrintSelectionWidget::VisibleEntries ||
        selectionMode == LogEntryPrintSelectionWidget::SelectedEntries ) )
    { helper.setCurrentKeyword( currentKeyword() ); }

    switch( selectionMode )
    {

        case LogEntryPrintSelectionWidget::AllEntries:
        helper.setEntries( Base::KeySet<LogEntry>( logbook_->entries() ).toList() );
        break;

        default:
        case LogEntryPrintSelectionWidget::VisibleEntries:
        helper.setEntries( entryModel_.get() );
        break;

        case LogEntryPrintSelectionWidget::SelectedEntries:
        helper.setEntries( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
        break;

    }

    // masks
    helper.setMask( (Logbook::Mask) XmlOptions::get().get<int>( "LOGBOOK_PRINT_OPTION_MASK" ) );
    helper.setEntryMask( (LogEntry::Mask) XmlOptions::get().get<int>( "LOGENTRY_PRINT_OPTION_MASK" ) );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this, CustomDialog::OkButton|CustomDialog::CancelButton );
    dialog.setWindowTitle( tr( "Print Preview - Elogbook" ) );
    dialog.setHelper( &helper );

    // print
    if( dialog.exec() ) _print( helper );
    else {
        statusbar_->label().clear();
        statusbar_->showLabel();
    }

}

//___________________________________________________________
void MainWindow::_toHtml( void )
{
    Debug::Throw( "MainWindow::_toHtml.\n" );

    // save EditionWindows
    if( _checkModifiedEntries( Base::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::Cancel ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create options widget
    LogbookPrintOptionWidget* logbookOptionWidget = new LogbookPrintOptionWidget();
    logbookOptionWidget->read( XmlOptions::get() );

    LogEntryPrintSelectionWidget* logEntrySelectionWidget = new LogEntryPrintSelectionWidget();
    logEntrySelectionWidget->read( XmlOptions::get() );

    LogEntryPrintOptionWidget* logEntryOptionWidget = new LogEntryPrintOptionWidget();
    logEntryOptionWidget->read( XmlOptions::get() );

    // create dialog
    HtmlDialog dialog( this );
    dialog.setOptionWidgets( QList<QWidget *>()
        << logEntrySelectionWidget
        << logbookOptionWidget
        << logEntryOptionWidget );

    // generate file name
    QString buffer;
    QTextStream( &buffer )  << "eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
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
        const QString buffer = QString( tr( "Cannot write to file '%1'. <View HTML> canceled." ) ).arg( file );
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
    helper.setLogbook( logbook_ );

    // assign entries and keyword
    auto selectionMode( logEntrySelectionWidget->mode() );
    helper.setEntries( _entries( selectionMode ) );

    if( treeModeAction_->isChecked() &&
        ( selectionMode == LogEntryPrintSelectionWidget::VisibleEntries ||
        selectionMode == LogEntryPrintSelectionWidget::SelectedEntries ) )
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
    { ( Command( command ) << file ).run(); }

}

//_______________________________________________
void MainWindow::_synchronize( void )
{
    Debug::Throw( "MainWindow::_synchronize.\n" );

    // check current logbook is valid
    if( !logbook_ ) {
        InformationDialog( this, tr( "No logbook opened. <Merge> canceled." ) ).exec();
        return;
    }

    // save EditionWindows
    if( _checkModifiedEntries( Base::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::Cancel ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create file dialog
    File remoteFile( FileDialog(this).getFile() );
    if( remoteFile.isNull() ) return;

    // debug
    Debug::Throw() << "MainWindow::_synchronize - number of local files: " << logbook_->children().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - number of local entries: " << logbook_->entries().size() << endl;

    // set busy flag
    Singleton::get().application<Application>()->busy();
    statusbar_->label().setText( "Reading remote logbook ... " );

    // opens file in remote logbook
    Debug::Throw() << "MainWindow::_synchronize - reading remote logbook from file: " << remoteFile << endl;

    Logbook remoteLogbook;
    connect( &remoteLogbook, SIGNAL(messageAvailable(QString)), SIGNAL(messageAvailable(QString)) );
    remoteLogbook.setFile( remoteFile );
    remoteLogbook.read();

    // check if logbook is valid
    XmlError::List errors( remoteLogbook.xmlErrors() );
    if( errors.size() )
    {

        QString buffer = QString( errors.size() > 1 ? tr( "Errors occured while parsing files.\n"):tr("An error occured while parsing files.\n") );
        buffer += errors.toString();
        InformationDialog( 0, buffer ).exec();

        Singleton::get().application<Application>()->idle();
        return;

    }

    // debug
    Debug::Throw() << "MainWindow::_synchronize - number of remote files: " << remoteLogbook.children().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - number of remote entries: " << remoteLogbook.entries().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - updating local from remote" << endl;

    // synchronize local with remote
    // retrieve map of duplicated entries
    QHash<LogEntry*,LogEntry*> duplicates( logbook_->synchronize( remoteLogbook ) );
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
    auto iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    entryList_->setFocus();

    // write local logbook
    if( !logbook_->file().isEmpty() ) save();

    // synchronize remove with local
    Debug::Throw() << "MainWindow::_synchronize - updating remote from local" << endl;
    unsigned int nDuplicated = remoteLogbook.synchronize( *logbook_ ).size();
    Debug::Throw() << "MainWindow::_synchronize - number of duplicated entries: " << nDuplicated << endl;

    // save remote logbook
    statusbar_->label().setText( tr( "Saving remote logbook..." ) );
    remoteLogbook.write();

    // idle
    Singleton::get().application<Application>()->idle();
    statusbar_->label().setText( "" );

    return;

}

//_______________________________________________
void MainWindow::_removeBackup( Backup backup )
{ _removeBackups( Backup::List() << backup ); }

//_______________________________________________
void MainWindow::_removeBackups( Backup::List backups )
{

    Debug::Throw( "MainWindow::_removeBackups.\n" );

    Singleton::get().application<Application>()->busy();

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

        // get list of children
        Logbook::List all( backupLogbook.children() );
        all.prepend( &backupLogbook );

        // remove all files
        for( const auto& logbook:all )
        {
            emit messageAvailable( QString( tr( "Removing '%1'" ) ).arg( logbook->file() ) );
            logbook->file().remove();
        }

        // clean logbook backups
        Backup::List backups( logbook_->backupFiles() );
        auto iter = std::find( backups.begin(), backups.end(), backup );
        if( iter != backups.end() )
        {
            backups.erase( iter );
            logbook_->setBackupFiles( backups );
            modified = true;
        }
    }

    Singleton::get().application<Application>()->idle();

    if( modified && !logbook_->file().isEmpty() )
    { save(); }

    if( invalidFiles.size() == 1 )
    {

        const QString buffer = QString( tr( "Unable to open file named '%1'. <Remove Backup> canceled." ) ).arg( invalidFiles.front() );
        InformationDialog( this, buffer ).exec();

    } else if( invalidFiles.size() > 1 ) {

        const QString buffer = QString( tr( "%i files could not be opened. <Remove Backup> canceled." ) ).arg( invalidFiles.size() );
        QString details;
        for( const auto& file:invalidFiles )
        { details += file + "\n"; }

        InformationDialog dialog( this, buffer );
        dialog.setDetails( details );
        dialog.exec();

    }

}

//_______________________________________________
void MainWindow::_restoreBackup( Backup backup )
{
    Debug::Throw( "MainWindow::_restoreBackup.\n" );
    if( !backup.file().exists() )
    {
        const QString buffer = QString( tr( "Unable to open file named '%1'. <Restore Backup> canceled." ) ).arg( backup.file() );
        InformationDialog( this, buffer ).exec();
        return;
    }

    // store old filename
    File oldName( logbook_->file() );

    // store old backups
    Backup::List backups( logbook_->backupFiles() );

    // store associated backup manager Widget
    Base::KeySet<BackupManagerWidget> widgets( logbook_ );

    // replace logbook with backup
    setLogbook( backup.file() );

    // remove the "backup" filename from the openPrevious list
    // to avoid confusion
    Singleton::get().application<Application>()->recentFiles().remove( backup.file().expand() );

    // change filename
    logbook_->setFile( oldName );

    // reassign backups
    logbook_->setBackupFiles( backups );

    // re-associate
    for( const auto& widget:widgets )
    { Base::Key::associate( widget, logbook_ ); }

    // and save
    if( !logbook_->file().isEmpty() )
    {
        save();
        Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expand() );
    }

}

//_______________________________________________
void MainWindow::_mergeBackup( Backup backup )
{
    Debug::Throw( "MainWindow::_mergeBackup.\n" );

    // check current logbook is valid
    if( !logbook_ ) {
        InformationDialog( this, tr( "No logbook opened. <Merge> canceled." ) ).exec();
        return;
    }

    if( !backup.file().exists() )
    {
        const QString buffer = QString( tr( "Unable to open file named '%1'. <Merge Backup> canceled." ) ).arg( backup.file() );
        return;
    }

    // save EditionWindows
    if( _checkModifiedEntries( Base::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::Cancel ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // debug
    Debug::Throw() << "MainWindow::_mergeBackup - number of local files: " << logbook_->children().size() << endl;
    Debug::Throw() << "MainWindow::_mergeBackup - number of local entries: " << logbook_->entries().size() << endl;

    // set busy flag
    Singleton::get().application<Application>()->busy();
    statusbar_->label().setText( tr( "Reading remote logbook..." ) );

    // opens file in remote logbook
    Debug::Throw() << "MainWindow::_mergeBackup - reading remote logbook from file: " << backup.file() << endl;

    Logbook backupLogbook;
    connect( &backupLogbook, SIGNAL(messageAvailable(QString)), SIGNAL(messageAvailable(QString)) );
    backupLogbook.setFile( backup.file() );
    backupLogbook.read();

    // check if logbook is valid
    XmlError::List errors( backupLogbook.xmlErrors() );
    if( errors.size() )
    {

        QString buffer = QString( errors.size() > 1 ? tr( "Errors occured while parsing files.\n"):tr("An error occured while parsing files.\n") );
        buffer += errors.toString();
        InformationDialog( 0, buffer ).exec();

        Singleton::get().application<Application>()->idle();
        return;

    }

    // debug
    Debug::Throw() << "MainWindow::_mergeBackup - number of remote files: " << backupLogbook.children().size() << endl;
    Debug::Throw() << "MainWindow::_mergeBackup - number of remote entries: " << backupLogbook.entries().size() << endl;
    Debug::Throw() << "MainWindow::_mergeBackup - updating local from remote" << endl;

    // synchronize local with remote
    // retrieve map of duplicated entries
    QHash<LogEntry*,LogEntry*> duplicates( logbook_->synchronize( backupLogbook ) );
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
    Base::KeySet<LogEntry> entries( logbook_->entries() );
    auto iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    entryList_->setFocus();

    // write local logbook
    if( !logbook_->file().isEmpty() ) save();

    // idle
    Singleton::get().application<Application>()->idle();
    statusbar_->label().setText( "" );

    return;

}

//_______________________________________________
void MainWindow::_reorganize( void )
{
    Debug::Throw( "MainWindow::_reorganize.\n" );

    if( !logbook_ )
    {
        InformationDialog( this, tr( "No valid logbook. <Reorganize> canceled." ) ).exec();
        return;
    }

    // retrieve all entries
    Base::KeySet<LogEntry> entries( logbook_->entries() );
    for( const auto& entry:entries )
    {

        Base::KeySet<Logbook> logbooks( entry );
        for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
        { logbook->setModified( true ); }

        entry->clearAssociations<Logbook>();

    }

    // put entry set into a list and sort by creation time.
    // First entry must the oldest
    QList<LogEntry*> entryList( entries.toList() );
    std::sort( entryList.begin(), entryList.end(), LogEntry::FirstCreatedFTor() );

    // put entries in logbook
    for( const auto& entry:entryList )
    {
        Logbook *logbook( MainWindow::logbook_->latestChild() );
        Base::Key::associate( entry, logbook );
        logbook->setModified( true );
    }

    // remove empty logbooks
    logbook_->removeEmptyChildren();

    // redo fileChecker registration
    fileCheck().clear();
    fileCheck().registerLogbook( logbook_ );

    // save
    logbook_->setModified( true );
    if( !logbook_->file().isEmpty() ) save();

}

//_______________________________________________
void MainWindow::_showDuplicatedEntries( void )
{
    Debug::Throw( "MainWindow::_showDuplicatedEntries.\n" );

    // keep track of the last visible entry
    LogEntry *lastVisibleEntry( 0 );

    // keep track of current index
    QModelIndex currentIndex( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( currentIndex.isValid() ? entryModel_.get( currentIndex ):0 );

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

        InformationDialog( this, tr( "No duplicated entries found" ) ).centerOnParent().exec();

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
void MainWindow::_viewLogbookStatistics( void )
{
    Debug::Throw( "MainWindow::_viewLogbookStatistics.\n" );

    if( !logbook_ )
    {
        InformationDialog( this, tr( "No logbook opened. <View Logbook Statistics> canceled." ) ).exec();
        return;
    }

    LogbookStatisticsDialog( this, logbook_ ).centerOnWidget( qApp->activeWindow() ).exec();

}

//_______________________________________________
void MainWindow::_editLogbookInformations( void )
{
    Debug::Throw( "MainWindow::_editLogbookInformations.\n" );

    if( !logbook_ )
    {
        InformationDialog( this, tr( "No logbook opened. <Edit Logbook Informations> canceled." ) ).exec();
        return;
    }

    // create dialog
    LogbookInformationDialog dialog( this, logbook_ );
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

        const QString buffer = QString( tr( "File '%1' is not a directory." ) ).arg( directory );
        InformationDialog( this, buffer ).exec();

    } else modified |= logbook_->setDirectory( directory );


    // save Logbook, if needed
    if( modified ) logbook_->setModified( true );
    if( !logbook_->file().isEmpty() ) save();

    updateWindowTitle();

}

//_______________________________________________
void MainWindow::_closeEditionWindows( bool askForSave ) const
{
    Debug::Throw( "MainWindow::_closeEditionWindows.\n" );

    // get all EditionWindows from MainWindow
    Base::KeySet<EditionWindow> windows( this );
    if( askForSave && _checkModifiedEntries( windows, true ) == AskForSaveDialog::Cancel ) return;
    for( const auto& window:windows )  window->deleteLater();

    return;

}

//____________________________________________
void MainWindow::_findEntries( void ) const
{

    Debug::Throw( "MainWindow::_findEntries.\n" );

    // try set from current selection
    QString text;
    if( !( text = qApp->clipboard()->text( QClipboard::Selection ) ).isEmpty() )
    {
        const int maxLength( 1024 );
        text = text.left( maxLength );
        searchWidget_->setText( text );
    }

    // check panel visibility
    if( !searchWidget_->isVisible() ) searchWidget_->show();

    // change focus
    searchWidget_->editor().lineEdit()->selectAll();
    searchWidget_->editor().setFocus();

}

//____________________________________________
void MainWindow::_newEntry( void )
{

    Debug::Throw( "MainWindow::_NewEntry.\n" );

    // retrieve associated EditionWindows, check if one matches the selected entry
    EditionWindow *editionWindow( 0 );
    Base::KeySet<EditionWindow> frames( this );
    Base::KeySetIterator<EditionWindow> iterator( frames );
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
        editionWindow = new EditionWindow( 0, false );
        editionWindow->setColorMenu( colorMenu_ );
        Base::Key::associate( this, editionWindow );
        connect( editionWindow, SIGNAL(scratchFileCreated(File)), this, SIGNAL(scratchFileCreated(File)) );
        connect( logbook_, SIGNAL(readOnlyChanged(bool)), editionWindow, SLOT(updateReadOnlyState()) );

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
void MainWindow::_editEntries( void )
{
    Debug::Throw( "MainWindow::_EditEntries .\n" );

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
void MainWindow::_deleteEntries( void )
{
    Debug::Throw( "MainWindow::_DeleteEntries .\n" );

    // retrieve selected rows;
    QModelIndexList selectedIndexes( entryList_->selectionModel()->selectedRows() );

    // convert into LogEntry list
    LogEntryModel::List selection;
    for( const auto& index:selectedIndexes )
    {
        // check if index is not being edited
        if( entryModel_.editionEnabled() && index ==  entryModel_.editionIndex() )
        {

            InformationDialog( this, tr( "Software limitation: cannot delete an entry that is being edited. <Delete Entries> canceled." ) ).exec();
            return;

        } else selection << entryModel_.get( index );

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

    Debug::Throw( "MainWindow::_displayEntry.\n" );

    // retrieve associated EditionWindows, check if one matches the selected entry
    EditionWindow *editionWindow( 0 );
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
        Base::KeySetIterator<EditionWindow> iterator( windows );
        iterator.toBack();
        while( iterator.hasPrevious() )
        {
            EditionWindow* current( iterator.previous() );

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

            // also kill all frames but one
            Base::KeySet< TextEditor > editors( editionWindow );
            if( editors.size() > 1 )
            {

                auto localIter( editors.begin() );
                ++localIter;
                for( ;localIter != editors.end(); ++localIter )
                { editionWindow->closeEditor( **localIter ); }

                (**editors.begin()).setFocus();
                editionWindow->setActiveEditor( **editors.begin() );

            }

            break;
        }

    }

    // if no editionWindow is found create a new one
    if( !editionWindow )
    {
        editionWindow = new EditionWindow( 0, false );
        editionWindow->setColorMenu( colorMenu_ );
        Base::Key::associate( this, editionWindow );

        // need to display entry before deleting sub views.
        if( treeModeAction_->isChecked() ) editionWindow->displayEntry( currentKeyword(), entry );
        else if( entry->hasKeywords() ) editionWindow->displayEntry( *entry->keywords().begin(), entry );
        else editionWindow->displayEntry( Keyword::Default, entry );

        connect( editionWindow, SIGNAL(scratchFileCreated(File)), this, SIGNAL(scratchFileCreated(File)) );
        connect( logbook_, SIGNAL(readOnlyChanged(bool)), editionWindow, SLOT(updateReadOnlyState()) );

    }

    editionWindow->setForceShowKeyword( !treeModeAction_->isChecked() );
    editionWindow->centerOnWidget( this );
    editionWindow->show();

}

//_______________________________________________
void MainWindow::_changeEntryTitle( LogEntry* entry, QString newTitle )
{
    Debug::Throw( "MainWindow::_changeEntryTitle.\n" );

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
    Debug::Throw( "MainWindow::_changeEntryColor.\n" );

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
        entry->setModification( entry->modification()+1 );

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
void MainWindow::_entryInformation( void )
{
    // retrieve current selection
    auto selection( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
    if( selection.size() != 1 ) return;

    // create dialog
    LogEntryInformationDialog( this, selection.front() ).centerOnParent().exec();

}

//____________________________________________
void MainWindow::_newKeyword( void )
{

    Debug::Throw( "MainWindow::_newKeyword.\n" );

    // create dialog
    EditKeywordDialog dialog( this );
    dialog.setWindowTitle( tr( "New Keyword - Elogbook" ) );

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
void MainWindow::_deleteKeyword( void )
{
    Debug::Throw("MainWindow::_deleteKeyword.\n" );

    // check that keywordlist has selected item
    auto selectedIndexes( keywordList_->selectionModel()->selectedRows() );
    if( selectedIndexes.empty() )
    {
        InformationDialog( this, tr( "No keyword selected. <Delete Keyword> canceled" ) ).exec();
        return;
    }

    // store corresponding list of keywords
    KeywordModel::List keywords;
    for( const auto& index:selectedIndexes )
    { if( index.isValid() ) keywords << keywordModel_.get( index ); }

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

        Debug::Throw( "MainWindow::_deleteKeyword - moving entries.\n" );
        for( const auto& keyword:keywords )
        { _renameKeyword( keyword, keyword.parent(), false ); }

    } else if( dialog.deleteEntries() ) {

        Debug::Throw( "MainWindow::_deleteKeyword - deleting entries.\n" );

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
        const Keyword& keyword( iter.previous() );

        // retrieve index associated to parent keyword
        // if valid, select and break
        QModelIndex index( keywordModel_.index( keyword.parent() ) );
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
void MainWindow::_renameKeyword( void )
{
    Debug::Throw("MainWindow::_renameKeyword.\n" );

    // check that keywordlist has selected item
    QModelIndex current( keywordList_->selectionModel()->currentIndex() );
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
    Debug::Throw("MainWindow::_confirmRenameKeyword.\n" );

    // check keywords are different
    if( keyword == newKeyword ) return;

    QMenu menu( this );
    menu.addActions( keywordChangedMenuActions_ );
    menu.ensurePolished();
    QAction* action( menu.exec( QCursor::pos() ) );
    if( action == keywordChangedMenuActions_[0] ) _renameKeyword( keyword, newKeyword, true );
    else return;

}


//____________________________________________
void MainWindow::_renameKeyword( const Keyword& keyword, const Keyword& newKeyword, bool updateSelection )
{

    Debug::Throw("MainWindow::_renameKeyword.\n" );

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
        for( const auto& entryKeyword:entry->keywords() )
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
            entry->setModification( entry->modification()+1 );

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
        QModelIndex parentIndex( keywordModel_.index( newKeyword.parent() ) );
        if( parentIndex.isValid() ) keywordList_->setExpanded( parentIndex, true );

        // retrieve current index, and select
        QModelIndex index( keywordModel_.index( newKeyword ) );
        keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList_->scrollTo( index );
    }

    _resetLogEntryList();

    // Save logbook if needed
    if( !logbook_->file().isEmpty() ) save();

}

//____________________________________________
void MainWindow::_renameEntryKeyword( void )
{
    Debug::Throw("MainWindow::_renameEntryKeyword.\n" );

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
        InformationDialog( this, tr( "No keyword selected. <Rename Entry Keyword> canceled" ) ).exec();
        return;
    }

    // get current selected keyword
    Keyword keyword( keywordModel_.get( keywordList_->selectionModel()->currentIndex() ) );

    // create dialog
    EditKeywordDialog dialog( this );
    dialog.setWindowTitle( tr( "Edit Keyword - Elogbook" ) );

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
void MainWindow::_confirmRenameEntryKeyword( Keyword newKeyword )
{

    Debug::Throw() << "MainWindow::_confirmRenameEntryKeyword - newKeyword: " << newKeyword << endl;

    QMenu menu( this );
    menu.addActions( entryKeywordChangedMenuActions_ );
    menu.ensurePolished();
    QAction* action( menu.exec( QCursor::pos() ) );
    if( action == entryKeywordChangedMenuActions_[0] ) _renameEntryKeyword( newKeyword );
    else if( action == entryKeywordChangedMenuActions_[1] ) _linkEntryKeyword( newKeyword );
    else return;

}

//_______________________________________________
void MainWindow::_renameEntryKeyword( Keyword newKeyword )
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
        entry->setModification( entry->modification()+1 );

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
void MainWindow::_linkEntryKeyword( Keyword newKeyword )
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
        entry->setModification( entry->modification()+1 );

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
void MainWindow::_updateSelection( Keyword newKeyword, Base::KeySet<LogEntry> entries )
{

    // check if at least one entry is selected
    if( entries.empty() ) return;

    // reset lists
    _resetKeywordList();

    // make sure parent keyword index is expanded
    QModelIndex parentIndex( keywordModel_.index( newKeyword.parent() ) );
    if( parentIndex.isValid() ) keywordList_->setExpanded( parentIndex, true );

    // retrieve current index, and select
    QModelIndex index( keywordModel_.index( newKeyword ) );
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
        QModelIndex index( entryModel_.index( entry ) );
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

    Debug::Throw( "MainWindow::_keywordSelectionChanged.\n" );
    if( !logbook_ ) return;
    if( !index.isValid() ) return;

    Keyword keyword( keywordModel_.get( index ) );
    Debug::Throw() << "MainWindow::_keywordSelectionChanged - keyword: " << keyword << endl;

    if( treeModeAction_->isChecked() ) entryModel_.setCurrentKeyword( keyword );

    // keep track of the current selected entry
    QModelIndex currentIndex( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( currentIndex.isValid() ? entryModel_.get( currentIndex ):0 );

    // retrieve all logbook entries
    Base::KeySet<LogEntry> turnedOffEntries;
    for( const auto& entry:logbook_->entries() )
    { entry->setKeywordSelected( entry->keywords().contains( keyword ) ); }

    // reinitialize logEntry list
    _resetLogEntryList();

    // if EditionWindow current entry is visible, select it
    // otherwise, select last entry in model
    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( entryModel_.rowCount() )  { selectEntry( entryModel_.get( entryModel_.index( entryModel_.rowCount()-1, 0 ) ) ); }

    return;
}

//_____________________________________________
void MainWindow::_updateKeywordActions( void )
{
    Debug::Throw( "MainWindow::_updateKeywordActions.\n" );

    const bool readOnly( logbook_ && logbook_->isReadOnly() );
    newKeywordAction_->setEnabled( !readOnly );
    editKeywordAction_->setEnabled( !readOnly && keywordList_->selectionModel()->currentIndex().isValid() );
    deleteKeywordAction_->setEnabled( !( readOnly || keywordList_->selectionModel()->selectedRows().empty() ) );

    return;
}

//_____________________________________________
void MainWindow::_updateEntryActions( void )
{
    Debug::Throw( "MainWindow::_updateEntryActions.\n" );
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
void MainWindow::_updateReadOnlyState( void )
{
    Debug::Throw( "MainWindow::_updateReadOnlyState.\n" );

    const bool readOnly( logbook_ && logbook_->isReadOnly() );
    synchronizeAction_->setEnabled( !readOnly );
    reorganizeAction_->setEnabled( !readOnly );
    saveAction_->setEnabled( !readOnly );
    saveForcedAction_->setEnabled( !readOnly );

    // clear the AttachmentWindow
    AttachmentWindow &attachmentWindow( Singleton::get().application<Application>()->attachmentWindow() );
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

        case LogEntryModel::Color: changed = logbook_->setSortMethod( Logbook::SortColor ); break;
        case LogEntryModel::Title: changed = logbook_->setSortMethod( Logbook::SortTitle ); break;
        case LogEntryModel::Creation: changed = logbook_->setSortMethod( Logbook::SortCreation ); break;
        case LogEntryModel::Modification: changed = logbook_->setSortMethod( Logbook::SortModification ); break;
        case LogEntryModel::Author: changed = logbook_->setSortMethod( Logbook::SortAuthor ); break;
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
    Debug::Throw( "MainWindow::_entryDataChanged.\n" );

    if( !index.isValid() ) return;
    LogEntry* entry( entryModel_.get( index ) );

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
void MainWindow::_startEntryEdition( void )
{

    Debug::Throw( "MainWindow::_startEntryEdition\n" );

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
void MainWindow::_showMonitoredFiles( void )
{

    Debug::Throw( "MainWindow::_showMonitoredFiles.\n" );
    FileCheckDialog dialog( qApp->activeWindow() );
    dialog.setFiles( fileCheck().fileSystemWatcher().files() );
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
    LogEntry* currentEntry( 0 );
    QModelIndex index( entryList_->selectionModel()->currentIndex() );
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

    // force show keyword
    for( const auto& window:Base::KeySet<EditionWindow>( this ) )
    {

        // if hiding keyword, first need ask for save
        if( value && !window->isClosed() && window->modified() && !window->isReadOnly() )
        { window->askForSave( false ); }

        // update force flag
        window->setForceShowKeyword( !value );

    }

    // make sure entry is visible
    if( currentEntry )
    {
        if( value && currentEntry->hasKeywords() )
        {
            // select keyword
            QModelIndex index( keywordModel_.index( *currentEntry->keywords().begin() ) );
            keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            keywordList_->scrollTo( index );
        }

        // select entry
        QModelIndex index( entryModel_.index( currentEntry ) );
        entryList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        entryList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        entryList_->scrollTo( index );

    }


    // save to options
    XmlOptions::get().set<bool>( "USE_TREE", value );

}

//_______________________________________________
void MainWindow::_updateConfiguration( void )
{

    Debug::Throw( "MainWindow::_updateConfiguration.\n" );

    resize( sizeHint() );

    // compression
    if( logbook_ ) logbook_->setUseCompression( XmlOptions::get().get<bool>( "USE_COMPRESSION" ) );

    // autoSave
    autoSaveDelay_ = 1000*XmlOptions::get().get<int>( "AUTO_SAVE_ITV" );
    bool autosave( XmlOptions::get().get<bool>( "AUTO_SAVE" ) );
    if( autosave ) autosaveTimer_.start( autoSaveDelay_, this );
    else autosaveTimer_.stop();

    // colors
    colorMenu_->reset();
    for( const auto& color:XmlOptions::get().specialOptions( "COLOR" ) )
    { colorMenu_->add( color.get<Base::Color>() ); }

    // max number of recent entries
    maxRecentEntries_ = XmlOptions::get().get<unsigned int>( "MAX_RECENT_ENTRIES" );

    // tree mode
    treeModeAction_->setChecked( XmlOptions::get().get<bool>( "USE_TREE" ) );

}

//_______________________________________________
LogEntryModel::List MainWindow::_entries( LogEntryPrintSelectionWidget::Mode mode )
{

    switch( mode )
    {
        default:
        case LogEntryPrintSelectionWidget::AllEntries:
        return Base::KeySet<LogEntry>( logbook_->entries() ).toList();

        case LogEntryPrintSelectionWidget::VisibleEntries:
        return entryModel_.get();

        case LogEntryPrintSelectionWidget::SelectedEntries:
        return entryModel_.get( entryList_->selectionModel()->selectedRows() );

    }

}
