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
  \file Attachment.cpp
  \brief Attached file object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "Attachment.h"
#include "Debug.h"
#include "File.h"
#include "HtmlUtil.h"
#include "LogEntry.h"
#include "Str.h"
#include "Util.h"
#include "XmlDef.h"
#include "XmlOptions.h"

#include <stdio.h>
#include <string.h>

using namespace std;

//_______________________________________
const string Attachment::NO_FILE( "" );
const string Attachment::NO_COMMENTS( "no comments" );
const string Attachment::NO_SIZE( " - " );

//_______________________________________
Attachment::Attachment( const std::string orig, const AttachmentType& type ):
  Counter( "Attachment" ),
  type_( AttachmentType::UNKNOWN ),
  source_file_( orig ),
  file_( NO_FILE ),
  comments_( NO_COMMENTS ),
  size_( 0 ),
  size_str_( NO_SIZE ),
  is_link_( UNKNOWN ),
  is_valid_( false )
{ setType( type ); }
  
//_______________________________________
Attachment::Attachment( const QDomElement& element):
  Counter( "Attachment" ),
  type_( AttachmentType::UNKNOWN ),
  source_file_( NO_FILE ),
  file_( NO_FILE ),
  comments_( NO_COMMENTS ),
  size_( 0 ),
  size_str_( NO_SIZE ),
  is_link_( UNKNOWN ),
  is_valid_( false )
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
    
    if( name == XML::SOURCE_FILE ) _setSourceFile( File( qPrintable( XmlUtil::xmlToText( value ) ) ) );
    else if( name == XML::TYPE ) setType( AttachmentType::get( qPrintable( value ) ) );
    else if( name == XML::FILE ) _setFile( File( qPrintable( XmlUtil::xmlToText( value ) ) ) );
    else if( name == XML::COMMENTS ) setComments( qPrintable( XmlUtil::xmlToText(value) ) );
    else if( name == XML::VALID ) setIsValid( (bool) value.toInt() );
    else if( name == XML::IS_LINK ) setIsLink( (LinkState) value.toInt() );  
    else cerr << "unrecognized attachment attribute: \"" << qPrintable( name ) << "\"\n";
  }
  
  // parse children elements
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( child_element.tagName() == XML::COMMENTS )
    setComments( qPrintable( XmlUtil::xmlToText( child_element.text() ) ) );
    else cout << "Attachment::Attachment - unrecognized child " << qPrintable( child_element.tagName() ) << ".\n";
  }
  
}

//____________________________________________________
QDomElement Attachment::domElement( QDomDocument& parent ) const
{
  
  Debug::Throw( "Attachment::DomElement.\n" );
  QDomElement out( parent.createElement( XML::ATTACHMENT ) );
  if( file().size() ) out.setAttribute( XML::FILE, XmlUtil::textToXml( file().c_str() ) );
  if( sourceFile().size() ) out.setAttribute( XML::SOURCE_FILE, XmlUtil::textToXml( sourceFile().c_str() ) );
  out.setAttribute( XML::TYPE, type().key().c_str() );
  out.setAttribute( XML::VALID, QString().setNum( (int) isValid() ) );
  out.setAttribute( XML::IS_LINK, QString().setNum( (int) isLink() ) );
  if( comments().size())
  {
    out.
      appendChild( parent.createElement(  XML::COMMENTS ) ).
      appendChild( parent.createTextNode( XmlUtil::textToXml( comments().c_str() ) ) );
  }
  
  return out;
    
}

//_______________________________________
void Attachment::setType( const AttachmentType& type )
{
  
  Debug::Throw() << "Attachment::setType.\n";
  type_  = type;
  
  if( Attachment::type() == AttachmentType::URL ) 
  { setIsLink( YES ); }
  
  return;
} 

//___________________________________
bool Attachment::operator < ( const Attachment &attachment ) const 
{ return Str( shortFile() ).isLower( attachment.shortFile(), XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ); }  

//__________________________________
void Attachment::updateSize( void )
{
  
  // check type
  if( type() == AttachmentType::URL || size() != 0 || !isValid() ) return;
  size_ = file().fileSize();
  size_str_ = file().sizeString();
   
}

//__________________________________
LogEntry* Attachment::entry( void ) const
{
  BASE::KeySet<LogEntry> entries( this );
  assert( entries.size() == 1 );
  return *entries.begin();
}

