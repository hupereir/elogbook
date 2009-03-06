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
  \file Logbook.cpp
  \brief log file parser based on xml
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QFile>
#include <QTextStream>

#include "Attachment.h"
#include "Debug.h"
#include "FileCheck.h"
#include "HtmlTextNode.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "XmlOptions.h"
#include "Str.h"
#include "Util.h"
#include "XmlDef.h"
#include "XmlString.h"
#include "XmlTimeStamp.h"

using namespace std;

//________________________________
// public methods

const QString Logbook::LOGBOOK_NO_TITLE = "untitled";
const QString Logbook::LOGBOOK_NO_AUTHOR = "anonymous";
const QString Logbook::LOGBOOK_NO_FILE = "";
const QString Logbook::LOGBOOK_NO_DIRECTORY = "";

//________________________________
Logbook::Logbook( const File& file ):
  Counter( "Logbook" ),
  modified_( false ),
  file_( "" ),
  parent_file_( "" ),
  directory_( LOGBOOK_NO_DIRECTORY ),
  title_( LOGBOOK_NO_TITLE ),
  author_( LOGBOOK_NO_AUTHOR ),
  creation_( TimeStamp::now() ),
  sort_method_( Logbook::SORT_CREATION ),
  sort_order_( 0 ),
  xml_entries_( 0 ),
  xml_children_( 0 )
{
  Debug::Throw( "Logbook::Logbook. (file)\n" );
  setFile( file );
  read();

}

//_________________________________
Logbook::~Logbook( void )
{
  Debug::Throw( "Logbook::~Logbook.\n" );

  // delete log children
  for( List::iterator it = children_.begin(); it != children_.end(); it++ )
  delete *it;
  children_.clear();

  // delete associated entries
  BASE::KeySet<LogEntry> entries( this );
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  delete *iter;

}


