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

#include "LogEntryModel.h"

#include "Attachment.h"
#include "ColorMenu.h"
#include "CustomPixmap.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "LogEntry.h"
#include "Singleton.h"
#include "TimeStamp.h"
#include "QtUtil.h"
#include "XmlOptions.h"

#include <QIcon>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>

//_______________________________________________
LogEntryModel::IconCache& LogEntryModel::_icons()
{
    static IconCache cache;
    return cache;
}

//_______________________________________________
const QString LogEntryModel::columnTitles_[ LogEntryModel::nColumns ] =
{
    "",
    tr( "Keyword" ),
    tr( "Subject" ),
    "",
    tr( "Creation" ),
    tr( "Modification" ),
    tr( "Author" )
};

//_______________________________________________________________
LogEntryModel::LogEntryModel( QObject* parent ):
    ListModel<LogEntry*>( parent ),
    Counter( "LogEntryModel" ),
    editionEnabled_( false ),
    iconSize_( 8 )
{
    Debug::Throw( "LogEntryModel::LogEntryModel.\n" );

    connect( Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    connect( this, SIGNAL(layoutChanged()), SLOT(_disableEdition()) );
    _updateConfiguration();

}

//__________________________________________________________________
Qt::ItemFlags LogEntryModel::flags(const QModelIndex &index) const
{

    if (!index.isValid()) return 0;

    // default flags
    Qt::ItemFlags out( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

    // check against edition index
    if( index == editionIndex() && editionEnabled() )
    { out |= Qt::ItemIsEditable; }

    // check column
    out |= Qt::ItemIsDragEnabled;
    return out;

}

//__________________________________________________________________
QVariant LogEntryModel::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    // retrieve associated file info
    LogEntry* entry( get()[index.row()] );

    // return text associated to file and column
    if( role == Qt::DisplayRole ) {

        switch( index.column() )
        {

            case Keyword:
            {
                QString keyword( entry->keyword().get() );
                if( keyword.size() > 1 && keyword[0] == '/' )
                { keyword = keyword.mid( 1 ); }

                return keyword;
            }

            case Title: return entry->title();
            case Creation: return entry->creation().toString();
            case Modification: return entry->modification().toString();
            case Author: return entry->author();

            default:
            return QVariant();
        }

    } else if( role == Qt::DecorationRole ) {

        switch( index.column() )
        {
            case Color:
            return _icon( entry->color() );

            case HasAttachment:
            return Base::KeySet<Attachment>(entry).empty() ? QVariant():_attachmentIcon();

            default:
            return QVariant();

        }

    } else if( role == Qt::TextAlignmentRole ) {

        switch( index.column() )
        {
            case Color:
            case Creation:
            case Modification:
            case Author:
            return Qt::AlignCenter;

            default: return QVariant();

        }

    } else if( role == Qt::SizeHintRole ) {

        switch( index.column() )
        {

            case HasAttachment:
            case Color:
            return QSize( iconSize_, iconSize_ );

            default:
            return QVariant();

        }

    }

    return QVariant();

}

//__________________________________________________________________
bool LogEntryModel::setData(const QModelIndex &index, const QVariant& value, int role )
{
    Debug::Throw( "LogEntryModel::setData.\n" );

    if( !editionEnabled() )
    {
        Debug::Throw(0, "LogEntryModel::setData - edition is disabled. setData canceled.\n" );
        return false;
    }

    if( !(index.isValid() && ( index.column() == Title || index.column() == Keyword ) && role == Qt::EditRole ) )
    {
        Debug::Throw(0, "LogEntryModel::setData - invalid index/role. setData canceled.\n" );
        return false;
    }

    LogEntry* entry( get( index ) );
    if( !entry )
    {
        Debug::Throw(0, "LogEntryModel::setData - entry is null. setData canceled.\n" );
        return false;
    }

    if( index.column() == Title && value != entry->title() )
    {

        entry->setTitle( value.toString() );
        emit dataChanged( index, index );

    } else if( index.column() == Keyword && value != entry->keyword().get() )  {

        entry->setKeyword( value.toString() );
        emit dataChanged( index, index );

    }

     return true;

}

//__________________________________________________________________
QVariant LogEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if( role == Qt::DisplayRole )
    {
        switch( section )
        {
            case HasAttachment: return _attachmentIcon();
            default: return columnTitles_[section];
        }

    } else if( role == Qt::TextAlignmentRole ) {

        switch( section )
        {
            case Color:
            case Creation:
            case Modification:
            case Author:
            return Qt::AlignCenter;

            default: return QVariant();

        }

    } else if( role == Qt::SizeHintRole ) {

        switch( section )
        {
            case HasAttachment:
            case Color:
            return QSize( iconSize_, iconSize_ );

            default:
            return QVariant();

        }

    }

    // return empty
    return QVariant();

}

