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
   \file KeywordList.cpp
   \brief customized list view to handle LogEntry keyword
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QApplication>
#include <QMouseEvent>
#include <QLineEdit>

#include "KeywordList.h"
#include "LogEntryList.h"
#include "LogEntry.h"

using namespace std;


//_______________________________________________
const QString KeywordList::DRAG = "KeywordList::Drag";
const char* KeywordList::column_titles_[ KeywordList::n_columns ] = { "keyword" };

//_______________________________________________
KeywordList::KeywordList( QWidget *parent ):
  TreeWidget( parent ),
  root_item_( 0 ),
  drop_item_( 0 ),
  drop_item_selected_( false ),
  drop_item_timer_( this ),
  process_drop_timer_( this ),
  edit_item_( 0 ),
  edit_timer_( this )
{
  
  Debug::Throw( "KeywordList::KeywordList.\n" );
  
  // configurate drop timer
  drop_item_timer_.setSingleShot( true );
  drop_item_timer_.setInterval( drop_item_delay_ );
  connect( &drop_item_timer_, SIGNAL( timeout() ), SLOT( _openDropItem() ) );

  process_drop_timer_.setSingleShot( true );
  process_drop_timer_.setInterval( 0 );
  connect( &process_drop_timer_, SIGNAL( timeout() ), SLOT( _processDrop() ) );
  
  edit_timer_.setSingleShot( true );
  edit_timer_.setInterval( edit_item_delay_ );
  connect( &edit_timer_, SIGNAL( timeout() ), SLOT( _startEdit() ) );
    
  setRootIsDecorated( true );    
  setAcceptDrops(true);  
  setAutoScroll(true);
  
  setColumnCount( n_columns );
  for( unsigned int i=0; i<n_columns; i++ )
  { setColumnName( i, column_titles_[i] ); }
    
  _createRootItem();
    
  connect( this, SIGNAL( itemActivated( QTreeWidgetItem*, int ) ), SLOT( _activate( QTreeWidgetItem* ) ) );
  connect( this, SIGNAL( itemExpanded( QTreeWidgetItem* ) ), SLOT( _resetEdit() ) );
  connect( this, SIGNAL( itemCollapsed( QTreeWidgetItem* ) ), SLOT( _resetEdit() ) );
}

//_______________________________________________
void KeywordList::add( string keyword )
{
  
  Debug::Throw() << "KeywordList::add - keyword=" << keyword << endl;

  // check root item
  _createRootItem();
    
  // check keyword
  keyword = LogEntry::formatKeyword( keyword );
  
  // parse keyword, insert in keywords tree  
  Item* parent_item( root_item_ );
  
  // retrieve sub-keywords, separated by "/"
  vector<string> keywords( LogEntry::parseKeyword( keyword ) );
  for( vector<string>::iterator iter = keywords.begin(); iter!= keywords.end(); iter++ )
  {
    
    if( !iter->size() ) continue;

    Item* item( 0 );
    
    // loop over children, stop at matching keyword
    for( int i=0; i < parent_item->childCount(); i++ )
    {
      Item* child( static_cast<Item*>(parent_item->child(i)) );
      if( child && (*iter) == qPrintable( child->text( KEYWORD ) ) )
      { 
        item = child;
        break;
      }
    }
          
    // if none found, create a new Item
    if( !item ) 
    {
      item = new Item();
      parent_item->addChild( item );
      item->setText( KEYWORD, iter->c_str() );  
    }
    
    // set as parent before processing the next keyword in the list
    parent_item = item;
    
  }
  
}  

