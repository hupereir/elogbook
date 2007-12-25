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

using namespace std;

//______________________________________________________________
const char* KeywordModel::column_titles_[ KeywordModel::n_columns ] = { "" };

//______________________________________________________________
KeywordModel::KeywordModel( QObject* parent ):
  TreeModel<Keyword>( parent )
{}


//__________________________________________________________________
QVariant KeywordModel::data( const QModelIndex& index, int role ) const
{
  
  Debug::Throw( "LogEntryModel::data.\n" );
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  if( role != Qt::DisplayRole ) return QVariant();
  if( index.column() != KEYWORD ) return QVariant();
  return QString( _find( index.internalId() ).get().current().c_str() ); 
  
}


//__________________________________________________________________
QVariant KeywordModel::headerData(int section, Qt::Orientation orientation, int role) const
{

  if( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < n_columns )
  { return QString( column_titles_[section] ); }
  
  // return empty
  return QVariant(); 

}
