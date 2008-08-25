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
  \file  AttachmentFrame.h
  \brief  handles attachment list
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QWidget>
#include <string>
#include <set>

#include "Attachment.h"
#include "AttachmentModel.h"
#include "Debug.h"
#include "Key.h"

class TreeView;

/*!
  \class  AttachmentFrame
  \brief  handles attachment list
*/ 
class AttachmentFrame: public QWidget, public BASE::Key
{ 

  //! Qt meta object declaration
  Q_OBJECT

  public:
  
  //! constructor
  AttachmentFrame( QWidget *parent, bool read_only );
  
  //! clear
  void clear( void )
  { 
    // clear model and associations
    _model().clear(); 
    BASE::Key::clearAssociations<Attachment>();
  }

  //! add attachment to the list
  void add( Attachment& attachment )
  {
    AttachmentModel::List attachments;
    attachments.push_back( &attachment );
    add( attachments );
  }
  
  //! add attachments to list
  void add( const AttachmentModel::List& attachments );
  
  //! update attachment in the list
  void update( Attachment& attachment );
  
  //! select attachment in the list
  void select( Attachment& attachment );
 
  //! remove attachment from list
  void remove( Attachment& attachment )
  { _model().remove( &attachment ); }
  
   //! change read only status
  void setReadOnly( bool value )
  { 
    read_only_ = value; 
    _updateActions();
  }
   
  //!@name actions
  //@{
  
  //! visibility action
  QAction& visibilityAction( void ) const
  { return *visibility_action_; }
  
  //! new attachment action
  QAction& newAttachmentAction( void ) const
  { return *new_attachment_action_; }
     
  //! view attachment action
  QAction& openAttachmentAction( void ) const
  { return *open_attachment_action_; }
  
  //! edit attachment action
  QAction& editAttachmentAction( void ) const
  { return *edit_attachment_action_; }
  
  //! delete attachment action
  QAction& deleteAttachmentAction( void ) const
  { return *delete_attachment_action_; }
  
  //@}
  
  signals:
  
  //! emitted when an item is selected in list
  void _itemSelected( Attachment& );
  
  private slots:
      
  //! update context menu
  void _updateActions( void );

  //! create new attachment 
  void _new( void );
 
  //! display current attachment 
  void _open( void );
  
  //! edit current attachment
  void _edit( void );
 
  //! delete current attachment
  void _delete( void );

  //!@name selections
  //@{

  //! restore selection
  void _storeSelection( void );
  
  //! store selection
  void _restoreSelection( void );

  //@}

  private:
  
  //! model
  AttachmentModel& _model( void )
  { return model_; }
  
  //! list
  TreeView& _list( void ) const
  { 
    assert( list_ );
    return *list_;
  }
  
  //! install actions
  void _installActions( void );
  
  //! if true, listbox is read only
  bool read_only_;
  
  //!@name actions
  //@{
  
  //! visibility action
  QAction* visibility_action_;
  
  //! new attachment
  QAction* new_attachment_action_;
  
  //! view attachment
  QAction* open_attachment_action_;
    
  //! edit attachment
  QAction* edit_attachment_action_;
  
  //! delete attachment
  QAction* delete_attachment_action_;

  //@}
  
  //! model
  AttachmentModel model_;
  
  //! list
  TreeView* list_;
  
};

#endif
