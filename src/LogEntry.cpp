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
\file LogEntry.cpp
\brief log file entry manipulation object
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include <cassert>
#include <cstdio>

#include "Debug.h"
#include "Attachment.h"
#include "ColorMenu.h"
#include "HtmlTextNode.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "TextFormat.h"
#include "XmlDef.h"
#include "XmlTextFormatBlock.h"
#include "XmlTimeStamp.h"
#include "XmlString.h"

using namespace std;

//__________________________________
const QString LogEntry::DRAG = "logEntry_drag";
const QString LogEntry::UNTITLED = "untitled";
const QString LogEntry::NO_AUTHOR = "anonymous";
const QString LogEntry::NO_TEXT = "";

//__________________________________
LogEntry::LogEntry( void ):
    Counter( "LogEntry" )
{
    Debug::Throw( "LogEntry::LogEntry.\n" );
    _init();
}

//_________________________________________________
LogEntry::LogEntry( const QDomElement& element ):
    Counter( "LogEntry" )
{
    Debug::Throw( "LogEntry::LogEntry [dom].\n" );
    _init();

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( unsigned int i=0; i<attributes.length(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        QString name( attribute.name() );
        QString value( attribute.value() );
        if( name == XML::TITLE ) setTitle( XmlString( value ).toText() );
        else if( name == XML::KEYWORD ) setKeyword( Keyword( XmlString( value ).toText() ) );
        else if( name == XML::AUTHOR ) setAuthor( XmlString( value ).toText() );
        else if( name == XML::COLOR ) setColor( XmlString( value ).toText() );
        else Debug::Throw(0) << "LogEntry::LogEntry - unrecognized entry attribute: \"" << name << "\"\n";
    }

    // parse children elements
    for(QDomNode childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        QDomElement childElement = childNode.toElement();
        if( childElement.isNull() ) continue;

        QString tagName( childElement.tagName() );
        if( tagName == XML::TEXT ) setText( XmlString( childElement.text() ).toText() );
        else if( tagName == XML::CREATION ) setCreation( XmlTimeStamp( childElement ) );
        else if( tagName == XML::MODIFICATION ) setModification( XmlTimeStamp( childElement ) );
        else if( tagName == FORMAT::XmlTextFormatBlock::XML_TAG ) addFormat( FORMAT::XmlTextFormatBlock( childElement ) );
        else if( tagName == XML::ATTACHMENT ) Key::associate( this, new Attachment( childElement ) );
        else Debug::Throw(0) << "LogEntry::LogEntry - unrecognized child " << childElement.tagName() << endl;
    }

}

//__________________________________
LogEntry::~LogEntry( void )
{
    Debug::Throw( "LogEntry::~LogEntry.\n" );

    // delete associated attachments
    BASE::KeySet<Attachment> attachments( this );
    for( BASE::KeySet<Attachment>::iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
    { delete *iter; }

}

//__________________________________
QDomElement LogEntry::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "LogEntry::domElement.\n" );
    QDomElement out( parent.createElement( XML::ENTRY ) );
    if( !title().isEmpty() ) out.setAttribute( XML::TITLE, XmlString( title() ).toXml() );
    if( !keyword().get().isEmpty() ) out.setAttribute( XML::KEYWORD, XmlString( keyword().get() ).toXml() );
    if( !author().isEmpty() ) out.setAttribute( XML::AUTHOR, XmlString( author() ).toXml() );

    // color
    bool color_valid( (!color().isEmpty()) && color().compare( ColorMenu::NONE, Qt::CaseInsensitive ) != 0 && QColor( color() ).isValid() );
    if( color_valid ) out.setAttribute( XML::COLOR, XmlString( color() ).toXml() );

    // dump timeStamp
    if( creation().isValid() ) out.appendChild( XmlTimeStamp( creation() ).domElement( XML::CREATION, parent ) );
    if( modification().isValid() ) out.appendChild( XmlTimeStamp( modification() ).domElement( XML::MODIFICATION, parent ) );

    // dump text
    if( !text().isEmpty() )
    {
        QString text( LogEntry::text() );
        if( text[text.size()-1] != '\n' ) text+='\n';

        out.
            appendChild( parent.createElement( XML::TEXT ) ).
            appendChild( parent.createTextNode( XmlString( text ).toXml() ) );
    }

    // dump text format
    for( FORMAT::TextFormatBlock::List::const_iterator iter = formats().begin(); iter != formats().end(); iter++ )
        if( !iter->isEmpty() ) out.appendChild( FORMAT::XmlTextFormatBlock( *iter ).domElement( parent ) );

    // dump attachments
    BASE::KeySet<Attachment> attachments( this );
    for( BASE::KeySet<Attachment>::iterator iter( attachments.begin() ); iter != attachments.end(); iter++ )
        out.appendChild( (*iter)->domElement( parent ) );

    return out;

}

