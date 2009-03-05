
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

/*!
  \file MainWindow.cpp
  \brief base class to display entries and keyword::
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QHeaderView>
#include <QMenu>

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
#include "HtmlHeaderNode.h"
#include "IconEngine.h"
#include "Icons.h"
#include "InformationDialog.h"
#include "LineEditor.h"
#include "Logbook.h"
#include "LogbookInformationDialog.h"
#include "LogbookModifiedDialog.h"
#include "LogbookStatisticsDialog.h"
#include "MainWindow.h"
#include "Menu.h"
#include "NewLogbookDialog.h"
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "RecentFilesMenu.h"
#include "SearchPanel.h"
#include "SelectionStatusBar.h"
#include "Singleton.h"
#include "TextEditionDelegate.h"
#include "Util.h"
#include "PrintLogbookDialog.h"
#include "XmlOptions.h"

using namespace std;

//_____________________________________________
MainWindow::MainWindow( QWidget *parent ):
  BaseMainWindow( parent ),
  Counter( "MainWindow" ),
  autosave_delay_( 60000 ),
  edition_delay_( 200 ),
  logbook_( 0 ),
  working_directory_( Util::workingDirectory() ),
  ignore_warnings_( false ),
  confirm_entries_( true )
{
  Debug::Throw( "MainWindow::MainWindow.\n" );
  setOptionName( "MAIN_WINDOW" );
  setWindowTitle( Application::MAIN_TITLE );
  
  // file checker
  file_check_ = new FileCheck( this );
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
  layout->addWidget( splitter = new QSplitter( main ), 1 );  
  splitter->setOrientation( Qt::Horizontal );
 
  // search panel
  search_panel_ = new SearchPanel( "Search panel", this, "SEARCH_PANEL" );
  connect( &searchPanel(), SIGNAL( selectEntries( QString, unsigned int ) ), SLOT( selectEntries( QString, unsigned int ) ) );
  connect( &searchPanel(), SIGNAL( showAllEntries() ), SLOT( showAllEntries() ) );  
  searchPanel().visibilityAction().setShortcut( Qt::CTRL+Qt::Key_F );
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
  
  // left box for Keywords and buttons
  QWidget* left = new QWidget();
  
  // set layout
  QVBoxLayout* v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  left->setLayout( v_layout );
  
  keyword_toolbar_ = new CustomToolBar( "Keywords Toolbar", left, "KEYWORD_TOOLBAR" );
  v_layout->addWidget( keyword_toolbar_ );
  
  // keyword actions
  keyword_toolbar_->addAction( &newKeywordAction() );
  keyword_toolbar_->addAction( &deleteKeywordAction() );
  keyword_toolbar_->addAction( &editKeywordAction() );
  Debug::Throw() << "MainWindow::MainWindow - keyword toolbar created." << endl;
  
  // create keyword list
  v_layout->addWidget( keyword_list_ = new KeywordList( left ), 1 );
  keywordList().setFindEnabled( false );
  keywordList().setModel( &_keywordModel() );
  keywordList().setRootIsDecorated( true );
  keywordList().setSortingEnabled( true );
  keywordList().setDragEnabled(true);
  keywordList().setAcceptDrops(true);
  keywordList().setDropIndicatorShown(true);
  
  // default width from options, if found
  if( XmlOptions::get().find( "KEYWORD_LIST_WIDTH" ) )
  { keyword_list_->setDefaultWidth( XmlOptions::get().get<int>( "KEYWORD_LIST_WIDTH" ) ); }

  // the use of a custom delegate unfortunately disable the 
  // nice selection appearance of the oxygen style.
  keywordList().setItemDelegate( new TextEditionDelegate( this ) );
  
  // update LogEntryList when keyword selection change
  connect( keywordList().selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _keywordSelectionChanged( const QModelIndex& ) ) );  
  connect( keywordList().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection& ) ), SLOT( _updateKeywordActions() ) );
  _updateKeywordActions();
    
  // rename selected entries when KeywordChanged is emitted with a single argument.
  // this correspond to drag and drop action from the logEntryList in the KeywordList
  connect( &keyword_model_, SIGNAL( entryKeywordChanged( Keyword ) ), SLOT( _renameEntryKeyword( Keyword ) ) );
  
  // rename all entries matching first keyword the second. This correspond to 
  // drag and drop inside the keyword list, or to direct edition of a keyword list item.
  connect( &keyword_model_, SIGNAL( keywordChanged( Keyword, Keyword ) ), SLOT( _renameKeyword( Keyword, Keyword ) ) );
    
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
  add the delete_keyword_action to the keyword list,
  so that the corresponding shortcut gets activated whenever it is pressed
  while the list has focus
  */
  keywordList().addAction( &deleteKeywordAction() );
  keywordList().addAction( &editKeywordAction() );
  
  // right box for entries and buttons
  QWidget* right = new QWidget();
  
  v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  right->setLayout( v_layout );
    
  entry_toolbar_ = new CustomToolBar( "Entries Toolbar", right, "ENTRY_TOOLBAR" );
  v_layout->addWidget( entry_toolbar_ );

  // entry actions
  entry_toolbar_->addAction( &newEntryAction() );
  entry_toolbar_->addAction( &editEntryAction() );
  
  // need to use a button to be able to set the popup mode
  QToolButton *button = new QToolButton(0);
  button->setText( "&Entry Color" );
  button->setIcon( IconEngine::get( ICONS::COLOR ) );
  button->setPopupMode( QToolButton::InstantPopup );
  button->setMenu( color_menu_ );
  entry_toolbar_->addWidget( button );

  entry_toolbar_->addAction( &deleteEntryAction() );
  entry_toolbar_->addAction( &saveAction() );
  entry_toolbar_->addAction( &printAction() );
   
  // create logEntry list
  v_layout->addWidget( entry_list_ = new AnimatedTreeView( right ), 1 );
  logEntryList().setFindEnabled( false );
  logEntryList().setModel( &_logEntryModel() );
  logEntryList().setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  logEntryList().setDragEnabled(true); 
  logEntryList().setOptionName( "ENTRY_LIST" );
  
  // the use of a custom delegate unfortunately disable the 
  // nice selection appearance of the oxygen style.
  logEntryList().setItemDelegate( new TextEditionDelegate( this ) );
  
  connect( logEntryList().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
  connect( logEntryList().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateEntryActions() ) );
  connect( entry_list_, SIGNAL( activated( const QModelIndex& ) ), SLOT( _entryItemActivated( const QModelIndex& ) ) ); 
  connect( entry_list_, SIGNAL( clicked( const QModelIndex& ) ), SLOT( _entryItemClicked( const QModelIndex& ) ) );
  _updateEntryActions();

  connect( &_logEntryModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelectedEntries() ) );
  connect( &_logEntryModel(), SIGNAL( layoutChanged() ), SLOT( _restoreSelectedEntries() ) );  
  connect( &_logEntryModel(), SIGNAL( layoutChanged() ), entry_list_, SLOT( resizeColumns() ) );  
  connect( &_logEntryModel(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _entryDataChanged( const QModelIndex& ) ) ); 

  /* 
  add the delete_entry_action to the list,
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
  
  
  // add widgets to splitters
  splitter->addWidget( left );
  splitter->addWidget( right );

  // assign stretch factors
  splitter->setStretchFactor( 0, 0 );
  splitter->setStretchFactor( 1, 1 );
 
  connect( splitter, SIGNAL( splitterMoved( int, int ) ), SLOT( _splitterMoved( void ) ) );
 
  // main menu
  menu_ = new Menu( this , this );
  setMenuBar( menu_ );
    
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
  
  Debug::Throw("MainWindow::SetLogbook.\n" );

  // reset current logbook
  if( logbook_ ) reset();

  // clear file checker
  fileCheck().clear();
  
  // create new logbook
  logbook_ = new Logbook();

  // save in menu
  menu().recentFilesMenu().setCurrentFile( file );
  
  // if filename is empty, return
  if( file.isEmpty() )
  {
    // update listView with new entries
    _resetKeywordList();
    _resetLogEntryList();
    emit ready();
    return false;
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
  setEnabled( false );
  logbook()->read();
  setEnabled( true );

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
  working_directory_ = File( logbook()->file() ).path();
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
  
  ignore_warnings_ = false;

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
  for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ ) 
  { 
    (*iter)->setIsClosed( true ); 
    (*iter)->hide();
  }
  
  return;
  
}

//____________________________________________
AskForSaveDialog::ReturnCode MainWindow::askForSave( const bool& enable_cancel )
{

  Debug::Throw( "MainWindow::askForSave.\n" );

  // create dialog 
  unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
  if( enable_cancel ) buttons |= AskForSaveDialog::CANCEL;
  
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
void MainWindow::updateEntry( LogEntry* entry, const bool& update_selection )
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
  if( update_selection )
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
  for( BASE::KeySet<Attachment>::iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  {

    // retrieve/delete associated attachment frames
    BASE::KeySet<AttachmentFrame> frames( *iter );
    for( BASE::KeySet<AttachmentFrame>::iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
    { (*frame_iter)->remove( **iter ); }

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
  for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  { 
    (*iter)->setIsClosed( true );
    (*iter)->hide();
  }

  // set logbooks as modified
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
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
  
  for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  { (*iter)->setReadOnly( true ); }
  
  return true;
}

//_______________________________________________
LogEntry* MainWindow::previousEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "MainWindow::previousEntry.\n" );
  QModelIndex index( _logEntryModel().index( entry ) );
  if( !( index.isValid() && index.row() > 0 ) ) return 0;
  
  QModelIndex previous_index( _logEntryModel().index( index.row()-1, index.column() ) );
  if( update_selection ) 
  {
    logEntryList().selectionModel()->select( previous_index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    logEntryList().selectionModel()->setCurrentIndex( previous_index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
  }
  
  return _logEntryModel().get( previous_index );
  
}

//_______________________________________________
LogEntry* MainWindow::nextEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "MainWindow::nextEntry.\n" );
  QModelIndex index( _logEntryModel().index( entry ) );
  if( !( index.isValid() && index.row()+1 < _logEntryModel().rowCount() ) ) return 0;
 
  QModelIndex next_index( _logEntryModel().index( index.row()+1, index.column() ) );
  if( update_selection ) 
  {
    logEntryList().selectionModel()->select( next_index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
    logEntryList().selectionModel()->setCurrentIndex( next_index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );
  }
  
  return _logEntryModel().get( next_index );

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
void MainWindow::save( const bool& confirm_entries )
{

  Debug::Throw( "MainWindow::_save.\n" );
 
  // check logbook
  if( !logbook_ ) 
  {
    InformationDialog( this, "no Logbook opened. <Save> canceled." ).exec();
    return;
  }

  if( !confirm_entries ) confirm_entries_ = false;
  
  if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), confirm_entries_ ) == AskForSaveDialog::CANCEL ) return;
  
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
      confirm_entries_ = true;
      return;
    }

    // check file is writable
    if( !fullname.isWritable() ) {
      InformationDialog( this, "selected file is not writable. <Save Logbook> canceled." ).exec();
      confirm_entries_ = true;
      return;
    }
    
  } else {

    File path( fullname.path() );
    if( !path.isDirectory() ) {
      InformationDialog( this, "selected path is not vallid. <Save Logbook> canceled." ).exec();
      confirm_entries_ = true;
      return;
    }

  }
  
  // write logbook to file, retrieve result
  Singleton::get().application<Application>()->busy();
  setEnabled( false );
  bool written( logbook()->write() );
  Singleton::get().application<Application>()->idle();
  setEnabled( true );

  if( written ) { setWindowTitle( Application::MAIN_TITLE );}
  
  // update StateFrame
  statusBar().label().setText( "" );
  statusBar().showLabel();
  
  // add new file to openPreviousMenu
  if( !logbook()->file().isEmpty() ) Singleton::get().application<Application>()->recentFiles().add( logbook()->file().expand() );

  // reset ignore_warning flag
  ignore_warnings_ = false;

  // reset confirm entries
  confirm_entries_ = true;
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
  LogEntry *last_visible_entry( 0 );

  // keep track of the current selected entry
  QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
  LogEntry *selected_entry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );
  
  // check is selection is a valid color when Color search is requested.
  bool color_valid = ( mode&SearchPanel::COLOR && ( selection.compare( ColorMenu::NONE, Qt::CaseInsensitive ) == 0 || QColor( selection ).isValid() ) );
  
  // retrieve all logbook entries
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  BASE::KeySet<LogEntry> turned_off_entries;
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
      if( entry->isKeywordSelected() || !(last_visible_entry && last_visible_entry->isKeywordSelected()) )
      { last_visible_entry = entry; }
    
    } else {
    
      turned_off_entries.insert( entry );
      entry->setFindSelected( false );
    
    }

  }

  // if no entries are found, restore the disabled entries and abort
  if( !found ) 
  {

    statusBar().label().setText( "no match found. Find canceled" );

    // reset flag for the turned off entries to true
    for( BASE::KeySet<LogEntry>::iterator it=turned_off_entries.begin(); it!= turned_off_entries.end(); it++ )
    (*it)->setFindSelected( true );

    return;
    
  }

  // reinitialize logEntry list
  _resetKeywordList();
  _resetLogEntryList();

  // if EditionWindow current entry is visible, select it;
  if( selected_entry && selected_entry->isSelected() ) selectEntry( selected_entry );
  else if( last_visible_entry ) selectEntry( last_visible_entry );

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
  LogEntry *selected_entry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

  // set all logbook entries to find_visible
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  for( BASE::KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
  { (*it)->setFindSelected( true ); }

  // reinitialize logEntry list
  _resetKeywordList();
  _resetLogEntryList();

  if( selected_entry && selected_entry->isSelected() ) selectEntry( selected_entry );
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

  if( event->timerId() == resize_timer_.timerId() )
  {
    
    // stop timer and save time
    resize_timer_.stop();
    XmlOptions::get().set<int>( "KEYWORD_LIST_WIDTH", keywordList().width() );
  
  } else if(event->timerId() == drag_timer_.timerId() ) {
  
    Debug::Throw(0) << "MainWindow::timerEvent - enabling drag" << endl;
    drag_timer_.stop();
    _logEntryModel().setDragEnabled( true );

  } else if(event->timerId() == edition_timer_.timerId() ) {
  
    edition_timer_.stop();
    
    // check if current index is valid and was 'double-clicked'
    QModelIndex index( logEntryList().currentIndex() );
    if( index.isValid() && index == _logEntryModel().editionIndex() ) 
    { _startEntryEdition(); }

  } else if(event->timerId() == autosave_timer_.timerId() ) {
    
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
  uniconify_action_ = new QAction( IconEngine::get( ICONS::HOME ), "&Main Window", this );
  uniconify_action_->setToolTip( "Raise application main window" );
  connect( uniconify_action_, SIGNAL( triggered() ), SLOT( uniconify() ) );

  new_keyword_action_ = new QAction( IconEngine::get( ICONS::NEW ), "&New Keyword", this );
  new_keyword_action_->setToolTip( "Create a new keyword" );
  connect( new_keyword_action_, SIGNAL( triggered() ), SLOT( _newKeyword() ) );

  addAction( edit_keyword_action_ = new QAction( IconEngine::get( ICONS::RENAME ), "&Rename Keyword", this ) );
  edit_keyword_action_->setToolTip( "Rename selected keyword" );
  edit_keyword_action_->setShortcut( Qt::Key_F2 );
  edit_keyword_action_->setShortcutContext( Qt::WidgetShortcut );  
  connect( edit_keyword_action_, SIGNAL( triggered() ), SLOT( _renameKeyword() ) );
  
  /*
  delete keyword action
  it is associated to the Qt::Key_Delete shortcut
  but the later is enabled only if the KeywordList has focus.
  */
  delete_keyword_action_ = new QAction( IconEngine::get( ICONS::DELETE ), "&Delete Keyword", this );
  delete_keyword_action_->setToolTip( "Delete selected keyword" );
  delete_keyword_action_->setShortcut( Qt::Key_Delete );
  delete_keyword_action_->setShortcutContext( Qt::WidgetShortcut );  
  connect( delete_keyword_action_, SIGNAL( triggered() ), SLOT( _deleteKeyword() ) );

  new_entry_action_ = new QAction( IconEngine::get( ICONS::NEW ), "&New Entry", this );
  new_entry_action_->setToolTip( "Create a new entry" );
  new_entry_action_->setShortcut( Qt::CTRL+Qt::Key_N );
  connect( new_entry_action_, SIGNAL( triggered() ), SLOT( _newEntry() ) );

  edit_entry_action_ = new QAction( IconEngine::get( ICONS::EDIT ), "&Edit Entries", this );
  edit_entry_action_->setToolTip( "Edit selected entries" );
  connect( edit_entry_action_, SIGNAL( triggered() ), SLOT( _editEntries() ) );

  edit_entry_title_action_ = new QAction( IconEngine::get( ICONS::RENAME ), "&Rename Entry", this );
  edit_entry_title_action_->setToolTip( "Edit selected entry title" );
  edit_entry_title_action_->setShortcut( Qt::Key_F2 );
  edit_entry_title_action_->setShortcutContext( Qt::WidgetShortcut );  
  connect( edit_entry_title_action_, SIGNAL( triggered() ), SLOT( _startEntryEdition() ) );

  /*
  delete entry action
  it is associated to the Qt::Key_Delete shortcut
  but the later is enabled only if the KeywordList has focus.
  */
  delete_entry_action_ = new QAction( IconEngine::get( ICONS::DELETE ), "&Delete Entries", this );
  delete_entry_action_->setToolTip( "Delete selected entries" );
  delete_entry_action_->setShortcut( Qt::Key_Delete );
  delete_entry_action_->setShortcutContext( Qt::WidgetShortcut );  
  connect( delete_entry_action_, SIGNAL( triggered() ), SLOT( _deleteEntries() ) );

  // color menu
  color_menu_ = new ColorMenu( this );
  color_menu_->setTitle( "&Change entry color" );
  connect( color_menu_, SIGNAL( selected( QColor ) ), SLOT( _changeEntryColor( QColor ) ) );

  entry_color_action_ = new QAction( IconEngine::get( ICONS::COLOR ), "&Entry Color", this );
  entry_color_action_->setToolTip( "Change selected entries color" );
  entry_color_action_->setMenu( color_menu_ );
  
  entry_keyword_action_ = new QAction( IconEngine::get( ICONS::EDIT ), "&Change Keyword", this );
  entry_keyword_action_->setToolTip( "Edit selected entries keyword" );
  connect( entry_keyword_action_, SIGNAL( triggered() ), SLOT( _renameEntryKeyword() ) );

  new_logbook_action_ = new QAction( IconEngine::get( ICONS::NEW ), "&New Logbook", this );
  new_logbook_action_->setToolTip( "Create a new logbook" );
  connect( new_logbook_action_, SIGNAL( triggered() ), SLOT( _newLogbook() ) );

  open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open", this );
  open_action_->setToolTip( "Open an existsing logbook" );
  open_action_->setShortcut( Qt::CTRL+Qt::Key_O );
  connect( open_action_, SIGNAL( triggered() ), SLOT( open() ) );

  synchronize_action_ = new QAction( "&Synchronize", this );
  synchronize_action_->setToolTip( "Synchronize current logbook with remote" );
  connect( synchronize_action_, SIGNAL( triggered() ), SLOT( _synchronize() ) );

  reorganize_action_ = new QAction( "&Reorganize", this );
  reorganize_action_->setToolTip( "Reoganize logbook entries in files" );
  connect( reorganize_action_, SIGNAL( triggered() ), SLOT( _reorganize() ) );

  save_action_ = new QAction( IconEngine::get( ICONS::SAVE ), "&Save", this );
  save_action_->setToolTip( "Save all edited entries" );
  connect( save_action_, SIGNAL( triggered() ), SLOT( save() ) );

  save_forced_action_ = new QAction( IconEngine::get( ICONS::SAVE ), "&Save (forced)", this );
  save_forced_action_->setToolTip( "Save all entries" );
  connect( save_forced_action_, SIGNAL( triggered() ), SLOT( _saveForced() ) );

  save_as_action_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save &As", this );
  save_as_action_->setToolTip( "Save logbook with a different name" );
  connect( save_as_action_, SIGNAL( triggered() ), SLOT( _saveAs() ) );

  save_backup_action_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "Save &Backup", this );
  save_backup_action_->setToolTip( "Save logbook backup" );
  connect( save_backup_action_, SIGNAL( triggered() ), SLOT( _saveBackup() ) );

  revert_to_save_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "&Reload", this );
  revert_to_save_action_->setToolTip( "Restore saved logbook" );
  revert_to_save_action_->setShortcut( Qt::Key_F5 );
  connect( revert_to_save_action_, SIGNAL( triggered() ), SLOT( _revertToSaved() ) );

  print_action_ = new QAction( IconEngine::get( ICONS::PRINT ), "&Print", this );
  print_action_->setShortcut( Qt::CTRL + Qt::Key_P );
  print_action_->setToolTip( "Convert logbook to html and print" );
  connect( print_action_, SIGNAL( triggered() ), SLOT( _print() ) );

  logbook_statistics_action_ = new QAction( IconEngine::get( ICONS::INFO ), "Logbook Statistics", this );
  logbook_statistics_action_->setToolTip( "View logbook statistics" );
  connect( logbook_statistics_action_, SIGNAL( triggered() ), SLOT( _viewLogbookStatistics() ) );
  
  logbook_informations_action_ = new QAction( IconEngine::get( ICONS::INFO ), "Logbook Informations", this );
  logbook_informations_action_->setToolTip( "Edit logbook informations" );
  connect( logbook_informations_action_, SIGNAL( triggered() ), SLOT( _editLogbookInformations() ) );

  close_frames_action_ = new QAction( IconEngine::get( ICONS::CLOSE ), "&Close Editors", this );
  close_frames_action_->setToolTip( "Close all entry editors" );
  connect( close_frames_action_, SIGNAL( triggered() ), SLOT( _closeEditionWindows() ) );

  show_duplicates_action_ = new QAction( "Show Duplicated Entries", this );
  show_duplicates_action_->setToolTip( "Show duplicated entries in logbook" );
  connect( show_duplicates_action_, SIGNAL( triggered() ), SLOT( _showDuplicatedEntries() ) );
  
  monitored_files_action_ = new QAction( "Show Monitored Files", this );
  monitored_files_action_->setToolTip( "Show monitored files" );
  connect( monitored_files_action_, SIGNAL( triggered() ), SLOT( _showMonitoredFiles() ) );

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
     
    LogEntryModel::List model_entries;
    BASE::KeySet<LogEntry> entries( logbook()->entries() );
    for( BASE::KeySet<LogEntry>::iterator it = entries.begin(); it != entries.end(); it++ )
    { if( (*it)->isSelected() ) model_entries.push_back( *it ); }
    
    _logEntryModel().add( model_entries );
    
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
  set<Keyword> new_keywords;
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )  
  { 
    if( (*iter)->isFindSelected() ) 
    {
      Keyword keyword( (*iter)->keyword() );
      while( keyword != Keyword::NO_KEYWORD ) 
      { 
        new_keywords.insert( keyword ); 
        keyword = keyword.parent(); 
      }
      
    }
  }
  
  _keywordModel().set( new_keywords );

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
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  { color_menu_->add( (*iter)->color() ); }

}


