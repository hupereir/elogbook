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
  \file EditionWindow.cpp
  \brief log entry edition/creation object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QLayout>
#include <QPainter>

#include "AttachmentFrame.h"
#include "AttachmentList.h"
#include "BaseIcons.h"
#include "ColorMenu.h"
#include "LineEditor.h"
#include "TextEditor.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"
#include "EditionWindow.h"
#include "File.h"
#include "FormatBar.h"
#include "HtmlUtil.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryInformationDialog.h"
#include "Application.h"
#include "Menu.h"
#include "Options.h"
#include "QtUtil.h"
#include "MainWindow.h"
#include "StatusBar.h"
#include "Str.h"
#include "Util.h"
#include "ViewHtmlEntryDialog.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellDialog.h"
#endif

using namespace std;
using namespace Qt;

//_______________________________________________
EditionWindow::EditionWindow( QWidget* parent, bool read_only ):
  CustomMainWindow( parent ),
  Counter( "EditionWindow" ),
  read_only_( read_only ),
  closed_( false ),
  color_widget_( 0 ),
  active_editor_( 0 ),
  format_toolbar_( 0 )
{
  Debug::Throw("EditionWindow::EditionWindow.\n" );
  setObjectName( "EDITFRAME" );
  
  QWidget* main( new QWidget( this ) ); 
  setCentralWidget( main );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setMargin(2);
  layout->setSpacing(2);
  main->setLayout( layout );
  
  title_layout_ = new QHBoxLayout();
  title_layout_->setMargin(0);
  title_layout_->setSpacing(2);
  layout->addLayout( title_layout_ );
  
  // title label and line
  title_layout_->addWidget( title_ = new LineEditor( main ), 1 );
    
  // splitter for EditionWindow and attachment list
  splitter_ = new QSplitter( main );
  splitter_->setOrientation( Qt::Vertical );
  layout->addWidget( splitter_, 1 );
  
  // create text
  splitter_->addWidget( main_ = new QWidget() );
  main_->setLayout( new QVBoxLayout() );
  main_->layout()->setMargin(0);
  main_->layout()->setSpacing(0);

  // create editor
  TextEditor& editor( _newTextEditor( main_ ) );
  main_->layout()->addWidget( &editor );
  
  connect( title_, SIGNAL( modificationChanged( bool ) ), SLOT( _titleModified( bool ) ) );
  connect( activeEditor().document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );
  connect( title_, SIGNAL( cursorPositionChanged( int, int ) ), SLOT( _displayCursorPosition( int, int ) ) );

  // create attachment list
  AttachmentList *attachment_list = new AttachmentList( splitter_, isReadOnly() );
  attachment_list->hide();

  // associate EditionWindow and attachment list
  Key::associate( this, attachment_list );

  // status bar for tooltips
  setStatusBar( statusbar_ = new StatusBar( this ) );
  statusBar().addLabel( 2 );
  statusBar().addLabels( 2, 0 );
  statusBar().addClock();

  // actions
  _installActions();
  
  // toolbars
  // lock toolbar is visible only when window is not editable
  lock_ = new CustomToolBar( "Lock", this, "LOCK_TOOLBAR" );
  CustomToolButton *button;
  button = new CustomToolButton( lock_, IconEngine::get( ICONS::LOCK ) );
  connect( button, SIGNAL( clicked() ), SLOT( _unlock() ) );
  button->setToolTip( "Remove read-only lock for current editor." );
  button->setText("Unlock");  
  lock_->addWidget( button );
  lock_->setMovable( false );

  // main toolbar
  CustomToolBar* toolbar;
  toolbar = new CustomToolBar( "Main", this, "MAIN_TOOLBAR" );
    
  // new entry
  toolbar->addAction( &newEntryAction() );
  toolbar->addAction( &saveAction() );

  // delete_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::DELETE ), "Delete the current entry" );
  connect( button, SIGNAL( clicked() ), SLOT( _deleteEntry() ) );
  button->setText("Delete");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // add_attachment button
  toolbar->addAction( &attachment_list->newAttachmentAction() );

  // format bar
  format_toolbar_ = new FormatBar( this, "FORMAT_TOOLBAR" );
  format_toolbar_->setTarget( activeEditor() );
  const FormatBar::ButtonMap& buttons( format_toolbar_->buttons() );
  for( FormatBar::ButtonMap::const_iterator iter = buttons.begin(); iter != buttons.end(); iter++ )
  { read_only_widgets_.push_back( iter->second ); }

  // edition toolbars
  toolbar = new CustomToolBar( "History", this, "EDITION_TOOLBAR" );
  toolbar->addAction( undo_action_ );
  toolbar->addAction( redo_action_ );
  read_only_widgets_.push_back( toolbar );

  // undo/redo connections
  connect( title_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateUndoAction() ) );
  connect( title_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateRedoAction() ) );
  connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );

  connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );
    
  // extra toolbar
  toolbar = new CustomToolBar( "Tools", this, "EXTRA_TOOLBAR" );

  #if WITH_ASPELL
  toolbar->addAction( &spellCheckAction() );
  #endif

  toolbar->addAction( &viewHtmlAction() );
  toolbar->addAction( &entryInfoAction() );
  
  // extra toolbar
  toolbar = new CustomToolBar( "Multiple views", this, "MULTIPLE_VIEW_TOOLBAR" );
  toolbar->addAction( &splitViewHorizontalAction() );
  toolbar->addAction( &splitViewVerticalAction() );
  toolbar->addAction( &cloneWindowAction() );
  toolbar->addAction( &closeAction() );
  
  // extra toolbar
  toolbar = new CustomToolBar( "Navigation", this, "NAVIGATION_TOOLBAR" );
  toolbar->addAction( &static_cast<Application*>(qApp)->selectionFrame().uniconifyAction() );
  toolbar->addAction( &previousEntryAction() );
  toolbar->addAction( &nextEntryAction() );

  // create menu if requested
  Menu* menu = new Menu( this, &static_cast<Application*>(qApp)->selectionFrame() ); 
  setMenuBar( menu );
  
  // changes display according to read_only flag
  setReadOnly( read_only_ );
  
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();
  Debug::Throw("EditionWindow::EditionWindow - done.\n" );

}

