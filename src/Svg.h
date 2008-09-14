
#ifndef Svg_h
#define Svg_h

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
  \file    Svg.h
  \brief   Icon filenames
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <string>
#include <QString>

/*
  BaseSvg is not necessary here
  but it is included to avoid conflicts 
  between icons wrapping names
*/

//! namespace for icon static name wrappers
namespace SVG
{
  
  static const QString TOP_LEFT="topleft";
  static const QString TOP="top";
  static const QString TOP_RIGHT="topright";
  static const QString RIGHT="right";
  static const QString BOTTOM_RIGHT="bottomright";
  static const QString BOTTOM="bottom";
  static const QString BOTTOM_LEFT="bottomleft";
  static const QString LEFT="left";
  static const QString CENTER="center";
        
};

#endif
