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
  \file SelectionFrame.cpp
  \brief base class to display entries and keyword::
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QHeaderView>

#include "AttachmentFrame.h"
#include "BaseIcons.h"
#include "ColorMenu.h"
#include "CustomLineEdit.h"
#include "CustomPixmap.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "DeleteKeywordDialog.h"
#include "EditFrame.h"
#include "EditKeywordDialog.h"
#include "HtmlUtil.h"
#include "IconEngine.h"
#include "Icons.h"
#include "KeywordDelegate.h"
#include "Logbook.h"
#include "LogEntryDelegate.h"
#include "LogbookInformationDialog.h"
#include "LogbookModifiedDialog.h"
#include "LogbookStatisticsDialog.h"
#include "Menu.h"
#include "MainFrame.h"
#include "NewLogbookDialog.h"
#include "OpenPreviousMenu.h"
#include "QtUtil.h"
#include "SearchPanel.h"
#include "SelectionFrame.h"
#include "StatusBar.h"
#include "Util.h"
#include "ViewHtmlLogbookDialog.h"
#include "XmlOptions.h"

using namespace std;
using namespace Qt;

//_____________________________________________
SelectionFrame::SelectionFrame( QWidget *parent ):
  CustomMainWindow( parent ),
  Counter( "SelectionFrame" ),
  autosave_timer_( this ),
  edition_timer_( this ),
  logbook_( 0 ),
  working_directory_( Util::workingDirectory() ),
  ignore_warnings_( false ),
  confirm_entries_( true )
{
  Debug::Throw( "SelectionFrame::SelectionFrame.\n" );
  setWindowTitle( MainFrame::MAIN_TITLE );
  
  // main widget
  QWidget* main = new QWidget( this );
  setCentralWidget( main ); 
  
  // local layout
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing(5);
  main->setLayout( layout );
    
  // splitter for KeywordList/LogEntryList
  layout->addWidget( splitter_ = new QSplitter( main ), 1 );  
  splitter_->setOrientation( Horizontal );
 
  // search panel
  search_panel_ = new SearchPanel( main );
  connect( search_panel_, SIGNAL( selectEntries( QString, unsigned int ) ), SLOT( selectEntries( QString, unsigned int ) ) );
  connect( search_panel_, SIGNAL( showAllEntries() ), SLOT( showAllEntries() ) );
  layout->addWidget( search_panel_ );  
  
  // status bar
  setStatusBar( statusbar_ = new StatusBar( this ) );
  statusBar().addLabel(1);
  statusBar().addClock();
  connect( this, SIGNAL( messageAvailable( const QString& ) ), &statusBar().label(), SLOT( setTextAndUpdate( const QString& ) ) );  
  
  // global scope actions
  _installActions();

  // left box for Keywords and buttons
  QWidget* left = new QWidget( splitter_ );
  QVBoxLayout* v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  left->setLayout( v_layout );
  
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );
  
  CustomToolBar* toolbar = new CustomToolBar( "keywords toolbar", left );
  v_layout->addWidget( toolbar );
  
  // keyword actions
  toolbar->addAction( &newKeywordAction() );
  toolbar->addAction( &editKeywordAction() );
  toolbar->addAction( &deleteKeywordAction() );
  Debug::Throw() << "SelectionFrame::SelectionFrame - keyword toolbar created." << endl;
  
  // create keyword list
  v_layout->addWidget( keyword_list_ = new TreeView( left ), 1 );
  keyword_list_->setModel( &_keywordModel() );
  keyword_list_->setRootIsDecorated( true );
  keyword_list_->setSortingEnabled( false );
  keyword_list_->setDragEnabled(true);
  keyword_list_->setAcceptDrops(true);
  keyword_list_->setDropIndicatorShown(true);
  keyword_list_->setItemDelegate( new KeywordDelegate( this ) );

  // update LogEntryList when keyword selection change
  connect( keyword_list_->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), SLOT( _keywordSelectionChanged( const QModelIndex& ) ) );  
  connect( keyword_list_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection& ) ), SLOT( _updateKeywordActions() ) );
  _updateKeywordActions();
    
  // rename selected entries when KeywordChanged is emitted with a single argument.
  // this correspond to drag and drop action from the logEntryList in the KeywordList
  connect( &keyword_model_, SIGNAL( entryKeywordChanged( Keyword ) ), SLOT( _renameEntryKeyword( Keyword ) ) );
  
  // rename all entries matching first keyword the second. This correspond to 
  // drag and drop inside the keyword list, or to direct edition of a keyword list item.
  connect( &keyword_model_, SIGNAL( keywordChanged( Keyword, Keyword ) ), SLOT( _renameKeyword( Keyword, Keyword ) ) );
    
  // popup menu for keyword list
  keyword_list_->menu().addAction( &newKeywordAction() ); 
  keyword_list_->menu().addAction( &editKeywordAction() );
  keyword_list_->menu().addAction( &deleteKeywordAction() );

  connect( &_keywordModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelectedKeywords() ) );
  connect( &_keywordModel(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeExpandedKeywords() ) );
  
  connect( &_keywordModel(), SIGNAL( layoutChanged() ), SLOT( _restoreSelectedKeywords() ) );
  connect( &_keywordModel(), SIGNAL( layoutChanged() ), SLOT( _restoreExpandedKeywords() ) );

  /* 
  add the delete_keyword_action to the keyword list,
  so that the corresponding shortcut gets activated whenever it is pressed
  while the list has focus
  */
  keyword_list_->addAction( &deleteKeywordAction() );
  
  // right box for entries and buttons
  QWidget* right = new QWidget( splitter_ );
  v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  right->setLayout( v_layout );
    
  toolbar = new CustomToolBar( "entries toolbar", right );
  v_layout->addWidget( toolbar );

  // entry actions
  toolbar->addAction( &newEntryAction() );
  toolbar->addAction( &editEntryAction() );
 
  CustomToolButton *button;
  toolbar->addWidget( button = new CustomToolButton( toolbar,&entryColorAction(), 0 ) );
  button->setPopupMode( QToolButton::InstantPopup );

  toolbar->addAction( &deleteEntryAction() );
  toolbar->addAction( &saveAction() );
  toolbar->addAction( &viewHtmlAction() );
   
  // create logEntry list
  v_layout->addWidget( entry_list_ = new TreeView( right ), 1 );
  entry_list_->setModel( &_logEntryModel() );
  entry_list_->setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  entry_list_->setDragEnabled(true); 
  entry_list_->setItemDelegate( new LogEntryDelegate( this ) );
  
  connect( entry_list_->header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
  connect( entry_list_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateEntryActions() ) );
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
  entry_list_->addAction( &deleteEntryAction() );
  
  // create popup menu for list
  entry_list_->menu().addAction( &newEntryAction() );
  entry_list_->menu().addAction( &editEntryAction() ); 
  entry_list_->menu().addAction( &entryKeywordAction() );
  entry_list_->menu().addAction( &deleteEntryAction() ); 
  entry_list_->menu().addAction( &entryColorAction() );
  
  // main menu
  menu_ = new Menu( this , this );
  setMenuBar( menu_ );
  
  connect( menu_, SIGNAL( save() ), SLOT( save() ) );
  connect( menu_, SIGNAL( closeWindow() ), &static_cast<MainFrame*>(qApp)->closeAction(), SLOT( trigger() ) );
  connect( menu_, SIGNAL( viewHtml() ), this, SLOT( _viewHtml() ) );
 
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();

  // autosave_timer_
  autosave_timer_.setSingleShot( false );
  connect( &autosave_timer_, SIGNAL( timeout() ), SLOT( _autoSave() ) );
  
  // edition timer
  edition_timer_.setSingleShot( true );
  connect( &edition_timer_, SIGNAL( timeout() ), SLOT( _startEntryEdition() ) );
  
}


