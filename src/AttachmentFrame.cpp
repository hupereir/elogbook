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
#include "Command.h"
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
#include "QuestionDialog.h"
#include "Singleton.h"
#include "TreeView.h"
#include "InformationDialog.h"

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
  list().setOptionName( "ATTACHMENTLIST" );
  list().setTextElideMode ( Qt::ElideMiddle );
  
  // install actions
  _installActions();
  
  list().menu().addAction( &newAction() );
  list().menu().addAction( &openAction() );
  list().menu().addAction( &saveAsAction() );
  list().menu().addAction( &editAction() );
  list().menu().addAction( &deleteAction() );
  list().menu().addAction( &reloadAction() );
  list().menu().addSeparator();
  list().menu().addAction( &cleanAction() );
  
  // connections
  connect( &_model(), SIGNAL( layoutAboutToBeChanged() ), SLOT( _storeSelection() ) );
  connect( &_model(), SIGNAL( layoutChanged() ), SLOT( _restoreSelection() ) );
  connect( list().selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions( void ) ) );
  connect( list().selectionModel(), SIGNAL( currentRowChanged(const QModelIndex &, const QModelIndex &) ), SLOT( _itemSelected( const QModelIndex& ) ) );
  connect( &list(), SIGNAL( activated( const QModelIndex& ) ), SLOT( _open( void ) ) );

  connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
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
  
  Debug::Throw( 0, "AttachmentFrame::update.\n" ); 
  assert( attachment.isAssociated( this ) );
  _model().add( &attachment );
  list().resizeColumns();
  Debug::Throw( 0, "AttachmentFrame::update - done.\n" ); 

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
    InformationDialog( this, "No valid entry found. <New Attachment> canceled." ).exec();
    return;
  }
  
  LogEntry *entry( *entries.begin() );
  BASE::KeySet<Logbook> logbooks( entry );
    
  // create dialog
  NewAttachmentDialog dialog( this );
    
  // update destination directory
  if( logbooks.size() && !(*logbooks.begin())->directory().empty() ) { dialog.setDestinationDirectory( (*logbooks.begin())->directory() ); }
  else { 
    MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
    if( mainwindow.logbook() && !mainwindow.logbook()->directory().empty() )
    { dialog.setDestinationDirectory( mainwindow.logbook()->directory() ); }
  }
  
  // type and action
  dialog.setType( AttachmentType::UNKNOWN );
  dialog.setAction( Attachment::COPY_VERSION );
  dialog.resize( 400, 350 );
  if( dialog.centerOnParent().exec() == QDialog::Rejected ) return;
 
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
      InformationDialog( this, o.str().c_str() ).exec();
    
    } else {
  
      // check destination directory
      if( !full_directory.exists() ) 
      {
        ostringstream o; 
        o << "Directory \"" << full_directory << "\" does not exists. Create ?";
        if( QuestionDialog( this, o.str().c_str() ).exec() ) 
        { ( Command( "mkdir" ) << full_directory.c_str() ).run(); }
      }
    }
  }
  
  // retrieve check attachment filename
  File file( dialog.file() );
  if( file.empty() ) 
  {
    InformationDialog( this, "Invalid name. <New Attachment> canceled." ).exec();
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
    InformationDialog( this, o.str().c_str() ).exec();
    delete attachment;
    break;
    
    case Attachment::DEST_NOT_FOUND:
    o << "Cannot find directory \"" << full_directory << "\" - <Add Attachment> canceled.";
    InformationDialog( this, o.str().c_str() ).exec();
    delete attachment;
    break;
    
    case Attachment::SOURCE_IS_DIR:
    o << "File \"" << file << "\" is a directory - <Add Attachment> canceled.";
    InformationDialog( this, o.str().c_str() ).exec();
    delete attachment;
    break;
    
    case Attachment::DEST_EXIST:
    o << "File \"" << file << "\" is allready in list.";
    InformationDialog( this, o.str().c_str() ).exec();
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
    Singleton::get().application<Application>()->attachmentWindow().frame().add( *attachment );
   
    // update logbooks destination directory
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ ) 
    {
      (*iter)->setModified( true );
      (*iter)->setDirectory( full_directory );
    }
    
    // change Application window title
    Singleton::get().application<Application>()->mainWindow().setWindowTitle( Application::MAIN_TITLE_MODIFIED );

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
    MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
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
  
}

