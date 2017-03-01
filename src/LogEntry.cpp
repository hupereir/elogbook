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
        if( name == Xml::Title ) setTitle( XmlString( value ) );
        else if( name == Xml::Keyword ) addKeyword( Keyword( XmlString( value ) ) );
        else if( name == Xml::Author ) setAuthor( XmlString( value ) );

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
        if( tagName == Xml::Keyword ) addKeyword( Keyword( childElement ) );
        else if( tagName == Base::Xml::Color ) {

            XmlColor color( childElement );
            if( color.isValid() ) setColor( color );

        } else if( tagName == Xml::Text ) setText( XmlString( childElement.text() ) );
        else if( tagName == Xml::Creation ) setCreation( XmlTimeStamp( childElement ) );
        else if( tagName == Xml::Modification ) setModification( XmlTimeStamp( childElement ) );
        else if( tagName == Format::Xml::Tag ) addFormat( Format::XmlTextFormatBlock( childElement ) );
        else if( tagName == Xml::Attachment ) Base::Key::associate( this, new Attachment( childElement ) );
        else Debug::Throw(0) << "LogEntry::LogEntry - unrecognized child " << childElement.tagName() << endl;
    }

}

//__________________________________
LogEntry::~LogEntry( void )
{

    // delete associated attachments
    for( const auto& attachment:Base::KeySet<Attachment>( this ) )
    { delete attachment; }

}

//__________________________________
QDomElement LogEntry::domElement( QDomDocument& document ) const
{
    Debug::Throw( "LogEntry::domElement.\n" );
    QDomElement out( document.createElement( Xml::Entry ) );

    // title and author
    if( !title_.isEmpty() ) out.setAttribute( Xml::Title, title_ );
    if( !author_.isEmpty() ) out.setAttribute( Xml::Author, author_ );

    // keyword
    if( keywords_.size() == 1 && !keywords_.begin()->get().isEmpty() ) out.setAttribute( Xml::Keyword, keywords_.begin()->get() );
    else {

        for( const auto& keyword:keywords_ )
        { if( !keyword.get().isEmpty() ) out.appendChild( keyword.domElement( document ) ); }

    }

    // color
    if( color_.isValid() ) out.appendChild( XmlColor( color_ ).domElement( document ) );

    // dump timeStamp
    if( creation_.isValid() ) out.appendChild( XmlTimeStamp( creation_ ).domElement( Xml::Creation, document ) );
    if( modification_.isValid() ) out.appendChild( XmlTimeStamp( modification_ ).domElement( Xml::Modification, document ) );


    // dump text
    if( !text_.isEmpty() )
    {
        QString text( text_ );
        if( !text.endsWith('\n') ) text+='\n';

        out.
            appendChild( document.createElement( Xml::Text ) ).
            appendChild( document.createTextNode( text ) );
    }

    // dump text format
    for( const auto& format:formats_ )
    {
        if( !format.isEmpty() && ((format.color().isValid() && format.color() != QPalette().color( QPalette::Text ) ) || format.format() != Format::Default ) )
        { out.appendChild( Format::XmlTextFormatBlock( format ).domElement( document ) ); }
     }

    // dump attachments
    for( const auto& attachment:Base::KeySet<Attachment>( this ) )
    { out.appendChild( attachment->domElement( document ) ); }

    return out;

}

//__________________________________
LogEntry* LogEntry::copy( void ) const
{
    Debug::Throw( "LogEntry::copy.\n" );

    LogEntry *out( new LogEntry( *this ) );

    // clear associations
    out->clearAssociations();

    // copy all Attachments
    for( const auto& attachment:Base::KeySet<Attachment>( this ) )
    {

        // copy attachment, associate to entry
        Attachment *copy( new Attachment( *attachment ) );
        copy->clearAssociations();
        Base::Key::associate( copy, out );

    }

    return out;
}

//__________________________________
bool LogEntry::matchTitle( QString buffer ) const
{ return title_.contains( buffer, _caseSensitive() ); }

//__________________________________
bool LogEntry::matchKeyword( QString buffer ) const
{
    for( const auto& keyword:keywords_ )
    { if( keyword.get().contains( buffer, _caseSensitive() ) ) return true; }

    return false;
}

//__________________________________
bool LogEntry::matchText(  QString buffer ) const
{ return text_.contains( buffer, _caseSensitive() ); }

//__________________________________
bool LogEntry::matchColor( QString buffer ) const
{
    if( !color_.isValid() && !QColor( buffer ).isValid() ) return true;
    else if( !color_.isValid() ) return false;
    else return QColor( buffer ) == color_;
}

//__________________________________
bool LogEntry::matchAttachment( QString buffer ) const
{
    Debug::Throw( "LogEntry::matchAttachment.\n" );

    // retrieve associated attachments
    for( const auto& attachment:Base::KeySet<Attachment>( this ) )
    {
        if( attachment->file().contains( buffer, _caseSensitive() ) )
        { return true; }
    }

    return false;
}

//__________________________________
void LogEntry::setModified( void )
{ modification_ = TimeStamp::now(); }

//__________________________________
void LogEntry::clearKeywords( void )
{ keywords_.clear(); }

//__________________________________
void LogEntry::addKeyword( Keyword keyword )
{
    if( !keywords_.contains( keyword ) && !keyword.get().isEmpty() )
    { keywords_.insert( keyword ); }
}

//__________________________________
void LogEntry::replaceKeyword( Keyword oldKeyword, Keyword newKeyword )
{
    if( keywords_.contains( oldKeyword ) )
    {
        keywords_.remove( oldKeyword );
        if( !newKeyword.get().isEmpty() )
        { keywords_.insert( newKeyword ); }

    } else {

        Debug::Throw(0) << "LogEntry::replaceKeyword - unable to find old keyword " << oldKeyword.get() << endl;
    }

}

//__________________________________
void LogEntry::removeKeyword( Keyword keyword )
{ keywords_.remove( keyword ); }

//__________________________________
void LogEntry::_init( void )
{
    findSelected_ = true;
    keywordSelected_ = false;
    creation_ = TimeStamp::now();
    modification_ = TimeStamp::now();
    title_.clear();
    keywords_.clear();
    author_.clear();
    text_.clear();
    color_ = Base::Color();
}

//________________________________________________________
Qt::CaseSensitivity LogEntry::_caseSensitive( void ) const
{ return XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ? Qt::CaseSensitive: Qt::CaseInsensitive; }