//_______________________________________________
void KeywordList::_remove( string keyword )
{
  
  Debug::Throw() << "KeywordList::_remove - keyword=" << keyword << endl;
  
  // when removing items one must stop all edition
  if( edit_item_ ) _resetEdit();
  
  // check root item
  _createRootItem();
    
  // format keyword
  keyword = LogEntry::formatKeyword( keyword );
  
  // parse keyword, insert in keywords tree  
  Item* parent_item( root_item_ );
  
  // retrieve sub-keywords, separated by "/"
  bool found( true );
  vector<string> keywords( LogEntry::parseKeyword( keyword ) );
  for( vector<string>::iterator iter = keywords.begin(); iter!= keywords.end(); iter++ )
  {
    
    if( !iter->size() ) continue;

    Item* item( 0 );
    
    // loop over children, stop at matching keyword
    for( int i=0; i < parent_item->childCount(); i++ )
    {
      Item* child( static_cast<Item*>(parent_item->child(i)) );
      if( child && (*iter) == qPrintable( child->text( KEYWORD ) ) )
      { 
        item = child;
        break;
      }
    }
          
    // if none found, stop parsing
    /* 
      this can appen when a parent item has already been deleted
      this is expected to happen during reset, during which a parent
      is removed when neither it nor any of its children would survive the
      reset anyway.
    */
    if( !item ) {
      found = false;
      break;
    }
    
    // set as parent before processing the next keyword in the list
    parent_item = item;
    
  }
  
  if( found ) delete parent_item;
  
}  

//_______________________________________________
void KeywordList::reset( set<string> new_keywords )
{
  
  Debug::Throw( "KeywordList::reset.\n" );
  
  // remove duplicated keywords
  _unique();
  
  // make sure root item is in the new_keywords list
  // so that it does not get deleted
  _createRootItem();
  new_keywords.insert( qPrintable( root_item_->text( KEYWORD ) ) );
  
  // retrieve current list of keywords
  set<string> old_keywords( keywords() );
  
  // remove keywords that are not included in new keywords
  for( set<string>::iterator iter = old_keywords.begin(); iter != old_keywords.end(); iter++ )
  {
    if( find_if( new_keywords.begin(), new_keywords.end(), ContainsFTor( *iter ) ) == new_keywords.end() )
    { _remove( *iter ); }
  }
  
  // add keywords that are not found in old list
  for( set<string>::const_iterator iter = new_keywords.begin(); iter != new_keywords.end(); iter++ )
  { 
    if( old_keywords.find( *iter ) == old_keywords.end() ) 
    { add( *iter ); }
  }
    
  // always expand the root item
  expandItem( rootItem() );
  Debug::Throw( "KeywordList::reset - done.\n" );
  
  return;
}
  