//_________________________________
bool Logbook::read( void )
{

  Debug::Throw( "Logbook::read.\n" );

  if( file().isEmpty() )
  {
    Debug::Throw( "Logbook::read - file is empty.\n" );
    return false;
  }

  // update StateFrame
  QString buffer;
  QTextStream( &buffer ) << "Reading \"" << file().localName() << "\"";
  emit messageAvailable( buffer );

  // check input file
  if( !file().exists() ) {
    Debug::Throw(0) << "Logbook::read - ERROR: cannot access file \"" << file() << "\".\n";
    return false;
  }

  // delete associated entries
  BASE::KeySet<LogEntry> entries( Logbook::entries() );
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  delete *iter;

  // parse the file
  QFile file( Logbook::file() );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    Debug::Throw(0, "Logbook::read - cannot open file.\n" );
    return false;
  }

  // create document
  QDomDocument document;
  error_ = XmlError( Logbook::file() );
  if ( !document.setContent( &file, &error_.error(), &error_.line(), &error_.column() ) )
  {
    file.close();
    Debug::Throw( "Logbook::read - cannot read file.\n" );
    return false;
  }

  // read first child
  QDomElement doc_element = document.documentElement();
  QString tag_name( doc_element.tagName() );
  if( tag_name != XML::LOGBOOK )
  {
    Debug::Throw(0) << "Logbook::read - invalid tag name: " << tag_name << endl;
    return false;
  }

  // read attributes
  QDomNamedNodeMap attributes( doc_element.attributes() );
  for( unsigned int i=0; i<attributes.length(); i++ )
  {

    QDomAttr attribute( attributes.item( i ).toAttr() );
    if( attribute.isNull() ) continue;
    QString name( attribute.name() );
    QString value( attribute.value() );

    if( name == XML::TITLE ) setTitle( XmlString( value ).toText() );
    else if( name == XML::FILE ) setFile( File( XmlString( value ).toText() ) );
    else if( name == XML::PARENT_FILE ) setParentFile( XmlString( value ).toText() );
    else if( name == XML::DIRECTORY ) setDirectory( File( XmlString( value ).toText() ) );
    else if( name == XML::AUTHOR ) setAuthor( XmlString( value ).toText() );
    else if( name == XML::SORT_METHOD ) setSortMethod( (SortMethod) value.toInt() );
    else if( name == XML::SORT_ORDER ) setSortOrder( value.toInt() );
    else if( name == XML::ENTRIES ) {
      
      setXmlEntries( value.toInt() );
      emit maximumProgressAvailable( value.toInt() );
      
    } else if( name == XML::CHILDREN ) setXmlChildren( value.toInt() );
    else Debug::Throw(0) << "Logbook::read - unrecognized logbook attribute: \"" << name << "\"\n";

  }

  // parse children
  static unsigned int progress( 10 );
  unsigned int entry_count( 0 );
  for(QDomNode node = doc_element.firstChild(); !node.isNull(); node = node.nextSibling() )
  {
    QDomElement element = node.toElement();
    if( element.isNull() ) continue;

    QString tag_name( element.tagName() );

    // children
    if( tag_name == XML::COMMENTS ) setComments( XmlString( element.text() ).toText() );
    else if( tag_name == XML::CREATION ) setCreation( XmlTimeStamp( element ) );
    else if( tag_name == XML::MODIFICATION ) setModification( XmlTimeStamp( element ) );
    else if( tag_name == XML::BACKUP ) setBackup( XmlTimeStamp( element ) );
    else if( tag_name == XML::ENTRY )
    {

      LogEntry* entry = new LogEntry( element );
      Key::associate( latestChild(), entry );
      entry_count++;
      if( !(entry_count%progress) ) emit progressAvailable( progress );
      
    } else if( tag_name == XML::CHILD ) {

      // try retrieve file from attributes
      QString file_attribute( element.attribute( XML::FILE ) );
      if( file_attribute.isNull() )
      {
        Debug::Throw(0) << "Logbook::read - no file given for child" << endl;
        continue;
      }

      File file( file_attribute );
      if( !file.isAbsolute() ) file = file.addPath( Logbook::file().path() );
      Logbook* child = new Logbook();
      child->setFile( file );
      
      // propagate progressAvailable signal.
      connect( child, SIGNAL( progressAvailable( unsigned int ) ), SIGNAL( progressAvailable( unsigned int ) ) );

      QString buffer;
      QTextStream( &buffer ) << "Reading \"" << child->file().localName() << "\"";
      emit messageAvailable( buffer );

      child->read();
      children_.push_back( child );

    } else Debug::Throw(0) << "Logbook::read - unrecognized tag_name: " << tag_name << endl;

  }
  
  emit progressAvailable( entry_count%progress );

  // discard modifications
  setModified( false );
  saved_ = Logbook::file().lastModified();
  return true;

}

