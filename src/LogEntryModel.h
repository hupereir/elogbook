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

#include "Color.h"
#include "Counter.h"
#include "Keyword.h"
#include "ListModel.h"

#include <QMap>

#include <array>

class LogEntry;

//* Job model. Stores job information for display in lists
class LogEntryModel : public ListModel<LogEntry*>, private Base::Counter<LogEntryModel>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit LogEntryModel(QObject* = nullptr );

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
    Qt::ItemFlags flags( const QModelIndex& ) const override;

    //* return data
    QVariant data( const QModelIndex&, int ) const override;

    //* header data
    QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const override;

    //* mime type
    QStringList mimeTypes() const override;

    //* mime data
    QMimeData* mimeData( const QModelIndexList& ) const override;

    //* number of columns for a given index
    int columnCount( const QModelIndex& = QModelIndex() ) const override
    { return nColumns; }

    //* enable edition
    bool editionEnabled() const
    { return editionEnabled_; }

    //* edition index
    const QModelIndex& editionIndex() const
    { return editionIndex_; }

    //@}

    //*@name modifiers
    //@{

    // modify data
    bool setData( const QModelIndex&, const QVariant&, int = Qt::EditRole ) override;

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
    void setCurrentKeyword( const Keyword &value )
    { currentKeyword_ = value; }

    //@}


    protected:

    //* sort
    void _sort( int, Qt::SortOrder ) override;

    private:

    //* update configuration
    void _updateConfiguration();

    //* used to disable edition when model is changed while editing
    void _disableEdition()
    { setEditionEnabled( false ); }

    //* color icon cache
    using IconCache = QMap<Base::Color, QIcon>;

    //* color icon cache
    static IconCache& _icons();

    //* reset icons
    void _resetIcons();

    //* create icon for a given color
    const QIcon& _icon( const Base::Color& ) const;

    //* attachment icon
    const QIcon& _attachmentIcon() const;

    //* used to sort IconCaches
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //* constructor
        explicit SortFTor( int type, Qt::SortOrder order ):
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
    const std::array<QString, nColumns> columnTitles_ =
    {{
        QString(),
        tr( "Subject" ),
        tr( "Keyword" ),
        QString(),
        tr( "Creation" ),
        tr( "Modification" ),
        tr( "Author" )
    }};

};

#endif
