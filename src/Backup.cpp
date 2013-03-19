
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
* ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA   02111-1307 USA
*
*
*******************************************************************************/

#include "Backup.h"
#include "Debug.h"
#include "XmlDef.h"

//______________________________________________________________________
Backup::Backup( const QDomElement& element ):
    Counter( "Backup" ),
    valid_( true )
{
    Debug::Throw( "Backup::Backup.\n" );

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( unsigned int i=0; i<attributes.length(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == XML::CREATION ) setCreation( attribute.value().toUInt() );
        else if( attribute.name() == XML::FILE ) setFile( File( attribute.value() ) );
        else Debug::Throw(0) << "Backup::Backup - unknown attribute name: " << attribute.name() << endl;
    }
}

//______________________________________________________________________
QDomElement Backup::domElement( QDomDocument& document ) const
{
    Debug::Throw( "Backup::domElement.\n" );
    QDomElement out( document.createElement( XML::LOGBOOK_BACKUP ) );
    out.setAttribute( XML::CREATION, QString().setNum( creation() ) );
    out.setAttribute( XML::FILE, file() );
    return out;
}