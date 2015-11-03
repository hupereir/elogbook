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

class Logbook;

//* handles threads for file auto-save
class FileCheck: public QObject, public Base::Key, public Counter
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    FileCheck( QObject* = nullptr );

    //* register logbook and children
    void registerLogbook( Logbook* );

    //* clear all
    void clear( void );

    //* used to monitor file changes
    class Data
    {
        public:

        //* flag
        enum Flag
        {
            None,
            FileRemoved,
            FileModified
        };

        //* constructor
        Data( QString file = QString(), Flag flag = None, TimeStamp stamp = TimeStamp() ):
            file_( file ),
            flag_( flag ),
            timeStamp_( stamp )
        {}

        //* equal to operator
        bool operator == ( const Data& data ) const
        { return file() == data.file(); }

        //* less than operator
        bool operator < ( const Data& data ) const
        { return file() < data.file(); }

        //* file
        void setFile( const QString& file )
        { file_ = file; }

        //* file
        const QString& file( void ) const
        { return file_; }

        //* flag
        void setFlag( const Flag& flag )
        { flag_ = flag; }

        //* flag
        const Flag& flag( void ) const
        { return flag_; }

        //* timestamp
        void setTimeStamp( const TimeStamp& stamp )
        { timeStamp_ = stamp; }

        //* timestamp
        const TimeStamp& timeStamp( void ) const
        { return timeStamp_; }

        private:

        //* file
        QString file_;

        //* flag
        Flag flag_ = None;

        //* timestamp
        TimeStamp timeStamp_;

    };

    //* logbook information model
    class Model: public ListModel<Data>, public Counter
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
        Model( QObject* parent = nullptr ):
            ListModel<Data>( parent ),
            Counter( "FileCheck::Model" )
        {}

        //*@name methods reimplemented from base class
        //@{

        //* flags
        virtual Qt::ItemFlags flags( const QModelIndex& ) const
        { return Qt::ItemIsEnabled |  Qt::ItemIsSelectable; }

        //* return data
        virtual QVariant data( const QModelIndex&, int ) const;

        //* header data
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
        {
            if( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < nColumns )
            { return columnTitles_[section]; }

            // return empty
            return QVariant();

        }

        //* number of columns for a given index
        virtual int columnCount(const QModelIndex& = QModelIndex()) const
        { return nColumns; }

        //@}

        protected:


        //* used to sort Counters
        class SortFTor: public ItemModel::SortFTor
        {

            public:

            //* constructor
            SortFTor( const int& type, Qt::SortOrder order ):
                ItemModel::SortFTor( type, order )
            {}

            //* prediction
            bool operator() ( Data, Data ) const;

        };

        //* sort
        virtual void _sort( int column, Qt::SortOrder order = Qt::AscendingOrder )
        { std::sort( _get().begin(), _get().end(), SortFTor( column, order ) ); }

        //* list column names
        static const QString columnTitles_[nColumns];

    };

    //* map data to file
    using DataSet = QSet<Data>;

    //* file system watcher
    const QFileSystemWatcher& fileSystemWatcher( void ) const
    { return fileSystemWatcher_; }

    Q_SIGNALS:

    //* files have been modified
    void filesModified( FileCheck::DataSet );

    protected:

    //* timer event, to handle multiple file modification at once
    virtual void timerEvent( QTimerEvent* event );

    private Q_SLOTS:

    //* one monitored file has been modified
    void _fileChanged( const QString& );

    protected:

    //* add file
    void _addFile( const QString& );

    //* remove file
    void _removeFile( const QString&, bool = false );

    //* file system watcher
    QFileSystemWatcher& _fileSystemWatcher( void )
    { return fileSystemWatcher_; }

    //* file system watcher
    QFileSystemWatcher fileSystemWatcher_;

    private:

    //* file set
    using FileSet = QSet<QString>;

    //* file set
    FileSet files_;

    //* map files and modification data
    DataSet data_;

    //* resize timer
    QBasicTimer timer_;

};

inline unsigned int qHash( const FileCheck::Data& data )
{ return qHash( data.file() ); }

#endif
