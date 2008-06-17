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
   \file SelectionStatusBar.cpp
   \brief  customized status bar for selection frame
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include "Debug.h"
#include "SelectionStatusBar.h"

using namespace std;

//______________________________________________________
SelectionStatusBar::SelectionStatusBar( QWidget* parent ):
  StatusBar( parent )
{
  Debug::Throw( "SelectionStatusBar::SelectionStatusBar.\n" );
  
  addPermanentWidget( stack_ = new QStackedWidget( this ), 1 );
  _stack().addWidget( label_ = new StatusBarLabel() );
  _stack().addWidget( progress_ = new ProgressBar() );
  addClock();
  
}

//______________________________________________________
void SelectionStatusBar::showProgressBar( void )
{ 
  // fix progress bar height, because normal display is too high 
  // with respect to neighbor label
  progressBar().setFixedHeight( fontMetrics().lineSpacing() + 6 );
  _stack().setCurrentWidget( &progressBar() ); 
}
  
//______________________________________________________
void SelectionStatusBar::showLabel( void )
{ _stack().setCurrentWidget( &_label() ); }
