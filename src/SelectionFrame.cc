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

#include <QSplitter>

#include "AttachmentFrame.h"
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
  TopWidget( parent ),
  autosave_timer_( this ),
  logbook_( 0 ),
  working_directory_( Util::workingDirectory() ),
  ignore_warnings_( false )
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
  QSplitter *splitter( new QSplitter( main ) );  
  splitter->setOrientation( Qt::Horizontal );
  layout->addWidget( splitter, 1 );
 
  // search panel
  search_panel_ = new SearchPanel( main );
  connect( search_panel_, SIGNAL( selectEntries( QString, unsigned int ) ), SLOT( selectEntries( QString, unsigned int ) ) );
  connect( search_panel_, SIGNAL( showAllEntries() ), SLOT( showAllEntries() ) );
  layout->addWidget( search_panel_ );  
  
  // state frame
  statusbar_ = new StatusBar( main );
  statusBar().addLabel( 2 );
  statusBar().addClock();
  connect( this, SIGNAL( messageAvailable( const QString& ) ), &statusBar().label(), SLOT( setTextAndUpdate( const QString& ) ) );  
  layout->addWidget( &statusBar() ); 
  
  // left box for Keywords and buttons
  QWidget* left = new QWidget( splitter );
  QVBoxLayout* v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  left->setLayout( v_layout );
  
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );
  
  CustomToolBar* toolbar = new CustomToolBar( left );
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
  button->setText( "Delete" );
  toolbar->addWidget( button );
  
  // create keyword list
  v_layout->addWidget( keyword_list_ = new KeywordList( left ), 1 );
  connect( keyword_list_, SIGNAL( itemSelectionChanged( void ) ), SLOT( _keywordSelectionChanged( void ) ) );  
  connect( keyword_list_, SIGNAL( keywordChanged( const std::string& ) ), SLOT( _changeEntryKeyword( const std::string& ) ) );
  connect( keyword_list_, SIGNAL( keywordChanged( const std::string&, const std::string& ) ), SLOT( _renameKeyword( const std::string&, const std::string& ) ) );
  // connect( keyword_list_, SIGNAL( itemRenamed( QTreeWidgetItem*, int ) ), SLOT( _renameKeyword( QTreeWidgetItem*, int ) ) );
    
  // popup menu for keyword list
  keywordList().addMenuAction( "&New keyword", this, SLOT( _newKeyword() ) );
  keywordList().addMenuAction( "&Rename keyword", this, SLOT( _renameKeyword() ), true );
  keywordList().addMenuAction( "&Delete keyword", this, SLOT( _deleteKeyword() ), true );
  
  // right box for entries and buttons
  QWidget* right = new QWidget( splitter );
  v_layout = new QVBoxLayout();
  v_layout->setMargin(0);
  v_layout->setSpacing( 5 );
  right->setLayout( v_layout );
    
  toolbar = new CustomToolBar( right );
  v_layout->addWidget( toolbar );
  
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::NEW, path_list ) ), "New entry", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _newEntry() ) );
  button->setText( "New" );
  toolbar->addWidget( button );
  
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::EDIT, path_list ) ), "Edit selected entries", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _editEntries() ) );
  button->setText( "Edit" );
  toolbar->addWidget( button );

  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::DELETE, path_list ) ), "Delete selected entries", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( _deleteEntries() ) );
  button->setText( "Delete" );
  toolbar->addWidget( button );

  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::HTML, path_list ) ), "Convert logbook to html", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( viewHtml() ) );
  button->setText( "Html" );
  toolbar->addWidget( button );

  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::SAVE, path_list ) ), "Save all edited entries", &statusBar().label() );
  connect( button, SIGNAL( clicked() ), SLOT( save() ) );
  button->setText( "Save" );
  toolbar->addWidget( button );
  
  // create logEntry list
  v_layout->addWidget( list_ = new LogEntryList( right, "log_entry_list" ), 1 );
  
  // connect( logEntryList().header(), SIGNAL( clicked( int ) ), SLOT( _StoreSortMethod( int ) ) );
  //connect( list_, SIGNAL( itemRenamed( QTreeWidgetItem*, int ) ), SLOT( _RenameEntry( QTreeWidgetItem*, int ) ) );
  connect( list_, SIGNAL( itemActivated( QTreeWidgetItem*, int ) ), SLOT( _showEditFrame( QTreeWidgetItem* ) ) );
  connect( new QShortcut( Key_Delete, list_ ), SIGNAL( activated() ), SLOT( _deleteEntries() ) );

  // create popup menu for list
  logEntryList().addMenuAction( "&New entry", this, SLOT( _newEntry() ) );
  logEntryList().addMenuAction( "&Edit entries", this, SLOT( _editEntries() ), true ); 
  logEntryList().addMenuAction( "&Delete entries", this, SLOT( _deleteEntries() ), true ); 
  logEntryList().addMenuAction( "&Change keyword", this, SLOT( _changeEntryKeyword() ), true );
 
  // main menu
  menu_ = new Menu( this , this );
  setMenuBar( menu_ );
  
  connect( menu_, SIGNAL( save() ), SLOT( save() ) );
  connect( menu_, SIGNAL( closeWindow() ), qApp, SLOT( exit() ) );
 
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );
  updateConfiguration();

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
  Debug::Throw( "SelectionFrame::setLogbook - lists set.\n" );
  
  // change sorting
  switch( logbook_->sortingMethod() ) 
  {
    case Logbook::SORT_TITLE: logEntryList().sortItems( LogEntryList::TITLE ); break;
    case Logbook::SORT_CREATION: logEntryList().sortItems( LogEntryList::CREATION ); break;
    case Logbook::SORT_MODIFICATION: logEntryList().sortItems( LogEntryList::MODIFICATION ); break;
    case Logbook::SORT_AUTHOR: logEntryList().sortItems( LogEntryList::AUTHOR ); break;
    default: break;
  }

  Debug::Throw( "SelectionFrame::setLogbook - lists sorted.\n" );

  // update attachment frame
  resetAttachmentFrame();
  Debug::Throw( "SelectionFrame::setLogbook - attachment frame reset.\n" );

  // retrieve last modified entry
  KeySet<LogEntry> entries( logbook_->entries() );
  KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
  selectEntry( *iter );
  logEntryList().setFocus();
  
  Debug::Throw( "SelectionFrame::setLogbook - entry selected.\n" );

  
  // see if logbook has parent file
  if( logbook_->parentFile().size() ) {
    ostringstream o; 
    o << "Warning: this logbook should be oppened via \"" << logbook_->parentFile() << "\" only.";
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
    XmlOptions::get().get<bool>( "AUTO_BACKUP" ) &&
    !logbook_->file().empty() &&
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
  list<File> files( logbook_->checkFiles() );
  if( files.empty() ) return;

  int state = LogbookModifiedDialog( this, files ).exec();
  if( state == LogbookModifiedDialog::RESAVE ) { save(); }
  else if( state == LogbookModifiedDialog::SAVE_AS ) { saveAs(); }
  else if( state == LogbookModifiedDialog::RELOAD ) 
  { 
    logbook_->setModifiedRecursive( false ); 
    revertToSaved(); 
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
  if( state == AskForSaveDialog::YES ) save();
  return AskForSaveDialog::ReturnCode(state);
}

//_______________________________________________
void SelectionFrame::selectEntry( LogEntry* entry )
{
  Debug::Throw( "SelectionFrame::selectEntry.\n" );
  
  if( !entry ) return;
  keywordList().selectKeyword( entry->keyword() );
  logEntryList().selectEntry( entry );
  return;
  
}

//_______________________________________________
void SelectionFrame::updateEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "SelectionFrame::updateEntry.\n" );
  
  // add entry into frame list or update existing    
  if( entry->keyword() != keywordList().currentKeyword() )
  {
    keywordList().addKeyword( entry->keyword() );
    keywordList().selectKeyword( entry->keyword() );
  }

  if( !KeySet<LogEntryList::Item>( entry ).empty() ) logEntryList().updateEntry( entry, update_selection );
  else logEntryList().addEntry( entry, update_selection );
  
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
  if(  !( (*iter)->isReadOnly() || (*iter)->isHidden() ) )
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
  if( !( item && (item = logEntryList().itemAbove( item, update_selection ) ) ) ) return 0;
  return item->entry();

}

