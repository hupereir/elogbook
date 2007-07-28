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

#include "BrowsedLineEdit.h"
#include "CustomDialog.h"
#include "CustomLineEdit.h"
#include "CustomTextEdit.h"
#include "File.h"
#include "QtUtil.h"

//! new attachment popup dialog
class NewLogbookDialog: public CustomDialog
{
  
  public:
      
  // constructor
  NewLogbookDialog( QWidget* parent );
  
  //! destructor
  virtual ~NewLogbookDialog( void )
  {}

  //! title
  void setTitle( const std::string& title )
  {
    title_->setText( title.c_str() );
    QtUtil::expand( title_ );
  }
  
  //! title
  std::string title( void ) const
  { return qPrintable( title_->text() ); }

  //! author
  void setAuthor( const std::string& author )
  {
    author_->setText( author.c_str() );
    QtUtil::expand( author_ );
  }
  
  //! filename
  std::string author( void ) const
  { return qPrintable( author_->text() ); }
  
  //! filename
  void setFile( const File& file )
  {
    file_->setFile( file );
    QtUtil::expand( &file_->editor() );
  }
  
  //! filename
  File file( void ) const
  { return File( qPrintable( file_->editor().text() ) ).expand(); }
  
  //! attachment directory
  void setAttachmentDirectory( const File& file )
  {
    attachment_directory_->setFile( file );
    QtUtil::expand( &attachment_directory_->editor() );
  }
  
  //! attachment directory
  File attachmentDirectory( void ) const
  { return File( qPrintable( attachment_directory_->editor().text() ) ).expand(); }
  
  //! comments
  void setComments( const std::string& comments )
  { comments_->setPlainText( comments.c_str() ); }
  
  //! comments
  std::string comments( void ) const
  { return qPrintable( comments_->toPlainText() ); }
      
  private:
  
  //! title line edit
  CustomLineEdit *title_;
  
  //! author line edit
  CustomLineEdit *author_;
            
  //! filename browsed line editor
  BrowsedLineEdit *file_;  
  
  //! destination directory browsed line edti
  BrowsedLineEdit *attachment_directory_;
  
  //! comments
  CustomTextEdit *comments_;
  
};

#endif
