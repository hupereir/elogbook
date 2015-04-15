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

#include "Debug.h"
#include "Attachment.h"
#include "ColorMenu.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "TextFormat.h"
#include "XmlColor.h"
#include "XmlDef.h"
#include "XmlTextFormatBlock.h"
#include "XmlTimeStamp.h"
#include "XmlString.h"

//__________________________________
const QString LogEntry::MimeType = "logbook/log-entry-list";

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
    for( int i=0; i<attributes.count(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        QString name( attribute.name() );
        QString value( attribute.value() );
        if( name == Xml::Title ) setTitle( XmlString( value ).toText() );
        else if( name == Xml::Keyword ) setKeyword( Keyword( XmlString( value ).toText() ) );
        else if( name == Xml::Author ) setAuthor( XmlString( value ).toText() );

        // kept for backward compatibility
        else if( name == Xml::Color ) setColor( QColor( value ) );
        else Debug::Throw(0) << "LogEntry::LogEntry - unrecognized entry attribute: \"" << name << "\"\n";
    }

    // parse children elements
    for(QDomNode childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        QDomElement childElement = childNode.toElement();
        if( childElement.isNull() ) continue;

        QString tagName( childElement.tagName() );
        if( tagName == Base::Xml::Color )
        {

            XmlColor color( childElement );
            if( color.isValid() ) setColor( color );

        } else if( tagName == Xml::Text ) setText( XmlString( childElement.text() ).toText() );
        else if( tagName == Xml::Creation ) setCreation( XmlTimeStamp( childElement ) );
        else if( tagName == Xml::Modification ) setModification( XmlTimeStamp( childElement ) );
        else if( tagName == Format::Xml::Tag ) addFormat( Format::XmlTextFormatBlock( childElement ) );
        else if( tagName == Xml::Attachment ) Key::associate( this, new Attachment( childElement ) );
        else Debug::Throw(0) << "LogEntry::LogEntry - unrecognized child " << childElement.tagName() << endl;
    }

}

//__________________________________
LogEntry::~LogEntry( void )
{
    Debug::Throw( "LogEntry::~LogEntry.\n" );

    // delete associated attachments
    foreach( Attachment* attachment, Base::KeySet<Attachment>( this ) )
    { delete attachment; }

}

//__________________________________
QDomElement LogEntry::domElement( QDomDocument& parent ) const
{
    Debug::Throw( "LogEntry::domElement.\n" );
    QDomElement out( parent.createElement( Xml::Entry ) );
    if( !title().isEmpty() ) out.setAttribute( Xml::Title, title() );
    if( !keyword().get().isEmpty() ) out.setAttribute( Xml::Keyword, keyword().get() );
    if( !author().isEmpty() ) out.setAttribute( Xml::Author, author() );

    // color
    if( color_.isValid() ) out.appendChild( XmlColor( color_ ).domElement( parent ) );

    // dump timeStamp
    if( creation().isValid() ) out.appendChild( XmlTimeStamp( creation() ).domElement( Xml::Creation, parent ) );
    if( modification().isValid() ) out.appendChild( XmlTimeStamp( modification() ).domElement( Xml::Modification, parent ) );

    // dump text
    if( !text_.isEmpty() )
    {
        QString text( text_ );
        if( !text.endsWith('\n') ) text+='\n';

        out.
            appendChild( parent.createElement( Xml::Text ) ).
            appendChild( parent.createTextNode( text ) );
    }

    // dump text format
    foreach( const Format::TextFormatBlock& format, formats() )
    {
        if( !format.isEmpty() && ((format.color().isValid() && format.color() != QPalette().color( QPalette::Text ) ) || format.format() != Format::Default ) )
        { out.appendChild( Format::XmlTextFormatBlock( format ).domElement( parent ) ); }
     }

    // dump attachments
    foreach( Attachment* attachment, Base::KeySet<Attachment>( this ) )
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
    foreach( Attachment* attachment, Base::KeySet<Attachment>( this ) )
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
{ return text_.contains( buffer, _caseSensitive() ); }

//__________________________________
bool LogEntry::matchColor( const QString& buffer ) const
{
    if( !color_.isValid() && !QColor( buffer ).isValid() ) return true;
    else if( !color_.isValid() ) return false;
    else return QColor( buffer ) == color_;
}

//__________________________________
bool LogEntry::matchAttachment( const QString& buffer ) const
{
    Debug::Throw( "LogEntry::matchAttachment.\n" );

    // retrieve associated attachments
    foreach( Attachment* attachment, Base::KeySet<Attachment>( this ) )
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
    title_.clear();
    keyword_.clear();
    author_.clear();
    text_.clear();
    color_ = Base::Color();
}

//________________________________________________________
Qt::CaseSensitivity LogEntry::_caseSensitive( void ) const
{ return XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ? Qt::CaseSensitive: Qt::CaseInsensitive; }