//_______________________________________________
void SelectionFrame::setLogbook( File file )
{
  Debug::Throw("SelectionFrame::SetLogbook.\n" );

  // reset current logbook
  if( logbook_ ) reset();

  // create new logbook
  logbook_ = new Logbook();

  // if filename is empty, return
  if( file.empty() )
  {
    // update listView with new entries
    _resetKeywordList();
    _resetLogEntryList();
    emit ready();
    return;
  }

  // set file
  logbook()->setFile( file );
  if( !file.exists() )
  {
    // update listView with new entries
    _resetKeywordList();
    _resetLogEntryList();
    emit ready();
    return;
  }

  connect( logbook_, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );

  logbook()->read();

  // update listView with new entries
  _resetKeywordList();
  _resetLogEntryList();
  _loadColors();
  
  Debug::Throw( "SelectionFrame::setLogbook - lists set.\n" );
  
  // change sorting
  Qt::SortOrder sort_order( (Qt::SortOrder) logbook()->sortOrder() );
  switch( logbook()->sortMethod() )
  {
    case Logbook::SORT_TITLE: logEntryList().sortByColumn( LogEntryModel::TITLE, sort_order ); break;
    case Logbook::SORT_CREATION: logEntryList().sortByColumn( LogEntryModel::CREATION, sort_order ); break;
    case Logbook::SORT_MODIFICATION: logEntryList().sortByColumn( LogEntryModel::MODIFICATION , sort_order); break;
    case Logbook::SORT_AUTHOR: logEntryList().sortByColumn( LogEntryModel::AUTHOR, sort_order ); break;
    default: break;
  }

  Debug::Throw( "SelectionFrame::setLogbook - lists sorted.\n" );

  // update attachment frame
  resetAttachmentFrame();
  Debug::Throw( "SelectionFrame::setLogbook - attachment frame reset.\n" );

  // retrieve last modified entry
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  BASE::KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
  selectEntry( *iter );
  logEntryList().setFocus();
  
  Debug::Throw( "SelectionFrame::setLogbook - entry selected.\n" );

  
  // see if logbook has parent file
  if( logbook()->parentFile().size() ) {
    ostringstream o; 
    o << "Warning: this logbook should be oppened via \"" << logbook()->parentFile() << "\" only.";
    QtUtil::infoDialog( this, o.str() );
  }

  // store logbook directory for next open, save comment
  working_directory_ = File( logbook()->file() ).path();
  statusBar().label().setText( "" );

  emit ready();

  // check errors
  XmlError::List errors( logbook()->xmlErrors() );
  if( errors.size() )
  {
    ostringstream what;
    if( errors.size() > 1 ) what << "Errors occured while parsing files." << endl;
    else what << "An error occured while parsing files." << endl;
    what << errors;
    QtUtil::infoDialog( 0, what.str().c_str() );
  }
  
  // add opened file to OpenPrevious mennu.
  menu().openPreviousMenu().add( logbook()->file() );
  
  ignore_warnings_ = false;
  
}

//_____________________________________________
void SelectionFrame::checkLogbookBackup( void )
{
  Debug::Throw( "SelectionFrame::checkLogbookBackup.\n" );

  // check logbook makes sense
  if( !logbook_ ) return;

  // check if oppened logbook needs backup
  if(
    XmlOptions::get().get<bool>( "AUTO_BACKUP" ) &&
    !logbook()->file().empty() &&
    logbook()->needsBackup() ) 
  {

    // ask if backup needs to be saved; save if yes
    if( QtUtil::questionDialog( this, "Current logbook needs backup. Make one?" )) 
    { _saveBackup(); }

  }

  return;
}

//_____________________________________________
void SelectionFrame::checkLogbookModified( void )
{
  
  Debug::Throw( "SelectionFrame::checkLogbookModified.\n" );

  if( ignore_warnings_ ) return;
  
  // retrieve logbook from SelectionFrame, ask for revert if needed
  if( !logbook_ ) return;
  list<File> files( logbook()->checkFiles() );
  if( files.empty() ) return;

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

//_____________________________________________
void SelectionFrame::reset( void ) 
{ 

  Debug::Throw( "SelectionFrame::reset.\n" );
  if( logbook_ ) {
        
    // delete the logbook, all corresponding entries
    delete logbook_;
    logbook_ = 0;
    
  }
    
  // clear list of entries
  _keywordModel().clear();
  _logEntryModel().clear();
    
  // clear the AttachmentFrame
  static_cast<MainFrame*>(qApp)->attachmentFrame().list().clear();
  
  // make all EditFrames for deletion
  BASE::KeySet<EditFrame> frames( this ); 
  for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ ) 
  {
    (*iter)->setIsClosed( true );
    (*iter)->hide();
  }
  
  return;
  
}

//____________________________________________
AskForSaveDialog::ReturnCode SelectionFrame::askForSave( const bool& enable_cancel )
{

  Debug::Throw( "SelectionFrame::askForSave.\n" );

  // create dialog 
  unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
  if( enable_cancel ) buttons |= AskForSaveDialog::CANCEL;
  AskForSaveDialog dialog( this, "Logbook has been modified. Save ?", buttons );
  QtUtil::centerOnParent( &dialog );
  
  // exec and check return code 
  int state = dialog.exec();
  if( state == AskForSaveDialog::YES ) save();
  return AskForSaveDialog::ReturnCode(state);
}

//_______________________________________________
void SelectionFrame::clearSelection( void )
{ logEntryList().clearSelection(); }

//_______________________________________________
void SelectionFrame::selectEntry( LogEntry* entry )
{
  Debug::Throw("SelectionFrame::selectEntry.\n" );
  
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
  Debug::Throw("SelectionFrame::selectEntry - done.\n" );
  return;
  
}

//_______________________________________________
void SelectionFrame::updateEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "SelectionFrame::updateEntry.\n" );
  
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

}


//_______________________________________________
void SelectionFrame::deleteEntry( LogEntry* entry, const bool& save )
{
  Debug::Throw( "SelectionFrame::deleteEntry.\n" );
  
  assert( entry );

  // get associated attachments
  BASE::KeySet<Attachment> attachments( entry );
  for( BASE::KeySet<Attachment>::iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  {

    // retrieve/delete associated attachmentlist items
    BASE::KeySet<AttachmentList::Item> items( *iter );
    for( BASE::KeySet<AttachmentList::Item>::iterator attachment_iter = items.begin(); attachment_iter != items.end(); attachment_iter++ )
    delete *attachment_iter;

    // delete attachment
    delete (*iter);

  };

  // remove from model
  _logEntryModel().remove( entry );
  
  /*
    hide associated EditFrames
    they will get deleted next time
    SelectionFrame::_displayEntry() is called
  */
  BASE::KeySet<EditFrame> frames( entry );
  for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  { 
    (*iter)->setIsClosed( true );
    (*iter)->hide();
  }
  //{delete *iter;}

  // set logbooks as modified
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
  (*iter)->setModified( true );

  // delete entry
  delete entry;

  //! save
  if( save && !logbook()->file().empty() )
  SelectionFrame::save();

  return;

}

//_______________________________________________
bool SelectionFrame::lockEntry( LogEntry* entry ) const
{
  Debug::Throw( "SelectionFrame::lockEntry.\n" );
  
  if( !entry ) return true;
  
  BASE::KeySet<EditFrame> frames( entry );
  for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  if(  !( (*iter)->isReadOnly() || (*iter)->isClosed() ) )
  {
    if( (*iter)->modified() && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return false;
    (*iter)->setReadOnly( true );
  }
  
  return true;
}


//_______________________________________________
LogEntry* SelectionFrame::previousEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "SelectionFrame::previousEntry.\n" );
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
LogEntry* SelectionFrame::nextEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "SelectionFrame::nextEntry.\n" );
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
void SelectionFrame::resetAttachmentFrame( void ) const
{

  Debug::Throw( "SelectionFrame::resetAttachmentFrame.\n" );

  // clear the AttachmentFrame
  AttachmentFrame &attachment_frame( static_cast<MainFrame*>(qApp)->attachmentFrame() );
  attachment_frame.list().clear();

  // check current logbook
  if( !logbook_ ) return;

  // retrieve logbook attachments, adds to AttachmentFrame
  BASE::KeySet<Attachment> attachments( logbook()->attachments() );
  for( BASE::KeySet<Attachment>::iterator it = attachments.begin(); it != attachments.end(); it++ )
  { attachment_frame.list().add( *it ); }
  attachment_frame.list().resizeColumns();
  return;

}

