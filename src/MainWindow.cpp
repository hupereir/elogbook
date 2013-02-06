
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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "MainWindow.h"

#include "Application.h"
#include "AttachmentWindow.h"
#include "BaseIcons.h"
#include "BackupManagerDialog.h"
#include "BackupManagerWidget.h"
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
#include "Icons.h"
#include "InformationDialog.h"
#include "LineEditor.h"
#include "Logbook.h"
#include "LogbookHtmlHelper.h"
#include "LogbookInformationDialog.h"
#include "LogbookModifiedDialog.h"
#include "LogbookStatisticsDialog.h"
#include "LogbookPrintOptionWidget.h"
#include "LogbookPrintHelper.h"
#include "LogEntryPrintOptionWidget.h"
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
#include "SearchPanel.h"
#include "Singleton.h"
#include "TextEditionDelegate.h"
#include "Util.h"
#include "XmlOptions.h"

#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QPrintDialog>
#include <QtGui/QSplitter>

//_____________________________________________
MainWindow::MainWindow( QWidget *parent ):
    BaseMainWindow( parent ),
    Counter( "MainWindow" ),
    autoSaveDelay_( 60000 ),
    editionDelay_( 200 ),
    maxRecentEntries_( 0 ),
    logbook_( 0 ),
    workingDirectory_( Util::workingDirectory() ),
    ignoreWarnings_( false ),
    confirmEntries_( true )
{
    Debug::Throw( "MainWindow::MainWindow.\n" );
    setOptionName( "MAIN_WINDOW" );
    setModified( false );

    // file checker
    fileCheck_ = new FileCheck( this );
    connect( &fileCheck(), SIGNAL( filesModified( FileCheck::DataSet ) ), SLOT( _filesModified( FileCheck::DataSet ) ) );

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

    // create hidden search panel
    addToolBar( Qt::BottomToolBarArea, searchPanel_ = new SearchPanel( "Search panel", this ) );
    searchPanel_->setAppearsInMenu( true );
    searchPanel_->hide();

    connect( &searchPanel(), SIGNAL( selectEntries( QString, unsigned int ) ), SLOT( selectEntries( QString, unsigned int ) ) );
    connect( &searchPanel(), SIGNAL( showAllEntries() ), SLOT( showAllEntries() ) );
    addAction( &searchPanel().visibilityAction() );

    // status bar
    setStatusBar( statusbar_ = new ProgressStatusBar( this ) );
    statusbar_->setProgressBar( new ProgressBar() );
    statusbar_->addClock();
    connect( this, SIGNAL( messageAvailable( const QString& ) ), &statusbar_->label(), SLOT( setTextAndUpdate( const QString& ) ) );
    connect( this, SIGNAL( messageAvailable( const QString& ) ), &statusbar_->progressBar(), SLOT( setText( const QString& ) ) );

    // global scope actions
    _installActions();

    // aditional actions from application
    Application& application( *Singleton::get().application<Application>() );
    addAction( &application.closeAction() );

    // Keyword container
    keywordContainer_ = new QWidget();

    // set layout
    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing( 5 );
    keywordContainer_->setLayout( vLayout );

    keywordToolBar_ = new CustomToolBar( "Keywords Toolbar", keywordContainer_, "KEYWORD_TOOLBAR" );
    keywordToolBar_->setTransparent( true );
    keywordToolBar_->setAppearsInMenu( true );
    vLayout->addWidget( keywordToolBar_ );

    // keyword actions
    keywordToolBar_->addAction( &newKeywordAction() );
    keywordToolBar_->addAction( &deleteKeywordAction() );
    keywordToolBar_->addAction( &editKeywordAction() );
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

    // the use of a custom delegate unfortunately disable the
    // nice selection appearance of the oxygen style.
    keywordList_->setItemDelegate( new TextEditionDelegate( this ) );

    // update LogEntryList when keyword selection change
    connect( keywordList_->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _keywordSelectionChanged( const QModelIndex& ) ) );
    connect( keywordList_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection& ) ), SLOT( _updateKeywordActions() ) );
    _updateKeywordActions();

    // rename selected entries when KeywordChanged is emitted with a single argument.
    // this correspond to drag and drop action from the logEntryList in the KeywordList
    connect( &keywordModel_, SIGNAL( entryKeywordChanged( Keyword ) ), SLOT( _renameEntryKeyword( Keyword ) ) );

    // rename all entries matching first keyword the second. This correspond to
    // drag and drop inside the keyword list, or to direct edition of a keyword list item.
    connect( &keywordModel_, SIGNAL( keywordChanged( Keyword, Keyword ) ), SLOT( _renameKeyword( Keyword, Keyword ) ) );

    {
        // popup menu for keyword list
        ContextMenu* menu = new ContextMenu( keywordList_ );
        menu->addAction( &newEntryAction() );
        menu->addAction( &newKeywordAction() );
        menu->addSeparator();
        menu->addAction( &deleteKeywordAction() );
        menu->addAction( &editKeywordAction() );
        menu->setHideDisabledActions( true );
    }

    /*
    add the deleteKeywordAction to the keyword list,
    so that the corresponding shortcut gets activated whenever it is pressed
    while the list has focus
    */
    keywordList_->addAction( &deleteKeywordAction() );
    keywordList_->addAction( &editKeywordAction() );

    // right box for entries and buttons
    QWidget* right = new QWidget();

    vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing( 5 );
    right->setLayout( vLayout );

    entryToolBar_ = new CustomToolBar( "Entries Toolbar", right, "ENTRY_TOOLBAR" );
    entryToolBar_->setTransparent( true );
    entryToolBar_->setAppearsInMenu( true );
    vLayout->addWidget( entryToolBar_ );

    // entry actions
    entryToolBar_->addAction( &newEntryAction() );
    entryToolBar_->addAction( &editEntryAction() );

    // need to use a button to be able to set the popup mode
    QToolButton *button = new QToolButton(0);
    button->setText( "Entry Color" );
    button->setIcon( IconEngine::get( ICONS::COLOR ) );
    button->setPopupMode( QToolButton::InstantPopup );
    button->setMenu( colorMenu_ );
    entryToolBar_->addWidget( button );

    entryToolBar_->addAction( &deleteEntryAction() );
    entryToolBar_->addAction( &saveAction() );
    entryToolBar_->addAction( &printAction() );

    // create logEntry list
    vLayout->addWidget( entryList_ = new AnimatedTreeView( right ), 1 );
    entryList_->setFindEnabled( false );
    entryList_->setModel( &entryModel_ );
    entryList_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    entryList_->setDragEnabled(true);
    entryList_->setOptionName( "ENTRY_LIST" );
    entryList_->lockColumnVisibility( LogEntryModel::KEYWORD );

    entryList_->setColumnHidden( LogEntryModel::TITLE, false );
    entryList_->lockColumnVisibility( LogEntryModel::TITLE );

    // the use of a custom delegate unfortunately disable the
    // nice selection appearance of the oxygen style.
    entryList_->setItemDelegate( new TextEditionDelegate( this ) );

    connect( entryList_->header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
    connect( entryList_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateEntryActions() ) );
    connect( entryList_, SIGNAL( activated( const QModelIndex& ) ), SLOT( _entryItemActivated( const QModelIndex& ) ) );
    connect( entryList_, SIGNAL( clicked( const QModelIndex& ) ), SLOT( _entryItemClicked( const QModelIndex& ) ) );
    _updateEntryActions();

    connect( &entryModel_, SIGNAL( layoutChanged() ), entryList_, SLOT( resizeColumns() ) );
    connect( &entryModel_, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _entryDataChanged( const QModelIndex& ) ) );

    /*
    add the deleteEntryAction to the list,
    so that the corresponding shortcut gets activated whenever it is pressed
    while the list has focus
    */
    entryList_->addAction( &deleteEntryAction() );
    entryList_->addAction( &editEntryTitleAction() );

    {
        // popup menu for list
        ContextMenu* menu = new ContextMenu( &logEntryList() );
        menu->addAction( &newEntryAction() );
        menu->addSeparator();
        menu->addAction( &editEntryTitleAction() );
        menu->addAction( &editEntryAction() );
        menu->addAction( &entryKeywordAction() );
        menu->addAction( &deleteEntryAction() );
        menu->addAction( &entryColorAction() );
        menu->setHideDisabledActions( true );
    }

    // add widgets to Hs
    splitter->addWidget( keywordContainer_ );
    splitter->addWidget( right );

    // assign stretch factors
    splitter->setStretchFactor( 0, 0 );
    splitter->setStretchFactor( 1, 1 );

    connect( splitter, SIGNAL( splitterMoved( int, int ) ), SLOT( _splitterMoved( void ) ) );

    // main menu
    menu_ = new Menu( this , this );
    setMenuBar( menu_ );
    connect( menu_, SIGNAL( entrySelected( LogEntry* ) ), SLOT( selectEntry( LogEntry* ) ) );
    connect( menu_, SIGNAL( entrySelected( LogEntry* ) ), SLOT( _displayEntry( LogEntry* ) ) );

    // configuration
    connect( &application, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    _updateConfiguration();
    _updateKeywordActions();
    _updateEntryActions();
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
    if( logbook_ && logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create a new logbook, with no file
    setLogbook( File() );
    Q_CHECK_PTR( logbook_ );

    logbook_->setTitle(  Logbook::LOGBOOK_NO_TITLE );
    logbook_->setAuthor( XmlOptions::get().raw( "USER" ) );
    logbook_->setDirectory( workingDirectory() );

    QString comments;
    QTextStream( &comments ) << "Default logbook created automatically on " << TimeStamp::now().toString( TimeStamp::LONG );
    logbook_->setComments( comments );

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

    connect( logbook_, SIGNAL( maximumProgressAvailable( int ) ), statusbar_, SLOT( showProgressBar() ) );
    connect( logbook_, SIGNAL( maximumProgressAvailable( int ) ), &statusbar_->progressBar(), SLOT( setMaximum( int ) ) );
    connect( logbook_, SIGNAL( progressAvailable( int ) ), &statusbar_->progressBar(), SLOT( addToProgress( int ) ) );
    connect( logbook_, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );

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
        case Logbook::SORT_COLOR: entryList_->sortByColumn( LogEntryModel::COLOR, sort_order ); break;
        case Logbook::SORT_TITLE: entryList_->sortByColumn( LogEntryModel::TITLE, sort_order ); break;
        case Logbook::SORT_CREATION: entryList_->sortByColumn( LogEntryModel::CREATION, sort_order ); break;
        case Logbook::SORT_MODIFICATION: entryList_->sortByColumn( LogEntryModel::MODIFICATION , sort_order); break;
        case Logbook::SORT_AUTHOR: entryList_->sortByColumn( LogEntryModel::AUTHOR, sort_order ); break;
        default: break;
    }

    Debug::Throw( "MainWindow::setLogbook - lists sorted.\n" );

    // update attachment frame
    resetAttachmentWindow();
    Debug::Throw( "MainWindow::setLogbook - attachment frame reset.\n" );

    // retrieve last modified entry
    BASE::KeySet<LogEntry> entries( logbook_->entries() );
    BASE::KeySet<LogEntry>::const_iterator iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    entryList_->setFocus();

    Debug::Throw( "MainWindow::setLogbook - entry selected.\n" );

    // see if logbook has parent file
    if( logbook_->parentFile().size() ) {
        QString buffer;
        QTextStream(&buffer ) << "Warning: this logbook should be oppened via \"" << logbook_->parentFile() << "\" only.";
        InformationDialog( this, buffer ).exec();
    }

    // store logbook directory for next open, save comment
    workingDirectory_ = File( logbook_->file() ).path();
    statusbar_->label().setText( "" );
    statusbar_->showLabel();

    // register logbook to fileCheck
    fileCheck().registerLogbook( logbook_ );

    emit ready();

    // check errors
    XmlError::List errors( logbook_->xmlErrors() );
    if( errors.size() )
    {
        QString buffer;
        QTextStream what( &buffer );
        if( errors.size() > 1 ) what << "Errors occured while parsing files." << endl;
        else what << "An error occured while parsing files." << endl;
        what << errors;
        InformationDialog( 0, buffer ).exec();
    }

    // add opened file to OpenPrevious mennu.
    if( !logbook_->file().isEmpty() )
    { Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expand() ); }

    ignoreWarnings_ = false;

    _updateKeywordActions();
    _updateEntryActions();

    setModified( false );

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
        logbook_->needsBackup() )
    {

        // ask if backup needs to be saved; save if yes
        if( QuestionDialog( this, "Current logbook needs backup. Make one?" ).exec() )
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
    foreach( EditionWindow* window, BASE::KeySet<EditionWindow>( this ) )
    {
        window->setIsClosed( true );
        window->hide();
    }

    return;

}