//____________________________________________
EditionWindow::~EditionWindow( void )
{ Debug::Throw( "EditionWindow::~EditionWindow.\n" ); }

//____________________________________________
void EditionWindow::displayEntry( LogEntry *entry )
{
  Debug::Throw( "EditionWindow::displayEntry.\n" );

  // disassociate with existing entries, if any
  clearAssociations< LogEntry >();

  // check entry
  if( !entry ) return;

  // update current entry
  Key::associate( entry, this );

  // update all display
  displayTitle();
  displayColor();
  _displayText();
  _displayAttachments();

  // retrieve selection frame
  MainWindow *frame( _selectionFrame() );
  
  // update previous and next action states
  Debug::Throw( "EditionWindow::displayEntry - setting button states.\n" );
  previousEntryAction().setEnabled( frame->previousEntry(entry, false) );
  nextEntryAction().setEnabled( frame->nextEntry(entry, false) );
  
  // reset modify flag; change title accordingly
  setModified( false );
  updateWindowTitle();

  Debug::Throw( "EditionWindow::displayEntry - done.\n" );
  return;
}

//____________________________________________
void EditionWindow::setReadOnly( const bool& value )
{

  Debug::Throw() << "EditionWindow::setReadOnly - " << (value ? "true":"false" ) << endl;
  
  // update read_only value
  read_only_ = value;

  // changes button state
  for( vector< QWidget* >::iterator it=read_only_widgets_.begin(); it != read_only_widgets_.end(); it++ )
  { (*it)->setEnabled( !isReadOnly() ); }

  // changes lock button state
  if( isReadOnly() && lock_->isHidden() ) 
  {
    
    Qt::ToolBarArea current_location = toolBarArea( lock_ );
    if( current_location == Qt::NoToolBarArea ) { addToolBar( Qt::LeftToolBarArea, lock_ ); }
    lock_->show();
    
  } else if( !(isReadOnly() || lock_->isHidden() ) ) { lock_->hide(); }

  // changes TextEdit readOnly status
  title_->setReadOnly( isReadOnly() );
  
  // update editors
  BASE::KeySet<TextEditor> editors( this );
  for( BASE::KeySet<TextEditor>::iterator iter = editors.begin(); iter != editors.end(); iter++ )
  { (*iter)->setReadOnly( isReadOnly() ); }

  // changes attachment list status
  attachmentList().setReadOnly( isReadOnly() );

  // changes window title
  updateWindowTitle();
  
}
  
//_____________________________________________
string EditionWindow::windowTitle( void ) const
{

  Debug::Throw( "EditionWindow::windowTitle.\n" );
  LogEntry* entry( EditionWindow::entry() );

  ostringstream title;
  if( entry )
  {
  
    title << entry->title();
    if( entry->keyword() != Keyword::NO_KEYWORD ) title << " - " << entry->keyword().get();

  } else title << "Electronic Logbook Editor";

  if( isReadOnly() ) title << " (read only)";
  else if( modified()  ) title << " (modified)";
  return title.str();

}

//____________________________________________
AskForSaveDialog::ReturnCode EditionWindow::askForSave( const bool& enable_cancel )
{
  
  Debug::Throw( "EditionWindow::askForSave.\n" );
  
  // retrieve other editFrames
  BASE::KeySet<EditionWindow> frames( _selectionFrame() );
  unsigned int count( count_if( frames.begin(), frames.end(), ModifiedFTor() ) );
  
  // create dialog
  unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
  if( enable_cancel ) buttons |= AskForSaveDialog::CANCEL;
  if( count > 1 ) buttons |= AskForSaveDialog::ALL; 
  AskForSaveDialog dialog( this, "Entry has been modified. Save ?", buttons );
  QtUtil::centerOnParent( &dialog );
  
  // exec and check return code
  int state = dialog.exec();
  if( state == AskForSaveDialog::YES ) _save( false );
  else if( state == AskForSaveDialog::ALL ) 
  {
    /*
      save_all: if the logbook has no valid file one save the modified edit frames one by one
      otherwise one directly save the loogbook, while disabling the confirmation for modified
      entries
    */
    if( _selectionFrame()->logbook()->file().empty() )
    {
      for( BASE::KeySet<EditionWindow>::iterator iter = frames.begin(); iter!= frames.end(); iter++ )
      { if( (*iter)->modified() && !(*iter)->isReadOnly() ) (*iter)->_save(enable_cancel); }
    } else _selectionFrame()->save( false );
  }
  
  return AskForSaveDialog::ReturnCode(state);
  
}

