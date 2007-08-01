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

#ifndef Icons_h
#define Icons_h

/*!
  \file    Icons.h
  \brief   Icon filenames
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <string>

//! namespace for icon static name wrappers
namespace ICONS
{
  
  //! new attachment icon
  static const std::string ATTACH="attach.png";
  
  //! entry deletion icon
  static const std::string DELETE="trashcan_empty.png";
  
  //! keyword/entry edition icon
  static const std::string EDIT="edit.png";
  
  //! raise main window icon
  static const std::string HOME="homeB.png";
  
  //! convert logbook/entry to html icon
  static const std::string HTML="html.png";
  
  //! entry information icon
  static const std::string INFO="info.png";
  
  //! editframe lock icon
  static const std::string LOCK="lock.png";

  //! previous entry icon
  static const std::string PREV="1leftarrow.png";
  
  //! next entry icon
  static const std::string NEXT="1rightarrow.png";

  //! new entry/keyword icon
  static const std::string NEW="filenew.png";

  //! open logbook
  static const std::string OPEN = "fileopen.png";
    
  //! logbook/entry save icon
  static const std::string SAVE="filesave.png";

  //! reload icon
  static const std::string RELOAD = "reload.png";

  //! entry spellCheck icon
  static const std::string SPELLCHECK="fonts.png";
    
};

#endif
