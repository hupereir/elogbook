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
  \file    KeywordModel.cpp
  \brief   keyword model
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include "KeywordModel.h"
#include "LogEntryModel.h"

using namespace std;

//______________________________________________________________
const char* KeywordModel::column_titles_[ KeywordModel::n_columns ] = { "keywords" };
const QString KeywordModel::DRAG = "elogbook/keywordmodel/drag";

//______________________________________________________________
KeywordModel::KeywordModel( QObject* parent ):
  TreeModel<Keyword>( parent ),
  Counter( "KeywordModel" )
{}


//__________________________________________________________________
Qt::ItemFlags KeywordModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) return Qt::ItemIsDropEnabled;
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDropEnabled| Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

//__________________________________________________________________
QVariant KeywordModel::data( const QModelIndex& index, int role ) const
{
    
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  if( role != Qt::DisplayRole ) return QVariant();
  if( index.column() != KEYWORD ) return QVariant();
  return QString( _find( index.internalId() ).get().current().c_str() ); 
  
}

//__________________________________________________________________
bool KeywordModel::setData(const QModelIndex &index, const QVariant& value, int role )
{
  Debug::Throw( "KeywordModel::setData.\n" );
  if( !(index.isValid() && index.column() == KEYWORD && role == Qt::EditRole ) ) return false;
  
  // retrieve parent index
  Keyword parent_keyword( get( parent( index ) ) );
  
  Keyword keyword( get( index ) );
  if( value.toString() != keyword.current().c_str() )
  { 
    // generate new keyword from value
    Keyword new_keyword( parent_keyword.append( qPrintable( value.toString() ) ) );
    Debug::Throw() << "KeywordModel::setData - old: " << keyword << " new: " << new_keyword << endl;
    emit keywordChanged( keyword, new_keyword );
    emit dataChanged( index, index ); 
  }

  return true;
  
}

//__________________________________________________________________
QVariant KeywordModel::headerData(int section, Qt::Orientation orientation, int role) const
{

  if( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < n_columns )
  { return QString( column_titles_[section] ); }
  
  // return empty
  return QVariant(); 

}

//______________________________________________________________________
QStringList KeywordModel::mimeTypes( void ) const
{
  QStringList types;
  types << DRAG << "text/plain";
  return types;
}

//______________________________________________________________________
QMimeData* KeywordModel::mimeData(const QModelIndexList &indexes) const
{

  assert( !indexes.empty() );
  
  // create mime data
  QMimeData *mime = new QMimeData();
  
  // set DRAG type
  for( QModelIndexList::const_iterator iter = indexes.begin(); iter != indexes.end(); iter++ )
  { if( iter->isValid() ) mime->setData( DRAG, get( *iter ).get().c_str() ); }
  
  // retrieve associated entry
  ostringstream what;
  for( QModelIndexList::const_iterator iter = indexes.begin(); iter != indexes.end(); iter++ )
  {
    if( !iter->isValid() ) continue;
    what << get(*iter) << endl;
  }
  
  // set plain text data
  mime->setData( "text/plain", what.str().c_str() );
  
  return mime;

}

//__________________________________________________________________
bool KeywordModel::dropMimeData(const QMimeData* data , Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
  
  Debug::Throw( "KeywordModel::dropMimeData\n" );
  
  // check action
  if( action == Qt::IgnoreAction) return true;
  
  // Drag from Keyword model
  if( data->hasFormat( DRAG ) ) 
  {

    // retrieve/check string
    QString keyword_string( data->data( DRAG ) );
    if( keyword_string.isNull() || keyword_string.isEmpty() ) return false;
    
    // retrieve old keyword
    Keyword old_keyword( qPrintable( keyword_string ) );
    
    // retrieve new location
    QModelIndex index = parent.isValid() ? parent : QModelIndex();
    Keyword new_keyword = get( index );
    
    // check that keyword is different
    if( new_keyword == old_keyword ) return false;
    
    // append new keyword to old
    new_keyword.append( old_keyword.current() );
    
    // emit keyword modification signal
    emit keywordChanged( old_keyword, new_keyword );
    return true;
  }
  
  // drag from LogEntryModel
  if( data->hasFormat( LogEntryModel::DRAG ) )
  {
    Debug::Throw( "KeywordModel::dropMimeData - LogEntryModel::DRAG.\n" );
    
    // no drag if parent is invalid
    if( !parent.isValid() ) return false;
    
    // retrieve new location
    Keyword new_keyword = get( parent );
    
    // emit logEntry keyword changed signal
    emit entryKeywordChanged( new_keyword );
    return true;
    
  }
  
  return false;
  
}

//____________________________________________________________
void KeywordModel::_sort( int column, Qt::SortOrder order )
{ 
  Debug::Throw() << "KeywordModel::sort - column: " << column << " order: " << order << endl;
  if( column != KEYWORD ) return;
  _root().sort( SortFTor(order) );
}


//________________________________________________________
bool KeywordModel::SortFTor::operator () ( Keyword first, Keyword second ) const
{
  
  if( order_ == Qt::AscendingOrder ) swap( first, second );
  return first < second;
  
}