//_______________________________________________
void KeywordList::select( string keyword )
{
  
  Debug::Throw() << "KeywordList::select - " << keyword << endl;

  // format
  keyword = LogEntry::formatKeyword( keyword );
    
  // retrieve selected items
  QList<QTreeWidgetItem*> items( QTreeWidget::selectedItems() );
  for( QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  { 
    if( keyword == qPrintable( (*iter)->text(KEYWORD) ) )
    {
      Debug::Throw() << "KeywordList::select - already selected." << endl;
      setCurrentItem( *iter );
      scrollToItem( *iter );
      emit currentItemChanged( *iter, QTreeWidget::currentItem() );
      break;
    }
  }
  
  // look for matching item in list
  items = TreeWidget::children();
  for( QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    string local(  KeywordList::keyword( *iter ) );
    Debug::Throw() << "KeywordList::select - local: " << local << " keyword: " << keyword << endl;
    if( local == keyword )
    {    
      Debug::Throw() << "KeywordList::select - found matching keyword." << endl;
      setCurrentItem( *iter );
      scrollToItem( *iter );
      return;
    }
  }
  
  // no keyword found returns root item
  Debug::Throw() << "KeywordList::select - no matching keyword found." << endl;
  setCurrentItem( root_item_ );
  scrollToItem( root_item_ );
  return;
  
}

//_______________________________________________
string KeywordList::current( void )
{
  Debug::Throw( "KeywordList::current.\n" );
  
  QList<QTreeWidgetItem*> items( QTreeWidget::selectedItems() );
  return items.size() ? keyword( items.back() ):qPrintable( root_item_->text( KEYWORD ) );
        
}

//__________________________________________________________
string KeywordList::keyword( QTreeWidgetItem* item ) const
{
  
  Debug::Throw( "KeywordList::keyword.\n" );
  
  if( item == root_item_ ) return qPrintable( root_item_->text( KEYWORD ) );
  
  QString out( item->text( KEYWORD ) );
  while( (item = item->parent()) != root_item_ )
  { out = item->text( KEYWORD ) + "/" + out; }
  
  out = root_item_->text( KEYWORD ) + out;

  return qPrintable(out);  
  
}
//_______________________________________________
set<string> KeywordList::keywords( void )
{
  Debug::Throw( "KeywordList::keywords.\n" );
  
  set<string> out;

  // look for matching item in list
  QList<QTreeWidgetItem*> items = TreeWidget::children();
  for( QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  { out.insert( keyword( *iter ) ); }

  return out;
  
} 

//_______________________________________________
void KeywordList::clear( void )
{
  Debug::Throw( "KeywordList::clear.\n" );  

  root_item_ = 0;
  _resetDrag();
  _resetEdit();

  TreeWidget::clear();
  
}

//_____________________________________________________
void KeywordList::_openDropItem( void )
{
  Debug::Throw( "KeyWordList::_openDropItem.\n" );
  if( drop_item_ && !isItemExpanded( drop_item_ ) )
  { expandItem( drop_item_ ); }
}

//_____________________________________________________
void KeywordList::_startEdit( void )
{
  Debug::Throw( "KeywordList::_startEdit.\n" );
  edit_item_ = QTreeWidget::currentItem();
  backup_ = edit_item_->text( KEYWORD );
  full_backup_ = keyword( edit_item_ );
  openPersistentEditor( edit_item_ );
}

//_____________________________________________________
void KeywordList::_resetEdit( const bool& restore_backup )
{
  Debug::Throw( "KeywordList::_resetEdit.\n" );

  // stop timer
  edit_timer_.stop();
  if( edit_item_ )
  {
    // close editor
    closePersistentEditor( edit_item_ );
    
    // restore backup if required
    if( restore_backup ) 
    { edit_item_->setText( KEYWORD, backup_ ); }
    
    // reset item
    edit_item_ = 0;
  }
  
}

//_____________________________________________________
void KeywordList::_activate( QTreeWidgetItem* item )
{
  Debug::Throw( "KeywordList::_activate.\n" );
  
  // check if item is edited
  if( !( edit_item_ && item == edit_item_ ) ) return;
    
  // cast associated editor and check for validity
  QLineEdit* editor( dynamic_cast<QLineEdit*>( itemWidget( item, 0 ) ) );
  if( !editor ) return;
  
  //! retrieve full backup keyword
  string old_keyword = full_backup_; 
  
  // update item text
  edit_item_->setText( KEYWORD, editor->text() );
  
  // retrieve full keyword
  string new_keyword = keyword( edit_item_ );
  
  // emit keyword changed signal if they are different
  if( old_keyword != new_keyword )
  { emit keywordChanged( old_keyword, new_keyword ); }
  
  // reset edit item
  // without restoring the backup
  _resetEdit( false );
  
}

//__________________________________________________________
void KeywordList::_createRootItem( void )
{ 
  if( root_item_ ) return;
  
  Debug::Throw( "KeywordList::_createRootItem.\n" );
  root_item_ = new Item();
  addTopLevelItem( root_item_ );
  root_item_->setText( KEYWORD, LogEntry::NO_KEYWORD.c_str() );

}

//__________________________________________________________
void KeywordList::_unique( QTreeWidgetItem* item )
{

  Debug::Throw( "KeywordList::_unique.\n" );  
  if( !item ) 
  {
    // when uniquing the list one must stop all edition
    if( edit_item_ ) _resetEdit();
    item = rootItem();
    
  } if( !item ) return;
  
  // loop over children
  for( int i=0; i<item->childCount(); i++ )
  {
    
    // check if name is empty
    if( item->child(i)->text( KEYWORD ).isEmpty() )
    {
      // move children
      for( int k = 0; k< item->child(i)->childCount(); k++ )
      { item->addChild( item->child(i)->takeChild(k) ); }
      delete item->takeChild(i);
      continue;
    }
    
    // compare to existing children 
    for( int j=0; j<i; j++ )
    {
      if( item->child(i)->text(KEYWORD) == item->child(j)->text(KEYWORD) )
      {
        // move children
        for( int k = 0; k< item->child(i)->childCount(); k++ )
        { item->child(j)->addChild( item->child(i)->takeChild(k) ); }
        delete item->takeChild(i);
        break;
      }
    }
  }
  
  // run unique on children
  for( int i=0; i<item->childCount(); i++ )
  { _unique( item->child(i) ); }
  
} 

//__________________________________________________________
void KeywordList::mousePressEvent( QMouseEvent* event )
{
  
  Debug::Throw( "KeywordList::mousePressEvent.\n" );
  
  // retrieve item under cursor
  QTreeWidgetItem* item = itemAt( event->pos());
    
  // see if current item match edited item
  // if not, close editor, restore old keyword
  if( edit_item_ && item != edit_item_ ) 
  { _resetEdit(); }
    
  // left button handling
  if (event->button() == Qt::LeftButton) 
  {
    
    // store event position for drag-start
    drag_start_ = event->pos();
    
    /* 
      see if click occured on current item,
      make sure it is not the root item, 
      start Edit timer
    */
    if( item && item != rootItem() && item == QTreeWidget::currentItem() && item != edit_item_ ) edit_timer_.start();
    else _resetEdit();
    
  }
  
  return TreeWidget::mousePressEvent( event );

}
  
//_____________________________________________________________
void KeywordList::mouseMoveEvent( QMouseEvent* event )
{
  
  Debug::Throw( "KeywordList::mouseMoveEvent.\n" );
  
  // check button
  if( !(event->buttons()&Qt::LeftButton) )  return TreeWidget::mouseMoveEvent( event );
  
  // check distance to last click
  if( (event->pos() - drag_start_ ).manhattanLength() < QApplication::startDragDistance() )
  { return TreeWidget::mouseMoveEvent( event ); }
  
  _startDrag( event );
  
}

//__________________________________________________________
void KeywordList::dragEnterEvent( QDragEnterEvent* event )
{
  Debug::Throw( "KeywordList::dragEnterEvent.\n" );

  // check if object can be decoded
  if( !_acceptDrag( event ) ) 
  {
    event->ignore();
    return;
  }
  
  // accept event
  event->acceptProposedAction() ;

  // retrieve item below pointer
  Item *item( static_cast<Item*>( itemAt( event->pos()) ) );

  if( item ) {
    
    _updateOpenItems( item );
    
    drop_item_ = item;
    drop_item_timer_.start();
    
    drop_item_selected_ = isItemSelected( drop_item_ );
    setItemSelected( drop_item_, true );
    
  }
      
}

//__________________________________________________________
void KeywordList::dragMoveEvent( QDragMoveEvent* event )
{

  Debug::Throw( "KeywordList::dragMoveEvent.\n" );

  // check if object can be decoded
  if( !_acceptDrag( event ) ) 
  {
    event->ignore();
    return;
  }
  
  // accept event
  event->acceptProposedAction() ;
  
  // retrieve item below pointer
  Item *item( static_cast<Item*>( itemAt( event->pos() ) ) );
  
  // update drop item  
  if( item ) 
  {            
    
    _updateOpenItems( item );
    
    if( drop_item_ != item )
    {

      if( drop_item_ ) setItemSelected( drop_item_, drop_item_selected_ );
      
      // change drop item
      drop_item_ = item;
      
      drop_item_selected_ = isItemSelected( drop_item_ );
      if( drop_item_ ) setItemSelected( drop_item_, true );
      
      // restart timer
      drop_item_timer_.start();
      
    }
    
  }
  
}

//__________________________________________________________
void KeywordList::dropEvent( QDropEvent* event )
{
  Debug::Throw( "KeywordList::dropEvent.\n" );
  
  // check if object can be decoded
  if( !_acceptDrag( event ) ) 
  {
    event->ignore();
    return;
  }
  
  // retrieve item below pointer
  Item *item( static_cast<Item*>(itemAt( event->pos() ) ) );
  if( !item )  {
    event->ignore();
    return;
  }

  // store mime data
  if( event->mimeData()->hasFormat( KeywordList::DRAG ) )
  {
    QString value( event->mimeData()->data( KeywordList::DRAG ) );
    if( value.isNull() || value.isEmpty() ) 
    {
      event->ignore();
      return;
    }
    
    drop_data_ = DropData( KeywordList::DRAG, value );

  } else if( event->mimeData()->hasFormat( LogEntryList::DRAG ) ) {
   
    drop_data_ = DropData( LogEntryList::DRAG );
    
  } else {
    
    event->ignore();
    return;
  }
    
  event->acceptProposedAction();
  process_drop_timer_.start();
  
  Debug::Throw( "KeywordList::dropEvent - done.\n" );
  return;
  
}

//__________________________________________________________
void KeywordList::_updateOpenItems( QTreeWidgetItem* item )
{
  
  Debug::Throw() << "KeywordList::_updateOpenItems - size: " << open_items_.size() << endl;
  
  while( !open_items_.empty() )
  {
    // exit if item is found in list
    if( item && item == open_items_.top() ) return;
    
    // abort loop if parent item is found in list
    if( item && item->parent() == open_items_.top() ) break;
    collapseItem( open_items_.top() );
    open_items_.pop();
    
  }
  
  // adds current item if needed
  if( item && !isItemExpanded( item ) ) open_items_.push( item );
  Debug::Throw( "KeywordList::_updateOpenItems - done.\n" );
  
}

//__________________________________________________________
bool KeywordList::_acceptDrag( QDropEvent *event ) const
{
  
  Debug::Throw( "KeywordList::_acceptDrag.\n" );
  return
    event->mimeData()->hasFormat( KeywordList::DRAG ) || 
    event->mimeData()->hasFormat( LogEntryList::DRAG );
   
}

//__________________________________________________________
bool KeywordList::_startDrag( QMouseEvent *event )
{
  
  Debug::Throw( "KeywordList::_startDrag.\n" );

  // check current item. Cannot drag root item
  if( QTreeWidget::currentItem() == root_item_ ) return false;
  
  // stop edit timer to avoid conflicts.
  _resetEdit();
  
  // start drag
  QDrag *drag = new QDrag(this);
  
  // store data
  QMimeData *mime = new QMimeData();
  mime->setData( KeywordList::DRAG, QTreeWidget::currentItem()->text(KEYWORD).toAscii() );
  mime->setText( QString( current().c_str() ) );
  drag->setMimeData(mime);
  drag->start(Qt::CopyAction);  
  return true;
  
}

//__________________________________________________________
void KeywordList::_resetDrag( void )
{
  
  Debug::Throw( "KeywordList::_resetDrag.\n" );

  // cleans drop_item
  _updateOpenItems( 0 );  
  if( drop_item_ ) setItemSelected( drop_item_, drop_item_selected_ );
  drop_item_ = 0;
  drop_item_timer_.stop();
  
  Debug::Throw( "KeywordList::_resetDrag - done.\n" );
  
}
//__________________________________________________________
bool KeywordList::_processDrop( void )
{
  
  Debug::Throw( "KeywordList::_processDrop.\n" );
  assert( drop_item_ );
  string current_keyword( keyword( drop_item_ ) );
  if( drop_data_.type() == KeywordList::DRAG )
  {
    
    Debug::Throw( "KeywordList::_processDrop - KeywordList::DRAG.\n" );    
    _resetDrag();
    
    // retrieve full keyword of selected items
    QList<Item*> items( selectedItems<Item>() );
    for( QList<Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
    {
      string old_keyword( keyword( *iter ) );
      
      // make sure that one does not move an item to itself
      if( old_keyword == current_keyword ) continue;

      emit keywordChanged( old_keyword, current_keyword + "/" + qPrintable( drop_data_.value() ) );
    }
    
    return true;
    
  } else if( drop_data_.type() == LogEntryList::DRAG ) {

    Debug::Throw( "KeywordList::_processDrop - KeywordList::LogEntryList.\n" );
    
    // dragging from logEntry list
    _resetDrag();

    Debug::Throw( "KeywordList::_processDrop - signal emmited.\n" );
    emit entryKeywordChanged( current_keyword );
    
    return true;
    
  } else _resetDrag();
  
  return false;
  
}
