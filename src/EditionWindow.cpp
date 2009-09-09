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

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QStylePainter>
#include <QStyleOptionToolButton>

#include "Application.h"
#include "AttachmentWindow.h"
#include "AttachmentFrame.h"
#include "BaseIcons.h"
#include "ColorMenu.h"
#include "Command.h"
#include "CustomToolBar.h"
#include "EditionWindow.h"
#include "File.h"
#include "FormatBar.h"
#include "HtmlHeaderNode.h"
#include "Icons.h"
#include "IconSize.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryInformationDialog.h"
#include "MainWindow.h"
#include "Menu.h"
#include "Options.h"
#include "QuestionDialog.h"
#include "InformationDialog.h"
#include "RecentFilesMenu.h"
#include "Singleton.h"
#include "StatusBar.h"
#include "Str.h"
#include "Util.h"
#include "PrintLogEntryDialog.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellDialog.h"
#endif

using namespace std;
using namespace Qt;

//_______________________________________________
EditionWindow::EditionWindow( QWidget* parent, bool read_only ):
  BaseMainWindow( parent ),
  Counter( "EditionWindow" ),
  read_only_( read_only ),
  closed_( false ),
  color_menu_( 0 ),
  color_widget_( 0 ),
  active_editor_( 0 ),
  format_toolbar_( 0 ),
  statusbar_( 0 )
{
  Debug::Throw("EditionWindow::EditionWindow.\n" );
  setOptionName( "EDITION_WINDOW" );
  setObjectName( "EDITFRAME" );

  QWidget* main( new QWidget( this ) );
  setCentralWidget( main );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setMargin(0);
  layout->setSpacing(2);
  main->setLayout( layout );

  // title layout (for editor and button)
  title_layout_ = new QHBoxLayout();
  title_layout_->setMargin(0);
  title_layout_->setSpacing(0);
  layout->addLayout( title_layout_ );

  // title label and line
  title_layout_->addWidget( title_ = new Editor( main ), 1 );
  title_->setHasClearButton( true );

  // splitter for EditionWindow and attachment list
  QSplitter* splitter = new QSplitter( main );
  splitter->setOrientation( Qt::Vertical );
  layout->addWidget( splitter, 1 );

  // create text
  splitter->addWidget( main_ = new QWidget() );
  main_->setLayout( new QVBoxLayout() );
  main_->layout()->setMargin(0);
  main_->layout()->setSpacing(0);

  // assign stretch factors
  splitter->setStretchFactor( 0, 1 );
  splitter->setStretchFactor( 1, 0 );

  connect( splitter, SIGNAL( splitterMoved( int, int ) ), SLOT( _splitterMoved( void ) ) );

  // create editor
  AnimatedTextEditor& editor( _newTextEditor( main_ ) );
  main_->layout()->addWidget( &editor );

  connect( title_, SIGNAL( modificationChanged( bool ) ), SLOT( _titleModified( bool ) ) );
  connect( activeEditor().document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );
  connect( title_, SIGNAL( cursorPositionChanged( int, int ) ), SLOT( _displayCursorPosition( int, int ) ) );

  // create attachment list
  AttachmentFrame *frame = new AttachmentFrame( 0, isReadOnly() );
  frame->visibilityAction().setChecked( false );
  frame->setDefaultHeight( XmlOptions::get().get<int>( "ATTACHMENT_FRAME_HEIGHT" ) );
  splitter->addWidget( frame );
  Key::associate( this, frame );

  // status bar for tooltips
  setStatusBar( statusbar_ = new StatusBar( this ) );
  statusBar().addLabel( 2, true );
  statusBar().addLabels( 3, 0 );
  statusBar().addClock();

  // actions
  _installActions();
  Application& application( *Singleton::get().application<Application>() );
  addAction( &application.closeAction() );

  // toolbars
  // lock toolbar is visible only when window is not editable
  lock_ = new CustomToolBar( "Lock", this, "LOCK_TOOLBAR" );
  lock_->setMovable( false );

  // hide lock_ visibility action because the latter should not be checkable in any menu
  lock_->visibilityAction().setVisible( false );

  QAction *action;
  lock_->addAction( action = new QAction( IconEngine::get( ICONS::LOCK ), "&Unlock", this ) );
  connect( action, SIGNAL( triggered() ), SLOT( _unlock() ) );
  action->setToolTip( "Remove read-only lock for current editor." );

  // main toolbar
  CustomToolBar* toolbar;
  toolbar = new CustomToolBar( "Main", this, "MAIN_TOOLBAR" );

  // new entry
  toolbar->addAction( &newEntryAction() );
  toolbar->addAction( &saveAction() );

  // delete_entry button
  toolbar->addAction( action = new QAction( IconEngine::get( ICONS::DELETE ), "&Delete Entry", this ) );
  connect( action, SIGNAL( triggered() ), SLOT( _deleteEntry() ) );
  read_only_actions_.push_back( action );

  // add_attachment button
  toolbar->addAction( &frame->newAction() );

  // format bar
  format_toolbar_ = new FormatBar( this, "FORMAT_TOOLBAR" );
  format_toolbar_->setTarget( activeEditor() );
  const FormatBar::ActionMap& actions( format_toolbar_->actions() );
  for( FormatBar::ActionMap::const_iterator iter = actions.begin(); iter != actions.end(); iter++ )
  { read_only_actions_.push_back( iter->second ); }

  // set proper connection for first editor
  // (because it could not be performed in _newTextEditor)
  connect(
    &editor, SIGNAL( currentCharFormatChanged( const QTextCharFormat& ) ),
    format_toolbar_, SLOT( updateState( const QTextCharFormat& ) ) );

  // edition toolbars
  toolbar = new CustomToolBar( "History", this, "EDITION_TOOLBAR" );
  toolbar->addAction( undo_action_ );
  toolbar->addAction( redo_action_ );
  read_only_actions_.push_back( undo_action_ );
  read_only_actions_.push_back( redo_action_ );

  // undo/redo connections
  connect( title_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateUndoAction() ) );
  connect( title_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateRedoAction() ) );
  connect( title_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateSaveAction() ) );
  connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );
  connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );

  // extra toolbar
  toolbar = new CustomToolBar( "Tools", this, "EXTRA_TOOLBAR" );

  #if WITH_ASPELL
  toolbar->addAction( &spellCheckAction() );
  #endif

  toolbar->addAction( &printAction() );
  toolbar->addAction( &entryInfoAction() );

  // extra toolbar
  toolbar = new CustomToolBar( "Multiple Views", this, "MULTIPLE_VIEW_TOOLBAR" );
  toolbar->addAction( &splitViewHorizontalAction() );
  toolbar->addAction( &splitViewVerticalAction() );
  toolbar->addAction( &cloneWindowAction() );
  toolbar->addAction( &closeAction() );

  // extra toolbar
  toolbar = new CustomToolBar( "Navigation", this, "NAVIGATION_TOOLBAR" );
  toolbar->addAction( &application.mainWindow().uniconifyAction() );
  toolbar->addAction( &previousEntryAction() );
  toolbar->addAction( &nextEntryAction() );

  // create menu if requested
  menu_ = new Menu( this, &Singleton::get().application<Application>()->mainWindow() );
  setMenuBar( menu_ );

  // changes display according to read_only flag
  setReadOnly( read_only_ );

  // configuration
  connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
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

  // retrieve selection frame
  MainWindow &mainwindow( _mainWindow() );
  _menu().recentFilesMenu().setCurrentFile( mainwindow.menu().recentFilesMenu().currentFile() );

  // check entry
  if( !entry ) return;

  // update current entry
  Key::associate( entry, this );

  // update all display
  displayTitle();
  displayColor();
  _displayText();
  _displayAttachments();

  // update previous and next action states
  Debug::Throw( "EditionWindow::displayEntry - setting button states.\n" );
  previousEntryAction().setEnabled( mainwindow.previousEntry(entry, false) );
  nextEntryAction().setEnabled( mainwindow.nextEntry(entry, false) );

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
  for( ActionList::iterator it=read_only_actions_.begin(); it != read_only_actions_.end(); it++ )
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
  BASE::KeySet<AnimatedTextEditor> editors( this );
  for( BASE::KeySet<AnimatedTextEditor>::iterator iter = editors.begin(); iter != editors.end(); iter++ )
  { (*iter)->setReadOnly( isReadOnly() ); }

  // changes attachment list status
  attachmentFrame().setReadOnly( isReadOnly() );

  // changes window title
  _updateSaveAction();
  updateWindowTitle();

}