//_______________________________________________
LogEntry* SelectionFrame::nextEntry( LogEntry* entry, const bool& update_selection )
{

  Debug::Throw( "SelectionFrame::nextEntry.\n" );
  LogEntryList::Item *item( logEntryList().item( entry ) );
  if( !( item && (item = logEntryList().itemBelow( item, update_selection ) ) ) ) return 0;
  return item->entry();

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
  KeySet<Attachment> attachments( logbook_->attachments() );
  for( KeySet<Attachment>::iterator it = attachments.begin(); it != attachments.end(); it++ )
  attachment_frame.list().addAttachment( *it );

  return;

}

//_______________________________________________
void SelectionFrame::updateConfiguration( void )
{
  
  Debug::Throw( "SelectionFrame::updateConfiguration.\n" );
  
  TopWidget::updateConfiguration();
  
  // autoSave
  autosave_timer_.setInterval( 1000*XmlOptions::get().get<int>( "AUTO_SAVE_ITV" ) );
  bool autosave( XmlOptions::get().get<bool>( "AUTO_SAVE" ) );
  if( autosave ) autosave_timer_.start();
  else autosave_timer_.stop();
    
  // resize
  resize( XmlOptions::get().get<int>("SELECTION_FRAME_WIDTH"), XmlOptions::get().get<int>("SELECTION_FRAME_HEIGHT") );
  
  
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
  if( logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

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

    static_cast<MainFrame*>(qApp)->idle();
    return;

  }

  // synchronize local with remote
  // retrieve map of duplicated entries
  std::map<LogEntry*,LogEntry*> duplicates( logbook_->synchronize( logbook ) );
  
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
  logbook.synchronize( *logbook_ );

  // reinitialize lists
  _resetKeywordList();
  _resetList();
  resetAttachmentFrame();

  // retrieve last modified entry
  KeySet<LogEntry> entries( logbook_->entries() );
  KeySet<LogEntry>::const_iterator iter = min_element( entries.begin(), entries.end(), LogEntry::LastModifiedFTor() );
  selectEntry( *iter );
  logEntryList().setFocus();

  // save current logbook if needed
  if( !logbook_->file().empty() )
  {
    statusBar().label().setText( "saving local logbook ... " );
    save();
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
  if( logbook_ && logbook_->modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

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
void SelectionFrame::open( FileRecord record )
{
  
  Debug::Throw( "SelectionFrame::open.\n" );

  // check if current logbook needs save
  if( logbook_ && logbook_->modified()  && askForSave() == AskForSaveDialog::CANCEL ) return;

  // open file from dialog if not set as argument
  if( record.file().empty() )
  {
    // create file dialog
    CustomFileDialog dialog( this );
    dialog.setFileMode( QFileDialog::ExistingFile );
    dialog.setDirectory( workingDirectory().c_str() );

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
  
  // add to open previous menu
  menu().openPreviousMenu().add( logbook_->file() );
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
  if( !((*iter)->isReadOnly() || (*iter)->isHidden() ) && (*iter)->modified() && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return;

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
bool SelectionFrame::saveAs( File default_file )
{
  Debug::Throw( "SelectionFrame::saveAs.\n");

  // check current logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no logbook opened. <Save Logbook> canceled." );
    return false;
  }

  // check default filename
  if( default_file.empty() ) default_file = logbook_->file();
  if( default_file.empty() ) default_file = File( "log.xml" ).addPath( workingDirectory() );

  // create file dialog
  CustomFileDialog dialog( this );
  dialog.setFileMode( QFileDialog::AnyFile );
  dialog.selectFile( default_file.c_str() );
  QtUtil::centerOnPointer( &dialog, false );
  if( dialog.exec() == QDialog::Rejected ) return false;

  // retrieve files
  QStringList files( dialog.selectedFiles() );
  if( files.empty() ) return false;
  
  // retrieve filename
  File fullname = File( qPrintable( files.back() ) ).expand();

  // update working directory
  working_directory_ = fullname.path();

  // check if file exist
  if(
    fullname.exist() &&
    !QtUtil::questionDialog( this, "selected file already exist. Overwrite ?" ) )
  return false;

  // change logbook filename and save
  logbook_->setFile( fullname );
  _saveForced( );

  /*
    force logbook state to unmodified since
    some children state may not have been reset properly
  */
  logbook_->setModifiedRecursive( false );

  // reset ignore_warning flag
  ignore_warnings_ = false;

  return true;
}

//_______________________________________________
void SelectionFrame::saveBackup( void )
{
  Debug::Throw( "SelectionFrame::saveBackup.\n");

  // check current logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no logbook opened. <Save Backup> canceled." );
    return;
  }

  string filename( logbook_->backupFilename( ) );
  if( filename.empty() ) {
    QtUtil::infoDialog( this, "no valid filename. Use <Save As> first." );
    return;
  }

  // store last backup time and update
  TimeStamp last_backup( logbook_->backup() );

  // stores current logbook filename
  string current_filename( logbook_->file() );

  // save logbook as backup
  bool saved( saveAs( filename ) );

  // remove the "backup" filename from the openPrevious list
  // to avoid confusion
  menu().openPreviousMenu().remove( filename );

  // restore initial filename
  logbook_->setFile( current_filename );

  if( saved ) {

    logbook_->setBackup( TimeStamp::now() );
    logbook_->setModified( true );
    setWindowTitle( MainFrame::MAIN_TITLE_MODIFIED );

    // Save logbook if needed (to make sure the backup stamp is updated)
    if( !logbook_->file().empty() ) save();
  }

}

//_____________________________________________
void SelectionFrame::revertToSaved( void )
{
  Debug::Throw( "SelectionFrame::revertToSaved.\n" );

  // check logbook
  if( !logbook_ ){
    QtUtil::infoDialog( this, "No logbook opened. <Revert to save> canceled." );
    return;
  }

  // ask for confirmation
  if(
    logbook_->modified() &&
    !QtUtil::questionDialog( this, "discard changes to current logbook ?" )
  ) return;

  // reinit SelectionFrame
  static_cast<MainFrame*>(qApp)->busy();
  setLogbook( logbook_->file() );
  static_cast<MainFrame*>(qApp)->idle();

  checkLogbookBackup();
  ignore_warnings_ = false;
  
}

//_____________________________________________
void SelectionFrame::viewHtml( void )
{
  Debug::Throw( "SelectionFrame::viewHtml.\n" );

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
  QtUtil::centerOnParent( &dialog, false );
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
  body.appendChild( logbook_->htmlElement( document, html_log_mask ) );

  // retrieve entries to write
  vector<LogEntry*> entries;
  if( dialog.allEntries() )
  {

    KeySet<LogEntry> entry_set( logbook_->entries() );
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
  what.str("");
  what << command << " " << file << " &";
  Util::run( what.str() );
  return;
}

//_______________________________________________
void SelectionFrame::showDuplicatedEntries( void )
{
  Debug::Throw( "SelectionFrame::showDuplicatedEntries.\n" );

  // keep track of the last visible entry
  LogEntry *last_visible_entry( 0 );

  // keep track of the current selected entry
  LogEntryList::Item* item( logEntryList().currentItem<LogEntryList::Item>() );
  LogEntry *selected_entry( (item) ? item->entry():0 );

  // keep track of found entries
  int found( 0 );

  // retrieve all logbook entries
  KeySet<LogEntry> entries( logbook_->entries() );
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
void SelectionFrame::editLogbookInformations( void )
{
  Debug::Throw( "SelectionFrame::editLogbookInformations.\n" );
  
  if( !logbook_ ) 
  {
    QtUtil::infoDialog( this, "No logbook opened." );
    return;
  }

  // create dialog
  LogbookInfoDialog dialog( this, logbook_ );
  if( dialog.exec() == QDialog::Rejected ) return;

  // keep track of logbook modifications
  bool modified( false );

  modified |= logbook_->setTitle( dialog.title() );
  modified |= logbook_->setAuthor( dialog.author() );
  modified |= logbook_->setComments( dialog.comments() );

  // retrieve logbook directory
  File directory = dialog.AttachmentDirectory();

  // check if fulldir is not a non directory existing file
  if( directory.exist() &&  !directory.isDirectory() )
  {

    ostringstream o;
    o << "File \"" << directory << "\" is not a directory.";
    QtUtil::infoDialog( this, o.str() );

  } else modified |= logbook_->setDirectory( directory );


  // save Logbook, if needed
  if( modified ) logbook_->setModified( true );
  if( !logbook_->file().empty() ) save();

}

//_______________________________________________
void SelectionFrame::viewLogbookStatistics( void )
{
  Debug::Throw( "SelectionFrame::viewLogbookStatistics.\n" );
  
  if( !logbook_ ) 
  {
    QtUtil::infoDialog( this, "No logbook opened." );
    return;
  }

  // create dialog
  LogbookStatisticsDialog dialog( this, logbook_ );
  dialog.exec();
}

//_______________________________________________
void SelectionFrame::reorganize( void )
{
  Debug::Throw( "SelectionFrame::reorganize.\n" );

  if( !logbook_ )
  {
    QtUtil::infoDialog( this,"No valid logbook. Canceled.\n");
    return;
  }

  // retrieve all entries
  KeySet<LogEntry> entries( logbook_->entries() );

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
    Logbook *logbook( logbook_->latestChild() );
    Key::associate( *iter, logbook );
    logbook->setModified( true );
  }
  
  // remove empty logbooks
  logbook_->removeEmptyChildren();
  
  // save
  logbook_->setModified( true );
  if( !logbook_->file().empty() ) save();

}

//_______________________________________________
void SelectionFrame::closeEditFrames( void ) const
{
  Debug::Throw( "SelectionFrame::CloseEditFrames.\n" );

  // get all EditFrames from SelectionFrame
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
  {
    if( (*iter)->modified() && !((*iter)->isReadOnly() || (*iter)->isHidden() ) && (*iter)->askForSave() == AskForSaveDialog::CANCEL ) return;
    delete *iter;
  }
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
  
  // retrieve all logbook entries
  KeySet<LogEntry> entries( logbook_->entries() );
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
    if( (mode&SearchPanel::COLOR ) && entry->matchColor( selection_string ) ) accept = true;

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
  KeySet<LogEntry> entries( logbook_->entries() );
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

// //____________________________________________
// void SelectionFrame::_LoadColors( void )
// {
//   Debug::Throw( "SelectionFrame::_LoadColors.\n" );
// 
//   // clear menu
//   color_menu_->clear();
// 
//   list<string> colors( Options::GetSpecialOptions<string>( "COLOR" ) );
//   colors.sort();
//   colors.unique();
//   for( list<string>::iterator iter = colors.begin(); iter != colors.end(); iter++ )
//   color_menu_->AddColor( *iter );
//   color_menu_->DisplayColors();
// 
//   return;
// 
// }

//_______________________________________________
void SelectionFrame::_storeSortMethod( int column )
{
  
  Debug::Throw( "SelectionFrame::_storeSortMethod.\n");
  if( !logbook_ ) return;

  switch( column ) {
    
    case LogEntryList::TITLE: logbook_->setSortingMethod( Logbook::SORT_TITLE ); break;
    case LogEntryList::CREATION: logbook_->setSortingMethod( Logbook::SORT_CREATION ); break;
    case LogEntryList::MODIFICATION: logbook_->setSortingMethod( Logbook::SORT_MODIFICATION ); break;
    case LogEntryList::AUTHOR: logbook_->setSortingMethod( Logbook::SORT_AUTHOR ); break;
    default: break;
    
  }

  // Save logbook if needed
  if( !logbook_->file().empty() ) save();

}

//_______________________________________________
void SelectionFrame::_showEditFrame( QTreeWidgetItem* item )
{

  Debug::Throw( "SelectionFrame::_showEditFrame.\n" );

  // cast item to LogEntryList::Item
  LogEntryList::Item *local_item( dynamic_cast<LogEntryList::Item*>(item) );
  Exception::assert( local_item, DESCRIPTION( "invalid item" ) );

  // retrieve associated entries
  KeySet<LogEntry> entries( local_item );
  Exception::assert( entries.size()==1, DESCRIPTION( "invalid association to entry" ) );
  LogEntry *entry( *entries.begin() );

  // retrieve associated EditFrames, check if one matches the selected entry
  EditFrame *edit_frame( 0 );
  KeySet<EditFrame> frames( this );
  for( KeySet<EditFrame>::iterator iter=frames.begin(); iter != frames.end(); iter++ )
  {
    
    // if Editframe is hidden, delete it. Hidden frames are used for delayed deletion
    if( (*iter)->isHidden() ) delete *iter;

    //! check if EditFrame is editable and match editor
    else if( !((*iter)->isReadOnly() || (*iter)->isHidden() ) && (*iter)->entry() == entry ) 
    {
      edit_frame = *iter;
      break;
    }

  }

  // create editFrame if not found
  if( !edit_frame )
  {
    edit_frame = new EditFrame( this, entry, false );
    Key::associate( this, edit_frame );
  }

  // show edit_frame
  edit_frame->show();
  //edit_frame->uniconify();

}

//_______________________________________________
void SelectionFrame::_keywordSelectionChanged( void )
{

  Debug::Throw( "SelectionFrame::_keywordSelectionChanged.\n" );
  if( !logbook_ ) return; 
  
  string keyword = keywordList().currentKeyword();
      
  // keep track of the last visible entry
  LogEntry *last_visible_entry( 0 );
  
  // keep track of the current selected entry
  LogEntryList::Item* local_item( ( logEntryList().currentItem<LogEntryList::Item>() ) );
  LogEntry *selected_entry( (local_item) ? local_item->entry():0 );
  
  // retrieve all logbook entries
  KeySet<LogEntry> entries( logbook_->entries() );
  KeySet<LogEntry> turned_off_entries;
  for( KeySet<LogEntry>::iterator it=entries.begin(); it!= entries.end(); it++ )
  {
  
    // retrieve entry
    LogEntry* entry( *it );
    if( entry->keyword() == keyword ) 
    {  
      entry->setKeywordSelected( true );
      if( entry->isFindSelected() ) last_visible_entry = entry;
    } else entry->setKeywordSelected( false );
    
  }
  
  // reinitialize logEntry list
  _resetList();
  
  // if EditFrame current entry is visible, select it;
  if( selected_entry && selected_entry->isSelected() ) logEntryList().selectEntry( selected_entry );
  else if( last_visible_entry ) logEntryList().selectEntry( last_visible_entry );
  
  return;
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
  dialog.addKeyword( *iter );
  dialog.setKeyword( keywordList().currentKeyword() );
  
  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve keyword from line_edit
  string keyword = LogEntry::formatKeyword( dialog.keyword() );
  if( !keyword.empty() ) {
    keywordList().addKeyword( keyword );
    keywordList().selectKeyword( keyword );
  }
}
  
//____________________________________________
void SelectionFrame::_renameKeyword( void )
{
  Debug::Throw("SelectionFrame::_renameKeyword.\n" );
  
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
  string keyword( keywordList().currentKeyword() );
      
  //! create CustomDialog
  EditKeywordDialog dialog( this );
  dialog.setWindowTitle( "Edit keyword" );

  const set<string>& keywords( keywordList().keywords() );
  for( set<string>::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  dialog.addKeyword( *iter );
  dialog.setKeyword( keyword );
  
  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve keyword from line_edit
  _renameKeyword( keyword, LogEntry::formatKeyword( dialog.keyword() ) );
  
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
  string keyword( keywordList().currentKeyword() );

  // retrieve associated entries
  KeySet<LogEntry> entries( logbook_->entries() );
  KeySet<LogEntry> associated_entries;
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  if( (*iter)->keyword().find( keyword ) == 0 )
  associated_entries.insert( *iter );    
                 
  //! create CustomDialog
  DeleteKeywordDialog dialog( this, keyword, associated_entries.size() );
    
  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;
  
  // retrieve parent keyword
  selected_item->setText(0, "");
  string new_keyword( LogEntry::formatKeyword( keywordList().keyword( selected_item ) ) );  

  // move entries
  if( dialog.moveEntries() && associated_entries.size() ) 
  { 

    _renameKeyword( keyword, new_keyword ); 
    return;
    
  } else if( dialog.deleteEntries() ) {
    
    for( KeySet<LogEntry>::iterator iter = associated_entries.begin(); iter != associated_entries.end(); iter++ )
    deleteEntry( *iter, false );
  
  }

  _resetKeywordList();
  keywordList().selectKeyword( new_keyword );

  _resetList();
    
  // Save logbook
  if( !logbook_->file().empty() ) save();  
  
  return;
  
}

//____________________________________________
void SelectionFrame::_renameKeyword( QTreeWidgetItem *item, int column )
{
  Debug::Throw("SelectionFrame::_RenameKeyword.\n" );
  if( !column == KeywordList::KEYWORD ) return;
  
  KeywordList::Item *local_item( dynamic_cast<KeywordList::Item*>(item));
  Exception::assert( local_item, DESCRIPTION( "invalid cast to KeywordList::Item" ) );
  
  string old_keyword( qPrintable( local_item->backup() ) );
  string new_keyword( keywordList().keyword( local_item ) );
  Debug::Throw() << "SelectionFrame::_RenameKeyword - old=" << old_keyword << " new=" << new_keyword << endl;
  
  // check if old differ from new
  _renameKeyword( old_keyword, new_keyword );
  
  return;

}

//____________________________________________
void SelectionFrame::_renameKeyword( const string& keyword, const string& new_keyword )
{
  
  Debug::Throw("SelectionFrame::_renameKeyword.\n" );
  
  // check keywords are different
  if( keyword == new_keyword ) return;
    
  // get entries matching the old_keyword, change the keyword
  KeySet<LogEntry> entries( logbook_->entries() );
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
      entry->modified();
      KeySet<Logbook> logbooks( entry );
      for( KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter!= logbooks.end(); log_iter++ )
      (*log_iter)->setModified( true );
    }
    
  }

  // reset lists
  _resetKeywordList();
  _resetList();
  keywordList().addKeyword( new_keyword );
  keywordList().selectKeyword( new_keyword );
    
  // Save logbook if needed
  if( !logbook_->file().empty() ) save();     
  
}

//_______________________________________________
void SelectionFrame::_changeEntryKeyword( void )
{
  Debug::Throw( "SelectionFrame::_changeEntryKeyword.\n" );  
      
  // check if selected item make sense
  QList<LogEntryList::Item*> items( logEntryList().selectedItems<LogEntryList::Item>() );
  if( items.empty() ) 
  {
    QtUtil::infoDialog( this, "no entry selected. Request canceled.");
    return;
  }
      
  //! create CustomDialog
  CustomDialog dialog( this );
  dialog.setWindowTitle( "Edit keyword" );
  
  QComboBox *combo( new QComboBox( &dialog ) );
  dialog.mainLayout().addWidget( combo );
  combo->setAutoCompletion( true );
  combo->setEditable( true );

  string keyword( keywordList().currentKeyword() );
  const set<string>& keywords( keywordList().keywords() );
  for( set<string>::const_iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  combo->addItem(iter->c_str() );
  combo->setCurrentIndex( combo->findText( keyword.c_str() ) );
  
  dialog.mainLayout().addWidget( new QLabel( "use \"/\" characters to add keyword to a specific branch", &dialog ) );
       
  // map dialog
  QtUtil::centerOnPointer( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve keyword from line_edit
  string new_keyword = LogEntry::formatKeyword( qPrintable( combo->currentText() ) );
  _changeEntryKeyword( new_keyword );
    
}
 
//_______________________________________________
void SelectionFrame::_changeEntryKeyword( const string& new_keyword )
{
      
  // check if selected item make sense
  QList<LogEntryList::Item*> items( logEntryList().selectedItems<LogEntryList::Item>() );
  KeySet<LogEntry> entries;
  
  for( QList<LogEntryList::Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    
    // retrieve entry
    LogEntry* entry( (*iter)->entry() );
    
    // change keyword and set as modified
    entry->setKeyword( new_keyword );
    entry->modified();
    
    // keep track of modified entries
    entries.insert( entry );
    
    // set associated logbooks as modified
    KeySet<Logbook> logbooks( entry );
    for( KeySet<Logbook>::iterator log_iter = logbooks.begin(); log_iter!= logbooks.end(); log_iter++ )
    { (*log_iter)->setModified( true ); }
  
  }  

  // reset lists
  _resetKeywordList();
  _resetList();
  keywordList().addKeyword( new_keyword );
  keywordList().selectKeyword( new_keyword );
  
  // update selection
  logEntryList().clearSelection();
  for( KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    LogEntryList::Item* item( logEntryList().item( *iter ) );
    logEntryList().setItemSelected( item, true );  
  }
  
  // scroll to last item
  if( !entries.empty() ) logEntryList().scrollToItem( *items.begin() );
   
  // Save logbook if needed
  if( !logbook_->file().empty() ) save();
  return;    
  
}

//____________________________________________
void SelectionFrame::_newEntry( void )
{
  
  Debug::Throw( "SelectionFrame::_NewEntry.\n" );

  // create new EditFrame
  EditFrame *frame = new EditFrame( this, 0, false );
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
  _showEditFrame( *iter );

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
   if( !QtUtil::questionDialog( this, "Delete selected entries ?" ) ) return;

  // retrieve associated entry
  for( QList<LogEntryList::Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    LogEntryList::Item* item( *iter );

    KeySet<LogEntry> entries( item );
    Exception::assert( entries.size()==1, DESCRIPTION( "invalid association to LogEntry" ) );
    deleteEntry( *entries.begin(), false );

  }

  // Save logbook if needed
  if( !logbook_->file().empty() ) save();

  return;

}

// //_______________________________________________
// void SelectionFrame::_ChangeEntryColor( const std::string& color )
// {
//   Debug::Throw( "SelectionFrame::_ChangeEntryColor.\n" );
// 
//   // try add color to options
//   list<string> colors( Options::GetSpecialOptions<string>( "COLOR" ) );
//   if( std::find( colors.begin(), colors.end(), color ) == colors.end() )
//   {
//     Options::Get().Add( Option( "COLOR", color ) );
//     if( Options::Get().file().size() ) Options::Get().Write();
//   }
// 
//   // check if selected item make sense
//   QList<LogEntryList::Item*> items( logEntryList().selectedItems<LogEntryList::Item>() );
//   if( items.empty() )
//   {
//     QtUtil::infoDialog( this, "no entry selected. Request canceled.");
//     return;
//   }
// 
//   // retrieve associated entry
//   for( QList<LogEntryList::Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
//   {
//     // get associated entry
//     LogEntryList::Item *item( *iter );
//     LogEntry* entry( item->entry() );
// 
//     entry->SetColor( color );
//     entry->Modified();
// 
//     // redraw item
//     item->repaint();
// 
//     // update EditFrame color
//     KeySet<EditFrame> frames( entry );
//     for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
//     if( !(*iter)->isHidden() ) (*iter)->DisplayColor();
// 
//     // set logbooks as modified
//     KeySet<Logbook> logbooks( entry );
//     for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
//     (*iter)->SetModified( true );
// 
//   }
// 
//   // save Logbook
//   if( GetLogbook()->file().size() ) save();
// 
// }

//_______________________________________________
void SelectionFrame::_autoSave( void )
{

  if( logbook_ && !logbook_->file().empty() ) 
  {
  
    statusBar().label().setText( "performing autoSave" );

    // retrieve non read only editors; perform save
    KeySet<EditFrame> frames( this );
    for( KeySet<EditFrame>::iterator iter = frames.begin(); iter != frames.end(); iter++ )
    if( !( (*iter)->isReadOnly() || (*iter)->isHidden() ) ) (*iter)->saveEntry();

    save();
  
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
void SelectionFrame::_saveForced( void )
{
  Debug::Throw( "SelectionFrame::_saveForced.\n" );

  // retrieve/check SelectionFrame/Logbook
  if( !logbook_ ) {
    QtUtil::infoDialog( this, "no Logbook opened. <Save> canceled." );
    return;
  }

  // set all logbooks as modified
  logbook_->setModifiedRecursive( true );
  save();

}

//_______________________________________________
void SelectionFrame::_resetList( void )
{
  
  Debug::Throw( "SelectionFrame::_resetList.\n" );
      
  // clear list of entries
  logEntryList().clear();
  
  if( !logbook_ ) return;
  KeySet<LogEntry> entries( logbook_->entries() );
  for( KeySet<LogEntry>::iterator it = entries.begin(); it != entries.end(); it++ )
  if( (*it)->isSelected() ) logEntryList().addEntry( *it );
      
}

//_______________________________________________
void SelectionFrame::_resetKeywordList( void )
{
  
  Debug::Throw( "SelectionFrame::_resetKeywordList.\n" );
      
  // clear list of entries
  keywordList().clear();
  
  if( !logbook_ ) return;
  KeySet<LogEntry> entries( logbook_->entries() );
  for( KeySet<LogEntry>::iterator it = entries.begin(); it != entries.end(); it++ )  
  if( (*it)->isFindSelected() ) keywordList().addKeyword( (*it)->keyword() );  
  
}