//_____________________________________________
void EditionWindow::displayTitle( void )
{
  Debug::Throw( "EditionWindow::displayTitle.\n" );

  LogEntry* entry( EditionWindow::entry() );
  title_->setText( ( entry && entry->title().size() ) ? entry->title().c_str(): LogEntry::UNTITLED.c_str()  );
  title_->setCursorPosition( 0 );
  return;
}

//_____________________________________________
void EditionWindow::displayColor( void )
{
  Debug::Throw( "EditionWindow::DisplayColor.\n" );

  // Create color widget if none
  if( !color_widget_ ) 
  { 
    color_widget_ = new ColorWidget( title_->parentWidget() );
    title_layout_->addWidget( color_widget_ );
  }

  // try load entry color
  QColor color;
  Str colorname( entry()->color() );
  if( colorname.isEqual( ColorMenu::NONE, false ) || !( color = QColor( colorname.c_str() ) ).isValid() ) { color_widget_->hide(); } 
  else { 
    color_widget_->setColor( color ); 
    color_widget_->show();
  }
  
  return;
  
}

//______________________________________________________
void EditionWindow::setModified( const bool& value )
{
  Debug::Throw( "EditionWindow::setModified.\n" );
  title_->setModified( value );
  activeEditor().document()->setModified( value );
}

//_____________________________________________
void EditionWindow::_save( bool update_selection )
{
  
  Debug::Throw( "EditionWindow::_save.\n" );
  if( isReadOnly() ) return;

  // retrieve associated entry
  LogEntry *entry( EditionWindow::entry() );

  // see if entry is new
  bool entry_is_new( !entry || BASE::KeySet<Logbook>( entry ).empty() );
  
  // create entry if none set
  if( !entry ) entry = new LogEntry();

  // check logbook
  MainWindow *frame( _selectionFrame() );
  Logbook *logbook( frame->logbook() );
  if( !logbook ) {
    QtUtil::infoDialog( this, "No logbook opened. <Save> canceled." );
    return;
  }
  Debug::Throw( "EditionWindow::_save - logbook checked.\n" );

  //! update entry text
  entry->setText( qPrintable( activeEditor().toPlainText() ) );
  entry->setFormats( format_toolbar_->get() );

  //! update entry title
  entry->setTitle( qPrintable( title_->text() ) );

  // update author
  entry->setAuthor( XmlOptions::get().raw( "USER" ) );

  // add _now_ to entry modification timestamps
  entry->modified();

  // status bar
  statusbar_->label().setText(  "writting entry to logbook ..." );
  Debug::Throw( "EditionWindow::_save - statusbar set.\n" );

  // add entry to logbook, if needed
  if( entry_is_new ) Key::associate( entry, logbook->latestChild() );

  // update this window title, set unmodified.
  setModified( false );
  updateWindowTitle();
  Debug::Throw( "EditionWindow::_save - modified state saved.\n" );

  // update associated EditionWindows
  BASE::KeySet<EditionWindow> editors( entry );
  for( BASE::KeySet<EditionWindow>::iterator iter = editors.begin(); iter != editors.end(); iter++ )
  {
    assert( *iter == this || (*iter)->isReadOnly() || (*iter)->isClosed() );
    if( *iter == this ) continue;
    (*iter)->displayEntry( entry );
  }
  Debug::Throw( "EditionWindow::_save - editFrames updated.\n" );
  
  // update selection frame
  frame->updateEntry( entry, update_selection );
  frame->setWindowTitle( Application::MAIN_TITLE_MODIFIED );
  Debug::Throw( "EditionWindow::_save - selectionFrame updated.\n" );

  // set logbook as modified
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter!= logbooks.end(); iter++ )
  (*iter)->setModified( true );
  Debug::Throw( "EditionWindow::_save - loogbook modified state updated.\n" );

  // Save logbook
  if( frame->logbook()->file().size() ) frame->save();
  Debug::Throw( "EditionWindow::_save - selectionFrame saved.\n" );

  statusbar_->label().setText( "" );
  Debug::Throw( "EditionWindow::_save - done.\n" );

  return;

}

