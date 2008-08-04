#ifndef NewLogbookDialog_h
#define NewLogbookDialog_h

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
   \file NewLogbookDialog.h
   \brief new logbook popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <string>

#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "ImprovedLineEditor.h"
#include "TextEditor.h"
#include "File.h"
#include "QtUtil.h"
#include "Debug.h"

//! new attachment popup dialog
class NewLogbookDialog: public CustomDialog
{
  
  public:
      
  //! constructor
  NewLogbookDialog( QWidget* parent );
  
  //! destructor
  virtual ~NewLogbookDialog( void )
  { Debug::Throw( "NewLogbookDialog::~NewLogbookDialog\n" ); }

  //! title
  void setTitle( const std::string& title )
  {
    Debug::Throw( "NewLogbookDialog::setTitle.\n" ); 
    title_->setText( title.c_str() );
    QtUtil::expand( title_ );
  }
  
  //! title
  std::string title( void ) const
  { 
    Debug::Throw( "NewLogbookDialog::title.\n" ); 
    return qPrintable( title_->text() ); 
  }

  //! author
  void setAuthor( const std::string& author )
  {
    Debug::Throw( "NewLogbookDialog::setAuthor.\n" ); 
    author_->setText( author.c_str() );
    QtUtil::expand( author_ );
  }
  
  //! filename
  std::string author( void ) const
  { 
    Debug::Throw( "NewLogbookDialog::author.\n" ); 
    return qPrintable( author_->text() ); 
  }
  
  //! filename
  void setFile( const File& file )
  {
    Debug::Throw( "NewLogbookDialog::setFile.\n" ); 
    file_->setFile( file );
    QtUtil::expand( &file_->editor() );
  }
  
  //! filename
  File file( void ) const
  { 
    Debug::Throw( "NewLogbookDialog::file.\n" ); 
    return File( qPrintable( file_->editor().text() ) ).expand(); 
  }
  
  //! attachment directory
  void setAttachmentDirectory( const File& file )
  {
    Debug::Throw( "NewLogbookDialog::setAttachmentDirectory.\n" ); 
    attachment_directory_->setFile( file );
    QtUtil::expand( &attachment_directory_->editor() );
  }
  
  //! attachment directory
  File attachmentDirectory( void ) const
  { 
    Debug::Throw( "NewLogbookDialog::attachmentDirectory.\n" ); 
    return File( qPrintable( attachment_directory_->editor().text() ) ).expand(); 
  }
  
  //! comments
  void setComments( const std::string& comments )
  { 
    Debug::Throw( "NewLogbookDialog::setComments.\n" ); 
    comments_->setPlainText( comments.c_str() ); 
  }
  
  //! comments
  std::string comments( void ) const
  {
    Debug::Throw( "NewLogbookDialog::comments.\n" ); 
    return qPrintable( comments_->toPlainText() ); 
  }
      
  private:
  
  //! title line edit
  LineEditor *title_;
  
  //! author line edit
  LineEditor *author_;
            
  //! filename browsed line editor
  BrowsedLineEditor *file_;  
  
  //! destination directory browsed line edti
  BrowsedLineEditor *attachment_directory_;
  
  //! comments
  TextEditor *comments_;
  
};

#endif