//__________________________________
LogEntry* LogEntry::clone( void ) const
{
    Debug::Throw( "LogEntry::clone.\n" );

    LogEntry *out( new LogEntry( *this ) );

    // clear associations
    out->clearAssociations();

    // copy all Attachments
    BASE::KeySet<Attachment> attachments( this );
    for( BASE::KeySet<Attachment>::iterator attachment_iter = attachments.begin(); attachment_iter != attachments.end(); attachment_iter++ )
    {

        // copy attachment, associate to entry
        Attachment *attachment( new Attachment( **attachment_iter ) );
        attachment->clearAssociations();
        Key::associate( attachment, out );

    }

    return out;
}

//__________________________________
bool LogEntry::matchTitle( const QString& buffer ) const
{ return title().contains( buffer, _caseSensitive() ); }

//__________________________________
bool LogEntry::matchKeyword( const QString& buffer ) const
{ return keyword().get().contains( buffer, _caseSensitive() ); }

//__________________________________
bool LogEntry::matchText(  const QString& buffer ) const
{ return text().contains( buffer, _caseSensitive() ); }

//__________________________________
bool LogEntry::matchColor( const QString& buffer ) const
{
    if( color_.contains( buffer, Qt::CaseInsensitive ) ) return true;
    if( color_ == ColorMenu::NONE ) return false;
    return QColor( buffer ) == QColor( color_ );
}

//__________________________________
bool LogEntry::matchAttachment( const QString& buffer ) const
{
    Debug::Throw( "LogEntry::matchAttachment.\n" );

    // retrieve associated attachments
    BASE::KeySet<Attachment> attachments( this );
    for( BASE::KeySet<Attachment>::const_iterator iter( attachments.begin() ); iter != attachments.end(); iter++ )
    {
        if( (*iter)->file().contains( buffer, _caseSensitive() ) )
        { return true; }
    }

    return false;
}

//__________________________________
void LogEntry::modified( void )
{
    Debug::Throw( "LogEntry::modified.\n" );
    modification_ = TimeStamp::now();
}

