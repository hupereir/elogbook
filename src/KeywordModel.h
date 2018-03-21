#ifndef KeywordModel_h
#define KeywordModel_h

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

#include <array>

//* Job model. Stores job information for display in lists
class KeywordModel : public TreeModel<Keyword>, private Base::Counter<KeywordModel>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit KeywordModel( QObject* = nullptr );

    //* column type enumeration
    enum { nColumns = 1 };

    //*@name accessors
    //@{

    //* flags
    Qt::ItemFlags flags( const QModelIndex& ) const override;

    //* return data
    QVariant data( const QModelIndex&, int ) const override;

    //* header data
    QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const override;

    //* number of columns for a given index
    int columnCount( const QModelIndex& = QModelIndex() ) const override
    { return nColumns; }

    //* mime type
    QStringList mimeTypes() const override;

    //* mime data
    QMimeData* mimeData( const QModelIndexList& ) const override;

    //@}

    //*@name modifiers
    //@{

    // modify data
    bool setData( const QModelIndex&, const QVariant&, int = Qt::EditRole ) override;

    //* drop mine data
    bool dropMimeData(const QMimeData*, Qt::DropAction, int, int, const QModelIndex&) override;

    //@}

    Q_SIGNALS:

    //* emitted when a logEntryList drag is accepted. Sends the new keyword
    void entryKeywordChangeRequest( Keyword );

    //* emitted when a keyword drag is accepted. Sends the new keyword
    void keywordChangeRequest( Keyword, Keyword );

    //* emitted when a keyword is changed directly in editor
    void keywordChanged( Keyword, Keyword );

    protected:

    //* sort
    void _sort( int, Qt::SortOrder ) override;

    private Q_SLOTS:

    //* keyword changed
    void _requestEntryKeywordChanged()
    { emit entryKeywordChangeRequest( keywordChangedData_.newKeyword() ); }

    //* keyword changed
    void _requestKeywordChanged()
    { emit keywordChangeRequest( keywordChangedData_.oldKeyword(), keywordChangedData_.newKeyword() ); }

    private:

    //* list column names
    const std::array<QString, nColumns> columnTitles_ = {{ tr( "Keywords" ) }};

    //* used to sort Jobs
    class SortFTor
    {

        public:

        //* constructor
        explicit SortFTor( Qt::SortOrder order ):
            order_( order )
            {}

        //* prediction
        bool operator() ( const Item& first, const Item& second ) const
        { return (*this)( first.get(), second.get() ); }

        //* prediction
        bool operator() ( Keyword first, Keyword second ) const;

        private:

        //* order
        Qt::SortOrder order_ = Qt::AscendingOrder;

    };

    //* data structure used for drag and drop
    class KeywordChangedData
    {
        public:

        //*@name modifiers
        //@{

        //* set keywords
        void set( Keyword oldKeyword, Keyword newKeyword )
        {
            oldKeyword_ = oldKeyword;
            newKeyword_ = newKeyword;
        }

        //* set keywords
        void set( Keyword newKeyword )
        {
            oldKeyword_ = Keyword();
            newKeyword_ = newKeyword;
        }

        //@}

        //*@name accessors
        //@{

        Keyword oldKeyword() const { return oldKeyword_; }
        Keyword newKeyword() const { return newKeyword_; }

        //@}

        private:

        Keyword oldKeyword_;
        Keyword newKeyword_;

    };

    //* keyword changed data
    KeywordChangedData keywordChangedData_;

};

#endif
