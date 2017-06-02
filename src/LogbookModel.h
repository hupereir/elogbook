#ifndef LogbookModel_h
#define LogbookModel_h

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

class Logbook;

//* logbook information model
class LogbookModel: public ListModel<Logbook*>, private Base::Counter<LogbookModel>
{

    Q_OBJECT

    public:

    //* column type enumeration
    enum ColumnType
    {
        Filename,
        Entries,
        Created,
        Modified,
        nColumns
    };

    //* constructor
    LogbookModel( QObject* parent = nullptr ):
        ListModel<Logbook*>( parent ),
        Counter( "LogbookStatisticsDialog::Model" )
    {}

    //*@name methods reimplemented from base class
    //@{

    //* flags
    virtual Qt::ItemFlags flags( const QModelIndex& ) const
    { return Qt::ItemIsEnabled |  Qt::ItemIsSelectable; }

    //* return data
    virtual QVariant data( const QModelIndex&, int ) const;

    //* header data
    virtual QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole) const;

    //* number of columns for a given index
    virtual int columnCount( const QModelIndex& = QModelIndex() ) const
    { return nColumns; }

    //@}

    protected:

    //* list column names
    static const QString columnTitles_[nColumns];

};

#endif
