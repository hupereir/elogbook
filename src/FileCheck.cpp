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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License        
* for more details.                     
*                          
* You should have received a copy of the GNU General Public License along with 
* software; if not, write to the Free Software Foundation, Inc., 59 Temple     
* Place, Suite 330, Boston, MA 02111-1307 USA                           
*                         
*                         
*******************************************************************************/

/*!
  \file FileCheck.cpp
  \brief keep track of external file modifications
  \author  Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <algorithm>
#include <assert.h>
#include <QStringList>

#include "Debug.h"
#include "File.h"
#include "FileCheck.h"
#include "Logbook.h"

using namespace std;

//____________________________________________________
FileCheck::FileCheck( QObject* parent ):
  QObject( parent ),
  Counter( "FileCheck" )
{ 
  Debug::Throw( "FileCheck::FileCheck.\n" );
  connect( &_fileSystemWatcher(), SIGNAL( fileChanged( const QString& ) ), SLOT( _fileChanged( const QString& ) ) );
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
  Logbook::List children( logbook->children() );
  for( Logbook::List::const_iterator iter = children.begin(); iter != children.end(); iter++ )
  {
    if( !(*iter)->file().isEmpty() )
    {
      
      // associate (make sure association is unique)
      if( !isAssociated( *iter ) ) 
      { BASE::Key::associate( this, *iter ); }
      
      _addFile( (*iter)->file() );
      
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
void FileCheck::printMonitoredFiles( void )
{
  
  Debug::Throw( 0, "FileCheck::printMonitoredFiles.\n" );
  QStringList files( _fileSystemWatcher().files());  
  
  for( QStringList::const_iterator iter = files.begin(); iter != files.end(); iter++ )
  { Debug::Throw(0) << "FileCheck::printMonitoredFiles - " << *iter << endl; }
  
  Debug::Throw(0) << endl;

}

//______________________________________________________
void FileCheck::timerEvent( QTimerEvent* event )
{
  if( event->timerId() == timer_.timerId() )
  {
    
    Debug::Throw(0, "FileCheck::timerEvent.\n" );
    
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
  files_.erase( file );
  _fileSystemWatcher().removePath( file );
  return;
  
}

//______________________________________________________
void FileCheck::_fileChanged( const QString& file )
{
    
  // filecheck data
  Data data;
  
  // find associated display with matching file
  BASE::KeySet<Logbook> logbooks( this );
  BASE::KeySet<Logbook>::iterator iter( find_if( logbooks.begin(), logbooks.end(), Logbook::SameFileFTor( file ) ) );
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
      data_.insert( make_pair( file, data ) ); 
      timer_.start( 200, this );
    }
      
  } else { 
    
    // remove file from list otherwise
    _removeFile( file, true );
    
  }
      
}