//_____________________________________________
void EditionWindow::setColorMenu( ColorMenu* menu )
{
  color_menu_ = menu;
  if( color_widget_ && !color_widget_->menu() )
  { color_widget_->setMenu( menu ); }
}

//_____________________________________________
QString EditionWindow::windowTitle( void ) const
{

  Debug::Throw( "EditionWindow::windowTitle.\n" );
  LogEntry* entry( EditionWindow::entry() );

  QString buffer;
  QTextStream what( &buffer );
  if( entry )
  {

    what << entry->title();
    if( entry->keyword() != Keyword::NO_KEYWORD ) what << " - " << entry->keyword().get();

  } else what << "Electronic Logbook Editor";

  if( isReadOnly() ) what << " (read only)";
  else if( modified()  ) what << " (modified)";
  return buffer;

}

//____________________________________________
AskForSaveDialog::ReturnCode EditionWindow::askForSave( const bool& enable_cancel )
{

  Debug::Throw( "EditionWindow::askForSave.\n" );

  // retrieve other editFrames
  BASE::KeySet<EditionWindow> editionwindows( &_mainWindow() );
  unsigned int count( count_if( editionwindows.begin(), editionwindows.end(), ModifiedFTor() ) );

  // create dialog
  unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
  if( enable_cancel ) buttons |= AskForSaveDialog::CANCEL;
  if( count > 1 ) buttons |= AskForSaveDialog::ALL;
  AskForSaveDialog dialog( this, "Entry has been modified. Save ?", buttons );

  // exec and check return code
  int state = dialog.centerOnParent().exec();
  if( state == AskForSaveDialog::YES ) _save( false );
  else if( state == AskForSaveDialog::ALL )
  {
    /*
      save_all: if the logbook has no valid file one save the modified editionwindows one by one
      otherwise one directly save the loogbook, while disabling the confirmation for modified
      entries
    */
    if( _mainWindow().logbook()->file().isEmpty() )
    {
      for( BASE::KeySet<EditionWindow>::iterator iter = editionwindows.begin(); iter!= editionwindows.end(); iter++ )
      { if( (*iter)->modified() && !(*iter)->isReadOnly() ) (*iter)->_save(enable_cancel); }
    } else _mainWindow().save( false );
  }

  return AskForSaveDialog::ReturnCode(state);

}