//____________________________________________
AskForSaveDialog::ReturnCode MainWindow::askForSave( const bool& enableCancel )
{

    Debug::Throw( "MainWindow::askForSave.\n" );

    // create dialog
    unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
    if( enableCancel ) buttons |= AskForSaveDialog::CANCEL;

    // exec and check return code
    int state = AskForSaveDialog( this, "Logbook has been modified. Save ?", buttons ).centerOnParent().exec();
    if( state == AskForSaveDialog::YES ) save();
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
    QModelIndex index = keywordModel_.index( entry->keyword() );
    keywordList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    keywordList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    keywordList_->scrollTo( index );

    index = entryModel_.index( entry );
    entryList_->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    entryList_->selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    entryList_->scrollTo( index );
    Debug::Throw("MainWindow::selectEntry - done.\n" );
    return;

}

//_______________________________________________
void MainWindow::updateEntry( LogEntry* entry, const bool& updateSelection )
{

    Debug::Throw( "MainWindow::updateEntry.\n" );

    // add entry into frame list or update existsing
    if( entry->keyword() != currentKeyword() )
    {
        keywordModel_.add( entry->keyword() );
        QModelIndex index = keywordModel_.index( entry->keyword() );
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
void MainWindow::deleteEntry( LogEntry* entry, const bool& save )
{
    Debug::Throw( "MainWindow::deleteEntry.\n" );

    Q_CHECK_PTR( entry );

    // get associated attachments
    BASE::KeySet<Attachment> attachments( entry );
    foreach( Attachment* attachment, attachments )
    {

        // retrieve/delete associated attachment frames
        BASE::KeySet<AttachmentFrame> frames( attachment );
        foreach( AttachmentFrame* frame, frames )
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
    BASE::KeySet<EditionWindow> windows( entry );
    foreach( EditionWindow* window, windows )
    {
        window->setIsClosed( true );
        window->hide();
    }

    // set logbooks as modified
    BASE::KeySet<Logbook> logbooks( entry );
    foreach( Logbook* logbook, logbooks )
    { logbook->setModified( true ); }

    // delete entry
    delete entry;

    //! save
    if( save && !logbook_->file().isEmpty() )
        MainWindow::save();

    return;

}

//_______________________________________________
bool MainWindow::lockEntry( LogEntry* entry ) const
{
    Debug::Throw( "MainWindow::lockEntry.\n" );

    if( !entry ) return true;

    BASE::KeySet<EditionWindow> windows( entry );
    if( _checkModifiedEntries( windows, true ) == AskForSaveDialog::CANCEL ) return false;

    foreach( EditionWindow* window, windows )
    { window->setReadOnly( true ); }

    return true;
}

//_______________________________________________
LogEntry* MainWindow::previousEntry( LogEntry* entry, const bool& updateSelection )
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
LogEntry* MainWindow::nextEntry( LogEntry* entry, const bool& updateSelection )
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
    BASE::KeySet<Attachment> attachments( logbook_->attachments() );
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
void MainWindow::save( const bool& confirmEntries )
{

    Debug::Throw( "MainWindow::_save.\n" );

    // check logbook
    if( !logbook_ )
    {
        InformationDialog( this, "no Logbook opened. <Save> canceled." ).exec();
        return;
    }

    if( !confirmEntries ) confirmEntries_ = false;

    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), confirmEntries_ ) == AskForSaveDialog::CANCEL ) return;

    // check logbook filename, go to Save As if no file is given and redirect is true
    if( logbook_->file().isEmpty() ) {
        _saveAs();
        return;
    }

    // check logbook filename is writable
    File fullname = File( logbook_->file() ).expand();
    if( fullname.exists() ) {

        // check file is not a directory
        if( fullname.isDirectory() ) {
            InformationDialog( this, "selected file is a directory. <Save Logbook> canceled." ).exec();
            confirmEntries_ = true;
            return;
        }

        // check file is writable
        if( !fullname.isWritable() ) {
            InformationDialog( this, "selected file is not writable. <Save Logbook> canceled." ).exec();
            confirmEntries_ = true;
            return;
        }

    } else {

        File path( fullname.path() );
        if( !path.isDirectory() ) {
            InformationDialog( this, "selected path is not vallid. <Save Logbook> canceled." ).exec();
            confirmEntries_ = true;
            return;
        }

    }

    // write logbook to file, retrieve result
    Singleton::get().application<Application>()->busy();
    _setEnabled( false );

    logbook_->truncateRecentEntriesList( maxRecentEntries_ );

    bool written( logbook_->write() );
    Singleton::get().application<Application>()->idle();
    _setEnabled( true );

    if( written ) { setModified( false );}

    // update StateFrame
    statusbar_->label().setText( "" );
    statusbar_->showLabel();

    // add new file to openPreviousMenu
    if( !logbook_->file().isEmpty() ) Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expand() );

    // reset ignore_warning flag
    ignoreWarnings_ = false;

    // reset confirm entries
    confirmEntries_ = true;
    return;
}

