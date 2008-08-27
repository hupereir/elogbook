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
  \brief   LogEntry information to display in lists
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QApplication>
#include <QIcon>
#include <QPainter>

#include "ColorMenu.h"
#include "CustomPixmap.h"
#include "LogEntryModel.h"
#include "LogEntry.h"
#include "TimeStamp.h"
#include "XmlOptions.h"

using namespace std;

//_______________________________________________
LogEntryModel::IconCache LogEntryModel::icons_;

//_______________________________________________
const QString LogEntryModel::DRAG = "elogbook/logentrymodel/drag";
const char* LogEntryModel::column_titles_[ LogEntryModel::n_columns ] =
{ 
  "",
  "title",
  "creation",
  "modification",
  "author"
};

//_______________________________________________________________
LogEntryModel::LogEntryModel( QObject* parent ):
  ListModel<LogEntry*>( parent ),
  edition_enabled_( false )
{ 
  Debug::Throw( "LogEntryModel::LogEntryModel.\n" );
  
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( this, SIGNAL( layoutChanged() ), SLOT( _disableEdition() ) ); 
  _updateConfiguration();
  
}
  
//__________________________________________________________________
Qt::ItemFlags LogEntryModel::flags(const QModelIndex &index) const
{
  
  if (!index.isValid()) return 0;
  
  // default flags
  Qt::ItemFlags out( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
  
  // check against edition index
  if( index == editionIndex() && edition_enabled_ ) 
  { out |= Qt::ItemIsEditable; }
  
  // check column
  if( index.column() == TITLE ) out |= Qt::ItemIsDragEnabled;
  return out;
    
}

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
      
      default:
      return QVariant();
    }
  }

  // return icon associated to file
  if( role == Qt::DecorationRole && index.column() == COLOR ) 
  { 
    
    QColor color( entry->color() != ColorMenu::NONE ? entry->color().c_str():QColor() );
    IconCache::iterator iter( icons_.find( color ) );
    if( iter == icons_.end() ) iter = icons_.insert( make_pair( color, _createIcon( color ) ) ).first;
    return iter->second;
    
  }
 
  // alignment
  if( role == Qt::TextAlignmentRole && index.column() == COLOR ) return Qt::AlignCenter;
  
  return QVariant();
  
}

//__________________________________________________________________
bool LogEntryModel::setData(const QModelIndex &index, const QVariant& value, int role )
{
  Debug::Throw( "LogEntryModel::setData.\n" );
  
  if( !edition_enabled_ )
  {
    Debug::Throw(0, "LogEntryModel::setData - edition is disabled. setData canceled.\n" );
    return false; 
  }
  
  if( !(index.isValid() && index.column() == TITLE && role == Qt::EditRole ) ) 
  {
    Debug::Throw(0, "LogEntryModel::setData - invalid index/role. setData canceled.\n" );
    return false;
  }
  
  LogEntry* entry( get( index ) );
  if( !entry ) 
  {
    Debug::Throw(0, "LogEntryModel::setData - entry is null. setData canceled.\n" );
    return false;
  }
  
  if( value != entry->title().c_str() )
  {
    entry->setTitle( qPrintable( value.toString() ) );
    emit dataChanged( index, index );
  }

  Debug::Throw( "LogEntryModel::setData - done. \n" );
  return true;
  
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

//______________________________________________________________________
QStringList LogEntryModel::mimeTypes( void ) const
{
  QStringList types;
  types << DRAG << "text/plain";
  return types;
}

//______________________________________________________________________
QMimeData* LogEntryModel::mimeData(const QModelIndexList &indexes) const
{

  assert( !indexes.empty() );
  
  // create mime data
  QMimeData *mime = new QMimeData();
  
  // set DRAG type
  mime->setData( DRAG, 0 );
  
  // retrieve associated entry
  ostringstream what;
  for( QModelIndexList::const_iterator iter = indexes.begin(); iter != indexes.end(); iter++ )
  {
    if( !( iter->isValid() && iter->column() == TITLE ) ) continue;
    LogEntry* entry( get( *iter ) );
    what << entry->keyword() << "/" << entry->title() << endl;
  }
  
  // set plain text data
  mime->setData( "text/plain", what.str().c_str() );
  
  return mime;
  
}



//____________________________________________________________
void LogEntryModel::_sort( int column, Qt::SortOrder order )
{ 
  Debug::Throw() << "LogEntryModel::sort - column: " << column << " order: " << order << endl;
  std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) );
}

//________________________________________________________
void LogEntryModel::_updateConfiguration( void ) 
{
  Debug::Throw( "LogEntryModel::_updateConfiguration.\n" );
  _resetIcons();
}

//________________________________________________________
void LogEntryModel::_resetIcons( void ) 
{

  Debug::Throw( "LogEntryModel::_resetIcons" );
  for( IconCache::iterator iter = icons_.begin(); iter != icons_.end(); iter++ )
  { icons_[iter->first] = _createIcon( iter->first ); }
  
}

//________________________________________________________
QIcon LogEntryModel::_createIcon( const QColor& color ) const
{
 
  Debug::Throw( "LogEntryModel::_createIcon" );
    
  unsigned int icon_size = max<unsigned int>( XmlOptions::get().get<unsigned int>( "LIST_ICON_SIZE" ), 16 );
  double pixmap_size = 0.75* XmlOptions::get().get<double>( "LIST_ICON_SIZE" );
  double offset = 0.5*( icon_size - pixmap_size );
  
  CustomPixmap pixmap( CustomPixmap().empty( QSize( icon_size, icon_size ) ) );
  
  if( color.isValid() ) 
  {

    QPainter painter( &pixmap );
    painter.setRenderHints(QPainter::Antialiasing );
    painter.setPen( Qt::NoPen );
    
    QRectF rect( QPointF( offset, offset ), QSizeF( pixmap_size, pixmap_size ) );
    painter.setBrush( color ); 
    painter.drawEllipse( rect );
    painter.end();
 
  }
  
  return QIcon( pixmap );
  
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
