#ifndef Keyword_h
#define Keyword_h

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
  \file Keyword.h
  \brief log entry keyword
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <string>
#include <vector>

#include "Counter.h"

//! log entry keyword
class Keyword: public Counter
{
  
  public:
  
  //! used when LogEntry keyword is not defined 
  static const Keyword NO_KEYWORD;  
  
  //! constructor
  Keyword( const std::string& value = std::string("")):
    Counter( "Keyword" )
  { value_ = _format( value ); }
  
  //! equal to operator
  bool operator == (const Keyword& keyword ) const
  { return get() == keyword.get(); }
  
  //! equal to operator
  bool operator != (const Keyword& keyword ) const
  { return !( *this == keyword ); }
  
  //! less than operator
  bool operator < (const Keyword& keyword ) const
  { return get() < keyword.get(); }

  //! set full keyword
  void set( const std::string& value )
  { value_ = _format( value ); }
  
  //! append
  void append( const std::string& value );
  
  //! full keyword
  const std::string& get( void ) const
  { return value_; }
  
  //! current keyword 
  std::string current( void ) const;
  
  //! parent keyword
  Keyword parent( void ) const;
  
  //! true if this keyword is child of argument
  bool isChild( const Keyword& keyword ) const;
  
  //! true if this keyword is parent of argument
  bool isParent( const Keyword& keyword ) const
  { return keyword.isChild( *this ); }
 
  private:

  //! format keyword
  static std::string _format( const std::string& );
  
  //! full value
  std::string value_;  
 
  //! streamer
  friend std::ostream& operator << (std::ostream& out, const Keyword& keyword )
  {
    out << keyword.get();
    return out;
  }
};

#endif