//_________________________________
bool Logbook::write( File file )
{

  Debug::Throw( "Logbook::write.\n" );
  
  // check filename
  if( file.isEmpty() ) file = Logbook::file();
  if( file.isEmpty() ) return false;

  Debug::Throw( ) << "Logbook::write - \"" << file << "\".\n";
  bool completed = true;

  // check number of entries and children to save in header
  if( setXmlEntries( entries().size() ) || setXmlChildren( children().size() ) )
  { setModified( true ); }  
  
  emit maximumProgressAvailable( xmlEntries() );

  // write logbook if filename differs from origin or logbook is modified
  if( file != Logbook::file() || modified_ )
  {

    // gets last saved timestamp
    TimeStamp saved_prev( file.lastModified() );

    // make a backup of the file, if necessary
    file.backup();

    // update stateFrame
    QString buffer;
    QTextStream( &buffer ) << "Writing \"" << file.localName() << "\"";
    emit messageAvailable( buffer );

    QFile out( file );
    if( !out.open( QIODevice::WriteOnly ) )
    {
      Debug::Throw(0) << "Logbook::write - unable to write to file " << file << endl;
      return false;
    }

    // create document
    QDomDocument document;

    // create main element
    QDomElement top = document.createElement( XML::LOGBOOK );
    if( !title().isEmpty() ) top.setAttribute( XML::TITLE, XmlString( title() ).toXml() );
    if( !directory().isEmpty() ) top.setAttribute( XML::DIRECTORY, XmlString(directory()) );
    if( !author().size() ) top.setAttribute( XML::AUTHOR, XmlString( author() ).toXml() ) ;

    top.setAttribute( XML::SORT_METHOD, Str().assign<unsigned int>(sort_method_) );
    top.setAttribute( XML::SORT_ORDER, Str().assign<int>(sort_order_) );

    // update number of entries and children
    top.setAttribute( XML::ENTRIES, Str().assign<int>(xmlEntries()) );
    top.setAttribute( XML::CHILDREN, Str().assign<int>(xmlChildren()) );

    // append node
    document.appendChild( top );

    // comments
    if( comments().size() )
    {
      QDomElement comments_element = document.createElement( XML::COMMENTS );
      QDomText comments_text = document.createTextNode( XmlString( comments() ).toXml() );
      comments_element.appendChild( comments_text );
      top.appendChild( comments_element );
    }

    // write time stamps
    if( creation().isValid() ) top.appendChild( XmlTimeStamp( creation() ).domElement( XML::CREATION, document ) );
    if( modification().isValid() ) top.appendChild( XmlTimeStamp( modification() ).domElement( XML::MODIFICATION, document ) );
    if( backup().isValid() ) top.appendChild( XmlTimeStamp( backup() ).domElement( XML::BACKUP, document ) );

    // write all entries
    static unsigned int progress( 10 );
    unsigned int entry_count( 0 );
    BASE::KeySet<LogEntry> entries( this );
    for( BASE::KeySet<LogEntry>::iterator it = entries.begin(); it != entries.end(); it++ )
    {
      
      top.appendChild( (*it)->domElement( document ) );
      entry_count++;
      if( !(entry_count%progress) ) emit progressAvailable( progress );

    }
    
    emit progressAvailable( entry_count%progress );

    // dump all logbook childrens
    unsigned int child_number=0;
    for( List::iterator it = children_.begin(); it != children_.end(); it++, child_number++ )
    {
      File child_filename = _childFilename( file, child_number );
      QDomElement child_element = document.createElement( XML::CHILD );
      child_element.setAttribute( XML::FILE, XmlString( child_filename ).toXml() );
      top.appendChild( child_element );
    }

    // finish
    out.write( document.toByteArray() );
    out.close();

    // gets/check new saved timestamp
    TimeStamp saved_new( file.lastModified() );
    if( !( saved_prev < saved_new ) ) completed = false;
    else if( file == Logbook::file() ) {

      // change logbook state if saved in nominal file
      // stores now as last save time
      modified_ = false;

    }
  } else { emit progressAvailable( BASE::KeySet<LogEntry>( this ).size() ); }

  
  // update saved timeStamp
  saved_ = Logbook::file().lastModified();

  // write children
  unsigned int child_number=0;
  for( List::iterator it = children_.begin(); it != children_.end(); it++, child_number++ )
  {

    File child_filename( _childFilename( file, child_number ).addPath( file.path() ) );

    // update stateFrame
    QString buffer;
    QTextStream( &buffer ) << "Writing \"" << child_filename.localName() << "\"";
    emit messageAvailable( buffer );

    (*it)->setParentFile( file );
    completed &= (*it)->write( child_filename );
  }

  Debug::Throw( ) << "Logbook::write - \"" << file << "\". Done.\n";
  return completed;
}

