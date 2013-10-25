#ifndef KeywordModel_h
#define KeywordModel_h

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

#include "Counter.h"
#include "Keyword.h"
#include "TreeModel.h"

#include <QMimeData>

//! Job model. Stores job information for display in lists
class KeywordModel : public TreeModel<Keyword>, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! constructor
    KeywordModel( QObject* = 0 );

    //! destructor
    virtual ~KeywordModel()
    {}

    //! number of columns
    enum { nColumns = 1 };

    //! column type enumeration
    enum ColumnType { KEYWORD };

    //!@name methods reimplemented from base class
    //@{

    //! flags
    virtual Qt::ItemFlags flags( const QModelIndex& ) const;

    //! return data
    virtual QVariant data( const QModelIndex&, int ) const;

    // modify data
    virtual bool setData( const QModelIndex&, const QVariant&, int = Qt::EditRole );

    //! header data
    virtual QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const;

    //! number of columns for a given index
    virtual int columnCount( const QModelIndex& = QModelIndex() ) const
    { return nColumns; }

    //! mime type
    virtual QStringList mimeTypes( void ) const;

    //! mime data
    virtual QMimeData* mimeData( const QModelIndexList& ) const;

    //! drop mine data
    virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int, int, const QModelIndex&);

    //@}

    Q_SIGNALS:

    /*! \brief
    emitted when a logEntryList drag is accepted.
    Sends the new keyword
    */
    void entryKeywordChanged( Keyword );

    /*! \brief
    emitted when a Keyword drag is accepted, or when keyword item
    is emitted directly in the list. Sends old keyword, new keyword
    */
    void keywordChanged( Keyword, Keyword );

    protected:

    //! sort
    virtual void _sort( int, Qt::SortOrder = Qt::AscendingOrder );

    private:

    //! list column names
    static const QString columnTitles_[nColumns];

    //! used to sort Jobs
    class SortFTor
    {

        public:

        //! constructor
        SortFTor( Qt::SortOrder order = Qt::AscendingOrder ):
            order_( order )
            {}

        //! prediction
        bool operator() ( const Item& first, const Item& second ) const
        { return (*this)( first.get(), second.get() ); }

        //! prediction
        bool operator() ( Keyword first, Keyword second ) const;

        private:

        //! order
        Qt::SortOrder order_;

    };

};

#endif
