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
   \file KeywordList.cc
   \brief customized list view to handle LogEntry keyword
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QApplication>

#include "KeywordList.h"
#include "LogEntryList.h"
#include "LogEntry.h"

using namespace std;


//_______________________________________________
const QString KeywordList::DRAG = "KeywordList::Drag";
char* KeywordList::column_titles_[ KeywordList::n_columns ] = { "keyword" };

//_______________________________________________
KeywordList::KeywordList( QWidget *parent ):
  CustomListView( parent ),
  root_item_( 0 ),
  drop_item_( 0 ),
  drop_item_selected_( false ),
  drop_item_timer_( this ),
  edit_item_( 0 ),
  edit_timer_( this )
{
  
  Debug::Throw( "KeywordList::KeywordList.\n" );
  
  // configurate drop timer
  drop_item_timer_.setSingleShot( true );
  drop_item_timer_.setInterval( drop_item_delay_ );
  connect( &drop_item_timer_, SIGNAL( timeout() ), SLOT( _openDropItem() ) );

  edit_timer_.setSingleShot( true );
  edit_timer_.setInterval( edit_item_delay_ );
  connect( &edit_timer_, SIGNAL( timeout() ), SLOT( _startEdit() ) );
  
  setRootIsDecorated( true );    
  setAcceptDrops(true);  
  setAutoScroll(true);
  setEditTriggers( QAbstractItemView::NoEditTriggers );
  //setEditTriggers( QAbstractItemView::SelectedClicked );
  
  setColumnCount( n_columns );
  for( unsigned int i=0; i<n_columns; i++ )
  { setColumnName( i, column_titles_[i] ); }
  
  
  _createRootItem();
    
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
  list<string> keywords( LogEntry::parseKeyword( keyword ) );
  for( list<string>::iterator iter = keywords.begin(); iter!= keywords.end(); iter++ )
  {
    
    if( !iter->size() ) continue;

    Item* item( 0 );
    
    // loop over children, stop at matching keyword
    for( int i=0; i < parent_item->childCount(); i++ )
    {
      Item* child( dynamic_cast<Item*>(parent_item->child(i)) );
      if( child && (*iter) == qPrintable( child->text( KEYWORD ) ) )
      { 
        item = child;
        break;
      }
    }
          
    // if none found, create a new Item
    if( !item ) {
      item = new Item( parent_item );
      item->setText( KEYWORD, iter->c_str() );  
      item->storeBackup();
    }
    
    // set as parent before processing the next keyword in the list
    parent_item = item;
    
  }
  
}  

