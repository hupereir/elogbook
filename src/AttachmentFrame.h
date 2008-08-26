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
#include "ValidFileThread.h"

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

  //! default size
  void setDefaultHeight( const int& );
    
  //! default height
  const int& defaultHeight( void ) const
  { return default_height_; }
    
  //! size
  QSize sizeHint( void ) const;  

  //! list
  bool hasList( void ) const
  { return (bool) list_; }
  
  //! list
  TreeView& list( void ) const
  { 
    assert( list_ );
    return *list_;
  }
  

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
   
  //! read only state
  const bool& readOnly( void ) const
  { return read_only_; }
  
  //!@name actions
  //@{
  
  //! visibility action
  QAction& visibilityAction( void ) const
  { return *visibility_action_; }
  
  //! new attachment action
  QAction& newAction( void ) const
  { return *new_action_; }
     
  //! view attachment action
  QAction& openAction( void ) const
  { return *open_action_; }
  
  //! edit attachment action
  QAction& editAction( void ) const
  { return *edit_action_; }
  
  //! delete attachment action
  QAction& deleteAction( void ) const
  { return *delete_action_; }
  
  //! clean action
  QAction& cleanAction( void ) const
  { return *clean_action_; }
  
  //@}
  
  signals:
  
  //! emitted when an item is selected in list
  void attachmentSelected( Attachment& );
  
  protected:
  
  //! enter event
  virtual void enterEvent( QEvent* );

  //! custom event, used to retrieve file validity check event
  void customEvent( QEvent* );  
  
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

  //! clean 
  void _clean( void );
  
  //!@name selections
  //@{

  //! current item changed
  void _itemSelected( const QModelIndex& );
  
  //! restore selection
  void _storeSelection( void );
  
  //! store selection
  void _restoreSelection( void );

  //@}

  private:

  //! install actions
  void _installActions( void );
  
  //! model
  AttachmentModel& _model( void )
  { return model_; }
  
  //! if true, listbox is read only
  bool read_only_;

  //! default height;
  int default_height_;  

  //!@name actions
  //@{
  
  //! visibility action
  QAction* visibility_action_;
  
  //! new attachment
  QAction* new_action_;
  
  //! view attachment
  QAction* open_action_;
    
  //! edit attachment
  QAction* edit_action_;
  
  //! delete attachment
  QAction* delete_action_;

  //! clean action
  QAction* clean_action_;
  
  //@}
  
  //! model
  AttachmentModel model_;
  
  //! list
  TreeView* list_;

  // valid file thread
  ValidFileThread thread_;

};

#endif
