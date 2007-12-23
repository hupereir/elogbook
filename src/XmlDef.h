#ifndef XmlDef_h
#define XmlDef_h

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
   \file XmlDef.h
   \brief some Xml definitions
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <string>

//! Some Xml definitions
namespace XML {

  //! logbook tag
  static const std::string LOGBOOK( "Logbook" );

  //! attachment tag
  static const std::string ATTACHMENT( "Attachment" );
      
  //! entry tag
  static const std::string ENTRY( "Entry" );  
  
  //! logbook child tag
  static const std::string CHILD( "Child" );

  //! attachment type
  static const std::string TYPE( "type" );
  
  //! attachment destination file
  static const std::string FILE( "file" );
  
  //! attachment source file
  static const std::string SOURCE_FILE( "orig" );

  //! parent file
  static const std::string PARENT_FILE( "parent_file" );

  //! directory
  static const std::string DIRECTORY( "directory" );
  
  //! comments
  static const std::string COMMENTS( "comments" );
  
  //! creation time
  static const std::string CREATION( "Creation" );
  
  //! modification time
  static const std::string MODIFICATION( "Modification" );
  
  //! backup time
  static const std::string BACKUP( "Backup" );
  
  //! title 
  static const std::string TITLE( "title" );
  
  //! logbook author
  static const std::string AUTHOR( "author" );
  
  //! logbook sort method
  static const std::string SORT_METHOD( "sort_method" );
  
  //! logbook sort order
  static const std::string SORT_ORDER( "sort_order" );
  
  //! logbook number of entries
  static const std::string ENTRIES( "entries" );
  
  //! logbook number of children
  static const std::string CHILDREN( "children" );
  
  //! entry text
  static const std::string TEXT( "Text" );
  
  //! entry key
  static const std::string KEYWORD( "key" );
    
  //! entry color
  static const std::string COLOR( "color" );
 
};

#endif
