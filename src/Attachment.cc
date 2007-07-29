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
  \file Attachment.cc
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
using namespace BASE;

//_______________________________________
const string Attachment::NO_FILE( "" );
const string Attachment::NO_COMMENTS( "no comments" );

//_______________________________________
Attachment::Attachment( const QDomElement& element):
  Counter( "Attachment" ),
  type_( AttachmentType::UNKNOWN ),
  source_file_( NO_FILE ),
  file_( NO_FILE ),
  comments_( NO_COMMENTS ),
  size_( 0 ),
  size_str_( "-" ),
  is_valid_( false )
{
  Debug::Throw() << "Attachment::Attachment.\n";
  
   // parse attributes
  QDomNamedNodeMap attributes( element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {
    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
    
    if( name == XML::SOURCE_FILE ) _setSourceFile( XmlUtil::xmlToText( value ) );
    else if( name == XML::TYPE ) setType( AttachmentType::get( value ) );
    else if( name == XML::FILE ) _setFile( XmlUtil::xmlToText( value ) );
    else if( name == XML::COMMENTS ) setComments( XmlUtil::xmlToText(value) );
    else cerr << "unrecognized attachment attribute: \"" << name << "\"\n";
  }
  
  // parse children elements
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() ) 
  {
    QDomElement child_element = child_node.toElement(); 
    if( string( qPrintable( child_element.tagName() ) ) == XML::COMMENTS )
    setComments( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
    else cout << "Attachment::Attachment - unrecognized child " << qPrintable( child_element.tagName() ) << ".\n";
  }
  
}

//____________________________________________________
QDomElement Attachment::domElement( QDomDocument& parent ) const
{
  
  Debug::Throw( "Attachment::DomElement.\n" );
  QDomElement out( parent.createElement( XML::ATTACHMENT.c_str() ) );
  if( file().size() ) out.setAttribute( XML::FILE.c_str(), XmlUtil::textToXml( file() ).c_str() );
  if( sourceFile().size() ) out.setAttribute( XML::SOURCE_FILE.c_str(), XmlUtil::textToXml( sourceFile() ).c_str() );
  out.setAttribute( XML::TYPE.c_str(), type().key().c_str() );
  
  if( comments().size())
  {
    out.
      appendChild( parent.createElement(  XML::COMMENTS.c_str() ) ).
      appendChild( parent.createTextNode( XmlUtil::textToXml( comments() ).c_str() ) );
  }
  
  return out;
    
}

//_______________________________________
void Attachment::setType( const AttachmentType& type )
{
  Debug::Throw() << "Attachment::setType.\n";
  type_  = type;
  if( type_ == AttachmentType::URL ) is_valid_ = true;
  return;
} 

//___________________________________
bool Attachment::operator < ( const Attachment &attachment ) const 
{ return Str( shortFile() ).isLower( attachment.shortFile(), XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ); }  

//__________________________________
LogEntry* Attachment::entry( void ) const
{
  KeySet<LogEntry> entries( this );
  Exception::assert( entries.size() == 1, DESCRIPTION( "wrong association to LogEntry" ) );
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
  if( type_ == AttachmentType::URL ) {
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
  if( !( type_ == AttachmentType::URL || fullname.exist() ) ) return SOURCE_NOT_FOUND;
  else if( !( type_ == AttachmentType::URL ) && fullname.isDirectory() ) return SOURCE_IS_DIR;
    
  // destination filename
  File destname( fullname.localName().addPath( destdir ).expand() );
  source_file_ = fullname;

  // for other process command
  string command_string;
  switch (command) {
    
    case COPY:
      
    if( destname.exist() ) return DEST_EXIST;
    else {
      command_string = string("cp \"") + source_file_ + "\" \"" + destname + "\"";
      break;
    }
    
    case LINK:
    
    if( destname.exist() ) return DEST_EXIST;
    else {
      command_string = string("ln -s \"") + source_file_ + "\" \"" + destname + "\"";
      break;
    }
    
    case COPY_FORCED:
    
    // first delete destination file
    destname.remove();

    // perform the copy
    command_string = string("cp \"") + source_file_ + "\" \"" + destname + "\"";
    break;
  
    case LINK_FORCED:
    
    // first remove destination file
    destname.remove();
    command_string = string("ln -s \"") + source_file_ + "\" \"" + destname + "\"";
    break;
    
    case COPY_VERSION:
      
    if( destname.exist() ) 
    {
      if( destname.diff( source_file_ ) ) 
      {
        destname = destname.version();
        command_string = string("cp \"") + source_file_ + "\" \"" + destname + "\"";
      }
    } else command_string = string("cp \"") + source_file_ + "\" \"" + destname + "\"";
    break;
    
    case LINK_VERSION:
      
    if( destname.exist() ) 
    {
      if( destname.diff( source_file_ ) ) 
      {
        destname = destname.version();
        command_string = string("ln -s \"") + source_file_ + "\" \"" + destname + "\"";
      }
    } else  command_string = string("ln -s \"") + source_file_ + "\" \"" + destname + "\"";
    break;
    
    case DO_NOTHING:
    break;
    
    default:
    throw runtime_error( DESCRIPTION( "unrecognized command" ) );
  }
  
  // process copy
  Debug::Throw()
    << "Attachment::ProcessCopy - command= "
    << command_string
    << endl;
  if( command_string.size() ) Util::run( command_string );  
  
  // update long/short filenames.
  _setFile( destname );
  return SUCCESS;
}

//________________________________________
File Attachment::shortFile( void ) const
{
  Debug::Throw( "Attachment::shortFile.\n" );
  return( type_ == AttachmentType::URL ) ? file_:file_.localName();
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
  what << "(" << (isValid() ? type().name():"not found") << ")";
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
  file_  = file;
  if( XmlOptions::get().get<bool>("CHECK_ATTACHMENT") && file.exist() ) 
  {
    size_ = file.size();
    size_str_ = file.sizeString();
    is_valid_ = true;
    modification_ = TimeStamp( file.lastModified() );
  }
  return;
} 
