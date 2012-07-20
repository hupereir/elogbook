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
#include "Attachment.h"
#include "ColorMenu.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "TextFormat.h"
#include "XmlDef.h"
#include "XmlTextFormatBlock.h"
#include "XmlTimeStamp.h"
#include "XmlString.h"

#include <cassert>
#include <cstdio>

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
    foreach( Attachment* attachment, BASE::KeySet<Attachment>( this ) )
    { delete attachment; }

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
    foreach( const FORMAT::TextFormatBlock& format, formats() )
    { if( !format.isEmpty() ) out.appendChild( FORMAT::XmlTextFormatBlock( format ).domElement( parent ) ); }

    // dump attachments
    foreach( Attachment* attachment, BASE::KeySet<Attachment>( this ) )
    { out.appendChild( attachment->domElement( parent ) ); }

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
    foreach( Attachment* attachment, BASE::KeySet<Attachment>( this ) )
    {

        // copy attachment, associate to entry
        Attachment *copy( new Attachment( *attachment ) );
        copy->clearAssociations();
        Key::associate( copy, out );

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
    foreach( Attachment* attachment, BASE::KeySet<Attachment>( this ) )
    {
        if( attachment->file().contains( buffer, _caseSensitive() ) )
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
void LogEntry::_init( void )
{
    findSelected_ = true;
    keywordSelected_ = false;
    creation_ = TimeStamp::now();
    modification_ = TimeStamp::now();
    title_ = UNTITLED;
    keyword_ = Keyword::NO_KEYWORD;
    author_ = NO_AUTHOR;
    text_ = NO_TEXT;
    color_ = "None";
}

//________________________________________________________
Qt::CaseSensitivity LogEntry::_caseSensitive( void ) const
{ return XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ? Qt::CaseSensitive: Qt::CaseInsensitive; }