//_________________________________
map<LogEntry*,LogEntry*> Logbook::synchronize( const Logbook& logbook )
{
  Debug::Throw( "Logbook::synchronize.\n" );

  // retrieve logbook entries
  BASE::KeySet<LogEntry> new_entries( logbook.entries() );
  BASE::KeySet<LogEntry> current_entries( entries() );

  // map of duplicated entries
  std::map< LogEntry*, LogEntry* > duplicates;

  // merge new entries into current entries
  for( BASE::KeySet< LogEntry>::iterator it = new_entries.begin(); it != new_entries.end(); it++ )
  {

    // check if there is an entry with matching creation and modification time
    BASE::KeySet< LogEntry >::iterator duplicate( find_if(
      current_entries.begin(),
      current_entries.end(),
      LogEntry::DuplicateFTor( *it ) ) );

    // if duplicate entry found and modified more recently, skip the new entry
    if( duplicate != current_entries.end() && (*duplicate)->modification() >= (*it)->modification() ) continue;

    // retrieve logbook where entry is to be added
    Logbook* child( latestChild() );

    // create a new entry
    LogEntry *entry( (*it )->clone() );

    // associate entry with logbook
    Key::associate( entry, child );

    // set child as modified
    child->setModified( true );

    // safe remove the duplicated entry
    if( duplicate != current_entries.end() )
    {
      // set logbooks as modified
      // and disassociate with entry
      BASE::KeySet<Logbook> logbooks( *duplicate );
      for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
      {
        (*iter)->setModified( true );
        BASE::Key::disassociate( *iter, *duplicate );
      }

      // insert duplicate pairs in map
      duplicates.insert( make_pair( *duplicate, *it ) );
      
      // reset current entries
      current_entries = entries();

    }

  }

  return duplicates;

}

//_________________________________
XmlError::List Logbook::xmlErrors( void ) const
{
  Debug::Throw( "Logbook::xmlErrors.\n" );
  XmlError::List out;
  if( error_ ) out.push_back( error_ );
  for( List::const_iterator it = children_.begin(); it != children_.end(); it++ )
  {
    XmlError::List tmp( (*it)->xmlErrors() );
    out.merge( tmp );
  }
  return out;
}

//_________________________________
Logbook::List Logbook::children( void ) const
{
  List out;
  for(
    List::const_iterator it = children_.begin();
    it!= children_.end();
    it++ )
  {
    out.push_back( *it );
    List children( (*it)->children() );
    out.merge( children );
  }

  return out;
}

//_________________________________
Logbook* Logbook::latestChild( void )
{

  Debug::Throw( "Logbook::latestChild.\n" );

  // get older parent
  Logbook* dest = 0;

  // check parent number of entries
  if( BASE::KeySet<LogEntry>(this).size() < MAX_ENTRIES ) dest = this;

  // check if one existsing child is not complete
  else {
    for( List::iterator it = children_.begin(); it != children_.end(); it++ )
    if( *it && BASE::KeySet<LogEntry>(*it).size() < MAX_ENTRIES ) {
      dest = *it;
      break;
    }
  }

  // add a new child if nothing found
  if( !dest ) {

    dest = new Logbook();
    dest->setTitle( title() );
    dest->setDirectory( directory() );
    dest->setAuthor( author() );
    dest->setFile( _childFilename( file(), children_.size() ).addPath( file().path() ) );
    dest->setModified( true );

    children_.push_back( dest );
    setModified( true );
    
    // associate to existing FileCheck if any
    BASE::KeySet<FileCheck> file_checks( this );
    if( !file_checks.empty() )
    { 
      assert( file_checks.size() == 1 );
      (*file_checks.begin())->registerLogbook( dest );
    }

  }

  return dest;
}

//_________________________________
BASE::KeySet<LogEntry> Logbook::entries( void ) const
{

  BASE::KeySet<LogEntry> out( this );
  for( List::const_iterator iter = children_.begin(); iter != children_.end(); iter++ )
  out.merge( (*iter)->entries() );

  return out;

}

//_________________________________
BASE::KeySet<Attachment> Logbook::attachments( void ) const
{

  BASE::KeySet<Attachment> out;

  // loop over associated entries, add entries associated attachments
  BASE::KeySet<LogEntry> entries( this );
  for( BASE::KeySet<LogEntry>::iterator iter = entries.begin(); iter != entries.end(); iter++ )
  out.merge( BASE::KeySet<Attachment>(*iter) );

  // loop over children, add associated attachments
  for( List::const_iterator iter = children_.begin(); iter != children_.end(); iter++ )
  out.merge( (*iter)->attachments() );

  return out;

}