//_____________________________________________
void AttachmentFrame::_updateActions( void )
{
  
  bool has_selection( !list().selectionModel()->selectedRows().isEmpty() );
  newAction().setEnabled( !readOnly() );
  openAction().setEnabled( has_selection );
  saveAsAction().setEnabled( has_selection );
  reloadAction().setEnabled( has_selection );
  editAction().setEnabled( has_selection );
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
    InformationDialog( this, "No attachment selected. <Open> canceled.\n" ).exec();
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
      QString buffer;
      QTextStream( &buffer ) << "Cannot find file \"" << fullname.c_str() << "\". <Open> canceled.";
      InformationDialog( this, buffer ).exec();
      continue;
    }
    
    OpenAttachmentDialog dialog( this, attachment );  
    dialog.centerOnParent();
    
    if( dialog.exec() == QDialog::Accepted ) 
    {
      if( dialog.action() == OpenAttachmentDialog::OPEN ) ( Command( dialog.command().c_str() ) << fullname.c_str() ).run();
      else  {
  
        // create and configure SaveAs dialog
        CustomFileDialog dialog( this );
        dialog.setFileMode( QFileDialog::AnyFile );
        dialog.setWindowTitle( "Open" );
        
        File destname( fullname.localName().addPath( dialog.workingDirectory() ) );
        dialog.selectFile( destname.c_str() );
        
        if( dialog.exec() == QDialog::Accepted ) 
        {
          // retrieve selected file
          QStringList files( dialog.selectedFiles() );
          if( files.empty() ) continue;
          
          destname = File( qPrintable( files.front() ) ).expand();
          if( destname.exists() && !QuestionDialog( this, "selected file already exists. Overwrite ?" ).exec() ) return;
          
          // make the copy
          ( Command("cp") << fullname.c_str() << destname.c_str() ).run();
        }
        
      }
      
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
    InformationDialog( this, "No attachment selected. <Edit Attachment> canceled.\n" ).exec();
    return;
  }
 
  // loop over attachments
  AttachmentModel::List modified_attachments;
  for( AttachmentModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {

    // create/check attachment full name
    Attachment& attachment( **iter ); 

    // update time stamps
    bool attachment_changed( false );
    attachment_changed |= attachment.updateTimeStamps();
    EditAttachmentDialog dialog( this, attachment );
  
    // map dialog
    if( dialog.centerOnParent().exec() == QDialog::Accepted )
    {
      
      // change attachment type
      attachment_changed |= attachment.setType( dialog.type() );
      
      // retrieve comments
      attachment_changed |= attachment.setComments( dialog.comments() );
      
      // update time stamps
      attachment_changed |= attachment.updateTimeStamps();
    
    }
    
    if( !attachment_changed ) continue;
    modified_attachments.push_back( &attachment );
          
  }

  // save
  _saveAttachments( modified_attachments );
  
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
    InformationDialog( this, "No attachment selected. <Delete Attachment> canceled.\n" ).exec();
    return;
  }

  // retrieve/check associated EditionWindow/LogEntry
  BASE::KeySet<EditionWindow> windows( this );
  assert( windows.size() == 1 );
  EditionWindow &window( **windows.begin() );

  // loop over attachments
  bool logbook_changed( false );
  for( AttachmentModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    
    Attachment *attachment( *iter ); 
  
    // dialog
    DeleteAttachmentDialog dialog( this, *attachment );
    dialog.centerOnParent();

    if( dialog.exec() == QDialog::Accepted ) 
    {
    
      logbook_changed = true;
      
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
          
          InformationDialog( this, "Attachment still in use by other entries. Kept on disk." ).exec();
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
    
  }
  
  if( logbook_changed )
  {
    Singleton::get().application<Application>()->mainWindow().setWindowTitle( Application::MAIN_TITLE_MODIFIED );
    window.saveAction().trigger();
    
    // resize columns
    list().resizeColumns();
  }
  
  return;      
  
}

//_____________________________________________
void AttachmentFrame::_reload( void )
{
  Debug::Throw( "AttachmentFrame::_reload.\n" );

  // get selection
  AttachmentModel::List selection( _model().get( list().selectionModel()->selectedRows() ) );
 
  // check items
  if( selection.empty() ) 
  {
    InformationDialog( this, "No attachment selected. <Save As> canceled.\n" ).exec();
    return;
  }
  
  // loop over attachments
  AttachmentModel::List modified_attachments;
  for( AttachmentModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  { if ( (*iter)->updateTimeStamps() ) { modified_attachments.push_back( *iter ); } }

  _saveAttachments( modified_attachments );
  
}

//_____________________________________________
void AttachmentFrame::_saveAs( void )
{
  Debug::Throw( "AttachmentFrame::_saveAs.\n" );

  // get selection
  AttachmentModel::List selection( _model().get( list().selectionModel()->selectedRows() ) );
 
  // check items
  if( selection.empty() ) 
  {
    InformationDialog( this, "No attachment selected. <Save As> canceled.\n" ).exec();
    return;
  }
 
  // loop over attachments
  for( AttachmentModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    
    Attachment& attachment( **iter );     
    AttachmentType type = attachment.type();
    File fullname( ( type == AttachmentType::URL ) ? attachment.file():attachment.file().expand() );
    if( type == AttachmentType::URL ) 
    {

      QString buffer;
      QTextStream( &buffer ) << "Selected attachement is URL. <Save As> canceled.";
      InformationDialog( this, buffer ).exec();
      continue;
      
    } else if( !fullname.exists() ) {
      
      QString buffer;
      QTextStream( &buffer ) << "Cannot find file \"" << fullname.c_str() << "\". <Save As> canceled.";
      InformationDialog( this, buffer ).exec();
      continue;
      
    } else {
      
      // create and configure SaveAs dialog
      CustomFileDialog dialog( this );
      dialog.setFileMode( QFileDialog::AnyFile );
      dialog.setWindowTitle( "Open" );
      
      File destname( fullname.localName().addPath( dialog.workingDirectory() ) );
      dialog.selectFile( destname.c_str() );
        
      if( dialog.exec() == QDialog::Accepted ) 
      {
        // retrieve selected file
        QStringList files( dialog.selectedFiles() );
        if( files.empty() ) continue;
        
        destname = File( qPrintable( files.front() ) ).expand();
        if( destname.exists() && !QuestionDialog( this, "selected file already exists. Overwrite ?" ).exec() ) return;
        
        // make the copy
        ( Command("cp") << fullname.c_str() << destname.c_str() ).run();
      
      }
    
    }
    
  }
  
  return;
  
}


//_________________________________________________________________________
void AttachmentFrame::_clean( void )
{ 
  Debug::Throw( "AttachmentFrame::clean.\n" ); 

  // ask for confirmation
  if( !QuestionDialog( this, "Remove all invalid attachments ?" ).exec() ) return;
  
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
    MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
    mainwindow.setWindowTitle( Application::MAIN_TITLE_MODIFIED );
    if( mainwindow.logbook()->file().size() ) mainwindow.save();
    
  }

}
  
