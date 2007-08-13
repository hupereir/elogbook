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
  \file  AttachmentList.cc
  \brief  customized list view to handle attachment 
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <list>
#include <sstream>
#include <QShortcut>

#include "Attachment.h"
#include "AttachmentFrame.h"
#include "AttachmentList.h"
#include "CustomFileDialog.h"
#include "CustomPixmap.h"
#include "Debug.h"
#include "DeleteAttachmentDialog.h"
#include "EditAttachmentDialog.h"
#include "EditFrame.h"
#include "File.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "MainFrame.h"
#include "NewAttachmentDialog.h"
#include "OpenAttachmentDialog.h"
#include "QtUtil.h"
#include "SelectionFrame.h"
#include "Util.h"

using namespace std;
using namespace BASE;

//_______________________________________________
char* AttachmentList::column_titles_[ AttachmentList::n_columns ] = 
{
  "file",
  "type",
  "size",
  "modification"
};

//_____________________________________________
AttachmentList::AttachmentList( QWidget *parent, bool read_only ):
  CustomListView( parent ),
  read_only_( read_only )
{ 
  Debug::Throw( "AttachmentList::AttachmentList.\n" ); 

  setColumnCount( n_columns );
  for( unsigned int i=0; i<n_columns; i++ ) 
  { setColumnName( i, column_titles_[i] ); }
  
  setRootIsDecorated( false );
  setSortingEnabled( true );
  setSelectionMode( QAbstractItemView::ContiguousSelection );
  setTextElideMode ( Qt::ElideMiddle );
  
  // buttons pixmap path list
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  new_attachment_action_ = new QAction( IconEngine::get( ICONS::ATTACH, path_list ), "&New", this );
  new_attachment_action_->setToolTip( "Attach a file/URL to the current entry" );
  connect( new_attachment_action_, SIGNAL( triggered() ), SLOT( _newAttachment() ) );
  menu().addAction( new_attachment_action_ );
  menu().addSeparator();
  
  view_attachment_action_ = new QAction( IconEngine::get( ICONS::OPEN, path_list ), "&Open", this );
  view_attachment_action_->setToolTip( "Open selected attachments" );
  connect( view_attachment_action_, SIGNAL( triggered() ), SLOT( _open() ) );
  menu().addAction( view_attachment_action_ );
     
  edit_attachment_action_ = new QAction( IconEngine::get( ICONS::EDIT, path_list ), "&Edit", this );
  edit_attachment_action_->setToolTip( "Edit selected attachments informations" );
  connect( edit_attachment_action_, SIGNAL( triggered() ), SLOT( _edit() ) );
  menu().addAction( edit_attachment_action_ );

  delete_attachment_action_ = new QAction( IconEngine::get( ICONS::DELETE, path_list ), "&Delete", this ); 
  delete_attachment_action_->setToolTip( "Delete selected attachments" );
  delete_attachment_action_->setShortcut( Qt::Key_Delete );
  connect( delete_attachment_action_, SIGNAL( triggered() ), SLOT( _delete() ) );
  menu().addAction( delete_attachment_action_ );
  
  // connections
  connect( this, SIGNAL( itemActivated( QTreeWidgetItem*, int ) ), SLOT( _open( QTreeWidgetItem* ) ) );
  connect( this, SIGNAL( itemSelectionChanged() ), SLOT( _updateActions() ) );
  _updateActions();
}

//_____________________________________________
void AttachmentList::add( Attachment* attachment )
{

  Debug::Throw( "AttachmentList::add.\n" ); 
  Exception::check( attachment, DESCRIPTION( "invalid attachment" ) );
  
  Item *item( new Item( this ) );
  Key::associate( item, attachment );
  item->update();
  
}

//_____________________________________________
void AttachmentList::update( Attachment* attachment )
{
  
  Debug::Throw( "AttachmentList::update.\n" ); 
  Exception::check( attachment, DESCRIPTION( "invalid attachment" ) );
  
  // retrieve associated Item, check and update
  KeySet<Item> items( attachment );
  Exception::check( items.size()==1, DESCRIPTION( "invalid association to local_item" ) );
  
  (*items.begin())->update();
  resizeColumns();

}

//_____________________________________________
void AttachmentList::selectAttachment( Attachment* attachment )
{
  
  Debug::Throw( "AttachmentList::SelectAttachment.\n" ); 
  
  // check attachment
  Exception::check( attachment, DESCRIPTION( "invalid attachment" ) );

  // retrieve associated Item, check 
  KeySet<Item> items( attachment );
  Exception::check( items.size()==1, DESCRIPTION( "invalid association to local_item" ) );

  // select local item
  clearSelection();
  setItemSelected( *items.begin(), true );
  scrollToItem( *items.begin() );
  
  return;
    
}

//_____________________________________________
void AttachmentList::resizeColumns( void )
{
  Debug::Throw( "AttachmentList::resizeColumns.\n" );
  resizeColumnToContents(SIZE);
  resizeColumnToContents(MODIFICATION);
  resizeColumnToContents(FILE);
  resizeColumnToContents(TYPE);
}
  
