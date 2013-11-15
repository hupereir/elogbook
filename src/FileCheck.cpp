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

#include "Debug.h"
#include "File.h"
#include "FileCheck.h"
#include "FileCheck.moc"
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
FileCheck::~FileCheck( void )
{ Debug::Throw( "FileCheck::~FileCheck.\n" ); }

//______________________________________________________
void FileCheck::registerLogbook( Logbook* logbook )
{

    Debug::Throw( "FileCheck::clear.\n" );

    // associate logbook to this and add corresponding file
    if( !logbook->file().isEmpty() )
    {

        // associate (make sure association is unique)
        if( !isAssociated( logbook ) )
        { BASE::Key::associate( this, logbook ); }

        _addFile( logbook->file() );
    }

    // loop over children and register
    foreach( Logbook* iter, logbook->children() )
    {
        if( !iter->file().isEmpty() )
        {

            // associate (make sure association is unique)
            if( !isAssociated( iter ) )
            { BASE::Key::associate( this, iter ); }

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
    BASE::KeySet<Logbook> logbooks( this );
    BASE::KeySet<Logbook>::iterator iter( std::find_if( logbooks.begin(), logbooks.end(), Logbook::SameFileFTor( file ) ) );
    if( iter != logbooks.end() )
    {

        File local( file );
        if( !local.exists() )
        {

            data.setFlag( Data::REMOVED );

        } else {

            data.setFlag( Data::MODIFIED );
            data.setTimeStamp( local.lastModified() );

        }

        if( data.flag() == Data::REMOVED || ((*iter)->saved().isValid() && (*iter)->saved() < data.timeStamp()) )
        {
            data_.insert( data );
            timer_.start( 200, this );
        }

    } else {

        // remove file from list otherwise
        _removeFile( file, true );

    }

}


//_______________________________________________
const QString FileCheck::Model::columnTitles_[ FileCheck::Model::nColumns ] =
{
    tr( "file" ),
    tr( "flag" ),
    tr( "time stamp" )
};

//_______________________________________________________________________________________
QVariant FileCheck::Model::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    // retrieve associated file info
    const Data& data( get()[index.row()] );

    // return text associated to file and column
    if( role == Qt::DisplayRole )
    {

        switch( index.column() )
        {

            case FILE: return data.file();
            case FLAG:
            {
                switch( data.flag() )
                {
                    case Data::MODIFIED: return tr( "Modified" );
                    case Data::REMOVED: return tr( "Removed" );
                    case Data::NONE:
                    default: return tr( "None" );
                }
            }

            case TIME: return data.timeStamp().toString();

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
        case FILE: return first.file() < second.file();
        case FLAG: return first.flag() < second.flag();
        case TIME: return (first.timeStamp() != second.timeStamp() ) ? (first.timeStamp() < second.timeStamp()):first.file() < second.file();

    }

}
