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

#ifndef LogEntryList_h
#define LogEntryList_h

/*!
  \file  LogEntryList.h
  \brief  customized list view to handle LogEntry
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <string>
#include <list>

#include "Counter.h"
#include "CustomListView.h"
#include "Debug.h"
#include "Exception.h"
#include "Key.h"
#include "LogEntry.h"

class LogEntry;
class QPainter;

/*!
  \class  LogEntryList
  \brief  customized ListView to handle LogEntry
*/ 
class LogEntryList: public CustomListView
{ 

  public:
  
  //! constructor
  LogEntryList( QWidget *parent, const std::string& name = "log_entry_list" );

  //! number of columns
  enum { n_columns = 5 };

  //! column type enumeration
  enum ColumnTypes {
    TITLE, 
    CREATION,
    MODIFICATION,
    AUTHOR,
    COLOR
  };
  
  //! column titles
  static const char* column_titles_[ n_columns ];
  
  //! update LogEntry in list
  void updateEntry( LogEntry* entry, bool update_selection = true );
  
  //! select LogEntry in list
  void selectEntry( LogEntry* entry );
      
  //! handle listviewitem and logbook entry association
  class Item: public CustomListView::Item, public BASE::Key
  {
    public:
    
    //! constructor
    Item( LogEntryList* parent ):
      CustomListView::Item( parent )
    { 
      setFlag( Qt::ItemIsDragEnabled, true );
      setFlag( Qt::ItemIsDropEnabled, false );
      setFlag( Qt::ItemIsEditable, true );
    }
    
    //! retrieve associated entry
    LogEntry* entry( void ) const 
    { 
      BASE::KeySet<LogEntry> entries( this );
      Exception::assert( entries.size()==1, DESCRIPTION("invalid association to entries") );
      return *entries.begin();
    }
    
    //! update item text using associated entry
    void update( void );
   
    //! order operator
    virtual bool operator<( const QTreeWidgetItem &other ) const;
              
  };
  
  //! retrieve Item associated to given entry
  Item* item( LogEntry* entry )
  {
    Debug::Throw( "LogEntryList::item.\n" );
    Exception::assert( entry, DESCRIPTION( "invalid entry" ) );
    
    // retrieve associated entries
    BASE::KeySet<Item> items( entry );
    Exception::assert( items.size()==1, DESCRIPTION( "invalid association to Item" ) );
    return *items.begin();    
    
  }
  
  //! add LogEntry to the list
  void addEntry( LogEntry* entry, bool update_selection = false );

  //! select previous item in list, if any
  Item* itemBelow( QTreeWidgetItem* item, bool update_selection = true );
  
  //! select next item in list, if any
  Item* itemAbove( QTreeWidgetItem* item, bool update_selection = true );    
  
  //! retrieve all entries
  std::list< LogEntry* > entries( void );

  //! retrieve selected entries
  std::list< LogEntry* > selectedEntries( void );
    
  // protected:

  //! dragging [overloaded]
  //QDragObject* dragObject( void )
  //{ return new QTextDrag( LogEntry::DRAG.c_str(), this ); }

};

#endif
