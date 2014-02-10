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

#include "Attachment.h"
#include "Command.h"
#include "Debug.h"
#include "File.h"
#include "LogEntry.h"
#include "XmlDef.h"
#include "XmlOptions.h"
#include "XmlString.h"
#include "XmlTimeStamp.h"

#include <cstdio>
#include <cstring>

//_______________________________________
const QString Attachment::NoFile;
const QString Attachment::NoComments( QObject::tr( "no comments" ) );
const QString Attachment::NoSize( " - " );

//_______________________________________
Attachment::Attachment( const QString orig, const AttachmentType& type ):
    Counter( "Attachment" ),
    type_( AttachmentType::Unknown ),
    source_file_( orig ),
    file_( NoFile ),
    comments_( NoComments ),
    size_( 0 ),
    sizeString_( NoSize ),
    isLink_( Unknown ),
    valid_( false )
{
    Debug::Throw( "Attachment::Attachment.\n" );
    setType( type );
}

//_______________________________________
Attachment::Attachment( const QDomElement& element):
    Counter( "Attachment" ),
    type_( AttachmentType::Unknown ),
    source_file_( NoFile ),
    file_( NoFile ),
    comments_( NoComments ),
    size_( 0 ),
    sizeString_( NoSize ),
    isLink_( Unknown ),
    valid_( false )
{
    Debug::Throw() << "Attachment::Attachment.\n";

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( unsigned int i=0; i<attributes.length(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        QString name( attribute.name() );
        QString value( attribute.value() );

        if( name == Xml::SourceFile ) _setSourceFile( XmlString( value ).toText() );
        else if( name == Xml::Type ) setType( AttachmentType::get( value ) );
        else if( name == Xml::File ) _setFile( XmlString( value ).toText() );
        else if( name == Xml::Comments ) setComments( XmlString( value ).toText() );
        else if( name == Xml::Valid ) setIsValid( (bool) value.toInt() );
        else if( name == Xml::IsLink ) setIsLink( (LinkState) value.toInt() );
        else Debug::Throw(0) << "unrecognized attachment attribute: \"" << name << "\"\n";
    }

    // parse children elements
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        QString tag_name( child_element.tagName() );
        if( tag_name == Xml::Comments ) setComments( XmlString( child_element.text() ).toText() );
        else if( tag_name == Xml::Creation ) _setCreation( XmlTimeStamp( child_element ) );
        else if( tag_name == Xml::Modification ) _setModification( XmlTimeStamp( child_element ) );
        else Debug::Throw(0) << "Attachment::Attachment - unrecognized child " << child_element.tagName() << ".\n";
    }

    // by default all URL attachments are valid, provided that the SOURCE_FILE is not empty
    if( type() == AttachmentType::Url && !sourceFile().isEmpty() )
    { setIsValid( true ); }

}

//____________________________________________________
QDomElement Attachment::domElement( QDomDocument& parent ) const
{

    Debug::Throw( "Attachment::DomElement.\n" );
    QDomElement out( parent.createElement( Xml::Attachment ) );
    if( file().size() ) out.setAttribute( Xml::File, XmlString( file() ).toXml() );
    if( sourceFile().size() ) out.setAttribute( Xml::SourceFile, XmlString( sourceFile() ).toXml() );
    out.setAttribute( Xml::Type, type().key() );
    out.setAttribute( Xml::Valid, QString::number( (int) isValid() ) );
    out.setAttribute( Xml::IsLink, QString::number( (int) isLink() ) );
    if( comments().size())
    {
        out.
            appendChild( parent.createElement(  Xml::Comments ) ).
            appendChild( parent.createTextNode( XmlString( comments() ).toXml() ) );
    }

    // dump timeStamp
    if( creation().isValid() ) out.appendChild( XmlTimeStamp( creation() ).domElement( Xml::Creation, parent ) );
    if( modification().isValid() ) out.appendChild( XmlTimeStamp( modification() ).domElement( Xml::Modification, parent ) );

    return out;

}

//_______________________________________
bool Attachment::setType( const AttachmentType& type )
{

    Debug::Throw() << "Attachment::setType.\n";
    if( type_ == type ) return false;
    type_  = type;

    if( Attachment::type() == AttachmentType::Url )
    { setIsLink( Yes ); }

    return true;
}

//___________________________________
bool Attachment::operator < ( const Attachment &attachment ) const
{ return shortFile().compare( attachment.shortFile(), XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ? Qt::CaseSensitive:Qt::CaseInsensitive ) < 0; }