//__________________________________
QDomElement LogEntry::htmlElement( QDomDocument& document, const unsigned int &mask ) const
{
    Debug::Throw( "LogEntry::htmlElement.\n" );

    QDomElement out = document.createElement( "div" );

    // logbook entry header
    if( mask & HTML_HEADER_MASK )
    {

        // surrounding table
        QDomElement table = out.appendChild( document.createElement( "table" ) ).toElement();
        table.setAttribute( "class", "header_outer_table" );
        table.setAttribute( "width", "100%" );
        if( color()!="None" )
        {
            QString buffer;
            QTextStream( &buffer ) << "border: 2px solid" << color();
            table.setAttribute( "style", buffer );
        }

        QDomElement column = table.
            appendChild( document.createElement( "tr" ) ).
            appendChild( document.createElement( "td" ) ).
            toElement();
        column.setAttribute( "class", "header_column" );
        table = column.
            appendChild( document.createElement( "table" ) ).
            toElement();
        table.setAttribute( "class", "header_inner_table" );
        table.setAttribute( "width", "100%" );
        QDomElement row;
        if( !keyword().get().isEmpty() && (mask&HTML_KEYWORD) )
        {
            row = table.appendChild( document.createElement( "tr" ) ).toElement();
            column = row.appendChild( document.createElement( "td" ) ).toElement();
            column.setAttribute( "width", "15%" );
            column.appendChild( document.createTextNode( "Keyword(s): " ) );
            row.
                appendChild( document.createElement( "td" ) ).
                appendChild( document.createElement( "b" ) ).
                appendChild( document.createTextNode( keyword().get() ) );

        }

        if( !title().isEmpty() && (mask&HTML_TITLE) )
        {
            row = table.appendChild( document.createElement( "tr" ) ).toElement();
            column = row.appendChild( document.createElement( "td" ) ).toElement();
            column.setAttribute( "width", "15%" );
            column.appendChild( document.createTextNode( "Title: " ) );
            QDomElement ref = row.
                appendChild( document.createElement( "td" ) ).
                appendChild( document.createElement( "b" ) ).
                appendChild( document.createElement( "a" ) ).
                toElement();
            ref.setAttribute( "name", QString().setNum( creation() ) );
            ref.appendChild( document.createTextNode( title() ) );
        }

        if( creation().isValid() && (mask&HTML_CREATION) )
        {
            row = table.appendChild( document.createElement( "tr" ) ).toElement();
            column = row.appendChild( document.createElement( "td" ) ).toElement();
            column.setAttribute( "width", "15%" );
            column.appendChild( document.createTextNode( "Created: " ) );
            row.
                appendChild( document.createElement( "td" ) ).
                appendChild( document.createElement( "b" ) ).
                appendChild( document.createTextNode( creation().toString() ) );

        }

        if( modification().isValid() && (mask&HTML_MODIFICATION) )
        {
            row = table.appendChild( document.createElement( "tr" ) ).toElement();
            column = row.appendChild( document.createElement( "td" ) ).toElement();
            column.setAttribute( "width", "15%" );
            column.appendChild( document.createTextNode( "Modified: " ) );
            row.
                appendChild( document.createElement( "td" ) ).
                appendChild( document.createElement( "b" ) ).
                appendChild( document.createTextNode( modification().toString() ) );

        }

        if( !author().isEmpty() && (mask&HTML_AUTHOR) )
        {
            row = table.appendChild( document.createElement( "tr" ) ).toElement();
            column = row.appendChild( document.createElement( "td" ) ).toElement();
            column.setAttribute( "width", "15%" );
            column.appendChild( document.createTextNode( "Author: " ) );
            row.
                appendChild( document.createElement( "td" ) ).
                appendChild( document.createElement( "b" ) ).
                appendChild( document.createTextNode( author() ) );

        }

    }

    // write attachments
    BASE::KeySet<Attachment> attachments( this );
    if( !attachments.empty() && ( mask &  HTML_ATTACHMENT ) )
    {
        QDomElement par = out.appendChild( document.createElement("p") ).toElement();
        for( BASE::KeySet<Attachment>::iterator iter( attachments.begin() ); iter != attachments.end(); iter++ )
        { (*iter)->htmlElement( par, document ); }
    }

    // write text
    if( !text().isEmpty() && ( mask & HTML_TEXT ) )
    {
        QDomElement par = out.appendChild( document.createElement("p") ).toElement();
        _htmlTextNode( par, document );
    }

    out.appendChild( document.createElement( "br" ) );
    return out;

}

