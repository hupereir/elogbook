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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "AttachmentType.h"
#include "XmlOptions.h"

//___________________________________________________________________________________
AttachmentType AttachmentType::UNKNOWN( "UNKNOWN", QObject::tr( "Unknown" ), "unknown.png", "EDIT_UNKNOWN_ATC" );
AttachmentType AttachmentType::POSTSCRIPT( "POSTSCRIPT", QObject::tr( "Postscript" ), "application-postscript.png", "EDIT_POSTSCRIPT_ATC" );
AttachmentType AttachmentType::IMAGE( "IMAGE", QObject::tr( "Image" ), "image-x-generic.png", "EDIT_IMAGE_ATC" );
AttachmentType AttachmentType::PLAIN_TEXT( "PLAIN_TEXT", QObject::tr( "Plain Text" ), "text-plain.png", "EDIT_PLAIN_TEXT_ATC" );
AttachmentType AttachmentType::HTML( "HTML", QObject::tr( "HTML" ), "text-html.png", "EDIT_HTML_ATC" );
AttachmentType AttachmentType::URL( "URL",  QObject::tr( "URL" ), "text-html.png", "EDIT_URL_ATC" );

//___________________________________________________________________________________
const AttachmentType::Map& AttachmentType::types()
{
    static Map typeMap = _install();
    return typeMap;
};

//___________________________________________________________________________________
AttachmentType::AttachmentType( const QString& key, const QString& name, const QString& icon, const QString& option ):
    Counter( "AttachmentType" ),
    key_( key ),
    name_( name ),
    icon_( icon ),
    option_( option )
{}

//___________________________________________________________________________________
AttachmentType AttachmentType::get( const QString& key )
{

    Map::const_iterator iter = types().find( key );
    return ( iter == types().end() ) ? UNKNOWN:iter.value();

}

//______________________________________
QString AttachmentType::editCommand( void ) const
{
    if( !option_.size() ) return "";
    return XmlOptions::get().raw( option_ );
}

//______________________________________
AttachmentType::Map AttachmentType::_install( void )
{
    Map out;
    out.insert( "UNKNOWN", UNKNOWN );
    out.insert( "POSTSCRIPT", POSTSCRIPT );
    out.insert( "IMAGE", IMAGE );
    out.insert( "PLAIN_TEXT", PLAIN_TEXT );
    out.insert( "HTML", HTML );
    out.insert( "URL", URL );
    return out;
}
