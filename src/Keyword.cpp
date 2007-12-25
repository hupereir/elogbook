
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
  \file Keyword.cpp
  \brief log entry keyword
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <assert.h>
#include "Debug.h"
#include "Keyword.h"

using namespace std;

//_________________________________________________________________
const Keyword Keyword::NO_KEYWORD( "/" );

//_________________________________________________________________
std::string Keyword::current( void ) const
{
  
  size_t pos = value_.rfind( "/" );
  assert( pos != string::npos );
  return value_.substr( pos+1, value_.length() - pos - 1 );
  
}

//_________________________________________________________________
Keyword Keyword::parent( void ) const
{
  
  size_t pos = value_.rfind( "/" );
  assert( pos != string::npos );
  return Keyword( value_.substr( 0, pos ) );
  
}

//_________________________________________________________________
vector<string> Keyword::parse( Keyword keyword )
{
  vector<string>out;
  if( keyword != NO_KEYWORD )
  {
    out = parse( keyword.parent() );
    out.push_back( keyword.current() );
  }
  
  return out;

}

//_________________________________________________________________
string Keyword::_format( const string& value )
{  
  
  // make sure value is not empty
  if( value.empty() ) 
  { return NO_KEYWORD.get(); }
  
  string out( value );
  
  // add leading "/"
  if( out[0] != '/' ) out = string( "/" ) + out;
  
  // look for "/"
  // replace next character by uppercase
  size_t pos(0);
  while( ( pos = out.find( "/", pos ) ) != string::npos )
  {
    if( pos+1 < out.length() ) 
    {
      out[pos+1] = toupper( out[pos+1] );
      pos++;
    } else break;
  }
  
  return out;
}
