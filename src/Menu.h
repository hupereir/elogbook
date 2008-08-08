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

#ifndef Menu_h
#define Menu_h

/*!
  \file Menu.h
  \brief  main menu
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <assert.h>
#include <QMenuBar>
#include <string>

#include "Debug.h"
#include "Counter.h"

class MainWindow;

/*!
  \class Menu
  \brief main menu  
*/

class Menu:public QMenuBar, public Counter 
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:  
  
  //! creator
  Menu( QWidget* parent, MainWindow* frame );
  
  //! destructor
  ~Menu( void ) 
  { Debug::Throw() << "Menu::~Menu.\n"; }

  private slots:
  
  //! get list of editor windows into menu
  void _updateEditorMenu( void );
  
  //! update preference menu
  void _updatePreferenceMenu( void );
  
  private:

  //! editor windows menu
  QMenu* editor_menu_;
 
  //! preference menu
  QMenu* preference_menu_;
  
};

#endif
