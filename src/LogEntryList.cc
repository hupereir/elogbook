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
   \file LogEntryList.cc
   \brief customized list view to handle LogEntry
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include "Debug.h"
#include "Logbook.h"
#include "LogEntryList.h"

using namespace std;
using namespace BASE;

//_______________________________________________
const char* LogEntryList::column_titles_[ LogEntryList::n_columns ] = 
{
  "title",
  "creation",
  "modification",
  "author",
  "color"
};

//_____________________________________________
LogEntryList::LogEntryList( QWidget *parent, const string& name ):
  CustomListView( parent )
{
  
  Debug::Throw( "LogEntryList::LogEntryList.\n" );
  
  setColumnCount( n_columns );
  for( unsigned int i=0; i<n_columns; i++ )
  { setColumnName( i, column_titles_[i] ); }

  setSelectionMode( QAbstractItemView::ContiguousSelection );

}
 
//_______________________________________________
void LogEntryList::addEntry( LogEntry* entry, bool update_selection )
{
  
  Debug::Throw("LogEntryList::AddEntry.\n" );
  Exception::assert( entry, DESCRIPTION( "invalid entry" ) );
  
  Item *item( new Item( this ) );
  BASE::Key::associate( item, entry );
  
  item->update( );

  if( update_selection ) {
    clearSelection();
    setItemSelected( item, true );
    scrollToItem( item );
  }
  
  return;
  
}

//_______________________________________________
void LogEntryList::updateEntry( LogEntry* entry, bool update_selection )
{
  
  Debug::Throw( "LogEntryList::UpdateEntry.\n" );
  Exception::assert( entry, DESCRIPTION( "wrong entry" ) );
  
  // get associated Item
  Item *item( LogEntryList::item( entry ) );
  
  // update item
  item->update();
  if( update_selection && entry->isKeywordSelected() ) 
  {
    clearSelection();
    setItemHidden( item, false );
    setItemSelected( item, true );
    scrollToItem( item );
  }
  
  return;  
}
   
//_______________________________________________
void LogEntryList::selectEntry( LogEntry* entry )
{
  Debug::Throw( "LogEntryList::SelectEntry.\n" );
  Exception::assert( entry, DESCRIPTION("invalid entry") );

  Item *item( LogEntryList::item( entry ) );

  clearSelection();
  setItemHidden( item, false );
  setItemSelected( item, true );
  scrollToItem( item );
  return;  
}

//___________________________________
list< LogEntry* > LogEntryList::selectedEntries( void )
{ 
  Debug::Throw( "LogEntryList::entries" );

  // retrieve logbook entries
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
  QList<Item*> items( CustomListView::children<Item>() );
  for( QList<Item*>::iterator iter = items.begin(); iter != items.end(); iter++ ) 
  {
    
    // check item visibility
    if( isItemHidden( *iter ) ) continue;
    entries.push_back( (*iter)->entry() );

  }
  
  return entries;
  
} 

//_______________________________________________
LogEntryList::Item* LogEntryList::itemBelow( QTreeWidgetItem* item, bool update_selection )
{
  
  Debug::Throw( "LogEntryList::ItemBelow.\n" );
  
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
  
  Item* out = dynamic_cast<Item*>( tmp );
  if( out && update_selection ) {
  
    clearSelection();
    setItemSelected( out, true );
    scrollToItem( out );
    
  }
  
  return out;
  
}

//_______________________________________________
LogEntryList::Item* LogEntryList::itemAbove( QTreeWidgetItem* item, bool update_selection )
{
  
  Debug::Throw( "LogEntryList::ItemBelow.\n" );
  
  QTreeWidgetItem* tmp = 0;
  
  // see if item has children
  if( !item->parent() )
  { 
    
    int index = indexOfTopLevelItem( item );
    if( index > 0 ) tmp = topLevelItem( index-1 );
    
  } else {
    
    // see if item is first child of parent
    // if yes, return item below parent
    int index = item->parent()->indexOfChild( item );
    if( index > 0 ) tmp = item->parent()->child(index-1);
    else tmp = item->parent();
    
  }
  
  // see if found item has children
  // one must retrieve the most downward child
  while( tmp->childCount() ) { tmp = tmp->child( tmp->childCount() - 1 ); }
  
  // if item is hidden, try the next one
  if( tmp && isItemHidden( tmp ) ) tmp = itemAbove( tmp, false );
  
  Item* out = dynamic_cast<Item*>( tmp );

  if( out && update_selection ) {
  
    setItemHidden( out, false );
    clearSelection();
    setItemSelected( out, true );
    scrollToItem( out );
    
  }
  
  return out;
  
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
  
}

//___________________________________
bool LogEntryList::Item::operator < (const QTreeWidgetItem& item ) const
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
  if( column == CREATION ) return entry()->creation() < local->entry()->creation();
  if( column == MODIFICATION ) return entry()->modification() < local->entry()->modification();

  // default case
  return QTreeWidgetItem::operator < (item);
  
}