//_______________________________________________
void MainWindow::_updateConfiguration( void )
{
  
  Debug::Throw( "MainWindow::_updateConfiguration.\n" );
    
  resize( sizeHint() );
  
  // autoSave
  autosave_delay_ = 1000*XmlOptions::get().get<int>( "AUTO_SAVE_ITV" );
  bool autosave( XmlOptions::get().get<bool>( "AUTO_SAVE" ) );
  if( autosave ) autosave_timer_.start( autosave_delay_, this );
  else autosave_timer_.stop();
  
  // colors
  Options::List color_list( XmlOptions::get().specialOptions( "COLOR" ) );
  for( Options::List::iterator iter = color_list.begin(); iter != color_list.end(); iter++ )
  { color_menu_->add( iter->raw() ); }
  
}

//_____________________________________________
void MainWindow::_filesModified( FileCheck::DataSet files )
{
  
  Debug::Throw( "MainWindow::_filesModified.\n" );
  
  if( ignore_warnings_ ) return;
  if( files.empty() ) return;
  
  // ask dialog and take action accordinly
  int state = LogbookModifiedDialog( this, files ).exec();
  if( state == LogbookModifiedDialog::RESAVE ) { save(); }
  else if( state == LogbookModifiedDialog::SAVE_AS ) { _saveAs(); }
  else if( state == LogbookModifiedDialog::RELOAD ) 
  { 
    
    logbook()->setModifiedRecursive( false ); 
    _revertToSaved(); 
  
  } else if( state == LogbookModifiedDialog::IGNORE ) { ignore_warnings_ = true; }
  
  return;
}

