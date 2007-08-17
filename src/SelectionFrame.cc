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
  \file SelectionFrame.cc
  \brief base class to display entries and keyword::
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QHeaderView>

#include "AttachmentFrame.h"
#include "BaseIcons.h"
#include "ColorMenu.h"
#include "CustomDialog.h"
#include "CustomLineEdit.h"
#include "CustomPixmap.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "DeleteKeywordDialog.h"
#include "EditFrame.h"
#include "EditKeywordDialog.h"
#include "Exception.h"
#include "HtmlUtil.h"
#include "IconEngine.h"
#include "Icons.h"
#include "Logbook.h"
#include "LogbookInfoDialog.h"
#include "LogbookModifiedDialog.h"
#include "LogbookStatisticsDialog.h"
#include "LogEntryList.h"
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
using namespace BASE;
using namespace Qt;

//_____________________________________________
SelectionFrame::SelectionFrame( QWidget *parent ):
  CustomMainWindow( parent ),
  Counter( "SelectionFrame" ),
  autosave_timer_( new QTimer( this ) ),
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
  statusbar_ = new StatusBar( main );
  statusBar().addLabel( 2 );
  statusBar().addClock();
  connect( this, SIGNAL( messageAvailable( const QString& ) ), &statusBar().label(), SLOT( setTextAndUpdate( const QString& ) ) );  
  layout->addWidget( &statusBar() ); 
  
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
  
  CustomToolBar* toolbar = new CustomToolBar( left );
  v_layout->addWidget( toolbar );
  
  // keyword actions
  QAction* new_keyword_action = new QAction( IconEngine::get( ICONS::NEW, path_list ), "&New keyword", this );
  new_keyword_action->setToolTip( "Create a new keyword" );
  connect( new_keyword_action, SIGNAL( triggered() ), SLOT( _newKeyword() ) );
  toolbar->addWidget( new CustomToolButton( toolbar, new_keyword_action, &statusBar().label() ) );
  
  QAction* edit_keyword_action = new QAction( IconEngine::get( ICONS::EDIT, path_list ), "&Rename keyword", this );
  edit_keyword_action->setToolTip( "Rename selected keyword" );
  connect( edit_keyword_action, SIGNAL( triggered() ), SLOT( _changeEntryKeyword() ) );
  toolbar->addWidget( new CustomToolButton( toolbar, edit_keyword_action, &statusBar().label() ) );
  
  QAction* delete_keyword_action = new QAction( IconEngine::get( ICONS::DELETE, path_list ), "&Delete keyword", this );
  delete_keyword_action->setToolTip( "Delete selected keyword" );
  connect( delete_keyword_action, SIGNAL( triggered() ), SLOT( _deleteKeyword() ) );
  toolbar->addWidget( new CustomToolButton( toolbar, delete_keyword_action, &statusBar().label() ) );

  // create keyword list
  v_layout->addWidget( keyword_list_ = new KeywordList( left ), 1 );
  connect( keyword_list_, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ), SLOT( _keywordSelectionChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ) );  
  connect( keyword_list_, SIGNAL( keywordChanged( std::string ) ), SLOT( _changeEntryKeyword( std::string ) ) );
  connect( keyword_list_, SIGNAL( keywordChanged( std::string, std::string ) ), SLOT( _changeEntryKeyword( std::string, const std::string& ) ) );
    
  // popup menu for keyword list
  keywordList().addMenuAction( new_keyword_action ); 
  keywordList().addMenuAction( edit_keyword_action, true ); 
  keywordList().addMenuAction( delete_keyword_action, true ); 
  
  // right box for entries and buttons
  QWidget* right = new QWidget( splitter_ );
  v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  right->setLayout( v_layout );
    
  toolbar = new CustomToolBar( right );
  v_layout->addWidget( toolbar );

  // entry actions
  QAction* new_entry_action = new QAction( IconEngine::get( ICONS::NEW, path_list ), "&New entry", this );
  new_entry_action->setToolTip( "Create a new entry" );
  connect( new_entry_action, SIGNAL( triggered() ), SLOT( _newEntry() ) );
  toolbar->addWidget( new CustomToolButton( toolbar, new_entry_action, &statusBar().label() ) );

  QAction* edit_entry_action = new QAction( IconEngine::get( ICONS::EDIT, path_list ), "&Edit selected entries", this );
  edit_entry_action->setToolTip( "Edit selected entries" );
  connect( edit_entry_action, SIGNAL( triggered() ), SLOT( _editEntries() ) );
  toolbar->addWidget( new CustomToolButton( toolbar, edit_entry_action, &statusBar().label() ) );

  QAction* delete_entry_action = new QAction( IconEngine::get( ICONS::DELETE, path_list ), "&Delete selected entries", this );
  delete_entry_action->setToolTip( "Delete selected entries" );
  delete_entry_action->setShortcut( Key_Delete );
  connect( delete_entry_action, SIGNAL( triggered() ), SLOT( _deleteEntries() ) );
  toolbar->addWidget( new CustomToolButton( toolbar, delete_entry_action, &statusBar().label() ) );
  
  toolbar->addWidget( new CustomToolButton( toolbar, &saveAction(), &statusBar().label() ) );
  toolbar->addWidget( new CustomToolButton( toolbar, &viewHtmlAction(), &statusBar().label() ) );

  // color menu
  color_menu_ = new ColorMenu( this );
  color_menu_->setTitle( "&Change entry color" );
  connect( color_menu_, SIGNAL( selected( QColor ) ), SLOT( _changeEntryColor( QColor ) ) );
   
  QAction* color_action = new QAction( IconEngine::get( ICONS::COLOR, path_list ), "&Entry color", this );
  color_action->setToolTip( "Change selected entries color" );
  color_action->setMenu( color_menu_ );
  CustomToolButton* button = new CustomToolButton( toolbar, color_action, &statusBar().label() );
  button->setPopupMode( QToolButton::InstantPopup );
  toolbar->addWidget( button );

  edit_keyword_action = new QAction( IconEngine::get( ICONS::EDIT, path_list ), "&Change keyword", this );
  edit_keyword_action->setToolTip( "Edit selected entries keyword" );
  connect( edit_keyword_action, SIGNAL( triggered() ), SLOT( _changeEntryKeyword() ) );
    
  // create logEntry list
  v_layout->addWidget( list_ = new LogEntryList( right ), 1 );
  
  connect( logEntryList().header(), SIGNAL( sectionClicked( int ) ), SLOT( _storeSortMethod( int ) ) );
  connect( list_, SIGNAL( entrySelected( LogEntry* ) ), SLOT( _displayEntry( LogEntry* ) ) );
  connect( list_, SIGNAL( entryRenamed( LogEntry*, std::string ) ), SLOT( _changeEntryTitle( LogEntry*, std::string ) ) );

  // create popup menu for list
  logEntryList().addMenuAction( new_entry_action );
  logEntryList().addMenuAction( edit_entry_action, true ); 
  logEntryList().addMenuAction( edit_keyword_action, true );
  logEntryList().addMenuAction( delete_entry_action, true ); 
  logEntryList().addMenuAction( color_action, true );
  
  // main menu
  menu_ = new Menu( this , this );
  setMenuBar( menu_ );
  
  connect( menu_, SIGNAL( save() ), SLOT( save() ) );
  connect( menu_, SIGNAL( closeWindow() ), qApp, SLOT( exit() ) );
  connect( menu_, SIGNAL( viewHtml() ), this, SLOT( _viewHtml() ) );
 
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( saveConfiguration() ) );
  updateConfiguration();

  // autosave_timer_
  autosave_timer_->setSingleShot( false );
  connect( autosave_timer_, SIGNAL( timeout() ), SLOT( _autoSave() ) );
  
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
    _resetList();
    emit ready();
    return;
  }

  // set file
  logbook()->setFile( file );
  if( !file.exists() )
  {
    // update listView with new entries
    _resetKeywordList();
    _resetList();
    emit ready();
    return;
  }

  connect( logbook_, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );

  logbook()->read();

  // update listView with new entries
  _resetKeywordList();
  _resetList();
  _loadColors();
  
  Debug::Throw( "SelectionFrame::setLogbook - lists set.\n" );
  
  // change sorting
  switch( logbook()->sortingMethod() ) 
  {
    case Logbook::SORT_TITLE: logEntryList().sortItems( LogEntryList::TITLE, AscendingOrder ); break;
    case Logbook::SORT_CREATION: logEntryList().sortItems( LogEntryList::CREATION, AscendingOrder ); break;
    case Logbook::SORT_MODIFICATION: logEntryList().sortItems( LogEntryList::MODIFICATION , AscendingOrder); break;
    case Logbook::SORT_AUTHOR: logEntryList().sortItems( LogEntryList::AUTHOR, AscendingOrder ); break;
    default: break;
  }

  Debug::Throw( "SelectionFrame::setLogbook - lists sorted.\n" );

  // update attachment frame
  resetAttachmentFrame();
  Debug::Throw( "SelectionFrame::setLogbook - attachment frame reset.\n" );

  // retrieve last modified entry
  KeySet<LogEntry> entries( logbook()->entries() );
  KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
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
  keywordList().clear();
  logEntryList().clear();
    
  // clear the AttachmentFrame
  dynamic_cast<MainFrame*>(qApp)->attachmentFrame().list().clear();
  
  // make all EditFrames for deletion
  KeySet<EditFrame> frames( this ); 
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ ) 
  //{delete *iter;}
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
void SelectionFrame::selectEntry( LogEntry* entry )
{
  Debug::Throw( "SelectionFrame::selectEntry.\n" );
  
  if( !entry ) return;
  keywordList().select( entry->keyword() );
  logEntryList().select( entry );
  return;
  
}

