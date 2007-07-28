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
  \file SelectionFrame.h
  \brief base class to display entries and keyword
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "AttachmentFrame.h"
#include "CustomDialog.h"
#include "CustomLineEdit.h"
#include "Debug.h"
#include "EditFrame.h"
#include "Exception.h"
#include "HtmlUtil.h"
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

//_____________________________________________
SelectionFrame::SelectionFrame( QWidget *parent, const std::string& name ):
  TopWidget( parent, name.c_str() ),
  autosave_timer_( this ),
  logbook_( 0 ),
  working_directory_( Util::GetWorkingDirectory() ),
  ignore_warnings_( false )
{
  Debug::Throw( "SelectionFrame::SelectionFrame.\n" );
  
  // main widget
  QWidget* main = new QWidget( this );
  setCentralWidget( main ); 
  
  // local layout
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing(5);
  main->setLayout( layout );
  
  // main menu
  menu_ = new Menu( main_widget_, this );
  setMenuBar( menu );
  
  connect( menu_, SIGNAL( save() ), SLOT( saveLogbook() ) );
  connect( menu, SIGNAL( qApp(), SLOT( closeAllWindows() ) );
  
  // splitter for KeywordList/LogEntryList
  QSplitter *splitter( new QSplitter( main ) );  
  splitter->setOrientation( Qt::Horizontal );
  layout.addWidget( splitter );
  
  // left box for Keywords and buttons
  QWidget* left = new QWidget( splitter );
  QVBoxLayout* v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  left->setLayout( v_layout );
  
  list<string> path_list( Options::GetSpecialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );
  
  QToolBar* toolbar = new QToolBar( left );
  v_layout->addWidget( toolbar );
  
  CustomToolButton *button;
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::NEW, path_list ) ), "New keyword", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _newKeyword() ) );
  toolbar->addWidget( button );
  button->setText( "New" );
  
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::EDIT, path_list ) ), "Edit keyword", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _renameKeyword() ) );
  button->setText( "Edit" );
  toolbar->addWidget( button );
  
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::DELETE, path_list ) ), "Delete keyword", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _deleteKeyword() ) );
  button->setText( "Delete", false );
  toolbar->addWidget( button );
  
  // create keyword list
  v_layout->addWidget( keyword_list_ = new KeywordList( left ) );
  connect( keyword_list_, SIGNAL( selectionChanged( QTreeWidgetItem* ) ), SLOT( _keywordSelectionChanged( QTreeWidgetItem* ) ) );  
  connect( keyword_list_, SIGNAL( keywordChanged( const std::string& ) ), SLOT( _changeEntryKeyword( const std::string& ) ) );
  connect( keyword_list_, SIGNAL( keywordChanged( const std::string&, const std::string& ) ), SLOT( _renameKeyword( const std::string&, const std::string& ) ) );
  connect( keyword_list_, SIGNAL( itemRenamed( QTreeWidgetItem*, int ) ), SLOT( _renameKeyword( QTreeWidgetItem*, int ) ) );
    
  // popup menu for keyword list
  keyword_list_->addMenuAction( "&New keyword", this, SLOT( _newKeyword() ) );
  keyword_list_->addMenuAction( "&Rename keyword", this, SLOT( _renameKeyword() ), true );
  keyword_list_->addMenuAction( "&Delete keyword", this, SLOT( _deleteKeyword() ), true );
  
  // right box for entries and buttons
  QWidget* right = new QWidget( splitter );
  QVBoxLayout* v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  right->setLayout( v_layout );
    
  toolbar = new QToolBar( right_vbox );
  v_layout->addWidget( toolbar );
  
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::NEW, path_list ), "New entry", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _NewEntry() ) );
  button->setText( "New" );
  toolbar->addWidget( button );
  
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::EDIT, path_list ), "Edit selected entries", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _EditEntries() ) );
  button->setText( "Edit" );
  toolbar->addWidget( button );

  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::DELETE, path_list ), "Delete selected entries", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _DeleteEntries() ) );
  button->setText( "Delete" );
  toolbar->addWidget( button );

  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::HTML, path_list ), "Convert logbook to html", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( ViewHtml() ) );
  button->setText( "Html" );
  toolbar->addWidget( button );

  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::SAVE, path_list ), "Save all edited entries", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( SaveLogbook() ) );
  button->setText( "Save" );
  toolbar->addWidget( button );
  
  // create logEntry list
  list_ = new LogEntryList( right_vbox, "log_entry_list" );
  // connect( list_->header(), SIGNAL( clicked( int ) ), SLOT( _StoreSortMethod( int ) ) );
  //connect( list_, SIGNAL( itemRenamed( QTreeWidgetItem*, int ) ), SLOT( _RenameEntry( QTreeWidgetItem*, int ) ) );
  connect( list_, SIGNAL( itemActivated( QTreeWidgetItem* ) ), SLOT( _ShowEditFrame( QTreeWidgetItem* ) ) );
  connect( new QShortCut( Key_Delete, list_ ), SLOT( _deleteEntries() ) );

  // create popup menu for list
  list_->addMenuAction( "&New entry", this, SLOT( _newEntry() ) );
  list_->addMenuAction( "&Edit entries", this, SLOT( _editEntries() ), true ); 
  list_->addMenuAction( "&Delete entries", this, SLOT( _deleteEntries() ), true ); 
  list_->addMenuAction( "&Change keyword", this, SLOT( _changeEntryKeyword() ), true );
  
  // search panel
  search_panel_ = new SearchPanel( main );
  connect( search_panel_, SIGNAL( SelectEntries( const std::string&, const unsigned int& ) ), SLOT( SelectEntries( const std::string&, const unsigned int& ) ) );
  connect( search_panel_, SIGNAL( ShowAllEntries() ), SLOT( ShowAllEntries() ) );
  layout->addWidget( search_panel_ );  
  
  // state frame
  statusbar_ = new StateFrame( main_widget_ );
  QtUtil::FixSize( statusbar_, QtUtil::HEIGHT );
  statusbar_->GetLayout().setStretchFactor( &statusbar_->label(), 2 );
  statusbar_->GetLayout().addWidget( new ClockLabel( statusbar_ ) );
  connect( this, SIGNAL( LogbookMessageAvailable( const QString& ) ), &statusbar_->label(), SLOT( setTextAndUpdate( const QString& ) ) );  
  layout->addWidget( statusbar_ ); 
 
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );

  // autosave_timer_
  autosave_timer_.setSingleShot( false );
  connect( &autosave_timer_, SIGNAL( timeout() ), SLOT( _autoSave() ) );
  
}