//________________________________________________________
void MainWindow::_splitterMoved( void )
{
  Debug::Throw( "MainWindow::_splitterMoved.\n" );
  resize_timer_.start( 200, this );  
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
  if( _checkModifiedEntries( BASE::KeySet<EditionWindow>( this ), confirm_entries_ ) == AskForSaveDialog::CANCEL ) return;
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
  working_directory_ = fullname.path();

  // check if file exists
  if(
    fullname.exists() &&
    !QuestionDialog( this, "selected file already exists. Overwrite ?" ).exec() )
  return false;

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
  ignore_warnings_ = false;

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
  if( ( _hasModifiedEntries() || logbook()->modified() ) && !QuestionDialog( this, "discard changes to current logbook ?" ).exec() ) 
  { return; }

  // reinit MainWindow
  Singleton::get().application<Application>()->busy();
  setLogbook( logbook()->file() );
  Singleton::get().application<Application>()->idle();

  checkLogbookBackup();
  ignore_warnings_ = false;
  
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
  for( std::map<LogEntry*,LogEntry*>::iterator iter = duplicates.begin(); iter != duplicates.end(); iter++ )
  {
    
    // display the new entry in all matching edit frames
    BASE::KeySet<EditionWindow> frames( iter->first );
    for( BASE::KeySet<EditionWindow>::iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
    { (*frame_iter)->displayEntry( iter->second ); }

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
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    
    BASE::KeySet<Logbook> logbooks( *iter );
    for( BASE::KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter != logbooks.end(); log_iter++ )
    { (*log_iter)->setModified( true ); }
    
    (*iter)->clearAssociations<Logbook>();
    
  }

  //! put entry set into a list and sort by creation time. 
  // First entry must the oldest
  list<LogEntry*> entry_list( entries.begin(), entries.end() );
  entry_list.sort( LogEntry::FirstCreatedFTor() );
  
  // put entries in logbook
  for( list<LogEntry*>::iterator iter = entry_list.begin(); iter != entry_list.end(); iter++ )
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
  LogEntry *last_visible_entry( 0 );

  // keep track of current index
  QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
  LogEntry *selected_entry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );

  // keep track of found entries
  int found( 0 );

  // retrieve all logbook entries
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  BASE::KeySet<LogEntry> turned_off_entries;
  for( BASE::KeySet<LogEntry>::iterator iter=entries.begin(); iter!= entries.end(); iter++ )
  {

    // retrieve entry
    LogEntry* entry( *iter );

    // if entry is already hidden, skipp
    if( !entry->isSelected() ) continue;

    // check duplicated entries
    int n_duplicates( count_if( entries.begin(), entries.end(), LogEntry::SameCreationFTor( (*iter)->creation() ) ) );
    if( n_duplicates < 2 ) {
      
      entry->setFindSelected( false );
      turned_off_entries.insert( entry );
    
    } else {
    
      found++;
      last_visible_entry = entry;
    
    }

  }

  if( !found ) 
  {
    
    InformationDialog( this, "No matching entry found.\nRequest canceled." ).centerOnParent().exec();

    // reset flag for the turned off entries to true
    for( BASE::KeySet<LogEntry>::iterator it=turned_off_entries.begin(); it!= turned_off_entries.end(); it++ )
    (*it)->setFindSelected( true );

    return;
  }

  // reinitialize logEntry list
  _resetKeywordList();
  _resetLogEntryList();

  // if EditionWindow current entry is visible, select it;
  if( selected_entry && selected_entry->isSelected() ) selectEntry( selected_entry );
  else if( last_visible_entry ) selectEntry( last_visible_entry );

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
  for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  { (*iter)->deleteLater(); }
  return;
}