//__________________________________
void Attachment::updateSize( void )
{

    Debug::Throw( "Attachment::updateSize.\n" );

    // check type
    if( type() == AttachmentType::Url || size() != 0 || !isValid() ) return;
    size_ = file().fileSize();
    sizeString_ = file().sizeString();

}


//__________________________________
bool Attachment::updateTimeStamps( void )
{

    Debug::Throw( "Attachment::updateTimeStamps.\n" );
    bool changed( false );
    if( type() == AttachmentType::Url ) {
        if( !creation().isValid() ) changed |= _setCreation( TimeStamp::now() );
        changed |= _setModification( TimeStamp() );
    } else {

        if( file().exists() )
        {
            if( !creation().isValid() ) changed |= _setCreation( file().created() );
            changed |= _setModification( file().lastModified() );
        } else {
            changed |= _setCreation( TimeStamp() );
            changed |= _setModification( TimeStamp() );
        }

    }

    return changed;

}


//__________________________________
LogEntry* Attachment::entry( void ) const
{
    Base::KeySet<LogEntry> entries( this );
    Q_ASSERT( entries.size() == 1 );
    return *entries.begin();
}

//__________________________________
Attachment::ErrorCode Attachment::copy( const Command& command, const QString& destdir )
{
    Debug::Throw() << "Attachment::ProcessCopy.\n";

    // check original file
    if( source_file_.isEmpty() )
    {
        Debug::Throw(0) << "Attachment::ProcessCopy - orig not set. Canceled.\n";
        return SourceNotFound;
    }

    // for URL attachments, just copy origin to file, whatever the command
    if( type() == AttachmentType::Url )
    {
        _setFile( source_file_ );
        _setCreation( TimeStamp::now() );
        _setModification( TimeStamp() );
        setIsValid( true );
        return Success;
    }

    // check destination directory
    if( destdir.isEmpty() )
    {
        Debug::Throw(0) << "Attachment::ProcessCopy - destdir not set. Canceled.\n";
        return DestNotFound;
    }

    // generate expanded source name
    File fullname( source_file_ .expand() );
    if( !( type() == AttachmentType::Url || fullname.exists() ) ) return SourceNotFound;
    else if( !( type() == AttachmentType::Url ) && fullname.isDirectory() ) return SourceIsDir;

    // destination filename
    File destname( fullname.localName().addPath( destdir ).expand() );
    source_file_ = fullname;

    // for other process command
    ::Command command_string;
    switch (command) {

        case Copy:
        if( destname.exists() ) return DestExist;
        else {
            command_string << "cp" << source_file_ << destname;
            setIsLink( No );
            break;
        }

        case Link:
        if( destname.exists() ) return DestExist;
        else {
            command_string << "ln" << "-s" << source_file_ << destname;
            setIsLink( Yes );
            break;
        }

        case ForceCopy:
        destname.remove();
        command_string << "cp" << source_file_ << destname;
        setIsLink( No );
        break;

        case ForceLink:
        destname.remove();
        command_string << "ln" << "-s" << source_file_ << destname;
        setIsLink( Yes );
        break;

        case CopyVersion:
        if( destname.exists() && destname.diff( source_file_ ) )
        { destname = destname.version(); }
        command_string << "cp" << source_file_ << destname;
        setIsLink( No );
        break;

        case LinkVersion:
        if( destname.exists() && destname.diff( source_file_ ) )
        { destname = destname.version(); }
        command_string << "ln" << "-s" << source_file_ << destname;
        setIsLink( Yes );
        break;

        case Nothing:
        default:
        break;

    }

    // process copy
    command_string.run();

    // update long/short filenames.
    _setFile( destname );
    if( file().exists() )
    {
        _setCreation( file().created() );
        _setModification( file().lastModified() );
    }

    setIsValid( true );
    return Success;
}

//________________________________________
File Attachment::shortFile( void ) const
{
    Debug::Throw( "Attachment::shortFile.\n" );

    File file( file_ );

    // remove trailing slash
    if( file.size() && file[file.size()-1] == '/' ) { file = File( file.left( file.size()-1 ) ); }

    return file.localName();
}

//_______________________________________
void Attachment::_setFile( const File& file )
{
    Debug::Throw() << "Attachment::_SetFile.\n";

    // store file
    file_  = file;

    // remove trailing spaces
    static QRegExp regexp( "\\s+$" );
    if( regexp.indexIn( file_ ) >= 0 )
    { file_ = file_.left( file_.size() - regexp.matchedLength() ); }

    return;
}