//_______________________________________________
void SelectionFrame::updateEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "SelectionFrame::updateEntry.\n" );
  
  // add entry into frame list or update existsing    
  if( entry->keyword() != keywordList().current() )
  {
    keywordList().add( entry->keyword() );
    keywordList().select( entry->keyword() );
  }

  if( !KeySet<LogEntryList::Item>( entry ).empty() ) logEntryList().update( entry, update_selection );
  else {
    logEntryList().add( entry, update_selection );
    logEntryList().sort();
  }
  
  // make sure columns are properly displayed
  logEntryList().sort();
  logEntryList().resizeColumns();

}


//_______________________________________________
void SelectionFrame::deleteEntry( LogEntry* entry, const bool& save )
{
  Debug::Throw( "SelectionFrame::deleteEntry.\n" );
  
  Exception::checkPointer( entry, DESCRIPTION( "invalid entry" ) );

  // get associated attachments
  KeySet<Attachment> attachments( entry );
  for( KeySet<Attachment>::iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  {

    // retrieve/delete associated attachmentlist items
    KeySet<AttachmentList::Item> items( *iter );
    for( KeySet<AttachmentList::Item>::iterator attachment_iter = items.begin(); attachment_iter != items.end(); attachment_iter++ )
    delete *attachment_iter;

    // delete attachment
    delete (*iter);

  };

  // get associated logentrylist items
  KeySet<LogEntryList::Item> items( entry );
  for( KeySet<LogEntryList::Item>::iterator iter = items.begin(); iter != items.end(); iter++ )
  delete *iter;

  /*
    hide associated EditFrames
    they will get deleted next time
    SelectionFrame::_displayEntry() is called
  */
  KeySet<EditFrame> frames( entry );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  { 
    (*iter)->setIsClosed( true );
    (*iter)->hide();
  }
  //{delete *iter;}

  // set logbooks as modified
  KeySet<Logbook> logbooks( entry );
  for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
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
  
  KeySet<EditFrame> frames( entry );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
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
  
  LogEntryList::Item *item( logEntryList().item( entry ) );
  if( !( item && (item = logEntryList().itemAbove( item, update_selection ) ) ) )
  {
    Debug::Throw() << "SelectionFrame::previousEntry - no entry found" << endl;
    return 0;
  }
  
  return item->entry();

}

//_______________________________________________
LogEntry* SelectionFrame::nextEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "SelectionFrame::nextEntry.\n" );
  LogEntryList::Item *item( logEntryList().item( entry ) );
  if( !( item && (item = logEntryList().itemBelow( item, update_selection ) ) ) )
  {
    Debug::Throw() << "SelectionFrame::previousEntry - no entry found" << endl;
    return 0;
  }
  
  return item->entry();

}

