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

  //! used to tag Keyword drags
  static const QString DRAG;

  //! constructor
  KeywordList( QWidget *parent );

  //! number of columns
  enum { n_columns = 1 };

  //! column type enumeration
  enum ColumnTypes { KEYWORD };

  //! column titles
  static char* column_titles_[ n_columns ];

  //! add keyword, check for unicity
  void add( std::string keyword );

  //! remove keyword
  void remove( std::string keyword );

  //! reset
  void reset( std::set<std::string> keywords );

  //! select a keyword, if found
  void select( std::string keyword );

  //! get currently selected keyword
  std::string current( void );

  //! get full keyword from a given QTreeWidgetItem (appending parent keywords)
  std::string keyword( QTreeWidgetItem* item ) const;

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
      {}

    //! constructor
    /*! subitems can be renamed, as opposed to topLevel items */
    Item( Item* parent ):
      CustomListView::Item( parent )
      {}
  };

  //! root item
  Item* rootItem( void ) const
  { return root_item_; }

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

  protected slots:

  //! open drop item
  /*! this is connected to the time-out of the drop timer */
  void _openDropItem( void );

  //! start editting current item
  void _startEdit( void );

  //! item activated
  /*! used to catch end of item edition and propagate */
  void _activate( QTreeWidgetItem* item, int column );

  protected:

  //! mouse press events [needed to start drag]
  virtual void mousePressEvent( QMouseEvent *event );

  //! mouse move events [needed to start drag]
  virtual void mouseMoveEvent( QMouseEvent *event );

  //! drag enter event [overloaded]
  void dragEnterEvent( QDragEnterEvent* event );

  //! drag move event [overloaded]
  void dragMoveEvent( QDragMoveEvent* event );

  //! drag leave event [overloaded]
  void dragLeaveEvent( QDragLeaveEvent* event )
  {  _resetDrag(); }

  //! drop event [overload]
  void dropEvent( QDropEvent* event );

  private:

  //! create root item
  void _createRootItem( void );

  //!@name drag and drop methods
  //@{

  //! update opened item stack (when dragging to the list)
  void _updateOpenItems( QTreeWidgetItem* item );

  //! start drag sequence
  bool _startDrag( QMouseEvent *event );

  //! reset drag sequence
  void _resetDrag( void );

  //! return true if QMimeSource is an accepted TextDrag
  bool _acceptDrag( QDropEvent *event ) const;

  //! process drop action (for accepted drags)
  bool _processDrop( QDropEvent *event );

  //@}

  //! needed for smart reset
  class ContainsFTor
  {
    public:

    //! constructor
    ContainsFTor( const std::string& value ):
      value_( value )
      {}

    //! predicate
    bool operator() ( const std::string& value ) const
    { return value.find( value_ ) == 0; }

    private:

    //! predicted value
    std::string value_;

  };

  //! root Item
  Item *root_item_;

  //!@name drag and drop
  //@{

  //! drop target item
  Item *drop_item_;

  //! drop item 'initial' selection state
  bool drop_item_selected_;

  //! timer to open drop_item when selected
  QTimer drop_item_timer_;

  //! store possible mouse drag start position
  QPoint drag_start_;

  //! keep track of items which have been opened while dragging
  std::stack< QTreeWidgetItem* > open_items_;

  //! drop_item_open delay (ms)
  enum { drop_item_delay_ = 500 };

  //@}

  //!@name editting
  //@{

  //! currently edited timer
  QTreeWidgetItem* edit_item_;

  //! backup keyword
  QString backup_;
  
  //! backup keyword (including full path)
  std::string full_backup_;
  
  //! editing timer
  /*! editting is enabled only if a certain delay is passed during which no drag/drop starts */
  QTimer edit_timer_;

  //! edit_item delay (ms)
  /* 
    it is used to start delayed edition of keywords
    directly from the list 
  */
  enum { edit_item_delay_ = 500 };

  //@}

};

#endif
