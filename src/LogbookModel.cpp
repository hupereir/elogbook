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

#include "LogbookModel.h"
#include "Logbook.h"
#include "LogEntry.h"

//_______________________________________________
const QString LogbookModel::columnTitles_[ LogbookModel::nColumns ] =
{
    tr( "File" ),
    tr( "Entries" ),
    tr( "Created" ),
    tr( "Modified" )
};


//_______________________________________________________________________________________
QVariant LogbookModel::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    // retrieve associated file info
    Logbook& logbook( *get()[index.row()] );

    // return text associated to file and column
    if( role == Qt::DisplayRole )
    {

        switch( index.column() )
        {

            case FILE: return logbook.file().localName();
            case ENTRIES: return int(BASE::KeySet<LogEntry>(&logbook).size());
            case CREATED: return logbook.creation().toString();
            case MODIFIED: return logbook.modification().toString();
            default: return QVariant();
        }
    }

    return QVariant();

}
