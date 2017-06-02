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

class Attachment;
class MimeTypeIconProvider;

//* Job model. Stores job information for display in lists
class AttachmentModel : public ListModel<Attachment*>, private Base::Counter<AttachmentModel>
{

    Q_OBJECT

    public:

    //* constructor
    AttachmentModel( QObject* = nullptr );

    //* column type enumeration
    enum ColumnType
    {
        Filename,
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
    void _sort( int, Qt::SortOrder ) override;

    //* icon matching given model index
    virtual QIcon _icon( const QModelIndex& ) const;

    private:


    //* used to sort IconCaches
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //* constructor
        SortFTor( int type, Qt::SortOrder order ):
            ItemModel::SortFTor( type, order )
        {}

        //* prediction
        bool operator() ( Attachment* first, Attachment* second ) const;

    };

    //* list column names
    static const QString columnTitles_[nColumns];

    //* icon provider
    MimeTypeIconProvider* iconProvider_ = nullptr;

};

#endif
