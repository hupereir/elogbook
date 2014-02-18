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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "AttachmentModel.h"
#include "AttachmentModel.moc"

#include "Attachment.h"
#include "IconEngine.h"
#include "Singleton.h"
#include "TimeStamp.h"
#include "XmlOptions.h"

//__________________________________________________________________
AttachmentModel::IconCache& AttachmentModel::_icons()
{
    static IconCache cache;
    return cache;
}

//_______________________________________________
const QString AttachmentModel::columnTitles_[ AttachmentModel::nColumns ] =
{
    QString(),
    tr( "File" ),
    tr( "Type" ),
    tr( "Size" ),
    tr( "Creation" ),
    tr( "Modification" )
};

//_______________________________________________________________
AttachmentModel::AttachmentModel( QObject* parent ):
    ListModel<Attachment*>( parent ),
    Counter( "AttachmentModel" )
{
    Debug::Throw( "AttachmentModel::AttachmentModel.\n" );
    connect( Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
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

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    // retrieve associated file info
    Attachment* attachment( get()[index.row()] );

    // return text associated to file and column
    if( role == Qt::DisplayRole )
    {

        switch( index.column() )
        {

            case Filename:
            return
                attachment->type() == AttachmentType::Url ?
                attachment->file() : attachment->shortFile();

            case Type: return attachment->type().name();

            case Size: return attachment->sizeString();

            case Creation:
            return (attachment->creation().isValid() ) ? attachment->creation().toString(): QVariant();

            case Modification:
            return (attachment->modification().isValid() ) ? attachment->modification().toString(): QVariant();

            default: return QVariant();
        }
    } else if( role == Qt::ToolTipRole ) return attachment->file();
    else if( role == Qt::DecorationRole && index.column() == Icon ) return _icon( attachment->type().icon() );
    else if( role == Qt::TextAlignmentRole && index.column() == Icon ) return Qt::AlignCenter;

    return QVariant();

}

//__________________________________________________________________
QVariant AttachmentModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if(
        orientation == Qt::Horizontal &&
        role == Qt::DisplayRole &&
        section >= 0 &&
        section < nColumns )
    { return QString( columnTitles_[section] ); }

    // return empty
    return QVariant();

}

//____________________________________________________________
void AttachmentModel::_updateConfiguration( void )
{
    Debug::Throw( "AttachmentModel::_updateConfiguration.\n" );
    _icons().clear();
}

//____________________________________________________________
void AttachmentModel::_sort( int column, Qt::SortOrder order )
{
    Debug::Throw() << "AttachmentModel::sort - column: " << column << " order: " << order << endl;
    std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );
}

//________________________________________________________
bool AttachmentModel::SortFTor::operator () ( Attachment* first, Attachment* second ) const
{

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        default:
        case Filename: return first->shortFile() < second->shortFile();
        case Icon: return first->type().name() < second->type().name();
        case Type: return first->type().name() < second->type().name();
        case Size: return  first->size() < second->size();
        case Creation: return first->creation() < second->creation();
        case Modification: return first->modification() < second->modification();

    }

}
//________________________________________________________
const QIcon& AttachmentModel::_icon( QString type )
{

    IconCache::const_iterator iter( _icons().find( type ) );
    if( iter != _icons().end() ) return iter.value();
    else return _icons().insert( type, IconEngine::get( type ) ).value();

}