//_______________________________________________
void SelectionFrame::setLogbook( const File& file )
{
  Debug::Throw("SelectionFrame::SetLogbook.\n" );

  // reset current logbook
  if( logbook_ ) reset( );

  // create new logbook
  logbook_ = new Logbook();

  // if filename is empty, return
  if( file.empty() )
  {
    emit ready();
    return;
  }

  // set file
  logbook_->setFile( file );
  if( !file.exist() )
  {
    emit ready();
    return;
  }

  connect( logbook_, SIGNAL( messageAvailable( const QString& ) ), SIGNAL( messageAvailable( const QString& ) ) );

  logbook_->read();

  // update listView with new entries
  _resetKeywordList();
  _resetList();

  // change sorting
  switch( logbook_->sortingMethod() ) 
  {
    case Logbook::SORT_KEYWORD: list_->sortItems( LogEntryList::KEYWORD ); break;
    case Logbook::SORT_TITLE: list_->sortItems( LogEntryList::TITLE ); break;
    case Logbook::SORT_CREATION: list_->sortItems( LogEntryList::CREATION ); break;
    case Logbook::SORT_MODIFICATION: list_->sortItems( LogEntryList::MODIFICATION ); break;
    case Logbook::SORT_AUTHOR: list_->sortItems( LogEntryList::AUTHOR ); break;
    default: break;
  }

  // update attachment frame
  resetAttachmentFrame();

  // retrieve last modified entry
  KeySet<LogEntry> entries( logbook_->entries() );
  KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::lastModifiedFTor() );
  selectEntry( *iter );
  list_->setFocus();

  // see if logbook has parent file
  if( logbook_->parentFile().size() ) {
    ostringstream o; 
    o << "Warning: this logbook should be oppened via \"" << logbook_->GetParentFile() << "\" only.";
    QtUtil::infoDialog( this, o.str() );
  }

  // store logbook directory for next open, save comment
  working_directory_ = File( logbook_->file() ).path();
  statusBar().label().setText( "" );

  emit ready();

  // check errors
  XmlError::List errors( logbook_->xmlErrors() );
  if( errors.size() )
  {
    ostringstream what;
    if( errors.size() > 1 ) what << "Errors occured while parsing files." << endl;
    else what << "An error occured while parsing files." << endl;
    what << errors;
    QtUtil::infoDialog( 0, what.str().c_str() );
  }
  
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
    XmlOptions::get()get<bool>( "AUTO_BACKUP" ) &&
    logbook_->file().size() &&
    logbook_->needsBackup() ) 
  {

    // ask if backup needs to be saved; save if yes
    if( QtUtil::questionDialog( this, "Current logbook needs backup. Make one?" )) 
    { saveBackup(); }

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
  list<string> files( logbook_->checkFiles() );
  if( files.empty() ) return;

  int state = LogbookModifiedDialog( this, file() ).exec();
  if( state == LogbookModifiedDialog::RESAVE ) { save(); }
  else if( state == LogbookModifiedDialog::SAVE_AS ) { saveAs(); }
  else if( state == LogbookModifiedDialog::RELOAD ) { modified_ = false; revertToSave(); }
  else if( state == LogbookModifiedDialog::IGNORE ) { ignore_warnings_ = true; }
  
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
  keyword_list_->clear();
  list_->clear();
    
  // clear the AttachmentFrame
  static_cast<MainFrame*>(qApp)->attachmentFrame().list().clear();
  
  // delete all EditFrames
  KeySet<EditFrame> frames( this ); 
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ ) 
  (*iter)->hide();
  
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
  
  // exec and check return code 
  int state = dialog.exec();
  if( state == AskForSaveDialog::YES ) saveLogbook();
  return AskForSaveDialog::ReturnCode(state);
}

//_______________________________________________
void TreeSelectionFrame::selectEntry( LogEntry* entry )
{
  Debug::Throw( "SelectionFrame::selectEntry.\n" );
  
  if( !entry ) return;
  keyword_list_->selectKeyword( entry->keyword() );
  list_->selectEntry( entry );
  return;
  
}

//_______________________________________________
void SelectionFrame::updateEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "SelectionFrame::updateEntry.\n" );
  
  // add entry into frame list or update existing    
  if( entry-keyword() != keyword_list_->currentKeyword() )
  {
    keyword_list_->addKeyword( entry->keyword() );
    keyword_list_->selectKeyword( entry->keyword() );
  }

  if( !KeySet<LogEntryList::LocalItem>( entry ).empty() ) list_->updateEntry( entry, update_selection );
  else list_->addEntry( entry, update_selection );
  
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
    KeySet<AttachmentList::LocalItem> items( *iter );
    for( KeySet<AttachmentList::LocalItem>::iterator attachment_iter = items.begin(); attachment_iter != items.end(); attachment_iter++ )
    delete *attachment_iter;

    // delete attachment
    delete (*iter);

  };

  // get associated logentrylist items
  KeySet<LogEntryList::LocalItem> items( entry );
  for( KeySet<LogEntryList::LocalItem>::iterator iter = items.begin(); iter != items.end(); iter++ )
  delete *iter;

  /*
    hide associated EditFrames
    they will get deleted next time
    SelectionFrame::_showEditFrame() is called
  */
  KeySet<EditFrame> frames( entry );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  (*iter)->hide();

  // set logbooks as modified
  KeySet<Logbook> logbooks( entry );
  for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
  (*iter)->setModified( true );

  // delete entry
  delete entry;

  //! save
  if( save && !logbook()->file().empty() )
  saveLogbook();

  return;

}

