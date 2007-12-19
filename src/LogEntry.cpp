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

#include "Debug.h"
#include "Attachment.h"
#include "ColorMenu.h"
#include "HtmlUtil.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "TextFormat.h"
#include "XmlDef.h"
#include "XmlTextFormatBlock.h"
#include "XmlTimeStamp.h"
#include "XmlUtil.h"

#include <stdio.h>

using namespace std;

//__________________________________
const string LogEntry::DRAG = "logEntry_drag";
const string LogEntry::UNTITLED = "untitled";
const string LogEntry::NO_KEYWORD = "/";
const string LogEntry::NO_AUTHOR = "anonymous";
const string LogEntry::NO_TEXT = "";

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
    Str name( qPrintable( attribute.name() ) );
    Str value( qPrintable( attribute.value() ) );
    if( name == XML::TITLE ) setTitle( XmlUtil::xmlToText( value ) );
    else if( name == XML::KEYWORD ) setKeyword( XmlUtil::xmlToText( value ) );
    else if( name == XML::AUTHOR ) setAuthor( XmlUtil::xmlToText( value ) );
    else if( name == XML::COLOR ) setColor( XmlUtil::xmlToText( value ) );
    else cerr << "LogEntry::LogEntry - unrecognized entry attribute: \"" << name << "\"\n";
  }

  // parse children elements
  for(QDomNode child_node = element.firstChild(); !child_node.isNull(); child_node = child_node.nextSibling() )
  {
    QDomElement child_element = child_node.toElement();
    if( child_element.isNull() ) continue;

    Str tag_name( qPrintable( child_element.tagName() ) );
    if( tag_name == XML::TEXT ) setText( XmlUtil::xmlToText( qPrintable( child_element.text() ) ) );
    else if( tag_name == XML::CREATION ) setCreation( XmlTimeStamp( child_element ) );
    else if( tag_name == XML::MODIFICATION ) setModification( XmlTimeStamp( child_element ) );
    else if( tag_name == FORMAT::XmlTextFormatBlock::XML_TAG ) addFormat( FORMAT::XmlTextFormatBlock( child_element ) );
    else if( tag_name == XML::ATTACHMENT ) Key::associate( this, new Attachment( child_element ) );
    else cout << "Option::Option - unrecognized child " << qPrintable( child_element.tagName() ) << ".\n";
  }

}

//__________________________________
LogEntry::~LogEntry( void )
{
  Debug::Throw( "LogEntry::~LogEntry.\n" );

  // delete associated attachments
  BASE::KeySet<Attachment> attachments( this );
  for( BASE::KeySet<Attachment>::iterator iter = attachments.begin(); iter != attachments.end(); iter++ )
  delete *iter;

}

