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

#include "Counter.h"
#include "ListModel.h"

#include <QtCore/QHash>

class Attachment;

//! Job model. Stores job information for display in lists
class AttachmentModel : public ListModel<Attachment*>, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    AttachmentModel( QObject* = 0 );

    //! destructor
    virtual ~AttachmentModel()
    {}

    //! number of columns
    enum { nColumns = 6 };

    //! column type enumeration
    enum ColumnType
    {
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
    virtual Qt::ItemFlags flags( const QModelIndex& ) const;

    //! return data
    virtual QVariant data( const QModelIndex&, int ) const;

    //! header data
    virtual QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole) const;

    //! number of columns for a given index
    virtual int columnCount( const QModelIndex& = QModelIndex() ) const
    { return nColumns; }

    //@}

    protected:

    //! sort
    virtual void _sort( int, Qt::SortOrder = Qt::AscendingOrder );

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
    static const QString columnTitles_[nColumns];

    //! icon
    static QIcon _icon( QString );

    //! icon cache
    typedef QHash<QString, QIcon> IconCache;

    //! type icon cache
    static IconCache& _icons();

};

#endif

