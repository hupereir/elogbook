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

#ifndef AttachmentWindow_h
#define AttachmentWindow_h

/*!
   \file    AttachmentWindow.h
   \brief   popup window to list/edit all attachments 
            independantly from entries
   \author  Hugo Pereira
   \version $Revision$
   \date    $Date$
*/

#include <QEvent>


#include "CustomDialog.h"
#include "Counter.h"
#include "AttachmentFrame.h"
#include "Debug.h"

class Attachment;

/*!
  \class  AttachmentWindow
   \brief popup window to list/edit all attachments independantly from entries
*/ 

class AttachmentWindow: public CustomDialog
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:

  //! creator 
  AttachmentWindow( QWidget* parent = 0 );
    
  //! display widget
  void show( void );

  //! retrieve associated List
  AttachmentFrame& frame()
  { 
    assert( frame_ );
    return *frame_; 
  }
    
  //! uniconify window
  QAction& uniconifyAction( void )
  { return *uniconify_action_; }

  public slots:
  
  //! uniconify
  void uniconify( void );
  
  protected:
  
  //! overloaded enter event handler
  void enterEvent( QEvent *event );

  protected slots:

  //! display entry associated to selected attachment when selection changes
  void _displayEntry( Attachment& );

  private:
  
  //! associated attachment list
  AttachmentFrame* frame_;

  //! uniconify action
  QAction* uniconify_action_;

};

#endif
 