//_______________________________________________
Keyword SelectionFrame::currentKeyword( void ) const
{
  Debug::Throw( "SelectionFrame::currentKeyword.\n" );
  QModelIndex index( keywordList().selectionModel()->currentIndex() );
  return index.isValid() ? _keywordModel().get( index ) : Keyword();
}

//_______________________________________________
void SelectionFrame::save( const bool& confirm_entries )
{

  Debug::Throw( "SelectionFrame::save.\n" );
 
  // check logbook
  if( !logbook_ ) 
  {
    QtUtil::infoDialog( this, "no Logbook opened. <Save> canceled." );
    return;
  }

  if( !confirm_entries ) confirm_entries_ = false;
  
  // check if editable EditFrames needs save
  // cancel if required
  BASE::KeySet<EditFrame> frames( this );
  for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  {
    if( !( (*iter)->isReadOnly() || (*iter)->isClosed() ) && (*iter)->modified() )
    {
      if( !confirm_entries_ ) 
      {
        (*iter)->save();
      } else if( (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return;
    }
  }
  
  // check logbook filename, go to Save As if no file is given and redirect is true
  if( logbook()->file().empty() ) {
    _saveAs();
    return;
  }

  // check logbook filename is writable
  File fullname = File( logbook()->file() ).expand();
  if( fullname.exists() ) {

    // check file is not a directory
    if( fullname.isDirectory() ) {
      QtUtil::infoDialog( this, "selected file is a directory. <Save Logbook> canceled." );
      confirm_entries_ = true;
      return;
    }

    // check file is writable
    if( !fullname.isWritable() ) {
      QtUtil::infoDialog( this, "selected file is not writable. <Save Logbook> canceled." );
      confirm_entries_ = true;
      return;
    }
    
  } else {

    File path( fullname.path() );
    if( !path.isDirectory() ) {
      QtUtil::infoDialog(
        this, "selected path is not vallid. <Save Logbook> canceled."
      );
      confirm_entries_ = true;
      return;
    }

  }

  // write logbook to file, retrieve result
  static_cast<MainFrame*>(qApp)->busy();
  bool written( logbook()->write() );
  static_cast<MainFrame*>(qApp)->idle();

  if( written ) { setWindowTitle( MainFrame::MAIN_TITLE );}

  // update StateFrame
  statusBar().label().setText( "" );

  // add new file to openPreviousMenu
  menu().openPreviousMenu().add( logbook()->file() );

  // reset ignore_warning flag
  ignore_warnings_ = false;

  // reset confirm entries
  confirm_entries_ = true;
  return;
}

//_______________________________________________
void SelectionFrame::selectEntries( QString selection, unsigned int mode )
{
  Debug::Throw() << "SelectionFrame::selectEntries - selection: " << qPrintable( selection ) << " mode:" << mode << endl;

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
    QtUtil::infoDialog( this, "At least on field must be selected" , QtUtil::CENTER_ON_PARENT );
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

  string selection_string( qPrintable( selection ) );
  
  // check is selection_string is a valid color when Color search is requested.
  bool color_valid = ( 
    mode&SearchPanel::COLOR && ( 
    selection_string == ColorMenu::NONE ||
    QColor( selection_string.c_str() ).isValid() ) );
  
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
    if( (mode&SearchPanel::TITLE ) && entry->matchTitle( selection_string ) ) accept = true;
    if( (mode&SearchPanel::KEYWORD ) && entry->matchKeyword( selection_string ) ) accept = true;
    if( (mode&SearchPanel::TEXT ) && entry->matchText( selection_string ) ) accept = true;
    if( (mode&SearchPanel::ATTACHMENT ) && entry->matchAttachment( selection_string ) ) accept = true;
    if( color_valid && entry->matchColor( selection_string ) ) accept = true;

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

  // if EditFrame current entry is visible, select it;
  if( selected_entry && selected_entry->isSelected() ) selectEntry( selected_entry );
  else if( last_visible_entry ) selectEntry( last_visible_entry );

  ostringstream out;
  out << found << " out of " << total;
  if( found > 1 ) out << " entries selected";
  else out << " entry selected";
  
  statusBar().label().setText( out.str().c_str() );

  return;
}

//_______________________________________________
void SelectionFrame::showAllEntries( void )
{
  Debug::Throw( "SelectionFrame::showAllEntries.\n" );

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


//____________________________________________
void SelectionFrame::enterEvent( QEvent *event )
{
  Debug::Throw( "SelectionFrame::enterEvent.\n" );

  // base class enterEvent
  QWidget::enterEvent( event );
  checkLogbookModified();
  
  return;
}

//____________________________________
void SelectionFrame::closeEvent( QCloseEvent *event )
{
  Debug::Throw( "SelectionFrame::closeEvent.\n" );
  event->accept();    
  static_cast<MainFrame*>(qApp)->closeAction().trigger();
}

//_______________________________________________
void SelectionFrame::_installActions( void )
{
  
  Debug::Throw( "SelectionFrame::_installActions.\n" );
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  uniconify_action_ = new QAction( IconEngine::get( ICONS::HOME, path_list ), "&Main window", this );
  uniconify_action_->setToolTip( "Raise application main window" );
  connect( uniconify_action_, SIGNAL( triggered() ), SLOT( _uniconify() ) );

  new_keyword_action_ = new QAction( IconEngine::get( ICONS::NEW, path_list ), "&New keyword", this );
  new_keyword_action_->setToolTip( "Create a new keyword" );
  connect( new_keyword_action_, SIGNAL( triggered() ), SLOT( _newKeyword() ) );

  edit_keyword_action_ = new QAction( IconEngine::get( ICONS::EDIT, path_list ), "&Rename keyword", this );
  edit_keyword_action_->setToolTip( "Rename selected keyword" );
  connect( edit_keyword_action_, SIGNAL( triggered() ), SLOT( _renameKeyword() ) );
  
  /*
  delete keyword action
  it is associated to the Key_Delete shortcut
  but the later is enabled only if the KeywordList has focus.
  */
  delete_keyword_action_ = new QAction( IconEngine::get( ICONS::DELETE, path_list ), "&Delete keyword", this );
  delete_keyword_action_->setToolTip( "Delete selected keyword" );
  delete_keyword_action_->setShortcut( Key_Delete );
  delete_keyword_action_->setShortcutContext( WidgetShortcut );  
  connect( delete_keyword_action_, SIGNAL( triggered() ), SLOT( _deleteKeyword() ) );

  new_entry_action_ = new QAction( IconEngine::get( ICONS::NEW, path_list ), "&New entry", this );
  new_entry_action_->setToolTip( "Create a new entry" );
  connect( new_entry_action_, SIGNAL( triggered() ), SLOT( _newEntry() ) );
  
  edit_entry_action_ = new QAction( IconEngine::get( ICONS::EDIT, path_list ), "&Edit selected entries", this );
  edit_entry_action_->setToolTip( "Edit selected entries" );
  connect( edit_entry_action_, SIGNAL( triggered() ), SLOT( _editEntries() ) );

  /*
  delete entry action
  it is associated to the Key_Delete shortcut
  but the later is enabled only if the KeywordList has focus.
  */
  delete_entry_action_ = new QAction( IconEngine::get( ICONS::DELETE, path_list ), "&Delete selected entries", this );
  delete_entry_action_->setToolTip( "Delete selected entries" );
  delete_entry_action_->setShortcut( Key_Delete );
  delete_entry_action_->setShortcutContext( WidgetShortcut );  
  connect( delete_entry_action_, SIGNAL( triggered() ), SLOT( _deleteEntries() ) );

  // color menu
  color_menu_ = new ColorMenu( this );
  color_menu_->setTitle( "&Change entry color" );
  connect( color_menu_, SIGNAL( selected( QColor ) ), SLOT( _changeEntryColor( QColor ) ) );

  entry_color_action_ = new QAction( IconEngine::get( ICONS::COLOR, path_list ), "&Entry color", this );
  entry_color_action_->setToolTip( "Change selected entries color" );
  entry_color_action_->setMenu( color_menu_ );
  
  entry_keyword_action_ = new QAction( IconEngine::get( ICONS::EDIT, path_list ), "&Change keyword", this );
  entry_keyword_action_->setToolTip( "Edit selected entries keyword" );
  connect( entry_keyword_action_, SIGNAL( triggered() ), SLOT( _renameEntryKeyword() ) );

  new_logbook_action_ = new QAction( IconEngine::get( ICONS::NEW, path_list ), "&New", this );
  new_logbook_action_->setToolTip( "Create a new logbook" );
  new_logbook_action_->setShortcut( CTRL+Key_N );
  connect( new_logbook_action_, SIGNAL( triggered() ), SLOT( _newLogbook() ) );

  open_action_ = new QAction( IconEngine::get( ICONS::OPEN, path_list ), "&Open", this );
  open_action_->setToolTip( "Open an existsing logbook" );
  open_action_->setShortcut( CTRL+Key_O );
  connect( open_action_, SIGNAL( triggered() ), SLOT( open() ) );

  synchronize_action_ = new QAction( "&Synchronize", this );
  synchronize_action_->setToolTip( "Synchronize current logbook with remote" );
  connect( synchronize_action_, SIGNAL( triggered() ), SLOT( _synchronize() ) );

  reorganize_action_ = new QAction( "&Reorganize", this );
  reorganize_action_->setToolTip( "Reoganize logbook entries in files" );
  connect( reorganize_action_, SIGNAL( triggered() ), SLOT( _reorganize() ) );

  save_action_ = new QAction( IconEngine::get( ICONS::SAVE, path_list ), "&Save", this );
  save_action_->setToolTip( "Save all edited entries" );
  connect( save_action_, SIGNAL( triggered() ), SLOT( save() ) );

  save_forced_action_ = new QAction( IconEngine::get( ICONS::SAVE, path_list ), "&Save (forced)", this );
  save_forced_action_->setToolTip( "Save all entries" );
  connect( save_forced_action_, SIGNAL( triggered() ), SLOT( _saveForced() ) );

  save_as_action_ = new QAction( IconEngine::get( ICONS::SAVE_AS, path_list ), "Save &As", this );
  save_as_action_->setToolTip( "Save logbook with a different name" );
  connect( save_as_action_, SIGNAL( triggered() ), SLOT( _saveAs() ) );

  save_backup_action_ = new QAction( "Save &Backup", this );
  save_backup_action_->setToolTip( "Save logbook backup" );
  connect( save_backup_action_, SIGNAL( triggered() ), SLOT( _saveBackup() ) );

  revert_to_save_action_ = new QAction( IconEngine::get( ICONS::RELOAD, path_list ), "&Revert to Saved", this );
  revert_to_save_action_->setToolTip( "Restore saved logbook" );
  connect( revert_to_save_action_, SIGNAL( triggered() ), SLOT( _revertToSaved() ) );

  view_html_action_ = new QAction( IconEngine::get( ICONS::HTML, path_list ), "&Html", this );
  view_html_action_->setToolTip( "Convert logbook to html" );
  connect( view_html_action_, SIGNAL( triggered() ), SLOT( _viewHtml() ) );

  logbook_statistics_action_ = new QAction( IconEngine::get( ICONS::INFO, path_list ), "Logbook statistics", this );
  logbook_statistics_action_->setToolTip( "View logbook statistics" );
  connect( logbook_statistics_action_, SIGNAL( triggered() ), SLOT( _viewLogbookStatistics() ) );
  
  logbook_informations_action_ = new QAction( IconEngine::get( ICONS::INFO, path_list ), "Logbook informations", this );
  logbook_informations_action_->setToolTip( "Edit logbook informations" );
  connect( logbook_informations_action_, SIGNAL( triggered() ), SLOT( _editLogbookInformations() ) );

  close_frames_action_ = new QAction( "&Close editors", this );
  close_frames_action_->setToolTip( "Close all entry editors" );
  connect( close_frames_action_, SIGNAL( triggered() ), SLOT( _closeEditFrames() ) );

  show_duplicates_action_ = new QAction( "Show duplicated entries", this );
  show_duplicates_action_->setToolTip( "Show duplicated entries in logbook" );
  connect( show_duplicates_action_, SIGNAL( triggered() ), SLOT( _showDuplicatedEntries() ) );

}

//_______________________________________________
void SelectionFrame::_resetLogEntryList( void )
{
  
  Debug::Throw( "SelectionFrame::_resetLogEntryList.\n" );

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
  
  // loop over associated editframes
  // update navigation buttons
  BASE::KeySet<EditFrame> frames( this );
  for( BASE::KeySet<EditFrame>::iterator it = frames.begin(); it != frames.end(); it++ )
  {
    
    // skip frames that are about to be deleted
    if( (*it)->isClosed() ) continue;
    
    // get associated entry and see if selected
    LogEntry* entry( (*it)->entry() );
    (*it)->previousEntryAction().setEnabled( entry && entry->isSelected() && previousEntry(entry, false) );
    (*it)->nextEntryAction().setEnabled( entry && entry->isSelected() && nextEntry(entry, false) );
    
  }

  return;
  
}

//_______________________________________________
void SelectionFrame::_resetKeywordList( void )
{
  
  Debug::Throw( "SelectionFrame::_resetKeywordList.\n" );
  assert( logbook() );
      
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
  
  _keywordModel().set( KeywordModel::List( new_keywords.begin(), new_keywords.end() ) );

}

//_______________________________________________
void SelectionFrame::_loadColors( void )
{
  
  Debug::Throw( "SelectionFrame::_loadColors.\n" );
  
  if( !logbook_ ) return;
  
  //! retrieve all entries
  BASE::KeySet<LogEntry> entries( logbook()->entries() );
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  { color_menu_->add( (*iter)->color() ); }

}


//_______________________________________________
void SelectionFrame::_updateConfiguration( void )
{
  
  Debug::Throw( "SelectionFrame::_updateConfiguration.\n" );
    
  // autoSave
  autosave_timer_.setInterval( 1000*XmlOptions::get().get<int>( "AUTO_SAVE_ITV" ) );
  bool autosave( XmlOptions::get().get<bool>( "AUTO_SAVE" ) );
  if( autosave ) autosave_timer_.start();
  else autosave_timer_.stop();
    
  // resize
  resize( XmlOptions::get().get<int>("SELECTION_FRAME_WIDTH"), XmlOptions::get().get<int>("SELECTION_FRAME_HEIGHT") );
  
  QList<int> sizes;
  sizes.push_back( XmlOptions::get().get<int>( "KEYWORD_LIST_WIDTH" ) );
  sizes.push_back( XmlOptions::get().get<int>( "ENTRY_LIST_WIDTH" ) );
  splitter_->setSizes( sizes );
  
  // entry list mask
  if( XmlOptions::get().find( "ENTRY_LIST_MASK" ) ) logEntryList().setMask( XmlOptions::get().get<unsigned int>( "ENTRY_LIST_MASK" ) );
  
  // colors
  list<string> color_list( XmlOptions::get().specialOptions<string>( "COLOR" ) );
  for( list<string>::iterator iter = color_list.begin(); iter != color_list.end(); iter++ )
  { color_menu_->add( *iter ); }
  
}

//_______________________________________________
void SelectionFrame::_saveConfiguration( void )
{
  
  Debug::Throw( "SelectionFrame::_saveConfiguration.\n" );
  
  // sizes
  XmlOptions::get().set<int>( "SELECTION_FRAME_WIDTH", width() );
  XmlOptions::get().set<int>( "SELECTION_FRAME_HEIGHT", height() );
  XmlOptions::get().set<int>( "KEYWORD_LIST_WIDTH", keywordList().width() );
  XmlOptions::get().set<int>( "ENTRY_LIST_WIDTH", logEntryList().width() );
  
  // entry list mask
  XmlOptions::get().set<unsigned int>( "ENTRY_LIST_MASK", logEntryList().mask() );
  
}

//_______________________________________________
void SelectionFrame::_newLogbook( void )
{
  Debug::Throw( "SelectionFrame::_newLogbook.\n" );

  // check current logbook
  if( logbook_ && logbook()->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

  // new logbook
  NewLogbookDialog dialog( this );
  dialog.setTitle( Logbook::LOGBOOK_NO_TITLE );
  dialog.setAuthor( XmlOptions::get().raw( "USER" ) );

  // filename and directory
  File file = File( "log.xml" ).addPath( workingDirectory() );
  dialog.setFile( file );
  dialog.setAttachmentDirectory( workingDirectory() );

  // map dialog
  Debug::Throw( "SelectionFrame::newLogbook - dialog created.\n" );
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  Debug::Throw() << "SelectionFrame::new - file: " << dialog.file() << endl;
  
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

    ostringstream o;
    o << "File \"" << directory << "\" is not a directory.";
    QtUtil::infoDialog( this, o.str() );

  } else logbook()->setDirectory( directory );

  // add new file to openPreviousMenu
  if( !logbook()->file().empty() )
  { menu().openPreviousMenu().add( logbook()->file() ); }

}

//_______________________________________________
void SelectionFrame::open( FileRecord record )
{
  
  Debug::Throw( "SelectionFrame::open.\n" );

  // check if current logbook needs save
  if( logbook_ && logbook()->modified()  && askForSave() == AskForSaveDialog::CANCEL ) return;

  // open file from dialog if not set as argument
  if( record.file().empty() )
  {
    
    // create file dialog
    CustomFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setDirectory( workingDirectory().c_str() );

    QtUtil::centerOnParent( &dialog );
    if( dialog.exec() == QDialog::Rejected ) return;

    QStringList files( dialog.selectedFiles() );
    if( files.empty() ) return;
    record = FileRecord( File( qPrintable( files.front() ) ) );
    
  }

  // create logbook from file
  static_cast<MainFrame*>(qApp)->busy();
  setLogbook( record.file() );
  static_cast<MainFrame*>(qApp)->idle();

  // check if backup is needed
  checkLogbookBackup();
  
  return;
}

//_______________________________________________
bool SelectionFrame::_saveAs( File default_file )
{
  Debug::Throw( "SelectionFrame::_saveAs.\n");

  // check current logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no logbook opened. <Save Logbook> canceled." );
    return false;
  }

  // check default filename
  if( default_file.empty() ) default_file = logbook()->file();
  if( default_file.empty() ) default_file = File( "log.xml" ).addPath( workingDirectory() );

  // create file dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::AnyFile );
  dialog.setDirectory( QDir( default_file.path().c_str() ) );
  dialog.selectFile( default_file.localName().c_str() );
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return false;

  // retrieve files
  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return false;
  
  // retrieve filename
  File fullname = File( qPrintable( files.back() ) ).expand();

  // update working directory
  working_directory_ = fullname.path();

  // check if file exists
  if(
    fullname.exists() &&
    !QtUtil::questionDialog( this, "selected file already exists. Overwrite ?" ) )
  return false;

  // change logbook filename and save
  logbook()->setFile( fullname );
  logbook()->setModifiedRecursive( true );
  save();

  /*
    force logbook state to unmodified since
    some children state may not have been reset properly
  */
  logbook()->setModifiedRecursive( false );

  // add new file to openPreviousMenu
  menu().openPreviousMenu().add( logbook()->file() );

  // reset ignore_warning flag
  ignore_warnings_ = false;

  return true;
}


//_____________________________________________
void SelectionFrame::_saveForced( void )
{
  Debug::Throw( "SelectionFrame::_saveForced.\n" );

  // retrieve/check SelectionFrame/Logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no Logbook opened. <Save> canceled." );
    return;
  }

  // set all logbooks as modified
  logbook()->setModifiedRecursive( true );
  save();

}

//_______________________________________________
void SelectionFrame::_saveBackup( void )
{
  Debug::Throw( "SelectionFrame::_saveBackup.\n");

  // check current logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no logbook opened. <Save Backup> canceled." );
    return;
  }

  string filename( logbook()->backupFilename( ) );
  if( filename.empty() ) {
    QtUtil::infoDialog( this, "no valid filename. Use <Save As> first." );
    return;
  }

  // store last backup time and update
  TimeStamp last_backup( logbook()->backup() );

  // stores current logbook filename
  string current_filename( logbook()->file() );

  // save logbook as backup
  bool saved( _saveAs( filename ) );

  // remove the "backup" filename from the openPrevious list
  // to avoid confusion
  menu().openPreviousMenu().remove( filename );

  // restore initial filename
  logbook()->setFile( current_filename );

  if( saved ) {

    logbook()->setBackup( TimeStamp::now() );
    logbook()->setModified( true );
    setWindowTitle( MainFrame::MAIN_TITLE_MODIFIED );

    // Save logbook if needed (to make sure the backup stamp is updated)
    if( !logbook()->file().empty() ) save();
  }

}

