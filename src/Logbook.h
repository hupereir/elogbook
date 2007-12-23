#ifndef Logbook_h
#define Logbook_h

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
 * ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or         
 * FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License         
 * for more details.                    
 *                         
 * You should have received a copy of the GNU General Public License along with 
 * software; if not, write to the Free Software Foundation, Inc., 59 Temple     
 * Place, Suite 330, Boston, MA   02111-1307 USA                          
 *                        
 *                        
 *******************************************************************************/

/*!
   \file Logbook.h
   \brief log file parser based on xml
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QDomElement>
#include <QDomDocument>
#include <QObject>

#include <list>
#include <iostream>
#include <sstream>
#include <string>

#include "Counter.h"
#include "Debug.h"


#include "File.h"
#include "Key.h"
#include "TimeStamp.h"
#include "XmlError.h"
#include "XmlUtil.h"

class LogEntry;
class Attachment;
class XmlTextFormatInfo;

/*!
   \class Logbook
   \brief log file parser based on xml
*/
class Logbook:public QObject, public Counter, public BASE::Key 
{
 
  Q_OBJECT
   
  public:   
    
  //! default string when no title given
  static const std::string LOGBOOK_NO_TITLE;       
  
  //! default string when no author given
  static const std::string LOGBOOK_NO_AUTHOR;     
  
  //! default string when no file given
  static const std::string LOGBOOK_NO_FILE;       
  
  //! default string when no directory given
  static const std::string LOGBOOK_NO_DIRECTORY;   
  
  //! max number of entries in logbook (make child logbook if larger)
  static const unsigned int MAX_ENTRIES = 50;         
  
  //! HTML flags
  enum HtmlTag 
  {
    HTML_TITLE = 1<<0,
    HTML_AUTHOR = 1<<1,
    HTML_FILE = 1<<2,
    HTML_CREATION = 1<<3,
    HTML_MODIFICATION = 1<<4,
    HTML_BACKUP = 1<<5,
    HTML_DIRECTORY = 1<<6,
    HTML_COMMENTS = 1<< 7,
    HTML_TABLE = 1<<8,
    HTML_CONTENT = 1<<9,
    HTML_ALL_MASK = (1<<10)-1
  };

  //! constructor from file
  Logbook( const File& file = File("") );   
   
  //! destructor
  virtual ~Logbook( void ); 
  
  //! read from file
  /*! 
    reads all xml based objects in the input file and chlids, 
    if any [recursive]
  */
  bool read( void );

  //! writes all xml based objects in given|input file, if any [recursive]
  bool write( File file = File("") ); 

  //! synchronize logbook with remote
  /*!
    returns a map of duplicated entries.
    The first entry is local and can be safely deleted
    The second entry is the remote replacement
  */
  std::map<LogEntry*,LogEntry*> synchronize( const Logbook& logbook );
  
  //! retrieve Xml parsing errors [recursive]
  XmlError::List xmlErrors( void ) const;
  
  //! shortcut for logbook children list
  typedef std::list< Logbook* > LogbookList;
  
  //!@name recursive accessors
  //@{
  
  //! retrieves list of all child logbook [recursive]
  LogbookList children( void ) const;
  
  //! retrieves first not full child.
  Logbook* latestChild( void );
  
  //! retrieve all associated entries [recursive]
  BASE::KeySet<LogEntry> entries( void ) const;
  
  //! retrieve all associated attachments [recursive]
  BASE::KeySet<Attachment> attachments( void ) const;
  
  //! returns true if logbook is empty (no recursive entries found)
  bool empty( void ) const 
  { return entries().empty(); }
  
  //@}
  
  //! remove empty children logbooks from list [recursive]
  void removeEmptyChildren( void );
  
  //!@name attributes
  //@{
  
  //! creation TimeStamp
  TimeStamp creation( void ) const 
  { return creation_; }     
  
  //! creation TimeStamp
  void setCreation( const TimeStamp& stamp ) 
  { creation_ = stamp; }       
  
  //! modification TimeStamp
  TimeStamp modification( void ) const 
  { return modification_; } 
  
  //! modification TimeStamp
  void setModification( const TimeStamp& stamp );
  
  //! backup TimeStamp
  TimeStamp backup( void ) const 
  { return backup_; }       
  
  //! backup TimeStamp
  void setBackup( const TimeStamp& stamp )
  { backup_ = stamp; }         
  
  //! saved TimeStamp
  TimeStamp saved( void ) const 
  { return saved_; }         
  
  //! saved TimeStamp
  void setSaved( const TimeStamp& stamp )
  { saved_ = stamp; }         
  
  //! logbook filename
  const File& file( void ) const 
  { return file_; }         
  
  //! logbook filename
  void setFile( const File& file )
  { 
    file_ = file; 
    saved_ = File( file_ ).lastModified();
  }
  
  //! parent logbook filename
  const File& parentFile( void ) const 
  { return parent_file_; }         
  
  //! parent logbook filename
  void setParentFile( const std::string& file )
  { parent_file_ = file; }     
  
  //! logbook title
  std::string title( void ) const 
  { return title_; }         
  
  //! logbook title. Returns true if changed.
  bool setTitle( const std::string& title )     
  { 
    if( title_ == title ) return false;
    title_ = title; 
    return true;
  }
  
  //! logbook last author
  std::string author( void ) const 
  { return author_; }       
  
  //! logbook author. Returns true if changed.
  bool setAuthor( const std::string& author )   
  { 
    if( author_ == author ) return false;
    author_ = author; 
    return true;
  } 
  
  //! retrieves attachment comments
  const std::string& comments( void ) const 
  { return comments_; }        
  
