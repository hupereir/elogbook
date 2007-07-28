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
  void addAttachment( Attachment* attachment );
  
  //! update attachment in the list
  void updateAttachment( Attachment* attachment );
  
  //! select attachment in the list
  void selectAttachment( Attachment* attachment );
 
   //! change read only status
  void setReadOnly( bool value )
  { read_only_ = value; }
   
  //! overloaded drag event handler
  // void dragEnterEvent( QDragEnterEvent *event );
  
  //! overloaded drop event handler
  // void dropEvent( QDropEvent *event );
    
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
      Exception::assert( attachments.size()==1, DESCRIPTION( "invalid association to attachment") );
      return *attachments.begin();
    }
    
    //! order operator
    virtual bool operator<( const QTreeWidgetItem &other ) const;
    
  };
  
  public slots:
      
  //! display current attachment 
  void newAttachment( 
    const std::string& file = "", 
    const AttachmentType& type = AttachmentType::UNKNOWN );
  
  private slots:
  
  //! update context menu
  void _updateMenu( void );
  
  //! display current attachment 
  void _openAttachment( QTreeWidgetItem* item = 0 );
  
  //! edit current attachment
  void _editAttachment( void );
 
  //! delete current attachment
  void _deleteAttachment( void );

  private:
  
  //! if true, listbox is read only
  bool read_only_;
  
  //! new attachment
  QAction* new_attachment_;
  
  //! view attachment
  QAction* view_attachment_;
    
  //! edit attachment
  QAction* edit_attachment_;
  
  //! delete attachment
  QAction* delete_attachment_;
      
};

#endif