//_______________________________________________
bool SelectionFrame::lockEntry( LogEntry* entry ) const
{
  Debug::Throw( "SelectionFrame::lockEntry.\n" );
  
  if( !entry ) return true;
  
  KeySet<EditFrame> frames( entry );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  if(  !( (*iter)->isReadOnly() || (*iter)->isHidden() ) )
  {
    if( (*iter)->modified() && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return false;
    (*iter)->setReadOnly( true );
  }
  
  return true;
}


//_______________________________________________
LogEntry* SelectionFrame::previousEntry( LogEntry* entry, const bool& update_selection ) const
{

  Debug::Throw( "SelectionFrame::previousEntry.\n" );
  
  LogEntryList::LocalItem *item( list_->item( entry ) );
  if( !( item && (item = item->itemAbove( update_selection ) ) ) ) return 0;
  return item->entry();

}

//_______________________________________________
LogEntry* SelectionFrame::nextEntry( LogEntry* entry, const bool& update_selection ) const
{

  Debug::Throw( "SelectionFrame::nextEntry.\n" );
  LogEntryList::LocalItem *item( list_->item( entry ) );
  if( !( item && (item = item->itemBelow( update_selection ) ) ) ) return 0;
  return item->entry();

}

//_______________________________________________
void SelectionFrame::resetAttachmentFrame( void ) const
{

  Debug::Throw( "SelectionFrame::resetAttachmentFrame.\n" );

  // clear the AttachmentFrame
  AttachmentFrame &attachment_frame( static_cast<MainFrame*>(qApp)->GetAttachmentFrame() );
  attachment_frame.list().clear();

  // check current logbook
  if( !logbook_ ) return;

  // retrieve logbook attachments, adds to AttachmentFrame
  KeySet<Attachment> attachments( logbook_->attachments() );
  for( KeySet<Attachment>::iterator it = attachments.begin(); it != attachments.end(); it++ )
  attachment_frame.list().addAttachment( *it );

  return;

}

//_______________________________________________
void SelectionFrame::setAutoSave( bool on )
{
  Debug::Throw( "SelectionFrame::setAutoSave.\n" );
  if( on ) {

    if( timer_ && timer_->isActive() ) return;
    if( !timer_ ) {
      timer_ = new QTimer( this );
      connect( timer_, SIGNAL( timeout() ), SLOT( _AutoSave() ) );
    }
    timer_->start( 1000*Options::Get<int>( "AUTO_SAVE_ITV" ) );

  } else {
    if( !( timer_ && timer_->isActive() ) ) return;
    timer_->stop();
  }

}

//_______________________________________________
void SelectionFrame::updateConfiguration( void )
{
  
  Debug::Throw( "SelectionFrame::updateConfiguration.\n" );
  
  // autoSave
  autosave_timer_.setInterval( 1000*XmlOptions::get().get<int>( "AUTO_SAVE_ITV" ) );
  bool autosave( XmlOptions::get().get<bool>( "AUTO_SAVE" ) );
  if( autosave ) autosave_timer_.start();
  else autosave_timer_.stop();
  
}

//_______________________________________________
void SelectionFrame::synchronize( void )
{
  Debug::Throw( "SelectionFrame::SynchronizeLogbook.\n" );

  // check current logbook is valid
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "No logbook opened. <Merge> canceled." );
    return;
  }

  // save EditFrames
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  if( !((*iter)->isReadOnly() || (*iter)->isHidden() ) && (*iter)->modified() && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return;

  // save current logbook
  if( logbook_->modified() && AskForSave() == AskForSaveDialog::CANCEL ) return;

  // create file dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::ExistingFile );

  if( dialog.exec() != QDialog::Accepted ) return;

  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return;
  
  // set busy flag
  static_cast<MainFrame*>(qApp)->busy();
  statusBar().label().setText( "reading remote logbook ... " );
  
  // opens file in a local logbook
  Logbook logbook;
  connect( &logbook, SIGNAL( messageAvailable() ), SIGNAL( messageAvailable() ) );
  logbook.setFile( File( qPrintable( files.back() ) );
  
  // check if logbook is valid
  XmlError::List errors( logbook.xmlErrors() );
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

  // synchronize local with remote
  // retrieve map of duplicated entries
  map<LogEntry*,LogEntry*> duplicates( logbook_->synchronize( logbook );
  
  // update possible EditFrames when duplicated entries are found
  // delete the local duplicated entries
  for( map<LogEntry*,LogEntry*>::iterator iter = duplicates.begin(); iter != duplicates.end(); iter++ )
  {
    
    // display the new entry in all matching edit frames
    KeySet<EditFrame> frames( iter->first );
    for( KeySet<EditFrame>::iterator frame_iter = frames.begin(); frame_iter != frames.end(); frame_iter++ )
    { (*frame_iter)->displayEntry( iter->first ); }

    delete iter->first;

  }

  // synchronize remove with local
  logbook->synchronize( logbook_ );

  // reinitialize lists
  _resetKeywordList();
  _resetList();
  resetAttachmentFrame();

  // retrieve last modified entry
  KeySet<LogEntry> entries( logbook_->entries() );
  KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
  selectEntry( *iter );
  list_->setFocus();

  // save current logbook if needed
  if( !logbook_->file().empty() )
  {
    statusBar().label().setText( "saving local logbook ... " );
    saveLogbook();
  }

  // save remote logbook
  statusBar().label().setText( "saving remote logbook ... " );
  logbook.write();

  // idle
  static_cast<MainFrame*>(qApp)->idle();
  statusBar().label().setText( "" );

  return;

}

//_______________________________________________
void SelectionFrame::newLogbook( void )
{
  Debug::Throw( "SelectionFrame::newLogbook.\n" );

  // check current logbook
  if( logbook_ && logbook_->GetModified() && AskForSave() == AskForSaveDialog::CANCEL ) return;

  // new logbook
  NewLogbookDialog dialog( this );
  dialog.setTitle( Logbook::LOGBOOK_NO_TITLE );
  dialog.setAuthor( Options::Get<string>( "USER" ) );

  // filename and directory
  File file = File( "log.xml" ).addPath( workingDirectory() );
  dialog.setFile( file );
  dialog.setAttachmentDirectory( workingDirectory() );

  // map dialog
  QtUtil:cCenterOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // create a new logbook, with no file
  setLogbook( dialog.file() );
  Exception::checkPointer( logbook_, DESCRIPTION( "could not create Logbook") );

  logbook_->setTitle( dialog.title() );
  logbook_->setAuthor( dialog.author() );
  logbook_->setComments( dialog.comments() );

  // attachment directory
  File directory( dialog.attachmentDirectory() );

  // check if fulldir is not a non directory existing file
  if( directory.exist() && !directory.isDirectory() )
  {

    ostringstream o;
    o << "File \"" << directory << "\" is not a directory.";
    QtUtil::infoDialog( this, o.str() );

  } else logbook_->setDirectory( directory );

}

//_______________________________________________
void SelectionFrame::open( const FileRecord& record )
{
  
  Debug::Throw( "SelectionFrame::open.\n" );

  // check if current logbook needs save
  if( logbook_ && logbook_->GetModified()  && AskForSave() == AskForSaveDialog::CANCEL ) return;

  // open file from dialog if not set as argument
  if( record.file().empty() )
  {
    // create file dialog
    CustomFileDialog dialog( this, "file dialog", TRUE );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setDirectory( workingDirectory().c_str() );

    if( dialog.exec() == QDialog::Rejected ) return;

    QStringList files( dialog.selectedFiles() );
    if( files.empty() ) return;
    record = FileRecord( File( qPrintable( files.front() ) ) );
    
  }

  // create logbook from file
  static_cast<MainFrame*>(qApp)->busy();
  setLogbook( file );
  static_cast<MainFrame*>(qApp)->idle();

  // check if backup is needed
  checkLogbookBackup();
  
  // add to open previous menu
  menu().openPreviousMenu().Add( logbook_->file() );
  return;
}

//_______________________________________________
void SelectionFrame::save( void )
{
  Debug::Throw( "SelectionFrame::save.\n");

  // check logbook
  if( !logbook_ ) 
  {
    QtUtil::infoDialog( this, "no Logbook opened. <Save> canceled." );
    return;
  }

  // check if editable EditFrames needs save
  // cancel if required
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  if( !((*iter)->isReadOnly() || (*iter)->isHidden() ) && (*iter)->modified() && (*iter)->askForSave() == askForSaveDialog::CANCEL ) return;

  // check logbook filename, go to Save As if no file is given and redirect is true
  if( logbook_->file().empty() ) {
    saveAs();
    return;
  }

  // check logbook filename is writable
  File fullname = File( logbook_->file() ).expand();
  if( fullname.exist() ) {

    // check file is not a directory
    if( fullname.isDirectory() ) {
      QtUtil::infoDialog( this, "selected file is a directory. <Save Logbook> canceled." );
      return;
    }

    // check file is writable
    if( !fullname.isWritable() ) {
      QtUtil::infoDialog( this, "selected file is not writable. <Save Logbook> canceled." );
      return;
    }
    
  } else {

    File path( fullname.path() );
    if( !path.isDirectory() ) {
      QtUtil::infoDialog(
        this, "selected path is not vallid. <Save Logbook> canceled."
      );
      return;
    }

  }

  // write logbook to file, retrieve result
  static_cast<MainFrame*>(qApp)->busy();
  bool written( logbook_->write() );
  static_cast<MainFrame*>(qApp)->idle();

  if( written ) {

    menu().openPreviousMenu().add( logbook_->file() );
    setWindowTitle( MainFrame::MAIN_TITLE );

  }

  // update StateFrame
  statusBar().label().setText( "" );

  // reset ignore_warning flag
  ignore_warnings_ = false;

  return;
}

//_______________________________________________
bool SelectionFrame::saveLogbookAs( File default_file )
{
  Debug::Throw( "SelectionFrame::SaveLogbookAs.\n");

  // check current logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no logbook opened. <Save Logbook> canceled." );
    return false;
  }

  // check default filename
  if( default_file.empty() ) default_file = logbook_->file();
  if( default_file.empty() ) default_file = File( "log.xml" ).addPath( workingDirectory() );

  // create file dialog
  CustomFileDialog dialog( this, TRUE );
  dialog.setFileMode( QFileDialog::AnyFile );
  dialog.setSelection( default_file.c_str() );
  QtUtil::CenterOnPointer( &dialog, false );
  if( dialog.exec() == QDialog::Rejected ) return false;

  // retrieve filename
  File fullname = File( (const char*) dialog.selectedFile() ).Expand();

  // update working directory
  working_directory_ = fullname.GetPath();

  // check if file exist
  if(
    fullname.Exist() &&
    !QtUtil::questionDialog( this, "selected file already exist. Overwrite ?" ) )
  return false;

  // change logbook filename and save
  logbook_->SetFile( fullname );
  _SaveLogbookForced( );

  /*
    force logbook state to unmodified since
    some children state may not have been reset properly
  */
  logbook_->SetModifiedRecursive( false );

  // reset ignore_warning flag
  ignore_warnings_ = false;

  return true;
}

//_______________________________________________
void SelectionFrame::SaveLogbookBackup( void )
{
  Debug::Throw( "SelectionFrame::SaveLogbookBackup.\n");

  // check current logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no logbook opened. <Save Logbook> canceled." );
    return;
  }

  string filename( logbook_->MakeBackupFilename( ) );
  if( filename.empty() ) {
    QtUtil::infoDialog( this, "no valid filename. Use <Save As> first." );
    return;
  }

  // store last backup time and update
  TimeStamp last_backup( logbook_->GetBackup() );

  // stores current logbook filename
  string current_filename( logbook_->file() );

  // save logbook as backup
  bool saved( SaveLogbookAs( filename ) );

  // remove the "backup" filename from the openPrevious list
  GetMenu().GetOpenPreviousMenu().Remove( filename );

  // set logbook filename back to initial filename
  logbook_->SetFile( current_filename );

  if( saved ) {

    logbook_->SetBackup( TimeStamp::Now() );
    logbook_->SetModified( true );
    static_cast<MainFrame*>(qApp)->SetWindowTitle( MainFrame::MAIN_TITLE_MODIFIED );

    // Save logbook if needed
    if( logbook_->file().size() ) SaveLogbook();
  }

}

