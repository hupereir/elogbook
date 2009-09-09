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
\file    HtmlHeaderNode.cpp
\brief  Some Html utilities
\author  Hugo Pereira
\version $Revision$
\date    $Date$
*/

#include "HtmlHeaderNode.h"
#include "Debug.h"

using namespace std;

//_________________________________________
HtmlHeaderNode::HtmlHeaderNode( QDomElement& parent, QDomDocument& document )
{
  Debug::Throw( "HtmlHeaderNode::Header.\n" );
  QDomElement head = parent.appendChild( document.createElement( "head" ) ).toElement();
  QDomElement meta = head.appendChild( document.createElement( "meta" ) ).toElement();
  meta.setAttribute( "content", "text/html; charset=iso-8859-1" );
  meta.setAttribute( "http-equiv", "Content-Type" );

  meta = head.appendChild( document.createElement( "meta" ) ).toElement();
  meta.setAttribute( "content", "eLogbook" );
  meta.setAttribute( "name", "Generator" );

  head.
    appendChild( document.createElement( "title" ) ).
    appendChild( document.createTextNode( "Electronic Logbook" ) );

  QDomElement style = head.appendChild( document.createElement( "style" ) ).toElement();
  style.setAttribute( "type", "text/css" );
  style.appendChild( document.createTextNode(
    "body { \n"
    "  background: white; color: black; \n "
    "  margin-right: 20px; margin-left: 20px; \n"
    "} \n"
    "\n"
    "A:hover { text-decoration: none; background-color: #f2f2ff }\n"
    "\n"
    ".header_outer_table {\n"
    "  width: 100%; \n"
    "  border: 1px solid #868686; \n"
    "  background-color: #f2f2ff; \n"
    "  cellspacing: 0;\n"
    "} \n"
    "\n"
    ".header_inner_table {"
    "  width: 100%; \n"
    "  border: 0; \n"
    "  cellpadding: 0; \n"
    "  cellspacing: 0; \n"
    "} \n"
    "\n"
    ".header_column { padding: 8px 20px; }\n "
    "\n" ) );
}