//_________________________________
void Logbook::removeEmptyChildren( void )
{
  Debug::Throw( "Logbook::removeEmptyChildren.\n" );

  // loop over children
  List tmp;
  for( List::iterator iter = children_.begin(); iter != children_.end(); iter++ )
  {
    (*iter)->removeEmptyChildren();
    if( (*iter)->empty() )
    {

      // remove file
      if( !(*iter)->file().isEmpty() ) (*iter)->file().remove();

      // delete logbook
      delete *iter;

    } else tmp.push_back( *iter );
  }

  children_ = tmp;
  return;
}

//_________________________________
void Logbook::setFile( const File& file )
{
  Debug::Throw( "Logbook::setFile.\n" );
  file_ = file; 
  saved_ = File( file_ ).lastModified();
}

//_________________________________
bool Logbook::needsBackup( void ) const
{
  Debug::Throw( "Logbook::needsBackup.\n" );
  if( !backup().isValid() ) return true;
  return( int(TimeStamp::now())-int(backup()) > (24*3600)*XmlOptions::get().get<double>( "BACKUP_ITV" ) );
}

//_________________________________
QString Logbook::backupFilename( void ) const
{
  Debug::Throw( "Logbook::MakeBackupFilename.\n" );
  QString head( File( file_ ).truncatedName() );
  QString foot( File( file_ ).extension() );
  if( !foot.isEmpty() ) foot = QString(".") + foot;
  QString tag( TimeStamp::now().toString( TimeStamp::DATE_TAG ) );
  QString out;
  QTextStream( &out ) << head << "_backup_" << tag << foot;
  return out;
}

//__________________________________
QDomElement Logbook::htmlElement( QDomDocument& document, const unsigned int& mask ) const
{
  Debug::Throw( "Logbook::htmlElement.\n" );

  // check header
  if( !(mask & HTML_ALL_MASK ) ) return QDomElement();

  // surrounding table
  QDomElement out = document.createElement( "table" );
  out.setAttribute( "class", "header_outer_table" );
  QDomElement column = out.
    appendChild( document.createElement( "tr" ) ).
    appendChild( document.createElement( "td" ) ).
    toElement();
  column.setAttribute( "class", "header_column" );
  QDomElement table = column.
    appendChild( document.createElement( "table" ) ).
    toElement();
  table.setAttribute( "class", "header_inner_table" );
  table.setAttribute( "width", "100%" );

  QDomElement row;
  if( title().size() && (mask&HTML_TITLE) )
  {
    row = table.appendChild( document.createElement( "tr" ) ).toElement();
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "colspan", "2" );
    column.
      appendChild( document.createElement( "h1" ) ).
      appendChild( document.createTextNode( title() ) );
  }

  if( author().size() && (mask&HTML_AUTHOR ) )
  {
    
    row = table.appendChild( document.createElement( "tr" ) ).toElement();
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "width", "15%" );
    column.appendChild( document.createTextNode( "Author:" ) );
    row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( author() ) );

  }

  if( file().size() && (mask&HTML_FILE ) )
  {
    row = table.appendChild( document.createElement( "tr" ) ).toElement();
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "width", "15%" );
    column.appendChild( document.createTextNode( "File:" ) );
    row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( file() ) );

  }

  if( directory().size() && (mask&HTML_DIRECTORY ) )
  {
    row = table.appendChild( document.createElement( "tr" ) ).toElement();
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "width", "15%" );
    column.appendChild( document.createTextNode( "Directory:" ) );
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( directory() ) );
    if( !checkDirectory() ) column.appendChild( document.createTextNode( " (not found)" ) );

  }

  if( creation().isValid() && (mask&HTML_CREATION ) )
  {
    row = table.appendChild( document.createElement( "tr" ) ).toElement();
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "width", "15%" );
    column.appendChild( document.createTextNode( "Created:" ) );
    row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( creation().toString() ) );

  }

  if( modification().isValid() && (mask&HTML_MODIFICATION ) )
  {
    row = table.appendChild( document.createElement( "tr" ) ).toElement();
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "width", "15%" );
    column.appendChild( document.createTextNode( "Last modified:" ) );
    row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( modification().toString() ) );

  }

  if( backup().isValid() && (mask&HTML_BACKUP) )
  {
    row = table.appendChild( document.createElement( "tr" ) ).toElement();
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "width", "15%" );
    column.appendChild( document.createTextNode( "Last backup:" ) );
    row.
      appendChild( document.createElement( "td" ) ).
      appendChild( document.createElement( "b" ) ).
      appendChild( document.createTextNode( backup().toString() ) );

  }

  if( comments().size() && (mask&HTML_COMMENTS) )
  {
    row = table.appendChild( document.createElement( "tr" ) ).toElement();
    column = row.appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "width", "15%" );
    column.appendChild( document.createTextNode( "Comments:" ) );
    column = table.
      appendChild( document.createElement( "tr" ) ).
      appendChild( document.createElement( "td" ) ).toElement();
    column.setAttribute( "colspan", "2" );
    HtmlTextNode( comments(), column, document );
  }

  return out;
}

