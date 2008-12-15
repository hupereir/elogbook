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

#ifndef AttachmentType_h
#define AttachmentType_h

#include <string>
#include <map>
#include "Counter.h"

/*!
   \file    AttachmentType.h
   \brief   Attached file types for file manipulations
   \author  Hugo Pereira
   \version $Revision$
   \date    $Date$
*/

/*!
   \class   AttachmentType
   \brief   Attached file types for file manipulations
*/
class AttachmentType: public Counter{
 
  public:   

  //! unknown file type  
  static AttachmentType UNKNOWN;    
  
  //! postscript file type 
  static AttachmentType POSTSCRIPT; 
  
  //! image file type
  static AttachmentType IMAGE;      
  
  //! plain text file type
  static AttachmentType PLAIN_TEXT; 
  
  //! html file type
  static AttachmentType HTML;       
  
  //! url (pseudo) file type
  static AttachmentType URL;        
 
  /*! 
    retrieves predefined attachment type from key
    returns atcUNKNOWN if key is not found
  */
  static AttachmentType get( const std::string& key );
    
  //! equal to operator
  bool operator == (const AttachmentType& type ) const
  { return key_ == type.key_; }

  //! creator
  AttachmentType( const std::string& key, const std::string& name, const std::string& icon, const std::string& option = ""  );
    
  //! retrieves attachment key
  const std::string& key( void ) const 
  { return key_; }
  
  //! retrieves attachment name
  const std::string& name( void ) const 
  { return name_; }
  
  //! icon
  const std::string& icon( void ) const
  { return icon_; }
  
  //! retrieves command used to edit the attachment
  std::string editCommand( void ) const;
   
  //! attachment types map
  typedef std::map< std::string, AttachmentType > Map;

  //! returns predefined attachment types
  static const Map& types( void );
  
  private:
    
  //! install static type map
  static Map _install( void );
  
  //! used to identify the file type
  std::string key_;           
  
  //! used to display the file type in readable format
  std::string name_;          
  
  //! icon
  std::string icon_;
  
  //! option name for looking up command
  std::string option_;   
  
};

#endif