//____________________________________________
void MainWindow::_newEntry( void )
{
  
  Debug::Throw( "MainWindow::_NewEntry.\n" );

  // retrieve associated EditionWindows, check if one matches the selected entry
  EditionWindow *edit_frame( 0 );
  BASE::KeySet<EditionWindow> frames( this );
  
  // the order is reversed to start from latest
  for( BASE::KeySet<EditionWindow>::reverse_iterator iter=frames.rbegin(); iter != frames.rend(); iter++ )
  {
    // skip closed editors
    if( !(*iter)->isClosed() ) continue;
    edit_frame = *iter;
    edit_frame->setIsClosed( false );
    edit_frame->setReadOnly( false );
  }
  
  if( !edit_frame )
  {
    // create new EditionWindow
    edit_frame = new EditionWindow( 0, false );
    edit_frame->setColorMenu( color_menu_ );
    Key::associate( this, edit_frame );
  }

  // call NewEntry for the selected frame
  edit_frame->newEntryAction().trigger();

  // show frame
  edit_frame->centerOnWidget( this );
  edit_frame->show();

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
  for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
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
  for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
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
  for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
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
  EditionWindow *edit_frame( 0 );
  BASE::KeySet<EditionWindow> frames( this );
  for( BASE::KeySet<EditionWindow>::iterator iter=frames.begin(); iter != frames.end(); iter++ )
  {
    
    // skip closed editors
    if( (*iter)->isClosed() ) continue;
    
    // check if EditionWindow is editable and match editor
    if( !((*iter)->isReadOnly() ) && (*iter)->entry() == entry ) 
    {
      edit_frame = *iter;
      edit_frame->uniconifyAction().trigger();
      break;
    }

  }

  // if no editFrame is found, try re-used a closed editor
  if( !edit_frame )
  {
    
    // the order is reversed to start from latest
    for( BASE::KeySet<EditionWindow>::reverse_iterator iter=frames.rbegin(); iter != frames.rend(); iter++ )
    {
      
      // skip closed editors
      if( !(*iter)->isClosed() ) continue;
      edit_frame = *iter;
      edit_frame->setIsClosed( false );
      edit_frame->setReadOnly( false );
      break;
    }
    
  }
  
  // if no editFrame is found create a new one
  if( !edit_frame )
  {
    edit_frame = new EditionWindow( 0, false );
    edit_frame->setColorMenu( color_menu_ );
    Key::associate( this, edit_frame );
  }
  
  edit_frame->centerOnWidget( this );
  edit_frame->displayEntry( entry );
  edit_frame->show();
  
  Debug::Throw( "MainWindow::_displayEntry - done.\n" );

}

