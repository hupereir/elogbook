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
*******************************************************************************/


/*!
   \file LogEntryList.cpp
   \brief customized list view to handle LogEntry
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QApplication>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>

#include "Debug.h"
#include "Logbook.h"
#include "ColorMenu.h"
#include "LogEntryList.h"

using namespace std;

//_______________________________________________
const QString LogEntryList::DRAG = "LogEntryList::Drag";
const char* LogEntryList::column_titles_[ LogEntryList::n_columns ] = 
{
  "title",
  "creation",
  "modification",
  "author",
  "color"
};

//_____________________________________________
LogEntryList::LogEntryList( QWidget *parent ):
  TreeWidget( parent ),
  first_item_( 0 ),
  last_item_( 0 ),
  drag_enabled_( false ),
  edit_item_( 0 ),
  edit_timer_( new QTimer( this ) )
{
  
  Debug::Throw( "LogEntryList::LogEntryList.\n" );
 
  setRootIsDecorated( false );    
  setAcceptDrops(false);  
 
  setColumnCount( n_columns );
  for( unsigned int i=0; i<n_columns; i++ )
  { setColumnName( i, column_titles_[i] ); }

  // selection mode
  setSelectionMode( QAbstractItemView::ContiguousSelection );
  
  // editing 
  edit_timer_->setSingleShot( true );
  edit_timer_->setInterval( edit_item_delay_ );
  connect( edit_timer_, SIGNAL( timeout() ), SLOT( _startEdit() ) );
  connect( this, SIGNAL( itemActivated( QTreeWidgetItem*, int ) ), SLOT( _activate( QTreeWidgetItem* ) ) );

}
 
//_______________________________________________
void LogEntryList::add( LogEntry* entry, bool update_selection )
{
  
  Debug::Throw("LogEntryList::add.\n" );
  assert( entry );
  
  Item *item( new Item() );
  addTopLevelItem( item );

  BASE::Key::associate( item, entry );  
  item->update();
  
  if( update_selection ) {
    clearSelection();
    setCurrentItem( item );
    scrollToItem( item );
  }
  
  return;
  
}

//_______________________________________________
void LogEntryList::resizeColumns()
{

  Debug::Throw( "LogEntryList::resizeColumns.\n" );
  
  // if no items present, do nothing
  if( QTreeWidget::topLevelItemCount() == 0 ) return;
  
  // resize. Goes from back to front, for visible columns only
  unsigned int mask( LogEntryList::mask() );
 // for( int i = columnCount()-1; i >= 0; i-- )
  for( int i = 0; i < columnCount(); i++ )
  { if( mask & (1<<i) ) resizeColumnToContents(i); }
  
  return;
}

//_______________________________________________
void LogEntryList::update( LogEntry* entry, bool update_selection )
{
  
  Debug::Throw( "LogEntryList::update.\n" );
  assert( entry );
  
  // get associated Item
  Item *item( LogEntryList::item( entry ) );
  
  // update item
  item->update();
  if( update_selection && entry->isKeywordSelected() ) 
  {
    clearSelection();
    setItemHidden( item, false );
    setCurrentItem( item );
    scrollToItem( item );
  }
  
  return;  
}
   
//_______________________________________________
void LogEntryList::select( LogEntry* entry )
{
  Debug::Throw( "LogEntryList::SelectEntry.\n" );
  assert( entry );

  Item *item( LogEntryList::item( entry ) );

  clearSelection();
  setItemHidden( item, false );
  setCurrentItem( item );
  scrollToItem( item );
  return;  
}

//_______________________________________________
LogEntryList::Item* LogEntryList::itemBelow( QTreeWidgetItem* item, bool update_selection )
{
  
  Debug::Throw( "LogEntryList::itemBelow.\n" );
  
  QTreeWidgetItem* tmp = 0;
  
  // see if item has children
  if( item->childCount() ) tmp = item->child(0);
  else if( !item->parent() )
  { 
    
    int index = indexOfTopLevelItem( item );
    if( index + 1 < topLevelItemCount() ) tmp = topLevelItem( index+1 );
    
  } else {
    
    // see if item is last child of parent
    // if yes, return item below parent
    int index = item->parent()->indexOfChild( item );
    if( index+1 < item->parent()->childCount() ) tmp = item->parent()->child(index+1);
    else tmp = itemBelow( item->parent(), false );
    
  }
  
  // if item is hidden, try the next one
  if( tmp && isItemHidden( tmp ) ) tmp = itemBelow( tmp, false );
  
  Item* out = static_cast<Item*>( tmp );
  if( out && update_selection ) {
  
    clearSelection();
    setCurrentItem( out );
    scrollToItem( out );
    
  }
  
  return out;
  
}

//_______________________________________________
LogEntryList::Item* LogEntryList::itemAbove( QTreeWidgetItem* item, bool update_selection )
{
  
  Debug::Throw( "LogEntryList::itemAbove.\n" );
  
  QTreeWidgetItem* tmp = 0;
  
  // see if item has children
  if( !item->parent() )
  { 
    
    int index = indexOfTopLevelItem( item );
    Debug::Throw() << "LogEntryList::itemAbove - topLevelItem - index: " << index << endl;
    if( index > 0 ) tmp = topLevelItem( index-1 );
    
  } else {
    
    // see if item is first child of parent
    // if yes, return item below parent
    int index = item->parent()->indexOfChild( item );
    if( index > 0 ) tmp = item->parent()->child(index-1);
    else tmp = item->parent();
    
  }
  
  // no item found.
  if( !tmp ) 
  {
    Debug::Throw() << "LogEntryList::itemAbove - no item found" << endl;
    return 0;
  }
  
  // see if found item has children
  // one must retrieve the most downward child
  while( tmp->childCount() ) { tmp = tmp->child( tmp->childCount() - 1 ); }
  
  // if item is hidden, try the next one
  if( tmp && isItemHidden( tmp ) ) tmp = itemAbove( tmp, false );
  
  Item* out = static_cast<Item*>( tmp );

  if( out && update_selection ) {
  
    setItemHidden( out, false );
    clearSelection();
    setCurrentItem( out );
    scrollToItem( out );
    
  }
  
  return out;
  
}

//___________________________________
list< LogEntry* > LogEntryList::selectedEntries( void )
{ 
  Debug::Throw( "LogEntryList::entries" );

  // retrieve selected items
  // store associated entries
  list< LogEntry* > entries; 
  QList<Item*> items( LogEntryList::selectedItems<Item>() );
  for( QList<Item*>::iterator iter = items.begin(); iter != items.end(); iter++ ) 
  {
    
    // check item visibility
    if( isItemHidden( *iter ) ) continue;
    entries.push_back( (*iter)->entry() );

  }
  
  return entries;
  
} 

//___________________________________
list< LogEntry* > LogEntryList::entries( void )
{ 
  Debug::Throw( "LogEntryList::entries" );

  // retrieve logbook entries
  list< LogEntry* > entries; 
  QList<Item*> items( TreeWidget::children<Item>() );
  for( QList<Item*>::iterator iter = items.begin(); iter != items.end(); iter++ ) 
  {
    
    // check item visibility
    if( isItemHidden( *iter ) ) continue;
    entries.push_back( (*iter)->entry() );

  }
  
  return entries;
  
} 

//________________________________________
void LogEntryList::clear( void )
{
  Debug::Throw( "LogEntryList::clear.\n" );
  first_item_ = 0;
  last_item_ = 0;
  drag_enabled_ = false;
  edit_item_ = 0;
  edit_timer_->stop();
  TreeWidget::clear();
}

//_____________________________________________________
void LogEntryList::_startEdit( void )
{
  Debug::Throw( "LogEntryList::_startEdit.\n" );
  edit_item_ = QTreeWidget::currentItem();
  backup_ = edit_item_->text( TITLE );
  openPersistentEditor( edit_item_, TITLE );
}

//_____________________________________________________
void LogEntryList::_resetEdit( const bool& restore_backup )
{
  Debug::Throw( "LogEntryList::_resetEdit.\n" );

  // stop timer
  edit_timer_->stop();
  if( edit_item_ )
  {
    // close editor
    closePersistentEditor( edit_item_ );
    
    // restore backup if required
    if( restore_backup ) 
    { edit_item_->setText( TITLE, backup_ ); }
    
    // reset item
    edit_item_ = 0;
  }

  Debug::Throw( "LogEntryList::_resetEdit - done.\n" );
  
}

//_______________________________________________
void LogEntryList::_activate( QTreeWidgetItem *item )
{ 
  
  Debug::Throw( "LogEntryList::_activate.\n" );
  assert( item, "invalid item" );
    
  // check if item is edited
  // if not, emit the selectend entry signal
  if( item == edit_item_  && !edit_timer_->isActive() )
  {
      
    Debug::Throw( "LogEntryList::_activate - matching item.\n" );
    
    // cast associated editor and check for validity
    QLineEdit* editor( dynamic_cast<QLineEdit*>( itemWidget( item, 0 ) ) );
    if( !editor ) return;

    // retrieve Item title
    // check against backup
    QString title( editor->text() );
    if( title != backup_ ) 
    { emit entryRenamed( static_cast<Item*>( item )->entry(), qPrintable( title ) ); }
    
    // reset edition
    // without restoring the backup
    _resetEdit( false );
                  
  } else {
    
    Debug::Throw( "LogEntryList::_activate - item selected.\n" );
    emit entrySelected( static_cast<Item*>(item)->entry() );
   _resetEdit();
    
 }
  
  Debug::Throw( "LogEntryList::_activate - done.\n" );
  return;
      
}

//_______________________________________________
void LogEntryList::mousePressEvent( QMouseEvent* event )
{
  
  Debug::Throw( "LogEntryList::mousePressEvent.\n" );
  
  #ifdef BASE_EVENT_HANDLERS
  return TreeWidget::mousePressEvent( event );
  #endif
  
  // retrieve item under cursor
  QTreeWidgetItem* item = itemAt( event->pos());
  
  // retrieve column under cursor
  int column = columnAt( event->pos().x() );
  
  // see if current item and column match edited item TITLE column
  // if not, close editor, restore old keyword
  if( edit_item_ && ( item != edit_item_ || column != TITLE ) )
  {
    
    Debug::Throw( "LogEntryList::mousePressEvent - closing editor.\n" );
    
    // close editor
    closePersistentEditor( edit_item_ );
    
    // restore backup text
    edit_item_->setText( TITLE, backup_ );
    
    // reset item
    edit_item_ = 0;
    
  }

  // check button
  /* so far all actions linked to other than the left or right buttons are left default */
  if( !( event->buttons() & (Qt::LeftButton|Qt::RightButton) ) ) 
  { return TreeWidget::mousePressEvent( event ); }

  // retrieve Item at position
  if( !item ) 
  {
    clearSelection();
    return;
  }
  
  // see if click occured on current item, start Edit timer
  if( 
    event->button() == Qt::LeftButton && item && 
    isItemSelected( item ) &&
    column == TITLE &&
    item != edit_item_ ) 
  { 
    
    edit_timer_->start(); 
    TreeWidget::mousePressEvent( event );
  
  } else _resetEdit();
      
  // enable drag if item was already selected  
  if( isItemSelected( item ) && event->button() == Qt::LeftButton )
  {
    drag_enabled_ = true;
    drag_start_ = event->pos();
  }
  
  // reset extended selection begin/end items
  if( event->button() == Qt::LeftButton )
  {
    first_item_ = item;
    last_item_ = item;
  }
  
  // finally, if item was not selected, or column is other than title run default
  if( !( isItemSelected( item ) && column == TITLE )  )
  { TreeWidget::mousePressEvent( event ); }
  
  Debug::Throw( "LogEntryList::mousePressEvent. Done.\n" );

}
  