//_____________________________________________
void SelectionFrame::RevertToSavedLogbook( void )
{
  Debug::Throw( "SelectionFrame::RevertToSavedLogbook.\n" );

  // check logbook
  if( !logbook_ ){
    QtUtil::infoDialog( this, "No logbook opened. <Revert to save> canceled." );
    return;
  }

  // ask for confirmed
  if(
    logbook_->GetModified() &&
    !QtUtil::questionDialog( this, "Discard changes to current logbook ?" )
  ) return;

  // stores logbook filename
  string filename( logbook_->file() );

  // reinit SelectionFrame
  static_cast<MainFrame*>(qApp)->Busy();
  SetLogbook( filename );
  static_cast<MainFrame*>(qApp)->Idle();

  CheckLogbookBackup();

}

//_____________________________________________
void SelectionFrame::ViewHtml( void )
{
  Debug::Throw( "SelectionFrame::ViewHtml.\n" );

  // check logbook
  if( !logbook_ ){
    QtUtil::infoDialog( this, "No logbook opened. <View HTML> canceled." );
    return;
  }

  // create custom dialog, retrieve check vbox child
  ViewHtmlLogbookDialog dialog( this );
  dialog.SetSelection( ViewHtmlLogbookDialog::ALL );
  dialog.SetLogbookMask( Logbook::HTML_ALL_MASK );
  dialog.SetEntryMask( LogEntry::HTML_ALL_MASK );
  dialog.SetCommand(
    ( AttachmentType::HTML.GetEditCommand().size() ) ?
    AttachmentType::HTML.GetEditCommand():"" );

  ostringstream what;
  what << "/tmp/_eLogbook_" << TimeStamp::Now().GetUnixTime() << "_" << Util::GetPid() << ".html";
  dialog.SetFile( File( what.str() ) );

  // map dialog
  QtUtil::CenterOnParent( &dialog, false );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve/check file
  File file( dialog.file() );
  if( file.empty() ) {
    QtUtil::infoDialog(this, "No output file specified. <View HTML> canceled." );
    return;
  }

  // open/check temp file
  ofstream out( file.c_str() );
  if( !out ) {
    ostringstream o;
    o << "Cannot write to file \"" << file << "\". <View HTML> canceled.";
    QtUtil::infoDialog( this, o.str() );
    return;
  }

  // retrieve mask
  unsigned int html_log_mask = dialog.GetLogbookMask();
  unsigned int html_entry_mask = dialog.GetEntryMask();

 QDomDocument document( "html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"DTD/xhtml1-strict.dtd\"" );
  
  // html
  QDomElement html = document.appendChild( document.createElement( "html" ) ).toElement(); 
  html.setAttribute( "xmlns", "http://www.w3.org/1999/xhtml" );
  
  // head
  HtmlUtil::Header( html, document );
  
  // body
  QDomElement body = html.appendChild( document.createElement( "body" ) ).toElement();

  // dump logbook header
  logbook_->HtmlElement( body, document, html_log_mask );

  // retrieve entries to write
  vector<LogEntry*> entries;
  if( dialog.AllEntries() )
  {

    KeySet<LogEntry> entry_set( logbook_->GetAllEntries() );
    for( KeySet<LogEntry>::iterator iter = entry_set.begin(); iter != entry_set.end(); iter++ )
    entries.push_back( *iter );

  } else if( dialog.VisibleEntries() ) {

    list<LogEntry*> entry_list = ( list_->GetAllEntries() );
    for( list<LogEntry*>::iterator iter = entry_list.begin(); iter != entry_list.end(); iter++ )
    entries.push_back( *iter );

  } else {

    list<LogEntryList::LocalItem*> items( list_->GetSelectedItems<LogEntryList::LocalItem>() );
    for( list<LogEntryList::LocalItem*>::iterator iter = items.begin(); iter!=items.end(); iter++ )
    entries.push_back( (*iter)->GetEntry() );

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
    { (*it)->HtmlSummary( table, document ); }
    
  }
    
  // dump full logbook
  if( html_log_mask & Logbook::HTML_CONTENT )
  for( vector<LogEntry*>::iterator it=entries.begin(); it != entries.end(); it++ )
  (*it)->HtmlElement( body, document, html_entry_mask );

  out << document.toString();
  out.close();

  // retrieve command
  string command( dialog.GetCommand() );
  if( command.empty() ) return;

  // execute command
  what.str("");
  what << command << " " << file << " &";
  Util::Run( what.str() );
  return;
}