//__________________________________
Attachment::ErrorCode Attachment::copy( const Command& command, const string& destdir )
{
  Debug::Throw() << "Attachment::ProcessCopy.\n";
  
  // check original file
  if( source_file_.empty() ) {
    cout << "Attachment::ProcessCopy - orig not set. Canceled.\n";
    return SOURCE_NOT_FOUND; // returns true to cancel version processing
  }

  // for URL attachments, just copy origin to file, whatever the command
  if( type() == AttachmentType::URL ) {
    _setFile( source_file_ );
    return SUCCESS;
  }
  
  // check destination directory
  if( destdir.empty() ) {
    cerr << "Attachment::ProcessCopy - destdir not set. Canceled.\n";
    return DEST_NOT_FOUND;  // returns true to cancel version processing
  }  
  
  // generate expanded source name
  File fullname( source_file_ .expand() );
  if( !( type() == AttachmentType::URL || fullname.exists() ) ) return SOURCE_NOT_FOUND;
  else if( !( type() == AttachmentType::URL ) && fullname.isDirectory() ) return SOURCE_IS_DIR;
    
  // destination filename
  File destname( fullname.localName().addPath( destdir ).expand() );
  source_file_ = fullname;

  // for other process command
  QStringList command_string;
  switch (command) {
    
    case COPY:
    if( destname.exists() ) return DEST_EXIST;
    else {
      command_string << "cp" << source_file_.c_str() << destname.c_str();
      setIsLink( NO );
      break;
    }
    
    case LINK:
    if( destname.exists() ) return DEST_EXIST;
    else {
      command_string << "ln" << "-s" << source_file_.c_str() << destname.c_str();
      setIsLink( YES );
      break;
    }
    
    case COPY_FORCED:
    destname.remove();
    command_string << "cp" << source_file_.c_str() << destname.c_str();
    setIsLink( NO );
    break;
  
    case LINK_FORCED:
    destname.remove();
    command_string << "ln" << "-s" << source_file_.c_str() << destname.c_str();
    setIsLink( YES );
    break;
    
    case COPY_VERSION:
    if( destname.exists() && destname.diff( source_file_ ) ) 
    { destname = destname.version(); }
    command_string << "cp" << source_file_.c_str() << destname.c_str();
    setIsLink( NO );
    break;
    
    case LINK_VERSION:
    if( destname.exists() && destname.diff( source_file_ ) ) 
    { destname = destname.version(); }
    command_string << "ln" << "-s" << source_file_.c_str() << destname.c_str();
    setIsLink( YES );
    break;
    
    case DO_NOTHING:
    break;
    
    default:
    throw runtime_error( DESCRIPTION( "unrecognized command" ) );
  }
  
  // process copy
  Util::run( command_string );  
  
  // update long/short filenames.
  _setFile( destname );
  setIsValid( true );
  return SUCCESS;
}

//________________________________________
File Attachment::shortFile( void ) const
{
  Debug::Throw( "Attachment::shortFile.\n" );
  
  File file( file_ );
  
  // remove trailing slash
  if( file.size() && file[file.size()-1] == '/' ) { file = File( file.substr( 0, file.size()-1 ) ); }
   
  return file.localName();
}

//_______________________________________
void Attachment::htmlElement( QDomElement& parent, QDomDocument& document ) const
{
  Debug::Throw( "Attachment::HtmlElement.\n" );
  
  // reference to attached file
  QDomElement par = parent.appendChild( document.createElement( "p" ) ).toElement();
  QDomElement ref = par.
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createElement( "a" ) ).toElement();
  if( type() == AttachmentType::URL ) ref.setAttribute( "href", file().c_str() );
  else ref.setAttribute( "href", (string("file:")+file()).c_str() );
  ref.appendChild( document.createTextNode( shortFile().c_str() ) );
  
  ostringstream what;
  what << "(" << type().name() << ")";
  par.appendChild( document.createTextNode( what.str().c_str() ) );
  
  // comments
  if( comments().size() )
  HtmlUtil::textNode( comments().c_str(), par, document );
  return;
}  
  
//_______________________________________
void Attachment::_setFile( const File& file )
{
  Debug::Throw() << "Attachment::_SetFile.\n";

  // store file
  file_  = file;
 
  // remove trailing spaces
  static QRegExp regexp( "\\s+$" );
  if( regexp.indexIn( file_.c_str() ) >= 0 )
  { file_ = File( file_.substr( 0, file_.size() - regexp.matchedLength() ) ); }

  return;
} 
