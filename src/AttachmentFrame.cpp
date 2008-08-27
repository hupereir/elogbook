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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License        
* for more details.                    
*                         
* You should have received a copy of the GNU General Public License along with 
* software; if not, write to the Free Software Foundation, Inc., 59 Temple    
* Place, Suite 330, Boston, MA  02111-1307 USA                          
*                        
*                        
*******************************************************************************/

/*!
  \file  AttachmentFrame.cpp
  \brief  customized list view to handle attachment 
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <list>
#include <sstream>
#include <QHeaderView>
#include <QShortcut>

#include "Application.h"
#include "Attachment.h"
#include "AttachmentWindow.h"
#include "AttachmentFrame.h"
#include "CustomFileDialog.h"
#include "Debug.h"
#include "DeleteAttachmentDialog.h"
#include "EditAttachmentDialog.h"
#include "EditionWindow.h"
#include "File.h"
#include "FileRecord.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "MainWindow.h"
#include "NewAttachmentDialog.h"
#include "OpenAttachmentDialog.h"
#include "TreeView.h"
#include "QtUtil.h"
#include "Util.h"

using namespace std;

//_____________________________________________
AttachmentFrame::AttachmentFrame( QWidget *parent, bool read_only ):
  QWidget( parent ),
  read_only_( read_only ),
  default_height_( -1 ),
  thread_( this )
{ 
  Debug::Throw( "AttachmentFrame::AttachmentFrame.\n" ); 

  // tell validFile thread not to check duplicates
  // this is needed when checking files that are links
  thread_.setCheckDuplicates( false );
  
  // default layout
  setLayout( new QVBoxLayout() );
  layout()->setMargin(0);
  layout()->setSpacing(5);
  
  // create list
  layout()->addWidget( list_ = new TreeView( this ) );
  list().setModel( &_model() );
  list().setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  list().setMaskOptionName( "ATTACHMENTLIST_MASK" );
  list().setTextElideMode ( Qt::ElideMiddle );
  
  // install actions
  _installActions();
  
  list().menu().addAction( &newAction() );
  list().menu().addAction( &openAction() );
  list().menu().addAction( &editAction() );
  list().menu().addAction( &deleteAction() );
  list().menu().addSeparator();
  list().menu().addAction( &cleanAction() );
  
  // connections
  connect( &_model(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), SLOT( _restoreSelection() ) );
  connect( list().header(), SIGNAL( sortIndicatorChanged( int, Qt::SortOrder ) ), SLOT( _storeSortMethod( int, Qt::SortOrder ) ) );
  connect( list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions( void ) ) );
  connect( list().selectionModel(), SIGNAL( currentRowChanged(const QModelIndex &, const QModelIndex &) ), SLOT( _itemSelected( const QModelIndex& ) ) );
  connect( &list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _open( void ) ) );

  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  _updateConfiguration();
  _updateActions();
}


//______________________________________________________________________
void AttachmentFrame::setDefaultHeight( const int& value )
{ default_height_ = value; }

//____________________________________________
QSize AttachmentFrame::sizeHint( void ) const
{ return (default_height_ ) >= 0 ? QSize( 0, default_height_ ):QWidget::sizeHint(); }

//_____________________________________________
void AttachmentFrame::add( const AttachmentModel::List& attachments )
{

  Debug::Throw( "AttachmentFrame::add.\n" ); 
  for( AttachmentModel::List::const_iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  { BASE::Key::associate( this, *iter ); }
  
  _model().add( attachments );
  list().resizeColumns();
  
}

//_____________________________________________
void AttachmentFrame::update( Attachment& attachment )
{
  
  Debug::Throw( "AttachmentFrame::update.\n" ); 
  assert( attachment.isAssociated( this ) );
  _model().add( &attachment );

}

//_____________________________________________
void AttachmentFrame::select( Attachment& attachment )
{
  
  Debug::Throw( "AttachmentFrame::select.\n" ); 
  assert( attachment.isAssociated( this ) );
    
  // get matching model index
  QModelIndex index( _model().index( &attachment ) );

  // check if index is valid and not selected
  if( ( !index.isValid() ) || list().selectionModel()->isSelected( index ) ) return;
  
  // select
  list().selectionModel()->select( index,  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
  
  return;
    
}
  
//_____________________________________________
void AttachmentFrame::_new( void )
{ 
  
  Debug::Throw( "AttachmentFrame::_new.\n" );  
  
  // retrieve/check associated EditionWindow/LogEntry
  BASE::KeySet<EditionWindow> windows( this );
  assert( windows.size() == 1 );
  
  EditionWindow &window( **windows.begin() );
  
  BASE::KeySet<LogEntry> entries( window );
  if( entries.size() != 1 )
  {
    QtUtil::infoDialog( this, "No valid entry found. <New Attachment> canceled." );
    return;
  }
  
  LogEntry *entry( *entries.begin() );
  BASE::KeySet<Logbook> logbooks( entry );
    
  // create dialog
  NewAttachmentDialog dialog( this );
    
  // update destination directory
  if( logbooks.size() && !(*logbooks.begin())->directory().empty() )
  dialog.setDestinationDirectory( (*logbooks.begin())->directory() );
  
  // type and action
  dialog.setType( AttachmentType::UNKNOWN );
  dialog.setAction( Attachment::COPY_VERSION );
  
  // map dialog
  QtUtil::centerOnParent( &dialog );
  dialog.resize( 400, 350 );
  if( dialog.exec() == QDialog::Rejected ) return;
 
  // retrieve Attachment type
  AttachmentType type( dialog.type() );
  
  // retrieve destination directory
  File full_directory = dialog.destinationDirectory();  
  
  // check destination directory (if file is not URL)
  if( !(type == AttachmentType::URL) )
  {
    // check if destination directory is not a non directory existsing file
    if( full_directory.exists() && !full_directory.isDirectory() ) 
    { 
    
      ostringstream o;
      o << "File \"" << full_directory << "\" is not a directory.";
      QtUtil::infoDialog( this, o.str() );
    
    } else {
  
      // check destination directory
      if( !full_directory.exists() ) {
        ostringstream o; 
        o << "Directory \"" << full_directory << "\" does not exists. Create ?";
        if( QtUtil::questionDialog( this, o.str() ) ) 
        { Util::run( QStringList() << "mkdir" << full_directory.c_str() ); }
      }
    }
  }
  
  // retrieve check attachment filename
  File file( dialog.file() );
  if( file.empty() ) 
  {
    QtUtil::infoDialog( this, "Invalid name. <New Attachment> canceled." );
    return;
  }

  // create attachment with correct type
  Attachment *attachment = new Attachment( file, type );
      
  // retrieve check comments
  attachment->setComments( dialog.comments() );
  
  // retrieve command
  Attachment::Command command( dialog.action() );
  
  // process attachment command
  Attachment::ErrorCode error = attachment->copy( command, full_directory );
  ostringstream o;
  switch (error) 
  {
  
    case Attachment::SOURCE_NOT_FOUND:
    o << "Cannot find file \"" << file << "\" - <Add Attachment> canceled.";
    QtUtil::infoDialog( this, o.str() );
    delete attachment;
    break;
    
    case Attachment::DEST_NOT_FOUND:
    o << "Cannot find directory \"" << full_directory << "\" - <Add Attachment> canceled.";
    QtUtil::infoDialog( this, o.str() );
    delete attachment;
    break;
    
    case Attachment::SOURCE_IS_DIR:
    o << "File \"" << file << "\" is a directory - <Add Attachment> canceled.";
    QtUtil::infoDialog( this, o.str() );
    delete attachment;
    break;
    
    case Attachment::DEST_EXIST:
    o << "File \"" << file << "\" is allready in list.";
    QtUtil::infoDialog( this, o.str() );
    delete attachment;
    break;
    
    case Attachment::SUCCESS:  

    // associate attachment to entry
    Key::associate( entry, attachment );
    
    // update all windows edition windows associated to entry
    windows = BASE::KeySet<EditionWindow>( entry );
    for( BASE::KeySet<EditionWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ )
    {
      
      (*iter)->attachmentFrame().visibilityAction().setChecked( true );
      (*iter)->attachmentFrame().add( *attachment );
      
    }
    
    // update attachment frame
    static_cast<Application*>(qApp)->attachmentWindow().frame().add( *attachment );
   
    // update logbooks destination directory
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ ) 
    {
      (*iter)->setModified( true );
      (*iter)->setDirectory( full_directory );
    }
    
    // change Application window title
    static_cast<Application*>(qApp)->mainWindow().setWindowTitle( Application::MAIN_TITLE_MODIFIED );

    // save EditionWindow entry
    window.saveAction().trigger();
    
    break;
    
    default: delete attachment; break;
    
  }
    
}  

//_____________________________________________
void AttachmentFrame::enterEvent( QEvent* event )
{
  
  Debug::Throw( "AttachmentFrame::enterEvent.\n" );
  if( thread_.isRunning() || !hasList() ) return;
  
  // create file records
  FileRecord::List records;

  // retrieve all attachments from model
  AttachmentModel::List attachments( _model().get() );
  for( AttachmentModel::List::iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  {
    
    assert( *iter );
    Attachment &attachment( **iter );
    
    if( attachment.type() == AttachmentType::URL ) continue;
    if( attachment.file().empty() ) continue;
    
    records.push_back( FileRecord( attachment.file() ) );
    
    if( attachment.isLink() == Attachment::YES || attachment.isLink() == Attachment::UNKNOWN )
    { records.push_back( FileRecord( attachment.sourceFile() ) ); }
    
  }
  
  // setup thread and start
  thread_.setRecords( records );
  thread_.start();
  
  return QWidget::enterEvent( event );
  
}

//_______________________________________________ 
void AttachmentFrame::customEvent( QEvent* event )
{
  
  if( event->type() != QEvent::User ) return QWidget::customEvent( event ); 
  
  ValidFileEvent* valid_file_event( dynamic_cast<ValidFileEvent*>(event) );
  if( !valid_file_event ) return QWidget::customEvent( event );
  
  Debug::Throw() << "AttachmentFrame::customEvent." << endl;
  
  // set file records validity
  const FileRecord::List& records( valid_file_event->records() ); 

  // retrieve all attachments from model
  // true if some modifications are to be saved
  bool modified( false );
  AttachmentModel::List attachments( _model().get() );
  for( AttachmentModel::List::iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  {
    
    assert( *iter );
    Attachment &attachment( **iter );
        
    if( attachment.type() == AttachmentType::URL ) continue;
    if( attachment.file().empty() ) continue;
    
    Debug::Throw() << "AttachmentFrame::customEvent - checking: " << attachment.file() << endl;
    
    bool is_valid( attachment.isValid() );
    Attachment::LinkState is_link( attachment.isLink() );

    // check destination file
    FileRecord::List::const_iterator found = find_if( 
      records.begin(),
      records.end(), 
      FileRecord::SameFileFTor( attachment.file() ) );
    if( found != records.end() ) { is_valid = found->isValid(); }
    else { Debug::Throw() << "AttachmentFrame::customEvent - not found." << endl; }
    
    // check link status
    if( is_valid && is_link == Attachment::UNKNOWN )
    {
      // check if destination is a link
      QFileInfo file_info( attachment.file().c_str() );
      is_link = file_info.isSymLink() ? Attachment::YES : Attachment::NO;
    }
    
    // check source file
    if( is_valid && is_link == Attachment::YES )
    {
      found = find_if( 
        records.begin(),
        records.end(), 
        FileRecord::SameFileFTor( attachment.sourceFile() ) );
      if( found != records.end() ) { is_valid &= found->isValid(); }
      else { Debug::Throw() << "AttachmentFrame::customEvent - not found." << endl; }
    }
   
    // update validity flag and set parent logbook as modified if needed
    Debug::Throw() << "AttachmentFrame::customEvent - valid: " << is_valid << " link: " << is_link << endl;
    if( attachment.setIsValid( is_valid ) || attachment.setIsLink( is_link ) )
    {
      
      // get associated entry
      BASE::KeySet<LogEntry> entries( &attachment );
      assert( entries.size() == 1 );
      LogEntry& entry( **entries.begin() );
      entry.modified();
      
      // get associated logbooks
      BASE::KeySet<Logbook> logbooks( &entry );
      for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter!= logbooks.end(); iter++ )
      { (*iter)->setModified( true ); }
      
      modified = true;
      
    }
    
    // update attachment size
    attachment.updateSize();

  }
  
  // save logbooks
  if( modified ) 
  {
        
    // set main window title
    MainWindow& mainwindow( static_cast<Application*>(qApp)->mainWindow() );
    mainwindow.setWindowTitle( Application::MAIN_TITLE_MODIFIED );
    if( mainwindow.logbook()->file().size() ) mainwindow.save();
    
  }
  
  cleanAction().setEnabled( valid_file_event->hasInvalidRecords() );
  return QWidget::customEvent( event ); 

}

//_____________________________________________
void AttachmentFrame::_updateConfiguration( void )
{
  
  Debug::Throw( "AttachmentFrame::_updateConfiguration.\n" );
  int icon_size( XmlOptions::get().get<int>( "ATTACHMENT_LIST_ICON_SIZE" ) );
  list().setIconSize( QSize( icon_size, icon_size ) );
  
  // session files list sorting
  if( XmlOptions::get().find( "ATTACHMENT_LIST_SORT_COLUMN" ) && XmlOptions::get().find( "ATTACHMENT_LIST_SORT_ORDER" ) )
  { 
    
    list().sortByColumn( 
      XmlOptions::get().get<int>( "ATTACHMENT_LIST_SORT_COLUMN" ), 
      (Qt::SortOrder) XmlOptions::get().get<int>( "ATTACHMENT_LIST_SORT_ORDER" ) ); 
  }
  
}

//______________________________________________________________________
void AttachmentFrame::_storeSortMethod( int column, Qt::SortOrder order )
{
  
  Debug::Throw( "SessionFilesFrame::_storeSortMethod.\n" );
  XmlOptions::get().set<int>( "ATTACHMENT_LIST_SORT_COLUMN", column );
  XmlOptions::get().set<int>( "ATTACHMENT_LIST_SORT_ORDER", order );
  
}

//_____________________________________________
void AttachmentFrame::_updateActions( void )
{
  
  bool has_selection( !list().selectionModel()->selectedRows().isEmpty() );
  newAction().setEnabled( !readOnly() );
  openAction().setEnabled( has_selection );
  editAction().setEnabled( has_selection && !readOnly() );
  deleteAction().setEnabled( has_selection && !readOnly() );
  return;
  
}

//_____________________________________________
void AttachmentFrame::_open( void )
{
  Debug::Throw( "AttachmentFrame::_open.\n" );  

  // get selection
  AttachmentModel::List selection( _model().get( list().selectionModel()->selectedRows() ) );
 
  // check items
  if( selection.empty() ) 
  {
    QtUtil::infoDialog( this, "No attachment selected. <Open Attachment> canceled.\n" );
    return;
  }
 
  // loop over attachments
  for( AttachmentModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    
    Attachment& attachment( **iter ); 
    AttachmentType type = attachment.type();
    File fullname( ( type == AttachmentType::URL ) ? attachment.file():attachment.file().expand() );
    if( !( type == AttachmentType::URL || fullname.exists() ) )
    {
      ostringstream what; 
      what << "Cannot find file \"" << fullname << "\". <Open Attachment> canceled.";
      QtUtil::infoDialog( this, what.str() );
      return;
    }
    
    OpenAttachmentDialog dialog( this, attachment );  
    QtUtil::centerOnParent( &dialog );
    
    if( dialog.exec() == QDialog::Rejected ) return;
    if( dialog.action() == OpenAttachmentDialog::OPEN )
    { Util::run( QStringList() << dialog.command().c_str() << fullname.c_str() ); } 
    else 
    {
  
      // create and configure SaveAs dialog
      CustomFileDialog dialog( this );
      dialog.setFileMode( QFileDialog::AnyFile );
      
      File destname( fullname.localName().addPath( dialog.workingDirectory() ) );
      dialog.selectFile( destname.c_str() );
      
      if( dialog.exec() == QDialog::Rejected ) return;
      
      // retrieve selected file
      QStringList files( dialog.selectedFiles() );
      if( files.empty() ) return;
      
      destname = File( qPrintable( files.front() ) ).expand();
      if( destname.exists() && !QtUtil::questionDialog( this, "selected file already exists. Overwrite ?" ) ) return;
      
      // make the copy
      Util::run( QStringList() << "cp" << fullname.c_str() << destname.c_str() );
      
    }
  
  }
  
  return;
  
}  

//_____________________________________________
void AttachmentFrame::_edit( void )
{
  Debug::Throw( "AttachmentFrame::_edit.\n" );
  
  // store selected item locally
  // get selection
  AttachmentModel::List selection( _model().get( list().selectionModel()->selectedRows() ) );
 
  // check items
  if( selection.empty() ) 
  {
    QtUtil::infoDialog( this, "No attachment selected. <Edit Attachment> canceled.\n" );
    return;
  }
 
  // retrieve/check associated EditionWindow/LogEntry
  BASE::KeySet<EditionWindow> windows( this );
  assert( windows.size() == 1 );
  EditionWindow &window( **windows.begin() );

  // loop over attachments
  for( AttachmentModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {

    // create/check attachment full name
    Attachment& attachment( **iter ); 
    EditAttachmentDialog dialog( this, attachment );
  
    // map dialog
    QtUtil::centerOnParent( &dialog );
    if( dialog.exec() == QDialog::Rejected ) return;
  
    // change attachment type
    attachment.setType( dialog.type() );
  
    // retrieve comments
    attachment.setComments( dialog.comments() );
  
    // set associated entry modified
    BASE::KeySet<LogEntry> entries( &attachment );
    assert( entries.size() == 1 );

    // update attachment associated list items
    BASE::KeySet<AttachmentFrame> frames( &attachment );
    for( BASE::KeySet<AttachmentFrame>::const_iterator iter = frames.begin(); iter != frames.end(); iter++ )
    { (*iter)->update( attachment ); }
          
  }
  
  // set main window title
  static_cast<Application*>(qApp)->mainWindow().setWindowTitle( Application::MAIN_TITLE_MODIFIED );
  
  // save EditionWindow entry
  window.saveAction().trigger();
  
}

//_____________________________________________
void AttachmentFrame::_delete( void )
{
  Debug::Throw( "AttachmentFrame::_delete.\n" );
  
  // store selected item locally
  // get selection
  AttachmentModel::List selection( _model().get( list().selectionModel()->selectedRows() ) );
 
  // check items
  if( selection.empty() ) 
  {
    QtUtil::infoDialog( this, "No attachment selected. <Delete Attachment> canceled.\n" );
    return;
  }

  // retrieve/check associated EditionWindow/LogEntry
  BASE::KeySet<EditionWindow> windows( this );
  assert( windows.size() == 1 );
  EditionWindow &window( **windows.begin() );

  // loop over attachments
  for( AttachmentModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    
    Attachment *attachment( *iter ); 
  
    // dialog
    DeleteAttachmentDialog dialog( this, *attachment );
    QtUtil::centerOnParent( &dialog );

    if( dialog.exec() == QDialog::Rejected ) return;    
    
    // retrieve action
    bool from_disk( dialog.action() == DeleteAttachmentDialog::FROM_DISK );
    
    // retrieve associated attachment frames and remove item
    BASE::KeySet<AttachmentFrame> frames( attachment );
    for( BASE::KeySet<AttachmentFrame>::const_iterator iter = frames.begin(); iter != frames.end(); iter++ ) 
    { (*iter)->_model().remove( attachment ); }
  
    // retrieve associated entries
    BASE::KeySet<LogEntry> entries( attachment );
    assert( entries.size() == 1 );
    LogEntry& entry( **entries.begin() );
    entry.modified();
    
    // retrieve associated logbooks
    BASE::KeySet<Logbook> logbooks( &entry );
    
    // check sharing attachments to avoid from_disk deletion
    if( from_disk && logbooks.size() ) 
    {
      
      BASE::KeySet<Attachment> attachments( (*logbooks.begin())->attachments() );
      unsigned int n_share = count_if( attachments.begin(), attachments.end(), Attachment::SameFileFTor( attachment ) ); 
      if( n_share > 1 ) {
        
        QtUtil::infoDialog( this, "Attachment still in use by other entries. Kept on disk." );
        from_disk = false;
        
      }
    
    }
  
    // remove file from disk, if required
    File file( attachment->file().expand() );
    if( from_disk && ( !( attachment->type() == AttachmentType::URL ) ) && file.isWritable() ) 
    { file.remove(); }
    
    // delete attachment
    delete attachment;
       
  }
  
  // set main window title
  static_cast<Application*>(qApp)->mainWindow().setWindowTitle( Application::MAIN_TITLE_MODIFIED );
  
  // save EditionWindow entry
  window.saveAction().trigger();

  return;      
  
}

//_________________________________________________________________________
void AttachmentFrame::_clean( void )
{ 
  Debug::Throw( "AttachmentFrame::clean.\n" ); 

  // ask for confirmation
  if( !QtUtil::questionDialog( this, "Remove all invalid attachments ?" ) ) return;
  
  // retrieve all attachments from model
  // true if some modifications are to be saved
  bool modified( false );
  AttachmentModel::List attachments( _model().get() );
  for( AttachmentModel::List::iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  {
    
    assert( *iter );
    Attachment *attachment( *iter );
        
    // skip attachment if valid
    if( attachment->isValid() ) continue;
    
    Debug::Throw() << "AttachmentFrame::_clean - removing: " << attachment->file() << endl;
    
    // retrieve associated attachment frames and remove item
    BASE::KeySet<AttachmentFrame> frames( attachment );
    for( BASE::KeySet<AttachmentFrame>::const_iterator iter = frames.begin(); iter != frames.end(); iter++ ) 
    { (*iter)->_model().remove( attachment ); }
    
    // retrieve associated entries
    BASE::KeySet<LogEntry> entries( attachment );
    assert( entries.size() == 1 );
    LogEntry& entry( **entries.begin() );
    entry.modified();
    
    // retrieve associated logbooks
    BASE::KeySet<Logbook> logbooks( &entry );
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter!= logbooks.end(); iter++ )
    { (*iter)->setModified( true ); }

    // delete attachment
    delete attachment;
    modified = true;
  }
  
  if( modified ) 
  {
   
    // set main window title
    MainWindow& mainwindow( static_cast<Application*>(qApp)->mainWindow() );
    mainwindow.setWindowTitle( Application::MAIN_TITLE_MODIFIED );
    if( mainwindow.logbook()->file().size() ) mainwindow.save();
    
  }

}
  
//_________________________________________________________________________
void AttachmentFrame::_itemSelected( const QModelIndex& index )
{
  Debug::Throw( "AttachmentFrame::_itemSelected.\n" );
  if( !index.isValid() ) return;
  emit attachmentSelected( *model_.get( index ) );
  
}
  
//______________________________________________________________________
void AttachmentFrame::_storeSelection( void )
{ 
  Debug::Throw( "AttachmentFrame::_storeSelection.\n" ); 

  // clear
  _model().clearSelectedIndexes();
  
  // retrieve selected indexes in list
  QModelIndexList selected_indexes( list().selectionModel()->selectedRows() );
  for( QModelIndexList::iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
  { 
    // check column
    if( !iter->column() == 0 ) continue;
    _model().setIndexSelected( *iter, true ); 
  }

  return;
  
}

//______________________________________________________________________
void AttachmentFrame::_restoreSelection( void )
{ 
  
  Debug::Throw( "AttachmentFrame::_restoreSelection.\n" ); 

  // retrieve indexes
  QModelIndexList selected_indexes( _model().selectedIndexes() );
  if( selected_indexes.empty() ) list().selectionModel()->clear();
  else {
    
    list().selectionModel()->select( selected_indexes.front(),  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
    for( QModelIndexList::const_iterator iter = selected_indexes.begin(); iter != selected_indexes.end(); iter++ )
    { list().selectionModel()->select( *iter, QItemSelectionModel::Select|QItemSelectionModel::Rows ); }
  
  }
  
  return;
  
}

//_______________________________________________________________________
void AttachmentFrame::_installActions( void )
{
  Debug::Throw( "AttachmentFrame::_installActions.\n" );

  addAction( visibility_action_ = new QAction( IconEngine::get( ICONS::ATTACH ), "Show &Attachment list", this ) );
  visibilityAction().setToolTip( "Show/hide attachment list" );
  visibilityAction().setCheckable( true );
  visibilityAction().setChecked( true );
  connect( &visibilityAction(), SIGNAL( toggled( bool ) ), SLOT( setVisible( bool ) ) );
  
  addAction( new_action_ = new QAction( IconEngine::get( ICONS::ATTACH ), "&New", this ) );
  newAction().setToolTip( "Attach a file/URL to the current entry" );
  connect( &newAction(), SIGNAL( triggered() ), SLOT( _new() ) );
  
  addAction( open_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open", this ) );
  openAction().setToolTip( "Open selected attachments" );
  connect( &openAction(), SIGNAL( triggered() ), SLOT( _open() ) );
     
  addAction( edit_action_ = new QAction( IconEngine::get( ICONS::EDIT ), "&Edit", this ) );
  editAction().setToolTip( "Edit selected attachments informations" );
  connect( &editAction(), SIGNAL( triggered() ), SLOT( _edit() ) );

  addAction( delete_action_ = new QAction( IconEngine::get( ICONS::DELETE ), "&Delete", this ) );
  deleteAction().setShortcut( Qt::Key_Delete );
  deleteAction().setToolTip( "Delete selected attachments" );
  connect( &deleteAction(), SIGNAL( triggered() ), SLOT( _delete() ) );

  clean_action_ = new QAction( IconEngine::get( ICONS::DELETE ), "&Clean", this );
  cleanAction().setToolTip( "Delete selected attachments" );
  connect( &cleanAction(), SIGNAL( triggered() ), SLOT( _clean() ) );  
}
