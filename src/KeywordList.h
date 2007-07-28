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

#ifndef KeywordList_h
#define KeywordList_h

/*!
  \file KeywordList.h
  \brief customized list view to handle LogEntry keywords only in tree mode
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDrag>
#include <QTimer>

#include <set>
#include <stack>
#include <string>

#include "Counter.h"
#include "CustomListView.h"
#include "Debug.h"
#include "Exception.h"

/*!
  \class KeywordList
  \brief customized ListView to handle LogEntry keyword
*/ 
class KeywordList: public CustomListView
{ 

  //! Qt meta object declaration
  Q_OBJECT

  public:
  
  //! constructor
  KeywordList( QWidget *parent );

  //! used to tag Keyword drags
  static const std::string DRAG;
  
  //! number of columns
  enum { n_columns = 1 };

  //! column type enumeration
  enum ColumnTypes { KEYWORD };
  
  //! column titles
  static char* column_titles_[ n_columns ];

  //! add keyword, check for unicity
  void addKeyword( std::string keyword );
      
  //! select a keyword, if found
  void selectKeyword( std::string keyword );

  //! get currently selected keyword
  std::string currentKeyword( void );

  //! get all registered keywords
  std::set< std::string > keywords( void );
  
  //! local items
  class Item: public CustomListView::Item 
  {
    
    public:
        
    /*! 
      constructor. Only the "root" item is created from the list
      it cannot be dragged
    */
    Item( KeywordList* parent ):
      CustomListView::Item( parent )
    { 
      // setRenameEnabled( KEYWORD, false );
      setFlag( Qt::ItemIsDragEnabled, false ); 
      setFlag( Qt::ItemIsDropEnabled,true );
      setFlag( Qt::ItemIsEditable, false );
    }

    //! constructor. All "sub items" can be dragged       
    Item( Item* parent ):
      CustomListView::Item( parent )
    { 
      // setRenameEnabled( KEYWORD, true );
      setFlag( Qt::ItemIsDragEnabled, true ); 
      setFlag( Qt::ItemIsDropEnabled,true );
      setFlag( Qt::ItemIsEditable, true );
   }
    
    //! store background color
    void storeColors( int column )
    { 
      foreground_ = textColor( column );
      background_ = backgroundColor( column ); 
    }
    
    //! restore background color
    void restoreColors( int column )
    { 
      if( foreground_.isValid() ) setTextColor( column, foreground_ );
      if( background_.isValid() ) setBackgroundColor( column, background_ ); 
    }
    
    //! retrieves backup keyword
    const QString& backup( void ) const
    { return backup_; }

//     //! stores original keyword when renaming [overloaded]
//     void startRename( int column )
//     {
//       if( column == KEYWORD )
//       backup_ = (dynamic_cast<KeywordList*>( listView() ) )->GetKeyword( this );
//       CustomListView::Item::startRename( column );
//     }
                      
    private:
        
    //! unselected foreground color
    QColor foreground_;
       
    //! unselected background color
    QColor background_;
    
    //! backup of the keyword for rename actions
    QString backup_;
    
  };
  
  //! root item
  Item* rootItem( void ) const
  { return root_item_; }
        
  //! get full keyword from a given QTreeWidgetItem (appending parent keywords)
  std::string keyword( QTreeWidgetItem* item ) const;

  signals:
  
  /*! 
    emitted when a logEntryList drag is accepted.
    sends the new keyword
  */
  void keywordChanged( const std::string& new_keyword );  

  /*!
    emitted when a Keyword drag is accepted
    sends old keyword, new keyword
  */
  void keywordChanged( const std::string& old_keyword, const std::string& new_keyword );
        
  public slots:
    
  //! clear list and set of unique keywords
  void clear();      

  protected:
  
  //! returns supported drop actions
  //QtDropActions supportedDropActions( void ) const
  //{}
  
  //! drag enter event [overloaded]
  void dragEnterEvent( QDragEnterEvent* event );                                
    
  //! drag move event [overloaded]
  void dragMoveEvent( QDragMoveEvent* event );                                

  //! drag leave event [overloaded]
  void dragLeaveEvent( QDragLeaveEvent* event )
  {  _resetDrag(); }

  //! drop event [overload]
  void dropEvent( QDropEvent* event );

  //! dragging [overloaded]
  //QDragObject* dragObject( void )
  //{ return new QTextDrag( DRAG.c_str(), this ); }
  
  private slots:
  
  //! open drop item
  /*! this is connected to the time-out of the drop timer */
  void _openDropItem( void )
  {
    if( drop_item_ && !isItemExpanded( drop_item_ ) )
    expandItem( drop_item_ );
  }
              
  private:
  
  //! create root item
  void _createRootItem( void );

  //! update opened item stack (when dragging to the list)
  void _updateOpenItems( QTreeWidgetItem* item );

  //! reset drag sequence
  void _resetDrag( void );
  
  //! return true if QMimeSource is an accepted TextDrag
  static bool _acceptTextDrag( QDropEvent *event );
    
  //! root Item
  Item *root_item_;
            
  //! drop target item
  Item *drop_item_;

  //! timer to open drop_item when selected
  QTimer drop_item_timer_;

  //! keep track of items which have been opened while dragging
  std::stack< QTreeWidgetItem* > open_items_;  
  
  //! drop_item_open delay (ms)
  static const int drop_item_delay_ = 500;      
  
};

#endif
