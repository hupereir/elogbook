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
#include <QTimer>

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

  Q_OBJECT
  
  public:
 
  //! used to tag Keyword drags
  static const QString DRAG;
 
  //! constructor
  LogEntryList( QWidget *parent );

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
  void update( LogEntry* entry, bool update_selection = true );
  
  //! select LogEntry in list
  void select( LogEntry* entry );
 
  //! add LogEntry to the list
  void add( LogEntry* entry, bool update_selection = false );
      
  //! resize columns to match shown entries
  void resizeColumns( void );
  
  //! handle listviewitem and logbook entry association
  class Item: public CustomListView::Item, public BASE::Key
  {
    public:
    
    //! constructor
    Item( LogEntryList* parent ):
      CustomListView::Item( parent )
    { 
      Debug::Throw( "LogEntryList::item::item.\n" ); 
      setFlag( Qt::ItemIsDragEnabled, true );
      setFlag( Qt::ItemIsDropEnabled, false );
    }
    
    //! retrieve associated entry
    LogEntry* entry( void ) const 
    { 
      BASE::KeySet<LogEntry> entries( this );
      Exception::check( entries.size()==1, DESCRIPTION("invalid association to entries") );
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
    Exception::checkPointer( entry, DESCRIPTION( "invalid entry" ) );

    Debug::Throw() << "LogEntryList::item - entry: " << entry->key() << std::endl;
    
    // retrieve associated entries
    BASE::KeySet<Item> items( entry );
    if( items.empty() ) return 0;
    if( items.size() > 1 ) throw std::logic_error( DESCRIPTION( "invalid association to Item" ) );
    return *items.begin();    
    
  }

  //! select previous item in list, if any
  Item* itemBelow( QTreeWidgetItem* item, bool update_selection = true );
  
  //! select next item in list, if any
  Item* itemAbove( QTreeWidgetItem* item, bool update_selection = true );    

  //! retrieve selected entries
  std::list< LogEntry* > selectedEntries( void );
  
  //! retrieve all entries
  std::list< LogEntry* > entries( void );
   
  signals:
  
  // emited when an items entry is activated
  void entrySelected( LogEntry* entry );
  
  // emited when an items entry title is modified
  void entryRenamed( LogEntry* entry, std::string );
  
  public slots:
  
  //! clear list
  void clear( void );
  
  protected slots:
  
  //! start editing current item
  /*! slot is activated at the end of the edit_timer_ delay */
  void _startEdit( void );
  
  //! reset edition
  void _resetEdit( const bool& restore_backup = true );
  
  //! recieved when item gets activated
  /*! used to retrieve edited text */
  void _activate( QTreeWidgetItem* );
  
  protected:

  //! mouse press events [needed to start drag]
  virtual void mousePressEvent( QMouseEvent *event );
  
  //! mouse move events [needed to start drag]
  virtual void mouseMoveEvent( QMouseEvent *event );
 
  //! mouse move events [needed to start drag]
  virtual void mouseReleaseEvent( QMouseEvent *event );
 
  //! start drag
  virtual bool _startDrag( void );

  private:
  
  //!@name list multi selection management
  //@{
  //! clicked item
  QTreeWidgetItem* first_item_;
  
  //! clicked item
  QTreeWidgetItem* last_item_;
  
  //@}
  
  //!@name list drag and drop
  //@{
  
  //! store possible mouse drag start position
  QPoint drag_start_;
 
  //! true when drag is allowed (as opposed to entry selection
  bool drag_enabled_;
  //@}
  
  //!@name editting
  //@{

  //! currently edited timer
  QTreeWidgetItem* edit_item_;

  //! backup title for edited item
  QString backup_;
    
  //! editing timer
  /*! editting is enabled only if a certain delay is passed during which no drag/drop starts */
  QTimer* edit_timer_;

  //! edit_item delay (ms)
  /* 
    it is used to start delayed edition of keywords
    directly from the list 
  */
  enum { edit_item_delay_ = 500 };

  //@}

  
};

#endif