//_______________________________________________
void MainWindow::selectEntries( QString selection, unsigned int mode )
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
    if( mode == SearchPanel::NONE )
    {
        InformationDialog( this, "At least on field must be selected"  ).centerOnParent().exec();
        return;
    }

    // number of found items
    unsigned int found( 0 );
    unsigned int total( 0 );

    // keep track of the last visible entry
    LogEntry *lastVisibleEntry( 0 );

    // keep track of the current selected entry
    QModelIndex current_index( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? entryModel_.get( current_index ):0 );

    // check is selection is a valid color when Color search is requested.
    bool colorValid = ( mode&SearchPanel::COLOR && ( selection.compare( ColorMenu::NONE, Qt::CaseInsensitive ) == 0 || QColor( selection ).isValid() ) );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> turnedOffEntries;
    foreach( LogEntry* entry, logbook_->entries() )
    {

        total++;

        // if entry is already hidder, skipp
        if( !entry->isFindSelected() ) continue;

        // check entry
        bool accept( false );
        if( (mode&SearchPanel::TITLE ) && entry->matchTitle( selection ) ) accept = true;
        if( (mode&SearchPanel::KEYWORD ) && entry->matchKeyword( selection ) ) accept = true;
        if( (mode&SearchPanel::TEXT ) && entry->matchText( selection ) ) accept = true;
        if( (mode&SearchPanel::ATTACHMENT ) && entry->matchAttachment( selection ) ) accept = true;
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

        statusbar_->label().setText( "no match found. Find canceled" );

        // reset flag for the turned off entries to true
        foreach( LogEntry* entry, turnedOffEntries )
        { entry->setFindSelected( true ); }

        return;

    }

    // reinitialize logEntry list
    _resetKeywordList();
    _resetLogEntryList();

    // if EditionWindow current entry is visible, select it;
    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( lastVisibleEntry ) selectEntry( lastVisibleEntry );

    QString buffer;
    QTextStream out( &buffer );
    out << found << " out of " << total;
    if( found > 1 ) out << " entries selected";
    else out << " entry selected";

    statusbar_->label().setText( buffer );

    return;
}

//_______________________________________________
void MainWindow::showAllEntries( void )
{
    Debug::Throw( "MainWindow::showAllEntries.\n" );

    // keep track of the current selected entry
    QModelIndex current_index( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? entryModel_.get( current_index ):0 );

    // set all logbook entries to find_visible
    foreach( LogEntry* entry, logbook_->entries() )
    { entry->setFindSelected( true ); }

    // reinitialize logEntry list
    _resetKeywordList();
    _resetLogEntryList();

    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( entryModel_.rowCount() ) selectEntry( entryModel_.get( entryModel_.index( entryModel_.rowCount()-1, 0 ) ) );

    statusbar_->label().setText( "" );
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
    QWidget *child = childAt(event->pos());
    while (child && child != this)
    {
        if( child == &keywordToolBar() || child == &entryToolBar() )
        {
            accepted = true;
            break;
        }

        child = child->parentWidget();
    }

    if( !accepted ) return;
    QMenu* menu = createPopupMenu();
    menu->exec( event->globalPos() );
    menu->deleteLater();
    event->accept();

    return;

}

//_______________________________________________
void MainWindow::_installActions( void )
{

    Debug::Throw( "MainWindow::_installActions.\n" );
    uniconifyAction_ = new QAction( IconEngine::get( ICONS::HOME ), "Main Window", this );
    uniconifyAction_->setToolTip( "Raise application main window" );
    connect( uniconifyAction_, SIGNAL( triggered() ), SLOT( uniconify() ) );

    newKeywordAction_ = new QAction( IconEngine::get( ICONS::NEW ), "New Keyword", this );
    newKeywordAction_->setToolTip( "Create a new keyword" );
    connect( newKeywordAction_, SIGNAL( triggered() ), SLOT( _newKeyword() ) );

    addAction( editKeywordAction_ = new QAction( IconEngine::get( ICONS::RENAME ), "Rename Keyword...", this ) );
    editKeywordAction_->setToolTip( "Rename selected keyword" );
    editKeywordAction_->setShortcut( Qt::Key_F2 );
    editKeywordAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( editKeywordAction_, SIGNAL( triggered() ), SLOT( _renameKeyword() ) );

    /*
    delete keyword action
    it is associated to the Qt::Key_Delete shortcut
    but the later is enabled only if the KeywordList has focus.
    */
    deleteKeywordAction_ = new QAction( IconEngine::get( ICONS::DELETE ), "Delete Keyword", this );
    deleteKeywordAction_->setToolTip( "Delete selected keyword" );
    deleteKeywordAction_->setShortcut( QKeySequence::Delete );
    deleteKeywordAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( deleteKeywordAction_, SIGNAL( triggered() ), SLOT( _deleteKeyword() ) );

    findEntriesAction_ = new QAction( IconEngine::get( ICONS::FIND ), "Find", this );
    findEntriesAction_->setShortcut( QKeySequence::Find );
    findEntriesAction_->setToolTip( "Find entries matching specific criteria" );
    connect( findEntriesAction_, SIGNAL( triggered() ), SLOT( _findEntries() ) );

    newEntryAction_ = new QAction( IconEngine::get( ICONS::NEW ), "New Entry...", this );
    newEntryAction_->setToolTip( "Create a new entry" );
    newEntryAction_->setShortcut( QKeySequence::New );
    connect( newEntryAction_, SIGNAL( triggered() ), SLOT( _newEntry() ) );

    editEntryAction_ = new QAction( IconEngine::get( ICONS::EDIT ), "Edit Entries...", this );
    editEntryAction_->setToolTip( "Edit selected entries" );
    connect( editEntryAction_, SIGNAL( triggered() ), SLOT( _editEntries() ) );

    editEntryTitleAction_ = new QAction( IconEngine::get( ICONS::RENAME ), "Rename Entry...", this );
    editEntryTitleAction_->setToolTip( "Edit selected entry title" );
    editEntryTitleAction_->setShortcut( Qt::Key_F2 );
    editEntryTitleAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( editEntryTitleAction_, SIGNAL( triggered() ), SLOT( _startEntryEdition() ) );

    /*
    delete entry action
    it is associated to the Qt::Key_Delete shortcut
    but the later is enabled only if the KeywordList has focus.
    */
    deleteEntryAction_ = new QAction( IconEngine::get( ICONS::DELETE ), "Delete Entries", this );
    deleteEntryAction_->setToolTip( "Delete selected entries" );
    deleteEntryAction_->setShortcut( QKeySequence::Delete );
    deleteEntryAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( deleteEntryAction_, SIGNAL( triggered() ), SLOT( _deleteEntries() ) );

    // color menu
    colorMenu_ = new ColorMenu( this );
    colorMenu_->setTitle( "Change entry color" );
    connect( colorMenu_, SIGNAL( selected( QColor ) ), SLOT( _changeEntryColor( QColor ) ) );

    entryColorAction_ = new QAction( IconEngine::get( ICONS::COLOR ), "Entry Color", this );
    entryColorAction_->setToolTip( "Change selected entries color" );
    entryColorAction_->setMenu( colorMenu_ );

    entryKeywordAction_ = new QAction( IconEngine::get( ICONS::EDIT ), "Change Keyword...", this );
    entryKeywordAction_->setToolTip( "Edit selected entries keyword" );
    connect( entryKeywordAction_, SIGNAL( triggered() ), SLOT( _renameEntryKeyword() ) );

    newLogbookAction_ = new QAction( IconEngine::get( ICONS::NEW ), "New Logbook...", this );
    newLogbookAction_->setToolTip( "Create a new logbook" );
    connect( newLogbookAction_, SIGNAL( triggered() ), SLOT( _newLogbook() ) );

    openAction_ = new QAction( IconEngine::get( ICONS::OPEN ), "Open...", this );
    openAction_->setToolTip( "Open an existsing logbook" );
    openAction_->setShortcut( QKeySequence::Open );
    connect( openAction_, SIGNAL( triggered() ), SLOT( open() ) );

    synchronizeAction_ = new QAction( IconEngine::get( ICONS::MERGE ), "Synchronize...", this );
    synchronizeAction_->setToolTip( "Synchronize current logbook with remote" );
    connect( synchronizeAction_, SIGNAL( triggered() ), SLOT( _synchronize() ) );

    reorganizeAction_ = new QAction( "Reorganize", this );
    reorganizeAction_->setToolTip( "Reoganize logbook entries in files" );
    connect( reorganizeAction_, SIGNAL( triggered() ), SLOT( _reorganize() ) );

    saveAction_ = new QAction( IconEngine::get( ICONS::SAVE ), "Save", this );
    saveAction_->setToolTip( "Save all edited entries" );
    connect( saveAction_, SIGNAL( triggered() ), SLOT( save() ) );

    saveForcedAction_ = new QAction( IconEngine::get( ICONS::SAVE ), "Save (forced)", this );
    saveForcedAction_->setToolTip( "Save all entries" );
    connect( saveForcedAction_, SIGNAL( triggered() ), SLOT( _saveForced() ) );

    saveAsAction_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save As...", this );
    saveAsAction_->setToolTip( "Save logbook with a different name" );
    connect( saveAsAction_, SIGNAL( triggered() ), SLOT( _saveAs() ) );

    saveBackupAction_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save Backup...", this );
    saveBackupAction_->setToolTip( "Save logbook backup" );
    connect( saveBackupAction_, SIGNAL( triggered() ), SLOT( _saveBackup() ) );

    backupManagerAction_ = new QAction( IconEngine::get( ICONS::CONFIGURE_BACKUPS ), "Manage Backups...", this );
    backupManagerAction_->setToolTip( "Save logbook backup" );
    connect( backupManagerAction_, SIGNAL( triggered() ), SLOT( _manageBackups() ) );

    revertToSaveAction_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Reload", this );
    revertToSaveAction_->setToolTip( "Restore saved logbook" );
    revertToSaveAction_->setShortcut( QKeySequence::Refresh );
    connect( revertToSaveAction_, SIGNAL( triggered() ), SLOT( _revertToSaved() ) );

    // print
    printAction_ = new QAction( IconEngine::get( ICONS::PRINT ), "Print...", this );
    printAction_->setShortcut( QKeySequence::Print );
    connect( printAction_, SIGNAL( triggered() ), SLOT( _print() ) );

    // print preview
    addAction( printPreviewAction_ = new QAction( IconEngine::get( ICONS::PRINT_PREVIEW ), "Print Preview...", this ) );
    printPreviewAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_P );
    connect( printPreviewAction_, SIGNAL( triggered() ), SLOT( _printPreview() ) );

    // export to HTML
    htmlAction_ = new QAction( IconEngine::get( ICONS::HTML ), "Export to HTML...", this );
    connect( htmlAction_, SIGNAL( triggered() ), SLOT( _toHtml() ) );

    logbookStatisticsAction_ = new QAction( IconEngine::get( ICONS::INFORMATION ), "Logbook Statistics...", this );
    logbookStatisticsAction_->setToolTip( "View logbook statistics" );
    connect( logbookStatisticsAction_, SIGNAL( triggered() ), SLOT( _viewLogbookStatistics() ) );

    logbookInformationsAction_ = new QAction( IconEngine::get( ICONS::INFORMATION ), "Logbook Properties...", this );
    logbookInformationsAction_->setToolTip( "Edit logbook properties" );
    connect( logbookInformationsAction_, SIGNAL( triggered() ), SLOT( _editLogbookInformations() ) );

    closeFramesAction_ = new QAction( IconEngine::get( ICONS::CLOSE ), "Close Editors", this );
    closeFramesAction_->setToolTip( "Close all entry editors" );
    connect( closeFramesAction_, SIGNAL( triggered() ), SLOT( _closeEditionWindows() ) );

    // show duplicated entries
    showDuplicatesAction_ = new QAction( "Show Duplicated Entries...", this );
    showDuplicatesAction_->setToolTip( "Show duplicated entries in logbook" );
    connect( showDuplicatesAction_, SIGNAL( triggered() ), SLOT( _showDuplicatedEntries() ) );

    // view monitored files
    monitoredFilesAction_ = new QAction( "Show Monitored Files...", this );
    monitoredFilesAction_->setToolTip( "Show monitored files" );
    connect( monitoredFilesAction_, SIGNAL( triggered() ), SLOT( _showMonitoredFiles() ) );

    // tree mode
    treeModeAction_ = new QAction( "Use Tree to Display Entries and Keywords", this );
    treeModeAction_->setCheckable( true );
    treeModeAction_->setChecked( true );
    connect( treeModeAction_, SIGNAL( toggled( bool ) ), SLOT( _toggleTreeMode( bool ) ) );

}