//__________________________________
QDomElement LogEntry::htmlSummary( QDomDocument& document, const unsigned int& mask ) const
{

    Debug::Throw( "LogEntry::htmlSummary.\n" );

    QDomElement row = document.createElement( "tr" );
    if( mask & HTML_KEYWORD )
    {
        QDomElement ref = ((row.
            appendChild( document.createElement( "td" ) )).
            appendChild( document.createElement( "b" ) )).
            appendChild( document.createElement( "a" ) ).toElement();
        ref.setAttribute( "href", ( QStringList() << "#" << QString().setNum( creation() ) ).join("") );
        ref.appendChild( document.createTextNode( (keyword().get().isEmpty() ) ? Keyword::NO_KEYWORD.get():keyword().get() ) );
    }

    if( mask & HTML_TITLE )
    {
        QDomElement ref = ((row.
            appendChild( document.createElement( "td" ) )).
            appendChild( document.createElement( "b" ) )).
            appendChild( document.createElement( "a" ) ).toElement();
        ref.setAttribute( "href", (QStringList() << "#" << QString().setNum( creation() ) ).join("") );
        ref.appendChild( document.createTextNode( (title().isEmpty()) ? UNTITLED:title() ) );
    }
    if( mask & HTML_CREATION ) row.
        appendChild( document.createElement( "td" )).
        appendChild( document.createTextNode( creation().toString() ) );
    if( mask & HTML_MODIFICATION ) row.
        appendChild( document.createElement( "td" )).
        appendChild( document.createTextNode( modification().toString() ) );
    return row;
}

//__________________________________
void LogEntry::_init( void )
{
    find_selected_ = true;
    keyword_selected_ = false;
    creation_ = TimeStamp::now();
    modification_ = TimeStamp::now();
    title_ = UNTITLED;
    keyword_ = Keyword::NO_KEYWORD;
    author_ = NO_AUTHOR;
    text_ = NO_TEXT;
    color_ = "None";
}

//__________________________________
void LogEntry::_htmlTextNode(
    QDomElement& parent,
    QDomDocument& document ) const
{
    Debug::Throw( "LogEntry::_htmlTextNode.\n" );

    // dump format list
    Debug::Throw() << formats_ << endl;

    // copy format list
    FORMAT::TextFormatBlock::List formats( formats_ );

    // loop over index position
    QString buffer;
    for( int index = 0; index < text().size(); )
    {
        Debug::Throw() << "index: " << index << " position: " << index << endl;
        FORMAT::TextFormatBlock::List::iterator iter( find_if(
            formats.begin(),
            formats.end(),
            FORMAT::TextFormatBlock::ContainsFTor( index ) ) );
        if( iter == formats.end() || iter->isEmpty() )
        {
            QTextStream( &buffer ) << text()[index];
            index ++;

        } else {

            Debug::Throw() << *iter << endl;

            // write previous text
            HtmlTextNode( buffer, parent, document );
            buffer.clear();

            // open new element define format
            QDomElement local_node( parent );
            if( iter->format() & FORMAT::UNDERLINE ) local_node = local_node.appendChild( document.createElement( "u" ) ).toElement();
            if( iter->format() & FORMAT::ITALIC ) local_node = local_node.appendChild( document.createElement( "i" ) ).toElement();
            if( iter->format() & FORMAT::BOLD ) local_node = local_node.appendChild( document.createElement( "b" ) ).toElement();
            if( iter->format() & FORMAT::STRIKE ) local_node = local_node.appendChild( document.createElement( "s" ) ).toElement();
            if( !( iter->color().isEmpty() || iter->color().compare( ColorMenu::NONE, Qt::CaseInsensitive ) == 0 ) )
            {
                local_node = local_node.appendChild( document.createElement( "font" ) ).toElement();
                local_node.setAttribute( "color", iter->color() );
            }

            while( index < text().size() && (int)index < iter->end() )
            {
                QTextStream( &buffer ) << text()[index];
                index++;
            }

            // remove format from list
            formats.erase( iter );

            // process text
            HtmlTextNode( buffer, local_node, document );
            buffer.clear();

        }
    }

    HtmlTextNode( buffer, parent, document );

}

//________________________________________________________
Qt::CaseSensitivity LogEntry::_caseSensitive( void ) const
{ return XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ? Qt::CaseSensitive: Qt::CaseInsensitive; }