//_____________________________________________
void EditionWindow::_newEntry( void )
{

  Debug::Throw( "EditionWindow::_newEntry.\n" );

  // check if entry is modified
  if( modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

  // create new entry, set author, set keyword
  LogEntry* entry = new LogEntry();
  entry->setAuthor( XmlOptions::get().raw( "USER" ) );
  entry->setKeyword( _selectionFrame()->currentKeyword() );

  // add logbook parent if any
  Logbook *logbook( _selectionFrame()->logbook() );
  if( logbook ) Key::associate( entry, logbook->latestChild() );

  // display new entry
  displayEntry( entry );
  
  // set focus to title bar 
  title_->setFocus();
  title_->selectAll();

}

//____________________________________________
void EditionWindow::closeEvent( QCloseEvent *event )
{
  Debug::Throw( "EditionWindow::closeEvent.\n" );
  
  // ask for save if entry is modified
  if( !(isReadOnly() || isClosed() ) && modified() && askForSave() == AskForSaveDialog::CANCEL ) event->ignore();
  else
  {
    _saveConfiguration();
    setIsClosed( true );
    event->accept();
  }
  
  return;
}

//____________________________________________
void EditionWindow::enterEvent( QEvent *event )
{
  Debug::Throw( "EditionWindow::enterEvent.\n" );

  // base class enterEvent
  QMainWindow::enterEvent( event );
  _selectionFrame()->checkLogbookModified();

}

//_______________________________________________________
void EditionWindow::resizeEvent( QResizeEvent* event )
{
  resize_timer_.start( 200, this );
  return CustomMainWindow::resizeEvent( event );
}

//_______________________________________________________
void EditionWindow::timerEvent( QTimerEvent* event )
{

  if( event->timerId() == resize_timer_.timerId() )
  {
    
    // stop timer
    resize_timer_.stop();
    
    // save size
    XmlOptions::get().set<int>( "EDIT_FRAME_HEIGHT", height() );
    XmlOptions::get().set<int>( "EDIT_FRAME_WIDTH", width() );
    
  } else return CustomMainWindow::timerEvent( event );
  
}

//_____________________________________________
void EditionWindow::_installActions( void )
{
  Debug::Throw( "EditionWindow::_installActions.\n" );
  
  // undo action
  addAction( undo_action_ = new QAction( IconEngine::get( ICONS::UNDO ), "&Undo", this ) );
  undo_action_->setToolTip( "Undo last modification" );
  connect( undo_action_, SIGNAL( triggered() ), SLOT( _undo() ) );
  
  // redo action
  addAction( redo_action_ = new QAction( IconEngine::get( ICONS::REDO ), "&Redo", this ) );
  redo_action_->setToolTip( "Redo last undone modification" );
  connect( redo_action_, SIGNAL( triggered() ), SLOT( _redo() ) );

  // new entry
  addAction( new_entry_action_ = new QAction( IconEngine::get( ICONS::NEW ), "&New Entry", this ) );
  new_entry_action_->setToolTip( "Create new entry in current editor" );
  connect( new_entry_action_, SIGNAL( triggered() ), SLOT( _newEntry() ) );
  
  // previous_entry action
  addAction( previous_entry_action_ = new QAction( IconEngine::get( ICONS::PREV ), "&Previous Entry", this ) );
  previous_entry_action_->setToolTip( "Display previous entry in current list" );
  connect( previous_entry_action_, SIGNAL( triggered() ), SLOT( _previousEntry() ) );

  // previous_entry action
  addAction( next_entry_action_ = new QAction( IconEngine::get( ICONS::NEXT ), "&Next Entry", this ) );
  next_entry_action_->setToolTip( "Display next entry in current list" );
  connect( next_entry_action_, SIGNAL( triggered() ), SLOT( _nextEntry() ) );
  next_entry_action_->setShortcut( CTRL+Key_N );

  // save
  addAction( save_action_ = new QAction( IconEngine::get( ICONS::SAVE ), "&Save Entry", this ) );
  save_action_->setToolTip( "Save current entry" );
  connect( save_action_, SIGNAL( triggered() ), SLOT( _save() ) );
  save_action_->setShortcut( CTRL+Key_S );

  #if WITH_ASPELL
  addAction( spellcheck_action_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "Spell", this ) );
  spellcheck_action_->setToolTip( "Check spelling of current entry" );
  connect( spellcheck_action_, SIGNAL( triggered() ), SLOT( _spellCheck() ) );
  #endif

  // entry_info button
  addAction( entry_info_action_ = new QAction( IconEngine::get( ICONS::INFO ), "Entry Information", this ) );
  entry_info_action_->setToolTip( "Show current entry information" );
  connect( entry_info_action_, SIGNAL( triggered() ), SLOT( _entryInfo() ) );
  
  // html
  addAction( view_html_action_ = new QAction( IconEngine::get( ICONS::HTML ), "&View HTML", this ) );
  view_html_action_->setToolTip( "Convert current entry to HTML file" );
  connect( view_html_action_, SIGNAL( triggered() ), SLOT( _viewHtml() ) );
    
  // split action
  addAction( split_view_horizontal_action_ =new QAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM ), "Split view top/bottom", this ) );
  split_view_horizontal_action_->setToolTip( "Split current text editor vertically" );
  connect( split_view_horizontal_action_, SIGNAL( triggered() ), SLOT( _splitViewVertical() ) );

  addAction( split_view_vertical_action_ =new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "Split view left/right", this ) );
  split_view_vertical_action_->setToolTip( "Split current text editor horizontally" );
  connect( split_view_vertical_action_, SIGNAL( triggered() ), SLOT( _splitViewHorizontal() ) );

  // clone window action
  addAction( clone_window_action_ = new QAction( IconEngine::get( ICONS::VIEW_CLONE ), "Clone window", this ) );
  clone_window_action_->setToolTip( "Create a new edition window displaying the same entry" );
  connect( clone_window_action_, SIGNAL( triggered() ), SLOT( _cloneWindow() ) );

  // close window action
  addAction( close_action_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE ), "&Close view", this ) );
  close_action_->setShortcut( CTRL+Key_W );
  close_action_->setToolTip( "Close current view" );
  connect( close_action_, SIGNAL( triggered() ), SLOT( _close() ) );
 
  addAction( uniconify_action_ = new QAction( "&uniconify", this ) );
  connect( uniconify_action_, SIGNAL( triggered() ), SLOT( _uniconify() ) );

}

//_____________________________________________
void EditionWindow::_updateConfiguration( void )
{
  
  Debug::Throw( "EditionWindow::_updateConfiguration.\n" );
  
  // window size
  resize( XmlOptions::get().get<int>( "EDIT_FRAME_WIDTH" ), XmlOptions::get().get<int>( "EDIT_FRAME_HEIGHT" ) );
  
  // set splitter default size
  QList<int> sizes;
  sizes << XmlOptions::get().get<int>( "EDT_HEIGHT" );
  sizes << XmlOptions::get().get<int>( "ATC_HEIGHT" );
  splitter_->setSizes( sizes );
  
}