//_____________________________________________________________
void LogEntryList::mouseMoveEvent( QMouseEvent* event )
{
  
  Debug::Throw( "LogEntryList::mouseMoveEvent.\n" );
  
  if( !(event->buttons()&Qt::LeftButton) ) return TreeWidget::mouseMoveEvent( event );
  
  // retrieve Item at position
  QTreeWidgetItem* item( itemAt( event->pos() ) );
  if( !item ) return;
     
  // check distance to last click
  if( (event->pos() - drag_start_ ).manhattanLength() >= QApplication::startDragDistance() && drag_enabled_ )
  { _startDrag(); }

  
  // select all items between first_item_ and current item
  int first( indexOfTopLevelItem( first_item_ ) );
  int last( indexOfTopLevelItem( last_item_ ) );
  int index( indexOfTopLevelItem( item ) );
  
  if( first <= last )
  {
    
    if( index > last )
    {
      for( int i = last; i <= index; i++ )
      { setItemSelected( topLevelItem(i), true ); }
    
    } else if( index <= last && index >= first ) { 
    
      for( int i = index+1; i<= last; i++ )
      { setItemSelected( topLevelItem(i), false ); }
    
    } else {
      
      for( int i = first+1; i<= last; i++ )
      { setItemSelected( topLevelItem(i), false ); }
      
      for( int i = index; i<=first; i++ )
      { setItemSelected( topLevelItem(i), true ); }
    
    }
    
  } else {
      
    if( index < last )
    {
    
      for( int i = index; i <= last; i++ )
      { setItemSelected( topLevelItem(i), true ); }
    
    } else if( index >= last && index <= first ) {
    
      for( int i = last; i< index; i++ )
      { setItemSelected( topLevelItem(i), false ); }
    
    } else {

      for( int i = last; i<first; i++ )
      { setItemSelected( topLevelItem(i), false ); }
    
      for( int i = first; i<= index; i++ )
       { setItemSelected( topLevelItem(i), true ); }

    }
  
  }
  
  last_item_ = item;
  scrollToItem( item );
  
  return;
  
}
    
