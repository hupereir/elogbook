#ifndef LogEntryModel_h
#define LogEntryModel_h

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

#include "Counter.h"
#include "ListModel.h"

#include <QtCore/QHash>

class LogEntry;

//! Job model. Stores job information for display in lists
class LogEntryModel : public ListModel<LogEntry*>, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! used to tag Keyword drags
    static const QString DRAG;

    //! constructor
    LogEntryModel(QObject *parent = 0);

    //! destructor
    virtual ~LogEntryModel()
    {}

    //! number of columns
    enum { nColumns = 7 };

    //! column type enumeration
    enum ColumnType {
        COLOR,
        KEYWORD,
        TITLE,
        ATTACHMENT,
        CREATION,
        MODIFICATION,
        AUTHOR
    };

    //!@name methods reimplemented from base class
    //@{

    //! flags
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    //! return data
    virtual QVariant data(const QModelIndex &index, int role) const;

    // modify data
    virtual bool setData(const QModelIndex &index, const QVariant& value, int role = Qt::EditRole );

    //! header data
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    //! mime type
    virtual QStringList mimeTypes( void ) const;

    //! mime data
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;

    //! number of columns for a given index
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
    { return nColumns; }

    //@}

    //!@name edition
    //@{

    //! enable edition
    const bool& editionEnabled( void ) const
    { return editionEnabled_; }

    //! enable edition
    void setEditionEnabled( const bool& value )
    { editionEnabled_ = value; }

    //! edition index
    const QModelIndex& editionIndex( void ) const
    { return editionIndex_; }

    //! edition index
    void setEditionIndex( const QModelIndex& index )
    {
        editionEnabled_ = false;
        editionIndex_ = index;
    }

    //@}


    protected:

    //! sort
    virtual void _sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

    private slots:

    //! update configuration
    void _updateConfiguration( void );

    //! used to disable edition when model is changed while editing
    void _disableEdition( void )
    { setEditionEnabled( false ); }

    private:

    //! color icon cache
    typedef QHash<QString, QIcon> IconCache;

    //! color icon cache
    static IconCache& _icons( void );

    //! reset icons
    void _resetIcons( void );

    //! create icon for a given color
    static QIcon _icon( const QColor& );

    //! attachment icon
    static QIcon& _attachmentIcon( void );

    //! used to sort IconCaches
    class SortFTor: public ItemModel::SortFTor
    {

        public:

        //! constructor
        SortFTor( const int& type, Qt::SortOrder order = Qt::AscendingOrder ):
            ItemModel::SortFTor( type, order )
        {}

        //! prediction
        bool operator() ( LogEntry*, LogEntry* ) const;

    };

    //! edition flag
    bool editionEnabled_;

    //! edition index
    /*! needs to be stored to start delayed edition */
    QModelIndex editionIndex_;

    //! list column names
    static const QString columnTitles_[nColumns];

};

#endif
