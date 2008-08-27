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
   \file    AttachmentType.cpp
   \brief   Attached file types for file manipulations
   \author  Hugo Pereira
   \version $Revision$
   \date    $Date$
*/

#include "AttachmentType.h"
#include "XmlOptions.h"

using namespace std;

//___________________________________________________________________________________
AttachmentType AttachmentType::UNKNOWN( "UNKNOWN", "Unknown", "unknown.png", "EDIT_UNKNOWN_ATC" );
AttachmentType AttachmentType::POSTSCRIPT( "POSTSCRIPT", "Postscript", "application-postscript.png", "EDIT_POSTSCRIPT_ATC" );
AttachmentType AttachmentType::IMAGE( "IMAGE", "Image", "image-png.png", "EDIT_IMAGE_ATC" );
AttachmentType AttachmentType::PLAIN_TEXT( "PLAIN_TEXT", "Plain Text", "text-plain.png", "EDIT_PLAIN_TEXT_ATC" );
AttachmentType AttachmentType::HTML( "HTML", "HTML", "text-html.png", "EDIT_HTML_ATC" );
AttachmentType AttachmentType::URL( "URL",  "URL", "text-html.png", "EDIT_URL_ATC" );

std::map< string, AttachmentType > AttachmentType::types_;

//___________________________________________________________________________________
AttachmentType::AttachmentType( const std::string& key, const std::string& name, const std::string& icon, const std::string& option ):
  Counter( "AttachmentType" ),
  key_( key ),
  name_( name ),
  icon_( icon ),
  option_( option )
{}
  
//___________________________________________________________________________________
AttachmentType AttachmentType::get( const string& key )
{
  
  std::map< string, AttachmentType >::const_iterator iter = types().find( key );
  return ( iter == types().end() ) ? UNKNOWN:iter->second;
  
}

//______________________________________
string AttachmentType::editCommand( void ) const
{
  if( !option_.size() ) return "";
  return XmlOptions::get().raw( option_ );
}

//______________________________________
bool AttachmentType::_install( void )
{
  types_.insert( make_pair( "UNKNOWN", UNKNOWN ) );
  types_.insert( make_pair( "POSTSCRIPT", POSTSCRIPT ) );
  types_.insert( make_pair( "IMAGE", IMAGE ) );
  types_.insert( make_pair( "PLAIN_TEXT", PLAIN_TEXT ) );
  types_.insert( make_pair( "HTML", HTML ) );
  types_.insert( make_pair( "URL", URL ) );
  return true;
}
