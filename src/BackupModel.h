#ifndef BackupModel_h
#define BackupModel_h

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

#include "Backup.h"
#include "Counter.h"
#include "ListModel.h"

#include <array>

//* model
class BackupModel: public ListModel< Backup >, private Base::Counter<BackupModel>
{

    Q_OBJECT

    public:

    //* constructor
    explicit BackupModel():
        Counter( "BackupModel" )
    {}

    //* column type enumeration
    enum ColumnType
    {
        FileName,
        Path,
        Creation,
        nColumns
    };

    //* flags
    Qt::ItemFlags flags( const QModelIndex& ) const override;

    //* return data
    QVariant data( const QModelIndex&, int ) const override;

    //* header data
    QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const override;

    //* number of columns for a given index
    int columnCount( const QModelIndex& = QModelIndex() ) const override
    { return nColumns; }

    protected:

    //* sort
    void _sort( int column, Qt::SortOrder order ) override;

    private:

    //* list column names
    const std::array<QString, nColumns> columnTitles_ =
    {{
        tr( "File" ),
        tr( "Path" ),
        tr( "Created" )
    }};

    //* used to sort IconCaches
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //* constructor
        explicit SortFTor( int type, Qt::SortOrder order ):
            ItemModel::SortFTor( type, order )
        {}

        //* prediction
        bool operator() ( Backup, Backup ) const;

    };

};

#endif
