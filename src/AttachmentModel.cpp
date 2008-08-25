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
  \file    AttachmentModel.h
  \brief   Attachment information to display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QApplication>
#include <QIcon>
#include <QPainter>

#include "AttachmentModel.h"
#include "Attachment.h"
#include "TimeStamp.h"

using namespace std;

//_______________________________________________
const char* AttachmentModel::column_titles_[ AttachmentModel::n_columns ] =
{ 
  "file",
  "type",
  "size",
  "modification"
};

//_______________________________________________________________
AttachmentModel::AttachmentModel( QObject* parent ):
  ListModel<Attachment*>( parent )
{ Debug::Throw( "AttachmentModel::AttachmentModel.\n" ); }
  
//__________________________________________________________________
Qt::ItemFlags AttachmentModel::flags(const QModelIndex &index) const
{
  
  if (!index.isValid()) return 0;
  
  // default flags
  Qt::ItemFlags out( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  return out;
  
}

//__________________________________________________________________
QVariant AttachmentModel::data( const QModelIndex& index, int role ) const
{
  Debug::Throw( "AttachmentModel::data.\n" );
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  Attachment* attachment( get()[index.row()] );
  
  // return text associated to file and column
  if( role == Qt::DisplayRole ) {
    
    switch( index.column() )
    {

      case FILE: return attachment->shortFile().c_str();
      
      case TYPE: return attachment->type().name().c_str();
      
      case SIZE: return attachment->sizeString().c_str();
      
      case MODIFICATION: 
      return (attachment->modification().isValid() ) ? attachment->modification().string().c_str(): QVariant();
      
      default: return QVariant();
    }
  }
  
  return QVariant();
  
}

//__________________________________________________________________
QVariant AttachmentModel::headerData(int section, Qt::Orientation orientation, int role) const
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
void AttachmentModel::_sort( int column, Qt::SortOrder order )
{ 
  Debug::Throw() << "AttachmentModel::sort - column: " << column << " order: " << order << endl;
  std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );
}

//________________________________________________________
bool AttachmentModel::SortFTor::operator () ( Attachment* first, Attachment* second ) const
{
  
  if( order_ == Qt::AscendingOrder ) swap( first, second );
  
  switch( type_ )
  {

    // check if column is a TimeStamp
    case FILE: return first->file() < second->file();
    case TYPE: return first->type().name() < second->type().name();
    case SIZE: return  first->size() < second->size();
    case MODIFICATION: return first->modification() < second->modification();

    default:
    throw runtime_error( DESCRIPTION( "invalid column" ) );
    return true;
    
  }
 
}
