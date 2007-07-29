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
  \file TopWidget.cc
  \brief main widget
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QCloseEvent>

#include "TopWidget.h"
#include "QtUtil.h"
#include "MainFrame.h"

//_______________________________________  
void TopWidget::uniconify( void ) 
{

  Debug::Throw( "TopWidget::uniconify.\n" );
  QtUtil::uniconify( this );
  
  return;
}

//____________________________________
void TopWidget::closeEvent( QCloseEvent *event )
{
  Debug::Throw( "TopWidget::closeEvent.\n" );
  event->accept();    
  static_cast<MainFrame*>(qApp)->exit();
}