//_______________________________________________
void MainWindow::_resetLogEntryList( void )
{

    Debug::Throw( "MainWindow::_resetLogEntryList.\n" );

    // animation
    entryList_->initializeAnimation();

    // clear list of entries
    entryModel_.clear();

    if( logbook_ )
    {

        LogEntryModel::List modelEntries;
        foreach( LogEntry* entry, logbook_->entries() )
        {
            if( (!treeModeAction().isChecked() && entry->isFindSelected()) || entry->isSelected() )
            { modelEntries << entry; }
        }

        entryModel_.add( modelEntries );

    }

    // loop over associated editionwindows
    // update navigation buttons
    BASE::KeySet<EditionWindow> windows( this );
    foreach( EditionWindow* window, windows )
    {

        // skip closed editors
        if( window->isClosed() ) continue;

        // get associated entry and see if selected
        LogEntry* entry( window->entry() );
        window->previousEntryAction().setEnabled( entry && entry->isSelected() && previousEntry(entry, false) );
        window->nextEntryAction().setEnabled( entry && entry->isSelected() && nextEntry(entry, false) );

    }

    // animation
    entryList_->startAnimation();

    return;

}

//_______________________________________________
void MainWindow::_resetKeywordList( void )
{

    Debug::Throw( "MainWindow::_resetKeywordList.\n" );
    Q_CHECK_PTR( logbook_ );

    // animation
    keywordList_->initializeAnimation();

    // retrieve new list of keywords (from logbook)
    KeywordModel::List newKeywords;
    foreach( LogEntry* entry, logbook_->entries() )
    {
        if( entry->isFindSelected() )
        {
            Keyword keyword( entry->keyword() );
            while( keyword != Keyword::NoKeyword )
            {
                if( !newKeywords.contains( keyword ) ) newKeywords << keyword;
                keyword = keyword.parent();
            }

        }
    }

    keywordModel_.set( newKeywords );

    // animation
    keywordList_->startAnimation();

}

//_______________________________________________
void MainWindow::_loadColors( void )
{

    Debug::Throw( "MainWindow::_loadColors.\n" );

    if( !logbook_ ) return;

    //! retrieve all entries
    foreach( LogEntry* entry, logbook_->entries() )
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
    foreach( QToolBar* toolbar, qFindChildren<QToolBar*>( this ) )
    { toolbar->setEnabled( value ); }

}


//__________________________________________________________________
bool MainWindow::_hasModifiedEntries( void ) const
{
    BASE::KeySet<EditionWindow> frames( this );
    return std::find_if( frames.begin(), frames.end(), EditionWindow::ModifiedFTor() ) != frames.end();
}

//_______________________________________________
void MainWindow::_autoSave( void )
{

    if( logbook_ && !logbook_->file().isEmpty() )
    {

        statusbar_->label().setText( "performing autoSave" );

        // retrieve non read only editors; perform save
        BASE::KeySet<EditionWindow> windows( this );
        foreach( EditionWindow* window, windows )
        {
            if( window->isReadOnly() || window->isClosed() ) continue;
            window->saveAction().trigger();
        }

        save();

    } else {

        statusbar_->label().setText( "no logbook filename. <Autosave> skipped" );

    }

}

//__________________________________________________________________
AskForSaveDialog::ReturnCode MainWindow::_checkModifiedEntries( BASE::KeySet<EditionWindow> windows, const bool& confirmEntries ) const
{
    Debug::Throw( "_MainWindow::checkModifiedEntries.\n" );

    // check if editable EditionWindows needs save
    // cancel if required
    foreach( EditionWindow* window, windows )
    {
        if( !(window->isReadOnly() || window->isClosed()) && window->modified() )
        {
            if( !confirmEntries ) { window->saveAction().trigger(); }
            else if( window->askForSave() == AskForSaveDialog::CANCEL ) return AskForSaveDialog::CANCEL;
        }
    }

    return  AskForSaveDialog::YES;
}

