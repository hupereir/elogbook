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

#ifndef AttachmentFrame_h
#define AttachmentFrame_h

/*!
   \file    AttachmentFrame.h
   \brief   popup window to list/edit all attachments 
            independantly from entries
   \author  Hugo Pereira
   \version $Revision$
   \date    $Date$
*/

#include <QEvent>
#include <QWidget>
#include <string>

#include "Counter.h"
#include "AttachmentList.h"
#include "Debug.h"

class Attachment;

/*!
  \class  AttachmentFrame
   \brief popup window to list/edit all attachments independantly from entries
*/ 

class AttachmentFrame: public QWidget, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:

  //! creator 
  AttachmentFrame( QWidget* parent );
    
  //! display widget
  void show( void );

  //! retrieve associated List
  AttachmentList& list()
  { 
    Exception::checkPointer( list_, DESCRIPTION("list_ not initialized") );
    return *list_; 
  }
    
  //! uniconify window
  QAction* uniconifyAction( void )
  { return uniconify_action_; }

  public slots:
  
  //! update configuration
  void updateConfiguration( void );
  
  //! save configuration
  void saveConfiguration( void );
  
  protected slots:
  
  //! uniconify window
  void _uniconify( void );

  protected:
  
  //! overloaded enter event handler
  void enterEvent( QEvent *event );


  private slots:

  //! display entry associated to selected attachment when selection changes
  void _displayEntry( QTreeWidgetItem*, QTreeWidgetItem* );

  private:
  
  //! associated attachment list
  AttachmentList* list_;

  //! uniconify action
  QAction* uniconify_action_;

};

#endif
 
