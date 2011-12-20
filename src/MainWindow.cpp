
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
#include "IconEngine.h"
#include "Icons.h"
#include "InformationDialog.h"
#include "LineEditor.h"
#include "Logbook.h"
#include "LogbookInformationDialog.h"
#include "LogbookModifiedDialog.h"
#include "LogbookStatisticsDialog.h"
#include "LogbookPrintOptionWidget.h"
#include "LogbookPrintHelper.h"
#include "LogEntryPrintOptionWidget.h"
#include "LogEntryPrintSelectionWidget.h"
#include "Menu.h"
#include "NewLogbookDialog.h"
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
    setWindowTitle( Application::MAIN_TITLE );

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

    // default width from options, if found
    if( XmlOptions::get().find( "KEYWORD_LIST_WIDTH" ) )
    { keywordList_->setDefaultWidth( XmlOptions::get().get<int>( "KEYWORD_LIST_WIDTH" ) ); }

    // the use of a custom delegate unfortunately disable the
    // nice selection appearance of the oxygen style.
    keywordList().setItemDelegate( new TextEditionDelegate( this ) );

    // update LogEntryList when keyword selection change
    connect( keywordList().selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( KeywordSelectionChanged( const QModelIndex& ) ) );
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
    add the deleteKeyword_action to the keyword list,
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
    button->setText( "&Entry Color" );
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
    add the deleteEntry_action to the list,
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
    BASE::KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
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
    BASE::KeySet<EditionWindow> frames( this );
    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); ++iter )
    {
        (*iter)->setIsClosed( true );
        (*iter)->hide();
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
    for( BASE::KeySet<Attachment>::iterator iter = attachments.begin(); iter != attachments.end(); ++iter )
    {

        // retrieve/delete associated attachment frames
        BASE::KeySet<AttachmentFrame> frames( *iter );
        for( BASE::KeySet<AttachmentFrame>::iterator frameIter = frames.begin(); frameIter != frames.end(); ++frameIter )
        { (*frameIter)->remove( **iter ); }

        // delete attachment
        delete (*iter);

    };

    // remove from model
    _logEntryModel().remove( entry );

    /*
    hide associated EditionWindows
    they will get deleted next time
    MainWindow::_displayEntry() is called
    */
    BASE::KeySet<EditionWindow> frames( entry );
    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); ++iter )
    {
        (*iter)->setIsClosed( true );
        (*iter)->hide();
    }

    // set logbooks as modified
    BASE::KeySet<Logbook> logbooks( entry );
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); ++iter )
        (*iter)->setModified( true );

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

    BASE::KeySet<EditionWindow> frames( entry );
    if( _checkModifiedEntries( frames, true ) == AskForSaveDialog::CANCEL ) return false;

    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); ++iter )
    { (*iter)->setReadOnly( true ); }

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
    AttachmentWindow &attachment_window( Singleton::get().application<Application>()->attachmentWindow() );
    attachment_window.frame().clear();

    // check current logbook
    if( !logbook_ ) return;

    // retrieve logbook attachments, adds to AttachmentWindow
    BASE::KeySet<Attachment> attachments( logbook()->attachments() );
    attachment_window.frame().add( AttachmentModel::List( attachments.begin(), attachments.end() ) );

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

    if( written ) { setWindowTitle( Application::MAIN_TITLE );}

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
    LogEntry *last_visibleEntry( 0 );

    // keep track of the current selected entry
    QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

    // check is selection is a valid color when Color search is requested.
    bool color_valid = ( mode&SearchPanel::COLOR && ( selection.compare( ColorMenu::NONE, Qt::CaseInsensitive ) == 0 || QColor( selection ).isValid() ) );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    BASE::KeySet<LogEntry> turned_offEntries;
    for( BASE::KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
    {

        // retrieve entry
        LogEntry* entry( *it );
        total++;

        // if entry is already hidder, skipp
        if( !entry->isFindSelected() ) continue;

        // check entry
        bool accept( false );
        if( (mode&SearchPanel::TITLE ) && entry->matchTitle( selection ) ) accept = true;
        if( (mode&SearchPanel::KEYWORD ) && entry->matchKeyword( selection ) ) accept = true;
        if( (mode&SearchPanel::TEXT ) && entry->matchText( selection ) ) accept = true;
        if( (mode&SearchPanel::ATTACHMENT ) && entry->matchAttachment( selection ) ) accept = true;
        if( color_valid && entry->matchColor( selection ) ) accept = true;

        if( accept )
        {

            found++;
            if( entry->isKeywordSelected() || !(last_visibleEntry && last_visibleEntry->isKeywordSelected()) )
            { last_visibleEntry = entry; }

        } else {

            turned_offEntries.insert( entry );
            entry->setFindSelected( false );

        }

    }

    // if no entries are found, restore the disabled entries and abort
    if( !found )
    {

        statusBar().label().setText( "no match found. Find canceled" );

        // reset flag for the turned off entries to true
        for( BASE::KeySet<LogEntry>::iterator it=turned_offEntries.begin(); it!= turned_offEntries.end(); it++ )
            (*it)->setFindSelected( true );

        return;

    }

    // reinitialize logEntry list
    _resetKeywordList();
    _resetLogEntryList();

    // if EditionWindow current entry is visible, select it;
    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( last_visibleEntry ) selectEntry( last_visibleEntry );

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
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    for( BASE::KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
    { (*it)->setFindSelected( true ); }

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

    addAction( editKeywordAction_ = new QAction( IconEngine::get( ICONS::RENAME ), "Rename Keyword", this ) );
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
    deleteKeywordAction_->setShortcut( Qt::Key_Delete );
    deleteKeywordAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( deleteKeywordAction_, SIGNAL( triggered() ), SLOT( _deleteKeyword() ) );

    findEntriesAction_ = new QAction( IconEngine::get( ICONS::FIND ), "Find", this );
    findEntriesAction_->setShortcut( Qt::CTRL+Qt::Key_F );
    findEntriesAction_->setToolTip( "Find entries matching specific criteria" );
    connect( findEntriesAction_, SIGNAL( triggered() ), SLOT( _findEntries() ) );

    newEntryAction_ = new QAction( IconEngine::get( ICONS::NEW ), "New Entry", this );
    newEntryAction_->setToolTip( "Create a new entry" );
    newEntryAction_->setShortcut( Qt::CTRL+Qt::Key_N );
    connect( newEntryAction_, SIGNAL( triggered() ), SLOT( _newEntry() ) );

    editEntryAction_ = new QAction( IconEngine::get( ICONS::EDIT ), "Edit Entries", this );
    editEntryAction_->setToolTip( "Edit selected entries" );
    connect( editEntryAction_, SIGNAL( triggered() ), SLOT( _editEntries() ) );

    editEntryTitleAction_ = new QAction( IconEngine::get( ICONS::RENAME ), "Rename Entry", this );
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
    deleteEntryAction_->setShortcut( Qt::Key_Delete );
    deleteEntryAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( deleteEntryAction_, SIGNAL( triggered() ), SLOT( _deleteEntries() ) );

    // color menu
    colorMenu_ = new ColorMenu( this );
    colorMenu_->setTitle( "Change entry color" );
    connect( colorMenu_, SIGNAL( selected( QColor ) ), SLOT( _changeEntryColor( QColor ) ) );

    entryColorAction_ = new QAction( IconEngine::get( ICONS::COLOR ), "Entry Color", this );
    entryColorAction_->setToolTip( "Change selected entries color" );
    entryColorAction_->setMenu( colorMenu_ );

    entryKeywordAction_ = new QAction( IconEngine::get( ICONS::EDIT ), "Change Keyword", this );
    entryKeywordAction_->setToolTip( "Edit selected entries keyword" );
    connect( entryKeywordAction_, SIGNAL( triggered() ), SLOT( _renameEntryKeyword() ) );

    newLogbookAction_ = new QAction( IconEngine::get( ICONS::NEW ), "New Logbook", this );
    newLogbookAction_->setToolTip( "Create a new logbook" );
    connect( newLogbookAction_, SIGNAL( triggered() ), SLOT( _newLogbook() ) );

    openAction_ = new QAction( IconEngine::get( ICONS::OPEN ), "Open", this );
    openAction_->setToolTip( "Open an existsing logbook" );
    openAction_->setShortcut( Qt::CTRL+Qt::Key_O );
    connect( openAction_, SIGNAL( triggered() ), SLOT( open() ) );

    synchronizeAction_ = new QAction( "Synchronize", this );
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

    saveAsAction_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save As", this );
    saveAsAction_->setToolTip( "Save logbook with a different name" );
    connect( saveAsAction_, SIGNAL( triggered() ), SLOT( _saveAs() ) );

    saveBackupAction_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save Backup", this );
    saveBackupAction_->setToolTip( "Save logbook backup" );
    connect( saveBackupAction_, SIGNAL( triggered() ), SLOT( _saveBackup() ) );

    revertToSaveAction_ = new QAction( IconEngine::get( ICONS::RELOAD ), "Reload", this );
    revertToSaveAction_->setToolTip( "Restore saved logbook" );
    revertToSaveAction_->setShortcut( Qt::Key_F5 );
    connect( revertToSaveAction_, SIGNAL( triggered() ), SLOT( _revertToSaved() ) );

    // print
    printAction_ = new QAction( IconEngine::get( ICONS::PRINT ), "Print", this );
    printAction_->setShortcut( Qt::CTRL + Qt::Key_P );
    connect( printAction_, SIGNAL( triggered() ), SLOT( _print() ) );

    // print preview
    addAction( printPreviewAction_ = new QAction( IconEngine::get( ICONS::PRINT_PREVIEW ), "Print Preview", this ) );
    printPreviewAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_P );
    connect( printPreviewAction_, SIGNAL( triggered() ), SLOT( _printPreview() ) );

    logbookStatisticsAction_ = new QAction( IconEngine::get( ICONS::INFO ), "Logbook Statistics", this );
    logbookStatisticsAction_->setToolTip( "View logbook statistics" );
    connect( logbookStatisticsAction_, SIGNAL( triggered() ), SLOT( _viewLogbookStatistics() ) );

    logbookInformationsAction_ = new QAction( IconEngine::get( ICONS::INFO ), "Logbook Informations", this );
    logbookInformationsAction_->setToolTip( "Edit logbook informations" );
    connect( logbookInformationsAction_, SIGNAL( triggered() ), SLOT( _editLogbookInformations() ) );

    closeFramesAction_ = new QAction( IconEngine::get( ICONS::CLOSE ), "Close Editors", this );
    closeFramesAction_->setToolTip( "Close all entry editors" );
    connect( closeFramesAction_, SIGNAL( triggered() ), SLOT( _closeEditionWindows() ) );

    // show duplicated entries
    showDuplicatesAction_ = new QAction( "Show Duplicated Entries", this );
    showDuplicatesAction_->setToolTip( "Show duplicated entries in logbook" );
    connect( showDuplicatesAction_, SIGNAL( triggered() ), SLOT( _showDuplicatedEntries() ) );

    // view monitored files
    monitoredFilesAction_ = new QAction( "Show Monitored Files", this );
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
        BASE::KeySet<LogEntry> entries( logbook()->entries() );
        for( BASE::KeySet<LogEntry>::iterator it = entries.begin(); it != entries.end(); it++ )
        {
            if( (!treeModeAction().isChecked() && (*it)->isFindSelected()) || (*it)->isSelected() )
            { modelEntries.push_back( *it ); }
        }

        _logEntryModel().add( modelEntries );

    }

    // loop over associated editionwindows
    // update navigation buttons
    BASE::KeySet<EditionWindow> frames( this );
    for( BASE::KeySet<EditionWindow>::iterator it = frames.begin(); it != frames.end(); it++ )
    {

        // skip closed editors
        if( (*it)->isClosed() ) continue;

        // get associated entry and see if selected
        LogEntry* entry( (*it)->entry() );
        (*it)->previousEntryAction().setEnabled( entry && entry->isSelected() && previousEntry(entry, false) );
        (*it)->nextEntryAction().setEnabled( entry && entry->isSelected() && nextEntry(entry, false) );

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
    std::set<Keyword> newKeywords;
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); ++iter )
    {
        if( (*iter)->isFindSelected() )
        {
            Keyword keyword( (*iter)->keyword() );
            while( keyword != Keyword::NO_KEYWORD )
            {
                newKeywords.insert( keyword );
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
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); ++iter )
    { colorMenu_->add( (*iter)->color() ); }

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
    return find_if( frames.begin(), frames.end(), EditionWindow::ModifiedFTor() ) != frames.end();
}

