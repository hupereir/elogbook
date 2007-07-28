#ifndef EditFrame_h
#define EditFrame_h

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

/*!
  \file EditFrame.h
  \brief log entry edition/creation singleton object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QMainWindow>
#include <string>
#include <set>
#include <map>
#include <list>

#include "AskForSaveDialog.h"
#include "AttachmentList.h"
#include "CustomMainWindow.h"
#include "Counter.h"
#include "Debug.h"
#include "Exception.h"
#include "Key.h"
#include "LogEntry.h"
#include "QtUtil.h"
#include "TextPosition.h"

class Attachment;
class CustomLineEdit;
class CustomTextEdit;
class CustomToolButton;
class SelectionFrame;
class StatusBar;

//! log entry edition/creation object
/*!
  Note: 
  though EditFrames are TopLevel widgets, they are not deleted at window closure
  to avoid crash when object is deleted when still within one of its methods.
  On the contrary, a close event hides the window, and the SelectionFrame will delete it
  because of that next time it is asked to create a new EditFrame, thus acting like a
  garbage collector
*/
  
class EditFrame: public CustomMainWindow, public Counter, public BASE::Key
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:
  
  //! creator 
  EditFrame( QWidget* parent, LogEntry* entry=0, bool read_only = true );
  
  //! destructor
  ~EditFrame( void );

//   //! shortcut to color pair
//   typedef std::pair< std::string, QIconSet > ColorPair;
//   
//   //! shortcut to color pair iterator
//   typedef std::map< std::string, QIconSet >::const_iterator ColorIterator;
  
  //! display all entries informations
  void displayEntry( LogEntry *entry = 0 );
  
  //! returns current entry
  LogEntry* entry( void ) const 
  { 
    BASE::KeySet<LogEntry> entries( this );
    Exception::assert( entries.size() <= 1, DESCRIPTION( "wrong association to LogEntry" ) ); 
    return( entries.size() ) ? *entries.begin():0;
  }  
  
  //! retrieve attachment list
  AttachmentList& attachmentList( void ) 
  {
    BASE::KeySet<AttachmentList> attachment_list( this );
    Exception::assert( attachment_list.size() == 1, DESCRIPTION( "wrong associateion to AttachmentList" ) );
    return **attachment_list.begin(); 
  }
  
  //! window title for modified entry
  static const std::string EDIT_TITLE_MODIFIED;  
  
  //! window title for read only editor
  static const std::string EDIT_TITLE_READONLY;  
    
  //! default window title
  static const std::string EDIT_TITLE;            
  
  //! get text editor
  CustomTextEdit& editor( void ) 
  { 
    Exception::assert( text_, DESCRIPTION( "text_ not initialized" ) );
    return *text_;
  }
  
  //! check if this editor is read_only or not
  const bool& isReadOnly( void ) const 
  { return read_only_; }
  
  //! set read_only state of the EditFrame
  void setReadOnly( const bool& value );
  
  //! check if current entry has been modified or not
  const bool& modified( void ) const 
  { return modified_; }
  
  //! check if current entry has been modified or not
  void setModified( const bool& value ) 
  { modified_ = value; }
  
  //! computes window title
  std::string windowTitle() const;
  
  //! change window title
  void updateWindowTitle()
  { setWindowTitle( windowTitle().c_str() ); }
      
  //! creates dialog to ask for LogEntry save. 
  AskForSaveDialog::ReturnCode askForSave( const bool & enable_cancel = true );
  
  //! update title Widget from current entry
  void displayTitle( void );    
  
  //! update color Widget from current entry
  // void displayColor( void );    
  
  public slots:  
  
  //! configuration
  void updateConfiguration( void );
  
  //! Save Current entry
  void saveEntry( bool update_selection = true );
  
  //! creates a new entry
  void newEntry( void );

  //! uniconify
  void uniconify( void )
  { QtUtil::uniconify( this ); }
  
  protected:
  
  //! close window event handler
  void closeEvent( QCloseEvent *event );
  
  //! enter event handler
  void enterEvent( QEvent *event );
    
  private slots:
  
  //! select previous entry
  void _previousEntry( void );
  
  //! select next entry
  void _nextEntry( void );
  
  //! show entry info
  void _entryInfo( void );
  
  //! Delete Current entry
  void _deleteEntry( void );
  
  //! check spelling of current entry
  void _spellCheck( void );

  //! undo in focused editor (text/title/keyword)
  // void _undo( void );
  
  //! redo in focused editor (text/title/keyword);
  // void _redo( void );
 
  //! opens a read_only EditFrame with same entry
  void _newWindow( void );    
  
  //! view current entry as HTML
  void _viewHtml( void );
 
  //! unlock read-only editors
  void _unlock( void );
  
  //! Set entry as modified, change window title
  void _modified( void );
  
  //! display cursor position
  void _displayCursorPosition( void )
  { _displayCursorPosition( text_->textPosition() ); }

  //! display cursor position
  void _displayCursorPosition( int old_position, int new_position )
  { _displayCursorPosition( TextPosition( 0, new_position ) ); }

  private:
  
  //! display cursor position
  void _displayCursorPosition( const TextPosition& position );
  
//   //! internal customized label to have correct background color
//   class CustomLabel : public QLabel
//   {
//     
//     public:
//     
//     //! constructor
//     CustomLabel( QWidget *parent ):
//       QLabel( parent, name.c_str() ),
//       color_( "" )
//       {}
//       
//     //! set color
//     void SetColor( const std::string& color )
//     { color_ = color; }
//     
//     protected:
//     
//     //! overloaded draw method
//     void drawContents( QPainter *painter );
//   
//     private:
//     
//     //! parent ColorSelectionFrame
//     std::string color_;
//   
//   };
  
  //! retrieve associated SelectionFrame
  SelectionFrame* _selectionFrame( void ) const; 
  
  //! update text Widget from current entry  
  void _displayText( void );    
  
  //! update attachment list Widget from current entry  
  void _displayAttachments( void );            

  //! if true, LogEntry associated to EditFrame cannot be modified
  bool read_only_;    
  
  //! list of buttons to disactivate in case of read-only
  std::list< QWidget* > read_only_widgets_;
  
  //! true if entry associated to EditFrame has been modified
  bool modified_;    
  
  //! text formating bar
  // FORMAT::TextFormat* text_format_;
    
  //! lock toolbutton
  CustomToolButton* lock_;
  
  //! LogEntry title Object
  CustomLineEdit *title_;          
  
  //! LogEntry attachment list bounding box
  // QVBox *attachment_box_;          
  
  //! LogEntry text Object
  CustomTextEdit *text_;            
    
  //! pointer to statusbar    
  StatusBar* statusbar_;            
  
};

#endif
