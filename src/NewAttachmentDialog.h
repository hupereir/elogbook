#ifndef NewAttachmentDialog_h
#define NewAttachmentDialog_h

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
   \file NewAttachmentDialog.h
   \brief new attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QComboBox>

#include "Attachment.h"
#include "AttachmentType.h"
#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "TextEditor.h"
#include "File.h"

//! new attachment popup dialog
class NewAttachmentDialog: public CustomDialog
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:
      
  // constructor
  NewAttachmentDialog( QWidget* parent );
  
  //! destructor
  virtual ~NewAttachmentDialog( void )
  {}

  //! filename
  void setFile( const File& );
  
  //! filename
  File file( void ) const;
  
  //! destination directory
  void setDestinationDirectory( const File& );
  
  //! destination directory
  File destinationDirectory( void ) const;
  
  //! attachment type
  void setType( const AttachmentType& );
  
  //! attachment type
  AttachmentType type( void ) const;
  
  //! Action
  void setAction( const Attachment::Command& );
  
  //! Action
  Attachment::Command action( void ) const;

  //! comments
  void setComments( const QString& comments );
  
  //! comments
  QString comments( void ) const;
  
  private slots:
      
  //! attachment type changed
  void _attachmentTypeChanged( int index );
    
  private:
  
  //! filename browsed line editor
  BrowsedLineEditor *file_line_edit_;  
      
  //! destination directory vbox
  QLayout* directory_layout_;
  
  //! destination directory browsed line edti
  BrowsedLineEditor *dest_dir_line_edit_;
  
  //! file type combo box
  QComboBox *file_type_combo_box_;  
        
  //! action combo box
  QComboBox *action_combo_box_;  
  
  //! comments
  TextEditor *comments_text_edit_;
  
      
};

#endif