//__________________________________
QDomElement LogEntry::domElement( QDomDocument& parent ) const
{
  Debug::Throw( "LogEntry::domElement.\n" );
  QDomElement out( parent.createElement( XML::ENTRY.c_str() ) );
  if( title().size() ) out.setAttribute( XML::TITLE.c_str(), XmlUtil::textToXml(title()).c_str() );
  if( keyword().size() ) out.setAttribute( XML::KEYWORD.c_str(), XmlUtil::textToXml(keyword()).c_str() );
  if( author().size() ) out.setAttribute( XML::AUTHOR.c_str(), XmlUtil::textToXml(author()).c_str() );

  // color
  bool color_valid( color().size() && !Str(color()).isEqual( ColorMenu::NONE, false ) && QColor( color().c_str() ).isValid() );
  if( color_valid ) out.setAttribute( XML::COLOR.c_str(), XmlUtil::textToXml(color()).c_str() );

  // dump timeStamp
  if( creation().isValid() ) out.appendChild( XmlTimeStamp( creation() ).domElement( XML::CREATION, parent ) );
  if( modification().isValid() ) out.appendChild( XmlTimeStamp( modification() ).domElement( XML::MODIFICATION, parent ) );

  // dump text
  if( !text().empty() )
  {
    string text( LogEntry::text() );
    if( text[text.size()-1] != '\n' ) text+='\n';

    out.
      appendChild( parent.createElement( XML::TEXT.c_str() ) ).
      appendChild( parent.createTextNode( XmlUtil::textToXml( text ).c_str() ) );
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
string LogEntry::formatKeyword( const string& keyword )
{
  Debug::Throw() << "LogEntry::formatKeyword - " << keyword << endl;
  vector<string> keywords( parseKeyword( keyword ) );

  string out = "";
  for( vector<string>::iterator iter = keywords.begin(); iter != keywords.end(); iter++ )
  {
    string local( *iter );

    // remove empty trailing spaces
    while( local.size() && local[local.size() - 1] == ' ' )
    local = local.substr( 0, local.size() - 1 );

    // remove empty leading spaces
    while( local.size() && local[0] == ' ' )
    local = local.substr( 1, local.size() - 1  );

    // change first character to uppercase
    if( local.size() ) local[0] = toupper( local[0] );

    // append to global name
    if( local.size() )
    {
      if( out.empty() ) out = local;
      else out += "/"+local;
    }
  }
  
  // for debugging, check leading character
  assert( out.empty() || out.find( NO_KEYWORD ) != 0, "incorrect keyword format" );
  
  // add leading backspace
  out = NO_KEYWORD + out;
  return out;
}

//__________________________________
vector<string> LogEntry::parseKeyword( const string& keyword )
{
  Debug::Throw() << "LogEntry::parseKeyword" << endl;

  vector<string> out;
  size_t pos = keyword.find( "/" );
  if( pos == string::npos ) out.push_back( keyword );
  else {

    //! get leading keyword add to the list
    string first( keyword.substr( 0, pos ) );
    if( !first.empty() ) out.push_back( first );

    //! parse trailing keyword
    string second( keyword.substr( pos+1, keyword.size()-pos-1 ) );
    vector<string> tmp( parseKeyword( second ) );
    out.insert( out.end(), tmp.begin(), tmp.end() );
  
  }

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
void LogEntry::setKeyword( const string& keyword )
{
  Debug::Throw( "LoqEntry::setKeyword.\n");
  keyword_ = formatKeyword( keyword );
}

//__________________________________
bool LogEntry::matchColor( const string& buf )
{
  if( Str( buf ).isIn( color_, false ) ) return true;
  if( color_ == ColorMenu::NONE ) return false;
  return QColor( buf.c_str() ) == QColor( color_.c_str() );
}
  
//__________________________________
bool LogEntry::matchAttachment( const string& buf )
{
  Debug::Throw( "LogEntry::matchAttachment.\n" );

  // retrieve associated attachments
  BASE::KeySet<Attachment> attachments( this );
  for( BASE::KeySet<Attachment>::iterator iter( attachments.begin() ); iter != attachments.end(); iter++ )
  if( Str(buf).isIn( (*iter)->file(), XmlOptions::get().get<bool>( "CASE_SENSITIVE" ) ) )
  return true;

  return false;
}

//__________________________________
void LogEntry::modified( void )
{
  Debug::Throw( "LogEntry::modified.\n" );
  modification_ = TimeStamp::now();
}

//__________________________________
QDomElement LogEntry::htmlElement( QDomDocument& document, const unsigned int &mask )
{
  Debug::Throw( "LogEntry::htmlElement.\n" );

  QDomElement out = document.createElement( "div" );
  
  // logbook entry header
  if( mask & HTML_HEADER_MASK ) {
    
    // surrounding table
    QDomElement table = out.appendChild( document.createElement( "table" ) ).toElement();
    table.setAttribute( "class", "header_outer_table" );
    table.setAttribute( "width", "100%" );
    if( color()!="None" )
    {
      ostringstream what;
      what << "border: 2px solid" << color();
      table.setAttribute( "style", what.str().c_str() );
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
    if( keyword().size() && (mask&HTML_KEYWORD) )
    {
      row = table.appendChild( document.createElement( "tr" ) ).toElement();
      column = row.appendChild( document.createElement( "td" ) ).toElement();
      column.setAttribute( "width", "15%" );
      column.appendChild( document.createTextNode( "Keyword(s): " ) );
      row.
        appendChild( document.createElement( "td" ) ).
        appendChild( document.createElement( "b" ) ).
        appendChild( document.createTextNode( keyword().c_str() ) );

    }

    if( title().size() && (mask&HTML_TITLE) )
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
      ref.setAttribute( "name", Str().assign<int>( creation() ).c_str() );
      ref.appendChild( document.createTextNode( title().c_str() ) );
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
        appendChild( document.createTextNode( creation().string().c_str() ) );

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
        appendChild( document.createTextNode( modification().string().c_str() ) );

    }

    if( author().size() && (mask&HTML_AUTHOR) )
    {
      row = table.appendChild( document.createElement( "tr" ) ).toElement();
      column = row.appendChild( document.createElement( "td" ) ).toElement();
      column.setAttribute( "width", "15%" );
      column.appendChild( document.createTextNode( "Author: " ) );
      row.
        appendChild( document.createElement( "td" ) ).
        appendChild( document.createElement( "b" ) ).
        appendChild( document.createTextNode( author().c_str() ) );

    }

  }

  // write attachments
  BASE::KeySet<Attachment> attachments( this );
  if( attachments.size() && ( mask &  HTML_ATTACHMENT ) )
  {
    QDomElement par = out.appendChild( document.createElement("p") ).toElement();
    for( BASE::KeySet<Attachment>::iterator iter( attachments.begin() ); iter != attachments.end(); iter++ )
    (*iter)->htmlElement( par, document );
  }

  // write text
  if( text().size() && ( mask & HTML_TEXT ) )
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
    ref.setAttribute( "href", (string("#")+Str().assign<int>( creation() )).c_str() );
    ref.appendChild( document.createTextNode( (keyword().size()) ? keyword().c_str():NO_KEYWORD.c_str() ) );
  }

  if( mask & HTML_TITLE )
  {
    QDomElement ref = ((row.
        appendChild( document.createElement( "td" ) )).
        appendChild( document.createElement( "b" ) )).
        appendChild( document.createElement( "a" ) ).toElement();
    ref.setAttribute( "href", (string("#")+Str().assign<int>( creation() )).c_str() );
    ref.appendChild( document.createTextNode( (title().size()) ? title().c_str():UNTITLED.c_str() ) );
  }
  if( mask & HTML_CREATION ) row.
    appendChild( document.createElement( "td" )).
    appendChild( document.createTextNode( creation().string().c_str() ) );
  if( mask & HTML_MODIFICATION ) row.
    appendChild( document.createElement( "td" )).
    appendChild( document.createTextNode( modification().string().c_str() ) );
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
  keyword_ = NO_KEYWORD;
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
  ostringstream what;
  unsigned int index = 0;
  while( index < text().size() )
  {
    Debug::Throw() << "index: " << index << " position: " << index << endl;
    FORMAT::TextFormatBlock::List::iterator iter( find_if(
        formats.begin(),
        formats.end(),
        FORMAT::TextFormatBlock::ContainsFTor( index ) ) );
    if( iter == formats.end() || iter->isEmpty() )
    {
      what << text()[index];
      index ++;
      
    } else {

      Debug::Throw() << *iter << endl;
      
      // write previous text
      HtmlUtil::textNode( what.str().c_str(), parent, document );
      what.str("");
      
      // open new element define format
      QDomElement local_node( parent );
      if( iter->format() & FORMAT::UNDERLINE ) local_node = local_node.appendChild( document.createElement( "u" ) ).toElement();
      if( iter->format() & FORMAT::ITALIC ) local_node = local_node.appendChild( document.createElement( "i" ) ).toElement();
      if( iter->format() & FORMAT::BOLD ) local_node = local_node.appendChild( document.createElement( "b" ) ).toElement();
      if( iter->format() & FORMAT::STRIKE ) local_node = local_node.appendChild( document.createElement( "s" ) ).toElement();
      if( iter->color().size() && iter->color() != string("None") )
      {
        local_node = local_node.appendChild( document.createElement( "font" ) ).toElement();
        local_node.setAttribute( "color", iter->color().c_str() );
      }

      while( index < text().size() && (int)index < iter->end() )
      {
        what << text()[index];
        index++;
      }
      
      // remove format from list
      formats.erase( iter );
      
      // process text
      HtmlUtil::textNode( what.str().c_str(), local_node, document );
      
      // reset stream
      what.str("");
    }
  }

  HtmlUtil::textNode( what.str().c_str(), parent, document );
  what.str("");

}
