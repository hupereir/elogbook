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

#include "LogbookModel.h"
#include "LogEntry.h"
#include "Logbook.h"

//_______________________________________________________________________________________
QVariant LogbookModel::data( const QModelIndex& index, int role ) const
{

    // check index
    if( !contains( index ) ) return QVariant();

    // retrieve associated file info
    const auto& logbook( *get(index) );

    // return text associated to file and column
    if( role == Qt::DisplayRole )
    {

        switch( index.column() )
        {

            case FileName: return logbook.file().localName().get();
            case Entries: return int(Base::KeySet<LogEntry>(&logbook).size());
            case Created: return logbook.creation().toString();
            case Modified: return logbook.modification().toString();
            default: return QVariant();

        }

    } else if( role == Qt::TextAlignmentRole ) {

        switch( index.column() )
        {
            case Entries:
            case Created:
            case Modified:
            return Qt::AlignCenter;

            default: return QVariant();

        }

    }

    return QVariant();

}

//_______________________________________________________________________________________
QVariant LogbookModel::headerData(int section, Qt::Orientation, int role ) const
{
    if( section >= 0 && section < nColumns )
    {
        if( role == Qt::DisplayRole ) return columnTitles_[section];
        else if( role == Qt::TextAlignmentRole ) {

            switch( section )
            {
                case Entries:
                case Created:
                case Modified:
                return Qt::AlignCenter;

                default: return QVariant();

            }

        }

    }
    // return empty
    return QVariant();

}
