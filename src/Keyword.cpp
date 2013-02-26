
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

#include "Debug.h"
#include "Keyword.h"

//_________________________________________________________________
const QString Keyword::MimeType( "logbook/keyword-list" );

//_________________________________________________________________
Keyword& Keyword::append( const QString& value )
{
  // check string to append
  if( value.isEmpty() || ( value.size() == 1 && value[0] == '/' ) ) return *this;

  // make sure leading "/" is added
  if( value[0] == '/' ) value_ += value;
  else value_ += QString( "/" ) + value;

  // reformat
  value_ = _format( value_ );

  return *this;

}

//_________________________________________________________________
QString Keyword::current( void ) const
{

  int pos = value_.lastIndexOf( "/" );
  return ( pos < 0 ) ? value_:value_.mid( pos+1 );

}

//_________________________________________________________________
Keyword Keyword::parent( void ) const
{

  int pos = value_.lastIndexOf( "/" );
  Q_ASSERT( pos >= 0 );
  return Keyword( value_.left( pos ) );

}

//_______________________________________________
bool Keyword::isChild( const Keyword& keyword ) const
{ return parent() == keyword; }

//_______________________________________________
bool Keyword::inherits( const Keyword& keyword ) const
{

  if( *this == keyword ) return true;
  if( get().size() < keyword.get().size() ) return false;
  int pos( get().indexOf( keyword.get() ) );
  return pos == 0 && get()[keyword.get().size()]=='/';

}


//_________________________________________________________________
QString Keyword::_format( const QString& value )
{

  // make sure value is not empty
  if( value.isEmpty() ) return QString('/');

  QString out( value );

  // add leading "/"
  if( out[0] != '/' ) out = QString( "/" ) + out;

  // look for "/"
  // replace next character by uppercase
  int pos(0);
  while( ( pos = out.indexOf( "/", pos ) ) >= 0 )
  {
    if( pos+1 < out.length() )
    {
      out[pos+1] = out[pos+1].toUpper();
      pos++;
    } else break;
  }

  // remove trailing "/" if any
  if( out.length() && out[out.length()-1] == '/' )
  { out = out.left(out.length()-1); }

  return out;
}