//_______________________________________________
void MainWindow::_autoSave( void )
{

    if( logbook_ && !logbook()->file().isEmpty() )
    {

        statusBar().label().setText( "performing autoSave" );

        // retrieve non read only editors; perform save
        BASE::KeySet<EditionWindow> frames( this );
        for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); ++iter )
        {
            if( (*iter)->isReadOnly() || (*iter)->isClosed() ) continue;
            (*iter)->saveAction().trigger();
        }

        save();

    } else {

        statusBar().label().setText( "no logbook filename. <Autosave> skipped" );

    }

}

//__________________________________________________________________
AskForSaveDialog::ReturnCode MainWindow::_checkModifiedEntries( BASE::KeySet<EditionWindow> frames, const bool& confirmEntries ) const
{
    Debug::Throw( "_MainWindow::checkModifiedEntries.\n" );

    // check if editable EditionWindows needs save
    // cancel if required
    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); ++iter )
    {
        if( !((*iter)->isReadOnly() || (*iter)->isClosed()) && (*iter)->modified() )
        {
            if( !confirmEntries ) { (*iter)->saveAction().trigger(); }
            else if( (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return AskForSaveDialog::CANCEL;
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
    BASE::KeySet<EditionWindow> frames( entry );
    for( BASE::KeySet< EditionWindow >::iterator it = frames.begin(); it != frames.end(); it++ )
    {

        // keep track of already modified EditionWindows
        bool frameModified( (*it)->modified() && !(*it)->isReadOnly() );

        // update EditionWindow
        if( mask&TITLE_MASK ) (*it)->displayTitle();
        if( mask&KEYWORD_MASK ) (*it)->displayKeyword();

        // save if needed [title/keyword changes are discarded since saved here anyway]
        if( frameModified ) (*it)->askForSave( false );
        else (*it)->setModified( false );

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
bool MainWindow::_saveAs( File default_file, bool register_logbook )
{
    Debug::Throw( "MainWindow::_saveAs.\n");

    // check current logbook
    if( !logbook_ ) {
        InformationDialog( this, "no logbook opened. <Save Logbook> canceled." ).exec();
        return false;
    }

    // check default filename
    if( default_file.isEmpty() ) default_file = logbook()->file();
    if( default_file.isEmpty() ) default_file = File( "log.xml" ).addPath( workingDirectory() );

    // create file dialog
    FileDialog dialog( this );
    dialog.setAcceptMode( QFileDialog::AcceptSave );
    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.selectFile( default_file );

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
    if( register_logbook )
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
    TimeStamp last_backup( logbook()->backup() );

    // stores current logbook filename
    QString current_filename( logbook()->file() );

    // save logbook as backup
    bool saved( _saveAs( filename, false ) );

    // remove the "backup" filename from the openPrevious list
    // to avoid confusion
    Singleton::get().application<Application>()->recentFiles().remove( File(filename).expand() );

    // restore initial filename
    logbook()->setFile( current_filename );

    if( saved ) {

        logbook()->setBackup( TimeStamp::now() );
        logbook()->setModified( true );
        setWindowTitle( Application::MAIN_TITLE_MODIFIED );

        // Save logbook if needed (to make sure the backup stamp is updated)
        if( !logbook()->file().isEmpty() ) save();
    }

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

    // create options widget
    LogbookPrintOptionWidget* logbookOptionWidget = new LogbookPrintOptionWidget();
    logbookOptionWidget->read();

    LogEntryPrintSelectionWidget* logEntrySelectionWidget = new LogEntryPrintSelectionWidget();
    logEntrySelectionWidget->read();

    LogEntryPrintOptionWidget* logEntryOptionWidget = new LogEntryPrintOptionWidget();
    logEntryOptionWidget->read();

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setOptionTabs( QList<QWidget *>()
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

    // create print helper
    LogbookPrintHelper helper( this );
    helper.setLogbook( logbook() );

    // select entries
    LogEntryModel::List entries;
    switch( logEntrySelectionWidget->mode() )
    {
        default:
        case LogEntryPrintSelectionWidget::ALL_ENTRIES:
        {
            BASE::KeySet<LogEntry> entry_set( logbook()->entries() );
            entries = LogEntryModel::List( entry_set.begin(), entry_set.end() );
            break;
        }

        case LogEntryPrintSelectionWidget::VISIBLE_ENTRIES:
        entries = _logEntryModel().get();
        break;

        case LogEntryPrintSelectionWidget::SELECTED_ENTRIES:
        entries = _logEntryModel().get( logEntryList().selectionModel()->selectedRows() );
        break;

    }
    helper.setEntries( entries );

    // retrieve mask and assign
    helper.setMask( logbookOptionWidget->mask() );
    helper.setEntryMask( logEntryOptionWidget->mask() );

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

    // select entries
    LogEntryModel::List entries;
    switch( XmlOptions::get().get<unsigned int>( "LOGENTRY_PRINT_SELECTION" ) )
    {
        default:
        case LogEntryPrintSelectionWidget::ALL_ENTRIES:
        {
            BASE::KeySet<LogEntry> entry_set( logbook()->entries() );
            entries = LogEntryModel::List( entry_set.begin(), entry_set.end() );
            break;
        }

        case LogEntryPrintSelectionWidget::VISIBLE_ENTRIES:
        entries = _logEntryModel().get();
        break;

        case LogEntryPrintSelectionWidget::SELECTED_ENTRIES:
        entries = _logEntryModel().get( logEntryList().selectionModel()->selectedRows() );
        break;

    }
    helper.setEntries( entries );

    // masks
    helper.setMask( XmlOptions::get().get<unsigned int>( "LOGBOOK_PRINT_OPTION_MASK" ) );
    helper.setEntryMask( XmlOptions::get().get<unsigned int>( "LOGENTRY_PRINT_OPTION_MASK" ) );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this );
    dialog.setWindowTitle( "Print Preview - elogbook" );
    dialog.setHelper( helper );
    dialog.exec();

    // reset status bar
    statusBar().label().setText( "" );
    statusBar().showLabel();

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
    File remote_file( FileDialog(this).getFile() );
    if( remote_file.isNull() ) return;

    // debug
    Debug::Throw() << "MainWindow::_synchronize - number of local files: " << MainWindow::logbook()->children().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - number of local entries: " << MainWindow::logbook()->entries().size() << endl;

    // set busy flag
    Singleton::get().application<Application>()->busy();
    statusBar().label().setText( "reading remote logbook ... " );

    // opens file in remote logbook
    Debug::Throw() << "MainWindow::_synchronize - reading remote logbook from file: " << remote_file << endl;

    Logbook remote_logbook;
    connect( &remote_logbook, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );
    remote_logbook.setFile( remote_file );
    remote_logbook.read();

    // check if logbook is valid
    XmlError::List errors( remote_logbook.xmlErrors() );
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
    Debug::Throw() << "MainWindow::_synchronize - number of remote files: " << remote_logbook.children().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - number of remote entries: " << remote_logbook.entries().size() << endl;
    Debug::Throw() << "MainWindow::_synchronize - updating local from remote" << endl;

    // synchronize local with remote
    // retrieve map of duplicated entries
    std::map<LogEntry*,LogEntry*> duplicates( MainWindow::logbook()->synchronize( remote_logbook ) );
    Debug::Throw() << "MainWindow::_synchronize - number of duplicated entries: " << duplicates.size() << endl;

    // update possible EditionWindows when duplicated entries are found
    // delete the local duplicated entries
    for( std::map<LogEntry*,LogEntry*>::iterator iter = duplicates.begin(); iter != duplicates.end(); ++iter )
    {

        // display the new entry in all matching edit frames
        BASE::KeySet<EditionWindow> frames( iter->first );
        for( BASE::KeySet<EditionWindow>::iterator frameIter = frames.begin(); frameIter != frames.end(); ++frameIter )
        { (*frameIter)->displayEntry( iter->second ); }

        delete iter->first;

    }

    // reinitialize lists
    _resetKeywordList();
    _resetLogEntryList();
    resetAttachmentWindow();

    // retrieve last modified entry
    BASE::KeySet<LogEntry> entries( MainWindow::logbook()->entries() );
    BASE::KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
    selectEntry( *iter );
    logEntryList().setFocus();

    // write local logbook
    if( !MainWindow::logbook()->file().isEmpty() ) save();

    // synchronize remove with local
    Debug::Throw() << "MainWindow::_synchronize - updating remote from local" << endl;
    unsigned int n_duplicated = remote_logbook.synchronize( *MainWindow::logbook() ).size();
    Debug::Throw() << "MainWindow::_synchronize - number of duplicated entries: " << n_duplicated << endl;

    // save remote logbook
    statusBar().label().setText( "saving remote logbook ... " );
    remote_logbook.write();

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

    // dissasociate from logbook
    for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); ++iter )
    {

        BASE::KeySet<Logbook> logbooks( *iter );
        for( BASE::KeySet<Logbook>::iterator logIter = logbooks.begin(); logIter != logbooks.end(); ++logIter )
        { (*logIter)->setModified( true ); }

        (*iter)->clearAssociations<Logbook>();

    }

    //! put entry set into a list and sort by creation time.
    // First entry must the oldest
    std::list<LogEntry*> entry_list( entries.begin(), entries.end() );
    entry_list.sort( LogEntry::FirstCreatedFTor() );

    // put entries in logbook
    for( std::list<LogEntry*>::iterator iter = entry_list.begin(); iter != entry_list.end(); ++iter )
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
    LogEntry *last_visibleEntry( 0 );

    // keep track of current index
    QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

    // keep track of found entries
    int found( 0 );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    BASE::KeySet<LogEntry> turned_offEntries;
    for( BASE::KeySet<LogEntry>::iterator iter=entries.begin(); iter!= entries.end(); ++iter )
    {

        // retrieve entry
        LogEntry* entry( *iter );

        // if entry is already hidden, skipp
        if( !entry->isSelected() ) continue;

        // check duplicated entries
        int n_duplicates( count_if( entries.begin(), entries.end(), LogEntry::SameCreationFTor( (*iter)->creation() ) ) );
        if( n_duplicates < 2 ) {

            entry->setFindSelected( false );
            turned_offEntries.insert( entry );

        } else {

            found++;
            last_visibleEntry = entry;

        }

    }

    if( !found )
    {

        InformationDialog( this, "No matching entry found.\nRequest canceled." ).centerOnParent().exec();

        // reset flag for the turned off entries to true
        for( BASE::KeySet<LogEntry>::iterator it=turned_offEntries.begin(); it!= turned_offEntries.end(); it++ )
            (*it)->setFindSelected( true );

        return;
    }

    // reinitialize logEntry list
    _resetKeywordList();
    _resetLogEntryList();

    // if EditionWindow current entry is visible, select it;
    if( selectedEntry && selectedEntry->isSelected() ) selectEntry( selectedEntry );
    else if( last_visibleEntry ) selectEntry( last_visibleEntry );

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
void MainWindow::_closeEditionWindows( void ) const
{
    Debug::Throw( "MainWindow::_closeEditionWindows.\n" );

    // get all EditionWindows from MainWindow
    BASE::KeySet<EditionWindow> frames( this );
    if( _checkModifiedEntries( frames, true ) == AskForSaveDialog::CANCEL ) return;
    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); ++iter )
    { (*iter)->deleteLater(); }
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
    EditionWindow *editFrame( 0 );
    BASE::KeySet<EditionWindow> frames( this );

    // the order is reversed to start from latest
    for( BASE::KeySet<EditionWindow>::reverse_iterator iter=frames.rbegin(); iter != frames.rend(); ++iter )
    {
        // skip closed editors
        if( !(*iter)->isClosed() ) continue;
        editFrame = *iter;
        editFrame->setIsClosed( false );
        editFrame->setReadOnly( false );
    }

    if( !editFrame )
    {
        // create new EditionWindow
        editFrame = new EditionWindow( 0, false );
        editFrame->setColorMenu( colorMenu_ );
        Key::associate( this, editFrame );
    }

    // force editFrame show keyword flag
    editFrame->setForceShowKeyword( !treeModeAction().isChecked() );

    // call NewEntry for the selected frame
    editFrame->newEntryAction().trigger();

    // show frame
    editFrame->centerOnWidget( this );
    editFrame->show();

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
    QModelIndexList selected_indexes( logEntryList().selectionModel()->selectedRows() );

    // convert into LogEntry list
    LogEntryModel::List selection;
    bool has_edited_index( false );
    for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); ++iter )
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
    EditionWindow *editFrame( 0 );
    BASE::KeySet<EditionWindow> frames( this );
    for( BASE::KeySet<EditionWindow>::iterator iter=frames.begin(); iter != frames.end(); ++iter )
    {

        // skip closed editors
        if( (*iter)->isClosed() ) continue;

        // check if EditionWindow is editable and match editor
        if( !((*iter)->isReadOnly() ) && (*iter)->entry() == entry )
        {
            editFrame = *iter;
            editFrame->uniconifyAction().trigger();
            return;
        }

    }

    // if no editFrame is found, try re-used a closed editor
    if( !editFrame )
    {

        // the order is reversed to start from latest
        for( BASE::KeySet<EditionWindow>::reverse_iterator iter=frames.rbegin(); iter != frames.rend(); ++iter )
        {

            // skip closed editors
            if( !(*iter)->isClosed() ) continue;

            editFrame = *iter;
            editFrame->setIsClosed( false );
            editFrame->setReadOnly( false );

            // also clear modification state
            editFrame->setModified( false );

            // need to display entry before deleting sub views.
            editFrame->displayEntry( entry );

            // also kill all frames but one
            BASE::KeySet< AnimatedTextEditor > editors( *iter );
            if( editors.size() > 1 )
            {

                BASE::KeySet< AnimatedTextEditor >::iterator localIter( editors.begin() );
                ++localIter;
                for( ;localIter != editors.end(); ++localIter )
                { (*iter)->closeEditor( **localIter ); }

                (**editors.begin()).setFocus();
                (*iter)->setActiveEditor( **editors.begin() );

            }

            break;
        }

    }

    // if no editFrame is found create a new one
    if( !editFrame )
    {
        editFrame = new EditionWindow( 0, false );
        editFrame->setColorMenu( colorMenu_ );
        Key::associate( this, editFrame );
        editFrame->displayEntry( entry );
    }

    editFrame->setForceShowKeyword( !treeModeAction().isChecked() );
    editFrame->centerOnWidget( this );
    editFrame->show();

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
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); ++iter )
    { (*iter)->setModified( true ); }

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
        BASE::KeySet<EditionWindow> frames( entry );
        for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); ++iter )
        { if( !(*iter)->isClosed() ) (*iter)->displayColor(); }

        // set logbooks as modified
        BASE::KeySet<Logbook> logbooks( entry );
        for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); ++iter )
        { (*iter)->setModified( true ); }

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
    dialog.setWindowTitle( "New Keyword - Elogbook" );

    KeywordModel::List keywords( _keywordModel().children() );
    for( KeywordModel::List::const_iterator iter = keywords.begin(); iter != keywords.end(); ++iter )
        dialog.add( *iter );
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
    QModelIndexList selected_indexes( keywordList().selectionModel()->selectedRows() );
    if( selected_indexes.empty() )
    {
        InformationDialog( this, "no keyword selected. Request canceled" ).exec();
        return;
    }

    // store corresponding list of keywords
    KeywordModel::List keywords;
    for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); ++iter )
    { if( iter->isValid() ) keywords.push_back( _keywordModel().get( *iter ) ); }

    // retrieve associated entries
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    BASE::KeySet<LogEntry> associatedEntries;

    for( KeywordModel::List::iterator iter = keywords.begin(); iter != keywords.end(); ++iter )
    {
        for( BASE::KeySet<LogEntry>::iterator entryIter = entries.begin(); entryIter != entries.end(); ++entryIter )
        { if( (*entryIter)->keyword().inherits( *iter ) ) associatedEntries.insert( *entryIter );  }
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
        for( BASE::KeySet<LogEntry>::iterator iter = associatedEntries.begin(); iter != associatedEntries.end(); ++iter )
        { deleteEntry( *iter, false ); }

    }


    // reset keywords
    _resetKeywordList();

    // select last valid keyword parent
    for( KeywordModel::List::reverse_iterator iter = keywords.rbegin(); iter != keywords.rend(); ++iter )
    {

        // retrieve index associated to parent keyword
        // if valid, select and break
        QModelIndex index( _keywordModel().index( iter->parent() ) );
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
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); ++iter )
    {

        LogEntry* entry( *iter );

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
            for( BASE::KeySet<Logbook>::iterator logIter = logbooks.begin(); logIter!= logbooks.end(); ++logIter )
            { (*logIter)->setModified( true ); }

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
    dialog.setWindowTitle( "Edit Keyword - Elogbook" );

    const KeywordModel::List& keywords( _keywordModel().children() );
    for( KeywordModel::List::const_iterator iter = keywords.begin(); iter != keywords.end(); ++iter )
    { dialog.add( *iter ); }

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
    LogEntryModel::List selection( _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );
    for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); ++iter )
    {

        // retrieve entry
        LogEntry* entry( *iter );

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
        for( BASE::KeySet<Logbook>::iterator logIter = logbooks.begin(); logIter!= logbooks.end(); ++logIter )
        { (*logIter)->setModified( true ); }

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
        QModelIndex last_index;
        for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); ++iter )
        {
            QModelIndex index( _logEntryModel().index( *iter ) );
            if( index.isValid() )
            {
                last_index = index;
                logEntryList().selectionModel()->select( index, QItemSelectionModel::Select|QItemSelectionModel::Rows );
            }
        }

        // update current index
        if( last_index.isValid() )
        {
            logEntryList().selectionModel()->setCurrentIndex( last_index,  QItemSelectionModel::Select|QItemSelectionModel::Rows );
            logEntryList().scrollTo( last_index );
        }
    }

    // Save logbook if needed
    if( !logbook()->file().isEmpty() ) save();

    return;

}

