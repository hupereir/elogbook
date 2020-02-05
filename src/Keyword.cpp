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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Keyword.h"

#include "Debug.h"
#include "XmlDef.h"

#include <QObject>

//_________________________________________________________________
const Keyword Keyword::Default( QObject::tr( "New entries" ) );
const QString Keyword::MimeType( "logbook/keyword-list" );

//_________________________________________________________________
Keyword::Keyword( QString value):
    Counter( QStringLiteral("Keyword") ),
    value_( _format( value ) )
{}

//_________________________________________________________________
Keyword::Keyword( const QDomElement& element ):
    Counter( QStringLiteral("Keyword") ),
    value_( _format( element.text() ) )
{}

//_________________________________________________________________
QDomElement Keyword::domElement( QDomDocument& document ) const
{
    QDomElement out( document.createElement( Xml::Keyword ) );
    out.appendChild( document.createTextNode( value_ ) );
    return out;
}

//_________________________________________________________________
QString Keyword::current() const
{

    int pos = value_.lastIndexOf( '/' );
    return ( pos >= 0 ) ? value_.mid( pos+1 ):value_;

}

//_________________________________________________________________
Keyword Keyword::parent() const
{
    const int pos = value_.lastIndexOf( '/' );
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
Keyword& Keyword::append( QString value )
{

    // check string to append
    if( value.isEmpty() || value == "/" ) return *this;

    // make sure leading "/" is added
    if( value.startsWith( '/' ) || value_.endsWith( '/' ) ) value_ += value;
    else value_ += QString( '/' ) + value;

    // reformat
    value_ = _format( value_ );

    return *this;

}

//_________________________________________________________________
QString Keyword::_format( QString value ) const
{

    // make sure value is not empty
    if( value.isEmpty() ) return QString('/');

    // keep "root" unchanged
    if( value == "/" ) return value;

    QString out( value );

    // add leading "/"
    if( !out.startsWith( '/' ) ) out.prepend( '/' );

    // look for "/"
    // replace next character by uppercase
    for( int position = 0; ( position = out.indexOf( "/", position ) ) >= 0 && position < out.length(); ++position )
    { out[position+1] = out[position+1].toUpper(); }

    // remove trailing "/" if any
    if( out.endsWith( '/' ) )
    { out.truncate(out.length()-1); }

    return out;
}
