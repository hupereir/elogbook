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

#include <QMenuBar>
#include <string>

#include "Debug.h"
#include "Counter.h"



class OpenPreviousMenu;
class SelectionFrame;

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
  Menu( QWidget* parent, SelectionFrame* frame );
  
  //! destructor
  ~Menu( void ) 
  { Debug::Throw() << "Menu::~Menu.\n"; }

  //! retrieve OpenPreviousMenu
  OpenPreviousMenu& openPreviousMenu( void ) const
  {
    assert( open_previous_menu_, DESCRIPTION( "open_previous_menu_ not initialized.\n" ) );
    return *open_previous_menu_;
  }
  
  signals:
  
  //! close window
  /*! 
  this signal is reinterpreted by the parent to close either the
  the entry edit frame or the whole application
  */
  void closeWindow( void );
  
  //! save
  /*! 
  this signal is reinterpreted by the parent to save
  either the current entry or the full logbook
  */
  void save( void );
  
  //! view html
  /*! 
  this signal is reinterpreted by the parent to view
  either the current entry or the full logbook
  */
  void viewHtml( void );
  
  private slots:
  
  //! get list of editor windows into menu
  void _updateEditorMenu( void );
  
  private:

  //! open previous menu, in which the open files are stored
  OpenPreviousMenu* open_previous_menu_;      
 
  //! editor windows menu
  QMenu* editor_menu_;
 
};

#endif