//_____________________________________________
void SelectionFrame::_revertToSaved( void )
{
  Debug::Throw( "SelectionFrame::_revertToSaved.\n" );

  // check logbook
  if( !logbook_ ){
    QtUtil::infoDialog( this, "No logbook opened. <Revert to save> canceled." );
    return;
  }

  // ask for confirmation
  if(
    logbook()->modified() &&
    !QtUtil::questionDialog( this, "discard changes to current logbook ?" )
  ) return;

  // reinit SelectionFrame
  static_cast<MainFrame*>(qApp)->busy();
  string file( logbook()->file() );
  setLogbook( logbook()->file() );
  static_cast<MainFrame*>(qApp)->idle();

  checkLogbookBackup();
  ignore_warnings_ = false;
  
}

//_______________________________________________
void SelectionFrame::_synchronize( void )
{
  Debug::Throw( "SelectionFrame::_synchronize.\n" );

  // check current logbook is valid
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "No logbook opened. <Merge> canceled." );
    return;
  }

  // save EditFrames
  BASE::KeySet<EditFrame> frames( this );
  for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  if( !((*iter)->isReadOnly() || (*iter)->isClosed() ) && (*iter)->modified() && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return;

  // save current logbook
  if( logbook()->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

  // create file dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::ExistingFile );

  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() != QDialog::Accepted ) return;

  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return;

  // debug
  Debug::Throw() << "SelectionFrame::_synchronize - number of local files: " << SelectionFrame::logbook()->children().size() << endl;
  Debug::Throw() << "SelectionFrame::_synchronize - number of local entries: " << SelectionFrame::logbook()->entries().size() << endl;
  
  // set busy flag
  static_cast<MainFrame*>(qApp)->busy();
  statusBar().label().setText( "reading remote logbook ... " );
  
  // opens file in remote logbook
  File remote_file( qPrintable( files.front() ) );
  Debug::Throw() << "SelectionFrame::_synchronize - reading remote logbook from file: " << remote_file << endl;
  
  Logbook remote_logbook;
  connect( &remote_logbook, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );
  remote_logbook.setFile( remote_file );
  remote_logbook.read();
  
  // check if logbook is valid
  XmlError::List errors( remote_logbook.xmlErrors() );
  if( errors.size() ) 
  {

    ostringstream what;
    if( errors.size() > 1 ) what << "Errors occured while parsing files." << endl;
    else what << "An error occured while parsing files." << endl;
    what << errors;
    QtUtil::infoDialog( 0, what.str().c_str() );

    static_cast<MainFrame*>(qApp)->idle();
    return;

  }

  // debug
  Debug::Throw() << "SelectionFrame::_synchronize - number of remote files: " << remote_logbook.children().size() << endl;
  Debug::Throw() << "SelectionFrame::_synchronize - number of remote entries: " << remote_logbook.entries().size() << endl;
  Debug::Throw() << "SelectionFrame::_synchronize - updating local from remote" << endl;

  // synchronize local with remote
  // retrieve map of duplicated entries
  std::map<LogEntry*,LogEntry*> duplicates( SelectionFrame::logbook()->synchronize( remote_logbook ) );
  Debug::Throw() << "SelectionFrame::_synchronize - number of duplicated entries: " << duplicates.size() << endl;

  // update possible EditFrames when duplicated entries are found
  // delete the local duplicated entries
  for( std::map<LogEntry*,LogEntry*>::iterator iter = duplicates.begin(); iter != duplicates.end(); iter++ )
  {
    
    // display the new entry in all matching edit frames
    BASE::KeySet<EditFrame> frames( iter->first );
    for( BASE::KeySet<EditFrame>::iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
    { (*frame_iter)->displayEntry( iter->second ); }

    delete iter->first;

  }

  // reinitialize lists
  _resetKeywordList();
  _resetLogEntryList();
  resetAttachmentFrame();

  // retrieve last modified entry
  BASE::KeySet<LogEntry> entries( SelectionFrame::logbook()->entries() );
  BASE::KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
  selectEntry( *iter );
  logEntryList().setFocus();

  // write local logbook
  if( !SelectionFrame::logbook()->file().empty() ) save();
  
  // synchronize remove with local
  Debug::Throw() << "SelectionFrame::_synchronize - updating remote from local" << endl;
  unsigned int n_duplicated = remote_logbook.synchronize( *SelectionFrame::logbook() ).size();
  Debug::Throw() << "SelectionFrame::_synchronize - number of duplicated entries: " << n_duplicated << endl;

  // save remote logbook
  statusBar().label().setText( "saving remote logbook ... " );
  remote_logbook.write();

  // idle
  static_cast<MainFrame*>(qApp)->idle();
  statusBar().label().setText( "" );

  return;

}

//_______________________________________________
void SelectionFrame::_reorganize( void )
{
  Debug::Throw( "SelectionFrame::_reorganize.\n" );

  if( !logbook_ )
  {
    QtUtil::infoDialog( this,"No valid logbook. Canceled.\n");
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
    Logbook *logbook( SelectionFrame::logbook()->latestChild() );
    Key::associate( *iter, logbook );
    logbook->setModified( true );
  }
  
  // remove empty logbooks
  logbook()->removeEmptyChildren();
  
  // save
  logbook()->setModified( true );
  if( !logbook()->file().empty() ) save();

}

//_______________________________________________
void SelectionFrame::_showDuplicatedEntries( void )
{
  Debug::Throw( "SelectionFrame::_showDuplicatedEntries.\n" );

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

  if( !found ) {
    QtUtil::infoDialog(
      this,
      "No matching entry found.\n"
      "Request canceled.", QtUtil::CENTER_ON_PARENT );

    // reset flag for the turned off entries to true
    for( BASE::KeySet<LogEntry>::iterator it=turned_off_entries.begin(); it!= turned_off_entries.end(); it++ )
    (*it)->setFindSelected( true );

    return;
  }

  // reinitialize logEntry list
  _resetKeywordList();
  _resetLogEntryList();

  // if EditFrame current entry is visible, select it;
  if( selected_entry && selected_entry->isSelected() ) selectEntry( selected_entry );
  else if( last_visible_entry ) selectEntry( last_visible_entry );

  return;
}

//_______________________________________________
void SelectionFrame::_viewLogbookStatistics( void )
{
  Debug::Throw( "SelectionFrame::_viewLogbookStatistics.\n" );
  
  if( !logbook_ ) 
  {
    QtUtil::infoDialog( this, "No logbook opened." );
    return;
  }

  // create dialog
  LogbookStatisticsDialog dialog( this, logbook_ );
  QtUtil::centerOnWidget( &dialog, qApp->activeWindow() );
  dialog.exec();
}

//_______________________________________________
void SelectionFrame::_editLogbookInformations( void )
{
  Debug::Throw( "SelectionFrame::_editLogbookInformations.\n" );
  
  if( !logbook_ ) 
  {
    QtUtil::infoDialog( this, "No logbook opened." );
    return;
  }

  // create dialog
  LogbookInformationDialog dialog( this, logbook_ );
  QtUtil::centerOnWidget( &dialog, qApp->activeWindow() );
  if( dialog.exec() == QDialog::Rejected ) return;

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

    ostringstream o;
    o << "File \"" << directory << "\" is not a directory.";
    QtUtil::infoDialog( this, o.str() );

  } else modified |= logbook()->setDirectory( directory );


  // save Logbook, if needed
  if( modified ) logbook()->setModified( true );
  if( !logbook()->file().empty() ) save();

}

