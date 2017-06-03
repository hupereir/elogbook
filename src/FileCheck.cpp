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

#include "Debug.h"
#include "File.h"
#include "FileCheck.h"
#include "Logbook.h"

#include <QStringList>
#include <algorithm>

//____________________________________________________
FileCheck::FileCheck( QObject* parent ):
    QObject( parent ),
    Counter( "FileCheck" )
{
    Debug::Throw( "FileCheck::FileCheck.\n" );
    connect( &_fileSystemWatcher(), SIGNAL(fileChanged(QString)), SLOT(_fileChanged(QString)) );
}

//______________________________________________________
void FileCheck::registerLogbook( Logbook* logbook )
{

    Debug::Throw( "FileCheck::clear.\n" );

    // associate logbook to this and add corresponding file
    if( !logbook->file().isEmpty() )
    {

        // associate (make sure association is unique)
        if( !isAssociated( logbook ) )
        { Base::Key::associate( this, logbook ); }

        _addFile( logbook->file() );
    }

    // loop over children and register
    for( const auto& iter:logbook->children() )
    {
        if( !iter->file().isEmpty() )
        {

            // associate (make sure association is unique)
            if( !isAssociated( iter ) )
            { Base::Key::associate( this, iter ); }

            _addFile( iter->file() );

        }
    }

}

//______________________________________________________
void FileCheck::clear( void )
{

    Debug::Throw( "FileCheck::clear.\n" );

    // clear associated logbooks
    clearAssociations<Logbook>();

    // clear internal list of monitored files
    files_.clear();

    // clear file system watcher
    if( !_fileSystemWatcher().files().isEmpty() )
    { _fileSystemWatcher().removePaths( _fileSystemWatcher().files() ); }

}

//______________________________________________________
void FileCheck::timerEvent( QTimerEvent* event )
{
    if( event->timerId() == timer_.timerId() )
    {

        // stop timer
        timer_.stop();

        // emit signal
        if( !data_.empty() )
        {
            emit filesModified( data_ );
            data_.clear();
        }

    } else return QObject::timerEvent( event );


}

//______________________________________________________
void FileCheck::_addFile( const QString& file )
{

    Debug::Throw() << "FileCheck::addFile: " << file << endl;
    if( files_.find( file ) == files_.end() )
    {
        files_.insert( file );
        _fileSystemWatcher().addPath( file );
    }

}

//______________________________________________________
void FileCheck::_removeFile( const QString& file, bool forced )
{

    Debug::Throw() << "FileCheck::removeFile: " << file << endl;
    files_.remove( file );
    _fileSystemWatcher().removePath( file );
    return;

}

//______________________________________________________
void FileCheck::_fileChanged( const QString& file )
{

    // filecheck data
    Data data( file );

    // find associated display with matching file
    Base::KeySet<Logbook> logbooks( this );
    auto iter( std::find_if( logbooks.begin(), logbooks.end(), Logbook::SameFileFTor( file ) ) );
    if( iter != logbooks.end() )
    {

        File local( file );
        if( !local.exists() )
        {

            data.setFlag( Data::FileRemoved );

        } else {

            data.setFlag( Data::FileModified );
            data.setTimeStamp( local.lastModified() );

        }

        if( data.flag() == Data::FileRemoved || ((*iter)->saved().isValid() && (*iter)->saved() < data.timeStamp()) )
        {
            data_.insert( data );
            timer_.start( 200, this );
        }

    } else {

        // remove file from list otherwise
        _removeFile( file, true );

    }

}

//_______________________________________________________________________________________
QVariant FileCheck::Model::data( const QModelIndex& index, int role ) const
{

    // check index
    if( !contains( index ) ) return QVariant();

    // retrieve associated file info
    const Data& data( get()[index.row()] );

    // return text associated to file and column
    if( role == Qt::DisplayRole )
    {

        switch( index.column() )
        {

            case File: return data.file();
            case Flag:
            {
                switch( data.flag() )
                {
                    case Data::FileModified: return tr( "Modified" );
                    case Data::FileRemoved: return tr( "Removed" );
                    case Data::None:
                    default: return tr( "None" );
                }
            }

            case Time: return data.timeStamp().toString();

            default: return QVariant();
        }
    }

    return QVariant();

}

//________________________________________________________
bool FileCheck::Model::SortFTor::operator () ( FileCheck::Data first, FileCheck::Data second ) const
{

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        default:
        case File: return first.file() < second.file();
        case Flag: return first.flag() < second.flag();
        case Time: return (first.timeStamp() != second.timeStamp() ) ? (first.timeStamp() < second.timeStamp()):first.file() < second.file();

    }

}
