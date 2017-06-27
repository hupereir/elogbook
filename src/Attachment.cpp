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
Attachment::Attachment( const QString orig ):
    Counter( "Attachment" ),
    sourceFile_( orig )
{ Debug::Throw( "Attachment::Attachment.\n" ); }

//_______________________________________
Attachment::Attachment( const QDomElement& element):
    Counter( "Attachment" )
{
    Debug::Throw() << "Attachment::Attachment.\n";

    // parse attributes
    QDomNamedNodeMap attributes( element.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {
        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        QString name( attribute.name() );
        QString value( attribute.value() );

        if( name == Xml::SourceFile ) _setSourceFile( File( XmlString( value ) ) );
        else if( name == Xml::File ) _setFile( File( XmlString( value ) ) );
        else if( name == Xml::Type ) setIsUrl( XmlString( value ) == "URL" );
        else if( name == Xml::Comments ) setComments( XmlString( value ) );
        else if( name == Xml::Valid ) setIsValid( (bool) value.toInt() );
        else if( name == Xml::IsLink ) setIsLink( (LinkState) value.toInt() );
        else if( name == Xml::IsUrl ) setIsUrl( (bool) value.toInt() );
        else Debug::Throw(0) << "unrecognized attachment attribute: \"" << name << "\"\n";
    }

    // parse children elements
    for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
    {
        QDomElement child_element = child_node.toElement();
        QString tag_name( child_element.tagName() );
        if( tag_name == Xml::Comments ) setComments( XmlString( child_element.text() ) );
        else if( tag_name == Xml::Creation ) _setCreation( XmlTimeStamp( child_element ) );
        else if( tag_name == Xml::Modification ) _setModification( XmlTimeStamp( child_element ) );
        else Debug::Throw(0) << "Attachment::Attachment - unrecognized child " << child_element.tagName() << ".\n";
    }

    // by default all URL attachments are valid, provided that the SOURCE_FILE is not empty
    if( isUrl_ && !sourceFile_.isEmpty() ) setIsValid( true );

}

//____________________________________________________
QDomElement Attachment::domElement( QDomDocument& parent ) const
{

    Debug::Throw( "Attachment::DomElement.\n" );
    QDomElement out( parent.createElement( Xml::Attachment ) );
    if( file_.size() ) out.setAttribute( Xml::File, file_ );
    if( sourceFile_.size() ) out.setAttribute( Xml::SourceFile, sourceFile_ );
    out.setAttribute( Xml::Valid, QString::number( isValid() ) );
    out.setAttribute( Xml::IsLink, QString::number( isLink() ) );
    out.setAttribute( Xml::IsUrl, QString::number( isUrl() ) );
    if( comments().size())
    {
        out.
            appendChild( parent.createElement(  Xml::Comments ) ).
            appendChild( parent.createTextNode( comments() ) );
    }

    // dump timeStamp
    if( creation().isValid() ) out.appendChild( XmlTimeStamp( creation() ).domElement( Xml::Creation, parent ) );
    if( modification().isValid() ) out.appendChild( XmlTimeStamp( modification() ).domElement( Xml::Modification, parent ) );

    return out;

}

//___________________________________
bool Attachment::operator < ( const Attachment &attachment ) const
{ return shortFile().compare( attachment.shortFile(), XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ? Qt::CaseSensitive:Qt::CaseInsensitive ) < 0; }

//__________________________________
void Attachment::updateSize()
{

    Debug::Throw( "Attachment::updateSize.\n" );

    // check type
    if( isUrl_ || size() != 0 || !isValid() ) return;
    size_ = file_.fileSize();
    sizeString_ = file_.sizeString();

}


//__________________________________
bool Attachment::updateTimeStamps()
{

    Debug::Throw( "Attachment::updateTimeStamps.\n" );
    bool changed( false );
    if( isUrl_ )
    {

        if( !creation().isValid() ) changed |= _setCreation( TimeStamp::now() );
        changed |= _setModification( TimeStamp() );

    } else {

        if( file_.exists() )
        {

            if( !creation_.isValid() ) changed |= _setCreation( file_.created() );
            changed |= _setModification( file_.lastModified() );

        } else {

            changed |= _setCreation( TimeStamp() );
            changed |= _setModification( TimeStamp() );

        }

    }

    return changed;

}


//__________________________________
LogEntry* Attachment::entry() const
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
    if( sourceFile_.isEmpty() )
    {
        Debug::Throw(0) << "Attachment::ProcessCopy - orig not set. Canceled.\n";
        return SourceNotFound;
    }

    // for URL attachments, just copy origin to file, whatever the command
    if( isUrl_ )
    {
        _setFile( sourceFile_ );
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
    File fullname( sourceFile_ .expand() );
    if( !( isUrl_ || fullname.exists() ) ) return SourceNotFound;
    else if( !isUrl_ && fullname.isDirectory() ) return SourceIsDir;

    // destination filename
    File destname( fullname.localName().addPath( File( destdir ) ).expand() );
    sourceFile_ = fullname;

    // for other process command
    ::Command command_string;
    switch (command) {

        case Copy:
        if( destname.exists() ) return DestExist;
        else {
            command_string << "cp" << sourceFile_ << destname;
            setIsLink( No );
            break;
        }

        case Link:
        if( destname.exists() ) return DestExist;
        else {
            command_string << "ln" << "-s" << sourceFile_ << destname;
            setIsLink( Yes );
            break;
        }

        case ForceCopy:
        destname.remove();
        command_string << "cp" << sourceFile_ << destname;
        setIsLink( No );
        break;

        case ForceLink:
        destname.remove();
        command_string << "ln" << "-s" << sourceFile_ << destname;
        setIsLink( Yes );
        break;

        case CopyVersion:
        if( destname.exists() && destname.diff( sourceFile_ ) )
        { destname = destname.version(); }
        command_string << "cp" << sourceFile_ << destname;
        setIsLink( No );
        break;

        case LinkVersion:
        if( destname.exists() && destname.diff( sourceFile_ ) )
        { destname = destname.version(); }
        command_string << "ln" << "-s" << sourceFile_ << destname;
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
    if( file_.exists() )
    {
        _setCreation( file_.created() );
        _setModification( file_.lastModified() );
    }

    setIsValid( true );
    return Success;
}

//________________________________________
File Attachment::shortFile() const
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
    { file_ = File( file_.left( file_.size() - regexp.matchedLength() ) ); }

    return;
}
