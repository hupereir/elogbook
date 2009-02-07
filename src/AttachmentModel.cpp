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

#include <QIcon>

#include "AttachmentModel.h"
#include "Attachment.h"
#include "CustomPixmap.h"
#include "Singleton.h"
#include "TimeStamp.h"
#include "XmlOptions.h"

using namespace std;

//__________________________________________________________________
AttachmentModel::IconCache& AttachmentModel::_icons()
{
  static IconCache cache;
  return cache;
}

//_______________________________________________
const QString AttachmentModel::column_titles_[ AttachmentModel::n_columns ] =
{ 
  "",
  "File",
  "Type",
  "Size",
  "Creation", 
  "Modification"
};

//_______________________________________________________________
AttachmentModel::AttachmentModel( QObject* parent ):
  ListModel<Attachment*>( parent ),
  Counter( "AttachmentModel" )
{ 
  Debug::Throw( "AttachmentModel::AttachmentModel.\n" ); 
  connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
}
  
//__________________________________________________________________
Qt::ItemFlags AttachmentModel::flags(const QModelIndex &index) const
{
  
  // default flags
  Qt::ItemFlags flags;
  if( index.isValid() )
  {
  
    // check associated record validity
    const Attachment& attachment( *get(index) );
    if( attachment.isValid() ) flags |=  Qt::ItemIsEnabled |  Qt::ItemIsSelectable;
  
  }
  
  return flags;
  
}

//__________________________________________________________________
QVariant AttachmentModel::data( const QModelIndex& index, int role ) const
{
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  Attachment* attachment( get()[index.row()] );
  
  // return text associated to file and column
  if( role == Qt::DisplayRole ) 
  {
    
    switch( index.column() )
    {

      case FILE: return attachment->shortFile();
      
      case TYPE: return attachment->type().name();
      
      case SIZE: return attachment->sizeString();
      
      case CREATION: 
      return (attachment->creation().isValid() ) ? attachment->creation().toString(): QVariant();
     
      case MODIFICATION: 
      return (attachment->modification().isValid() ) ? attachment->modification().toString(): QVariant();
      
      default: return QVariant();
    }
  } else if( role == Qt::ToolTipRole ) return attachment->file();
  else if( role == Qt::DecorationRole && index.column() == ICON ) return _icon( attachment->type().icon() );
  else if( role == Qt::TextAlignmentRole && index.column() == ICON ) return Qt::AlignCenter;

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
void AttachmentModel::_updateConfiguration( void )
{
  Debug::Throw( "AttachmentModel::_updateConfiguration.\n" );
  _icons().clear();
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
    case FILE: return first->shortFile() < second->shortFile();
    case ICON: return first->type().name() < second->type().name();
    case TYPE: return first->type().name() < second->type().name();
    case SIZE: return  first->size() < second->size();
    case CREATION: return first->creation() < second->creation();
    case MODIFICATION: return first->modification() < second->modification();
    default: assert(0);
    
  }
 
}
//________________________________________________________
QIcon AttachmentModel::_icon( QString type )
{

  //Debug::Throw( "SessionFilesModel::_icon.\n" );
   
  IconCache::const_iterator iter( _icons().find( type ) );
  if( iter != _icons().end() ) return iter->second;

  // pixmap size
  unsigned int pixmap_size = XmlOptions::get().get<unsigned int>( "ATTACHMENT_LIST_ICON_SIZE" );
  QSize size( pixmap_size, pixmap_size );
  QSize scale(size*0.9);
 
  QIcon icon;
  CustomPixmap pixmap( CustomPixmap().find( type ) );
  if( !pixmap.isNull() )
  {
    icon = CustomPixmap()
      .empty( size )
      .merge( pixmap.scaled( scale, Qt::KeepAspectRatio, Qt::SmoothTransformation ), CustomPixmap::CENTER );
  }
  
  // store in map and return
  _icons().insert( make_pair( type, icon ) );
  return icon;
   
}