//_____________________________________________________________
void LogEntryList::mouseReleaseEvent( QMouseEvent* event )
{
  Debug::Throw( "LogEntryList::mouseReleaseEvent.\n" );

  // retrieve Item at position
  QTreeWidgetItem* item( itemAt( event->pos() ) );

  // clear selection
  // if mouse release occur in same event as mouse press,
  // and if shift key is not active (since it is supposed to extend the selection
  if( first_item_ == item && event->modifiers() != Qt::ShiftModifier) 
  {
    clearSelection();
    setCurrentItem( item );
  }
  
  drag_enabled_ = false;
  return TreeWidget::mouseReleaseEvent( event );  
}

//_______________________________________________
bool LogEntryList::_startDrag( void )
{
  Debug::Throw( "LogEntryList::_startDrag.\n" );
  
  // stop edition timer
  //if( edit_item_ ) 
  //{
  edit_item_ = 0;
  edit_timer_->stop();
  //}
  
  // start drag
  QDrag *drag = new QDrag(this);
  
  // store data
  QMimeData *mime = new QMimeData();
  mime->setData( LogEntryList::DRAG, 0 );
  drag->setMimeData(mime);
  drag->start(Qt::CopyAction);  

  // disable drag
  drag_enabled_ = false;
  
  return true;
}