//_____________________________________________
void EditionWindow::displayTitle( void )
{
  Debug::Throw( "EditionWindow::displayTitle.\n" );

  LogEntry* entry( EditionWindow::entry() );
  title_->setText( ( entry && entry->title().size() ) ? entry->title(): LogEntry::UNTITLED  );
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
    color_widget_->setToolTip( "Change entry color.\nThis is used to tag entries in the main window list." );
    color_widget_->setAutoRaise( true );
    color_widget_->setPopupMode( QToolButton::InstantPopup );
    if( color_menu_ ) color_widget_->setMenu( color_menu_ );
    title_layout_->addWidget( color_widget_ );

  }

  // try load entry color
  QColor color;
  Str colorname( entry()->color() );
  if( colorname.compare( ColorMenu::NONE, Qt::CaseInsensitive ) == 0 || !( color = QColor( colorname ) ).isValid() )
  {

    color_widget_->hide();

  } else {

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
  MainWindow &mainwindow( _mainWindow() );
  Logbook *logbook( mainwindow.logbook() );
  if( !logbook ) {
    InformationDialog( this, "No logbook opened. <Save> canceled." ).exec();
    return;
  }
  Debug::Throw( "EditionWindow::_save - logbook checked.\n" );

  //! update entry text
  entry->setText( activeEditor().toPlainText() );
  entry->setFormats( format_toolbar_->get() );

  //! update entry title
  entry->setTitle( title_->text() );

  // update author
  entry->setAuthor( XmlOptions::get().raw( "USER" ) );

  // add _now_ to entry modification timestamps
  entry->modified();

  // status bar
  statusBar().label().setText(  "writting entry to logbook ..." );
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
  mainwindow.updateEntry( entry, update_selection );
  mainwindow.setWindowTitle( Application::MAIN_TITLE_MODIFIED );
  Debug::Throw( "EditionWindow::_save - mainWindow updated.\n" );

  // set logbook as modified
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter!= logbooks.end(); iter++ )
  { (*iter)->setModified( true ); }

  Debug::Throw( "EditionWindow::_save - loogbook modified state updated.\n" );

  // add to main logbook recent entries
  mainwindow.logbook()->addRecentEntry( entry );

  // Save logbook
  if( mainwindow.logbook()->file().size() )
  { mainwindow.save(); }

  Debug::Throw( "EditionWindow::_save - mainWindow saved.\n" );

  statusBar().label().setText( "" );
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
  entry->setKeyword( _mainWindow().currentKeyword() );

  // add logbook parent if any
  // Logbook *logbook( _mainWindow().logbook() );
  // if( logbook ) Key::associate( entry, logbook->latestChild() );

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
    setIsClosed( true );
    event->accept();
  }

  return;
}

