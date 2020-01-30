#ifndef FileCheck_h
#define FileCheck_h

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

#include "Key.h"
#include "ListModel.h"
#include "TimeStamp.h"

#include <QBasicTimer>
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimerEvent>
#include <QSet>

#include <array>

class Logbook;

//* handles threads for file auto-save
class FileCheck: public QObject, public Base::Key, private Base::Counter<FileCheck>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit FileCheck( QObject* = nullptr );

    //* register logbook and children
    void registerLogbook( Logbook* );

    //* clear all
    void clear();

    //* used to monitor file changes
    class Data
    {
        public:

        //* flag
        enum class Flag
        {
            None,
            FileRemoved,
            FileModified
        };

        //* constructor
        explicit Data( QString file = QString(), Flag flag = Flag::None, TimeStamp stamp = TimeStamp() ):
            file_( file ),
            flag_( flag ),
            timeStamp_( stamp )
        {}

        //* file
        void setFile( const QString& file )
        { file_ = file; }

        //* file
        const QString& file() const
        { return file_; }

        //* flag
        void setFlag( const Flag& flag )
        { flag_ = flag; }

        //* flag
        const Flag& flag() const
        { return flag_; }

        //* timestamp
        void setTimeStamp( const TimeStamp& stamp )
        { timeStamp_ = stamp; }

        //* timestamp
        const TimeStamp& timeStamp() const
        { return timeStamp_; }

        private:

        //* file
        QString file_;

        //* flag
        Flag flag_ = Flag::None;

        //* timestamp
        TimeStamp timeStamp_;

    };

    //* logbook information model
    class Model: public ListModel<Data>, private Base::Counter<Model>
    {

        public:

        //* column type enumeration
        enum ColumnType {
            File,
            Flag,
            Time,
            nColumns
        };

        //* constructor
        explicit Model( QObject* parent = nullptr ):
            ListModel( parent ),
            Counter( "FileCheck::Model" )
        {}

        //*@name methods reimplemented from base class
        //@{

        //* flags
        Qt::ItemFlags flags( const QModelIndex& ) const override
        { return Qt::ItemIsEnabled |  Qt::ItemIsSelectable; }

        //* return data
        QVariant data( const QModelIndex&, int ) const override;

        //* header data
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
        {
            if( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < nColumns )
            { return columnTitles_[section]; }

            // return empty
            return QVariant();

        }

        //* number of columns for a given index
        int columnCount(const QModelIndex& = QModelIndex()) const override
        { return nColumns; }

        //@}

        protected:


        //* used to sort Counters
        class SortFTor: public ItemModel::SortFTor
        {

            public:

            //* constructor
            explicit SortFTor( int type, Qt::SortOrder order ):
                ItemModel::SortFTor( type, order )
            {}

            //* prediction
            bool operator() ( Data, Data ) const;

        };

        //* sort
        void _sort( int column, Qt::SortOrder order ) override
        { std::sort( _get().begin(), _get().end(), SortFTor( column, order ) ); }

        //* list column names
        const std::array<QString, nColumns> columnTitles_ =
        {{
            tr( "file" ),
            tr( "flag" ),
            tr( "time stamp" )
        }};

    };

    //* map data to file
    using DataSet = QSet<Data>;

    //* file system watcher
    const QFileSystemWatcher& fileSystemWatcher() const
    { return fileSystemWatcher_; }

    Q_SIGNALS:

    //* files have been modified
    void filesModified( FileCheck::DataSet );

    protected:

    //* timer event, to handle multiple file modification at once
    void timerEvent( QTimerEvent* ) override;

    protected:

    //* add file
    void _addFile( const QString& );

    //* remove file
    void _removeFile( const QString&, bool = false );

    //* file system watcher
    QFileSystemWatcher& _fileSystemWatcher()
    { return fileSystemWatcher_; }

    //* file system watcher
    QFileSystemWatcher fileSystemWatcher_;

    private:

    //* one monitored file has been modified
    void _fileChanged( const QString& );

    //* file set
    using FileSet = QSet<QString>;

    //* file set
    FileSet files_;

    //* map files and modification data
    DataSet data_;

    //* resize timer
    QBasicTimer timer_;

};

//* equal to operator
inline bool operator == ( const FileCheck::Data& first, const FileCheck::Data& second)
{ return first.file() == second.file(); }

//* less than operator
inline bool operator < ( const FileCheck::Data& first, const FileCheck::Data& second)
{ return first.file() < second.file(); }

//* hash
inline uint qHash( const FileCheck::Data& data )
{ return qHash( data.file() ); }

#endif
