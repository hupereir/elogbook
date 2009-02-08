#ifndef OpenAttachmentDialog_h
#define OpenAttachmentDialog_h

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
   \file OpenAttachmentDialog.h
   \brief open attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <qradiobutton.h>


#include "Attachment.h"
#include "BrowsedLineEditor.h"
#include "CustomDialog.h"

//! open attachment popup dialog
class OpenAttachmentDialog: public CustomDialog
{
  
  public:
      
  //! constructor
  OpenAttachmentDialog( QWidget* parent, const Attachment& attachment );
  
  //! destructor
  virtual ~OpenAttachmentDialog( void )
  {}
  
  //! get command
  QString command( void ) const;
  
  //! action
  enum Action
  {
    //! open attachment with command
    OPEN,
        
    //! save attachment locally
    SAVE_AS

  };
  
  //! get action
  Action action( void ) const;
        
  private:
  
  //! command browsed line editor
  BrowsedLineEditor *command_line_edit_;  
  
  //! open with radio button
  QRadioButton* open_radio_button_;
  
  //! save as radio button
  QRadioButton* save_radio_button_;
  
};

#endif
