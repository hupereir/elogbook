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

#include "KeywordModel.h"
#include "LogEntry.h"

//______________________________________________________________
const QString KeywordModel::columnTitles_[ KeywordModel::nColumns ] = { "Keywords" };

//______________________________________________________________
KeywordModel::KeywordModel( QObject* parent ):
    TreeModel<Keyword>( parent ),
    Counter( "KeywordModel" )
{}

//__________________________________________________________________
Qt::ItemFlags KeywordModel::flags(const QModelIndex &index) const
{
    if( !index.isValid() ) return Qt::ItemIsDropEnabled;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled| Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

//__________________________________________________________________
QVariant KeywordModel::data( const QModelIndex& index, int role ) const
{

    // check index
    if( !index.isValid() ) return QVariant();

    // check index, role and column
    if( role == Qt::DisplayRole ) return _find( index.internalId() ).get().current();
    else return QVariant();

}

//__________________________________________________________________
bool KeywordModel::setData(const QModelIndex &index, const QVariant& value, int role )
{
    Debug::Throw( "KeywordModel::setData.\n" );
    if( !(index.isValid() && role == Qt::EditRole ) ) return false;

    // retrieve parent index
    Keyword parent_keyword( get( parent( index ) ) );

    Keyword keyword( get( index ) );
    if( value.toString() != keyword.current() )
    {
        // generate new keyword from value
        Keyword newKeyword( parent_keyword.append( value.toString() ) );
        Debug::Throw() << "KeywordModel::setData - old: " << keyword << " new: " << newKeyword << endl;
        emit keywordChanged( keyword, newKeyword );
        emit dataChanged( index, index );
    }

    return true;

}

//__________________________________________________________________
QVariant KeywordModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < nColumns )
    { return columnTitles_[section]; }

    // return empty
    return QVariant();

}

//______________________________________________________________________
QStringList KeywordModel::mimeTypes( void ) const
{
    QStringList types;
    types << Keyword::MimeType << "text/plain";
    return types;
}

//______________________________________________________________________
QMimeData* KeywordModel::mimeData(const QModelIndexList &indexes) const
{

    // create mime data
    QMimeData *mime = new QMimeData();

    // add keywords mimetype
    foreach( const QModelIndex& index, indexes )
    { if( index.isValid() ) mime->setData( Keyword::MimeType, get( index ).get().toLatin1() ); }

    // add plain text mimetype
    QString buffer;
    foreach( const QModelIndex& index, indexes )
    {
        if( !index.isValid() ) continue;
        buffer += get(index).get();
    }

    // set plain text data
    mime->setData( "text/plain", buffer.toLatin1() );

    return mime;

}

//__________________________________________________________________
bool KeywordModel::dropMimeData(const QMimeData* data , Qt::DropAction action, int, int, const QModelIndex& parent)
{

    Debug::Throw( "KeywordModel::dropMimeData\n" );

    // check action
    if( action == Qt::IgnoreAction) return true;

    // Drag from Keyword model
    if( data->hasFormat( Keyword::MimeType ) )
    {

        // retrieve/check string
        QString keyword_string( data->data( Keyword::MimeType ) );
        if( keyword_string.isNull() || keyword_string.isEmpty() ) return false;

        // retrieve old keyword
        Keyword oldKeyword( keyword_string );

        // retrieve new location
        QModelIndex index = parent.isValid() ? parent : QModelIndex();
        Keyword newKeyword = get( index );

        // check that keyword is different
        if( newKeyword == oldKeyword ) return false;

        // append new keyword to old
        newKeyword.append( oldKeyword.current() );

        // emit keyword modification signal
        emit keywordChanged( oldKeyword, newKeyword );
        return true;
    }

    // drag from LogEntryModel
    if( data->hasFormat( LogEntry::MimeType ) )
    {

        // no drag if parent is invalid
        if( !parent.isValid() ) return false;

        // retrieve new location
        Keyword newKeyword = get( parent );

        // emit logEntry keyword changed signal
        emit entryKeywordChanged( newKeyword );
        return true;

    }

    return false;

}

//____________________________________________________________
void KeywordModel::_sort( int column, Qt::SortOrder order )
{
    Debug::Throw() << "KeywordModel::sort - column: " << column << " order: " << order << endl;
    _root().sort( SortFTor(order) );
}


//________________________________________________________
bool KeywordModel::SortFTor::operator () ( Keyword first, Keyword second ) const
{

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );
    return first < second;

}