//_____________________________________________
void AttachmentList::_newAttachment( void )
{  Debug::Throw( "AttachmentList::_newAttachment.\n" );  
  
  // retrieve/check associated EditFrame/LogEntry
  KeySet<EditFrame> edit_frames( this );
  Exception::check( edit_frames.size() == 1, DESCRIPTION("wrong association to EditFrames") );
  EditFrame *edit_frame( *edit_frames.begin() );
  
  KeySet<LogEntry> entries( edit_frame );
  if( entries.size() != 1 )
  {
    QtUtil::infoDialog( this, "No valid entry found. <New Attachment> canceled." );
    return;
  }
  
  LogEntry *entry( *entries.begin() );
  
  // retrieve/check associated logbook, logbook directory
  KeySet<Logbook> logbooks( entry );
    
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
        if( QtUtil::questionDialog( this, o.str() ) ) {
    
          ostringstream o;
          o << "mkdir \"" << full_directory << "\"";
          Util::run( o.str() );
        
        }
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
    
    // update all edit_frames AttachmentList associated to entry
    edit_frames = KeySet<EditFrame>( entry );
    for( KeySet<EditFrame>::iterator iter = edit_frames.begin(); iter != edit_frames.end(); iter++ )
    {
      if( !(*iter)->attachmentList().topLevelItemCount() ) (*iter)->attachmentList().show();
      (*iter)->attachmentList().add( attachment );
      (*iter)->attachmentList().resizeColumns();
    }
    
    // update attachment frame
    static_cast<MainFrame*>(qApp)->attachmentFrame().list().add( attachment );
    static_cast<MainFrame*>(qApp)->attachmentFrame().list().resizeColumns();
   
    // update logbooks destination directory
    for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ ) 
    {
      (*iter)->setModified( true );
      (*iter)->setDirectory( full_directory );
    }
    
    // change MainFrame window title
    static_cast<MainFrame*>(qApp)->selectionFrame().setWindowTitle( MainFrame::MAIN_TITLE_MODIFIED );

    // save EditFrame entry
    edit_frame->save();
    
    break;
    
    default: delete attachment; break;
    
  }
    
}  

//_____________________________________________
void AttachmentList::_updateActions( void )
{
  
  bool has_selection( !QTreeWidget::selectedItems().empty() );
  new_attachment_action_->setEnabled( !read_only_ );
  view_attachment_action_->setEnabled( has_selection );
  edit_attachment_action_->setEnabled( has_selection && !read_only_ );
  delete_attachment_action_->setEnabled( has_selection && !read_only_ );
  return;
  
}

//_____________________________________________
void AttachmentList::_open( QTreeWidgetItem* item )
{
  Debug::Throw( "AttachmentList::_open.\n" );  

  // select item if valid
  QList<Item*> items;
  Item* local_item( dynamic_cast<Item*>( item ) );
  if( item ) 
  {
    items.push_back( local_item );
    setItemSelected( local_item, true );
  } else items = selectedItems<Item>();
  
  // check items
  if( items.empty() ) 
  {
    QtUtil::infoDialog( this, "No attachment selected. <View Attachment> canceled.\n" );
    return;
  }
 
  // create/check attachment full name
  for( QList<Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    Item& current( **iter );
    Attachment& attachment( *current.attachment() ); 
    AttachmentType type = attachment.type();
    File fullname( ( type == AttachmentType::URL ) ? attachment.file():attachment.file().expand() );
    if( !( type == AttachmentType::URL || fullname.exists() ) )
    {
      ostringstream what; 
      what << "Cannot find file \"" << fullname << "\". <View Attachment> canceled.";
      QtUtil::infoDialog( this, what.str() );
      return;
    }
    
    OpenAttachmentDialog dialog( this, attachment );  
    QtUtil::centerOnParent( &dialog );
    
    if( dialog.exec() == QDialog::Rejected ) return;
    if( dialog.action() == OpenAttachmentDialog::OPEN )
    {
      string command( dialog.command() );
    
      // run edit command
      ostringstream what;
      what << command << " \"" << fullname << "\"& ";
      Debug::Throw() << "AttachmentList::_open - command=" << what.str() << endl;
      Util::run( what.str() );
    
    } else {
  
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
      Util::run( string("cp ") + "\"" + fullname + "\" \"" + destname + "\"" );
      
    }
  
  }
  
  return;
  
}  

