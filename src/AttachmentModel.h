#ifndef AttachmentModel_h
#define AttachmentModel_h

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
\file    AttachmentModel.h
\brief   Stores file information for display in lists
\author  Hugo Pereira
\version $Revision$
\date    $Date$
*/

#include "Counter.h"
#include "ListModel.h"

class Attachment;

//! Job model. Stores job information for display in lists
class AttachmentModel : public ListModel<Attachment*>, public Counter
{

  Q_OBJECT

  public:

  //! constructor
  AttachmentModel(QObject *parent = 0);

  //! destructor
  virtual ~AttachmentModel()
  {}

  //! number of columns
  enum { n_columns = 6 };

  //! column type enumeration
  enum ColumnType {
    ICON,
    FILE,
    TYPE,
    SIZE,
    CREATION,
    MODIFICATION
  };

  //!@name methods reimplemented from base class
  //@{

  //! flags
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;

  //! return data
  virtual QVariant data(const QModelIndex &index, int role) const;

  //! header data
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  //! number of columns for a given index
  virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
  { return n_columns; }

  //@}

  protected:

  //! sort
  virtual void _sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

  private slots:

  //! update configuration
  void _updateConfiguration( void );

  private:


  //! used to sort IconCaches
  class SortFTor: public ItemModel::SortFTor
  {

    public:

    //! constructor
    SortFTor( const int& type, Qt::SortOrder order = Qt::AscendingOrder ):
      ItemModel::SortFTor( type, order )
      {}

      //! prediction
      bool operator() ( Attachment* first, Attachment* second ) const;

    };

    //! list column names
    static const QString column_titles_[n_columns];

    //! icon
    static QIcon _icon( QString );

    //! icon cache
    typedef std::map<QString, QIcon> IconCache;

    //! type icon cache
    static IconCache& _icons();

  };

  #endif

