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
#ifndef _TopWidget_h_
#define _TopWidget_h_
 
/*!
  \file TopWidget.h
  \brief customized QVBox to recieve close event
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <string>

#include "CustomMainWindow.h"
#include "Counter.h"  

/*!
  \class TopWidget
  \brief customized QVBox to recieve close event
*/
class TopWidget: public CustomMainWindow, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:
  
  //! constructor;
  TopWidget( QWidget* parent ):
    CustomMainWindow( parent )
  {} 
  
  public slots:

  //! raise widget
  void uniconify( void );
  
  protected:
  
  //! close window event handler
  void closeEvent( QCloseEvent *event );
  
};

#endif
