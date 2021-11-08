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
#include "Application.h"
#include "Attachment.h"
#include "ColorMenu.h"
#include "CppUtil.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "LogEntry.h"
#include "Pixmap.h"
#include "QtUtil.h"
#include "Singleton.h"
#include "TimeStamp.h"
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

//_______________________________________________________________
LogEntryModel::LogEntryModel( QObject* parent ):
    ListModel( parent ),
    Counter( QStringLiteral("LogEntryModel") )
{
    Debug::Throw( QStringLiteral("LogEntryModel::LogEntryModel.\n") );

    connect( Base::Singleton::get().application<Application>(), &Application::configurationChanged, this, &LogEntryModel::_updateConfiguration );
    connect( this, &QAbstractItemModel::layoutChanged, this, &LogEntryModel::_disableEdition );
    _updateConfiguration();

}

//__________________________________________________________________
Qt::ItemFlags LogEntryModel::flags(const QModelIndex &index) const
{

    if( !contains(index) ) return {};

    // default flags
    Qt::ItemFlags out( Qt::ItemIsEnabled | Qt::ItemIsSelectable );

    // check against edition index
    if( index == editionIndex_ && editionEnabled_ )
    { out |= Qt::ItemIsEditable; }

    // check column
    out |= Qt::ItemIsDragEnabled;
    return out;

}

//__________________________________________________________________
QVariant LogEntryModel::data( const QModelIndex& index, int role ) const
{

    // check index
    if( !contains( index ) ) return QVariant();

    // retrieve associated file info
    LogEntry* entry( get()[index.row()] );

    // return text associated to file and column
    switch( role )
    {
        case Qt::DisplayRole:
        {

            switch( index.column() )
            {

                case Key:
                {

                    auto keywords = entry->keywords();
                    if( !keywords.empty() )
                    {

                        auto keyword( keywords.begin()->get() );
                        if( keyword.size() > 1 && keyword[0] == '/' )
                        { keyword.remove( 0, 1 ); }

                        return keyword;

                    } else return QVariant();
                }

                case Title: return entry->title();
                case Creation: return entry->creation().toString();
                case Modification: return entry->modification().toString();
                case Author: return entry->author();

                default:
                return QVariant();
            }
            break;
        }

        case Qt::DecorationRole:
        {

            switch( index.column() )
            {
                case Color: return _icon( entry->color() );
                case HasAttachment: return Base::KeySet<Attachment>(entry).empty() ? QVariant():_attachmentIcon();
                default: return QVariant();
            }
            break;
        }

        case Qt::TextAlignmentRole:
        {

            switch( index.column() )
            {
                case Color:
                case Creation:
                case Modification:
                case Author:
                return Qt::AlignCenter;

                default: return QVariant();

            }
            break;

        }

        case Qt::SizeHintRole:
        {

            switch( index.column() )
            {

                case HasAttachment:
                case Color:
                return QSize( iconSize_, iconSize_ );

                default:
                return QVariant();

            }

            break;

        }

        default: break;

    }

    return QVariant();

}

//__________________________________________________________________
bool LogEntryModel::setData(const QModelIndex &index, const QVariant& value, int role )
{
    Debug::Throw( QStringLiteral("LogEntryModel::setData.\n") );

    if( !editionEnabled_ ) return false;
    if( !(index.isValid() && ( index.column() == Title || index.column() == Key ) && role == Qt::EditRole ) ) return false;

    auto entry = get( index );
    if( !entry ) return false;

    if( index.column() == Title && value != entry->title() )
    {

        entry->setTitle( value.toString() );
        emit dataChanged( index, index );

    } else if( index.column() == Key ) {

        auto keywords( entry->keywords() );
        if( keywords.empty() )
        {
            entry->addKeyword( Keyword( value.toString() ) );
            emit dataChanged( index, index );

        } else if( keywords.begin()->get() != value ) {

            entry->replaceKeyword( *keywords.begin(), Keyword( value.toString() ) );
            emit dataChanged( index, index );

        }

    }

    return true;

}

