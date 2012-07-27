
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
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "RecentFilesMenu.h"
#include "ScratchFileMonitor.h"
#include "SearchPanel.h"
#include "SelectionStatusBar.h"
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
    setStatusBar( statusbar_ = new SelectionStatusBar( this ) );
    connect( this, SIGNAL( messageAvailable( const QString& ) ), &statusBar().label(), SLOT( setTextAndUpdate( const QString& ) ) );
    connect( this, SIGNAL( messageAvailable( const QString& ) ), &statusBar().progressBar(), SLOT( setText( const QString& ) ) );

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
    keywordToolBar_->setAppearsInMenu( true );
    vLayout->addWidget( keywordToolBar_ );

    // keyword actions
    keywordToolBar_->addAction( &newKeywordAction() );
    keywordToolBar_->addAction( &deleteKeywordAction() );
    keywordToolBar_->addAction( &editKeywordAction() );
    Debug::Throw() << "MainWindow::MainWindow - keyword toolbar created." << endl;

    // create keyword list
    vLayout->addWidget( keywordList_ = new KeywordList( keywordContainer_ ), 1 );
    keywordList().setFindEnabled( false );
    keywordList().setModel( &_keywordModel() );
    keywordList().setRootIsDecorated( true );
    keywordList().setSortingEnabled( true );
    keywordList().setDragEnabled(true);
    keywordList().setAcceptDrops(true);
    keywordList().setDropIndicatorShown(true);
    keywordList().setOptionName( "KEYWORD_LIST" );

    // default width from options, if found
    if( XmlOptions::get().contains( "KEYWORD_LIST_WIDTH" ) )
    { keywordList_->setDefaultWidth( XmlOptions::get().get<int>( "KEYWORD_LIST_WIDTH" ) ); }

    // the use of a custom delegate unfortunately disable the
    // nice selection appearance of the oxygen style.
    keywordList().setItemDelegate( new TextEditionDelegate( this ) );

    // update LogEntryList when keyword selection change
    connect( keywordList().selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _keywordSelectionChanged( const QModelIndex& ) ) );
    connect( keywordList().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection& ) ), SLOT( _updateKeywordActions() ) );
    _updateKeywordActions();

    // rename selected entries when KeywordChanged is emitted with a single argument.
    // this correspond to drag and drop action from the logEntryList in the KeywordList
    connect( &keywordModel_, SIGNAL( entryKeywordChanged( Keyword ) ), SLOT( _renameEntryKeyword( Keyword ) ) );

    // rename all entries matching first keyword the second. This correspond to
    // drag and drop inside the keyword list, or to direct edition of a keyword list item.
    connect( &keywordModel_, SIGNAL( keywordChanged( Keyword, Keyword ) ), SLOT( _renameKeyword( Keyword, Keyword ) ) );

    // popup menu for keyword list
    keywordList().menu().addAction( &newEntryAction() );
    keywordList().menu().addAction( &newKeywordAction() );
    keywordList().menu().addAction( &deleteKeywordAction() );
    keywordList().menu().addAction( &editKeywordAction() );

    connect( &_keywordModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelectedKeywords() ) );
    connect( &_keywordModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeExpandedKeywords() ) );

    connect( &_keywordModel(), SIGNAL( layoutChanged() ), SLOT( _restoreSelectedKeywords() ) );
    connect( &_keywordModel(), SIGNAL( layoutChanged() ), SLOT( _restoreExpandedKeywords() ) );

    /*
    add the deleteKeywordAction to the keyword list,
    so that the corresponding shortcut gets activated whenever it is pressed
    while the list has focus
    */
    keywordList().addAction( &deleteKeywordAction() );
    keywordList().addAction( &editKeywordAction() );

    // right box for entries and buttons
    QWidget* right = new QWidget();

    vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing( 5 );
    right->setLayout( vLayout );

    entryToolBar_ = new CustomToolBar( "Entries Toolbar", right, "ENTRY_TOOLBAR" );
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
    logEntryList().setFindEnabled( false );
    logEntryList().setModel( &_logEntryModel() );
    logEntryList().setSelectionMode( QAbstractItemView::ContiguousSelection );
    logEntryList().setDragEnabled(true);
    logEntryList().setOptionName( "ENTRY_LIST" );
    logEntryList().lockColumnVisibility( LogEntryModel::KEYWORD );

    // the use of a custom delegate unfortunately disable the
    // nice selection appearance of the oxygen style.
    logEntryList().setItemDelegate( new TextEditionDelegate( this ) );

    connect( logEntryList().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
    connect( logEntryList().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateEntryActions() ) );
    connect( entryList_, SIGNAL( activated( const QModelIndex& ) ), SLOT( _entryItemActivated( const QModelIndex& ) ) );
    connect( entryList_, SIGNAL( clicked( const QModelIndex& ) ), SLOT( _entryItemClicked( const QModelIndex& ) ) );
    _updateEntryActions();

    connect( &_logEntryModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelectedEntries() ) );
    connect( &_logEntryModel(), SIGNAL( layoutChanged() ), SLOT( _restoreSelectedEntries() ) );
    connect( &_logEntryModel(), SIGNAL( layoutChanged() ), entryList_, SLOT( resizeColumns() ) );
    connect( &_logEntryModel(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _entryDataChanged( const QModelIndex& ) ) );

    /*
    add the deleteEntryAction to the list,
    so that the corresponding shortcut gets activated whenever it is pressed
    while the list has focus
    */
    logEntryList().addAction( &deleteEntryAction() );
    logEntryList().addAction( &editEntryTitleAction() );

    // create popup menu for list
    logEntryList().menu().addAction( &newEntryAction() );
    logEntryList().menu().addAction( &editEntryTitleAction() );
    logEntryList().menu().addAction( &editEntryAction() );
    logEntryList().menu().addAction( &entryKeywordAction() );
    logEntryList().menu().addAction( &deleteEntryAction() );
    logEntryList().menu().addAction( &entryColorAction() );

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
    logbook()->setFile( file );
    if( !file.exists() )
    {
        // update listView with new entries
        _resetKeywordList();
        _resetLogEntryList();
        emit ready();
        return false;
    }

    connect( logbook_, SIGNAL( maximumProgressAvailable( unsigned int ) ), &statusBar(), SLOT( showProgressBar() ) );
    connect( logbook_, SIGNAL( maximumProgressAvailable( unsigned int ) ), &statusBar().progressBar(), SLOT( setMaximumProgress( unsigned int ) ) );
    connect( logbook_, SIGNAL( progressAvailable( unsigned int ) ), &statusBar().progressBar(), SLOT( addToProgress( unsigned int ) ) );
    connect( logbook_, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );

    // one need to disable everything in the window
    // to prevent user to interact with the application while loading
    _setEnabled( false );
    logbook()->read();
    _setEnabled( true );

    Debug::Throw( "MainWindow::setLogbook - finished reading.\n" );

    // update listView with new entries
    _resetKeywordList();
    _resetLogEntryList();
    _loadColors();

    Debug::Throw( "MainWindow::setLogbook - lists set.\n" );

    // change sorting
    Qt::SortOrder sort_order( (Qt::SortOrder) logbook()->sortOrder() );
    Debug::Throw( "MainWindow::setLogbook - got sort order.\n" );

    switch( logbook()->sortMethod() )
    {
        case Logbook::SORT_COLOR: logEntryList().sortByColumn( LogEntryModel::COLOR, sort_order ); break;
        case Logbook::SORT_TITLE: logEntryList().sortByColumn( LogEntryModel::TITLE, sort_order ); break;
        case Logbook::SORT_CREATION: logEntryList().sortByColumn( LogEntryModel::CREATION, sort_order ); break;
        case Logbook::SORT_MODIFICATION: logEntryList().sortByColumn( LogEntryModel::MODIFICATION , sort_order); break;
        case Logbook::SORT_AUTHOR: logEntryList().sortByColumn( LogEntryModel::AUTHOR, sort_order ); break;
        default: break;
    }

    Debug::Throw( "MainWindow::setLogbook - lists sorted.\n" );

    // update attachment frame
    resetAttachmentWindow();
    Debug::Throw( "MainWindow::setLogbook - attachment frame reset.\n" );

    // retrieve last modified entry
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    BASE::KeySet<LogEntry>::const_iterator iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    logEntryList().setFocus();

    Debug::Throw( "MainWindow::setLogbook - entry selected.\n" );

    // see if logbook has parent file
    if( logbook()->parentFile().size() ) {
        QString buffer;
        QTextStream(&buffer ) << "Warning: this logbook should be oppened via \"" << logbook()->parentFile() << "\" only.";
        InformationDialog( this, buffer ).exec();
    }

    // store logbook directory for next open, save comment
    workingDirectory_ = File( logbook()->file() ).path();
    statusBar().label().setText( "" );
    statusBar().showLabel();

    // register logbook to fileCheck
    fileCheck().registerLogbook( logbook() );

    emit ready();

    // check errors
    XmlError::List errors( logbook()->xmlErrors() );
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
    if( !logbook()->file().isEmpty() )
    { Singleton::get().application<Application>()->recentFiles().add( logbook()->file().expand() ); }

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
        !logbook()->file().isEmpty() &&
        logbook()->needsBackup() )
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
    _keywordModel().clear();
    _logEntryModel().clear();

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
{ logEntryList().clearSelection(); }