//_____________________________________________
void EditionWindow::_saveConfiguration( void )
{
  
  Debug::Throw( "EditionWindow::_saveConfiguration.\n" );
  
  // retrieve attachment list
  AttachmentList* atc_list( *BASE::KeySet<AttachmentList>(this).begin() );
  if( !atc_list->isHidden() )
  {
    XmlOptions::get().set<int>( "EDT_HEIGHT", activeEditor().height() );
    XmlOptions::get().set<int>( "ATC_HEIGHT", (*BASE::KeySet<AttachmentList>(this).begin())->height() );
  }
  
}

//_______________________________________________
void EditionWindow::_previousEntry( void )
{
  Debug::Throw( "EditionWindow::_previousEntry.\n" );

  //if( isReadOnly() ) return;
    
  MainWindow *frame( _selectionFrame() );
  LogEntry* entry( frame->previousEntry( EditionWindow::entry(), true ) );
  if( !( entry  && frame->lockEntry( entry ) ) ) return;
  displayEntry( entry );
  setReadOnly( false );

}

//_______________________________________________
void EditionWindow::_nextEntry( void )
{
  Debug::Throw( "EditionWindow::_nextEntry.\n" );

  //if( isReadOnly() ) return;

  MainWindow *frame( _selectionFrame() );
  LogEntry* entry( frame->nextEntry( EditionWindow::entry(), true ) );
  if( !( entry && frame->lockEntry( entry ) ) ) return;
  displayEntry( entry );
  setReadOnly( false );

}

//_____________________________________________
void EditionWindow::_entryInfo( void )
{

  Debug::Throw( "EditionWindow::_EntryInfo.\n" );

  // check entry
  LogEntry *entry( EditionWindow::entry() );
  if( !entry ) {
    QtUtil::infoDialog( this, "No valid entry."  );
    return;
  }

  // create dialog
  LogEntryInformationDialog dialog( this, entry );
  QtUtil::centerOnParent( &dialog );
  dialog.exec();

}

//_____________________________________________
void EditionWindow::_undo( void )
{
  Debug::Throw( "EditionWindow::_undo.\n" );
  if( activeEditor().QWidget::hasFocus() ) activeEditor().document()->undo();
  else if( title_->hasFocus() ) title_->undo();
  return;
}

//_____________________________________________
void EditionWindow::_redo( void )
{
  Debug::Throw( "EditionWindow::_redo.\n" );
  if( activeEditor().QWidget::hasFocus() ) activeEditor().document()->redo();
  else if( title_->hasFocus() ) title_->redo();
  return;
}

//_____________________________________________
void EditionWindow::_updateUndoAction( void )
{ 
  Debug::Throw( "EditionWindow::_updateUndoAction.\n" );
  if( title_->hasFocus() ) undo_action_->setEnabled( title_->isUndoAvailable() );
  if( activeEditor().QWidget::hasFocus() ) undo_action_->setEnabled( activeEditor().document()->isUndoAvailable() );
}

//_____________________________________________
void EditionWindow::_updateRedoAction( void )
{ 
  Debug::Throw( "EditionWindow::_updateRedoAction.\n" );
  if( title_->hasFocus() ) redo_action_->setEnabled( title_->isRedoAvailable() );
  if( activeEditor().QWidget::hasFocus() ) redo_action_->setEnabled( activeEditor().document()->isRedoAvailable() );
}

//_____________________________________________
void EditionWindow::_updateUndoRedoActions( QWidget*, QWidget* current )
{
  Debug::Throw( "EditionWindow::_updateUndoRedoAction.\n" );
  if( current == title_ )
  {
    undo_action_->setEnabled( title_->isUndoAvailable() );
    redo_action_->setEnabled( title_->isRedoAvailable() );
  }

  if( current == &activeEditor() )
  {
    undo_action_->setEnabled( activeEditor().document()->isUndoAvailable() );
    redo_action_->setEnabled( activeEditor().document()->isRedoAvailable() );
  }

}

//_____________________________________________
void EditionWindow::_deleteEntry( void )
{

  Debug::Throw( "EditionWindow::_deleteEntry.\n" );

  // check current entry
  LogEntry *entry( EditionWindow::entry() );

  if( !entry ) {
    QtUtil::infoDialog( this, "No entry selected. <Delete Entry> canceled." );
    return;
  }

  // ask confirmation
  if( !QtUtil::questionDialog( this, "Delete current entry ?" ) ) return;

  // get associated attachments
  _selectionFrame()->deleteEntry( entry );

  return;

}

//_____________________________________________
void EditionWindow::_spellCheck( void )
{
#if WITH_ASPELL
  Debug::Throw( "EditionWindow::_spellCheck.\n" );
  
  // create dialog
  SPELLCHECK::SpellDialog dialog( &activeEditor() );
  
  // set dictionary and filter
  dialog.setFilter( XmlOptions::get().raw("DICTIONARY_FILTER") );
  dialog.setDictionary( XmlOptions::get().raw("DICTIONARY") );
  dialog.nextWord();
  dialog.exec();
  
  // update dictionary and filter from dialog
  XmlOptions::get().setRaw( "DICTIONARY_FILTER", dialog.interface().filter() );
  XmlOptions::get().setRaw( "DICTIONARY", dialog.interface().dictionary() );
  
#endif
}