//_______________________________________________
void MainWindow::_updateEntryFrames( LogEntry* entry, unsigned int mask )
{
    Debug::Throw( "MainWindow::_updateEntryFrames.\n" );

    if( !mask ) return;

    // update associated EditionWindows
    BASE::KeySet<EditionWindow> windows( entry );
    foreach( EditionWindow* window, windows )
    {

        // keep track of already modified EditionWindows
        bool windowModified( window->modified() && !window->isReadOnly() );

        // update EditionWindow
        if( mask&TITLE_MASK ) window->displayTitle();
        if( mask&KEYWORD_MASK ) window->displayKeyword();

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

    // ask dialog and take action accordinly
    int state = LogbookModifiedDialog( this, files ).exec();
    if( state == LogbookModifiedDialog::RESAVE ) { save(); }
    else if( state == LogbookModifiedDialog::SAVE_AS ) { _saveAs(); }
    else if( state == LogbookModifiedDialog::RELOAD )
    {

        logbook_->setModifiedRecursive( false );
        _revertToSaved();

    } else if( state == LogbookModifiedDialog::IGNORE ) { ignoreWarnings_ = true; }

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
    if( logbook_ && logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // new logbook
    NewLogbookDialog dialog( this );
    dialog.setTitle( Logbook::LOGBOOK_NO_TITLE );
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

        QString buffer;
        QTextStream(&buffer ) << "File \"" << directory << "\" is not a directory.";
        InformationDialog( this, buffer ).exec();

    } else logbook_->setDirectory( directory );

}

//_______________________________________________
void MainWindow::setModified( bool value )
{

    Debug::Throw() << "MainWindow::setModified - " << value << endl;

    QString buffer;
    QTextStream what( &buffer );
    if( logbook_ && !logbook_->file().isEmpty() )
    {

        what << logbook_->file().localName();
        if( value ) what << " (modified)";
        what << " - Elogbook";

    } else {

        what << "Elogbook";
        if( value ) what << " (modified)";

    }

    setWindowTitle( buffer );

}

//_______________________________________________
void MainWindow::open( FileRecord record )
{

    Debug::Throw( "MainWindow::open.\n" );

    // check if current logbook needs save
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), confirmEntries_ ) == AskForSaveDialog::CANCEL ) return;
    if( logbook_ && logbook_->modified()  && askForSave() == AskForSaveDialog::CANCEL ) return;

    // open file from dialog if not set as argument
    if( record.file().isEmpty() )
    {

        QString file( FileDialog(this).selectFile( workingDirectory() ).getFile() );
        if( file.isNull() ) return;
        else record = FileRecord( file );

    }

    // create logbook from file
    Singleton::get().application<Application>()->busy();
    setLogbook( record.file() );
    Singleton::get().application<Application>()->idle();

    // check if backup is needed
    checkLogbookBackup();

    return;
}

//_______________________________________________
bool MainWindow::_saveAs( File defaultFile, bool registerLogbook )
{
    Debug::Throw( "MainWindow::_saveAs.\n");

    // check current logbook
    if( !logbook_ ) {
        InformationDialog( this, "no logbook opened. <Save Logbook> canceled." ).exec();
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
    if( !logbook_ ) {
        InformationDialog( this, "no Logbook opened. <Save> canceled." ).exec();
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
        InformationDialog( this, "no logbook opened. <Save Backup> canceled." ).exec();
        return;
    }

    QString filename( logbook_->backupFilename( ) );
    if( filename.isEmpty() ) {
        InformationDialog( this, "no valid filename. Use <Save As> first." ).exec();
        return;
    }

    // store last backup time and update
    TimeStamp lastBackup( logbook_->backup() );

    // stores current logbook filename
    QString currentFilename( logbook_->file() );

    // save logbook as backup
    bool saved( _saveAs( filename, false ) );

    // remove the "backup" filename from the openPrevious list
    // to avoid confusion
    Singleton::get().application<Application>()->recentFiles().remove( File(filename).expand() );

    // restore initial filename
    logbook_->setFile( currentFilename );

    if( saved ) {

        logbook_->addBackup( filename );
        logbook_->setModified( true );
        setModified( true );

        // Save logbook if needed (to make sure the backup stamp is updated)
        if( !logbook_->file().isEmpty() ) save();
    }

}

//_______________________________________________
void MainWindow::_manageBackups( void )
{
    Debug::Throw( "MainWindow::_manageBackups.\n");

    BackupManagerDialog dialog( this );
    Key::associate( &dialog.managerWidget(), logbook_ );
    dialog.managerWidget().updateBackups();

    // connections
    connect( &dialog.managerWidget(), SIGNAL( saveLogbookRequested( void ) ), SLOT( save( void ) ) );
    connect( &dialog.managerWidget(), SIGNAL( backupRequested( void ) ), SLOT( _saveBackup( void ) ) );
    connect( &dialog.managerWidget(), SIGNAL( removeBackupRequested( Logbook::Backup ) ), SLOT( _removeBackup( Logbook::Backup ) ) );
    connect( &dialog.managerWidget(), SIGNAL( restoreBackupRequested( Logbook::Backup ) ), SLOT( _restoreBackup( Logbook::Backup ) ) );
    connect( &dialog.managerWidget(), SIGNAL( mergeBackupRequested( Logbook::Backup ) ), SLOT( _mergeBackup( Logbook::Backup ) ) );

    dialog.exec();
}

//_____________________________________________
void MainWindow::_revertToSaved( void )
{
    Debug::Throw( "MainWindow::_revertToSaved.\n" );

    // check logbook
    if( !logbook_ ){
        InformationDialog( this, "No logbook opened. <Reload> canceled." ).exec();
        return;
    }

    // ask for confirmation
    QString buffer;
    QTextStream( &buffer ) << "Discard changes to " << logbook_->file().localName() << " ?";
    if( ( _hasModifiedEntries() || logbook_->modified() ) && !QuestionDialog( this, buffer ).exec() )
    { return; }

    // reinit MainWindow
    Singleton::get().application<Application>()->busy();
    setLogbook( logbook_->file() );
    Singleton::get().application<Application>()->idle();

    checkLogbookBackup();
    ignoreWarnings_ = false;

}

//___________________________________________________________
void MainWindow::_print( void )
{
    Debug::Throw( "MainWindow::_print.\n" );

    // save EditionWindows
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::CANCEL ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create printer
    QPrinter printer( QPrinter::HighResolution );

    // generate document name
    QString buffer;
    QTextStream( &buffer )  << "elogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid();
    printer.setDocName( buffer );

    // create helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook_ );
    helper.setEntries(
        BASE::KeySet<LogEntry>( logbook_->entries() ).toList(),
        entryModel_.get(),
        entryModel_.get( entryList_->selectionModel()->selectedRows() ) );

    // create options widget
    PrinterOptionWidget* optionWidget( new PrinterOptionWidget() );
    optionWidget->setHelper( &helper );
    connect( optionWidget, SIGNAL( orientationChanged( QPrinter::Orientation ) ), &helper, SLOT( setOrientation( QPrinter::Orientation ) ) );
    connect( optionWidget, SIGNAL( pageModeChanged( BasePrintHelper::PageMode ) ), &helper, SLOT( setPageMode( BasePrintHelper::PageMode ) ) );

    LogbookPrintOptionWidget* logbookOptionWidget = new LogbookPrintOptionWidget();
    connect( logbookOptionWidget, SIGNAL( maskChanged( unsigned int ) ), &helper, SLOT( setMask( unsigned int ) ) );
    logbookOptionWidget->read();

    LogEntryPrintOptionWidget* logEntryOptionWidget = new LogEntryPrintOptionWidget();
    connect( logEntryOptionWidget, SIGNAL( maskChanged( unsigned int ) ), &helper, SLOT( setEntryMask( unsigned int ) ) );
    logEntryOptionWidget->read();

    LogEntryPrintSelectionWidget* logEntrySelectionWidget = new LogEntryPrintSelectionWidget();
    connect( logEntrySelectionWidget, SIGNAL( modeChanged( LogEntryPrintSelectionWidget::Mode ) ), &helper, SLOT( setSelectionMode( LogEntryPrintSelectionWidget::Mode ) ) );
    logEntrySelectionWidget->read();

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setOptionTabs( QList<QWidget *>()
        << optionWidget
        << logEntrySelectionWidget
        << logbookOptionWidget
        << logEntryOptionWidget );

    dialog.setWindowTitle( "Print Logbook - elogbook" );
    if( dialog.exec() == QDialog::Rejected ) return;

    // add output file to scratch files, if any
    if( !printer.outputFileName().isEmpty() )
    { emit scratchFileCreated( printer.outputFileName() ); }

    // write options
    logbookOptionWidget->write();
    logEntrySelectionWidget->write();
    logEntryOptionWidget->write();

    // retrieve mask and assign
    helper.setMask( logbookOptionWidget->mask() );
    helper.setEntryMask( logEntryOptionWidget->mask() );
    helper.setSelectionMode( logEntrySelectionWidget->mode() );

    // print
    helper.print( &printer );

    // reset status bar
    statusbar_->label().setText( "" );
    statusbar_->showLabel();

    return;

}

//___________________________________________________________
void MainWindow::_printPreview( void )
{
    Debug::Throw( "MainWindow::_printPreview.\n" );

    // save EditionWindows
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::CANCEL ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook_ );
    helper.setEntries(
        BASE::KeySet<LogEntry>( logbook_->entries() ).toList(),
        entryModel_.get(),
        entryModel_.get( entryList_->selectionModel()->selectedRows() ) );

    helper.setSelectionMode( (LogEntryPrintSelectionWidget::Mode) XmlOptions::get().get<unsigned int>( "LOGENTRY_PRINT_SELECTION" ) );

    // masks
    helper.setMask( XmlOptions::get().get<unsigned int>( "LOGBOOK_PRINT_OPTION_MASK" ) );
    helper.setEntryMask( XmlOptions::get().get<unsigned int>( "LOGENTRY_PRINT_OPTION_MASK" ) );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this );
    dialog.setWindowTitle( "Print Preview - elogbook" );
    dialog.setHelper( &helper );
    dialog.exec();

    // reset status bar
    statusbar_->label().setText( "" );
    statusbar_->showLabel();

}