//_______________________________________________
void SelectionFrame::_closeEditFrames( void ) const
{
  Debug::Throw( "SelectionFrame::_closeEditFrames.\n" );

  // get all EditFrames from SelectionFrame
  BASE::KeySet<EditFrame> frames( this );
  for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  {
    if( (*iter)->modified() && !( (*iter)->isReadOnly() || (*iter)->isClosed() ) && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return;
    delete *iter;
  }
  return;
}

//____________________________________________
void SelectionFrame::_newEntry( void )
{
  
  Debug::Throw( "SelectionFrame::_NewEntry.\n" );

  // create new EditFrame
  EditFrame *frame = new EditFrame( 0, false );
  Key::associate( this, frame );

  // call NewEntry for the selected frame
  frame->newEntry();
  frame->show();

}

//____________________________________________
void SelectionFrame::_editEntries( void )
{
  Debug::Throw( "SelectionFrame::_EditEntries .\n" );

  // retrieve selected items; make sure they do not include the navigator
  LogEntryModel::List selection( _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );
  if( selection.empty() ) {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

  // retrieve associated entry
  for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { _displayEntry( *iter ); }

  return;

}

//____________________________________________
void SelectionFrame::_deleteEntries( void )
{
  Debug::Throw( "SelectionFrame::_DeleteEntries .\n" );

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
      QtUtil::infoDialog( this, "Cannot delete entry that is being edited." ); 
    } else selection.push_back( _logEntryModel().get( *iter ) ); 
  }
  
  // check selection size
  if( selection.empty() && !has_edited_index ) 
  {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

  // ask confirmation
  ostringstream what;
  what << "Delete selected entr" << ( selection.size() == 1 ? "y":"ies" );
  if( !QtUtil::questionDialog( this, what.str() ) ) return;

  // retrieve associated entry
  for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { deleteEntry( *iter, false ); }

  // Save logbook if needed
  if( !logbook()->file().empty() ) save();

  return;

}

//_______________________________________________
void SelectionFrame::_displayEntry( LogEntry* entry )
{

  Debug::Throw( "SelectionFrame::_displayEntry.\n" );

  // retrieve associated EditFrames, check if one matches the selected entry
  EditFrame *edit_frame( 0 );
  BASE::KeySet<EditFrame> frames( this );
  for( BASE::KeySet<EditFrame>::iterator iter=frames.begin(); iter != frames.end(); iter++ )
  {
    
    /*
      if Editframe is to be deleted, delete it. This is to avoid memory leak
      for EditFrames which should have been deleted before but could not to avoid crash
    */
    if( (*iter)->isClosed() )
    {
      delete *iter;
      continue;
    }
    
    //! check if EditFrame is editable and match editor
    if( !((*iter)->isReadOnly() ) && (*iter)->entry() == entry ) 
    {
      edit_frame = *iter;
      break;
    }

  }

  // create editFrame if not found
  if( !edit_frame )
  {
    edit_frame = new EditFrame( 0, false );
    Key::associate( this, edit_frame );
    QtUtil::centerOnParent( edit_frame );
    edit_frame->displayEntry( entry );
    edit_frame->show();

  } else edit_frame->uniconify();
  
  Debug::Throw( "SelectionFrame::_displayEntry - done.\n" );

}

//_______________________________________________
void SelectionFrame::_changeEntryTitle( LogEntry* entry, string new_title )
{
  Debug::Throw( "SelectionFrame::_changeEntryTitle.\n" );
  
  // make sure that title was changed
  if( new_title == entry->title() ) return;
  
  // update entry title
  entry->setTitle( new_title );
  
  // update associated EditFrames
  BASE::KeySet<EditFrame> frames( entry );
  for( BASE::KeySet< EditFrame >::iterator it = frames.begin(); it != frames.end(); it++ )
  {
    
    // keep track of already modified EditFrames
    bool frame_modified( (*it)->modified() && !(*it)->isReadOnly() );
    
    // update EditFrame
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
  if( logbook() && !logbook()->file().empty() ) save();
 
}

//_______________________________________________
void SelectionFrame::_changeEntryColor( QColor color )
{
  Debug::Throw( "SelectionFrame::_changeEntryColor.\n" );

  // retrieve current selection
  LogEntryModel::List selection( _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );
  if( selection.empty() ) 
  {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

  // retrieve associated entry
  for( LogEntryModel::List::iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    
    // get associated entry
    LogEntry* entry( *iter );

    entry->setColor( color.isValid() ? qPrintable( color.name() ):ColorMenu::NONE );
    entry->setModification( entry->modification()+1 );
        
    // update EditFrame color
    BASE::KeySet<EditFrame> frames( entry );
    for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    { if( !(*iter)->isClosed() ) (*iter)->displayColor(); }

    // set logbooks as modified
    BASE::KeySet<Logbook> logbooks( entry );
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
    { (*iter)->setModified( true ); }

  }

  // update model
  _logEntryModel().add( selection );
  
  // save Logbook
  if( !logbook()->file().empty() ) save();

}

//____________________________________________
void SelectionFrame::_newKeyword( void )
{
  
  Debug::Throw( "SelectionFrame::_newKeyword.\n" );
  
  //! create dialog
  EditKeywordDialog dialog( this );
  dialog.setWindowTitle( "New keyword" );

  KeywordModel::List keywords( _keywordModel().values() );
  for( KeywordModel::List::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  dialog.add( *iter );
  dialog.setKeyword( currentKeyword() );
  
  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

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
void SelectionFrame::_deleteKeyword( void )
{
  Debug::Throw("SelectionFrame::_deleteKeyword.\n" );
  
  //! check that keywordlist has selected item
  QModelIndexList selected_indexes( keywordList().selectionModel()->selectedRows() );
  if( selected_indexes.empty() )
  {
    QtUtil::infoDialog( this, "no keyword selected. Request canceled" );
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
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;
  
  if( dialog.moveEntries() && associated_entries.size() ) 
  {

    Debug::Throw( "SelectionFrame::_deleteKeyword - moving entries.\n" );
    for( KeywordModel::List::iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
    { _renameKeyword( *iter, iter->parent(), false );  }

  } else if( dialog.deleteEntries() ) {
    
    Debug::Throw( "SelectionFrame::_deleteKeyword - deleting entries.\n" );
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
  if( !logbook()->file().empty() ) save();  
  
  return;
  
}

//____________________________________________
void SelectionFrame::_renameKeyword( void )
{
  Debug::Throw("SelectionFrame::_renameKeyword.\n" );
  
  //! check that keywordlist has selected item
  if( !keywordList().selectionModel()->currentIndex().isValid() )
  {
    QtUtil::infoDialog( this, "no keyword selected. Request canceled" );
    return;
  }

  // get current selected keyword
  Keyword keyword( _keywordModel().get( keywordList().selectionModel()->currentIndex() ) );
      
  //! create dialog
  EditKeywordDialog dialog( this );
  dialog.setWindowTitle( "Edit keyword" );

  const KeywordModel::List& keywords( _keywordModel().values() );
  for( KeywordModel::List::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  { dialog.add( *iter ); }
  dialog.setKeyword( keyword );
  
  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // change keyword for all entries that match the old one
  _renameKeyword( keyword, dialog.keyword() );
  
}

//____________________________________________
void SelectionFrame::_renameKeyword( Keyword keyword, Keyword new_keyword, bool update_selection )
{

  Debug::Throw("SelectionFrame::_renameKeyword.\n" );
  
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
  if( !logbook()->file().empty() ) save();     
  
}
 
//____________________________________________
void SelectionFrame::_renameEntryKeyword( void )
{
  Debug::Throw("SelectionFrame::_renameEntryKeyword.\n" );
  
  // retrieve current selection
  LogEntryModel::List selection( _logEntryModel().get( logEntryList().selectionModel()->selectedRows() ) );
  if( selection.empty() ) 
  {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }
        
  //! check that current keyword make sense
  if( !keywordList().selectionModel()->currentIndex().isValid() )
  {
    QtUtil::infoDialog( this, "no keyword selected. Request canceled" );
    return;
  }
  
  // get current selected keyword
  Keyword keyword( _keywordModel().get( keywordList().selectionModel()->currentIndex() ) );
  
  //! create dialog
  EditKeywordDialog dialog( this );
  dialog.setWindowTitle( "Edit keyword" );

  const KeywordModel::List& keywords( _keywordModel().values() );
  for( KeywordModel::List::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  dialog.add( *iter );
  dialog.setKeyword( keyword );
  
  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // check if keyword was changed
  Keyword new_keyword( dialog.keyword() );
  if( keyword == new_keyword ) return;
  
  // change keyword for all entries that match the old one
  _renameEntryKeyword( new_keyword );
  
}

//_______________________________________________
void SelectionFrame::_renameEntryKeyword( Keyword new_keyword, bool update_selection )
{
      
  Debug::Throw() << "SelectionFrame::_renameEntryKeyword - new_keyword: " << new_keyword << endl;
  
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
  if( !logbook()->file().empty() ) save();
  
  return;    
  
}

//_______________________________________________
void SelectionFrame::_keywordSelectionChanged( const QModelIndex& index )
{

  Debug::Throw( "SelectionFrame::_keywordSelectionChanged.\n" );
  if( !logbook_ ) return; 
  if( !index.isValid() ) return;
  
  Keyword keyword( _keywordModel().get( index ) );
  Debug::Throw() << "SelectionFrame::_keywordSelectionChanged - keyword: " << keyword << endl;
      
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
      Debug::Throw() << "SelectionFrame::_keywordSelectionChanged - found entry: " << entry->key() << endl;
    } else entry->setKeywordSelected( false );
    
  }
  
  // reinitialize logEntry list
  _resetLogEntryList();
  
  // if EditFrame current entry is visible, select it;
  if( selected_entry && selected_entry->isSelected() ) selectEntry( selected_entry );
  else if( last_visible_entry ) selectEntry( last_visible_entry );
  
  return;
} 

//_____________________________________________
void SelectionFrame::_updateKeywordActions( void )
{
  Debug::Throw( "SelectionFrame::_updateKeywordActions.\n" );
  bool has_selection( !keywordList().selectionModel()->selectedRows().empty() );  
  editKeywordAction().setEnabled( has_selection );
  deleteKeywordAction().setEnabled( has_selection );
  return;
}

//_____________________________________________
void SelectionFrame::_updateEntryActions( void )
{
  Debug::Throw( "SelectionFrame::_updateEntryActions.\n" );
  bool has_selection( !logEntryList().selectionModel()->selectedRows().empty() );
  editEntryAction().setEnabled( has_selection );
  deleteEntryAction().setEnabled( has_selection );
  entryColorAction().setEnabled( has_selection );
  entryKeywordAction().setEnabled( has_selection );
  return;
}

//_____________________________________________
void SelectionFrame::_viewHtml( void )
{
  Debug::Throw( "SelectionFrame::_viewHtml.\n" );

  // check logbook
  if( !logbook_ )
  {
    QtUtil::infoDialog( this, "No logbook opened. <View HTML> canceled." );
    return;
  }

  // create custom dialog, retrieve check vbox child
  ViewHtmlLogbookDialog dialog( this );
  dialog.setSelection( ViewHtmlLogbookDialog::ALL );
  dialog.setLogbookMask( Logbook::HTML_ALL_MASK );
  dialog.setEntryMask( LogEntry::HTML_ALL_MASK );
  dialog.setCommand(
    ( AttachmentType::HTML.editCommand().size() ) ?
    AttachmentType::HTML.editCommand():"" );

  ostringstream what;
  what << "_eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
  dialog.setFile( File( what.str() ).addPath( Util::tmp() ) );

  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve/check file
  File file( dialog.file() );
  if( file.empty() ) 
  {
    QtUtil::infoDialog(this, "No output file specified. <View HTML> canceled." );
    return;
  }

  // open/check temp file
  QFile out( file.c_str() );
  if( !out.open( QIODevice::WriteOnly ) ) {
    ostringstream o;
    o << "Cannot write to file \"" << file << "\". <View HTML> canceled.";
    QtUtil::infoDialog( this, o.str() );
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
  HtmlUtil::header( html, document );
  
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

  out.write( document.toByteArray() );
  out.close();

  // retrieve command
  string command( dialog.command() );
  if( command.empty() ) return;

  // execute command
  Util::run( QStringList() << command.c_str() << file.c_str() );
  return;
}

//_______________________________________________
void SelectionFrame::_storeSortMethod( int column, Qt::SortOrder order  )
{
  
  Debug::Throw( "SelectionFrame::_storeSortMethod.\n");
  if( !logbook_ ) return;

  switch( column ) {
    
    case LogEntryModel::TITLE: logbook()->setSortMethod( Logbook::SORT_TITLE ); break;
    case LogEntryModel::CREATION: logbook()->setSortMethod( Logbook::SORT_CREATION ); break;
    case LogEntryModel::MODIFICATION: logbook()->setSortMethod( Logbook::SORT_MODIFICATION ); break;
    case LogEntryModel::AUTHOR: logbook()->setSortMethod( Logbook::SORT_AUTHOR ); break;
    default: return;
    
  }

  // Save logbook if needed
  logbook()->setSortOrder( int( order ) );
  if( !logbook()->file().empty() ) save();

}


//____________________________________________________________
void SelectionFrame::_entryItemActivated( const QModelIndex& index )
{ 
  // stop edition timer
  _logEntryModel().setEditionIndex( QModelIndex() );
  edition_timer_.stop();
  if( index.isValid() ) _displayEntry( _logEntryModel().get( index ) ); 
}

//____________________________________________________________
void SelectionFrame::_entryItemClicked( const QModelIndex& index )
{ 
  
  // do nothing if index do not correspond to an entry title
  if( !(index.isValid() && index.column() == LogEntryModel::TITLE ) ) return;
  
  // do nothing if index is not already selected
  if( !logEntryList().selectionModel()->isSelected( index ) ) return;

  // compare to model edition index
  if( index == _logEntryModel().editionIndex() ) edition_timer_.start( edition_delay_ );
  else _logEntryModel().setEditionIndex( index );
  
}

//_______________________________________________
void SelectionFrame::_entryDataChanged( const QModelIndex& index )
{
  Debug::Throw( "SelectionFrame::_entryDataChanged.\n" );
  
  if( !( index.isValid() && index.column() == LogEntryModel::TITLE ) ) return;
  LogEntry* entry( _logEntryModel().get( index ) );
    
  // update associated EditFrames
  BASE::KeySet<EditFrame> frames( entry );
  for( BASE::KeySet< EditFrame >::iterator it = frames.begin(); it != frames.end(); it++ )
  {
    
    // keep track of already modified EditFrames
    bool frame_modified( (*it)->modified() && !(*it)->isReadOnly() );
    
    // update EditFrame
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
  if( logbook() && !logbook()->file().empty() ) save();
 
}

//________________________________________
void SelectionFrame::_startEntryEdition( void )
{   

  Debug::Throw( "SelectionFrame::_startEntryEdition\n" );
  QModelIndex index( logEntryList().currentIndex() );
  if( !( index.isValid() && index == _logEntryModel().editionIndex() ) ) return;

  // enable model edition
  _logEntryModel().setEditionEnabled( true );
  
  // edit item
  logEntryList().edit( index );
  
}

//________________________________________
void SelectionFrame::_storeSelectedEntries( void )
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
void SelectionFrame::_restoreSelectedEntries( void )
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
void SelectionFrame::_storeSelectedKeywords( void )
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
void SelectionFrame::_restoreSelectedKeywords( void )
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
void SelectionFrame::_storeExpandedKeywords( void )
{   
  
  Debug::Throw( "SelectionFrame::_storeExpandedKeywords.\n" );
  // clear
  _keywordModel().clearExpandedIndexes();
  
  // retrieve expanded indexes in list
  QModelIndexList indexes( _keywordModel().indexes() );
  for( QModelIndexList::iterator iter = indexes.begin(); iter != indexes.end(); iter++ )
  { if( keywordList().isExpanded( *iter ) ) _keywordModel().setIndexExpanded( *iter, true ); }
    
}

//________________________________________
void SelectionFrame::_restoreExpandedKeywords( void )
{
  
  Debug::Throw( "SelectionFrame::_restoreExpandedKeywords.\n" );
  
  QModelIndexList expanded_indexes( _keywordModel().expandedIndexes() );
  keywordList().collapseAll();  
  for( QModelIndexList::const_iterator iter = expanded_indexes.begin(); iter != expanded_indexes.end(); iter++ )
  { keywordList().setExpanded( *iter, true ); }
  
  return;
}

//_______________________________________________
void SelectionFrame::_autoSave( void )
{

  if( logbook_ && !logbook()->file().empty() ) 
  {
  
    statusBar().label().setText( "performing autoSave" );

    // retrieve non read only editors; perform save
    BASE::KeySet<EditFrame> frames( this );
    for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    if( !((*iter)->isReadOnly() || (*iter)->isClosed() ) ) (*iter)->save();

    save();
  
  } else
  statusBar().label().setText( "no logbook filename. <Autosave> skipped" );

}
