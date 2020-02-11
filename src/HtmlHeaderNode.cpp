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

#include "HtmlHeaderNode.h"
#include "Debug.h"

//_________________________________________
HtmlHeaderNode::HtmlHeaderNode( QDomElement& parent, QDomDocument& document )
{
    Debug::Throw( QStringLiteral("HtmlHeaderNode::Header.\n") );
    QDomElement head = parent.appendChild( document.createElement( QStringLiteral("head") ) ).toElement();
    QDomElement meta = head.appendChild( document.createElement( QStringLiteral("meta") ) ).toElement();
    meta.setAttribute( QStringLiteral("content"), QStringLiteral("text/html; charset=utf8") );
    meta.setAttribute( QStringLiteral("http-equiv"), QStringLiteral("Content-Type") );

    meta = head.appendChild( document.createElement( QStringLiteral("meta") ) ).toElement();
    meta.setAttribute( QStringLiteral("content"), QStringLiteral("eLogbook") );
    meta.setAttribute( QStringLiteral("name"), QStringLiteral("Generator") );

    head.
        appendChild( document.createElement( QStringLiteral("title") ) ).
        appendChild( document.createTextNode( QStringLiteral("Electronic Logbook") ) );

    QDomElement style = head.appendChild( document.createElement( QStringLiteral("style") ) ).toElement();
    style.setAttribute( QStringLiteral("type"), QStringLiteral("text/css") );
    style.appendChild( document.createTextNode(
        "body { \n"
        "  background: white; color: black; \n "
        "  margin-right: 20px; margin-left: 20px; \n"
        "} \n"
        "\n"
        ".header_outer_table {\n"
        "  width: 100%; \n"
        "  border: 1px solid #888888; \n"
        "  background-color: white; \n"
        "  cellspacing: 0;\n"
        "} \n"
        "\n"
        ".header_inner_table {"
        "  border: 0; \n"
        "  cellpadding: 0; \n"
        "  cellspacing: 0; \n"
        "} \n"
        "\n"
        ".header_column { padding: 8px 20px; }\n "
        "\n" ) );
}
