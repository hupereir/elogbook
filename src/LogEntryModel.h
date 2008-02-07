#ifndef LogEntryModel_h
#define LogEntryModel_h

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
  \file    LogEntryModel.h
  \brief   Stores file information for display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "ColorMenu.h"
#include "Counter.h"
#include "ListModel.h"

class LogEntry;

//! Job model. Stores job information for display in lists
class LogEntryModel : public ListModel<LogEntry*>
{
 
  //! Qt meta object declaration
  Q_OBJECT
    
  public:

  //! used to tag Keyword drags
  static const QString DRAG;
    
  //! constructor
  LogEntryModel(QObject *parent = 0);
  
  //! destructor
  virtual ~LogEntryModel()
  {}
  
  //! number of columns
  enum { n_columns = 5 };

 //! column type enumeration
  enum ColumnType {
    COLOR,
    TITLE, 
    CREATION,
    MODIFICATION,
    AUTHOR
  };

  //!@name methods reimplemented from base class
  //@{

  //! flags
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  
  //! return data
  virtual QVariant data(const QModelIndex &index, int role) const;
  
  // modify data
  virtual bool setData(const QModelIndex &index, const QVariant& value, int role = Qt::EditRole );
   
  //! header data
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
 
  //! mime type
  virtual QStringList mimeTypes( void ) const;
   
  //! mime data
  virtual QMimeData* mimeData(const QModelIndexList &indexes) const;

  //! number of columns for a given index
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
  { return n_columns; }

  //@}
  
  //!@name edition
  //@{
  
  //! enable edition
  const bool& editionEnabled( void ) const
  { return edition_enabled_; }
  
  //! enable edition
  void setEditionEnabled( const bool& value )
  { edition_enabled_ = value; }
  
  //! edition index
  const QModelIndex& editionIndex( void ) const
  { return edition_index_; }
  
  //! edition index
  void setEditionIndex( const QModelIndex& index )
  { 
    edition_enabled_ = false;
    edition_index_ = index; 
  }
  
  //@}
  
  protected: 
  
  //! sort
  virtual void _sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

  private slots:
    
  //! update configuration
  void _updateConfiguration( void );
  
  private:
  
  //! reset icons
  void _resetIcons( void );
  
  //! create icon for a given color
  QIcon _createIcon( const QColor& ) const;
  
  //! used to sort LogEntrys
  class SortFTor
  {
    
    public:
    
    //! constructor
    SortFTor( const ColumnType& type, Qt::SortOrder order = Qt::AscendingOrder ):
      type_( type ),
      order_( order )
      {}
      
    //! prediction
    bool operator() ( LogEntry* first, LogEntry* second ) const;
    
    private:
    
    //! column
    ColumnType type_;
    
    //! order
    Qt::SortOrder order_;
    
  };

  //! edition flag
  bool edition_enabled_;
  
  //! edition index
  /*! needs to be stored to start delayed edition */
  QModelIndex edition_index_;
  
  //! list column names
  static const char* column_titles_[n_columns];
   
  //! color icon cache
  typedef std::map<QColor, QIcon, ColorMenu::ColorLessFTor> IconCache;
   
  //! color icon cache
  static IconCache icons_; 

};

#endif