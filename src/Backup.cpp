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
    for( int i=0; i<attributes.count(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        if( attribute.name() == Xml::Creation ) setCreation( TimeStamp(attribute.value().toInt()) );
        else if( attribute.name() == Xml::File ) setFile( File( attribute.value() ) );
    }
}

//______________________________________________________________________
QDomElement Backup::domElement( QDomDocument& document ) const
{
    Debug::Throw( "Backup::domElement.\n" );
    QDomElement out( document.createElement( Xml::BackupMask ) );
    out.setAttribute( Xml::Creation, QString::number( creation_.unixTime() ) );
    out.setAttribute( Xml::File, file() );
    return out;
}