//_____________________________________________
void EditionWindow::_cloneWindow( void )
{
  
  Debug::Throw( "EditionWindow::_cloneWindow.\n" );
  LogEntry *entry( EditionWindow::entry() );
  if( !entry ) {
    QtUtil::infoDialog( this, "No valid entry found. <New window> canceled." );
    return;
  }

  // retrieve selection frame
  MainWindow *frame( _selectionFrame() );

  // create new EditionWindow
  EditionWindow *edit_frame( new EditionWindow( frame ) );
  Key::associate( edit_frame, frame );
  edit_frame->displayEntry( entry );

  // raise EditionWindow
  edit_frame->show();

  return;
}

//_____________________________________________
void EditionWindow::_viewHtml( void )
{
  Debug::Throw( "EditionWindow::_viewHtml.\n" );

  // check logbook entry
  LogEntry *entry( EditionWindow::entry() );
  if( !entry ){
    QtUtil::infoDialog( this, "No entry. <View HTML> canceled." );
    return;
  }

  // create custom dialog, retrieve check vbox child
  ViewHtmlEntryDialog dialog( this );
  dialog.setLogbookMask( 0 );
  dialog.setEntryMask( LogEntry::HTML_ALL_MASK );
  dialog.setCommand( (AttachmentType::HTML.editCommand().size() ) ? AttachmentType::HTML.editCommand():"" );

  // generate default filename
  ostringstream what;
  what << "_eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
  dialog.setFile( File( what.str() ).addPath( Util::tmp() ) );

  // map dialog
  QtUtil::centerOnParent( &dialog );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve/check file
  File file( dialog.file() );
  if( file.empty() ) {
    QtUtil::infoDialog(this, "No output file specified. <View HTML> canceled." );
    return;
  }

  QFile out( file.c_str() );
  if( !out.open( QIODevice::WriteOnly ) )
  {
    ostringstream o;
    o << "Cannot write to file \"" << file << "\". <View HTML> canceled.";
    QtUtil::infoDialog( this, o.str() );
    return;
  }

  // retrieve mask
  unsigned int html_log_mask = dialog.logbookMask();
  unsigned int html_entry_mask = dialog.entryMask();

  // dump header/style
  QDomDocument document( "html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"DTD/xhtml1-strict.dtd\"" );
  
  // html
  QDomElement html = document.appendChild( document.createElement( "html" ) ).toElement(); 
  html.setAttribute( "xmlns", "http://www.w3.org/1999/xhtml" );
  
  // head
  HtmlUtil::header( html, document );
  
  // body
  QDomElement body = html.appendChild( document.createElement( "body" ) ).toElement();
  
  // dump logbook header
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
  { body.appendChild( (*iter)->htmlElement( document, html_log_mask ) ); }

  // dump entry
  body.appendChild( entry->htmlElement( document, html_entry_mask ) );

  out.write( document.toByteArray() );
  out.close();

  // retrieve command
  string command( dialog.command() );
  if( command.empty() ) return;

  // execute command
  Util::run( QStringList() << command.c_str() << file.c_str() );

  return;
}

//_____________________________________________
void EditionWindow::_unlock( void )
{
  
  Debug::Throw( "EditionWindow::_unlock.\n" );
  
  if( !isReadOnly() ) return;
  LogEntry *entry( EditionWindow::entry() );
  
  if( entry && ! _selectionFrame()->lockEntry( entry ) ) return;
  setReadOnly( false );
  
  return;
  
}

//_____________________________________________
void EditionWindow::_titleModified( bool state )
{
  Debug::Throw() << "EditionWindow::_titleModified - state: " << (state ? "true":"false" ) << endl;

  // check readonly status
  if( isReadOnly() ) return;

  bool text_modified( activeEditor().document()->isModified() );
  if( state && !text_modified ) updateWindowTitle();
  if( !(state || text_modified ) ) updateWindowTitle();

  return;
}

//_____________________________________________
void EditionWindow::_textModified( bool state )
{
  Debug::Throw() << "EditionWindow::_textModified - state: " << (state ? "true":"false" ) << endl;

  // check readonly status
  if( isReadOnly() ) return;
  
  bool title_modified( title_->isModified() );
  if( state && !title_modified ) updateWindowTitle();
  if( !(state || title_modified ) ) updateWindowTitle();
  
}

//_____________________________________________
void EditionWindow::_displayFocusChanged( TextEditor* editor )
{
  
  Debug::Throw() << "EditionWindow::_DisplayFocusChanged - " << editor->key() << endl;
  _setActiveEditor( *editor );  

}  

//________________________________________________________________
void EditionWindow::_setActiveEditor( TextEditor& editor )
{ 
  Debug::Throw() << "EditionWindow::_setActiveEditor - key: " << editor.key() << std::endl;
  assert( editor.isAssociated( this ) );
  
  active_editor_ = &editor;
  if( !activeEditor().isActive() )
  {

    BASE::KeySet<TextEditor> editors( this );
    for( BASE::KeySet<TextEditor>::iterator iter = editors.begin(); iter != editors.end(); iter++ )
    { (*iter)->setActive( false ); }
    
    activeEditor().setActive( true );

  }
  
  // associate with toolbar
  if( format_toolbar_ ) format_toolbar_->setTarget( activeEditor() );

  Debug::Throw( "EditionWindow::setActiveDisplay - done.\n" );
  

}

