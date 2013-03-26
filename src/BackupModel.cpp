// $Id$

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public license as published by the Free Software
* Foundation; either version 2 of the license, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public license
* for more details.
*
* You should have received a copy of the GNU General Public license along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "BackupModel.h"

//_______________________________________________
const QString BackupModel::columnTitles_[ BackupModel::nColumns ] =
{
    tr( "File" ),
    tr( "Path" ),
    tr( "Created" )
};

//__________________________________________________________________
Qt::ItemFlags BackupModel::flags(const QModelIndex &index) const
{

    // default flags
    Qt::ItemFlags flags;
    if( !index.isValid() ) return flags;

    // check associated record validity
    const Backup& backup( get(index) );
    if( !backup.isValid() ) return flags;

    // default flags
    flags |=  Qt::ItemIsEnabled |  Qt::ItemIsSelectable;

    return flags;

}

//_______________________________________________
QVariant BackupModel::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    const Backup backup( get( index ) );
    if( role == Qt::DisplayRole )
    {

        switch( index.column() )
        {
            case FILE: return backup.file().localName();
            case PATH: return backup.file().path();
            case CREATION: return backup.creation().toString();

            default:
            return QVariant();
        }

    } else if( role == Qt::TextAlignmentRole ) {

        switch( index.column() )
        {
            case CREATION:
            return Qt::AlignCenter;

            default: return QVariant();

        }

    }

    return QVariant();

}

//__________________________________________________________________
QVariant BackupModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if( section >= 0 && section < nColumns )
    {
        if( role == Qt::DisplayRole ) return columnTitles_[section];
        else if( role == Qt::TextAlignmentRole ) {

            switch( section )
            {
                case CREATION:
                return Qt::AlignCenter;

                default: return QVariant();

            }

        }

    }

    // return empty
    return QVariant();

}

//____________________________________________________________
void BackupModel::_sort( int column, Qt::SortOrder order )
{ std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) ); }

//________________________________________________________
bool BackupModel::SortFTor::operator () ( Backup first, Backup second ) const
{

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        case FILE: return first.file().localName() < second.file().localName();
        case PATH: return first.file().path() < second.file().path();
        case CREATION: return first.creation() < second.creation();
        default: return true;

    }

}