//_______________________________________________
void LogEntryList::Item::update( void ) 
{
    
  LogEntry *entry( Item::entry() );
  Debug::Throw() << "LogEntryList::Item::update - entry: " << entry->key() << endl;
  
  setText( LogEntryList::TITLE, string( entry->title()+" ").c_str() );
  setText( LogEntryList::CREATION, string( entry->creation().string()+" ").c_str() );
  setText( LogEntryList::MODIFICATION, string( entry->modification().string()+" ").c_str() );
  setText( LogEntryList::AUTHOR, string( entry->author()+" ").c_str() );
  setText( LogEntryList::COLOR, string( entry->color()+" ").c_str() );
  
  if( entry->color() != ColorMenu::NONE ) setColor( QColor( entry->color().c_str() ) );
  else setColor( QColor() );
  
  treeWidget()->update();
  
}

//___________________________________
bool LogEntryList::Item::operator < (const QTreeWidgetItem& item ) const
{

  // cast parent to custom list view
  const TreeWidget* parent( static_cast<const TreeWidget*>( treeWidget() ) );
  if( !parent ) return QTreeWidgetItem::operator < (item);
  
  // try cast other
  const Item* local( static_cast<const Item*>( &item ) );
  if( !local )  return QTreeWidgetItem::operator < (item);
  
  // retrieve column type
  int column( parent->sortColumn() );  
  
  // check if column is a TimeStamp
  if( column == CREATION ) return entry()->creation() < local->entry()->creation();
  if( column == MODIFICATION ) return entry()->modification() < local->entry()->modification();

  // default case
  return QTreeWidgetItem::operator < (item);
  
}