//_______________________________________________
void SelectionFrame::resetAttachmentFrame( void ) const
{

  Debug::Throw( "SelectionFrame::resetAttachmentFrame.\n" );

  // clear the AttachmentFrame
  AttachmentFrame &attachment_frame( dynamic_cast<MainFrame*>(qApp)->attachmentFrame() );
  attachment_frame.list().clear();

  // check current logbook
  if( !logbook_ ) return;

  // retrieve logbook attachments, adds to AttachmentFrame
  KeySet<Attachment> attachments( logbook()->attachments() );
  for( KeySet<Attachment>::iterator it = attachments.begin(); it != attachments.end(); it++ )
  { attachment_frame.list().add( *it ); }
  attachment_frame.list().resizeColumns();
  return;

}

//_______________________________________________
void SelectionFrame::updateConfiguration( void )
{
  
  Debug::Throw( "SelectionFrame::updateConfiguration.\n" );
  
  CustomMainWindow::updateConfiguration();
  
  // autoSave
  autosave_timer_->setInterval( 1000*XmlOptions::get().get<int>( "AUTO_SAVE_ITV" ) );
  bool autosave( XmlOptions::get().get<bool>( "AUTO_SAVE" ) );
  if( autosave ) autosave_timer_->start();
  else autosave_timer_->stop();
    
  // resize
  resize( XmlOptions::get().get<int>("SELECTION_FRAME_WIDTH"), XmlOptions::get().get<int>("SELECTION_FRAME_HEIGHT") );
  
  QList<int> sizes;
  sizes.push_back( XmlOptions::get().get<int>( "KEYWORD_LIST_WIDTH" ) );
  sizes.push_back( XmlOptions::get().get<int>( "ENTRY_LIST_WIDTH" ) );
  splitter_->setSizes( sizes );
  
  // entry list mask
  if( XmlOptions::get().find( "ENTRY_LIST_MASK" ) )
  logEntryList().setMask( XmlOptions::get().get<unsigned int>( "ENTRY_LIST_MASK" ) );
  
  // colors
  list<string> color_list( XmlOptions::get().specialOptions<string>( "COLOR" ) );
  for( list<string>::iterator iter = color_list.begin(); iter != color_list.end(); iter++ )
  { color_menu_->add( *iter ); }
  
}

