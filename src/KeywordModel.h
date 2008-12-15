#ifndef KeywordModel_h
#define KeywordModel_h

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
  \file    KeywordModel.h
  \brief   keyword model
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "ColorMenu.h"
#include "Counter.h"
#include "Keyword.h"
#include "TreeModel.h"

//! Job model. Stores job information for display in lists
class KeywordModel : public TreeModel<Keyword>, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT
    
  public:

  //! used to tag Keyword drags
  static const QString DRAG;
  
  //! constructor
  KeywordModel(QObject *parent = 0);
  
  //! destructor
  virtual ~KeywordModel()
  {}
  
  //! number of columns
  enum { n_columns = 1 };

 //! column type enumeration
  enum ColumnType { KEYWORD };

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
   
  //! number of columns for a given index
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
  { return n_columns; }
 
  //! mime type
  virtual QStringList mimeTypes( void ) const;
   
  //! mime data
  virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
  
  //! drop mine data
  virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex&);

  //@}

  signals:

  /*! \brief
    emitted when a logEntryList drag is accepted.
    Sends the new keyword
  */
  void entryKeywordChanged( Keyword );

  /*! \brief
    emitted when a Keyword drag is accepted, or when keyword item
    is emitted directly in the list. Sends old keyword, new keyword
  */
  void keywordChanged( Keyword old_keyword, Keyword new_keyword );

  protected: 
  
  //! sort
  virtual void _sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
  
  private:
   
  //! list column names
  static const QString column_titles_[n_columns];
    
  //! used to sort Jobs
  class SortFTor
  {
    
    public:
    
    //! constructor
    SortFTor( Qt::SortOrder order = Qt::AscendingOrder ):
      order_( order )
      {}
      
    //! prediction
    bool operator() ( const Item& first, const Item& second ) const
    { return (*this)( first.get(), second.get() ); }
    
    //! prediction
    bool operator() ( Keyword first, Keyword second ) const;
    
    private:
    
    //! order
    Qt::SortOrder order_;
    
  };

};

#endif