//___________________________________________________________
void EditionWindow::_closeEditor( TextEditor& editor )
{
  Debug::Throw( "EditionWindow::_closeEditor.\n" );
 
  // retrieve number of editors
  // if only one display, close the entire window
  BASE::KeySet<TextEditor> editors( this );
  if( editors.size() < 2 )
  {
    Debug::Throw() << "EditionWindow::_closeEditor - full close." << endl;
    close();
    return;
  }

  // retrieve parent and grandparent of current display
  QWidget* parent( editor.parentWidget() );    
  QSplitter* parent_splitter( dynamic_cast<QSplitter*>( parent ) );
  
  // retrieve editors associated to current
  editors = BASE::KeySet<TextEditor>( &editor );
    
  // check how many children remain in parent_splitter if any
  // take action if it is less than 2 (the current one to be deleted, and another one)
  if( parent_splitter && parent_splitter->count() <= 2 ) 
  {
    
    // retrieve child that is not the current editor
    // need to loop over existing widgets because the editor above has not been deleted yet
    QWidget* child(0);
    for( int index = 0; index < parent_splitter->count(); index++ )
    { 
      if( parent_splitter->widget( index ) != &editor ) 
      {
        child = parent_splitter->widget( index );
        break;
      }
    }    
    assert( child );
    Debug::Throw( "EditionWindow::_closeEditor - found child.\n" );
    
    // retrieve splitter parent
    QWidget* grand_parent( parent_splitter->parentWidget() );
    
    // try cast to a splitter
    QSplitter* grand_parent_splitter( dynamic_cast<QSplitter*>( grand_parent ) );
    
    // move child to grand_parent_splitter if any
    if( grand_parent_splitter )
    { 
      grand_parent_splitter->insertWidget( grand_parent_splitter->indexOf( parent_splitter ), child );
    }  else {
      child->setParent( grand_parent );
      grand_parent->layout()->addWidget( child );
    }
    
    // delete parent_splitter, now that it is empty
    parent_splitter->deleteLater();
    Debug::Throw( "EditionWindow::_closeEditor - deleted splitter.\n" );

  } else {
    
    // the editor is deleted only if its parent splitter is not
    // otherwise this will trigger double deletion of the editor 
    // which will then crash
    editor.deleteLater();
    
  }
      
  // update activeEditor
  bool active_found( false );
  for( BASE::KeySet<TextEditor>::reverse_iterator iter = editors.rbegin(); iter != editors.rend(); iter++ )
  { 
    if( (*iter) != &editor ) {
      _setActiveEditor( **iter ); 
      active_found = true;
      break;
    }
  }  
  assert( active_found );

  // change focus
  activeEditor().setFocus();
  Debug::Throw( "EditionWindow::_closeEditor - done.\n" );

}

//___________________________________________________________
TextEditor& EditionWindow::_splitView( const Orientation& orientation )
{
  Debug::Throw( "EditionWindow::_splitView.\n" );

  // keep local pointer to current active display
  TextEditor& active_editor_local( activeEditor() );  
  
  // compute desired dimension of the new splitter
  // along its splitting direction
  int dimension( (orientation == Horizontal) ? active_editor_local.width():active_editor_local.height() );

  // create new splitter
  QSplitter& splitter( _newSplitter( orientation ) );
  
  // create new display
  TextEditor& editor( _newTextEditor(0) );
  
  // insert in splitter, at correct position
  splitter.insertWidget( splitter.indexOf( &active_editor_local )+1, &editor );

  // recompute dimension
  // take the max of active display and splitter,
  // in case no new splitter was created.
  dimension = max( dimension, (orientation == Horizontal) ? splitter.width():splitter.height() );
  
  // assign equal size to all splitter children
  QList<int> sizes;
  for( int i=0; i<splitter.count(); i++ )
  { sizes.push_back( dimension/splitter.count() ); }
  splitter.setSizes( sizes );

  // synchronize both editors, if cloned
  /*
  if there exists no clone of active display,
  backup text and register a new Sync object
  */
  BASE::KeySet<TextEditor> editors( &active_editor_local );

  // clone new display
  editor.synchronize( &active_editor_local );
    
  // perform associations
  // check if active editors has associates and propagate to new
  for( BASE::KeySet<TextEditor>::iterator iter = editors.begin(); iter != editors.end(); iter++ )
  { BASE::Key::associate( &editor, *iter ); }
  
  // associate new display to active
  BASE::Key::associate( &editor, &active_editor_local );

  return editor;

}

