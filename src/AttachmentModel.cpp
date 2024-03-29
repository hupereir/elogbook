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

#include "AttachmentModel.h"
#include "Attachment.h"
#include "IconEngine.h"
#include "MimeTypeIconProvider.h"
#include "TimeStamp.h"


//_______________________________________________________________
AttachmentModel::AttachmentModel( QObject* parent ):
    ListModel( parent ),
    Counter( QStringLiteral("AttachmentModel") )
{
    Debug::Throw( QStringLiteral("AttachmentModel::AttachmentModel.\n") );
    iconProvider_ = new MimeTypeIconProvider( this );
}

//__________________________________________________________________
Qt::ItemFlags AttachmentModel::flags(const QModelIndex &index) const
{

    // default flags
    Qt::ItemFlags flags;
    if( index.isValid() && contains( index ) )
    {

        // check associated record validity
        const Attachment& attachment( *get(index) );
        if( attachment.isValid() ) flags |=  Qt::ItemIsEnabled |  Qt::ItemIsSelectable;

    }

    return flags;

}

//__________________________________________________________________
QVariant AttachmentModel::data( const QModelIndex& index, int role ) const
{

    // check index
    if( !contains( index ) ) return QVariant();

    // retrieve associated file info
    Attachment* attachment( get()[index.row()] );

    // return text associated to file and column
    if( role == Qt::DisplayRole )
    {

        switch( index.column() )
        {

            case FileName:
            return attachment->isUrl() ? attachment->file().get() : attachment->shortFile().get();

            case Size: return attachment->sizeString();

            case Creation:
            return (attachment->creation().isValid() ) ? attachment->creation().toString(): QVariant();

            case Modification:
            return (attachment->modification().isValid() ) ? attachment->modification().toString(): QVariant();

            default: return QVariant();
        }
    } else if( role == Qt::ToolTipRole ) return attachment->file().get();
    else if( role == Qt::DecorationRole && index.column() == FileName ) return _icon( index );

    return QVariant();

}

//__________________________________________________________________
QVariant AttachmentModel::headerData(int section, Qt::Orientation, int role) const
{

    if(
        role == Qt::DisplayRole &&
        section >= 0 &&
        section < nColumns )
    { return QString( columnTitles_[section] ); }

    // return empty
    return QVariant();

}

//____________________________________________________________
void AttachmentModel::_sort( int column, Qt::SortOrder order )
{
    Debug::Throw() << "AttachmentModel::sort - column: " << column << " order: " << order << Qt::endl;
    std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );
}

//________________________________________________________
bool AttachmentModel::SortFTor::operator () ( Attachment* first, Attachment* second ) const
{

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        default:
        case FileName: return first->shortFile() < second->shortFile();
        case Size: return  first->size() < second->size();
        case Creation: return first->creation() < second->creation();
        case Modification: return first->modification() < second->modification();

    }

}
//________________________________________________________
QIcon AttachmentModel::_icon( const QModelIndex& index ) const
{

    Attachment* attachment( get()[index.row()] );
    if( attachment->isUrl() ) return IconEngine::get( "text-html.png" );
    else return iconProvider_->icon( attachment->file().extension() );
}