//_______________________________________________
void MainWindow::KeywordSelectionChanged( const QModelIndex& index )
{

    Debug::Throw( "MainWindow::KeywordSelectionChanged.\n" );
    if( !logbook_ ) return;
    if( !index.isValid() ) return;

    Keyword keyword( _keywordModel().get( index ) );
    Debug::Throw() << "MainWindow::KeywordSelectionChanged - keyword: " << keyword << endl;

    // keep track of the current selected entry
    QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
    LogEntry *selectedEntry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

    // retrieve all logbook entries
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    BASE::KeySet<LogEntry> turned_offEntries;
    for( BASE::KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
    {
        LogEntry* entry( *it );
        entry->setKeywordSelected( (*it)->keyword() == keyword );
    }

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
    bool has_selection( selectedEntries > 0 );

    if( selectedEntries > 1 )
    {
        editEntryAction().setText( "&Edit Entries" );
        deleteEntryAction().setText( "&Delete Entries" );
    } else {
        editEntryAction().setText( "&Edit Entry" );
        deleteEntryAction().setText( "&Delete Entry" );
    }

    editEntryAction().setEnabled( has_selection );
    deleteEntryAction().setEnabled( has_selection );
    entryColorAction().setEnabled( has_selection );
    entryKeywordAction().setEnabled( has_selection );
    editEntryTitleAction().setEnabled( has_selection );

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
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); ++iter )
    { (*iter)->setModified( true ); }

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
    QModelIndexList selected_indexes( logEntryList().selectionModel()->selectedRows() );
    for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); ++iter )
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
    QModelIndexList selected_indexes( _logEntryModel().selectedIndexes() );
    if( selected_indexes.empty() ) logEntryList().selectionModel()->clear();
    else {

        logEntryList().selectionModel()->select( selected_indexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); ++iter )
        { logEntryList().selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

    }

    return;
}

