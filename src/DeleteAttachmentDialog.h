#ifndef DeleteAttachmentDialog_h
#define DeleteAttachmentDialog_h

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
   \file DeleteAttachmentDialog.h
   \brief Delete attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QRadioButton>
#include <string>

#include "Attachment.h"
#include "CustomDialog.h"

//! delete attachment popup dialog
class DeleteAttachmentDialog: public CustomDialog
{
  
  public:
      
  // constructor
  DeleteAttachmentDialog( QWidget* parent, const Attachment& attachment );
  
  //! destructor
  virtual ~DeleteAttachmentDialog( void )
  {}
  
  //! action
  enum Action
  {
    //! delete file from disk
    FROM_DISK,
        
    //! delete file from logbook
    FROM_LOGBOOK

  };
  
  //! get action
  Action action( void ) const;
        
  private:

  //! open with radio button
  QRadioButton* from_disk_radio_button_;
  
  //! save as radio button
  QRadioButton* from_logbook_radio_button_;

              
};

#endif
