#ifndef SelectionStatusBar_h
#define SelectionStatusBar_h

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
   \file SelectionStatusBar.h
   \brief customized status bar for selection frame
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QStackedWidget>

#include "ProgressBar.h"
#include "StatusBar.h"

//! customized status bar for selection frame
class SelectionStatusBar: public StatusBar
{
  
  Q_OBJECT
 
  public:
  
  //! constructor
  SelectionStatusBar( QWidget* parent );
  
  //! progress bar
  ProgressBar& progressBar( void ) const
  { 
    assert( progress_ );
    return *progress_; 
  }
  
  //! retrieves label with given index
  virtual StatusBarLabel& label( const unsigned int& i = 0  ) const
  { return (i==0) ? _label():StatusBar::label(i); }

  public slots:
  
  //! show progress bar as visible widget
  void showProgressBar( void );
  
  //! show label as visible widget
  void showLabel( void );
  
  protected:
  
  //! stack widget
  QStackedWidget& _stack( void ) const
  {
    assert( stack_ );
    return *stack_;
  }

  //! label
  StatusBarLabel& _label( void ) const
  {
    assert( label_ );
    return *label_;
  }
  
  private:
  
  //! stack widget
  QStackedWidget* stack_;
  
  //! progress bar
  ProgressBar* progress_;
  
  //! label
  StatusBarLabel* label_;
  
};

#endif