//_____________________________________________
void AttachmentList::_edit( void )
{
  Debug::Throw( "AttachmentList::_edit.\n" );
  
  // store selected item locally
  QList<Item*> items( selectedItems<Item>() );
  if( items.empty() ) 
  {
    QtUtil::infoDialog( this, "No attachment selected. <Edit Attachment> canceled.\n" );
    return;
  }

  // retrieve/check associated EditFrame/LogEntry
  KeySet<EditFrame> edit_frames( this );
  Exception::check( edit_frames.size() == 1, DESCRIPTION("wrong association to EditFrames") );
  EditFrame *edit_frame( *edit_frames.begin() );

  // create/check attachment full name
  for( QList<Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {

    Item& current( **iter );
    Attachment& attachment( *current.attachment() ); 
    EditAttachmentDialog dialog( this, attachment );
  
    // map dialog
    QtUtil::centerOnParent( &dialog );
    if( dialog.exec() == QDialog::Rejected ) return;
  
    // change attachment type
    attachment.setType( dialog.type() );
  
    // retrieve comments
    attachment.setComments( dialog.comments() );
  
    // set associated entry modified
    KeySet<LogEntry> entries( &attachment );
    Exception::check( entries.size() == 1, DESCRIPTION( "wrong association to LogEntry" ) );

    // update attachment associated list items
    KeySet<AttachmentList::Item> items( &attachment );
    for( KeySet<AttachmentList::Item>::iterator iter = items.begin(); iter != items.end(); iter++ )
    { (*iter)->update(); }
          
  }
  
  // set main window title
  static_cast<MainFrame*>(qApp)->selectionFrame().setWindowTitle( MainFrame::MAIN_TITLE_MODIFIED );
  
  // save EditFrame entry
  edit_frame->save();
  
}

//_____________________________________________
void AttachmentList::_delete( void )
{
  Debug::Throw( "AttachmentList::_delete.\n" );
  
  // store selected item locally
  QList<Item*> items( selectedItems<Item>() );
  if( items.empty() ) 
  {
    QtUtil::infoDialog( this, "No attachment selected. <Edit Attachment> canceled.\n" );
    return;
  }

  // retrieve/check associated EditFrame/LogEntry
  KeySet<EditFrame> edit_frames( this );
  Exception::check( edit_frames.size() == 1, DESCRIPTION("wrong association to EditFrames") );
  EditFrame *edit_frame( *edit_frames.begin() );

  // create/check attachment full name
  for( QList<Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {

    Item& current( **iter );
    Attachment* attachment( current.attachment() ); 
  
    // dialog
    DeleteAttachmentDialog dialog( this, *attachment );
    QtUtil::centerOnParent( &dialog );

    if( dialog.exec() == QDialog::Rejected ) return;    
    
    // retrieve action
    bool from_disk( dialog.action() == DeleteAttachmentDialog::FROM_DISK );
    
    // retrieve/delete associated attachmentlist items
    KeySet<AttachmentList::Item> items( attachment );
    for( KeySet<AttachmentList::Item>::iterator iter = items.begin(); iter != items.end(); iter++ ) 
    {
      if( (*iter)->treeWidget()->topLevelItemCount() == 1 ) (*iter)->treeWidget()->hide();
      delete *iter;
    }
  
    // retrieve associated entries
    KeySet<LogEntry> entries( attachment );
    Exception::check( entries.size() == 1, DESCRIPTION( "wrong association to LogEntry" ) );
    LogEntry* entry( *entries.begin() );
    entry->modified();
    
    // retrieve associated logbooks
    KeySet<Logbook> logbooks( entry );
    
    // check sharing attachments to avoid from_disk deletion
    if( from_disk && logbooks.size() ) 
    {
      
      KeySet<Attachment> attachments( (*logbooks.begin())->attachments() );
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
  static_cast<MainFrame*>(qApp)->selectionFrame().setWindowTitle( MainFrame::MAIN_TITLE_MODIFIED );
  
  // save EditFrame entry
  edit_frame->save();

  return;      
  
}
 

//_____________________________________________
void AttachmentList::Item::update( void )
{
  Attachment& attachment( *Item::attachment() );  
  setText( AttachmentList::FILE, string( attachment.shortFile()+" ").c_str() );
  setText( AttachmentList::TYPE, attachment.type().name().c_str() );
  setText( AttachmentList::SIZE, attachment.sizeString().c_str() );
  setText( AttachmentList::MODIFICATION, (attachment.modification().isValid() ) ? 
      attachment.modification().string().c_str():
      "-" );
}    

//___________________________________
bool AttachmentList::Item::operator < (const QTreeWidgetItem& item ) const
{

  // cast parent to custom list view
  const CustomListView* parent( dynamic_cast<const CustomListView*>( treeWidget() ) );
  if( !parent ) return QTreeWidgetItem::operator < (item);
  
  // try cast other
  const Item* local( dynamic_cast<const Item*>( &item ) );
  if( !local )  return QTreeWidgetItem::operator < (item);
  
  // retrieve column type
  int column( parent->sortColumn() );  
  
  // check if column is a TimeStamp
  if( column == SIZE ) return attachment()->size() < local->attachment()->size();
  if( column == MODIFICATION ) return attachment()->modification() < local->attachment()->modification();

  // default case
  return QTreeWidgetItem::operator < (item);
  
}
