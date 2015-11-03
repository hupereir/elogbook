#ifndef AttachmentModel_h
#define AttachmentModel_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Counter.h"
#include "ListModel.h"

#include <QHash>

class Attachment;

//* Job model. Stores job information for display in lists
class AttachmentModel : public ListModel<Attachment*>, public Counter
{

    Q_OBJECT

    public:

    //* constructor
    AttachmentModel( QObject* = nullptr );

    //* column type enumeration
    enum ColumnType
    {
        Icon,
        Filename,
        Type,
        Size,
        Creation,
        Modification,
        nColumns
    };

    //*@name methods reimplemented from base class
    //@{

    //* flags
    virtual Qt::ItemFlags flags( const QModelIndex& ) const;

    //* return data
    virtual QVariant data( const QModelIndex&, int ) const;

    //* header data
    virtual QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole) const;

    //* number of columns for a given index
    virtual int columnCount( const QModelIndex& = QModelIndex() ) const
    { return nColumns; }

    //@}

    protected:

    //* sort
    virtual void _sort( int, Qt::SortOrder = Qt::AscendingOrder );

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration( void );

    private:


    //* used to sort IconCaches
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //* constructor
        SortFTor( const int& type, Qt::SortOrder order = Qt::AscendingOrder ):
            ItemModel::SortFTor( type, order )
        {}

        //* prediction
        bool operator() ( Attachment* first, Attachment* second ) const;

    };

    //* list column names
    static const QString columnTitles_[nColumns];

    //* icon
    static const QIcon& _icon( QString );

    //* icon cache
    using IconCache = QHash<QString, QIcon>;

    //* type icon cache
    static IconCache& _icons();

};

#endif
