#ifndef SvgEngine_h
#define SvgEngine_h

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
  \file SvgEngine.h
  \brief customized Icon factory to provide better looking disabled icons
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QPixmap>
#include <QSize>
#include <QSvgRenderer>

#include <map>
#include <list>
#include <assert.h>

#include "Counter.h"
#include "PixmapEngine.h"

//! customized Icon factory to provide better looking disabled icons
class SvgEngine: public Counter
{
  
  public:
  
  //! retrieve singleton
  static SvgEngine& get( void )
  { return singleton_; }
  
  //! create pixmap
  /*! the size is stored into a cache to avoid duplicate pixmaps manipulations */
  static QPixmap get( const QSize& size )
  { return get()._get( size ); }
  
  //! is valid
  bool isValid( void ) const
  { return svg_.isValid(); }
  
  //! reload all icons set in cache from new path list
  bool reload( void );
  
  private:
  
  //! load svg
  bool _loadSvg( void );
  
  //! validate svg
  bool _validateSvg( void ) const;
      
  //! singleton
  static SvgEngine singleton_;
    
  //!@name non static methods are hidden
  //@{
  
  //! constructor
  SvgEngine( void );
  
  //! create icon
  /*! the file is stored into a cache to avoid all pixmaps manipulations */
  QPixmap _get( const QSize&, bool from_cache = true );
  
  //@}

  //! ordered QSize subclass
  class Size: public QSize
  {
    
    public:
    
    //! constructor
    Size( const QSize& size ):
      QSize( size )
      {}
      
    // order
    bool operator < ( const Size& size )  const
    { return ( width() < size.width() || ( width() == size.width() && height() < size.height() ) ); }
    
  };

  //! svg file
  std::string svg_file_;
  
  //! svg renderer
  QSvgRenderer svg_;
  
  //! svg offest
  int svg_offset_;

  //! map size and pixmap
  typedef std::map< Size, QPixmap > Cache;

  //! map size and pixmap
  Cache cache_;
  
};

#endif