//_______________________________________________
void SelectionFrame::saveConfiguration( void )
{
  
  Debug::Throw( "SelectionFrame::saveConfiguration.\n" );
  
  // sizes
  XmlOptions::get().set<int>( "SELECTION_FRAME_WIDTH", width() );
  XmlOptions::get().set<int>( "SELECTION_FRAME_HEIGHT", height() );
  XmlOptions::get().set<int>( "KEYWORD_LIST_WIDTH", keywordList().width() );
  XmlOptions::get().set<int>( "ENTRY_LIST_WIDTH", logEntryList().width() );
  
  // entry list mask
  XmlOptions::get().set<unsigned int>( "ENTRY_LIST_MASK", logEntryList().mask() );
  
}

//_______________________________________________
void SelectionFrame::save( const bool& confirm_entries )
{
  Debug::Throw( "SelectionFrame::save - confirm.\n" );

  // check logbook
  if( !logbook_ ) 
  {
    QtUtil::infoDialog( this, "no Logbook opened. <Save> canceled." );
    return;
  }

  if( !confirm_entries ) confirm_entries_ = false;
  
  // check if editable EditFrames needs save
  // cancel if required
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
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
  dynamic_cast<MainFrame*>(qApp)->busy();
  bool written( logbook()->write() );
  dynamic_cast<MainFrame*>(qApp)->idle();

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
  LogEntryList::Item* item( logEntryList().currentItem<LogEntryList::Item>() );
  LogEntry *selected_entry( (item) ? item->entry():0 );

  string selection_string( qPrintable( selection ) );
  
  // check is selection_string is a valid color when Color search is requested.
  bool color_valid = ( 
    mode&SearchPanel::COLOR && ( 
    selection_string == ColorMenu::NONE ||
    QColor( selection_string.c_str() ).isValid() ) );
  
  // retrieve all logbook entries
  KeySet<LogEntry> entries( logbook()->entries() );
  KeySet<LogEntry> turned_off_entries;
  for( KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
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
    for( KeySet<LogEntry>::iterator it=turned_off_entries.begin(); it!= turned_off_entries.end(); it++ )
    (*it)->setFindSelected( true );

    return;
    
  }

  // reinitialize logEntry list
  _resetKeywordList();
  _resetList();

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
  LogEntryList::Item* item( logEntryList().currentItem<LogEntryList::Item>() );
  LogEntry *selected_entry( (item) ? item->entry():0 );

  // set all logbook entries to find_visible
  KeySet<LogEntry> entries( logbook()->entries() );
  for( KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
  { (*it)->setFindSelected( true ); }

  // reinitialize logEntry list
  _resetKeywordList();
  _resetList();

  if( selected_entry && selected_entry->isSelected() ) selectEntry( selected_entry );
  else if( logEntryList().topLevelItemCount() > 0 )
  {
    
    LogEntryList::Item *item( dynamic_cast<LogEntryList::Item*>( logEntryList().topLevelItem( logEntryList().topLevelItemCount()-1) ));
    if( item ) selectEntry( item->entry() );;
  
  }

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
  dynamic_cast<MainFrame*>(qApp)->exit();
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
void SelectionFrame::_resetList( void )
{
  
  Debug::Throw( "SelectionFrame::_resetList.\n" );

  // clear list of entries
  logEntryList().clear();
  
  if( !logbook_ ) return;
  KeySet<LogEntry> entries( logbook()->entries() );
  for( KeySet<LogEntry>::iterator it = entries.begin(); it != entries.end(); it++ )
  { if( (*it)->isSelected() ) logEntryList().add( *it ); }
  
  logEntryList().sort();
  logEntryList().resizeColumns();
  
}

//_______________________________________________
void SelectionFrame::_resetKeywordList( void )
{
  
  Debug::Throw( "SelectionFrame::_resetKeywordList.\n" );
      
  // clear list of entries
  // keywordList().clear();
  
  if( !logbook_ ) return;
    
  // retrieve current list of keywords
  set<string> old_keywords = keywordList().keywords();
  
  // retrieve new list of keywords (from logbook)
  set<string> new_keywords;
  KeySet<LogEntry> entries( logbook()->entries() );
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )  
  { if( (*iter)->isFindSelected() ) new_keywords.insert( (*iter)->keyword() ); }
  
  // reset keyword list to new set
  keywordList().reset( new_keywords );
  
  // sort list
  keywordList().sort();

}

//_______________________________________________
void SelectionFrame::_loadColors( void )
{
  
  Debug::Throw( "SelectionFrame::_loadColors.\n" );
  
  if( !logbook_ ) return;
  
  //! retrieve all entries
  KeySet<LogEntry> entries( logbook()->entries() );
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  { color_menu_->add( (*iter)->color() ); }

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
  dialog.setAuthor( XmlOptions::get().get<string>( "USER" ) );

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
  Exception::checkPointer( logbook_, DESCRIPTION( "could not create Logbook") );

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
  dynamic_cast<MainFrame*>(qApp)->busy();
  setLogbook( record.file() );
  dynamic_cast<MainFrame*>(qApp)->idle();

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
  dynamic_cast<MainFrame*>(qApp)->busy();
  string file( logbook()->file() );
  setLogbook( logbook()->file() );
  dynamic_cast<MainFrame*>(qApp)->idle();

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
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
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
  
  // set busy flag
  dynamic_cast<MainFrame*>(qApp)->busy();
  statusBar().label().setText( "reading remote logbook ... " );
  
  // opens file in a local logbook
  Logbook logbook;
  connect( &logbook, SIGNAL( messageAvailable() ), SIGNAL( messageAvailable() ) );
  logbook.setFile( File( qPrintable( files.back() ) ) );
  
  // check if logbook is valid
  XmlError::List errors( logbook.xmlErrors() );
  if( errors.size() ) 
  {

    ostringstream what;
    if( errors.size() > 1 ) what << "Errors occured while parsing files." << endl;
    else what << "An error occured while parsing files." << endl;
    what << errors;
    QtUtil::infoDialog( 0, what.str().c_str() );

    dynamic_cast<MainFrame*>(qApp)->idle();
    return;

  }

  // synchronize local with remote
  // retrieve map of duplicated entries
  std::map<LogEntry*,LogEntry*> duplicates( SelectionFrame::logbook()->synchronize( logbook ) );
  
  // update possible EditFrames when duplicated entries are found
  // delete the local duplicated entries
  for( std::map<LogEntry*,LogEntry*>::iterator iter = duplicates.begin(); iter != duplicates.end(); iter++ )
  {
    
    // display the new entry in all matching edit frames
    KeySet<EditFrame> frames( iter->first );
    for( KeySet<EditFrame>::iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
    { (*frame_iter)->displayEntry( iter->first ); }

    delete iter->first;

  }

  // synchronize remove with local
  logbook.synchronize( *SelectionFrame::logbook() );

  // reinitialize lists
  _resetKeywordList();
  _resetList();
  resetAttachmentFrame();

  // retrieve last modified entry
  KeySet<LogEntry> entries( SelectionFrame::logbook()->entries() );
  KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
  selectEntry( *iter );
  logEntryList().setFocus();

  // save current logbook if needed
  if( !SelectionFrame::logbook()->file().empty() )
  {
    statusBar().label().setText( "saving local logbook ... " );
    save();
  }

  // save remote logbook
  statusBar().label().setText( "saving remote logbook ... " );
  logbook.write();

  // idle
  dynamic_cast<MainFrame*>(qApp)->idle();
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
  KeySet<LogEntry> entries( logbook()->entries() );

  // dissasociate from logbook
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    
    KeySet<Logbook> logbooks( *iter );
    for( KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter != logbooks.end(); log_iter++ )
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

  // keep track of the current selected entry
  LogEntryList::Item* item( logEntryList().currentItem<LogEntryList::Item>() );
  LogEntry *selected_entry( (item) ? item->entry():0 );

  // keep track of found entries
  int found( 0 );

  // retrieve all logbook entries
  KeySet<LogEntry> entries( logbook()->entries() );
  KeySet<LogEntry> turned_off_entries;
  for( KeySet<LogEntry>::iterator iter=entries.begin(); iter!= entries.end(); iter++ )
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
    for( KeySet<LogEntry>::iterator it=turned_off_entries.begin(); it!= turned_off_entries.end(); it++ )
    (*it)->setFindSelected( true );

    return;
  }

  // reinitialize logEntry list
  _resetKeywordList();
  _resetList();

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
  LogbookInfoDialog dialog( this, logbook_ );
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
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
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
  EditFrame *frame = new EditFrame( this, false );
  Key::associate( this, frame );

  // call NewEntry for the selected frame
  frame->newEntry();
  frame->show();

}

//____________________________________________
void SelectionFrame::_editEntries( void )
{
  Debug::Throw( "SelectionFrame::_EditEntries .\n" );

  // retrieve current selection
  QList<LogEntryList::Item*> items( logEntryList().selectedItems<LogEntryList::Item>() );
  if( items.empty() ) {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

  // retrieve associated entry
  for( QList<LogEntryList::Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  _displayEntry( (*iter)->entry() );

  return;

}

//____________________________________________
void SelectionFrame::_deleteEntries ( void )
{
  Debug::Throw( "SelectionFrame::_DeleteEntries .\n" );

  // retrieve current selection
  QList<LogEntryList::Item*> items( logEntryList().selectedItems<LogEntryList::Item>() );
  if( items.empty() ) 
  {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

  // ask confirmation
  ostringstream what;
  what << "Delete selected entr" << ( items.size() == 1 ? "y":"ies" );
  if( !QtUtil::questionDialog( this, what.str() ) ) return;

  // retrieve associated entry
  for( QList<LogEntryList::Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    LogEntryList::Item* item( *iter );

    KeySet<LogEntry> entries( item );
    Exception::check( entries.size()==1, DESCRIPTION( "invalid association to LogEntry" ) );
    deleteEntry( *entries.begin(), false );

  }

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
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter=frames.begin(); iter != frames.end(); iter++ )
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
    edit_frame = new EditFrame( this, false );
    Key::associate( this, edit_frame );
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
  KeySet<EditFrame> frames( entry );
  for( KeySet< EditFrame >::iterator it = frames.begin(); it != frames.end(); it++ )
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
  KeySet<Logbook> logbooks( entry );
  for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
  (*iter)->setModified( true );
  
  // save Logbook
  if( logbook() && !logbook()->file().empty() ) save();
 
}

//_______________________________________________
void SelectionFrame::_changeEntryColor( QColor color )
{
  Debug::Throw( "SelectionFrame::_changeEntryColor.\n" );

  // retrieve current selection
  QList<LogEntryList::Item*> items( logEntryList().selectedItems<LogEntryList::Item>() );
  if( items.empty() ) 
  {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

  // retrieve associated entry
  for( QList<LogEntryList::Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    
    // get associated entry
    LogEntryList::Item *item( *iter );
    LogEntry* entry( item->entry() );

    entry->setColor( color.isValid() ? qPrintable( color.name() ):ColorMenu::NONE );
    entry->modified();
    
    // update list items
    KeySet<LogEntryList::Item> entry_items( entry );
    for( KeySet<LogEntryList::Item>::iterator it = entry_items.begin(); it != entry_items.end(); it++ )
    { (*it)->update(); }
    
    // update EditFrame color
    KeySet<EditFrame> frames( entry );
    for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    { if( !(*iter)->isClosed() ) (*iter)->displayColor(); }

    // set logbooks as modified
    KeySet<Logbook> logbooks( entry );
    for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
    { (*iter)->setModified( true ); }

  }

  // save Logbook
  if( !logbook()->file().empty() ) save();

}

//____________________________________________
void SelectionFrame::_newKeyword( void )
{
  
  Debug::Throw( "SelectionFrame::_newKeyword.\n" );
  
  //! create CustomDialog
  EditKeywordDialog dialog( this );
  dialog.setWindowTitle( "New keyword" );

  const set<string>& keywords( keywordList().keywords() );
  for( set<string>::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  dialog.add( *iter );
  dialog.setKeyword( keywordList().current() );
  
  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve keyword from line_edit
  string keyword = LogEntry::formatKeyword( dialog.keyword() );
  if( !keyword.empty() ) {
    keywordList().add( keyword );
    keywordList().select( keyword );
  }
}
  

//____________________________________________
void SelectionFrame::_deleteKeyword( void )
{
  Debug::Throw("SelectionFrame::_deleteKeyword.\n" );
  
  //! check that keywordlist has selected item
  if( !keywordList().QTreeWidget::currentItem() )
  {
    QtUtil::infoDialog( this, "no keyword selected. Request canceled" );
    return;
  }
  
  //! check that keywordlist selection is not root
  if( keywordList().QTreeWidget::currentItem() == keywordList().rootItem() ) 
  {
    QtUtil::infoDialog( this, "can't delete root keyword. Request canceled" );
    return;
  }

  // get current selected keyword
  QTreeWidgetItem *selected_item( keywordList().QTreeWidget::currentItem() );
  string keyword( keywordList().current() );

  // retrieve associated entries
  KeySet<LogEntry> entries( logbook()->entries() );
  KeySet<LogEntry> associated_entries;
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  if( (*iter)->keyword().find( keyword ) == 0 )
  associated_entries.insert( *iter );    
                 
  //! create CustomDialog
  DeleteKeywordDialog dialog( this, keyword, associated_entries.size() );
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;
  
  // retrieve parent keyword
  selected_item->setText( KeywordList::KEYWORD, "");
  string new_keyword( LogEntry::formatKeyword( keywordList().keyword( selected_item ) ) );  

  // move entries
  if( dialog.moveEntries() && associated_entries.size() ) 
  { 

    _changeEntryKeyword( keyword, new_keyword ); 
    return;
    
  } else if( dialog.deleteEntries() ) {
    
    for( KeySet<LogEntry>::iterator iter = associated_entries.begin(); iter != associated_entries.end(); iter++ )
    deleteEntry( *iter, false );
  
  }

  _resetKeywordList();
  keywordList().select( new_keyword );

  _resetList();
    
  // Save logbook
  if( !logbook()->file().empty() ) save();  
  
  return;
  
}

//____________________________________________
void SelectionFrame::_changeEntryKeyword( void )
{
  Debug::Throw("SelectionFrame::_changeEntryKeyword.\n" );
  
  //! check that keywordlist has selected item
  if( !keywordList().QTreeWidget::currentItem() )
  {
    QtUtil::infoDialog( this, "no keyword selected. Request canceled" );
    return;
  }
  
  //! check that keywordlist selection is not root
  if( keywordList().QTreeWidget::currentItem() == keywordList().rootItem() ) 
  {
    QtUtil::infoDialog( this, "can't rename root keyword. Request canceled" );
    return;
  }

  // get current selected keyword
  string keyword( keywordList().current() );
      
  //! create CustomDialog
  EditKeywordDialog dialog( this );
  dialog.setWindowTitle( "Edit keyword" );

  const set<string>& keywords( keywordList().keywords() );
  for( set<string>::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  dialog.add( *iter );
  dialog.setKeyword( keyword );
  
  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve keyword from line_edit
  _changeEntryKeyword( LogEntry::formatKeyword( dialog.keyword() ) );
  
}

//_______________________________________________
void SelectionFrame::_changeEntryKeyword( string new_keyword )
{
      
  Debug::Throw() << "SelectionFrame::_changeEntryKeyword - new_keyword: " << new_keyword << endl;
  
  // keep track of modified entries
  KeySet<LogEntry> entries;
  
  // loop over selected entries
  QList<LogEntryList::Item*> items( logEntryList().selectedItems<LogEntryList::Item>() );
  for( QList<LogEntryList::Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    
    // retrieve entry
    LogEntry* entry( (*iter)->entry() );
    
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
    KeySet<Logbook> logbooks( entry );
    for( KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter!= logbooks.end(); log_iter++ )
    { (*log_iter)->setModified( true ); }
  
  }

  // check if at least one entry was changed
  if( entries.empty() ) return;
  
  // reset lists
  _resetKeywordList();
  _resetList();
  keywordList().select( new_keyword );
  
  // update selection
  logEntryList().clearSelection();
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    LogEntryList::Item* item( logEntryList().item( *iter ) );
    logEntryList().setItemSelected( item, true );  
  }
  
  // scroll to last item
  if( !entries.empty() ) logEntryList().scrollToItem( logEntryList().item( *entries.begin() ) );
  
  // Save logbook if needed
  if( !logbook()->file().empty() ) save();
  
  return;    
  
}

//____________________________________________
void SelectionFrame::_changeEntryKeyword( string keyword, string new_keyword )
{
  
  Debug::Throw() << "SelectionFrame::_changeEntryKeyword - keyword: " << keyword << " new:" << new_keyword << endl;
  
  // check keywords are different
  if( keyword == new_keyword ) return;
    
  // get entries matching the old_keyword, change the keyword
  KeySet<LogEntry> entries( logbook()->entries() );
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    LogEntry* entry( *iter );
    
    /* 
      if keyword to modify is a leading subset of current entry keyword, 
      update entry with new keyword
    */
    if( entry->keyword().find( keyword ) == 0 ) 
    {
      
      entry->setKeyword( Str( entry->keyword() ).replace( keyword, new_keyword ) );
      
      /* this is a kludge: add 1 second to the entry modification timeStamp to avoid loosing the 
      keyword change when synchronizing logbooks, without having all entries modification time
      set to now() */
      entry->setModification( entry->modification()+1 );

      KeySet<Logbook> logbooks( entry );
      for( KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter!= logbooks.end(); log_iter++ )
      (*log_iter)->setModified( true );
    }
    
  }

  // reset lists
  _resetKeywordList();
  _resetList();
  keywordList().select( new_keyword );
    
  // Save logbook if needed
  if( !logbook()->file().empty() ) save();     
  
}
 
//_______________________________________________
void SelectionFrame::_keywordSelectionChanged( QTreeWidgetItem* current, QTreeWidgetItem* old )
{

  Debug::Throw( "SelectionFrame::_keywordSelectionChanged.\n" );
  if( !logbook_ ) return; 
  if( !current ) return;
  
  string keyword = keywordList().keyword( current );
  Debug::Throw() << "SelectionFrame::_keywordSelectionChanged - keyword: " << keyword << endl;
      
  // keep track of the last visible entry
  LogEntry *last_visible_entry( 0 );
  
  // keep track of the current selected entry
  LogEntryList::Item* local_item( ( logEntryList().currentItem<LogEntryList::Item>() ) );
  LogEntry *selected_entry( (local_item) ? local_item->entry():0 );
  
  // retrieve all logbook entries
  KeySet<LogEntry> entries( logbook()->entries() );
  KeySet<LogEntry> turned_off_entries;
  for( KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
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
  _resetList();
  
  // if EditFrame current entry is visible, select it;
  if( selected_entry && selected_entry->isSelected() ) logEntryList().select( selected_entry );
  else if( last_visible_entry ) logEntryList().select( last_visible_entry );
  
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
  what << "/tmp/_eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
  dialog.setFile( File( what.str() ) );

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
  vector<LogEntry*> entries;
  if( dialog.allEntries() )
  {

    KeySet<LogEntry> entry_set( logbook()->entries() );
    entries = vector<LogEntry*>( entry_set.begin(), entry_set.end() );

  } else if( dialog.visibleEntries() ) {

    list<LogEntry*> entry_list = ( logEntryList().entries() );
    entries = vector<LogEntry*>( entry_list.begin(), entry_list.end() );

  } else {

    list<LogEntry*> entry_list = ( logEntryList().selectedEntries() );
    entries = vector<LogEntry*>( entry_list.begin(), entry_list.end() );

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
void SelectionFrame::_storeSortMethod( int column )
{
  
  Debug::Throw( "SelectionFrame::_storeSortMethod.\n");
  if( !logbook_ ) return;

  switch( column ) {
    
    case LogEntryList::TITLE: logbook()->setSortingMethod( Logbook::SORT_TITLE ); break;
    case LogEntryList::CREATION: logbook()->setSortingMethod( Logbook::SORT_CREATION ); break;
    case LogEntryList::MODIFICATION: logbook()->setSortingMethod( Logbook::SORT_MODIFICATION ); break;
    case LogEntryList::AUTHOR: logbook()->setSortingMethod( Logbook::SORT_AUTHOR ); break;
    default: break;
    
  }

  // Save logbook if needed
  if( !logbook()->file().empty() ) save();

}

//_______________________________________________
void SelectionFrame::_autoSave( void )
{

  if( logbook_ && !logbook()->file().empty() ) 
  {
  
    statusBar().label().setText( "performing autoSave" );

    // retrieve non read only editors; perform save
    KeySet<EditFrame> frames( this );
    for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    if( !((*iter)->isReadOnly() || (*iter)->isClosed() ) ) (*iter)->save();

    save();
  
  } else
  statusBar().label().setText( "no logbook filename. <Autosave> skipped" );

}
