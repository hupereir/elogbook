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

#include <qpainter.h>

#include "KeywordList.h"
#include "LogEntry.h"

using namespace std;


//_______________________________________________
char* KeywordList::column_titles_[ KeywordList::n_columns ] = { "keyword" };
const string KeywordList::DRAG = "KEYWORDLIST_DRAG";

//_______________________________________________
KeywordList::KeywordList( QWidget *parent ):
  CustomListView( parent ),
  root_item_( 0 ),
  drop_item_( 0 ),
  drop_item_timer_( this )
{
  
  Debug::Throw( "KeywordList::KeywordList.\n" );
  
  // configurate drop timer
  drop_item_timer_.setSingleShot( true );
  drop_item_timer_.setInterval( drop_item_delay_ );
  connect( &drop_item_timer_, SIGNAL( timeout() ), SLOT( _OpenDropItem() ) );
  
  setRootIsDecorated( true );  

  setColumnCount( n_columns );
  for( unsigned int i=0; i<n_columns; i++ )
  { setColumnName( i, column_titles_[i] ); }
  
  // connect direct item renaiming
  // setSelectionMode( Extended );
  // setDefaultRenameAction( Accept );
  
  _createRootItem();
    
}

//_______________________________________________
void KeywordList::addKeyword( string keyword )
{
  
  Debug::Throw() << "KeywordList::AddKeyword - keyword=" << keyword << endl;

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
    }
    
    // set as parent before processing the next keyword in the list
    parent_item = item;
    
  }
  
}  

//_______________________________________________
void KeywordList::selectKeyword( string keyword )
{
  
  Debug::Throw() << "KeywordList::selectKeyword - " << keyword << endl;

  // format
  keyword = LogEntry::formatKeyword( keyword );
    
  // retrieve selected items
  QList<QTreeWidgetItem*> items( QTreeWidget::selectedItems() );
  for( QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  { 
    if( keyword == qPrintable( (*iter)->text(KEYWORD) ) )
    {
      emit currentItemChanged( *iter, currentItem() );
      break;
    }
  }
  
  // look for matching item in list
  items = CustomListView::items();
  for( QList<QTreeWidgetItem*>::iterator iter = items.begin(); iter != items.end(); iter++ )
  {
    if( KeywordList::keyword( *iter ) == keyword )
    {    
      setCurrentItem( *iter );
      scrollToItem( *iter );
      return;
    }
  }
  
  // no keyword found returns root item
  setCurrentItem( root_item_ );
  scrollToItem( root_item_ );
  return;
  
}

//_______________________________________________
string KeywordList::currentKeyword( void )
{
  Debug::Throw( "KeywordList::currentKeyword.\n" );
  
  QList<QTreeWidgetItem*> items( QTreeWidget::selectedItems() );
  return items.size() ? keyword( items.back() ):qPrintable( root_item_->text( KEYWORD ) );
        
}

//_______________________________________________
set<string> KeywordList::keywords( void )
{
  Debug::Throw( "KeywordList::keywords.\n" );
  
  set<string> out;

  // look for matching item in list
  QList<QTreeWidgetItem*> items = CustomListView::items();
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
void KeywordList::dragEnterEvent( QDragEnterEvent* event )
{
  Debug::Throw( "KeywordList::dragEnterEvent.\n" );

  // check if object can be decoded
  if( !_acceptTextDrag( event ) ) return;   

  // retrieve item below pointer
  Item *item( dynamic_cast<Item*>( itemAt( event->pos()) ) );

  if( item ) {
    
    _updateOpenItems( item );
    
    drop_item_ = item;
    drop_item_timer_.start();
        
    // change colors
    drop_item_->storeColors( KEYWORD );
    drop_item_->setBackgroundColor( KEYWORD, palette().color( QPalette::Highlight ) );
    drop_item_->setTextColor( KEYWORD, palette().color( QPalette::HighlightedText ) );
   
  }
      
}

//__________________________________________________________
void KeywordList::dragMoveEvent( QDragMoveEvent* event )
{
  Debug::Throw( "KeywordList::dragMoveEvent.\n" ); 

  // check if event can be decoded
  if( !_acceptTextDrag( event ) ) return;
    
  // retrieve item below pointer
  Item *item( dynamic_cast<Item*>( itemAt( event->pos() ) ) );
  
  // update drop item  
  if( item ) 
  {            
    
    _updateOpenItems( item );
    
    // see if drop item has changes
    if( drop_item_ && drop_item_ != item )
    { drop_item_->restoreColors( KEYWORD ); }

    if( drop_item_ != item )
    {
      
      // change drop item
      drop_item_ = item;
      
      // change colors
      drop_item_->storeColors( KEYWORD );
      drop_item_->setBackgroundColor( KEYWORD, palette().color( QPalette::Highlight ) );
      drop_item_->setTextColor( KEYWORD, palette().color( QPalette::HighlightedText ) );
      
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
  if( !_acceptTextDrag( event ) )  
  {
    _resetDrag();
    return;
  }
  
  // retrieve item below pointer
  Item *item( dynamic_cast<Item*>( itemAt( event->pos() ) ) );
  
  // retrieve event keyword
  QString value( event->mimeData()->text() );
  if( value == QString( LogEntry::DRAG.c_str() ) ) 
  {
    
    _resetDrag();
    emit keywordChanged( keyword( item ) );
  
  } else if( value == QString( KeywordList::DRAG.c_str() ) )
  {
   
    // retrieve current Selection
    Item *old_item( dynamic_cast<Item*>(currentItem()) );
    if( !old_item || old_item == item ) return;
    
    // retrieve and cat keywords
    string old_keyword( qPrintable( old_item->text(KEYWORD) ) );
    string new_keyword( keyword( item ) );
    
    if( new_keyword.size() && old_keyword.size() ) new_keyword = new_keyword+"/"+old_keyword;
    else if( new_keyword.empty() ) new_keyword = old_keyword;
    
    _resetDrag();
    
    // notify that keywords are to be changed
    emit keywordChanged( keyword( old_item ), new_keyword );
    
  }
  
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
bool KeywordList::_acceptTextDrag( QDropEvent *event )
{
  
  Debug::Throw( "KeywordList::_acceptTextDrag.\n" );
  QString value( event->mimeData()->text() );
  return value == QString( LogEntry::DRAG.c_str() ) || value == QString( KeywordList::DRAG.c_str() );
   
}

//__________________________________________________________
void KeywordList::_resetDrag( void )
{
  
  Debug::Throw( "KeywordList::_resetDrag.\n" );

  // cleans drop_item
  _updateOpenItems( 0 );  
  if( drop_item_ ) drop_item_->restoreColors( KEYWORD );
  
  drop_item_ = 0;
  drop_item_timer_.stop();
  
}