//_______________________________________________
void SelectionFrame::ResizeEditors( const int& width, const int& height )
{
  Debug::Throw( "SelectionFrame::ResizeEditors.\n" );
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  { (*iter)->resize( width, height ); }
}

//_______________________________________________
void SelectionFrame::SetTabEmulation( const bool& active, const unsigned int& size )
{
  Debug::Throw( "SelectionFrame::ResizeEditors.\n" );
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  { (*iter)->GetEditor().SetTabEmulation( active, size ); }
}

//_______________________________________________
void SelectionFrame::ShowDuplicatedEntries( void )
{
  Debug::Throw( "SelectionFrame::ShowDuplicatedEntries.\n" );

  // keep track of the last visible entry
  LogEntry *last_visible_entry( 0 );

  // keep track of the current selected entry
  LogEntryList::LocalItem* item( dynamic_cast<LogEntryList::LocalItem*>( list_->selectedItem() ) );
  LogEntry *selected_entry( (item) ? item->GetEntry():0 );

  // keep track of found entries
  int found( 0 );

  // retrieve all logbook entries
  KeySet<LogEntry> entries( logbook_->GetAllEntries() );
  KeySet<LogEntry> turned_off_entries;
  for( KeySet<LogEntry>::iterator iter=entries.begin(); iter!= entries.end(); iter++ )
  {

    // retrieve entry
    LogEntry* entry( *iter );

    // if entry is already hidder, skipp
    if( !entry->IsSelected() ) continue;

    // check duplicated entries
    int n_duplicates( count_if( entries.begin(), entries.end(), LogEntry::SameCreationFTor( (*iter)->GetCreation() ) ) );
    if( n_duplicates < 2 ) {
      entry->SetFindSelected( false );
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
    (*it)->SetFindSelected( true );

    return;
  }

  // reinitialize logEntry list
  _ResetKeywordList();
  _ResetList();

  // if EditFrame current entry is visible, select it;
  if( selected_entry && selected_entry->IsSelected() ) SelectEntry( selected_entry );
  else if( last_visible_entry ) SelectEntry( last_visible_entry );

  return;
}

//_______________________________________________
void SelectionFrame::EditLogbookInfo( void )
{
  Debug::Throw( "SelectionFrame::EditLogbookInfo.\n" );
    if( !logbook_ ) {
    QtUtil::infoDialog( this, "No logbook opened." );
    return;
  }

  // create dialog
  LogbookInfoDialog dialog( 0, logbook_ );
  dialog.setCaption( "Logbook information" );
  dialog.resize( 350, 300 );
  if( dialog.exec() == QDialog::Rejected ) return;

  // keep track of logbook modifications
  bool modified( false );

  // retrieve, update logbook title
  string title( dialog.Title() );
  if( title.size() ) modified |= logbook_->SetTitle( title );

  // update logbook author
  string author( dialog.Author() );
  modified |= logbook_->SetAuthor( author );

  // retrieve logbook directory
  File directory = dialog.AttachmentDirectory();

  // check if fulldir is not a non directory existing file
  if( directory.Exist() &&  !directory.IsDirectory() )
  {

    ostringstream o;
    o << "File \"" << directory << "\" is not a directory.";
    QtUtil::infoDialog( this, o.str() );

  } else {

    // set logbook directory, check it, ask for creation
    modified |= logbook_->SetDirectory( directory );
    if( !File( logbook_->GetDirectory() ).Exist() ) {
      ostringstream o;
      o << "Directory \"" << logbook_->GetDirectory() << "\" does not exist. Create ?";
      if( QtUtil::questionDialog( this, o.str() ) ) {

        ostringstream o;
        o << "mkdir \"" << logbook_->GetDirectory() << "\"";
        Util::Run( o.str() );

      }
    }
  }

  // update logbook comments
  modified |= logbook_->SetComments( dialog.Comments() );

  // save Logbook, if needed
  if( modified ) logbook_->SetModified( true );

  // Save logbook if needed
  if( logbook_->file().size() ) SaveLogbook();

}

//_______________________________________________
void SelectionFrame::ViewLogbookStat( void )
{
  Debug::Throw( "SelectionFrame::ViewLogbookStat.\n" );
    if( !logbook_ ) {
    QtUtil::infoDialog( this, "No logbook opened." );
    return;
  }

  // create dialog
  LogbookStatisticsDialog dialog( 0, logbook_ );
  dialog.setCaption( "Logbook information" );
  dialog.exec();
}

//_______________________________________________
void SelectionFrame::ReorganizeLogbook( void )
{
  Debug::Throw( "SelectionFrame::ReorganizeLogbook.\n" );
  if( !logbook_ )
  {
    QtUtil::infoDialog( this,"No valid logbook. Canceled.\n");
    return;
  }

  // retrieve all entries
  KeySet<LogEntry> entries( logbook_->GetAllEntries() );

  // dissasociate from logbook
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    KeySet<Logbook> logbooks( *iter );
    for( KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter != logbooks.end(); log_iter++ )
    (*log_iter)->SetModified( true );
    (*iter)->ClearAssociations<Logbook>();
  }

  //! put entry set into a list and sort by creation time. 
  // First entry must the oldest
  list<LogEntry*> entry_list( entries.begin(), entries.end() );
  entry_list.sort( LogEntry::FirstCreatedFTor() );
  
  // put entries in logbook
  for( list<LogEntry*>::iterator iter = entry_list.begin(); iter != entry_list.end(); iter++ )
  {
    Logbook *logbook( logbook_->GetLatestChild() );
    Key::Associate( *iter, logbook );
    logbook->SetModified( true );
  }
  
  // remove empty logbooks
  logbook_->RemoveEmptyChildren();
  
  // save
  logbook_->SetModified( true );
  if( logbook_->file().size() ) SaveLogbook();

}

