#ifndef LogEntryModel_h
#define LogEntryModel_h

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
#include "Color.h"
#include "Keyword.h"
#include "ListModel.h"

#include <QMap>

class LogEntry;

//* Job model. Stores job information for display in lists
class LogEntryModel : public ListModel<LogEntry*>, private Base::Counter<LogEntryModel>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    LogEntryModel(QObject* = nullptr );

    //* column type enumeration
    enum ColumnType
    {
        Color,
        Title,
        Key,
        HasAttachment,
        Creation,
        Modification,
        Author,
        nColumns
    };

    //*@name accessors
    //@{

    //* flags
    virtual Qt::ItemFlags flags( const QModelIndex& ) const;

    //* return data
    virtual QVariant data( const QModelIndex&, int ) const;

    //* header data
    virtual QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const;

    //* mime type
    virtual QStringList mimeTypes( void ) const;

    //* mime data
    virtual QMimeData* mimeData( const QModelIndexList& ) const;

    //* number of columns for a given index
    virtual int columnCount( const QModelIndex& = QModelIndex() ) const
    { return nColumns; }

    //* enable edition
    bool editionEnabled( void ) const
    { return editionEnabled_; }

    //* edition index
    const QModelIndex& editionIndex( void ) const
    { return editionIndex_; }

    //@}

    //*@name modifiers
    //@{

    // modify data
    virtual bool setData( const QModelIndex&, const QVariant&, int = Qt::EditRole );

    //* enable edition
    void setEditionEnabled( bool value )
    { editionEnabled_ = value; }

    //* edition index
    void setEditionIndex( const QModelIndex& index )
    {
        editionEnabled_ = false;
        editionIndex_ = index;
    }

    //* current keyword
    void setCurrentKeyword( Keyword value )
    { currentKeyword_ = value; }

    //@}


    protected:

    //* sort
    virtual void _sort( int, Qt::SortOrder = Qt::AscendingOrder );

    private Q_SLOTS:

    //* update configuration
    void _updateConfiguration( void );

    //* used to disable edition when model is changed while editing
    void _disableEdition( void )
    { setEditionEnabled( false ); }

    private:

    //* color icon cache
    using IconCache = QMap<Base::Color, QIcon>;

    //* color icon cache
    static IconCache& _icons( void );

    //* reset icons
    void _resetIcons( void );

    //* create icon for a given color
    const QIcon& _icon( const Base::Color& ) const;

    //* attachment icon
    const QIcon& _attachmentIcon( void ) const;

    //* used to sort IconCaches
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //* constructor
        SortFTor( const int& type, Qt::SortOrder order = Qt::AscendingOrder ):
            ItemModel::SortFTor( type, order )
        {}

        //* prediction
        bool operator() ( LogEntry*, LogEntry* ) const;

    };

    //* current keyword, in tree mode
    Keyword currentKeyword_;

    //* edition flag
    bool editionEnabled_ = false;

    //* edition index
    /** needs to be stored to start delayed edition */
    QModelIndex editionIndex_;

    //* icon size
    int iconSize_ = 8;

    //* list column names
    static const QString columnTitles_[nColumns];

};

#endif
