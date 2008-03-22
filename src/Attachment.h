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

#ifndef Attachment_h
#define Attachment_h

/*!
  \file    Attachment.h
  \brief  Attached file object
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <iostream>
#include <string>
#include <list>
#include <qdom.h>

#include "AttachmentType.h"
#include "Counter.h"
#include "File.h"
#include "Key.h"
#include "TimeStamp.h"
#include "XmlUtil.h"

class LogEntry;

/*!
  \class Attachment
  \brief Attached file object
*/

class Attachment: public Counter, public BASE::Key
{
  public:

  //! default string when no file given
  static const std::string NO_FILE;      
  
  //! default string when no comments given
  static const std::string NO_COMMENTS;  


  //! contructor
  Attachment( 
    const std::string orig = "", 
    const AttachmentType& type = AttachmentType::UNKNOWN 
  ):
    Counter( "Attachment" ),
    type_( AttachmentType::UNKNOWN ),
    source_file_( orig ),
    file_( NO_FILE ),
    comments_( NO_COMMENTS )
  { setType( type ); }

  //! creator from DomElement
  Attachment( const QDomElement& element );
  
  //! destructor
  ~Attachment( void ) 
  {}
  
  //! domElement
  QDomElement domElement( QDomDocument& parent ) const;
  
  /*!\fn bool operator < (const Attachment& attachment ) const
    \brief inferior to operator, based on Attachment Short name lexicographic order
    \param attachment the attachment to which this is to be compared
  */
  bool operator < (const Attachment& attachment ) const;
  
  /*!\fn bool operator == (const Attachment& attachment ) const
    \brief equal to operator, based on Attachment Full name
    \param attachment the attachment to which this is to be compared
  */
  bool operator == (const Attachment& attachment ) const 
  { return file() == attachment.file(); }  
  
  //! used to check attachment filenames
  class SameFileFTor
  {
    public:
        
    //! constructor
    SameFileFTor( Attachment* attachment ):
        attachment_( attachment )
    {}
    
    //! predicate
    bool operator()(Attachment* attachment )
    { return *attachment == *attachment_; }
  
    private:
        
    //! reference attachment
    Attachment* attachment_;  
  
  };
  
  //! retrieves original file name
  bool isValid( void ) const 
  { return is_valid_; }    
  
  //! retrieves local file size
  double size( void ) const 
  { return size_; }    
  
  //! retrieves local file size
  std::string sizeString( void ) const 
  { return size_str_; }    

  //! retrieves associated entry
  LogEntry* entry() const;
  
  //! retrieves file last modification
  TimeStamp modification( void ) const
  { return modification_; }
  
  //! retrieves original file name
  const File& sourceFile( void ) const 
  { return source_file_; }    
  
  //! retrieves attachment file name
  const File& file( void ) const 
  { return file_; }    
  
  //! retrieves attachment short file name
  File shortFile( void ) const;    
    
  //! retrieves attachment comments
  const std::string& comments( void ) const 
  { return comments_; }        
  
  //! appends string to attachment comments
  void setComments( const std::string& buf ) 
  { comments_ = buf ;}  
  
  //! retrieves attachment type
  const AttachmentType& type( void ) const 
  { return type_; }              
  
  //! changes attachment type
  void setType( const AttachmentType& type );
  
  //! write html formated to stream 
  void htmlElement( QDomElement& parent, QDomDocument& document ) const;  
  
  //! command enum to tell who original file should be transformed into attached file
  enum Command { 
    COPY, 
    LINK, 
    COPY_FORCED, 
    LINK_FORCED, 
    COPY_VERSION, 
    LINK_VERSION, 
    DO_NOTHING };
  
  //! error codes output enum for ProcessCopy
  enum ErrorCode { 
    SUCCESS, 
    SOURCE_NOT_FOUND, 
    DEST_NOT_FOUND, 
    SOURCE_IS_DIR, 
    DEST_EXIST };
      
  /*! \fn ErrorCode copy( const Attachment::Command& command, const std::string& destdir ) 
    \brief ErrorCode convert original file into attached file. Returns true in case of success
    \param command tells how the original file is to be converted into attached file. Is one of the following:
      Attachment::COPY use command cp, if the attached file is not present
      Attachment::LINK use ln -s, if the attached file is not present
      Attachment::OVERWRITE use command cp, overwrite attached file if present
      Attachment::COPY_VERSION, modify filename to resolve copy, then use cp
      Attachment::LINK_VERSION, modify filename to resolve copy, then use ln -s
      Attachment::DO_NOTHING, just stores the attached file name, but do nothing
    \param destdir destination directory
  */
  ErrorCode copy( const Attachment::Command& command, const std::string& destdir );
  
  private: 
  
  //! set original attachment file name
  void _setSourceFile( const File& file )
  { source_file_ = file; }

  //! set attachment file name
  void _setFile( const File& file ); 
  
  //! attached file type
  AttachmentType type_;  
  
  //! attached file name
  File source_file_;      
  
  //! attached file name
  File file_;      
  
  //! comments
  std::string comments_;  
  
  //! file size (0 if not valid | URL )
  double size_;        
  
  //! corresponding size_string
  std::string size_str_;    
  
  //! file last modification timestamp 
  TimeStamp modification_;  
  
  //! true for URL or if file exist
  bool is_valid_;    
  
};

#endif