//_______________________________________________
void MainWindow::selectEntry( LogEntry* entry )
{
    Debug::Throw("MainWindow::selectEntry.\n" );

    if( !entry ) return;

    // select entry keyword
    QModelIndex index = _keywordModel().index( entry->keyword() );
    keywordList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    keywordList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    keywordList().scrollTo( index );

    index = _logEntryModel().index( entry );
    logEntryList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    logEntryList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    logEntryList().scrollTo( index );
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
        _keywordModel().add( entry->keyword() );
        QModelIndex index = _keywordModel().index( entry->keyword() );
        keywordList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }

    // umdate logEntry model
    _logEntryModel().add( entry );

    // select
    if( updateSelection )
    {
        QModelIndex index( _logEntryModel().index( entry ) );
        logEntryList().selectionModel()->select( index, QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        logEntryList().selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current|QItemSelectionModel::Rows );
    }

}


//_______________________________________________
void MainWindow::deleteEntry( LogEntry* entry, const bool& save )
{
    Debug::Throw( "MainWindow::deleteEntry.\n" );

    assert( entry );

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
    _logEntryModel().remove( entry );

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
    if( save && !logbook()->file().isEmpty() )
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
    QModelIndex index( _logEntryModel().index( entry ) );
    if( !( index.isValid() && index.row() > 0 ) ) return 0;

    QModelIndex previousIndex( _logEntryModel().index( index.row()-1, index.column() ) );
    if( updateSelection )
    {
        logEntryList().selectionModel()->select( previousIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        logEntryList().selectionModel()->setCurrentIndex( previousIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }

    return _logEntryModel().get( previousIndex );

}

//_______________________________________________
LogEntry* MainWindow::nextEntry( LogEntry* entry, const bool& updateSelection )
{

    Debug::Throw( "MainWindow::nextEntry.\n" );
    QModelIndex index( _logEntryModel().index( entry ) );
    if( !( index.isValid() && index.row()+1 < _logEntryModel().rowCount() ) ) return 0;

    QModelIndex nextIndex( _logEntryModel().index( index.row()+1, index.column() ) );
    if( updateSelection )
    {
        logEntryList().selectionModel()->select( nextIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        logEntryList().selectionModel()->setCurrentIndex( nextIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }

    return _logEntryModel().get( nextIndex );

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
    BASE::KeySet<Attachment> attachments( logbook()->attachments() );
    attachmentWindow.frame().add( attachments.toList() );

    return;

}

//_______________________________________________
Keyword MainWindow::currentKeyword( void ) const
{
    Debug::Throw( "MainWindow::currentKeyword.\n" );
    QModelIndex index( keywordList().selectionModel()->currentIndex() );
    return index.isValid() ? _keywordModel().get( index ) : Keyword();
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
    if( logbook()->file().isEmpty() ) {
        _saveAs();
        return;
    }

    // check logbook filename is writable
    File fullname = File( logbook()->file() ).expand();
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

    logbook()->truncateRecentEntriesList( maxRecentEntries_ );

    bool written( logbook()->write() );
    Singleton::get().application<Application>()->idle();
    _setEnabled( true );

    if( written ) { setModified( false );}

    // update StateFrame
    statusBar().label().setText( "" );
    statusBar().showLabel();

    // add new file to openPreviousMenu
    if( !logbook()->file().isEmpty() ) Singleton::get().application<Application>()->recentFiles().add( logbook()->file().expand() );

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
    QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

    // check is selection is a valid color when Color search is requested.
    bool colorValid = ( mode&SearchPanel::COLOR && ( selection.compare( ColorMenu::NONE, Qt::CaseInsensitive ) == 0 || QColor( selection ).isValid() ) );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> turnedOffEntries;
    foreach( LogEntry* entry, logbook()->entries() )
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

        statusBar().label().setText( "no match found. Find canceled" );

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

    statusBar().label().setText( buffer );

    return;
}

//_______________________________________________
void MainWindow::showAllEntries( void )
{
    Debug::Throw( "MainWindow::showAllEntries.\n" );

    // keep track of the current selected entry
    QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

    // set all logbook entries to find_visible
    foreach( LogEntry* entry, logbook()->entries() )
    { entry->setFindSelected( true ); }

    // reinitialize logEntry list
    _resetKeywordList();
    _resetLogEntryList();

    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( _logEntryModel().rowCount() ) selectEntry( _logEntryModel().get( _logEntryModel().index( _logEntryModel().rowCount()-1, 0 ) ) );

    statusBar().label().setText( "" );
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
        XmlOptions::get().set<int>( "KEYWORD_LIST_WIDTH", keywordList().width() );

    } else if(event->timerId() == editionTimer_.timerId() ) {

        editionTimer_.stop();

        // check if current index is valid and was 'double-clicked'
        QModelIndex index( logEntryList().currentIndex() );
        if( index.isValid() && index == _logEntryModel().editionIndex() )
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

    logbookStatisticsAction_ = new QAction( IconEngine::get( ICONS::INFO ), "Logbook Statistics...", this );
    logbookStatisticsAction_->setToolTip( "View logbook statistics" );
    connect( logbookStatisticsAction_, SIGNAL( triggered() ), SLOT( _viewLogbookStatistics() ) );

    logbookInformationsAction_ = new QAction( IconEngine::get( ICONS::INFO ), "Logbook Properties...", this );
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
    logEntryList().initializeAnimation();

    // clear list of entries
    _logEntryModel().clear();

    if( logbook_ )
    {

        LogEntryModel::List modelEntries;
        foreach( LogEntry* entry, logbook()->entries() )
        {
            if( (!treeModeAction().isChecked() && entry->isFindSelected()) || entry->isSelected() )
            { modelEntries << entry; }
        }

        _logEntryModel().add( modelEntries );

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
    logEntryList().startAnimation();

    return;

}

//_______________________________________________
void MainWindow::_resetKeywordList( void )
{

    Debug::Throw( "MainWindow::_resetKeywordList.\n" );
    assert( logbook() );

    // animation
    keywordList().initializeAnimation();

    // retrieve new list of keywords (from logbook)
    KeywordModel::List newKeywords;
    foreach( LogEntry* entry, logbook()->entries() )
    {
        if( entry->isFindSelected() )
        {
            Keyword keyword( entry->keyword() );
            while( keyword != Keyword::NO_KEYWORD )
            {
                if( !newKeywords.contains( keyword ) ) newKeywords << keyword;
                keyword = keyword.parent();
            }

        }
    }

    _keywordModel().set( newKeywords );

    // animation
    keywordList().startAnimation();

}

//_______________________________________________
void MainWindow::_loadColors( void )
{

    Debug::Throw( "MainWindow::_loadColors.\n" );

    if( !logbook_ ) return;

    //! retrieve all entries
    foreach( LogEntry* entry, logbook()->entries() )
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

    if( logbook_ && !logbook()->file().isEmpty() )
    {

        statusBar().label().setText( "performing autoSave" );

        // retrieve non read only editors; perform save
        BASE::KeySet<EditionWindow> windows( this );
        foreach( EditionWindow* window, windows )
        {
            if( window->isReadOnly() || window->isClosed() ) continue;
            window->saveAction().trigger();
        }

        save();

    } else {

        statusBar().label().setText( "no logbook filename. <Autosave> skipped" );

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

        logbook()->setModifiedRecursive( false );
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
    if( logbook_ && logbook()->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // new logbook
    NewLogbookDialog dialog( this );
    dialog.setTitle( Logbook::LOGBOOK_NO_TITLE );
    dialog.setAuthor( XmlOptions::get().raw( "USER" ) );
    File file = File( "log.xml" ).addPath( workingDirectory() );
    dialog.setFile( file );
    dialog.setAttachmentDirectory( workingDirectory() );
    if( !dialog.centerOnParent().exec() ) return;

    // create a new logbook, with no file
    setLogbook( dialog.file() );
    assert( logbook_ );

    logbook()->setTitle( dialog.title() );
    logbook()->setAuthor( dialog.author() );
    logbook()->setComments( dialog.comments() );

    // attachment directory
    File directory( dialog.attachmentDirectory() );

    // check if fulldir is not a non directory existsing file
    if( directory.exists() && !directory.isDirectory() )
    {

        QString buffer;
        QTextStream(&buffer ) << "File \"" << directory << "\" is not a directory.";
        InformationDialog( this, buffer ).exec();

    } else logbook()->setDirectory( directory );

    // add new file to openPreviousMenu
    if( !logbook()->file().isEmpty() )
    {
        Singleton::get().application<Application>()->recentFiles().add( logbook()->file().expand() );
        logbook()->setModified( true );
        save();
    }

}

//_______________________________________________
void MainWindow::setModified( bool value )
{

    Debug::Throw() << "MainWindow::setModified - " << value << endl;

    QString buffer;
    QTextStream what( &buffer );
    if( !logbook() || logbook()->file().isEmpty() ) what << "Elogbook";
    else what << logbook()->file().localName() << " - " << logbook()->file().path();
    if( value ) what << " (modified)";
    setWindowTitle( buffer );

}

//_______________________________________________
void MainWindow::open( FileRecord record )
{

    Debug::Throw( "MainWindow::open.\n" );

    // check if current logbook needs save
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), confirmEntries_ ) == AskForSaveDialog::CANCEL ) return;
    if( logbook_ && logbook()->modified()  && askForSave() == AskForSaveDialog::CANCEL ) return;

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
    if( defaultFile.isEmpty() ) defaultFile = logbook()->file();
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
    logbook()->setFile( fullname );
    logbook()->setModifiedRecursive( true );
    save();

    // update current file in menu
    menu().recentFilesMenu().setCurrentFile( fullname );

    /*
    force logbook state to unmodified since
    some children state may not have been reset properly
    */
    logbook()->setModifiedRecursive( false );

    // add new file to openPreviousMenu
    if( !logbook()->file().isEmpty() )
    { Singleton::get().application<Application>()->recentFiles().add( logbook()->file().expand() ); }

    // redo file check registration
    if( registerLogbook )
    {
        fileCheck().clear();
        fileCheck().registerLogbook( logbook() );
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
    logbook()->setModifiedRecursive( true );
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

    QString filename( logbook()->backupFilename( ) );
    if( filename.isEmpty() ) {
        InformationDialog( this, "no valid filename. Use <Save As> first." ).exec();
        return;
    }

    // store last backup time and update
    TimeStamp lastBackup( logbook()->backup() );

    // stores current logbook filename
    QString currentFilename( logbook()->file() );

    // save logbook as backup
    bool saved( _saveAs( filename, false ) );

    // remove the "backup" filename from the openPrevious list
    // to avoid confusion
    Singleton::get().application<Application>()->recentFiles().remove( File(filename).expand() );

    // restore initial filename
    logbook()->setFile( currentFilename );

    if( saved ) {

        logbook()->addBackup( filename );
        logbook()->setModified( true );
        setModified( true );

        // Save logbook if needed (to make sure the backup stamp is updated)
        if( !logbook()->file().isEmpty() ) save();
    }

}

//_______________________________________________
void MainWindow::_manageBackups( void )
{
    Debug::Throw( "MainWindow::_manageBackups.\n");

    BackupManagerDialog dialog( this );
    Key::associate( &dialog.managerWidget(), logbook() );
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
    QTextStream( &buffer ) << "Discard changes to " << logbook()->file().localName() << " ?";
    if( ( _hasModifiedEntries() || logbook()->modified() ) && !QuestionDialog( this, buffer ).exec() )
    { return; }

    // reinit MainWindow
    Singleton::get().application<Application>()->busy();
    setLogbook( logbook()->file() );
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
    if( logbook()->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create printer
    QPrinter printer( QPrinter::HighResolution );

    // generate document name
    QString buffer;
    QTextStream( &buffer )  << "elogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid();
    printer.setDocName( buffer );

    // create helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook() );
    helper.setEntries(
        BASE::KeySet<LogEntry>( logbook()->entries() ).toList(),
        _logEntryModel().get(),
        _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );

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
    { Singleton::get().application<Application>()->scratchFileMonitor().add( printer.outputFileName() ); }

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
    statusBar().label().setText( "" );
    statusBar().showLabel();

    return;

}

//___________________________________________________________
void MainWindow::_printPreview( void )
{
    Debug::Throw( "MainWindow::_printPreview.\n" );

    // save EditionWindows
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::CANCEL ) return;

    // save current logbook
    if( logbook()->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook() );
    helper.setEntries(
        BASE::KeySet<LogEntry>( logbook()->entries() ).toList(),
        _logEntryModel().get(),
        _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );

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
    statusBar().label().setText( "" );
    statusBar().showLabel();

}

//___________________________________________________________
void MainWindow::_toHtml( void )
{
    Debug::Throw( "MainWindow::_toHtml.\n" );

    // save EditionWindows
    if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), true ) == AskForSaveDialog::CANCEL ) return;

    // save current logbook
    if( logbook()->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

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
    Singleton::get().application<Application>()->scratchFileMonitor().add( file );

    // write options
    logbookOptionWidget->write();
    logEntrySelectionWidget->write();
    logEntryOptionWidget->write();

    // create print helper
    LogbookHtmlHelper helper( this );
    helper.setLogbook( logbook() );

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
    if( logbook()->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create file dialog
    File remoteFile( FileDialog(this).getFile() );
    if( remoteFile.isNull() ) return;

    // debug
    Debug::Throw() << "MainWindow::_synchronize - number of local files: " << logbook()->children().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - number of local entries: " << logbook()->entries().size() << endl;

    // set busy flag
    Singleton::get().application<Application>()->busy();
    statusBar().label().setText( "reading remote logbook ... " );

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
    QHash<LogEntry*,LogEntry*> duplicates( logbook()->synchronize( remoteLogbook ) );
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
    _keywordSelectionChanged( keywordList().selectionModel()->currentIndex() );

    // reset attachment window
    resetAttachmentWindow();

    // retrieve last modified entry
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    BASE::KeySet<LogEntry>::const_iterator iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    logEntryList().setFocus();

    // write local logbook
    if( !logbook()->file().isEmpty() ) save();

    // synchronize remove with local
    Debug::Throw() << "MainWindow::_synchronize - updating remote from local" << endl;
    unsigned int nDuplicated = remoteLogbook.synchronize( *logbook() ).size();
    Debug::Throw() << "MainWindow::_synchronize - number of duplicated entries: " << nDuplicated << endl;

    // save remote logbook
    statusBar().label().setText( "saving remote logbook ... " );
    remoteLogbook.write();

    // idle
    Singleton::get().application<Application>()->idle();
    statusBar().label().setText( "" );

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
    Logbook::Backup::List backups( logbook()->backupFiles() );
    Logbook::Backup::List::iterator iter = std::find( backups.begin(), backups.end(), backup );
    if( iter != backups.end() )
    {
        backups.erase( iter );
        logbook()->setBackupFiles( backups );
        if( !logbook()->file().isEmpty() )
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
    File oldName( logbook()->file() );

    // store old backups
    Logbook::Backup::List backups( logbook()->backupFiles() );

    // store associated backup manager Widget
    BASE::KeySet<BackupManagerWidget> widgets( logbook() );

    // replace logbook with backup
    setLogbook( backup.file() );

    // remove the "backup" filename from the openPrevious list
    // to avoid confusion
    Singleton::get().application<Application>()->recentFiles().remove( backup.file().expand() );

    // change filename
    logbook()->setFile( oldName );

    // reassign backups
    logbook()->setBackupFiles( backups );

    // re-associate
    foreach( BackupManagerWidget* widget, widgets )
    { BASE::Key::associate( widget, logbook() ); }

    // and save
    if( !logbook()->file().isEmpty() )
    {
        save();
        Singleton::get().application<Application>()->recentFiles().add( logbook()->file().expand() );
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
    if( logbook()->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // debug
    Debug::Throw() << "MainWindow::_mergeBackup - number of local files: " << logbook()->children().size() << endl;
    Debug::Throw() << "MainWindow::_mergeBackup - number of local entries: " << logbook()->entries().size() << endl;

    // set busy flag
    Singleton::get().application<Application>()->busy();
    statusBar().label().setText( "reading remote logbook ... " );

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
    QHash<LogEntry*,LogEntry*> duplicates( logbook()->synchronize( backupLogbook ) );
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
    _keywordSelectionChanged( keywordList().selectionModel()->currentIndex() );

    // reset attachments
    resetAttachmentWindow();

    // retrieve last modified entry
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    BASE::KeySet<LogEntry>::const_iterator iter = std::min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    logEntryList().setFocus();

    // write local logbook
    if( !logbook()->file().isEmpty() ) save();

    // idle
    Singleton::get().application<Application>()->idle();
    statusBar().label().setText( "" );

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
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
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
    for( QList<LogEntry*>::iterator iter = entryList.begin(); iter != entryList.end(); ++iter )
    {
        Logbook *logbook( MainWindow::logbook()->latestChild() );
        Key::associate( *iter, logbook );
        logbook->setModified( true );
    }

    // remove empty logbooks
    logbook()->removeEmptyChildren();

    // redo fileChecker registration
    fileCheck().clear();
    fileCheck().registerLogbook( logbook() );

    // save
    logbook()->setModified( true );
    if( !logbook()->file().isEmpty() ) save();

}

//_______________________________________________
void MainWindow::_showDuplicatedEntries( void )
{
    Debug::Throw( "MainWindow::_showDuplicatedEntries.\n" );

    // keep track of the last visible entry
    LogEntry *lastVisibleEntry( 0 );

    // keep track of current index
    QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

    // keep track of found entries
    int found( 0 );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
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

    modified |= logbook()->setTitle( dialog.title() );
    modified |= logbook()->setAuthor( dialog.author() );
    modified |= logbook()->setComments( dialog.comments() );

    // retrieve logbook directory
    File directory = dialog.AttachmentDirectory();

    // check if fulldir is not a non directory existsing file
    if( directory.exists() &&  !directory.isDirectory() )
    {

        QString buffer;
        QTextStream(&buffer ) << "File \"" << directory << "\" is not a directory.";
        InformationDialog( this, buffer ).exec();

    } else modified |= logbook()->setDirectory( directory );


    // save Logbook, if needed
    if( modified ) logbook()->setModified( true );
    if( !logbook()->file().isEmpty() ) save();

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
    EditionWindow *editWindow( 0 );
    BASE::KeySet<EditionWindow> frames( this );
    BASE::KeySetIterator<EditionWindow> iterator( frames );
    iterator.toBack();
    while( iterator.hasPrevious() )
    {
        EditionWindow* current( iterator.previous() );

        // skip closed editors
        if( !current->isClosed() ) continue;
        editWindow = current;
        editWindow->setIsClosed( false );
        editWindow->setReadOnly( false );
        break;
    }

    if( !editWindow )
    {
        // create new EditionWindow
        editWindow = new EditionWindow( 0, false );
        editWindow->setColorMenu( colorMenu_ );
        Key::associate( this, editWindow );
    }

    // force editWindow show keyword flag
    editWindow->setForceShowKeyword( !treeModeAction().isChecked() );

    // call NewEntry for the selected frame
    editWindow->newEntryAction().trigger();

    // show frame
    editWindow->centerOnWidget( this );
    editWindow->show();

}

//____________________________________________
void MainWindow::_editEntries( void )
{
    Debug::Throw( "MainWindow::_EditEntries .\n" );

    // retrieve selected items; make sure they do not include the navigator
    LogEntryModel::List selection( _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );
    if( selection.empty() ) {
        InformationDialog( this, "no entry selected. Request canceled.").exec();
        return;
    }

    // retrieve associated entry
    for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    { _displayEntry( *iter ); }

    return;

}

//____________________________________________
void MainWindow::_deleteEntries( void )
{
    Debug::Throw( "MainWindow::_DeleteEntries .\n" );

    // retrieve selected rows;
    QModelIndexList selectedIndexes( logEntryList().selectionModel()->selectedRows() );

    // convert into LogEntry list
    LogEntryModel::List selection;
    bool has_edited_index( false );
    for( QModelIndexList::iterator iter = selectedIndexes.begin(); iter != selectedIndexes.end(); ++iter )
    {
        // check if index is not being edited
        if( _logEntryModel().editionEnabled() && *iter ==  _logEntryModel().editionIndex() )
        {
            has_edited_index = true;
            InformationDialog( this, "Cannot delete entry that is being edited." ).exec();
        } else selection.push_back( _logEntryModel().get( *iter ) );
    }

    // check selection size
    if( selection.empty() && !has_edited_index )
    {
        InformationDialog( this, "no entry selected. Request canceled.").exec();
        return;
    }

    // ask confirmation
    QString buffer;
    QTextStream( &buffer ) << "Delete selected entr" << ( selection.size() == 1 ? "y":"ies" );
    if( !QuestionDialog( this, buffer ).exec() ) return;

    // retrieve associated entry
    for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    { deleteEntry( *iter, false ); }

    // Save logbook if needed
    if( !logbook()->file().isEmpty() ) save();

    return;

}

//_______________________________________________
void MainWindow::_displayEntry( LogEntry* entry )
{

    Debug::Throw( "MainWindow::_displayEntry.\n" );

    // retrieve associated EditionWindows, check if one matches the selected entry
    EditionWindow *editWindow( 0 );
    BASE::KeySet<EditionWindow> windows( this );
    foreach( EditionWindow* window, windows )
    {

        // skip closed editors
        if( window->isClosed() ) continue;

        // check if EditionWindow is editable and match editor
        if( !window->isReadOnly() && window->entry() == entry )
        {
            editWindow = window;
            editWindow->uniconifyAction().trigger();
            return;
        }

    }

    // if no editWindow is found, try re-used a closed editor
    if( !editWindow )
    {

        // the order is reversed to start from latest
        BASE::KeySetIterator<EditionWindow> iterator( windows );
        iterator.toBack();
        while( iterator.hasPrevious() )
        {
            EditionWindow* current( iterator.previous() );

            // skip closed editors
            if( !current->isClosed() ) continue;

            editWindow = current;
            editWindow->setIsClosed( false );
            editWindow->setReadOnly( false );

            // also clear modification state
            editWindow->setModified( false );

            // need to display entry before deleting sub views.
            editWindow->displayEntry( entry );

            // also kill all frames but one
            BASE::KeySet< EditionWindow::LocalTextEditor > editors( editWindow );
            if( editors.size() > 1 )
            {

                BASE::KeySet<EditionWindow::LocalTextEditor>::iterator localIter( editors.begin() );
                ++localIter;
                for( ;localIter != editors.end(); ++localIter )
                { editWindow->closeEditor( **localIter ); }

                (**editors.begin()).setFocus();
                editWindow->setActiveEditor( **editors.begin() );

            }

            break;
        }

    }

    // if no editWindow is found create a new one
    if( !editWindow )
    {
        editWindow = new EditionWindow( 0, false );
        editWindow->setColorMenu( colorMenu_ );
        Key::associate( this, editWindow );
        editWindow->displayEntry( entry );
    }

    editWindow->setForceShowKeyword( !treeModeAction().isChecked() );
    editWindow->centerOnWidget( this );
    editWindow->show();

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
    if( logbook() && !logbook()->file().isEmpty() ) save();

}

//_______________________________________________
void MainWindow::_changeEntryColor( QColor color )
{
    Debug::Throw( "MainWindow::_changeEntryColor.\n" );

    // retrieve current selection
    LogEntryModel::List selection( _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no entry selected. Request canceled.").exec();
        return;
    }

    // retrieve associated entry
    for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    {

        // get associated entry
        LogEntry* entry( *iter );

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
    _logEntryModel().add( selection );

    // save Logbook
    if( !logbook()->file().isEmpty() ) save();

}

//____________________________________________
void MainWindow::_newKeyword( void )
{

    Debug::Throw( "MainWindow::_newKeyword.\n" );

    //! create dialog
    EditKeywordDialog dialog( this );
    dialog.setWindowTitle( "New Keyword - elogbook" );

    foreach( const Keyword& keyword, _keywordModel().children() )
    { dialog.add( keyword ); }

    dialog.setKeyword( currentKeyword() );

    // map dialog
    if( !dialog.centerOnParent().exec() ) return;

    // retrieve keyword from line_edit
    Keyword keyword( dialog.keyword() );
    if( keyword != Keyword::NO_KEYWORD )
    {
        _keywordModel().add( keyword );
        QModelIndex index( _keywordModel().index( keyword ) );
        keywordList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    }
}


//____________________________________________
void MainWindow::_deleteKeyword( void )
{
    Debug::Throw("MainWindow::_deleteKeyword.\n" );

    //! check that keywordlist has selected item
    QModelIndexList selectedIndexes( keywordList().selectionModel()->selectedRows() );
    if( selectedIndexes.empty() )
    {
        InformationDialog( this, "no keyword selected. Request canceled" ).exec();
        return;
    }

    // store corresponding list of keywords
    KeywordModel::List keywords;
    for( QModelIndexList::iterator iter = selectedIndexes.begin(); iter != selectedIndexes.end(); ++iter )
    { if( iter->isValid() ) keywords.push_back( _keywordModel().get( *iter ) ); }

    // retrieve associated entries
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
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
        for( KeywordModel::List::iterator iter = keywords.begin(); iter != keywords.end(); ++iter )
        { _renameKeyword( *iter, iter->parent(), false );  }

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
        QModelIndex index( _keywordModel().index( keyword.parent() ) );
        if( index.isValid() )
        {
            keywordList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            keywordList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            _resetLogEntryList();
            break;
        }

    }

    // Save logbook
    if( !logbook()->file().isEmpty() ) save();

    return;

}

//____________________________________________
void MainWindow::_renameKeyword( void )
{
    Debug::Throw("MainWindow::_renameKeyword.\n" );

    //! check that keywordlist has selected item
    QModelIndex current( keywordList().selectionModel()->currentIndex() );
    if( !current.isValid() )
    {
        InformationDialog( this, "no keyword selected. Request canceled" ).exec();
        return;
    }

    keywordList().edit( current );
    return;

}

//____________________________________________
void MainWindow::_renameKeyword( Keyword keyword, Keyword newKeyword, bool updateSelection )
{

    Debug::Throw("MainWindow::_renameKeyword.\n" );

    // check keywords are different
    if( keyword == newKeyword ) return;

    // get entries matching the oldKeyword, change the keyword
    foreach( LogEntry* entry, logbook()->entries() )
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
        QModelIndex parentIndex( _keywordModel().index( newKeyword.parent() ) );
        if( parentIndex.isValid() ) keywordList().setExpanded( parentIndex, true );

        // retrieve current index, and select
        QModelIndex index( _keywordModel().index( newKeyword ) );
        keywordList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList().scrollTo( index );
    }

    _resetLogEntryList();

    // Save logbook if needed
    if( !logbook()->file().isEmpty() ) save();

}

//____________________________________________
void MainWindow::_renameEntryKeyword( void )
{
    Debug::Throw("MainWindow::_renameEntryKeyword.\n" );

    // retrieve current selection
    LogEntryModel::List selection( _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );
    if( selection.empty() )
    {
        InformationDialog( this, "no entry selected. Request canceled.").exec();
        return;
    }

    //! check that current keyword make sense
    if( !keywordList().selectionModel()->currentIndex().isValid() )
    {
        InformationDialog( this, "no keyword selected. Request canceled" ).exec();
        return;
    }

    // get current selected keyword
    Keyword keyword( _keywordModel().get( keywordList().selectionModel()->currentIndex() ) );

    //! create dialog
    EditKeywordDialog dialog( this );
    dialog.setWindowTitle( "Edit Keyword - elogbook" );

    foreach( const Keyword& keyword, _keywordModel().children() )
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
    foreach( LogEntry* entry, _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) )
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
        QModelIndex parentIndex( _keywordModel().index( newKeyword.parent() ) );
        if( parentIndex.isValid() ) keywordList().setExpanded( parentIndex, true );

        // retrieve current index, and select
        QModelIndex index( _keywordModel().index( newKeyword ) );
        keywordList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        keywordList().scrollTo( index );
    }

    // update entry selection
    _resetLogEntryList();

    if( updateSelection )
    {
        // clear current selection
        logEntryList().clearSelection();

        // select all modified entries
        QModelIndex lastIndex;
        foreach( LogEntry* entry, entries )
        {
            QModelIndex index( _logEntryModel().index( entry ) );
            if( index.isValid() )
            {
                lastIndex = index;
                logEntryList().selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
            }
        }

        // update current index
        if( lastIndex.isValid() )
        {
            logEntryList().selectionModel()->setCurrentIndex( lastIndex,  QItemSelectionModel::Select|QItemSelectionModel::Rows );
            logEntryList().scrollTo( lastIndex );
        }
    }

    // Save logbook if needed
    if( !logbook()->file().isEmpty() ) save();

    return;

}

//_______________________________________________
void MainWindow::_keywordSelectionChanged( const QModelIndex& index )
{

    Debug::Throw( "MainWindow::_keywordSelectionChanged.\n" );
    if( !logbook_ ) return;
    if( !index.isValid() ) return;

    Keyword keyword( _keywordModel().get( index ) );
    Debug::Throw() << "MainWindow::_keywordSelectionChanged - keyword: " << keyword << endl;

    // keep track of the current selected entry
    QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> turnedOffEntries;
    foreach( LogEntry* entry, logbook()->entries() )
    { entry->setKeywordSelected( entry->keyword() == keyword ); }

    // reinitialize logEntry list
    _resetLogEntryList();

    // if EditionWindow current entry is visible, select it
    // otherwise, select last entry in model
    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( _logEntryModel().rowCount() )  { selectEntry( _logEntryModel().get( _logEntryModel().index( _logEntryModel().rowCount()-1, 0 ) ) ); }

    return;
}

//_____________________________________________
void MainWindow::_updateKeywordActions( void )
{
    Debug::Throw( "MainWindow::_updateKeywordActions.\n" );

    deleteKeywordAction().setEnabled( !keywordList().selectionModel()->selectedRows().empty() );
    editKeywordAction().setEnabled( keywordList().selectionModel()->currentIndex().isValid() );

    return;
}

//_____________________________________________
void MainWindow::_updateEntryActions( void )
{
    Debug::Throw( "MainWindow::_updateEntryActions.\n" );
    int selectedEntries( logEntryList().selectionModel()->selectedRows().size() );
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

        case LogEntryModel::COLOR: changed = logbook()->setSortMethod( Logbook::SORT_COLOR ); break;
        case LogEntryModel::TITLE: changed = logbook()->setSortMethod( Logbook::SORT_TITLE ); break;
        case LogEntryModel::CREATION: changed = logbook()->setSortMethod( Logbook::SORT_CREATION ); break;
        case LogEntryModel::MODIFICATION: changed = logbook()->setSortMethod( Logbook::SORT_MODIFICATION ); break;
        case LogEntryModel::AUTHOR: changed = logbook()->setSortMethod( Logbook::SORT_AUTHOR ); break;
        default: return;

    }

    // Save logbook if needed
    changed |= logbook()->setSortOrder( int( order ) );
    if( changed && !logbook()->file().isEmpty() ) save();

}


//____________________________________________________________
void MainWindow::_entryItemActivated( const QModelIndex& index )
{
    // stop edition timer
    _logEntryModel().setEditionIndex( QModelIndex() );
    editionTimer_.stop();
    if( index.isValid() ) _displayEntry( _logEntryModel().get( index ) );
}

//____________________________________________________________
void MainWindow::_entryItemClicked( const QModelIndex& index )
{

    // do nothing if index do not correspond to an entry title
    if( !index.isValid() ) return;

    // do nothing if index is not already selected
    if( !logEntryList().selectionModel()->isSelected( index ) ) return;

    if( !( index.column() == LogEntryModel::TITLE ||  index.column() == LogEntryModel::KEYWORD ) )
    { return; }

    // compare to model edition index
    if( index == _logEntryModel().editionIndex() ) editionTimer_.start( editionDelay_, this );
    else _logEntryModel().setEditionIndex( index );

}

//_______________________________________________
void MainWindow::_entryDataChanged( const QModelIndex& index )
{
    Debug::Throw( "MainWindow::_entryDataChanged.\n" );

    if( !index.isValid() ) return;
    LogEntry* entry( _logEntryModel().get( index ) );

    unsigned int mask(0);
    if( index.column() == LogEntryModel::TITLE ) mask = TITLE_MASK;
    else if( index.column() == LogEntryModel::KEYWORD ) mask = KEYWORD_MASK;

    // update associated EditionWindows
    _updateEntryFrames( entry, mask );

    // set logbooks as modified
    BASE::KeySet<Logbook> logbooks( entry );
    foreach( Logbook* logbook, logbooks ) logbook->setModified( true );

    // save Logbook
    if( logbook() && !logbook()->file().isEmpty() ) save();

}

//________________________________________
void MainWindow::_startEntryEdition( void )
{

    Debug::Throw( "MainWindow::_startEntryEdition\n" );

    // make sure title column is visible
    // if( logEntryList().isColumnHidden( LogEntryModel::TITLE ) ) return;

    // get current index and check validity
    QModelIndex index( logEntryList().currentIndex() );
    if( !index.isValid() ) return;

    // make sure 'title' index is selected
    // index = _logEntryModel().index( index.row(), LogEntryModel::TITLE );

    // enable model edition
    _logEntryModel().setEditionIndex( index );
    _logEntryModel().setEditionEnabled( true );

    // edit item
    logEntryList().edit( index );

}

//________________________________________
void MainWindow::_storeSelectedEntries( void )
{
    // clear
    _logEntryModel().clearSelectedIndexes();

    // retrieve selected indexes in list
    QModelIndexList selectedIndexes( logEntryList().selectionModel()->selectedRows() );
    for( QModelIndexList::iterator iter = selectedIndexes.begin(); iter != selectedIndexes.end(); ++iter )
    {
        // check column
        if( !iter->column() == 0 ) continue;
        _logEntryModel().setIndexSelected( *iter, true );
    }

}

//________________________________________
void MainWindow::_restoreSelectedEntries( void )
{

    // retrieve indexes
    QModelIndexList selectedIndexes( _logEntryModel().selectedIndexes() );
    if( selectedIndexes.empty() ) logEntryList().selectionModel()->clear();
    else {

        logEntryList().selectionModel()->select( selectedIndexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        foreach( const QModelIndex& index, selectedIndexes )
        { logEntryList().selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    }

    return;
}

//________________________________________
void MainWindow::_storeSelectedKeywords( void )
{
    // clear
    _keywordModel().clearSelectedIndexes();

    // retrieve selected indexes in list
    QModelIndexList selectedIndexes( keywordList().selectionModel()->selectedRows() );
    for( QModelIndexList::iterator iter = selectedIndexes.begin(); iter != selectedIndexes.end(); ++iter )
    {
        // check column
        if( !iter->column() == 0 ) continue;
        _keywordModel().setIndexSelected( *iter, true );
    }

}

//________________________________________
void MainWindow::_restoreSelectedKeywords( void )
{

    // retrieve indexes
    QModelIndexList selectedIndexes( _keywordModel().selectedIndexes() );
    if( selectedIndexes.empty() ) keywordList().selectionModel()->clear();
    else {

        keywordList().selectionModel()->select( selectedIndexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        foreach( const QModelIndex& index, selectedIndexes )
        { keywordList().selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    }

    return;
}

//________________________________________
void MainWindow::_storeExpandedKeywords( void )
{

    Debug::Throw( "MainWindow::_storeExpandedKeywords.\n" );
    // clear
    _keywordModel().clearExpandedIndexes();

    // retrieve expanded indexes in list
    QModelIndexList indexes( _keywordModel().indexes() );
    for( QModelIndexList::iterator iter = indexes.begin(); iter != indexes.end(); ++iter )
    { if( keywordList().isExpanded( *iter ) ) _keywordModel().setIndexExpanded( *iter, true ); }

}

//________________________________________
void MainWindow::_restoreExpandedKeywords( void )
{

    Debug::Throw( "MainWindow::_restoreExpandedKeywords.\n" );

    QModelIndexList expandedIndexes( _keywordModel().expandedIndexes() );
    keywordList().collapseAll();
    foreach( const QModelIndex& index, expandedIndexes )
    { keywordList().setExpanded( index, true ); }

    return;
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
    logEntryList().setDragEnabled( value );

    // get current entry
    LogEntry* currentEntry( 0 );
    QModelIndex index( logEntryList().selectionModel()->currentIndex() );
    if( index.isValid() ) currentEntry = _logEntryModel().get( index );

    // update keyword list
    if( value ) _resetKeywordList();

    // update log entry list
    _resetLogEntryList();

    // update log entry mask
    unsigned int mask( logEntryList().mask() );
    if( value ) mask &= ~(1<<LogEntryModel::KEYWORD);
    else mask |= 1<<LogEntryModel::KEYWORD;
    logEntryList().setMask( mask );
    logEntryList().saveMask();

    logEntryList().resizeColumns();

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
            QModelIndex index( _keywordModel().index( currentEntry->keyword() ) );
            keywordList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            keywordList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
            keywordList().scrollTo( index );
        }

        // select entry
        QModelIndex index( _logEntryModel().index( currentEntry ) );
        logEntryList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        logEntryList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
        logEntryList().scrollTo( index );

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
    Options::List color_list( XmlOptions::get().specialOptions( "COLOR" ) );
    for( Options::List::iterator iter = color_list.begin(); iter != color_list.end(); ++iter )
    { colorMenu_->add( iter->raw() ); }

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
        { BASE::KeySet<LogEntry>( logbook()->entries() ).toList(); }

        case LogEntryPrintSelectionWidget::VISIBLE_ENTRIES:
        return _logEntryModel().get();

        case LogEntryPrintSelectionWidget::SELECTED_ENTRIES:
        return _logEntryModel().get( logEntryList().selectionModel()->selectedRows() );

    }

}
