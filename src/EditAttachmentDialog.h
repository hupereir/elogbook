#ifndef EditAttachmentDialog_h
#define EditAttachmentDialog_h

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
   \file EditAttachmentDialog.h
   \brief Edit attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QComboBox>
#include <string>

#include "Attachment.h"
#include "AttachmentType.h"
#include "CustomTextEdit.h"
#include "CustomDialog.h"

//! edit attachment popup dialog
class EditAttachmentDialog: public CustomDialog
{
  
  public:
      
  //! constructor
  EditAttachmentDialog( QWidget* parent, const Attachment& attachment );
  
  //! destructor
  virtual ~EditAttachmentDialog( void )
  {}
  
  //! attachment type
  AttachmentType type( void ) const;
  
  //! get comments
  std::string comments( void ) const;    
        
  private:
        
  //! file type combo box
  QComboBox *file_type_combo_box_;  
  
  //! comments editor
  CustomTextEdit *comments_text_edit_;
        
};

#endif
