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
  read_only_( read_only )
{ 
  Debug::Throw( "AttachmentFrame::AttachmentFrame.\n" ); 

  // default layout
  setLayout( new QVBoxLayout() );
  layout()->setMargin(0);
  layout()->setSpacing(5);
  
  // create list
  layout()->addWidget( list_ = new TreeView( this ) );
  _list().setModel( &_logEntryModel() );
  _list().setSelectionMode( QAbstractItemView::ContiguousSelection ); 
  _list().setMaskOptionName( "ATTACHMENT_LIST_MASK" );
  _list().setTextElideMode ( Qt::ElideMiddle );
  
  // install actions
  _installActions();
  
  _list().menu().addAction( &newAttachmentAction() );
  _list().menu().addSeparator();
  _list().menu().addAction( &openAttachmentAction() );
  _list().menu().addAction( &editAttachmentAction() );
  _list().menu().addAction( &deleteAttachmentAction() );
  
  // connections
  // connect( this, SIGNAL( itemActivated( QTreeWidgetItem*, int ) ), SLOT( _open() ) );
  // connect( this, SIGNAL( itemSelectionChanged() ), SLOT( _updateActions() ) );
  _updateActions();
}

//_____________________________________________
void AttachmentFrame::add( Attachment& attachment )
{

  Debug::Throw( "AttachmentFrame::add.\n" ); 
  BASE::Key::associate( this, &attachment );
  _model().add( &attachment );
  
}

//_____________________________________________
void AttachmentFrame::update( Attachment& attachment )
{
  
  Debug::Throw( "AttachmentFrame::update.\n" ); 
  assert( BASE::Key::isAssociated( &attachment, this ) );
  _model().add( &attachment );

}

//_____________________________________________
void AttachmentFrame::select( Attachment& attachment )
{
  
  Debug::Throw( "AttachmentFrame::SelectAttachment.\n" ); 
  assert( BASE::Key::isAssociated( &attachment, this ) );
    
  // get matching model index
  QModelIndex index( _model().get( &attachment ) );

  // check if index is valid and not selected
  if( ( !index.isValid() ) || list().selectionModel()->isSelected( index ) ) return;
  
  // select
  _list().selectionModel()->select( index,  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );
  
  return;
    
}
  
//_____________________________________________
void AttachmentFrame::_new( void )
{ 
  
  Debug::Throw( "AttachmentFrame::_new.\n" );  
  
  // retrieve/check associated EditionWindow/LogEntry
  BASE::KeySet<EditionWindow> windows( this );
  assert( windows.size() == 1 );
  
  EditionWindow &window( *windows.begin() );
  
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
    
    // update all windows AttachmentFrame associated to entry
    windows = BASE::KeySet<EditionWindow>( entry );
    for( BASE::KeySet<EditionWindow>::iterator iter = windows.begin(); iter != windows.end(); iter++ )
    {
      if( !(*iter)->attachmentList().topLevelItemCount() ) (*iter)->attachmentList().show();
      (*iter)->attachmentList().add( attachment );
      (*iter)->attachmentList().resizeColumns();
    }
    
    // update attachment frame
    static_cast<Application*>(qApp)->attachmentWindow().list().add( attachment );
    static_cast<Application*>(qApp)->attachmentWindow().list().resizeColumns();
   
    // update logbooks destination directory
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ ) 
    {
      (*iter)->setModified( true );
      (*iter)->setDirectory( full_directory );
    }
    
    // change Application window title
    static_cast<Application*>(qApp)->mainWindow().setWindowTitle( Application::MAIN_TITLE_MODIFIED );

    // save EditionWindow entry
    window->saveAction().trigger();
    
    break;
    
    default: delete attachment; break;
    
  }
    
}  

//_____________________________________________
void AttachmentFrame::_updateActions( void )
{
  
  bool has_selection( !_list().selectionModel()->selectedRows().isEmpty() );
  newAttachmentAction().setEnabled( !read_only_ );
  openAttachmentAction().setEnabled( has_selection );
  editAttachmentAction().setEnabled( has_selection && !read_only_ );
  deleteAttachmentAction().setEnabled( has_selection && !read_only_ );
  return;
  
}

//_____________________________________________
void AttachmentFrame::_open( void )
{
  Debug::Throw( "AttachmentFrame::_open.\n" );  

  // get selection
  AttachmentModel::List selection( _model().get( list().selectionModel()->selectedRows() ) );
 
  // check items
  if( selection.isEmpty() ) 
  {
    QtUtil::infoDialog( this, "No attachment selected. <Open Attachment> canceled.\n" );
    return;
  }
 
  // loop over attachments
  for( AttachmentModel::List::const_iterator iter = selection.begin(); iter != selection.end(); iter++ )
  {
    
    Attachment& attachment( *iter ); 
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
  if( selection.isEmpty() ) 
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
    Attachment& attachment( *iter ); 
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
    { (*iter)->_update( attachment ); }
          
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
  if( selection.isEmpty() ) 
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
    
    Attachment* attachment( *iter ); 
  
    // dialog
    DeleteAttachmentDialog dialog( this, *attachment );
    QtUtil::centerOnParent( &dialog );

    if( dialog.exec() == QDialog::Rejected ) return;    
    
    // retrieve action
    bool from_disk( dialog.action() == DeleteAttachmentDialog::FROM_DISK );
    
    // retrieve/delete associated attachment frames and remove item
    BASE::KeySet<AttachmentFrame> frames( *attachment );
    for( BASE::KeySet<AttachmentFrame>::const_iterator iter = frames.begin(); iter != frames.end(); iter++ ) 
    { (*iter)->_model().remove( *attachment ); }
  
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
 
//_______________________________________________________________________
void AttachmentFrame::_installActions( void )
{
  Debug::Throw( "AttachmentFrame::_installActions.\n" );

  addAction( new_attachment_action_ = new QAction( IconEngine::get( ICONS::ATTACH ), "&New", this ) );
  newAttachmentAction().setToolTip( "Attach a file/URL to the current entry" );
  connect( &newAttachmentAction(), SIGNAL( triggered() ), SLOT( _newAttachment() ) );
  
  addAction( open_attachment_action_ = new QAction( IconEngine::get( ICONS::OPEN ), "&Open", this ) );
  openAttachmentAction().setToolTip( "Open selected attachments" );
  connect( &openAttachmentAction(), SIGNAL( triggered() ), SLOT( _open() ) );
     
  addAction( edit_attachment_action_ = new QAction( IconEngine::get( ICONS::EDIT ), "&Edit", this ) );
  editAttachmentAction().setToolTip( "Edit selected attachments informations" );
  connect( &editAttachmentAction(), SIGNAL( triggered() ), SLOT( _edit() ) );

  delete_attachment_action_ = new QAction( IconEngine::get( ICONS::DELETE ), "&Delete", this );
  deleteAttachmentAction().setToolTip( "Delete selected attachments" );
  connect( &deleteAttachmentAction(), SIGNAL( triggered() ), SLOT( _delete() ) );
  
}