//_________________________________
void Logbook::setModified( const bool& value )
{
  Debug::Throw( "Logbook::setModified.\n");
  modified_ = value;
  if( value ) setModification( TimeStamp::now() );
}

//_________________________________
void Logbook::setModifiedRecursive( bool value )
{
  Debug::Throw( "Logbook::SetModifiedRecursive.\n" );
  modified_ = value;
  if( value ) setModification( TimeStamp::now() );
  for( List::iterator it = children_.begin(); it != children_.end(); it++ )
  (*it)->setModifiedRecursive( value );
}

//_________________________________
void Logbook::setModification( const TimeStamp& stamp )
{
  Debug::Throw( "Logbook::SetModification.\n" );
  modification_ = stamp;
}

//_________________________________
bool Logbook::modified( void ) const
{
  Debug::Throw( "Logbook::modified.\n" );
  if( modified_ ) return true;
  for( List::const_iterator it = children_.begin(); it != children_.end(); it++ )
  if ( (*it)->modified() ) return true;
  return false;
}

//______________________________________________________________________
bool Logbook::setSortMethod( const Logbook::SortMethod& sort_method )
{
  Debug::Throw( "Logbook::setSortMethod.\n" );
  bool changed = ( sortMethod() != sort_method );
  if( changed ) {
    sort_method_ = sort_method;
    setModified( true );
  }
  return changed;
}

//______________________________________________________________________
bool Logbook::setSortOrder( const int& order )
{
  Debug::Throw( "Logbook::setSortOrder.\n" );
  bool changed = (sortOrder() != order );
  if( changed )
  {
    sort_order_ = order;
    setModified( true );
  }
  return changed;
}

//_________________________________
bool Logbook::EntryLessFTor::operator () ( LogEntry* first, LogEntry* second ) const
{
  
  if( order_ ) std::swap( first, second );
  
  switch( sort_method_ )
  {
    
    case Logbook::SORT_COLOR:
    return (first->color() < second->color() );
    break;
    
    case Logbook::SORT_CREATION:
    return (first->creation() < second->creation());
    break;

    case Logbook::SORT_MODIFICATION:
    return (first->modification() < second->modification());
    break;

    case Logbook::SORT_TITLE:
    return (first->title() < second->title());
    break;

    case Logbook::SORT_KEYWORD:
    return (first->keyword() < second->keyword());

    case Logbook::SORT_AUTHOR:
    return (first->author() < second->author());

    default:
    Debug::Throw(0,"EntryLessFTor - invalid sort method.\n" );
    break;
  }
  return false;
}

//_________________________________
File Logbook::_childFilename( const File& file, const int& child_number ) const
{
  File head( file.localName().truncatedName() );
  QString foot( file.extension() );
  if( !foot.isEmpty() ) foot = QString(".") + foot;
  
  QString out; 
  QTextStream(&out) << head << "_include_" << child_number << foot;
  Debug::Throw( ) << "Logbook::_MakeChildFilename - \"" << out << "\".\n";
  return out;

}
