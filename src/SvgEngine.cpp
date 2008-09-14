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
  \file SvgEngine.cpp
  \brief customized Icon factory to provide better looking disabled icons
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QPainter>

#include "SvgEngine.h"
#include "Svg.h"
#include "XmlOptions.h"

using namespace std;

//__________________________________________________________
SvgEngine SvgEngine::singleton_;

//__________________________________________________________
SvgEngine::SvgEngine( void ):
  Counter( "SvgEngine" ),
  svg_offset_( 0 )
{ Debug::Throw( "SvgEngine::SvgEngine.\n" ); }

//__________________________________________________________
bool SvgEngine::reload( void )
{ 
  Debug::Throw( "SvgEngine::reload.\n" );

  // try load background file
  bool changed( _loadSvg() );
  if( !isValid() )
  { 
    cache_.clear();
    return true;
  }
  
  // check offset
  int svg_offset( XmlOptions::get().get<int>( "SVG_OFFSET" ) );
  
  // reload cache
  if( changed || svg_offset != svg_offset_ )
  {
    svg_offset_ = svg_offset;
    for( Cache::iterator iter = cache_.begin(); iter != cache_.end(); iter++ )
    { cache_[iter->first] = _get( iter->first, false ); }
    return true;
  }
  
  return false;
}

//__________________________________________________________
QPixmap SvgEngine::_get( const QSize& size, bool from_cache )
{
  Debug::Throw( "SvgEngine::_get.\n" );
  
  // try find file in cache
  if( from_cache )
  {
    Cache::iterator iter( cache_.find( size ) );
    if( iter != cache_.end() ) return iter->second;
  }
  
  // update svg pixmap if needed
  QPixmap svg_pixmap;
  svg_pixmap = QPixmap( size );
  svg_pixmap.fill( Qt::transparent );
  QPainter svg_painter( &svg_pixmap );
  
  QSizeF corner_size( svg_.boundsOnElement( SVG::TOP_LEFT ).size() );    
  double offset( svg_offset_ );
  double width = size.width() + 2.0*(offset - corner_size.width() );
  double height = size.height() + 2.0*( offset - corner_size.height() );
  
  // paint corners
  svg_.render( &svg_painter, SVG::TOP_LEFT, QRectF( QPointF( -offset, -offset ), corner_size ) ); 
  svg_.render( &svg_painter, SVG::TOP, QRectF( QPointF( corner_size.width()-offset, -offset ), QSizeF( width, corner_size.height() ) ) );
  svg_.render( &svg_painter, SVG::TOP_RIGHT, QRectF( QPointF( width + corner_size.width() - offset , -offset ), corner_size ) ); 
  
  svg_.render( &svg_painter, SVG::BOTTOM_LEFT, QRectF( QPointF( -offset, height+corner_size.height() - offset ), corner_size ) ); 
  svg_.render( &svg_painter, SVG::BOTTOM, QRectF( QPointF( corner_size.width() - offset, height+corner_size.height() - offset ), QSizeF( width, corner_size.height() ) ) );
  svg_.render( &svg_painter, SVG::BOTTOM_RIGHT, QRectF( QPointF( width + corner_size.width() - offset, height+corner_size.height() - offset ), corner_size ) ); 
  
  svg_.render( &svg_painter, SVG::LEFT, QRectF( QPointF( -offset, corner_size.height() - offset ), QSizeF( corner_size.width(), height ) ) );
  svg_.render( &svg_painter, SVG::RIGHT, QRectF( QPointF( width + corner_size.width()- offset, corner_size.height()- offset ), QSizeF( corner_size.width(), height ) ) );
  svg_.render( &svg_painter, SVG::CENTER, QRectF( QPointF( corner_size.width() - offset, corner_size.height() - offset ), QSizeF( width, height ) ) );
  
  // add to map
  cache_.insert( make_pair( size, svg_pixmap ) );
  return svg_pixmap;
  
}

//________________________________________________
bool SvgEngine::_loadSvg( void )
{
  Debug::Throw( "SvgEngine::_loadSvg.\n" );
  
  // get options
  bool changed( true );
  bool found( false );
  list<string> file_list( XmlOptions::get().specialOptions<string>( "SVG_BACKGROUND" ) );
  for( list<string>::const_iterator iter = file_list.begin(); iter != file_list.end(); iter++ )
  {
    svg_.load( QString( iter->c_str() ) );
    if( svg_.isValid() && _validateSvg() ) 
    {
      changed = ( svg_file_ != *iter );
      svg_file_ = *iter;
      found = true; 
      break;
    }
  }

  if( !found )  svg_.load( QString() );
  return changed;
}

//________________________________________________
bool SvgEngine::_validateSvg( void ) const
{
  Debug::Throw( "SvgEngine::_validateSvg.\n" );

  if( !svg_.elementExists( SVG::TOP_LEFT ) ) return false;
  if( !svg_.elementExists( SVG::TOP ) ) return false;
  if( !svg_.elementExists( SVG::TOP_RIGHT ) ) return false;
  
  if( !svg_.elementExists( SVG::BOTTOM_LEFT ) ) return false;
  if( !svg_.elementExists( SVG::BOTTOM ) ) return false;
  if( !svg_.elementExists( SVG::BOTTOM_RIGHT ) ) return false;
  
  if( !svg_.elementExists( SVG::LEFT ) ) return false;
  if( !svg_.elementExists( SVG::CENTER ) ) return false;
  if( !svg_.elementExists( SVG::RIGHT ) ) return false;
  return true;
}
