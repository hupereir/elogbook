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

#include <QBasicTimer>
#include <QLayout>
#include <QToolButton>
#include <QResizeEvent>
#include <QSplitter>
#include <QTimerEvent>

#include <string>
#include <set>
#include <map>
#include <list>

#include "AskForSaveDialog.h"
#include "AttachmentList.h"
#include "LineEditor.h"
#include "TextEditor.h"
#include "CustomMainWindow.h"
#include "Counter.h"
#include "Debug.h"

#include "Config.h"
#include "Key.h"
#include "LogEntry.h"
#include "QtUtil.h"
#include "TextPosition.h"

class Attachment;
class FormatBar;
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
  EditFrame( QWidget* parent, bool read_only = true );

  //! destructor
  ~EditFrame( void );

  //! display all entries informations
  void displayEntry( LogEntry *entry = 0 );

  //! returns current entry
  LogEntry* entry( void ) const
  {
    BASE::KeySet<LogEntry> entries( this );
    assert( entries.size() <= 1 );
    return( entries.size() ) ? *entries.begin():0;
  }

  //! retrieve attachment list
  AttachmentList& attachmentList( void )
  {
    BASE::KeySet<AttachmentList> attachment_list( this );
    assert( attachment_list.size() == 1 );
    return **attachment_list.begin();
  }

  //! status bar
  StatusBar& statusBar( void )
  {
    assert( statusbar_ );
    return *statusbar_;
  }

  //! check if this editor is read_only or not
  const bool& isReadOnly( void ) const
  { return read_only_; }

  //! set read_only state of the EditFrame
  void setReadOnly( const bool& value );

  //! closed flag
  const bool& isClosed( void ) const
  { return closed_; }

  //! closed flag
  void setIsClosed( const bool& value )
  { closed_ = value; }

  //! check if current entry has been modified or not
  bool modified( void ) const
  { return title_->isModified() || _activeEditor().document()->isModified(); }

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
  void displayColor( void );

  //! check if current entry has been modified or not
  void setModified( const bool& value );

  //! used to count modified EditFrames
  class ModifiedFTor
  {
    public:

    //! predicate
    bool operator() (const EditFrame* frame )
    { return frame->modified() && !frame->isReadOnly() && !frame->isClosed(); }

  };

  //! used to count alive frames, that are not subject to delayed deletion
  class aliveFTor
  {
    public:

    //! predicate
    bool operator() (const EditFrame* frame )
    { return !frame->isClosed(); }

  };
  
  //!@name actions
  //@{

  //! new entry action
  QAction& newEntryAction( void ) const
  { return *new_entry_action_; }
  
  //! previous entry action
  QAction& previousEntryAction( void ) const
  { return *previous_entry_action_; }

  //! next entry action
  QAction& nextEntryAction( void ) const
  { return *next_entry_action_; }
  
  //! save
  QAction& saveAction( void ) const
  { return *save_action_; }

  #if WITH_ASPELL  
  //! check spelling of current entry
  QAction& spellCheckAction( void ) const
  { return *spellcheck_action_; }
  #endif
  
  //! entry information
  QAction& entryInfoAction( void ) const
  { return *entry_info_action_; }
  
  //! view html
  QAction& viewHtmlAction( void ) const
  { return *view_html_action_; }
  
  //! split view horizontal
  QAction& splitViewHorizontalAction( void ) const
  { return *split_view_horizontal_action_; }
  
  //! split view vertical
  QAction& splitViewVerticalAction( void ) const
  { return *split_view_vertical_action_; }
  
  //! split view vertical
  QAction& cloneWindowAction( void ) const
  { return *clone_window_action_; }

  //! close view
  QAction& closeAction( void ) const
  { return *close_action_; }
  
  //! uniconify
  QAction& uniconifyAction( void ) const
  { return *uniconify_action_; } 

  //@}

  protected:

  //! close window event handler
  void closeEvent( QCloseEvent *event );

  //! enter event handler
  void enterEvent( QEvent *event );

  //! resize event
  void resizeEvent( QResizeEvent* );

  //! timer event
  void timerEvent( QTimerEvent* );

  private slots:

  //! Save Current entry
  void _save( bool update_selection = true );

  //! creates a new entry
  void _newEntry( void );

  //! uniconify
  void _uniconify( void )
  {
    Debug::Throw( "EditFrame::uniconify.\n" );
    QtUtil::uniconify( this );
  }

  //! configuration
  void _updateConfiguration( void );

  //! configuration
  void _saveConfiguration( void );

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
  void _undo( void );

  //! redo in focused editor (text/title/keyword);
  void _redo( void );

  //! clone editor
  void _cloneWindow( void );
  
  //! view current entry as HTML
  void _viewHtml( void );

  //! unlock read-only editors
  void _unlock( void );

  //! update (enable/disable) undo action
  void _updateUndoAction( void );

  //! update (enable/disable) redo action
  void _updateRedoAction( void );

  /*!
    \brief update (enable/disable) undo/redo action
    based on the widget that currently has focus
  */
  void _updateUndoRedoActions( QWidget* old, QWidget* current );

  //! Set entry as modified, change window title
  void _titleModified( bool );

  //! Set entry as modified, change window title
  void _textModified( bool );

  //! display cursor position
  void _displayCursorPosition( void )
  { _displayCursorPosition( _activeEditor().textPosition() ); }

  //! display cursor position
  void _displayCursorPosition( int, int new_position )
  { _displayCursorPosition( TextPosition( 0, new_position ) ); }

  //! close
  void _close( void )
  {
    Debug::Throw( "EditFrame::_closeView (SLOT)\n" );
    BASE::KeySet< TextEditor > editors( this );
    if( editors.size() > 1 ) _closeEditor( _activeEditor() );
    else close();    
  }
  
  //! clone current file
  void _splitView( void )
  { _splitView( Qt::Vertical ); }

  //! clone current file horizontal
  void _splitViewHorizontal( void )
  { _splitView( Qt::Horizontal ); }

  //! clone current file horizontal
  void _splitViewVertical( void )
  { _splitView( Qt::Vertical ); }
  
  //! display focus changed
  void _displayFocusChanged( TextEditor* );  

  private:  
  
  //! install actions
  void _installActions( void );


  //!@name display management
  //@{
  
  //! retrieve active display
  TextEditor& _activeEditor( void )
  { return *active_editor_; }
  
  //! retrieve active display
  const TextEditor& _activeEditor( void ) const
  { return *active_editor_; }

  //! change active display manualy
  void _setActiveEditor( TextEditor& ); 

  //! close view
  /*! Ask for save if view is modified */
  void _closeEditor( TextEditor& );

  //! split view
  TextEditor& _splitView( const Qt::Orientation& );
  
  //! create new splitter
  QSplitter& _newSplitter( const Qt::Orientation&  );
  
  //! create new TextEditor
  TextEditor& _newTextEditor( QWidget* parent );

  //@}
  
  //! display cursor position
  void _displayCursorPosition( const TextPosition& position );

  //! retrieve associated SelectionFrame
  SelectionFrame* _selectionFrame( void ) const;

  //! update text Widget from current entry
  void _displayText( void );

  //! update attachment list Widget from current entry
  void _displayAttachments( void );

  //! if true, LogEntry associated to EditFrame cannot be modified
  bool read_only_;

  //! list of buttons to disactivate in case of read-only
  std::vector< QWidget* > read_only_widgets_;

  //! "closed" flag
  /*! this flag is used for delayed deletion of EditFrames, when direct deletion might cause flags */
  bool closed_;

  //!@name stored actions to toggle visibility
  //@{

  //! lock toolbar
  QToolBar* lock_;

  //@}
  
  //! local QSplitter object, derived from Counter
  /*! helps keeping track of how many splitters are created/deleted */
  class LocalSplitter: public QSplitter, public Counter
  {
    
    public:
    
    //! constructor
    LocalSplitter( QWidget* parent ):
      QSplitter( parent ),
      Counter( "LocalSplitter" )
    { Debug::Throw( "LocalSplitter::LocalSplitter.\n" ); }

    //! destructor
    virtual ~LocalSplitter( void )
    { Debug::Throw( "LocalSplitter::~LocalSplitter.\n" ); }
    
  };

  
  //! main splitter
  QSplitter *splitter_;

  //! main widget (that contains first editor)
  QWidget *main_;
  
  //! titlebar layout
  QHBoxLayout* title_layout_;

  //! LogEntry title Object
  LineEditor *title_;

  //! color widget
  class ColorWidget: public QToolButton, public Counter
  {

    public:

    //! constructor
    ColorWidget( QWidget* parent );

    //! color
    void setColor( const QColor& color );

  };

  //! color widget
  ColorWidget* color_widget_;

  //! LogEntry text Object
  TextEditor *active_editor_;

  //! pointer to text format bar
  FormatBar* format_toolbar_;

  //! pointer to statusbar
  StatusBar* statusbar_;

  //! resize timer
  QBasicTimer resize_timer_;

  //!@name actions
  //@{

  //! undo
  QAction* undo_action_;

  //! redo
  QAction* redo_action_;

  //! new entry
  QAction* new_entry_action_;
  
  //! previous entry action
  QAction* previous_entry_action_;

  //! next entry action
  QAction* next_entry_action_;

  //! save
  QAction* save_action_;
  
  #if WITH_ASPELL
  QAction* spellcheck_action_;
  #endif
  
  //! entry information
  QAction* entry_info_action_;
  
  //! view html
  QAction* view_html_action_;
  
  //! split view horizontal
  QAction* split_view_horizontal_action_;
  
  //! split view vertical
  QAction* split_view_vertical_action_;

  //! new window action
  QAction* clone_window_action_;
  
  //! close view (or window) action
  QAction* close_action_;
  
  //! uniconify
  QAction* uniconify_action_;
  
  //@}
};

#endif
