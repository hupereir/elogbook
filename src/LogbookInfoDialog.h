#ifndef LogbookInfoDialog_h
#define LogbookInfoDialog_h
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
  \file LogbookInfoDialog.h
  \brief  logbook informations
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "BrowsedLineEdit.h"
#include "CustomDialog.h"
#include "CustomLineEdit.h"
#include "CustomTextEdit.h"

class Logbook;

class LogbookInfoDialog: public CustomDialog
{
  
  public:
      
  //! constructor
  LogbookInfoDialog( QWidget* parent, Logbook* logbook  );
  
  //! title
  std::string title( void ) const
  { return qPrintable( title_->text() ); }
  
  //! author
  std::string author( void ) const
  { return qPrintable( author_->text() ); }
  
  //! attachment directory
  File AttachmentDirectory( void ) const
  { return File( qPrintable( attachment_directory_->editor().text() ) ); }
  
  //! comments
  std::string comments( void ) const
  { return qPrintable( comments_->toPlainText() ); }
  
  private:
  
  //! title line edit
  CustomLineEdit* title_;
  
  //! author
  CustomLineEdit* author_;
  
  //! attachment directory
  BrowsedLineEdit* attachment_directory_;

  //! comments
  CustomTextEdit* comments_;
    
};

#endif