//_______________________________________________
void SelectionFrame::CloseEditFrames( void ) const
{
  Debug::Throw( "SelectionFrame::CloseEditFrames.\n" );

  // get all EditFrames from SelectionFrame
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  {
    if( (*iter)->Modified() && !((*iter)->ReadOnly() || (*iter)->isHidden() ) && (*iter)->AskForSave() == AskForSaveDialog::CANCEL ) return;
    delete *iter;
  }
  return;
}

//____________________________________________
void SelectionFrame::_LoadColors( void )
{
  Debug::Throw( "SelectionFrame::_LoadColors.\n" );

  // clear menu
  color_menu_->clear();

  list<string> colors( Options::GetSpecialOptions<string>( "COLOR" ) );
  colors.sort();
  colors.unique();
  for( list<string>::iterator iter = colors.begin(); iter != colors.end(); iter++ )
  color_menu_->AddColor( *iter );
  color_menu_->DisplayColors();

  return;

}

//_______________________________________________
void SelectionFrame::_StoreSortMethod( int column )
{
  Debug::Throw( "SelectionFrame::_StoreSortMethod.\n");
  if( !logbook_ ) return;

  switch( column ) {
    case LogEntryList::KEYWORD: logbook_->SetSortingMethod( Logbook::SORT_KEYWORD ); break;
    case LogEntryList::TITLE: logbook_->SetSortingMethod( Logbook::SORT_TITLE ); break;
    case LogEntryList::CREATION: logbook_->SetSortingMethod( Logbook::SORT_CREATION ); break;
    case LogEntryList::MODIFICATION: logbook_->SetSortingMethod( Logbook::SORT_MODIFICATION ); break;
    case LogEntryList::AUTHOR: logbook_->SetSortingMethod( Logbook::SORT_AUTHOR ); break;
    default: break;
  }

  // Save logbook if needed
  if( logbook_->file().size() ) SaveLogbook();

}

