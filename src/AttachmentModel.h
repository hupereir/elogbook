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
    explicit AttachmentModel( QObject* = nullptr );

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
    Qt::ItemFlags flags( const QModelIndex& ) const override;

    //* return data
    QVariant data( const QModelIndex&, int ) const override;

    //* header data
    QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole) const override;

    //* number of columns for a given index
    int columnCount( const QModelIndex& = QModelIndex() ) const override
    { return nColumns; }

    //@}

    protected:

    //* sort
    void _sort( int, Qt::SortOrder ) override;

    //* icon matching given model index
    QIcon _icon( const QModelIndex& ) const;

    private:


    //* used to sort IconCaches
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //* constructor
        explicit SortFTor( int type, Qt::SortOrder order ):
            ItemModel::SortFTor( type, order )
        {}

        //* prediction
        bool operator() ( Attachment* first, Attachment* second ) const;

    };

    //* list column names
    const std::array<QString, nColumns> columnTitles_ =
    {{
        tr( "File" ),
        tr( "Size" ),
        tr( "Creation" ),
        tr( "Modification" )
    }};

    //* icon provider
    MimeTypeIconProvider* iconProvider_ = nullptr;

};

#endif
