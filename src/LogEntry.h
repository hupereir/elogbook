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

#ifndef LogEntry_h
#define LogEntry_h

/*!
  \file LogEntry.h
  \brief log file entry manipulation object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDomElement>
#include <QDomDocument>

#include <vector>
#include <string>

#include "Counter.h"


#include "Key.h"
#include "Str.h"
#include "TextFormatBlock.h"
#include "TimeStamp.h"
#include "XmlOptions.h"

class Logbook;

/*!
  \class  LogEntry
  \brief  log file entry manipulation object
*/
class LogEntry:public Counter, public BASE::Key
{
  
  public:

  //! empty creator
  LogEntry( void );
  
  //! constructor from DOM
  LogEntry( const QDomElement& element );
    
  //! destructor
  ~LogEntry( void );

  //! retrieve DomElement
  QDomElement domElement( QDomDocument& parent ) const;
  
  //! return a new entry cloned from this
  LogEntry *clone( void ) const;

  //! HTML output configuration output
  enum HtmlTag 
  {
    HTML_TITLE = 1<<0,
    HTML_KEYWORD = 1<<1,
    HTML_CREATION = 1<<2,
    HTML_MODIFICATION = 1<<3,
    HTML_AUTHOR = 1<<4,
    HTML_TEXT = 1<<5,
    HTML_ATTACHMENT = 1<<6,
    HTML_HEADER_MASK = 
      HTML_TITLE | 
      HTML_KEYWORD | 
      HTML_CREATION | 
      HTML_MODIFICATION |
      HTML_AUTHOR,
    
    HTML_ALL_MASK = HTML_HEADER_MASK | HTML_TEXT
  }; 

  //! used to tag LogEntry drag from LogEntryList
  static const std::string DRAG;

  //! used when LogEntry title is not defined 
  static const std::string UNTITLED;    
  
  //! used when LogEntry keyword is not defined 
  static const std::string NO_KEYWORD;  
  
  //! used when LogEntry keyword is not defined 
  static const std::string NO_AUTHOR;  
  
  //! used when LogEntry keyword is not defined
  static const std::string NO_TEXT;      

  /*!
    format keyword: remove all trailing/leading space characters;
    replace empty string by NO_KEYWORD
    otherwise, uppercase first character.
  */
  static std::string formatKeyword( const std::string& );

  //! split keyword into a list of strings using "/" as the separator
  static std::vector<std::string> parseKeyword( const std::string& );
  
  //!@name attributes
  //@{
  
  //! set modification_ to _now_
  void modified( void );  
  
  //! creation TimeStamp
  void setCreation( const TimeStamp stamp )    
  { creation_ = stamp; }    

  //! creation TimeStamp
  TimeStamp creation() const 
  { return creation_; }        
  
  //! modification TimeStamp
  void setModification( const TimeStamp stamp ) 
  { modification_ = stamp; } 
  
  //! modification TimeStamp
  TimeStamp modification() const 
  { return modification_; }    
  
  //! LogEntry title
  void setTitle( const std::string& title )  
  { 
    title_ = title; 
    if( !title_.size() ) title_ = UNTITLED; 
  }    
  
  //! LogEntry title
  std::string title( void ) const 
  { return title_; }        
  
  //! LogEntry keyword
  void setKeyword( const std::string& keyword );  
  
  //! LogEntry keyword
  std::string keyword( void ) const 
  { return keyword_; }  
  
  //! LogEntry author
  void setAuthor( const std::string& author ) 
  { author_ = author; } 
  
  //! logbook last author
  std::string author( void ) const 
  { return author_; }  
  
  //! LogEntry color
  void setColor(  const std::string& color )  
  { color_ = color; }  
  
  //! LogEntry color
  const std::string& color( void ) const 
  { return color_; }    
  
  //! add TextFormatBlock
  void addFormat( const FORMAT::TextFormatBlock& format )
  { formats_.push_back( format ); }
  
  //! entry text format
  void setFormats( const FORMAT::TextFormatBlock::List& formats )
  { formats_ = formats; }
  
  //! entry text format
  const FORMAT::TextFormatBlock::List& formats( void ) const
  { return formats_; }
    
  //! clears LogEntry text
  void setText( const std::string& text ) 
  { text_ = text; }        
  
  //! LogEntry text
  const std::string& text( void ) const 
  { return text_; }    

  //! returns true if entry title matches buffer
  bool matchTitle( const std::string& buf ) 
  { return Str( buf ).isIn( title_, XmlOptions::get().get<bool>( "CASE_SENSITIVE" )); }
  