//_______________________________________________________
void EditionWindow::timerEvent( QTimerEvent* event )
{

  if( event->timerId() == resize_timer_.timerId() )
  {

    // stop timer
    resize_timer_.stop();

    // save size
    if( attachmentFrame().visibilityAction().isChecked() )
    { XmlOptions::get().set<int>( "ATTACHMENT_FRAME_HEIGHT", attachmentFrame().height() ); }

  } else return BaseMainWindow::timerEvent( event );

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
  new_entry_action_->setShortcut( Qt::CTRL + Qt::Key_N );
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
  addAction( spellcheck_action_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "Spellcheck", this ) );
  spellcheck_action_->setToolTip( "Check spelling of current entry" );
  connect( spellcheck_action_, SIGNAL( triggered() ), SLOT( _spellCheck() ) );
  #endif

  // entry_info
  addAction( entry_info_action_ = new QAction( IconEngine::get( ICONS::INFO ), "Entry Information", this ) );
  entry_info_action_->setToolTip( "Show current entry information" );
  connect( entry_info_action_, SIGNAL( triggered() ), SLOT( _entryInfo() ) );

  // html
  addAction( print_action_ = new QAction( IconEngine::get( ICONS::PRINT ), "&Print", this ) );
  print_action_->setToolTip( "Convert current entry to html and print" );
  print_action_->setShortcut( Qt::CTRL + Qt::Key_P );
  connect( print_action_, SIGNAL( triggered() ), SLOT( _print() ) );

  // split action
  addAction( split_view_horizontal_action_ =new QAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM ), "Split View Top/Bottom", this ) );
  split_view_horizontal_action_->setToolTip( "Split current text editor vertically" );
  connect( split_view_horizontal_action_, SIGNAL( triggered() ), SLOT( _splitViewVertical() ) );

  addAction( split_view_vertical_action_ =new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "Split View Left/Right", this ) );
  split_view_vertical_action_->setToolTip( "Split current text editor horizontally" );
  connect( split_view_vertical_action_, SIGNAL( triggered() ), SLOT( _splitViewHorizontal() ) );

  // clone window action
  addAction( clone_window_action_ = new QAction( IconEngine::get( ICONS::VIEW_CLONE ), "Clone Window", this ) );
  clone_window_action_->setToolTip( "Create a new edition window displaying the same entry" );
  connect( clone_window_action_, SIGNAL( triggered() ), SLOT( _cloneWindow() ) );

  // close window action
  addAction( close_action_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE ), "&Close View", this ) );
  close_action_->setShortcut( CTRL+Key_W );
  close_action_->setToolTip( "Close current view" );
  connect( close_action_, SIGNAL( triggered() ), SLOT( _close() ) );

  addAction( uniconify_action_ = new QAction( "&Uniconify", this ) );
  connect( uniconify_action_, SIGNAL( triggered() ), SLOT( uniconify() ) );

}

//_____________________________________________
void EditionWindow::_updateConfiguration( void )
{

  // one should check whether this is needed or not.
  Debug::Throw( "EditionWindow::_updateConfiguration.\n" );
  resize( sizeHint() );

}

//_____________________________________________
void EditionWindow::_splitterMoved( void )
{
  Debug::Throw( "MainWindow::_splitterMoved.\n" );
  resize_timer_.start( 200, this );
}

