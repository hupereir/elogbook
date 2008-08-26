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
  static const QString LOGBOOK( "Logbook" );

  //! attachment tag
  static const QString ATTACHMENT( "Attachment" );
      
  //! entry tag
  static const QString ENTRY( "Entry" );  
  
  //! logbook child tag
  static const QString CHILD( "Child" );

  //! attachment type
  static const QString TYPE( "type" );
  
  //! attachment destination file
  static const QString FILE( "file" );
  
  //! attachment source file
  static const QString SOURCE_FILE( "orig" );

  //! parent file
  static const QString PARENT_FILE( "parent_file" );

  //! attachment validity
  static const QString VALID( "valid" );
  
  //! attachement is link
  static const QString IS_LINK( "is_link" );
  
  //! directory
  static const QString DIRECTORY( "directory" );
  
  //! comments
  static const QString COMMENTS( "comments" );
  
  //! creation time
  static const QString CREATION( "Creation" );
  
  //! modification time
  static const QString MODIFICATION( "Modification" );
  
  //! backup time
  static const QString BACKUP( "Backup" );
  
  //! title 
  static const QString TITLE( "title" );
  
  //! logbook author
  static const QString AUTHOR( "author" );
  
  //! logbook sort method
  static const QString SORT_METHOD( "sort_method" );
  
  //! logbook sort order
  static const QString SORT_ORDER( "sort_order" );
  
  //! logbook number of entries
  static const QString ENTRIES( "entries" );
  
  //! logbook number of children
  static const QString CHILDREN( "children" );
  
  //! entry text
  static const QString TEXT( "Text" );
  
  //! entry key
  static const QString KEYWORD( "key" );
    
  //! entry color
  static const QString COLOR( "color" );
 
};

#endif
