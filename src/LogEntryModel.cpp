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

/*!
  \file    LogEntryModel.h
  \brief   Job model. Stores job information for display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QApplication>
#include <QIcon>

#include "CustomPixmap.h"
#include "LogEntryModel.h"
#include "LogEntry.h"
#include "TimeStamp.h"
#include "XmlOptions.h"

using namespace std;

//_______________________________________________
LogEntryModel::IconCache LogEntryModel::icons_;

//_______________________________________________
const char* LogEntryModel::column_titles_[ LogEntryModel::n_columns ] =
{ 
  "title",
  "creation",
  "modification",
  "author",
  "color"
};

//_______________________________________________________________
LogEntryModel::LogEntryModel( QObject* parent ):
  ListModel<LogEntry*>( parent )
{ Debug::Throw( "LogEntryModel::LogEntryModel.\n" ); }

//__________________________________________________________________
QVariant LogEntryModel::data( const QModelIndex& index, int role ) const
{
  Debug::Throw( "LogEntryModel::data.\n" );
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  LogEntry* entry( get()[index.row()] );
  
  // return text associated to file and column
  if( role == Qt::DisplayRole ) {
    
    switch( index.column() )
    {

      case TITLE: return entry->title().c_str();
      case CREATION: return entry->creation().string().c_str();
      case MODIFICATION: return entry->modification().string().c_str();
      case AUTHOR: return entry->author().c_str();
      case COLOR: return entry->color().c_str();
      
      default:
      return QVariant();
    }
  }
 
  return QVariant();
  
}

//__________________________________________________________________
QVariant LogEntryModel::headerData(int section, Qt::Orientation orientation, int role) const
{

  if( 
    orientation == Qt::Horizontal && 
    role == Qt::DisplayRole && 
    section >= 0 && 
    section < n_columns )
  { return QString( column_titles_[section] ); }
  
  // return empty
  return QVariant(); 

}

//____________________________________________________________
void LogEntryModel::sort( int column, Qt::SortOrder order )
{ 
  Debug::Throw() << "LogEntryModel::sort - column: " << column << " order: " << order << endl;

  // base class implementation
  ItemModel::sort( column, order );

  // prepare modifications
  emit layoutAboutToBeChanged();
  std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );
  emit layoutChanged();
      
}

//________________________________________________________
bool LogEntryModel::SortFTor::operator () ( LogEntry* first, LogEntry* second ) const
{
  
  if( order_ == Qt::AscendingOrder ) swap( first, second );
  
  switch( type_ )
  {

    // check if column is a TimeStamp
    case TITLE: return first->title() < second->title();
    case CREATION: return first->creation() < second->creation();
    case MODIFICATION: return first->modification() < second->modification();
    case AUTHOR: return first->author() < second->author();
    case COLOR: return  first->color() < second->color();

    default:
    throw runtime_error( DESCRIPTION( "invalid column" ) );
    return true;
    
  }
 
}