//_______________________________________________
void KeywordList::remove( string keyword )
{
  
  Debug::Throw() << "KeywordList::remove - keyword=" << keyword << endl;

  // check root item
  _createRootItem();
    
  // check keyword
  keyword = LogEntry::formatKeyword( keyword );
  
  // parse keyword, insert in keywords tree  
  Item* parent_item( root_item_ );
  
  // retrieve sub-keywords, separated by "/"
  bool found( true );
  list<string> keywords( LogEntry::parseKeyword( keyword ) );
  for( list<string>::iterator iter = keywords.begin(); iter!= keywords.end(); iter++ )
  {
    
    if( !iter->size() ) continue;

    Item* item( 0 );
    
    // loop over children, stop at matching keyword
    for( int i=0; i < parent_item->childCount(); i++ )
    {
      Item* child( dynamic_cast<Item*>(parent_item->child(i)) );
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
void KeywordList::reset( const set<string>& new_keywords )
{
  
  Debug::Throw( "KeywordList::reset.\n" );
  
  // retrieve current list of keywords
  set<string> old_keywords( keywords() );
  
  // remove keywords that are not included in new keywords
  for( set<string>::iterator iter = old_keywords.begin(); iter != old_keywords.end(); iter++ )
  {
    if( find_if( new_keywords.begin(), new_keywords.end(), ContainsFTor( *iter ) ) == new_keywords.end() )
    { remove( *iter ); }
  }
  
  // add keywords that are not found in old list
  for( set<string>::const_iterator iter = new_keywords.begin(); iter != new_keywords.end(); iter++ )
  { if( old_keywords.find( *iter ) == old_keywords.end() ) add( *iter ); }
  
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
  items = CustomListView::children();
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
  out = item->text( KEYWORD ) + "/" + out;

  return qPrintable(out);  
  
}
//_______________________________________________
set<string> KeywordList::keywords( void )
{
  Debug::Throw( "KeywordList::keywords.\n" );
  
  set<string> out;

  // look for matching item in list
  QList<QTreeWidgetItem*> items = CustomListView::children();
  for( QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  { out.insert( keyword( *iter ) ); }

  return out;
  
} 

//_______________________________________________
void KeywordList::clear( void )
{
  Debug::Throw( "KeywordList::clear.\n" );
  CustomListView::clear();
  root_item_ = 0;
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
  {
    closePersistentEditor( edit_item_ );
    dynamic_cast<Item*>( edit_item_ )->restoreBackup();
    edit_item_ = 0;
  }

  // left button handling
  if (event->button() == Qt::LeftButton) 
  {
    
    // store event position for drag-start
    drag_start_ = event->pos();
    
    // see if click occured on current item. Start Edit timer. 
    if( item && item == QTreeWidget::currentItem() && item != edit_item_ ) edit_timer_.start();
    else edit_timer_.stop();
    
  }
  
  return CustomListView::mousePressEvent( event );

}
  
//_____________________________________________________________
void KeywordList::mouseMoveEvent( QMouseEvent* event )
{
  
  Debug::Throw( "KeywordList::mouseMoveEvent.\n" );
  
  // check button
  if( !(event->buttons()&Qt::LeftButton) )  return CustomListView::mouseMoveEvent( event );
  
  // check distance to last click
  if( (event->pos() - drag_start_ ).manhattanLength() < QApplication::startDragDistance() )
  { return CustomListView::mouseMoveEvent( event ); }
  
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
  Item *item( dynamic_cast<Item*>( itemAt( event->pos()) ) );

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
  Item *item( dynamic_cast<Item*>( itemAt( event->pos() ) ) );
  
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
  if( !_acceptDrag( event ) ) event->ignore();
  else
  {
    _processDrop( event );
    _resetDrag();
  }
  
  return;
  
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
  dynamic_cast<Item*>(edit_item_)->storeBackup();
  openPersistentEditor( edit_item_ );
}

//__________________________________________________________
void KeywordList::_createRootItem( void )
{ 
  Debug::Throw( "KeywordList::_createRootItem.\n" );
  
  if( root_item_ ) return;
  root_item_ = new Item( this );
  root_item_->setText( KEYWORD, LogEntry::NO_KEYWORD.c_str() );
}

//__________________________________________________________
void KeywordList::_updateOpenItems( QTreeWidgetItem* item )
{
  
  Debug::Throw( "KeywordList::_createRootItem.\n" );
  
  while( open_items_.size() )
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
  edit_timer_.stop();
  
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
  
}
//__________________________________________________________
bool KeywordList::_processDrop( QDropEvent *event )
{
  
  Debug::Throw( "KeywordList::_processDrop.\n" );
  
  // retrieve item below pointer
  Item *item( dynamic_cast<Item*>(itemAt( event->pos() ) ) );
  if( !item ) return false;
  string current_keyword( keyword( item ) );
  
  // retrieve event keyword
  const QMimeData& mime( *event->mimeData() );
  if( mime.hasFormat( KeywordList::DRAG ) )
  {
   
    // dragging from one keyword to another
    // retrieve old keyword
    QString value( mime.data( KeywordList::DRAG ) );
    if( value.isNull() || value.isEmpty() ) return false;
    
    _resetDrag();
    
    // retrieve full keyword of selected items
    QList<Item*> items( selectedItems<Item>() );
    for( QList<Item*>::iterator iter = items.begin(); iter != items.end(); iter++ )
    {
      string old_keyword( keyword( *iter ) );
      
      // make sure that one does not move an item to itself
      if( old_keyword == current_keyword ) continue;

      emit keywordChanged( old_keyword, current_keyword + "/" + qPrintable( value ) );
    }
    
    return true;
  }
  
  if( mime.hasFormat( LogEntryList::DRAG ) )
  {
    
    // dragging from logEntry list
    _resetDrag();
    emit keywordChanged( current_keyword );
    
  }
  
  return false;
  
}