//_______________________________________________
void SelectionFrame::_ShowEditFrame( QTreeWidgetItem* item )
{

  Debug::Throw( "SelectionFrame::_ShowEditFrame.\n" );

  // cast item to LogEntryList::Item
  LogEntryList::LocalItem *local_item( dynamic_cast<LogEntryList::LocalItem*>(item) );
  Exception::Assert( local_item, DESCRIPTION( "invalid item" ) );

  // retrieve associated entries
  KeySet<LogEntry> entries( local_item );
  Exception::Assert( entries.size()==1, DESCRIPTION( "invalid association to entry" ) );
  LogEntry *entry( *entries.begin() );

  // retrieve associated EditFrames, check if one matches the selected entry
  EditFrame *edit_frame( 0 );
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter=frames.begin(); iter != frames.end(); iter++ )
  {
    /*
      if Editframe is to be deleted, delete it. This is to avoid memory leak
      for EditFrames which should have been deleted before but could not to avoid crash
    */
    if( (*iter)->isHidden() ) delete *iter;

    //! check if EditFrame is editable and match editor
    else if( !((*iter)->ReadOnly() || (*iter)->isHidden() ) && (*iter)->GetEntry() == entry ) {
      edit_frame = *iter;
      break;
    }

  }

  // create editFrame if not found
  if( !edit_frame )
  {
    edit_frame = new EditFrame( this, "edit_frame", entry, false );
    Key::Associate( this, edit_frame );
  }

  // show edit_frame
  edit_frame->Uniconify();

}