//_______________________________________________
void MainWindow::_changeEntryTitle( LogEntry* entry, QString new_title )
{
  Debug::Throw( "MainWindow::_changeEntryTitle.\n" );
  
  // make sure that title was changed
  if( new_title == entry->title() ) return;
  
  // update entry title
  entry->setTitle( new_title );
  
  // update associated EditionWindows
  BASE::KeySet<EditionWindow> frames( entry );
  for( BASE::KeySet< EditionWindow >::iterator it = frames.begin(); it != frames.end(); it++ )
  {
    
    // keep track of already modified EditionWindows
    bool frame_modified( (*it)->modified() && !(*it)->isReadOnly() );
    
    // update EditionWindow
    (*it)->displayTitle();
    
    // save if needed [title/keyword changes are discarded since saved here anyway]
    if( frame_modified ) (*it)->askForSave( false );
    else (*it)->setModified( false );
    
  }
  
  // set logbooks as modified
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
  (*iter)->setModified( true );
  
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
  for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    
    // get associated entry
    LogEntry* entry( *iter );

    entry->setColor( color.isValid() ? color.name():ColorMenu::NONE );
    entry->setModification( entry->modification()+1 );
        
    // update EditionWindow color
    BASE::KeySet<EditionWindow> frames( entry );
    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    { if( !(*iter)->isClosed() ) (*iter)->displayColor(); }

    // set logbooks as modified
    BASE::KeySet<Logbook> logbooks( entry );
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
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
  dialog.setWindowTitle( "New keyword" );

  KeywordModel::List keywords( _keywordModel().children() );
  for( KeywordModel::List::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
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
  for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
  { if( iter->isValid() ) keywords.push_back( _keywordModel().get( *iter ) ); }
  
  // retrieve associated entries
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  BASE::KeySet<LogEntry> associated_entries;
  
  for( KeywordModel::List::iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  {
    for( BASE::KeySet<LogEntry>::iterator entry_iter = entries.begin(); entry_iter != entries.end(); entry_iter++ )
    { if( (*entry_iter)->keyword().inherits( *iter ) ) associated_entries.insert( *entry_iter );  }
  }
  
  //! create dialog
  DeleteKeywordDialog dialog( this, keywords, !associated_entries.empty() );
  if( !dialog.centerOnParent().exec() ) 
  { return; }
  
  if( dialog.moveEntries() && associated_entries.size() ) 
  {

    Debug::Throw( "MainWindow::_deleteKeyword - moving entries.\n" );
    for( KeywordModel::List::iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
    { _renameKeyword( *iter, iter->parent(), false );  }

  } else if( dialog.deleteEntries() ) {
    
    Debug::Throw( "MainWindow::_deleteKeyword - deleting entries.\n" );
    for( BASE::KeySet<LogEntry>::iterator iter = associated_entries.begin(); iter != associated_entries.end(); iter++ )
    { deleteEntry( *iter, false ); }
    
  }

  
  // reset keywords 
  _resetKeywordList();

  // select last valid keyword parent
  for( KeywordModel::List::reverse_iterator iter = keywords.rbegin(); iter != keywords.rend(); iter++ )
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
void MainWindow::_renameKeyword( Keyword keyword, Keyword new_keyword, bool update_selection )
{

  Debug::Throw("MainWindow::_renameKeyword.\n" );
  
  // check keywords are different
  if( keyword == new_keyword ) return;
    
  // get entries matching the old_keyword, change the keyword
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    
    LogEntry* entry( *iter );
    
    /* 
      if keyword to modify is a leading subset of current entry keyword, 
      update entry with new keyword
    */
    if( entry->keyword().inherits( keyword ) ) 
    {
      
      entry->setKeyword( Keyword( Str( entry->keyword().get() ).replace( keyword.get(), new_keyword.get() ) ) );
      
      /* this is a kludge: add 1 second to the entry modification timeStamp to avoid loosing the 
      keyword change when synchronizing logbooks, without having all entries modification time
      set to now() */
      entry->setModification( entry->modification()+1 );

      BASE::KeySet<Logbook> logbooks( entry );
      for( BASE::KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter!= logbooks.end(); log_iter++ )
      { (*log_iter)->setModified( true ); }
    
    }
    
  }

  // reset lists
  _resetKeywordList();  
  if( update_selection )
  {
    
    // make sure parent keyword index is expanded
    QModelIndex parent_index( _keywordModel().index( new_keyword.parent() ) );
    if( parent_index.isValid() ) keywordList().setExpanded( parent_index, true );
    
    // retrieve current index, and select
    QModelIndex index( _keywordModel().index( new_keyword ) );
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
  dialog.setWindowTitle( "Edit keyword" );

  const KeywordModel::List& keywords( _keywordModel().children() );
  for( KeywordModel::List::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  dialog.add( *iter );
  dialog.setKeyword( keyword );
  
  // map dialog
  if( !dialog.centerOnParent().exec() ) return;

  // check if keyword was changed
  Keyword new_keyword( dialog.keyword() );
  if( keyword == new_keyword ) return;
  
  // change keyword for all entries that match the old one
  _renameEntryKeyword( new_keyword );
  
}

//_______________________________________________
void MainWindow::_renameEntryKeyword( Keyword new_keyword, bool update_selection )
{
      
  Debug::Throw() << "MainWindow::_renameEntryKeyword - new_keyword: " << new_keyword << endl;
  
  // keep track of modified entries
  BASE::KeySet<LogEntry> entries;
  
  // retrieve current selection
  LogEntryModel::List selection( _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );
  for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    
    // retrieve entry
    LogEntry* entry( *iter );
    
    // check if entry keyword has changed
    if( entry->keyword() == new_keyword ) continue;
    
    // change keyword and set as modified
    entry->setKeyword( new_keyword );
    
    /* this is a kludge: add 1 second to the entry modification timeStamp to avoid loosing the 
    keyword change when synchronizing logbooks, without having all entries modification time
    set to now() */
    entry->setModification( entry->modification()+1 );
    
    // keep track of modified entries
    entries.insert( entry );
    
    // set associated logbooks as modified
    BASE::KeySet<Logbook> logbooks( entry );
    for( BASE::KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter!= logbooks.end(); log_iter++ )
    { (*log_iter)->setModified( true ); }
  
  }

  // check if at least one entry was changed
  if( entries.empty() ) return;
  
  // reset lists
  _resetKeywordList();

  // update keyword selection
  if( update_selection )
  {
        
    // make sure parent keyword index is expanded
    QModelIndex parent_index( _keywordModel().index( new_keyword.parent() ) );
    if( parent_index.isValid() ) keywordList().setExpanded( parent_index, true );

    // retrieve current index, and select
    QModelIndex index( _keywordModel().index( new_keyword ) );
    keywordList().selectionModel()->select( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );    
    keywordList().selectionModel()->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows );    
    keywordList().scrollTo( index );
  }
  
  // update entry selection
  _resetLogEntryList();
  
  if( update_selection )
  {
    // clear current selection
    logEntryList().clearSelection();
    
    // select all modified entries
    QModelIndex last_index;
    for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
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
void MainWindow::_keywordSelectionChanged( const QModelIndex& index )
{

  Debug::Throw( "MainWindow::_keywordSelectionChanged.\n" );
  if( !logbook_ ) return; 
  if( !index.isValid() ) return;
  
  Keyword keyword( _keywordModel().get( index ) );
  Debug::Throw() << "MainWindow::_keywordSelectionChanged - keyword: " << keyword << endl;
      
  // keep track of the last visible entry
  LogEntry *last_visible_entry( 0 );
  
  // keep track of the current selected entry
  QModelIndex current_index( logEntryList().selectionModel()->currentIndex() );
  LogEntry *selected_entry( current_index.isValid() ? _logEntryModel().get( current_index ):0 );
  
  // retrieve all logbook entries
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  BASE::KeySet<LogEntry> turned_off_entries;
  for( BASE::KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
  {
  
    // retrieve entry
    LogEntry* entry( *it );
    if( entry->keyword() == keyword ) 
    {  
      entry->setKeywordSelected( true );
      if( entry->isFindSelected() ) last_visible_entry = entry;
      Debug::Throw() << "MainWindow::_keywordSelectionChanged - found entry: " << entry->key() << endl;
    } else entry->setKeywordSelected( false );
    
  }
  
  // reinitialize logEntry list
  _resetLogEntryList();
  
  // if EditionWindow current entry is visible, select it;
  if( selected_entry && selected_entry->isSelected() ) selectEntry( selected_entry );
  else if( last_visible_entry ) selectEntry( last_visible_entry );
  
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
  bool has_selection( !logEntryList().selectionModel()->selectedRows().empty() );
  editEntryAction().setEnabled( has_selection );
  deleteEntryAction().setEnabled( has_selection );
  entryColorAction().setEnabled( has_selection );
  entryKeywordAction().setEnabled( has_selection );
  editEntryTitleAction().setEnabled( has_selection );  
  return;
}

//_____________________________________________
void MainWindow::_print( void )
{
  Debug::Throw( "MainWindow::_print.\n" );

  // check logbook
  if( !logbook_ )
  {
    InformationDialog( this, "No logbook opened. <View HTML> canceled." ).exec();
    return;
  }

  // check if logbook is modified
  if( logbook()->modified() ) askForSave();
  
  // create custom dialog, retrieve check vbox child
  PrintLogbookDialog dialog( this );
  dialog.setSelection( PrintLogbookDialog::ALL );
  dialog.setLogbookMask( Logbook::HTML_ALL_MASK );
  dialog.setEntryMask( LogEntry::HTML_ALL_MASK );

  // add commands
  /* command list contains the HTML editor, PDF editor and any additional user specified command */
  Options::List commands( XmlOptions::get().specialOptions( "PRINT_COMMAND" ) );
  if( !AttachmentType::HTML.editCommand().isEmpty() ) commands.push_back( AttachmentType::HTML.editCommand() );
  for( Options::List::iterator iter = commands.begin(); iter != commands.end(); iter++ )
  { dialog.addCommand( iter->raw() ); }  
  
  QString buffer;
  QTextStream( &buffer ) << "_eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
  dialog.setFile( File( buffer ).addPath( Util::tmp() ) );

  // map dialog
  if( !dialog.centerOnParent().exec() ) return;

  // save command
  QString command( dialog.command() );
  XmlOptions::get().add( "PRINT_COMMAND", Option( command, Option::RECORDABLE|Option::CURRENT ) );  

  // retrieve/check file
  File file( dialog.file() );
  if( file.isEmpty() ) 
  {
    InformationDialog(this, "No output file specified. <View HTML> canceled." ).exec();
    return;
  }

  // open/check temp file
  QFile out( file );
  if( !out.open( QIODevice::WriteOnly ) ) 
  {
    QString buffer;
    QTextStream(&buffer ) << "Cannot write to file \"" << file << "\". <View HTML> canceled.";
    InformationDialog( this, buffer ).exec();
    return;
  }

  // retrieve mask
  unsigned int html_log_mask = dialog.logbookMask();
  unsigned int html_entry_mask = dialog.entryMask();

  QDomDocument document( "html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"DTD/xhtml1-strict.dtd\"" );
  
  // html
  QDomElement html = document.appendChild( document.createElement( "html" ) ).toElement(); 
  html.setAttribute( "xmlns", "http://www.w3.org/1999/xhtml" );
  
  // head
  HtmlHeaderNode( html, document );
  
  // body
  QDomElement body = html.appendChild( document.createElement( "body" ) ).toElement();

  // dump logbook header
  body.appendChild( logbook()->htmlElement( document, html_log_mask ) );

  // retrieve entries to write
  LogEntryModel::List entries;
  if( dialog.allEntries() )
  {

    BASE::KeySet<LogEntry> entry_set( logbook()->entries() );
    entries = LogEntryModel::List( entry_set.begin(), entry_set.end() );

  } else if( dialog.visibleEntries() ) { 
    
    entries = _logEntryModel().get();
    
  } else {

    entries = _logEntryModel().get( logEntryList().selectionModel()->selectedRows() );

  }

  // sort entries using keyword
  sort( entries.begin(), entries.end(), Logbook::EntryLessFTor( Logbook::SORT_KEYWORD ) );

  if( html_log_mask & Logbook::HTML_TABLE )
  {
    // dump table of content
    QDomElement table = body.appendChild( document.createElement( "table" ) ).toElement();
    table.setAttribute( "class", "header_outer_table" );
    QDomElement column = table.
      appendChild( document.createElement( "tr" ) ).
      appendChild( document.createElement( "td" ) ).
      toElement();
    column.setAttribute( "class", "header_column" );
    column.
      appendChild( document.createElement( "h2" ) ).
      appendChild( document.createTextNode( "Table of contents" ) );
    column = table.
      appendChild( document.createElement( "tr" ) ).
      appendChild( document.createElement( "td" ) ).
      toElement();
    column.setAttribute( "class", "header_column" );
    table = column.appendChild( document.createElement( "table" ) ).toElement();
    table.setAttribute( "class", "header_inner_table" );
    QDomElement row = table.appendChild( document.createElement( "tr" ) ).toElement();
    
    if( html_entry_mask & LogEntry::HTML_KEYWORD )
      row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( "keyword" ) );
    if( html_entry_mask & LogEntry::HTML_TITLE )
      row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( "title" ) );
    if( html_entry_mask & LogEntry::HTML_CREATION )
      row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( "created" ) );
    if( html_entry_mask & LogEntry::HTML_MODIFICATION )
      row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( "last modified" ) );
    
    for( vector< LogEntry* >::iterator it=entries.begin(); it != entries.end(); it++ )
    { table.appendChild( (*it)->htmlSummary( document ) ); }
    
  }
    
  // dump full logbook
  if( html_log_mask & Logbook::HTML_CONTENT )
  { 
    for( vector<LogEntry*>::iterator it=entries.begin(); it != entries.end(); it++ )
    { body.appendChild( (*it)->htmlElement( document, html_entry_mask ) ); }
  }

  out.write( document.toString().toAscii() );
  out.close();

  // retrieve command
  if( !dialog.command().isEmpty() ) ( Command( dialog.command() ) << file ).run();
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
  edition_timer_.stop();
  if( index.isValid() ) _displayEntry( _logEntryModel().get( index ) ); 
}

//____________________________________________________________
void MainWindow::_entryItemClicked( const QModelIndex& index )
{ 
  
  // do nothing if index do not correspond to an entry title
  if( !(index.isValid() && index.column() == LogEntryModel::TITLE ) ) return;

  // do nothing if index is not already selected
  if( !logEntryList().selectionModel()->isSelected( index ) ) return;

  // compare to model edition index
  if( index == _logEntryModel().editionIndex() ) edition_timer_.start( edition_delay_, this );
  else _logEntryModel().setEditionIndex( index );
  
}

//_______________________________________________
void MainWindow::_entryDataChanged( const QModelIndex& index )
{
  Debug::Throw( "MainWindow::_entryDataChanged.\n" );
  
  if( !( index.isValid() && index.column() == LogEntryModel::TITLE ) ) return;
  LogEntry* entry( _logEntryModel().get( index ) );
    
  // update associated EditionWindows
  BASE::KeySet<EditionWindow> frames( entry );
  for( BASE::KeySet< EditionWindow >::iterator it = frames.begin(); it != frames.end(); it++ )
  {
    
    // keep track of already modified EditionWindows
    bool frame_modified( (*it)->modified() && !(*it)->isReadOnly() );
    
    // update EditionWindow
    (*it)->displayTitle();
    
    // save if needed [title/keyword changes are discarded since saved here anyway]
    if( frame_modified ) (*it)->askForSave( false );
    else (*it)->setModified( false );
    
  }
  
  // set logbooks as modified
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
  (*iter)->setModified( true );
  
  // save Logbook
  if( logbook() && !logbook()->file().isEmpty() ) save();
 
}

//________________________________________
void MainWindow::_startEntryEdition( void )
{   

  Debug::Throw( "MainWindow::_startEntryEdition\n" );

  // make sure title column is visible
  if( logEntryList().isColumnHidden( LogEntryModel::TITLE ) ) return;
  
  // get current index and check validity
  QModelIndex index( logEntryList().currentIndex() );
  if( !index.isValid() ) return;

  // make sure 'title' index is selected
  index = _logEntryModel().index( index.row(), LogEntryModel::TITLE );
  
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
  for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
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
    for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
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
  for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
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
    for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
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
  for( QModelIndexList::iterator iter = indexes.begin(); iter != indexes.end(); iter++ )
  { if( keywordList().isExpanded( *iter ) ) _keywordModel().setIndexExpanded( *iter, true ); }
    
}

//________________________________________
void MainWindow::_restoreExpandedKeywords( void )
{
  
  Debug::Throw( "MainWindow::_restoreExpandedKeywords.\n" );
  
  QModelIndexList expanded_indexes( _keywordModel().expandedIndexes() );
  keywordList().collapseAll();  
  for( QModelIndexList::const_iterator iter = expanded_indexes.begin(); iter != expanded_indexes.end(); iter++ )
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

//_______________________________________________
void MainWindow::_autoSave( void )
{

  if( logbook_ && !logbook()->file().isEmpty() ) 
  {
  
    statusBar().label().setText( "performing autoSave" );

    // retrieve non read only editors; perform save
    BASE::KeySet<EditionWindow> frames( this );
    for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    { 
      if( (*iter)->isReadOnly() || (*iter)->isClosed() ) continue;
      (*iter)->saveAction().trigger();
    }
    
    save();
  
  } else
  statusBar().label().setText( "no logbook filename. <Autosave> skipped" );

}

//__________________________________________________________________
bool MainWindow::_hasModifiedEntries( void ) const
{
  BASE::KeySet<EditionWindow> frames( this );
  return find_if( frames.begin(), frames.end(), EditionWindow::ModifiedFTor() ) != frames.end();  
}

//__________________________________________________________________
AskForSaveDialog::ReturnCode MainWindow::_checkModifiedEntries( BASE::KeySet<EditionWindow> frames, const bool& confirm_entries ) const
{  
  Debug::Throw( "_MainWindow::checkModifiedEntries.\n" );

  // check if editable EditionWindows needs save
  // cancel if required
  for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  {
    if( !((*iter)->isReadOnly() || (*iter)->isClosed()) && (*iter)->modified() )
    {
      if( !confirm_entries ) { (*iter)->saveAction().trigger(); }
      else if( (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return AskForSaveDialog::CANCEL;
    }
  }
  
  return  AskForSaveDialog::YES;
}

//______________________________________________________________________
void MainWindow::KeywordList::setDefaultWidth( const int& value )
{ default_width_ = value; }

//____________________________________________
QSize MainWindow::KeywordList::sizeHint( void ) const
{ return (default_width_ ) >= 0 ? QSize( default_width_, 0 ):TreeView::sizeHint(); }
