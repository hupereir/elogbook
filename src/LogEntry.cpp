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

#include "LogEntry.h"

#include "Debug.h"
#include "Attachment.h"
#include "ColorMenu.h"
#include "Logbook.h"
#include "TextFormat.h"
#include "XmlColor.h"
#include "XmlDef.h"
#include "XmlTextFormatBlock.h"
#include "XmlTimeStamp.h"

//__________________________________
const QString LogEntry::MimeType = QStringLiteral("logbook/log-entry-list");

//__________________________________
LogEntry::LogEntry():
    Counter( QStringLiteral("LogEntry") ),
    creation_( TimeStamp::now() ),
    modification_( TimeStamp::now() )
{}

//_________________________________________________
LogEntry::LogEntry( const QDomElement& element ):
    Counter( QStringLiteral("LogEntry") ),
    creation_( TimeStamp::now() ),
    modification_( TimeStamp::now() )
{
    // parse attributes
    const auto attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        const auto attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        const auto name( attribute.name() );
        const auto value( attribute.value() );
        if( name == Xml::Title ) setTitle( value );
        else if( name == Xml::Keyword ) addKeyword( Keyword( value ) );
        else if( name == Xml::Author ) setAuthor( value );
        else if( name == Xml::Creation ) setCreation( TimeStamp( static_cast<time_t>(value.toLong()) ) );
        else if( name == Xml::Modification ) setModification( TimeStamp( static_cast<time_t>(value.toLong()) ) );
        else if( name == Xml::Color ) setColor( QColor( value ) );
    }

    // parse children elements
    for( auto&& childNode = element.firstChild(); !childNode.isNull(); childNode = childNode.nextSibling() )
    {
        const auto childElement = childNode.toElement();
        if( childElement.isNull() ) continue;

        const auto tagName( childElement.tagName() );
        if( tagName == Xml::Keyword ) addKeyword( Keyword( childElement ) );
        else if( tagName == Base::Xml::Color ) {

            XmlColor color( childElement );
            if( color.isValid() ) setColor( color );

        } else if( tagName == Xml::Text ) setText( childElement.text() );
        else if( tagName == Xml::Creation ) setCreation( XmlTimeStamp( childElement ) );
        else if( tagName == Xml::Modification ) setModification( XmlTimeStamp( childElement ) );
        else if( tagName == TextFormat::Xml::Tag ) addFormat( TextFormat::XmlBlock( childElement ) );
        else if( tagName == Xml::Attachment ) Base::Key::associate( this, new Attachment( childElement ) );
    }

}

//__________________________________
LogEntry::~LogEntry()
{
    // delete associated attachments
    for( const auto& attachment:Base::KeySet<Attachment>( this ) )
    { delete attachment; }
}

//__________________________________
QDomElement LogEntry::domElement( QDomDocument& document ) const
{
    Debug::Throw( QStringLiteral("LogEntry::domElement.\n") );
    auto out( document.createElement( Xml::Entry ) );

    // title and author
    if( !title_.isEmpty() ) out.setAttribute( Xml::Title, title_ );
    if( !author_.isEmpty() ) out.setAttribute( Xml::Author, author_ );
    if( creation_.isValid() ) out.setAttribute( Xml::Creation, QString::number( creation_.unixTime() ) );
    if( modification_.isValid() ) out.setAttribute( Xml::Modification, QString::number( modification_.unixTime() ) );

    if( color_.isValid() )
    {
        /*
        opaque color is written as attribute.
        translucent as child
        */
        if( color_.get().alpha() == 255 ) out.setAttribute( Xml::Color, color_.get().name() );
        else out.appendChild( XmlColor( color_ ).domElement( document ) );
    }

    // keyword
    if( keywords_.size() == 1 && !keywords_.begin()->get().isEmpty() ) out.setAttribute( Xml::Keyword, keywords_.begin()->get() );
    else {

        for( const auto& keyword:keywords_ )
        { if( !keyword.get().isEmpty() ) out.appendChild( keyword.domElement( document ) ); }

    }

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
        if( !format.isEmpty() && ((format.color().isValid() && format.color() != QPalette().color( QPalette::Text ) ) || format.format() != TextFormat::Default ) )
        { out.appendChild( TextFormat::XmlBlock( format ).domElement( document ) ); }
     }

    // dump attachments
    for( const auto& attachment:Base::KeySet<Attachment>( this ) )
    { out.appendChild( attachment->domElement( document ) ); }

    return out;

}

//__________________________________
LogEntry* LogEntry::copy() const
{
    Debug::Throw( QStringLiteral("LogEntry::copy.\n") );

    auto *out( new LogEntry( *this ) );

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
    Debug::Throw( QStringLiteral("LogEntry::matchAttachment.\n") );

    // retrieve associated attachments
    const auto attachments( Base::KeySet<Attachment>( this ) );
    return std::any_of( attachments.begin(), attachments.end(),
        [this, &buffer]( Attachment* attachment )
        { return attachment->file().contains( buffer, _caseSensitive() ); } );
}

//__________________________________
void LogEntry::setModified()
{ modification_ = TimeStamp::now(); }

//__________________________________
void LogEntry::clearKeywords()
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
void LogEntry::addFormat( TextFormat::Block format )
{
    if( format.isEmpty() ) return;
    if( format.color() == Qt::black ) format.unsetColor();
    if( format.format() == TextFormat::Default && !format.color().isValid() ) return;
    formats_.append(format);
}

//________________________________________________________
Qt::CaseSensitivity LogEntry::_caseSensitive() const
{ return XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ? Qt::CaseSensitive: Qt::CaseInsensitive; }