//_______________________________________________
void SelectionFrame::SelectEntries( const string& selection, const unsigned int& mode )
{
  Debug::Throw() << "SelectionFrame::_SelectEntries - selection: " << selection << " mode:" << mode << endl;

  // check logbook
  if( !logbook_ ) return;

  // check selection text
  if( selection.empty() ){
    ShowAllEntries();
    return;
  }

  // retrieve selection source
  if( mode == SearchPanel::NONE ) {
    QtUtil::infoDialog( this, "At least on field must be selected" , QtUtil::CENTER_ON_PARENT );
    return;
  }

  // number of found items
  unsigned int found( 0 );
  unsigned int total( 0 );

  // keep track of the last visible entry
  LogEntry *last_visible_entry( 0 );

  // keep track of the current selected entry
  LogEntryList::LocalItem* item( dynamic_cast<LogEntryList::LocalItem*>( list_->selectedItem() ) );
  LogEntry *selected_entry( (item) ? item->GetEntry():0 );

  // retrieve all logbook entries
  KeySet<LogEntry> entries( logbook_->GetAllEntries() );
  KeySet<LogEntry> turned_off_entries;
  for( KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
  {

    // retrieve entry
    LogEntry* entry( *it );
    total++;

    // if entry is already hidder, skipp
    if( !entry->IsFindSelected() ) continue;

    // check entry
    bool accept( false );
    if( (mode&SearchPanel::TITLE ) && entry->MatchTitle( selection ) ) accept = true;
    if( (mode&SearchPanel::KEYWORD ) && entry->MatchKeyword( selection ) ) accept = true;
    if( (mode&SearchPanel::TEXT ) && entry->MatchText( selection ) ) accept = true;
    if( (mode&SearchPanel::ATTACHMENT ) && entry->MatchAttachment( selection ) ) accept = true;
    if( (mode&SearchPanel::COLOR ) && entry->MatchColor( selection ) ) accept = true;

    if( accept ) {
      found++;
      if( entry->IsKeywordSelected() || !(last_visible_entry && last_visible_entry->IsKeywordSelected()) )
      last_visible_entry = entry;
    } else {
      turned_off_entries.insert( entry );
      entry->SetFindSelected( false );
    }

  }

  // if no entries are found, restore the disabled entries and abort
  if( !found ) {

    statusBar().label().setText( "no match found. Find canceled" );

    // reset flag for the turned off entries to true
    for( KeySet<LogEntry>::iterator it=turned_off_entries.begin(); it!= turned_off_entries.end(); it++ )
    (*it)->SetFindSelected( true );

    return;
  }

  // reinitialize logEntry list
  _ResetKeywordList();
  _ResetList();

  // if EditFrame current entry is visible, select it;
  if( selected_entry && selected_entry->IsSelected() ) SelectEntry( selected_entry );
  else if( last_visible_entry ) SelectEntry( last_visible_entry );

  ostringstream out;
  out << found << " out of " << total;
  if( found > 1 ) out << " entries selected";
  else out << " entry selected";
  statusBar().label().setText( out.str() );

  return;
}

//_______________________________________________
void SelectionFrame::ShowAllEntries( void )
{
  Debug::Throw( "SelectionFrame::ShowAllEntries.\n" );

  // keep track of the current selected entry
  LogEntryList::LocalItem* item( dynamic_cast<LogEntryList::LocalItem*>( list_->selectedItem() ) );
  LogEntry *selected_entry( (item) ? item->GetEntry():0 );

  // set all logbook entries to find_visible
  KeySet<LogEntry> entries( logbook_->GetAllEntries() );
  for( KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
  (*it)->SetFindSelected( true );

  // reinitialize logEntry list
  _ResetKeywordList();
  _ResetList();

  if( selected_entry && selected_entry->IsSelected() ) SelectEntry( selected_entry );
  else {
    LogEntryList::LocalItem *item( dynamic_cast<LogEntryList::LocalItem*>( list_->lastItem() ));
    if( item ) SelectEntry( item->GetEntry() );;
  }

  statusBar().label().setText( "" );
  return;
}

//____________________________________________
void SelectionFrame::_NewEntry( void )
{
  Debug::Throw( "SelectionFrame::_NewEntry.\n" );

  // create new EditFrame
  EditFrame *frame = new EditFrame( this, "edit_frame", 0, false );
  frame->SetWrapText( WrapText() );
  Key::Associate( this, frame );

  // call NewEntry for the selected frame
  frame->NewEntry();
  frame->Uniconify();

}

//____________________________________________
void SelectionFrame::_EditEntries( void )
{
  Debug::Throw( "SelectionFrame::_EditEntries .\n" );

  // retrieve current selection
  list<LogEntryList::LocalItem*> items( list_->GetSelectedItems<LogEntryList::LocalItem>() );
  if( items.empty() ) {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

  // retrieve associated entry
  for( list<LogEntryList::LocalItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  _ShowEditFrame( *iter );

  return;

}

//____________________________________________
void SelectionFrame::_DeleteEntries ( void )
{
  Debug::Throw( "SelectionFrame::_DeleteEntries .\n" );

  // retrieve current selection
  list<LogEntryList::LocalItem*> items( list_->GetSelectedItems<LogEntryList::LocalItem>() );
  if( items.empty() ) {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

   // ask confirmation
   if( !QtUtil::questionDialog( this, "Delete selected entries ?" ) ) return;

  // retrieve associated entry
  for( list<LogEntryList::LocalItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    LogEntryList::LocalItem* item( *iter );

    KeySet<LogEntry> entries( item );
    Exception::Assert( entries.size()==1, DESCRIPTION( "invalid association to LogEntry" ) );
    DeleteEntry( *entries.begin(), false );

  }

  // Save logbook if needed
  if( logbook_->file().size() ) SaveLogbook();

  return;

}

//_______________________________________________
void SelectionFrame::_ChangeEntryColor( const std::string& color )
{
  Debug::Throw( "SelectionFrame::_ChangeEntryColor.\n" );

  // try add color to options
  list<string> colors( Options::GetSpecialOptions<string>( "COLOR" ) );
  if( std::find( colors.begin(), colors.end(), color ) == colors.end() )
  {
    Options::Get().Add( Option( "COLOR", color ) );
    if( Options::Get().file().size() ) Options::Get().Write();
  }

  // check if selected item make sense
  list<LogEntryList::LocalItem*> items( list_->GetSelectedItems<LogEntryList::LocalItem>() );
  if( items.empty() )
  {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }

  // retrieve associated entry
  for( list<LogEntryList::LocalItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    // get associated entry
    LogEntryList::LocalItem *item( *iter );
    LogEntry* entry( item->GetEntry() );

    entry->SetColor( color );
    entry->Modified();

    // redraw item
    item->repaint();

    // update EditFrame color
    KeySet<EditFrame> frames( entry );
    for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    if( !(*iter)->isHidden() ) (*iter)->DisplayColor();

    // set logbooks as modified
    KeySet<Logbook> logbooks( entry );
    for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
    (*iter)->SetModified( true );

  }

  // save Logbook
  if( GetLogbook()->file().size() ) SaveLogbook();

}

//_______________________________________________
void SelectionFrame::_AutoSave( void )
{

  if( logbook_ && logbook_->file().size() ) {
    statusBar().label().setText( "performing autoSave" );

    // retrieve non read only editors; perform save
    KeySet<EditFrame> frames( this );
    for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    if( !( (*iter)->ReadOnly() || (*iter)->isHidden() ) ) (*iter)->SaveEntry();

    SaveLogbook();
  } else
  statusBar().label().setText( "no logbook filename. <Autosave> skipped" );

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

//_____________________________________________
void SelectionFrame::_SaveLogbookForced( void )
{
  Debug::Throw( "SelectionFrame::_SaveLogbookForced.\n" );

  // retrieve/check SelectionFrame/Logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no Logbook opened. <Save> canceled." );
    return;
  }

  // set all logbooks as modified
  logbook_->SetModifiedRecursive( true );
  SaveLogbook( );
}