//______________________________________________________________________
QStringList LogEntryModel::mimeTypes( void ) const
{
    QStringList types;
    types << LogEntry::MimeType << "text/plain";
    return types;
}

//______________________________________________________________________
QMimeData* LogEntryModel::mimeData(const QModelIndexList &indexes) const
{

    Q_ASSERT( !indexes.empty() );

    // create mime data
    QMimeData *mime = new QMimeData();

    // set drag type
    mime->setData( LogEntry::MimeType, 0 );

    // retrieve associated entry
    QString buffer;
    QTextStream what( &buffer );
    foreach( const QModelIndex& index, indexes )
    {
        if( !( index.isValid() && index.column() == Title ) ) continue;
        LogEntry* entry( get( index ) );
        what << entry->keyword() << "/" << entry->title() << endl;
    }

    // set plain text data
    mime->setData( "text/plain", qPrintable( buffer ) );

    return mime;

}



//____________________________________________________________
void LogEntryModel::_sort( int column, Qt::SortOrder order )
{
    Debug::Throw() << "LogEntryModel::sort - column: " << column << " order: " << order << endl;
    std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );
}

//________________________________________________________
void LogEntryModel::_updateConfiguration( void )
{
    Debug::Throw( "LogEntryModel::_updateConfiguration.\n" );
    iconSize_ =XmlOptions::get().get<unsigned int>( "LIST_ICON_SIZE" );
    _resetIcons();
}

//________________________________________________________
void LogEntryModel::_resetIcons( void )
{

    Debug::Throw( "LogEntryModel::_resetIcons" );
    _icons().clear();

}

//________________________________________________________
const QIcon& LogEntryModel::_icon( const Base::Color& color ) const
{

    IconCache::iterator iter( _icons().find( color.name() ) );
    if( iter != _icons().end() ) return iter.value();

    const double pixmapSize = 0.75*std::min<double>( 8, XmlOptions::get().get<double>( "LIST_ICON_SIZE" ) );
    const double offset = 0.5*( iconSize_ - pixmapSize );

    CustomPixmap pixmap( CustomPixmap( QSize( iconSize_, iconSize_ ), CustomPixmap::AllFlags ) );

    if( color.isValid() )
    {

        QPainter painter( &pixmap );
        painter.setRenderHints(QPainter::Antialiasing );
        painter.setPen( Qt::NoPen );

        QRectF rect( QPointF( offset, offset ), QSizeF( pixmapSize, pixmapSize ) );
        painter.setBrush( color );
        painter.drawEllipse( rect );
        painter.end();

    }

    return _icons().insert( color, QIcon( pixmap ) ).value();

}

//________________________________________________________
const QIcon& LogEntryModel::_attachmentIcon( void ) const
{

    Debug::Throw( "LogEntryModel::_attachmentIcon" );

    static QIcon attachmentIcon;
    if( attachmentIcon.isNull() ) attachmentIcon = IconEngine::get( IconNames::Attach );
    return attachmentIcon;

}

//________________________________________________________
bool LogEntryModel::SortFTor::operator () ( LogEntry* first, LogEntry* second ) const
{

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        case Keyword: return first->keyword() < second->keyword();
        case Title: return first->title() < second->title();
        case Creation: return first->creation() < second->creation();
        case Modification: return first->modification() < second->modification();
        case Author: return first->author() < second->author();
        case Color: return  first->color() < second->color();

        default: return true;

    }

}