//_______________________________________________
void EditionWindow::_previousEntry( void )
{
  Debug::Throw( "EditionWindow::_previousEntry.\n" );

  MainWindow &mainwindow( _mainWindow() );
  LogEntry* entry( mainwindow.previousEntry( EditionWindow::entry(), true ) );
  if( !( entry  && mainwindow.lockEntry( entry ) ) ) return;
  displayEntry( entry );
  setReadOnly( false );

}

//_______________________________________________
void EditionWindow::_nextEntry( void )
{
  Debug::Throw( "EditionWindow::_nextEntry.\n" );

  MainWindow &mainwindow( _mainWindow() );
  LogEntry* entry( mainwindow.nextEntry( EditionWindow::entry(), true ) );
  if( !( entry && mainwindow.lockEntry( entry ) ) ) return;
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
    InformationDialog( this, "No valid entry."  ).exec();
    return;
  }

  // create dialog
  LogEntryInformationDialog( this, entry ).centerOnParent().exec();

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
void EditionWindow::_updateSaveAction( void )
{ saveAction().setEnabled( !isReadOnly() && modified() ); }

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
    InformationDialog( this, "No entry selected. <Delete Entry> canceled." ).exec();
    return;
  }

  // ask confirmation
  if( !QuestionDialog( this, "Delete current entry ?" ).exec() ) return;

  // get associated attachments
  _mainWindow().deleteEntry( entry );

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
    InformationDialog( this, "No valid entry found. <New window> canceled." ).exec();
    return;
  }

  // retrieve selection frame
  MainWindow &mainwindow( _mainWindow() );

  // create new EditionWindow
  EditionWindow *edition_window( new EditionWindow( &mainwindow ) );
  Key::associate( edition_window, &mainwindow );
  edition_window->displayEntry( entry );

  // raise EditionWindow
  edition_window->show();

  return;
}