//________________________________________
void MainWindow::_storeSelectedKeywords( void )
{
    // clear
    _keywordModel().clearSelectedIndexes();

    // retrieve selected indexes in list
    QModelIndexList selected_indexes( keywordList().selectionModel()->selectedRows() );
    for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); ++iter )
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
    QModelIndexList selected_indexes( _keywordModel().selectedIndexes() );
    if( selected_indexes.empty() ) keywordList().selectionModel()->clear();
    else {

        keywordList().selectionModel()->select( selected_indexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
        for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); ++iter )
        { keywordList().selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }

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

    QModelIndexList expanded_indexes( _keywordModel().expandedIndexes() );
    keywordList().collapseAll();
    for( QModelIndexList::const_iterator iter = expanded_indexes.begin(); iter != expanded_indexes.end(); ++iter )
    { keywordList().setExpanded( *iter, true ); }

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
    BASE::KeySet<EditionWindow> frames( this );
    for( BASE::KeySet<EditionWindow>::const_iterator iter = frames.begin(); iter != frames.end(); ++iter )
    {

        // if hiding keyword, first need ask for save
        if( value && !(*iter)->isClosed() && (*iter)->modified() && !(*iter)->isReadOnly() )
        { (*iter)->askForSave( false ); }

        // update force flag
        (*iter)->setForceShowKeyword( !value );

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