  //! returns true if entry keyword matches buffer
  bool matchKeyword( const std::string& buf ) 
  { return Str( buf ).isIn( keyword_, XmlOptions::get().get<bool>( "CASE_SENSITIVE" )); }

  //! returns true if entry text matches buffer
  bool matchText(  const std::string& buf ) 
  { return Str( buf ).isIn( text_, XmlOptions::get().get<bool>( "CASE_SENSITIVE" )); }

  //! returns true if entry text matches buffer
  bool matchColor(  const std::string& buf );
  
  //! returns true if any entry attachment file name matches buffer
  bool matchAttachment( const std::string& buf ); 
  
  //! write text in html format to stream 
  QDomElement htmlElement( QDomDocument& document, const unsigned int &mask = HTML_ALL_MASK );  
  
  //! write text in html format to stream 
  QDomElement htmlSummary( QDomDocument& document, const unsigned int& mask = HTML_HEADER_MASK ) const;
      
  //! set if entry is said visible by the find bar
  void setFindSelected( bool value )
  { find_selected_ = value; }
  
  //! set if entry is said vidible by the keyword list
  void setKeywordSelected( bool value )
  { keyword_selected_ = value; }
  
  //! returns true if entry is visible (i.e. selected by the find bar and keyword list)
  bool isSelected( void ) const
  { return find_selected_ && keyword_selected_; }
  
  //! returns true if entry is selected by the find bar
  bool isFindSelected( void ) const
  { return find_selected_; }
  
  //! returns true if entry is selected by the keyword list
  bool isKeywordSelected( void ) const
  { return keyword_selected_; }

  //! use to get last modified entry
  class LastModifiedFTor
  {
    
    public:
        
    //! returns true if first entry was modified after the second
    bool operator() ( const LogEntry* first, const LogEntry* second )
    { return second->modification() < first->modification(); }    
        
  };

  //! use to get first created entry
  class FirstCreatedFTor
  {
    
    public:
        
    //! returns true if first entry was modified after the second
    bool operator() ( const LogEntry* first, const LogEntry* second )
    { return first->creation() < second->creation(); }    
        
  };
  
  //! use to check if entries have same creation time
  class SameCreationFTor
  {
    public:
        
    //! constructor
    SameCreationFTor( const TimeStamp& stamp ):
        stamp_( stamp )
    {}
    
    //! predicate
    bool operator()( const LogEntry *entry ) const
    { return entry->creation() == stamp_; }
    
    private:
        
    //! predicted stamp
    TimeStamp stamp_;

  };
    
  //! use to check if entries have same creation and modification time
  class DuplicateFTor
  {
    public:
        
    //! constructor
    DuplicateFTor( LogEntry* entry ):
        entry_( entry )
    {}
    
    //! predicate
    bool operator()( const LogEntry *entry ) const
    { return entry->creation() == entry_->creation(); }
    
    private:
        
    //! predicte entry
    LogEntry *entry_;

  };

  /*! 
    used to check if LogEntry keyword matches a given keyword.
    A match is found when the LogEntry keyword starts with the reference keyword
    It is used to check for keywords which have no associated entries
  */
  class MatchKeywordFTor
  {
    public:
        
    //! constructor
    MatchKeywordFTor( const std::string& keyword ):
      keyword_( keyword )
    {}
    
    //! predicate
    bool operator() (const LogEntry* entry ) const
    { return entry->keyword().find( keyword_ ) == 0;}
    
    private:
        
    //! comparison keyword
    std::string keyword_;
    
  };
  
      
  private:
  
  //! initialize fields (default values)
  void _init( void );
        
  //! format text for html using TextFormat info
  void _htmlTextNode( 
    QDomElement& parent,
    QDomDocument& document
  ) const; 
      
  //! log entry creation time
  TimeStamp creation_;                      
  
  //! log entry last modification time
  TimeStamp modification_;                  
 
  //! log entry title
  std::string title_;  
  
  //! log entry keywords
  std::string keyword_;
  
  //! last user name who had access to the entry    
  std::string author_; 
  
  //! LogEntry text
  std::string text_;  
  
  //! LogEntry color
  std::string color_;  

  //! set to true if entry is said visible by the selection bar 
  bool find_selected_;
 
  //! set to true if entry is said visible by the keyword selection
  bool keyword_selected_;
  
  //! list of text formats
  FORMAT::TextFormatBlock::List formats_;
    
};


#endif