//_____________________________________________
void EditionWindow::_print( void )
{
  Debug::Throw( "EditionWindow::_print.\n" );

  // check logbook entry
  LogEntry *entry( EditionWindow::entry() );
  if( !entry ){
    InformationDialog( this, "No entry. <View HTML> canceled." ).exec();
    return;
  }

  // check if entry is modified
  if( modified() ) askForSave();

  // create custom dialog, retrieve check vbox child
  PrintLogEntryDialog dialog( this );
  dialog.setLogbookMask( 0 );
  dialog.setEntryMask( LogEntry::HTML_ALL_MASK );

  // add commands
  /* command list contains the HTML editor, PDF editor and any additional user specified command */
  Options::List commands( XmlOptions::get().specialOptions( "PRINT_COMMAND" ) );
  if( !AttachmentType::HTML.editCommand().isEmpty() ) commands.push_back( AttachmentType::HTML.editCommand() );
  for( Options::List::iterator iter = commands.begin(); iter != commands.end(); iter++ )
  { dialog.addCommand( iter->raw() ); }

  // generate default filename
  QString buffer;
  QTextStream( &buffer )  << "_eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
  dialog.setFile( File( buffer ).addPath( Util::tmp() ) );

  // map dialog
  if( dialog.centerOnParent().exec() == QDialog::Rejected ) return;

  // save command
  QString command( dialog.command() );
  XmlOptions::get().add( "PRINT_COMMAND", Option( command, Option::RECORDABLE|Option::CURRENT ) );

  // retrieve/check file
  File file( dialog.file() );
  if( file.isEmpty() ) {
    InformationDialog(this, "No output file specified. <View HTML> canceled." ).exec();
    return;
  }

  QFile out( file );
  if( !out.open( QIODevice::WriteOnly ) )
  {
    QString buffer;
    QTextStream( &buffer ) << "Cannot write to file \"" << file << "\". <View HTML> canceled.";
    InformationDialog( this, buffer ).exec();
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
  HtmlHeaderNode( html, document );

  // body
  QDomElement body = html.appendChild( document.createElement( "body" ) ).toElement();

  // dump logbook header
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
  { body.appendChild( (*iter)->htmlElement( document, html_log_mask ) ); }

  // dump entry
  body.appendChild( entry->htmlElement( document, html_entry_mask ) );

  out.write( document.toString().toAscii() );
  out.close();

  // retrieve command
  if( command.isEmpty() ) return;

  // execute command
  ( Command( command ) << file ).run();

  return;
}

//_____________________________________________
void EditionWindow::_unlock( void )
{

  Debug::Throw( "EditionWindow::_unlock.\n" );

  if( !isReadOnly() ) return;
  LogEntry *entry( EditionWindow::entry() );

  if( entry && ! _mainWindow().lockEntry( entry ) ) return;
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
  _setActiveEditor( *static_cast<AnimatedTextEditor*>(editor) );

}

//________________________________________________________________
void EditionWindow::_modifiersChanged( unsigned int modifiers )
{
  if( !_hasStatusBar() ) return;
  QStringList buffer;
  if( modifiers & TextEditor::MODIFIER_WRAP ) buffer << "WRAP";
  if( modifiers & TextEditor::MODIFIER_INSERT ) buffer << "INS";
  if( modifiers & TextEditor::MODIFIER_CAPS_LOCK ) buffer << "CAPS";
  if( modifiers & TextEditor::MODIFIER_NUM_LOCK ) buffer << "NUM";
  statusBar().label(1).setText( buffer.join( " " ) );
}

//________________________________________________________________
void EditionWindow::_setActiveEditor( AnimatedTextEditor& editor )
{
  Debug::Throw() << "EditionWindow::_setActiveEditor - key: " << editor.key() << endl;
  assert( editor.isAssociated( this ) );

  active_editor_ = &editor;
  if( !activeEditor().isActive() )
  {

    BASE::KeySet<AnimatedTextEditor> editors( this );
    for( BASE::KeySet<AnimatedTextEditor>::iterator iter = editors.begin(); iter != editors.end(); iter++ )
    { (*iter)->setActive( false ); }

    activeEditor().setActive( true );

  }

  // associate with toolbar
  if( format_toolbar_ ) format_toolbar_->setTarget( activeEditor() );

  Debug::Throw( "EditionWindow::_setActiveEditor - done.\n" );

}

//___________________________________________________________
void EditionWindow::_closeEditor( AnimatedTextEditor& editor )
{
  Debug::Throw( "EditionWindow::_closeEditor.\n" );

  // retrieve number of editors
  // if only one display, close the entire window
  BASE::KeySet<AnimatedTextEditor> editors( this );
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
  editors = BASE::KeySet<AnimatedTextEditor>( &editor );

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
  for( BASE::KeySet<AnimatedTextEditor>::reverse_iterator iter = editors.rbegin(); iter != editors.rend(); iter++ )
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
AnimatedTextEditor& EditionWindow::_splitView( const Orientation& orientation )
{
  Debug::Throw( "EditionWindow::_splitView.\n" );

  // keep local pointer to current active display
  AnimatedTextEditor& active_editor_local( activeEditor() );

  // compute desired dimension of the new splitter
  // along its splitting direction
  int dimension( (orientation == Horizontal) ? active_editor_local.width():active_editor_local.height() );

  // create new splitter
  QSplitter& splitter( _newSplitter( orientation ) );

  // create new display
  AnimatedTextEditor& editor( _newTextEditor(0) );

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
  BASE::KeySet<AnimatedTextEditor> editors( &active_editor_local );

  // clone new display
  editor.synchronize( &active_editor_local );

  // perform associations
  // check if active editors has associates and propagate to new
  for( BASE::KeySet<AnimatedTextEditor>::iterator iter = editors.begin(); iter != editors.end(); iter++ )
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

  // try cast to splitter
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
AnimatedTextEditor& EditionWindow::_newTextEditor( QWidget* parent )
{
  Debug::Throw( "EditionWindow::_newTextEditor.\n" );

  // create textDisplay
  AnimatedTextEditor* editor = new AnimatedTextEditor( parent );

  // connections
  connect( editor, SIGNAL( hasFocus( TextEditor* ) ), SLOT( _displayFocusChanged( TextEditor* ) ) );
  connect( editor, SIGNAL( cursorPositionChanged() ), SLOT( _displayCursorPosition() ) );
  connect( editor, SIGNAL( modifiersChanged( unsigned int ) ), SLOT( _modifiersChanged( unsigned int ) ) );
  connect( editor, SIGNAL( undoAvailable( bool ) ), SLOT( _updateUndoAction() ) );
  connect( editor, SIGNAL( redoAvailable( bool ) ), SLOT( _updateRedoAction() ) );
  connect( editor->document(), SIGNAL( modificationChanged( bool ) ), SLOT( _updateSaveAction() ) );

  if( format_toolbar_ )
  {
    connect(
      editor, SIGNAL( currentCharFormatChanged( const QTextCharFormat& ) ),
      format_toolbar_, SLOT( updateState( const QTextCharFormat& ) ) );
  }

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
  if( !_hasStatusBar() ) return;

  QString buffer;
  QTextStream( &buffer ) << "Line: " << position.paragraph()+1;
  statusBar().label(2).setText( buffer, false );

  buffer.clear();
  QTextStream( &buffer )  << "Column: " << position.index()+1;
  statusBar().label(3).setText( buffer, true );

  return;
}

//_______________________________________________
MainWindow& EditionWindow::_mainWindow( void ) const
{
  Debug::Throw( "EditionWindow::_mainWindow.\n" );
  BASE::KeySet<MainWindow> mainwindows( this );
  assert( mainwindows.size()==1 );
  return **mainwindows.begin();
}

//_____________________________________________
void EditionWindow::_displayText( void )
{
  Debug::Throw( "EditionWindow::_displayText.\n" );
  if( !&activeEditor() ) return;

  LogEntry* entry( EditionWindow::entry() );
  activeEditor().setCurrentCharFormat( QTextCharFormat() );
  activeEditor().setPlainText( (entry) ? entry->text() : "" );
  format_toolbar_->load( entry->formats() );

  // reset undo/redo stack
  activeEditor().resetUndoRedoStack();

  return;
}

//_____________________________________________
void EditionWindow::_displayAttachments( void )
{
  Debug::Throw( "EditionWindow::_DisplayAttachments.\n" );

  AttachmentFrame &frame( attachmentFrame() );
  frame.clear();

  LogEntry* entry( EditionWindow::entry() );
  if( !entry ) {

    frame.visibilityAction().setChecked( false );
    return;

  }

  // get associated attachments
  BASE::KeySet<Attachment> attachments( entry );
  if( attachments.empty() ) {

    frame.visibilityAction().setChecked( false );
    return;

  } else {

    frame.visibilityAction().setChecked( true );
    frame.add( AttachmentModel::List( attachments.begin(), attachments.end() ) );

  }

  return;

}

//___________________________________________________________________________________
EditionWindow::ColorWidget::ColorWidget( QWidget* parent ):
  QToolButton( parent ),
  //QPushButton( parent ),
  Counter( "ColorWidget" )
{ Debug::Throw( "ColorWidget::ColorWidget.\n" ); }

//___________________________________________________________________________________
void EditionWindow::ColorWidget::setColor( const QColor& color )
{

  // create pixmap
  QPixmap pixmap( IconSize( IconSize::HUGE ) );
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

//___________________________________________________________________________________
QSize EditionWindow::ColorWidget::sizeHint( void ) const
{
  // the const_cast is use to temporarily remove the menu
  // in order to keep the size of the toolbutton minimum
  QMenu* menu( ColorWidget::menu() );
  const_cast<EditionWindow::ColorWidget*>( this )->setMenu(0);
  QSize out( QToolButton::sizeHint() );
  const_cast<EditionWindow::ColorWidget*>( this )->setMenu(menu);
  return out;
}

//___________________________________________________________________________________
QSize EditionWindow::ColorWidget::minimumSizeHint( void ) const
{
  // this is an ugly hack to keep the size of the toolbutton minimum
  QMenu* menu( ColorWidget::menu() );
  const_cast<EditionWindow::ColorWidget*>( this )->setMenu(0);
  QSize out( QToolButton::minimumSizeHint() );
  const_cast<EditionWindow::ColorWidget*>( this )->setMenu(menu);
  return out;
}

//___________________________________________________________________________________
void EditionWindow::ColorWidget::paintEvent( QPaintEvent* )
{
  // rotated paint
  QStylePainter painter(this);
  QStyleOptionToolButton option;
  initStyleOption(&option);

  // first draw normal frame and not text/icon
  option.features &= (~QStyleOptionToolButton::HasMenu);
  painter.drawComplexControl(QStyle::CC_ToolButton, option);

}