//___________________________________________________________
void MainWindow::_toHtml( void )
{
    Debug::Throw( "MainWindow::_toHtml.\n" );

    // save EditionWindows
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::CANCEL ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create options widget
    LogbookPrintOptionWidget* logbookOptionWidget = new LogbookPrintOptionWidget();
    logbookOptionWidget->read();

    LogEntryPrintSelectionWidget* logEntrySelectionWidget = new LogEntryPrintSelectionWidget();
    logEntrySelectionWidget->read();

    LogEntryPrintOptionWidget* logEntryOptionWidget = new LogEntryPrintOptionWidget();
    logEntryOptionWidget->read();

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
        InformationDialog(this, "No output file specified. <View HTML> canceled." ).exec();
        return;
    }

    QFile out( file );
    if( !out.open( QIODevice::WriteOnly ) )
    {
        QString buffer;
        QTextStream( &buffer ) << "Cannot write to file \"" << file << "\". <View HTML> canceled.";
        InformationDialog( this, buffer ).exec();
        return;
    }

    // add as scratch file
    emit scratchFileCreated( file );

    // write options
    logbookOptionWidget->write();
    logEntrySelectionWidget->write();
    logEntryOptionWidget->write();

    // create print helper
    LogbookHtmlHelper helper( this );
    helper.setLogbook( logbook_ );

    // select entries
    helper.setEntries( _entries( logEntrySelectionWidget->mode() ) );

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
        InformationDialog( this, "No logbook opened. <Merge> canceled." ).exec();
        return;
    }

    // save EditionWindows
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::CANCEL ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create file dialog
    File remoteFile( FileDialog(this).getFile() );
    if( remoteFile.isNull() ) return;

    // debug
    Debug::Throw() << "MainWindow::_synchronize - number of local files: " << logbook_->children().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - number of local entries: " << logbook_->entries().size() << endl;

    // set busy flag
    Singleton::get().application<Application>()->busy();
    statusbar_->label().setText( "reading remote logbook ... " );

    // opens file in remote logbook
    Debug::Throw() << "MainWindow::_synchronize - reading remote logbook from file: " << remoteFile << endl;

    Logbook remoteLogbook;
    connect( &remoteLogbook, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );
    remoteLogbook.setFile( remoteFile );
    remoteLogbook.read();

    // check if logbook is valid
    XmlError::List errors( remoteLogbook.xmlErrors() );
    if( errors.size() )
    {

        QString buffer;
        QTextStream what( &buffer );
        if( errors.size() > 1 ) what << "Errors occured while parsing files." << endl;
        else what << "An error occured while parsing files." << endl;
        what << errors;
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
    for( QHash<LogEntry*,LogEntry*>::iterator iter = duplicates.begin(); iter != duplicates.end(); ++iter )
    {

        // display the new entry in all matching edit frames
        BASE::KeySet<EditionWindow> windows( iter.key() );
        foreach( EditionWindow* window, windows )
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
    BASE::KeySet<LogEntry> entries( logbook_->entries() );
    BASE::KeySet<LogEntry>::const_iterator iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    entryList_->setFocus();

    // write local logbook
    if( !logbook_->file().isEmpty() ) save();

    // synchronize remove with local
    Debug::Throw() << "MainWindow::_synchronize - updating remote from local" << endl;
    unsigned int nDuplicated = remoteLogbook.synchronize( *logbook_ ).size();
    Debug::Throw() << "MainWindow::_synchronize - number of duplicated entries: " << nDuplicated << endl;

    // save remote logbook
    statusbar_->label().setText( "saving remote logbook ... " );
    remoteLogbook.write();

    // idle
    Singleton::get().application<Application>()->idle();
    statusbar_->label().setText( "" );

    return;

}

//_______________________________________________
void MainWindow::_removeBackup( Logbook::Backup backup )
{
    Debug::Throw( "MainWindow::_removeBackup.\n" );
    if( !backup.file().exists() )
    {
        QString buffer;
        QTextStream( &buffer ) << "Unable to open file named " << backup.file() << ". <Remove Backup> canceled";
        InformationDialog( this, buffer ).exec();
        return;
    }

    // read backup
    Logbook backupLogbook;
    backupLogbook.setFile( backup.file() );
    backupLogbook.read();

    // get list of children
    Logbook::List all( backupLogbook.children() );
    all.push_front( &backupLogbook );

    // remove all files
    foreach( Logbook* logbook, all )
    { logbook->file().remove(); }

    // clean logbook backups
    Logbook::Backup::List backups( logbook_->backupFiles() );
    Logbook::Backup::List::iterator iter = std::find( backups.begin(), backups.end(), backup );
    if( iter != backups.end() )
    {
        backups.erase( iter );
        logbook_->setBackupFiles( backups );
        if( !logbook_->file().isEmpty() )
        { save(); }
    }

}

//_______________________________________________
void MainWindow::_restoreBackup( Logbook::Backup backup )
{
    Debug::Throw( "MainWindow::_restoreBackup.\n" );
    if( !backup.file().exists() )
    {
        QString buffer;
        QTextStream( &buffer ) << "Unable to open file named " << backup.file() << ". <Remove Backup> canceled";
        InformationDialog( this, buffer ).exec();
        return;
    }

    // store old filename
    File oldName( logbook_->file() );

    // store old backups
    Logbook::Backup::List backups( logbook_->backupFiles() );

    // store associated backup manager Widget
    BASE::KeySet<BackupManagerWidget> widgets( logbook_ );

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
    foreach( BackupManagerWidget* widget, widgets )
    { BASE::Key::associate( widget, logbook_ ); }

    // and save
    if( !logbook_->file().isEmpty() )
    {
        save();
        Singleton::get().application<Application>()->recentFiles().add( logbook_->file().expand() );
    }

}

//_______________________________________________
void MainWindow::_mergeBackup( Logbook::Backup backup )
{
    Debug::Throw( "MainWindow::_mergeBackup.\n" );

    // check current logbook is valid
    if( !logbook_ ) {
        InformationDialog( this, "No logbook opened. <Merge> canceled." ).exec();
        return;
    }

    if( !backup.file().exists() )
    {
        QString buffer;
        QTextStream( &buffer ) << "Unable to open file named " << backup.file() << ". <Remove Backup> canceled";
        InformationDialog( this, buffer ).exec();
        return;
    }

    // save EditionWindows
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::CANCEL ) return;

    // save current logbook
    if( logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // debug
    Debug::Throw() << "MainWindow::_mergeBackup - number of local files: " << logbook_->children().size() << endl;
    Debug::Throw() << "MainWindow::_mergeBackup - number of local entries: " << logbook_->entries().size() << endl;

    // set busy flag
    Singleton::get().application<Application>()->busy();
    statusbar_->label().setText( "reading remote logbook ... " );

    // opens file in remote logbook
    Debug::Throw() << "MainWindow::_mergeBackup - reading remote logbook from file: " << backup.file() << endl;

    Logbook backupLogbook;
    connect( &backupLogbook, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );
    backupLogbook.setFile( backup.file() );
    backupLogbook.read();

    // check if logbook is valid
    XmlError::List errors( backupLogbook.xmlErrors() );
    if( errors.size() )
    {

        QString buffer;
        QTextStream what( &buffer );
        if( errors.size() > 1 ) what << "Errors occured while parsing files." << endl;
        else what << "An error occured while parsing files." << endl;
        what << errors;
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
    for( QHash<LogEntry*,LogEntry*>::iterator iter = duplicates.begin(); iter != duplicates.end(); ++iter )
    {

        // display the new entry in all matching edit frames
        BASE::KeySet<EditionWindow> windows( iter.key() );
        foreach( EditionWindow* window, windows )
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
    BASE::KeySet<LogEntry> entries( logbook_->entries() );
    BASE::KeySet<LogEntry>::const_iterator iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
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
        InformationDialog( this,"No valid logbook. Canceled.\n").exec();
        return;
    }

    // retrieve all entries
    BASE::KeySet<LogEntry> entries( logbook_->entries() );
    foreach( LogEntry* entry, entries )
    {

        BASE::KeySet<Logbook> logbooks( entry );
        foreach( Logbook* logbook, logbooks )
        { logbook->setModified( true ); }

        entry->clearAssociations<Logbook>();

    }

    //! put entry set into a list and sort by creation time.
    // First entry must the oldest
    QList<LogEntry*> entryList( entries.toList() );
    std::sort( entryList.begin(), entryList.end(), LogEntry::FirstCreatedFTor() );

    // put entries in logbook
    foreach( LogEntry* entry, entryList )
    {
        Logbook *logbook( MainWindow::logbook_->latestChild() );
        Key::associate( entry, logbook );
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
    QModelIndex current_index( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? entryModel_.get( current_index ):0 );

    // keep track of found entries
    int found( 0 );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> entries( logbook_->entries() );
    BASE::KeySet<LogEntry> turnedOffEntries;
    foreach( LogEntry* entry, entries )
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

        InformationDialog( this, "No matching entry found.\nRequest canceled." ).centerOnParent().exec();

        // reset flag for the turned off entries to true
        foreach( LogEntry* entry, turnedOffEntries )
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
        InformationDialog( this, "No logbook opened." ).exec();
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
        InformationDialog( this, "No logbook opened." ).exec();
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

    // retrieve logbook directory
    File directory = dialog.AttachmentDirectory();

    // check if fulldir is not a non directory existsing file
    if( directory.exists() &&  !directory.isDirectory() )
    {

        QString buffer;
        QTextStream(&buffer ) << "File \"" << directory << "\" is not a directory.";
        InformationDialog( this, buffer ).exec();

    } else modified |= logbook_->setDirectory( directory );


    // save Logbook, if needed
    if( modified ) logbook_->setModified( true );
    if( !logbook_->file().isEmpty() ) save();
    else setModified( true );

}

//_______________________________________________
void MainWindow::_closeEditionWindows( bool askForSave ) const
{
    Debug::Throw( "MainWindow::_closeEditionWindows.\n" );

    // get all EditionWindows from MainWindow
    BASE::KeySet<EditionWindow> windows( this );
    if( askForSave && _checkModifiedEntries( windows, true ) == AskForSaveDialog::CANCEL ) return;
    foreach( EditionWindow* window, windows )  window->deleteLater();

    return;

}

//____________________________________________
void MainWindow::_findEntries( void ) const
{

    Debug::Throw( "MainWindow::_findEntries.\n" );

    // check panel visibility
    if( !searchPanel().isVisible() ) {
        searchPanel().editor().clear();
        searchPanel().visibilityAction().trigger();
    } else searchPanel().editor().lineEdit()->selectAll();

    // change focus
    searchPanel().editor().setFocus();

}

//____________________________________________
void MainWindow::_newEntry( void )
{

    Debug::Throw( "MainWindow::_NewEntry.\n" );

    // retrieve associated EditionWindows, check if one matches the selected entry
    EditionWindow *editionWindow( 0 );
    BASE::KeySet<EditionWindow> frames( this );
    BASE::KeySetIterator<EditionWindow> iterator( frames );
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
        Key::associate( this, editionWindow );
        connect( editionWindow, SIGNAL( scratchFileCreated( const File& ) ), this, SIGNAL( scratchFileCreated( const File& ) ) );
    }

    // force editionWindow show keyword flag
    editionWindow->setForceShowKeyword( !treeModeAction().isChecked() );

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
    LogEntryModel::List selection( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
    if( selection.empty() ) {
        InformationDialog( this, "no entry selected. Request canceled.").exec();
        return;
    }

    // retrieve associated entry
    foreach( LogEntry* entry, selection )
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
    bool hasEditedIndex( false );
    foreach( const QModelIndex& index, selectedIndexes )
    {
        // check if index is not being edited
        if( entryModel_.editionEnabled() && index ==  entryModel_.editionIndex() )
        {
            hasEditedIndex = true;
            InformationDialog( this, "Cannot delete entry that is being edited." ).exec();
        } else selection << entryModel_.get( index );
    }

    // check selection size
    if( selection.empty() && !hasEditedIndex )
    {
        InformationDialog( this, "no entry selected. Request canceled.").exec();
        return;
    }

    // ask confirmation
    QString buffer;
    QTextStream( &buffer ) << "Delete selected entr" << ( selection.size() == 1 ? "y":"ies" );
    QuestionDialog dialog( this, buffer );
    dialog.okButton().setText( "Delete" );
    dialog.okButton().setIcon( IconEngine::get( ICONS::DELETE ) );
    if( !dialog.exec() ) return;

    // retrieve associated entry
    foreach( LogEntry* entry, selection )
    { deleteEntry( entry, false ); }

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
    BASE::KeySet<EditionWindow> windows( this );
    foreach( EditionWindow* window, windows )
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
        BASE::KeySetIterator<EditionWindow> iterator( windows );
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
            editionWindow->displayEntry( entry );

            // also kill all frames but one
            BASE::KeySet< EditionWindow::LocalTextEditor > editors( editionWindow );
            if( editors.size() > 1 )
            {

                BASE::KeySet<EditionWindow::LocalTextEditor>::iterator localIter( editors.begin() );
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
        Key::associate( this, editionWindow );
        editionWindow->displayEntry( entry );

        connect( editionWindow, SIGNAL( scratchFileCreated( const File& ) ), this, SIGNAL( scratchFileCreated( const File& ) ) );

    }

    editionWindow->setForceShowKeyword( !treeModeAction().isChecked() );
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
    _updateEntryFrames( entry, TITLE_MASK );

    // set logbooks as modified
    BASE::KeySet<Logbook> logbooks( entry );
    foreach( Logbook* logbook, logbooks ) logbook->setModified( true );

    // save Logbook
    if( logbook_ && !logbook_->file().isEmpty() ) save();

}

//_______________________________________________
void MainWindow::_changeEntryColor( QColor color )
{
    Debug::Throw( "MainWindow::_changeEntryColor.\n" );

    // retrieve current selection
    LogEntryModel::List selection( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no entry selected. Request canceled.").exec();
        return;
    }

    // retrieve associated entry
    foreach( LogEntry* entry, selection )
    {

        entry->setColor( color.isValid() ? color.name():ColorMenu::NONE );
        entry->setModification( entry->modification()+1 );

        // update EditionWindow color
        BASE::KeySet<EditionWindow> windows( entry );
        foreach( EditionWindow* window, windows )
        { if( !window->isClosed() ) window->displayColor(); }

        // set logbooks as modified
        BASE::KeySet<Logbook> logbooks( entry );
        foreach( Logbook* logbook, logbooks ) logbook->setModified( true );

    }

    // update model
    entryModel_.add( selection );

    // save Logbook
    if( !logbook_->file().isEmpty() ) save();

}

//____________________________________________
void MainWindow::_newKeyword( void )
{

    Debug::Throw( "MainWindow::_newKeyword.\n" );

    //! create dialog
    EditKeywordDialog dialog( this );
    dialog.setWindowTitle( "New Keyword - elogbook" );

    foreach( const Keyword& keyword, keywordModel_.children() )
    { dialog.add( keyword ); }

    dialog.setKeyword( currentKeyword() );

    // map dialog
    if( !dialog.centerOnParent().exec() ) return;

    // retrieve keyword from line_edit
    Keyword keyword( dialog.keyword() );
    if( keyword != Keyword::NoKeyword )
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

    //! check that keywordlist has selected item
    QModelIndexList selectedIndexes( keywordList_->selectionModel()->selectedRows() );
    if( selectedIndexes.empty() )
    {
        InformationDialog( this, "no keyword selected. Request canceled" ).exec();
        return;
    }

    // store corresponding list of keywords
    KeywordModel::List keywords;
    foreach( const QModelIndex& index, selectedIndexes )
    { if( index.isValid() ) keywords << keywordModel_.get( index ); }

    // retrieve associated entries
    BASE::KeySet<LogEntry> entries( logbook_->entries() );
    BASE::KeySet<LogEntry> associatedEntries;
    foreach( const Keyword& keyword, keywords )
    {
        foreach( LogEntry* entry, entries )
        { if( entry->keyword().inherits( keyword ) ) associatedEntries.insert( entry );  }
    }

    //! create dialog
    DeleteKeywordDialog dialog( this, keywords, !associatedEntries.empty() );
    if( !dialog.centerOnParent().exec() )
    { return; }

    if( dialog.moveEntries() && associatedEntries.size() )
    {

        Debug::Throw( "MainWindow::_deleteKeyword - moving entries.\n" );
        foreach( const Keyword& keyword, keywords )
        { _renameKeyword( keyword, keyword.parent(), false ); }

    } else if( dialog.deleteEntries() ) {

        Debug::Throw( "MainWindow::_deleteKeyword - deleting entries.\n" );
        foreach( LogEntry* entry, associatedEntries )
        { deleteEntry( entry, false ); }

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
            _resetLogEntryList();
            break;
        }

    }

    // Save logbook
    if( !logbook_->file().isEmpty() ) save();

    return;

}

//____________________________________________
void MainWindow::_renameKeyword( void )
{
    Debug::Throw("MainWindow::_renameKeyword.\n" );

    //! check that keywordlist has selected item
    QModelIndex current( keywordList_->selectionModel()->currentIndex() );
    if( !current.isValid() )
    {
        InformationDialog( this, "no keyword selected. Request canceled" ).exec();
        return;
    }

    keywordList_->edit( current );
    return;

}

//____________________________________________
void MainWindow::_renameKeyword( const Keyword& keyword, const Keyword& newKeyword, bool updateSelection )
{

    Debug::Throw("MainWindow::_renameKeyword.\n" );

    // check keywords are different
    if( keyword == newKeyword ) return;

    // get entries matching the oldKeyword, change the keyword
    foreach( LogEntry* entry, logbook_->entries() )
    {

        /*
        if keyword to modify is a leading subset of current entry keyword,
        update entry with new keyword
        */
        if( entry->keyword().inherits( keyword ) )
        {

            entry->setKeyword( Keyword( Str( entry->keyword().get() ).replace( keyword.get(), newKeyword.get() ) ) );

            /* this is a kludge: add 1 second to the entry modification timeStamp to avoid loosing the
            keyword change when synchronizing logbooks, without having all entries modification time
            set to now() */
            entry->setModification( entry->modification()+1 );

            // update frames
            _updateEntryFrames( entry, KEYWORD_MASK );

            // set associated logbooks as modified
            BASE::KeySet<Logbook> logbooks( entry );
            foreach( Logbook* logbook, logbooks ) logbook->setModified( true );

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
    LogEntryModel::List selection( entryModel_.get( entryList_->selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no entry selected. Request canceled.").exec();
        return;
    }

    //! check that current keyword make sense
    if( !keywordList_->selectionModel()->currentIndex().isValid() )
    {
        InformationDialog( this, "no keyword selected. Request canceled" ).exec();
        return;
    }

    // get current selected keyword
    Keyword keyword( keywordModel_.get( keywordList_->selectionModel()->currentIndex() ) );

    //! create dialog
    EditKeywordDialog dialog( this );
    dialog.setWindowTitle( "Edit Keyword - elogbook" );

    foreach( const Keyword& keyword, keywordModel_.children() )
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
void MainWindow::_renameEntryKeyword( Keyword newKeyword, bool updateSelection )
{

    Debug::Throw() << "MainWindow::_renameEntryKeyword - newKeyword: " << newKeyword << endl;

    // keep track of modified entries
    BASE::KeySet<LogEntry> entries;

    // retrieve current selection
    foreach( LogEntry* entry, entryModel_.get( entryList_->selectionModel()->selectedRows() ) )
    {

        // check if entry keyword has changed
        if( entry->keyword() == newKeyword ) continue;

        // change keyword and set as modified
        entry->setKeyword( newKeyword );

        /* this is a kludge: add 1 second to the entry modification timeStamp to avoid loosing the
        keyword change when synchronizing logbooks, without having all entries modification time
        set to now() */
        entry->setModification( entry->modification()+1 );

        // keep track of modified entries
        entries.insert( entry );

        // update frames
        _updateEntryFrames( entry, KEYWORD_MASK );

        // set associated logbooks as modified
        BASE::KeySet<Logbook> logbooks( entry );
        foreach( Logbook* logbook, logbooks ) logbook->setModified( true );

    }

    // check if at least one entry was changed
    if( entries.empty() ) return;

    // reset lists
    _resetKeywordList();

    // update keyword selection
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

    // update entry selection
    _resetLogEntryList();

    if( updateSelection )
    {
        // clear current selection
        entryList_->clearSelection();

        // select all modified entries
        QModelIndex lastIndex;
        foreach( LogEntry* entry, entries )
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
    }

    // Save logbook if needed
    if( !logbook_->file().isEmpty() ) save();

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

    // keep track of the current selected entry
    QModelIndex current_index( entryList_->selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? entryModel_.get( current_index ):0 );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> turnedOffEntries;
    foreach( LogEntry* entry, logbook_->entries() )
    { entry->setKeywordSelected( entry->keyword() == keyword ); }

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

    deleteKeywordAction().setEnabled( !keywordList_->selectionModel()->selectedRows().empty() );
    editKeywordAction().setEnabled( keywordList_->selectionModel()->currentIndex().isValid() );

    return;
}

//_____________________________________________
void MainWindow::_updateEntryActions( void )
{
    Debug::Throw( "MainWindow::_updateEntryActions.\n" );
    int selectedEntries( entryList_->selectionModel()->selectedRows().size() );
    bool hasSelection( selectedEntries > 0 );

    if( selectedEntries > 1 )
    {
        editEntryAction().setText( "Edit Entries" );
        deleteEntryAction().setText( "Delete Entries" );
    } else {
        editEntryAction().setText( "Edit Entry" );
        deleteEntryAction().setText( "Delete Entry" );
    }

    editEntryAction().setEnabled( hasSelection );
    deleteEntryAction().setEnabled( hasSelection );
    entryColorAction().setEnabled( hasSelection );
    entryKeywordAction().setEnabled( hasSelection );
    editEntryTitleAction().setEnabled( hasSelection );

    return;
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

        case LogEntryModel::COLOR: changed = logbook_->setSortMethod( Logbook::SORT_COLOR ); break;
        case LogEntryModel::TITLE: changed = logbook_->setSortMethod( Logbook::SORT_TITLE ); break;
        case LogEntryModel::CREATION: changed = logbook_->setSortMethod( Logbook::SORT_CREATION ); break;
        case LogEntryModel::MODIFICATION: changed = logbook_->setSortMethod( Logbook::SORT_MODIFICATION ); break;
        case LogEntryModel::AUTHOR: changed = logbook_->setSortMethod( Logbook::SORT_AUTHOR ); break;
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

    if( !( index.column() == LogEntryModel::TITLE ||  index.column() == LogEntryModel::KEYWORD ) )
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

    unsigned int mask(0);
    if( index.column() == LogEntryModel::TITLE ) mask = TITLE_MASK;
    else if( index.column() == LogEntryModel::KEYWORD ) mask = KEYWORD_MASK;

    // update associated EditionWindows
    _updateEntryFrames( entry, mask );

    // set logbooks as modified
    BASE::KeySet<Logbook> logbooks( entry );
    foreach( Logbook* logbook, logbooks ) logbook->setModified( true );

    // save Logbook
    if( logbook_ && !logbook_->file().isEmpty() ) save();

}

//________________________________________
void MainWindow::_startEntryEdition( void )
{

    Debug::Throw( "MainWindow::_startEntryEdition\n" );

    // get current index and check validity
    QModelIndex index( entryList_->currentIndex() );
    if( !index.isValid() ) return;

    // make sure 'title' index is selected
    index = entryModel_.index( index.row(), LogEntryModel::TITLE );

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

    // update log entry list
    _resetLogEntryList();

    // update log entry mask
    unsigned int mask( entryList_->mask() );
    if( value ) mask &= ~(1<<LogEntryModel::KEYWORD);
    else mask |= 1<<LogEntryModel::KEYWORD;
    entryList_->setMask( mask );
    entryList_->saveMask();

    entryList_->resizeColumns();

    // keyword toolbar visibility action
    keywordToolBar().visibilityAction().setEnabled( value );

    // force show keyword
    BASE::KeySet<EditionWindow> windows( this );
    foreach( EditionWindow* window, windows )
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
        if( value )
        {
            // select keyword
            QModelIndex index( keywordModel_.index( currentEntry->keyword() ) );
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

    // autoSave
    autoSaveDelay_ = 1000*XmlOptions::get().get<int>( "AUTO_SAVE_ITV" );
    bool autosave( XmlOptions::get().get<bool>( "AUTO_SAVE" ) );
    if( autosave ) autosaveTimer_.start( autoSaveDelay_, this );
    else autosaveTimer_.stop();

    // colors
    foreach( const Option& color, XmlOptions::get().specialOptions( "COLOR" ) )
    { colorMenu_->add( color.raw() ); }

    // max number of recent entries
    maxRecentEntries_ = XmlOptions::get().get<unsigned int>( "MAX_RECENT_ENTRIES" );

    // tree mode
    treeModeAction().setChecked( XmlOptions::get().get<bool>( "USE_TREE" ) );

}

//______________________________________________________________________
void MainWindow::KeywordList::setDefaultWidth( const int& value )
{ defaultWidth_ = value; }

//____________________________________________
QSize MainWindow::KeywordList::sizeHint( void ) const
{ return (defaultWidth_ ) >= 0 ? QSize( defaultWidth_, 0 ):TreeView::sizeHint(); }

//_______________________________________________
LogEntryModel::List MainWindow::_entries( LogEntryPrintSelectionWidget::Mode mode )
{

    switch( mode )
    {
        default:
        case LogEntryPrintSelectionWidget::ALL_ENTRIES:
        { BASE::KeySet<LogEntry>( logbook_->entries() ).toList(); }

        case LogEntryPrintSelectionWidget::VISIBLE_ENTRIES:
        return entryModel_.get();

        case LogEntryPrintSelectionWidget::SELECTED_ENTRIES:
        return entryModel_.get( entryList_->selectionModel()->selectedRows() );

    }

}
