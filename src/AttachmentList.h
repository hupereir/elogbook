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

#ifndef AttachmentList_h
#define AttachmentList_h

/*!
  \file  AttachmentList.h
  \brief  customized list box to handle Attachment
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <string>
#include <set>

#include "AttachmentType.h"
#include "Attachment.h"
#include "Counter.h"
#include "CustomListView.h"
#include "Debug.h"
#include "Exception.h"
#include "Key.h"

/*!
  \class  AttachmentList
  \brief  customized ListBox to handle Attachment
*/ 
class AttachmentList: public CustomListView, public BASE::Key
{ 

  //! Qt meta object declaration
  Q_OBJECT

  public:
  
  //! constructor
  AttachmentList( QWidget *parent, bool read_only );

  //! number of columns
  enum { n_columns = 4 };

  //! column type enumeration
  enum ColumnTypes {
    FILE, 
    TYPE,
    SIZE,
    MODIFICATION
  };
    
  //! column titles
  static char* column_titles_[ n_columns ];
  
  //! add attachment to the list
  void add( Attachment* attachment );
  
  //! update attachment in the list
  void update( Attachment* attachment );
  
  //! select attachment in the list
  void selectAttachment( Attachment* attachment );
 
   //! change read only status
  void setReadOnly( bool value )
  { 
    read_only_ = value; 
    _updateActions();
  }
       
  //! local list item for attachments
  class Item: public CustomListView::Item, public BASE::Key 
  {
    public: 
    
    //! constructor
    Item( AttachmentList *list ):
        CustomListView::Item( list )
    {}
    
    //! update current attachment
    void update( void );
    
    //! retrieve attachment
    Attachment* attachment( void ) const
    {
      BASE::KeySet<Attachment> attachments( this );
      Exception::check( attachments.size()==1, DESCRIPTION( "invalid association to attachment") );
      return *attachments.begin();
    }
    
    //! order operator
    virtual bool operator<( const QTreeWidgetItem &other ) const;
    
  };
  
  //! resize attachment columns
  void resizeColumns( void );
  
  //! new attachment action
  QAction& newAttachmentAction( void )
  { return *new_attachment_action_; }
     
  private slots:
      
  //! display current attachment 
  void _newAttachment( void );
 
  //! update context menu
  void _updateActions( void );
  
  //! display current attachment 
  void _open( void );
  
  //! edit current attachment
  void _edit( void );
 
  //! delete current attachment
  void _delete( void );

  private:
  
  //! if true, listbox is read only
  bool read_only_;
  
  //!@name actions
  //@{
  
  //! new attachment
  QAction* new_attachment_action_;
  
  //! view attachment
  QAction* view_attachment_action_;
    
  //! edit attachment
  QAction* edit_attachment_action_;
  
  //! delete attachment
  QAction* delete_attachment_action_;

  //@}
  
};

#endif