//__________________________________________________________________
QVariant LogEntryModel::headerData(int section, Qt::Orientation, int role) const
{

    switch( role )
    {

        case Qt::DisplayRole:
        {
            switch( section )
            {
                case HasAttachment: return _attachmentIcon();
                default: return columnTitles_[section];
            }

            break;

        }

        case Qt::TextAlignmentRole:
        {

            switch( section )
            {
                case Color:
                case Creation:
                case Modification:
                case Author:
                return Qt::AlignCenter;

                default: return QVariant();

            }

            break;

        }

        case Qt::SizeHintRole:
        {

            switch( section )
            {
                case HasAttachment:
                case Color:
                return QSize( iconSize_, iconSize_ );

                default:
                return QVariant();
            }

            break;
        }

        default: break;

    }

    // return empty
    return QVariant();

}

//______________________________________________________________________
QStringList LogEntryModel::mimeTypes() const
{ return { LogEntry::MimeType, "text/plain" }; }

//______________________________________________________________________
QMimeData* LogEntryModel::mimeData(const QModelIndexList &indexes) const
{

    Q_ASSERT( !indexes.empty() );

    // create mime data
    QMimeData *mime = new QMimeData;

    // set drag type
    mime->setData( LogEntry::MimeType, nullptr );

    // retrieve associated entry
    QString buffer;
    QTextStream what( &buffer );
    for( const auto& index:indexes )
    {
        if( !( index.isValid() && index.column() == Title ) ) continue;
        auto entry( get( index ) );
        auto keywords( entry->keywords() );

        if( keywords.empty() ) what << Keyword::Default << "/" << entry->title() << Qt::endl;
        else if( keywords.contains( currentKeyword_ ) ) what << currentKeyword_ << "/" << entry->title() << Qt::endl;
        else what << keywords.begin()->get() << "/" << entry->title() << Qt::endl;

    }

    // set plain text data
    mime->setData( QStringLiteral("text/plain"), qPrintable( buffer ) );

    return mime;

}



//____________________________________________________________
void LogEntryModel::_sort( int column, Qt::SortOrder order )
{
    Debug::Throw() << "LogEntryModel::sort - column: " << column << " order: " << order << Qt::endl;
    std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );
}

//________________________________________________________
void LogEntryModel::_updateConfiguration()
{
    Debug::Throw( QStringLiteral("LogEntryModel::_updateConfiguration.\n") );
    iconSize_ =XmlOptions::get().get<int>( QStringLiteral("LIST_ICON_SIZE") );
    _resetIcons();
}

//________________________________________________________
void LogEntryModel::_resetIcons()
{

    Debug::Throw( QStringLiteral("LogEntryModel::_resetIcons") );
    _icons().clear();

}

//________________________________________________________
const QIcon& LogEntryModel::_icon( const Base::Color& color ) const
{

    const auto iter( _icons().lowerBound( color ) );
    if( iter != _icons().cend() && Base::areEquivalent( color, iter.key() ) ) return iter.value();
    else
    {

        const double pixmapSize = 0.75*std::min<double>( 8, XmlOptions::get().get<double>( QStringLiteral("LIST_ICON_SIZE") ) );
        const double offset = 0.5*( iconSize_ - pixmapSize );

        Pixmap pixmap( QSize( iconSize_, iconSize_ ), Pixmap::Flag::Transparent );

        if( color.isValid() )
        {

            QPainter painter( &pixmap );
            painter.setRenderHints(QPainter::Antialiasing );
            painter.setPen( Qt::NoPen );

            QRectF rect( QPointF( offset, offset ), QSizeF( pixmapSize, pixmapSize ) );
            painter.setBrush( color.get() );
            painter.drawEllipse( rect );
            painter.end();

        }

        return _icons().insert( iter, color, QIcon( pixmap ) ).value();
    }
}

//________________________________________________________
const QIcon& LogEntryModel::_attachmentIcon() const
{

    Debug::Throw( QStringLiteral("LogEntryModel::_attachmentIcon") );

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

        case Key:
        {
            auto firstKeywords( first->keywords() );
            auto secondKeywords( second->keywords() );
            if( firstKeywords.empty() ) return true;
            else if( secondKeywords.empty() ) return false;
            else return *firstKeywords.begin() < *secondKeywords.begin();
        }
        case Title: return first->title() < second->title();
        case Creation: return first->creation() < second->creation();
        case Modification: return first->modification() < second->modification();
        case Author: return first->author() < second->author();
        case Color: return  first->color() < second->color();

        default: return true;

    }

}