  //! appends string to attachment comments. Returns true if changed.
  bool setComments( const std::string& comments ) 
  {   
    if( comments == comments_ ) return false;
    comments_ = comments ;
    return true;
  }  
  
  //! logbook directory
  const File& directory( void ) const 
  { return directory_; }     
  
  //! checks if logbook directory is set, exists and is a directory
  bool checkDirectory( void ) const   
  { return File( directory_ ).isDirectory(); }
  
  //! logbook directory. Returns true if changed.
  bool setDirectory( const File& directory ) 
  { 
    if( directory_ == directory ) return false;
    directory_ = directory; 
    return true;
  }   
  
  /*! \brief 
    number of entries in logbook as read from xml
    it is not supposed to be synchronized with current list of entries
  */
  const int& xmlEntries( void ) const
  { return xml_entries_; }
  
  /*! \brief 
    number of entries in logbook as read from xml
    it is not supposed to be synchronized with current list of entries
    Returns true if changed.
  */
  bool setXmlEntries( const int& value )
  { 
    if( xml_entries_ == value ) return false;
    xml_entries_ = value ; 
    return true;
  }
  
  /*! \brief 
    number of children in logbook as read from xml
    it is not supposed to be synchronized with current list of children
  */
  const int& xmlChildren( void ) const
  { return xml_children_; }   
  
  /*! \brief 
    number of children in logbook as read from xml
    it is not supposed to be synchronized with current list of children.
    Returns true if changed.   
  */
  bool setXmlChildren( const int& value )
  { 
    if( xml_children_ == value ) return false;
    xml_children_ = value; 
    return true;
  }   
  
  //@}
  
  //! true if last backup is too old
  bool needsBackup( void ) const;     
  
  //! generate tagged backup filename
  std::string backupFilename( void ) const;   
  
  //! html
  QDomElement htmlElement( QDomDocument& parent, const unsigned int& mask = HTML_ALL_MASK ) const;
    
  /*! 
    \brief checks if logbook/children files has been modified
    by another application. Returns list of modified files. [recursive]
  */
  std::list<File> checkFiles( void );
  
  //! sets logbook modified value
  void setModified( const bool& value );
  
  //! sets logbook and children modified value [recursive]
  void setModifiedRecursive( bool value );
  
  //! tells if logbook or children has been modified since last call [recursive]
  bool modified( void ) const; 
  
  //!@name sort
  //@{
  
  //! sort method enumeration
  enum SortMethod
  { 
    //! sort LogEntry objects according to creation time
    SORT_CREATION,     
      
    //! sort LogEntry objects according to last modification time
    SORT_MODIFICATION,
      
    //! sort LogEntry objects according to title 
    SORT_TITLE, 
    
    //! sort LogEntry objects according to keyword
    SORT_KEYWORD,
    
    //! sort LogEntry objects according to author
    SORT_AUTHOR
  };
  
  //! Sort entries depending on the sort method
  class EntryLessFTor
  {
    
    public:
    
    //! constructor
    EntryLessFTor( const Logbook::SortMethod& sort_method, const int& order = 0 ):
      sort_method_( sort_method ),
      order_(order)
    {}
      
    //! sort operator
    bool operator()( LogEntry* first, LogEntry* second ) const;
      
    private:
      
    //! sort method (defined a constructor)
    Logbook::SortMethod sort_method_; 
    
    //! order
    int order_;
    
  };
  
  /*! 
  changes sort method associated to oldest parent
  returns true if changed
  */
  bool setSortMethod( const SortMethod& sort_method )
  {
    bool changed = ( sortMethod() != sort_method );
    if( changed ) {
      sort_method_ = sort_method;
      setModified( true );
    }
    return changed;
  }
  
  //! retrieves current sort method associated to oldest parent
  SortMethod sortMethod( void ) 
  { return sort_method_; }

  //! sort order
  bool setSortOrder( const int& order )
  {
    bool changed = (sortOrder() != order );
    if( changed )
    {
      sort_order_ = order;
      setModified( true );
    }
    return changed;
  }
  
  //! sort order
  const int& sortOrder( void ) const
  { return sort_order_; }
  
  //@}
  
  signals:
  
  //! message emission for logbook status during reading/writting
  void messageAvailable( const QString& message );
  
  private:
  
  //! generate tagged backup filename
  File _childFilename( const File& file, const int& child_number ) const;   
  
  //! list of pointers to logbook children
  LogbookList children_; 
  
  //! true if at least one logbook entry have been modified/added/deleted until last save
  bool modified_;       
  
  //! file from which the log book entries are read
  File file_;       
  
  //! file of parent logbook, if any
  File parent_file_;
  
  //! directory where the attached files are read/saved 
  File directory_;   
  
  //! title of the log book
  std::string title_;       
  
  //! last user name who had access to the logbook
  std::string author_;     
  
  //! comments
  std::string comments_;
  
  //! logbook creation time
  TimeStamp creation_;     
  
  //! logbook last modification time
  TimeStamp modification_; 
  
  //! logbook last backup time
  TimeStamp backup_;       
  
  //! logbook last save time
  TimeStamp saved_;         
  
  //! method used for LogEntry sort
  SortMethod sort_method_;             
  
  //! sort order
  int sort_order_;
  
  //! number of entries in logbook as read from xml
  /*!  \brief 
  number of entries in logbook as read from xml
  it is not supposed to be synchronized with current list of entries
  */
  int xml_entries_;     
  
  //! number of children in logbook as read from xml
  /*!  
  number of children in logbook as read from xml
  it is not supposed to be synchronized with current list of children
  */
  int xml_children_;
  
  //! error when parsing xml file
  XmlError error_;
  
};

#endif