//____________________________________________________________
QSplitter& EditionWindow::_newSplitter( const Orientation& orientation )
{

  Debug::Throw( "EditionWindow::_newSplitter.\n" );
  QSplitter *splitter = 0;
      
  // retrieve parent of current display
  QWidget* parent( activeEditor().parentWidget() );  
  
  // try catch to splitter
  // do not create a new splitter if the parent has same orientation
  QSplitter *parent_splitter( dynamic_cast<QSplitter*>( parent ) );
  if( parent_splitter && parent_splitter->orientation() == orientation ) {
  
    Debug::Throw( "EditionWindow::_newSplitter - orientation match. No need to create new splitter.\n" );
    splitter = parent_splitter;
  
  } else {
    
    
    // move splitter to the first place if needed
    if( parent_splitter ) 
    {
      
      Debug::Throw( "EditionWindow::_newSplitter - found parent splitter with incorrect orientation.\n" );
      // create a splitter with correct orientation
      // give him no parent, because the parent is set in QSplitter::insertWidget()
      splitter = new LocalSplitter(0);
      splitter->setOrientation( orientation );
      parent_splitter->insertWidget( parent_splitter->indexOf( &activeEditor() ), splitter );
      
    } else {
      
      Debug::Throw( "EditionWindow::_newSplitter - no splitter found. Creating a new one.\n" );

      // create a splitter with correct orientation
      splitter = new LocalSplitter(parent);
      splitter->setOrientation( orientation );
      parent->layout()->addWidget( splitter );
      
    }
    
    // reparent current display
    splitter->addWidget( &activeEditor() );
    
    // resize parent splitter if any
    if( parent_splitter )
    {
      int dimension = ( parent_splitter->orientation() == Horizontal) ? 
        parent_splitter->width():
        parent_splitter->height();
      
      QList<int> sizes;
      for( int i=0; i<parent_splitter->count(); i++ )
      { sizes.push_back( dimension/parent_splitter->count() ); }
      parent_splitter->setSizes( sizes );
      
    }
    
  }
  
  // return created splitter
  return *splitter;

}

//_____________________________________________________________
TextEditor& EditionWindow::_newTextEditor( QWidget* parent )
{
  Debug::Throw( "EditionWindow::_newTextEditor.\n" );

  // create textDisplay
  TextEditor* editor = new TextEditor( parent );  

  // connections
  connect( editor, SIGNAL( hasFocus( TextEditor* ) ), SLOT( _displayFocusChanged( TextEditor* ) ) );
  connect( editor, SIGNAL( cursorPositionChanged() ), SLOT( _displayCursorPosition() ) );
  connect( editor, SIGNAL( undoAvailable( bool ) ), SLOT( _updateUndoAction() ) );
  connect( editor, SIGNAL( redoAvailable( bool ) ), SLOT( _updateRedoAction() ) );
  
  // associate display to this editFrame
  BASE::Key::associate( this, editor );
  
  // update current display and focus
  _setActiveEditor( *editor );
  editor->setFocus();
  Debug::Throw() << "EditionWindow::_newTextEditor - key: " << editor->key() << endl;
  Debug::Throw( "EditionWindow::_newTextEditor - done.\n" );
    
  return *editor;
  
}

//_____________________________________________
void EditionWindow::_displayCursorPosition( const TextPosition& position)
{
  Debug::Throw( "EditionWindow::_DisplayCursorPosition.\n" );
  if( !statusbar_ ) return;
  
  ostringstream what;
  what << "line : " << position.paragraph()+1;
  statusbar_->label(1).setText( what.str().c_str(), false );

  what.str("");
  what << "column : " << position.index()+1;
  statusbar_->label(2).setText( what.str().c_str(), true );
  
  return;
}

//_______________________________________________
MainWindow* EditionWindow::_selectionFrame( void ) const
{
  Debug::Throw( "EditionWindow::_selectionFrame.\n" );
  BASE::KeySet<MainWindow> frames( this );
  assert( frames.size()==1 );
  return *frames.begin();
}

//_____________________________________________
void EditionWindow::_displayText( void )
{
  Debug::Throw( "EditionWindow::_displayText.\n" );
  if( !&activeEditor() ) return;

  LogEntry* entry( EditionWindow::entry() );
  activeEditor().setPlainText( (entry) ? entry->text().c_str() : "" );
  format_toolbar_->load( entry->formats() );
  
  // reset undo/redo stack
  activeEditor().resetUndoRedoStack();
  
  return;
}

//_____________________________________________
void EditionWindow::_displayAttachments( void )
{
  Debug::Throw( "EditionWindow::_DisplayAttachments.\n" );

  AttachmentList &attachment_list( attachmentList() );
  attachment_list.clear();

  LogEntry* entry( EditionWindow::entry() );
  if( !entry ) {
    
    attachment_list.hide();
    return;
  
  }

  // get associated attachments
  BASE::KeySet<Attachment> attachments( entry );
  if( attachments.empty() ) {
  
    attachment_list.hide();
    return;
  
  }

  // display associated attachments
  for( BASE::KeySet<Attachment>::iterator it = attachments.begin(); it != attachments.end(); it++ )
  { attachment_list.add( *it ); }

  // show attachment list
  attachment_list.resizeColumns();
  attachment_list.sort();
  attachment_list.show();

  return;

}

//___________________________________________________________________________________
EditionWindow::ColorWidget::ColorWidget( QWidget* parent ):
  QToolButton( parent ), 
  Counter( "ColorWidget" )
{ Debug::Throw( "ColorWidget::ColorWidget.\n" ); }

//___________________________________________________________________________________
void EditionWindow::ColorWidget::setColor( const QColor& color )
{

  // create pixmap
  //QPixmap pixmap( QSize( 32, 32 ) );
  QPixmap pixmap( QSize( 48, 48 ) );
  pixmap.fill( Qt::transparent );
  
  QPainter painter( &pixmap );
  painter.setPen( Qt::NoPen );
  painter.setBrush( color );
  painter.setRenderHints( QPainter::Antialiasing|QPainter::SmoothPixmapTransform );
  
  QRect rect( pixmap.rect() );
  
  painter.drawEllipse( rect );
  painter.end();
  
  setIcon( QIcon( pixmap ) );
}