//_________________________________________________________________________
void AttachmentFrame::_itemSelected( const QModelIndex& index )
{
  if( !index.isValid() ) return;
  Attachment& attachment( *model_.get( index ) );
  Debug::Throw() << "AttachmentFrame::_itemSelected - " << attachment.file() << endl;
  emit attachmentSelected( attachment );
  
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

  addAction( visibility_action_ = new QAction( IconEngine::get( ICONS::ATTACH ), "Show &Attachment List", this ) );
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

  addAction( reload_action_ = new QAction( IconEngine::get( ICONS::RELOAD ), "&Reload", this ) );
  reloadAction().setShortcut( Qt::Key_F5 );
  reloadAction().setToolTip( "Reload attachments timestamps" );
  connect( &reloadAction(), SIGNAL( triggered() ), SLOT( _reload() ) );

  addAction( save_as_action_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), "&Save As", this ) );
  saveAsAction().setToolTip( "Save selected attachment with a different filename" );
  connect( &saveAsAction(), SIGNAL( triggered() ), SLOT( _saveAs() ) );
  
  
  clean_action_ = new QAction( IconEngine::get( ICONS::DELETE ), "&Clean", this );
  cleanAction().setToolTip( "Delete selected attachments" );
  connect( &cleanAction(), SIGNAL( triggered() ), SLOT( _clean() ) );  
}

//_______________________________________________________________________
void AttachmentFrame::_saveAttachments( const AttachmentModel::List& attachments )
{
  Debug::Throw( 0, "AttachmentFrame::_saveAttachments.\n" );
  if( attachments.empty() ) return;
  
  // associated lists
  BASE::KeySet<LogEntry> entries;
  
  // loop over attachments
  for( AttachmentModel::List::const_iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  {
    
    // get associated entries and store
    BASE::KeySet<LogEntry> local_entries( *iter );
    entries.insert( local_entries.begin(), local_entries.end() );
    Debug::Throw( "AttachmentFrame::_saveAttachments - entries.\n" );
    
    // get associated attachment frames and store
    BASE::KeySet<AttachmentFrame> local_frames( *iter );
    for( BASE::KeySet<AttachmentFrame>::iterator frame_iter = local_frames.begin(); frame_iter != local_frames.end(); frame_iter++ )
    { (*frame_iter)->update( **iter ); } 
    Debug::Throw( "AttachmentFrame::_saveAttachments - frames.\n" );
  
  }
    
  BASE::KeySet<Logbook> logbooks;
  BASE::KeySet<EditionWindow> edition_windows;
  
  // loop over entries
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  {
    
    // get associated logbooks and store
    BASE::KeySet<Logbook> local_logbooks( *iter );
    logbooks.insert( local_logbooks.begin(), local_logbooks.end() );
    Debug::Throw( "AttachmentFrame::_saveAttachments - logbooks.\n" );
    
    // get associated Edition windows
    BASE::KeySet<EditionWindow> local_edition_windows( *iter );
    edition_windows.insert( local_edition_windows.begin(), local_edition_windows.end() );
    Debug::Throw( "AttachmentFrame::_saveAttachments - edition windows.\n" );
    
  }
       
  // loop over logbook and set modified
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter!= logbooks.end(); iter++ )
  { (*iter)->setModified( true ); }
  Debug::Throw( "AttachmentFrame::_saveAttachments - logbooks modified.\n" );

  // loop over edition windows and trigger save action
  for( BASE::KeySet<EditionWindow>::iterator iter = edition_windows.begin(); iter != edition_windows.end(); iter++ )
  { (*iter)->saveAction().trigger();  }  
  Debug::Throw( "AttachmentFrame::_saveAttachments - edition windows saved.\n" );
  
  MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
  if( mainwindow.logbook()->file().size() ) mainwindow.save();
  Debug::Throw( "AttachmentFrame::_saveAttachments - logbook saved.\n" );

}
