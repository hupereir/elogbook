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
KeywordModel::KeywordModel( QObject* parent ):
    TreeModel( parent ),
    Counter( QStringLiteral("KeywordModel") )
{}

//__________________________________________________________________
Qt::ItemFlags KeywordModel::flags(const QModelIndex &index) const
{
    return index.isValid() ?
        Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled| Qt::ItemIsDragEnabled | Qt::ItemIsEditable:
        Qt::ItemIsDropEnabled;
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
QVariant KeywordModel::headerData(int section, Qt::Orientation, int role) const
{

    if( role == Qt::DisplayRole && section >= 0 && section < nColumns )
    { return columnTitles_[section]; }

    // return empty
    return QVariant();

}

//______________________________________________________________________
QStringList KeywordModel::mimeTypes() const
{ return { Keyword::MimeType, "text/plain" }; }

//______________________________________________________________________
QMimeData* KeywordModel::mimeData(const QModelIndexList &indexes) const
{

    // create mime data
    QMimeData *mime = new QMimeData;

    // add keywords mimetype
    for( const auto& index:indexes )
    { if( index.isValid() ) mime->setData( Keyword::MimeType, qPrintable( get( index ).get() ) ); }

    // add plain text mimetype
    QString buffer;
    for( const auto& index:indexes )
    {
        if( !index.isValid() ) continue;
        buffer += get(index).get();
    }

    // set plain text data
    mime->setData( QStringLiteral("text/plain"), qPrintable( buffer ) );

    return mime;

}

//__________________________________________________________________
bool KeywordModel::setData(const QModelIndex &index, const QVariant& value, int role )
{
    Debug::Throw( QStringLiteral("KeywordModel::setData.\n") );
    if( !(index.isValid() && role == Qt::EditRole ) ) return false;

    // retrieve parent index
    Keyword parentKeyword( get( parent( index ) ) );

    Keyword keyword( get( index ) );
    if( value.toString() != keyword.current() )
    {
        // generate new keyword from value
        Keyword newKeyword( parentKeyword.append( value.toString() ) );
        Debug::Throw() << "KeywordModel::setData - old: " << keyword << " new: " << newKeyword << Qt::endl;
        emit keywordChanged( keyword, newKeyword );
        emit dataChanged( index, index );
    }

    return true;

}

//__________________________________________________________________
bool KeywordModel::dropMimeData(const QMimeData* data , Qt::DropAction action, int, int, const QModelIndex& parent)
{

    Debug::Throw( QStringLiteral("KeywordModel::dropMimeData\n") );

    // check action
    if( action == Qt::IgnoreAction) return true;

    // Drag from Keyword model
    if( data->hasFormat( Keyword::MimeType ) )
    {

        // retrieve/check string
        QString keywordString( data->data( Keyword::MimeType ) );
        if( keywordString.isNull() || keywordString.isEmpty() ) return false;

        // retrieve old keyword
        Keyword oldKeyword( keywordString );

        // retrieve new location
        QModelIndex index = parent.isValid() ? parent : QModelIndex();
        Keyword newKeyword = get( index );

        // check that keyword is different
        if( newKeyword == oldKeyword ) return false;

        // append new keyword to old
        newKeyword.append( oldKeyword.current() );

        // delay keyword changed emission to fix issues with nested event loops
        keywordChangedData_.set(  oldKeyword, newKeyword );
        QMetaObject::invokeMethod( this, &KeywordModel::_requestKeywordChanged, Qt::QueuedConnection );

        return true;
    }

    // drag from LogEntryModel
    if( data->hasFormat( LogEntry::MimeType ) )
    {

        // no drag if parent is invalid
        if( !parent.isValid() ) return false;

        // retrieve new location
        Keyword newKeyword = get( parent );

        // delay keyword changed emission to fix issues with nested event loops
        keywordChangedData_.set(  newKeyword );
        QMetaObject::invokeMethod( this, &KeywordModel::_requestEntryKeywordChanged, Qt::QueuedConnection );

        return true;

    }

    return false;

}

//____________________________________________________________
void KeywordModel::_sort( int column, Qt::SortOrder order )
{
    Debug::Throw() << "KeywordModel::sort - column: " << column << " order: " << order << Qt::endl;
    _root().sort( SortFTor(order) );
}


//________________________________________________________
bool KeywordModel::SortFTor::operator () ( Keyword first, Keyword second ) const
{

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );
    return first < second;

}
