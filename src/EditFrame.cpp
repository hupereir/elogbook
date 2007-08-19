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
  \file EditFrame.cc
  \brief log entry edition/creation object
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLabel>
#include <QLayout>

#include "AttachmentFrame.h"
#include "AttachmentList.h"
#include "BaseIcons.h"
#include "BrowsedLineEdit.h"
#include "ColorMenu.h"
#include "CustomLineEdit.h"
#include "CustomPixmap.h"
#include "CustomTextEdit.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"
#include "EditFrame.h"
#include "File.h"
#include "FormatBar.h"
#include "HtmlUtil.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryInformationDialog.h"
#include "LogEntryList.h"
#include "MainFrame.h"
#include "Menu.h"
#include "Options.h"
#include "QtUtil.h"
#include "SelectionFrame.h"
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
EditFrame::EditFrame( QWidget* parent, bool read_only ):
  CustomMainWindow( parent ),
  Counter( "EditFrame" ),
  read_only_( read_only ),
  closed_( false ),
  color_frame_( 0 )
{
  Debug::Throw("EditFrame::EditFrame.\n" );
  setObjectName( "EDITFRAME" );
    
  QWidget* main( new QWidget( this ) ); 
  setCentralWidget( main );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing( 5 );
  main->setLayout( layout );
  
  title_layout_ = new QHBoxLayout();
  title_layout_->setMargin(0);
  title_layout_->setSpacing(2);
  layout->addLayout( title_layout_ );
  
  // title label and line
  title_layout_->addWidget( new QLabel( " Title: ", main ), 0, Qt::AlignVCenter );
  title_layout_->addWidget( title_ = new CustomLineEdit( main ), 1, Qt::AlignVCenter );
    
  // splitter for EditFrame and attachment list
  splitter_ = new QSplitter( main );
  splitter_->setOrientation( Qt::Vertical );
  layout->addWidget( splitter_, 1 );
  
  // create text
  text_ = new CustomTextEdit( splitter_ );
  
  connect( title_, SIGNAL( modificationChanged( bool ) ), SLOT( _titleModified( bool ) ) );
  connect( editor().document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );
  connect( text_, SIGNAL( cursorPositionChanged() ), SLOT( _displayCursorPosition() ) );
  connect( title_, SIGNAL( cursorPositionChanged( int, int ) ), SLOT( _displayCursorPosition( int, int ) ) );

  // create attachment list
  AttachmentList *attachment_list = new AttachmentList( splitter_, isReadOnly() );
  attachment_list->hide();

  // associate EditFrame and attachment list
  Key::associate( this, attachment_list );

  // state frame for tooltips
  layout->addWidget( statusbar_ = new StatusBar( main ) );
  statusBar().addLabel( 2 );
  statusBar().addLabels( 2, 0 );
  statusBar().addClock();

  // toolbars
  // toolbar buttons pixmap path list
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  // lock toolbar is visible only when window is not editable
  lock_ = new CustomToolBar( "Lock", this );
  CustomToolButton *button;
  button = new CustomToolButton( lock_, IconEngine::get( ICONS::LOCK, path_list ), "Unlock current editor", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _unlock() ) );
  button->setText("Unlock");  
  lock_->addWidget( button );
  lock_->setMovable( false );
  addToolBar( LeftToolBarArea, lock_ );

  // main toolbar
  CustomToolBar* toolbar( new CustomToolBar( "Main", this ) );
  toolbar->setObjectName( "MAIN_TOOLBAR" );
  toolbars_.push_back( make_pair( toolbar, "MAIN_TOOLBAR" ) );
  addToolBar( LeftToolBarArea, toolbar );
    
  // generic tool button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::NEW, path_list ), "Create a new entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( newEntry() ) );
  button->setText("New");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // save_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::SAVE, path_list ), "Save the current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( save() ) );
  button->setText("Save");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // delete_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::DELETE, path_list ), "Delete the current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _deleteEntry() ) );
  button->setText("Delete");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // add_attachment button
  toolbar->addWidget( new CustomToolButton( toolbar, &attachment_list->newAttachmentAction(), &statusbar_->label() ) );

  #if WITH_ASPELL
  // spellcheck button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::SPELLCHECK, path_list ), "Check spelling of current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _spellCheck() ) );
  read_only_widgets_.push_back( button );
  toolbar->addWidget( button );
  button->setText("Spell");
  #endif

  // format bar
  format_toolbar_ = new FormatBar( this );
  format_toolbar_->setObjectName( "FORMAT_TOOLBAR" );
  format_toolbar_->setToolTipLabel( &statusbar_->label() );
  format_toolbar_->setTarget( text_ );
  toolbars_.push_back( make_pair( format_toolbar_, "FORMAT_TOOLBAR" ) );
  addToolBar( LeftToolBarArea, format_toolbar_ );

  // edition toolbars
  toolbar = new CustomToolBar( "History", this );
  toolbar->setObjectName( "EDITION_TOOLBAR" );
  toolbars_.push_back( make_pair( toolbar, "EDITION_TOOLBAR" ) );
  addToolBar( LeftToolBarArea, toolbar );

  // undo button
  undo_action_ = new QAction( IconEngine::get( ICONS::UNDO, path_list ), "&Undo", this );
  undo_action_->setToolTip( "Undo last modification" );
  connect( undo_action_, SIGNAL( triggered() ), SLOT( _undo() ) );
  toolbar->addAction( undo_action_ );
  
  // redo button
  redo_action_ = new QAction( IconEngine::get( ICONS::REDO, path_list ), "&Redo", this );
  redo_action_->setToolTip( "Redo last undone modification" );
  connect( redo_action_, SIGNAL( triggered() ), SLOT( _redo() ) );
  toolbar->addAction( redo_action_ );

  // undo/redo connections
  connect( title_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateUndoAction() ) );
  connect( title_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateRedoAction() ) );
  connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );

  connect( text_, SIGNAL( undoAvailable( bool ) ), SLOT( _updateUndoAction() ) );
  connect( text_, SIGNAL( redoAvailable( bool ) ), SLOT( _updateRedoAction() ) );
  connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );
    
  // extra toolbar
  toolbar = new CustomToolBar( "Tools", this );
  toolbar->setObjectName( "EXTRA_TOOLBAR" );
  toolbars_.push_back( make_pair( toolbar, "EXTRA_TOOLBAR" ) );
  addToolBar( LeftToolBarArea, toolbar );

  // view_html button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::HTML, path_list ), "Convert the current entry to HTML", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _viewHtml() ) );
  button->setText("Html");
  toolbar->addWidget( button );

  // clone button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::COPY, path_list ), "Open a read-only editor for the current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _newWindow() ) );
  button->setText("Clone");
  toolbar->addWidget( button );

  // entry_info button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::INFO, path_list ), "Display the current entry informations", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _entryInfo() ) );
  button->setText("Info");
  toolbar->addWidget( button );

  // extra toolbar
  toolbar = new CustomToolBar( "Navigation", this );
  toolbars_.push_back( make_pair( toolbar, "NAVIGATION_TOOLBAR" ) );
  toolbar->setObjectName( "NAVIGATION_TOOLBAR" );
  addToolBar( LeftToolBarArea, toolbar );

  // main_window button
  toolbar->addAction( &dynamic_cast<MainFrame*>(qApp)->selectionFrame().uniconifyAction() );

  // previous_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::PREV, path_list ), "Display the previous entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _previousEntry() ) );
  button->setText("Previous");
  toolbar->addWidget( button );
  previous_entry_ = button;

  // next_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( ICONS::NEXT, path_list ), "Display the next entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _nextEntry() ) );
  button->setText("Next");
  toolbar->addWidget( button );
  next_entry_ = button;

  // create menu if requested
  Menu* menu = new Menu( this, &dynamic_cast<MainFrame*>(qApp)->selectionFrame() ); 
  setMenuBar( menu );
  
  connect( menu, SIGNAL( save() ), SLOT( save() ) );
  connect( menu, SIGNAL( closeWindow() ), SLOT( close() ) );
  connect( menu, SIGNAL( viewHtml() ), SLOT( _viewHtml() ) );

  // changes display according to read_only flag
  setReadOnly( read_only_ );
  
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();
  Debug::Throw("EditFrame::EditFrame - done.\n" );

}

//____________________________________________
EditFrame::~EditFrame( void )
{ Debug::Throw( "EditFrame::~EditFrame.\n" ); }

//____________________________________________
void EditFrame::displayEntry( LogEntry *entry )
{
  Debug::Throw( "EditFrame::displayEntry.\n" );

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
  SelectionFrame *frame( _selectionFrame() );
  
  // update previous and next action states
  Debug::Throw( "EditFrame::displayEntry - setting button states.\n" );
  previous_entry_->setEnabled( frame->previousEntry(entry, false) );
  next_entry_->setEnabled( frame->nextEntry(entry, false) );
  
  // reset modify flag; change title accordingly
  setModified( false );
  updateWindowTitle();

  Debug::Throw( "EditFrame::displayEntry - done.\n" );
  return;
}

//____________________________________________
void EditFrame::setReadOnly( const bool& value )
{

  Debug::Throw() << "EditFrame::setReadOnly - " << (value ? "true":"false" ) << endl;
  
  // update read_only value
  read_only_ = value;

  // changes button state
  for( list< QWidget* >::iterator it=read_only_widgets_.begin(); it != read_only_widgets_.end(); it++ )
  { (*it)->setEnabled( !isReadOnly() ); }

  // changes lock button state
  if( isReadOnly() && lock_->isHidden() ) lock_->show();
  if( !(isReadOnly() || lock_->isHidden() ) ) lock_->hide();

  // changes TextEdit readOnly status
  title_->setReadOnly( isReadOnly() );
  editor().setReadOnly( isReadOnly() );

  // changes attachment list status
  attachmentList().setReadOnly( isReadOnly() );

  // changes window title
  updateWindowTitle();
  
}
  
//_____________________________________________
string EditFrame::windowTitle( void ) const
{

  Debug::Throw( "EditFrame::windowTitle.\n" );
  LogEntry* entry( EditFrame::entry() );

  ostringstream title;
  if( entry )
  {
  
    title << entry->title();
    if( entry->keyword() != LogEntry::NO_KEYWORD ) title << " - " << entry->keyword();

  } else title << "Electronic Logbook Editor";

  if( isReadOnly() ) title << " (read only)";
  else if( modified()  ) title << " (modified)";
  return title.str();

}

//____________________________________________
AskForSaveDialog::ReturnCode EditFrame::askForSave( const bool& enable_cancel )
{
  
  Debug::Throw( "EditFrame::askForSave.\n" );
  
  // retrieve other editFrames
  BASE::KeySet<EditFrame> frames( _selectionFrame() );
  unsigned int count( count_if( frames.begin(), frames.end(), ModifiedFTor() ) );
  
  // create dialog
  unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
  if( enable_cancel ) buttons |= AskForSaveDialog::CANCEL;
  if( count > 1 ) buttons |= AskForSaveDialog::ALL; 
  AskForSaveDialog dialog( this, "Entry has been modified. Save ?", buttons );
  QtUtil::centerOnParent( &dialog );
  
  // exec and check return code
  int state = dialog.exec();
  if( state == AskForSaveDialog::YES ) save( false );
  else if( state == AskForSaveDialog::ALL ) 
  {
    /*
      save_all: if the logbook has no valid file one save the modified edit frames one by one
      otherwise one directly save the loogbook, while disabling the confirmation for modified
      entries
    */
    if( _selectionFrame()->logbook()->file().empty() )
    {
      for( BASE::KeySet<EditFrame>::iterator iter = frames.begin(); iter!= frames.end(); iter++ )
      { if( (*iter)->modified() && !(*iter)->isReadOnly() ) (*iter)->save(enable_cancel); }
    } else _selectionFrame()->save( false );
  }
  
  return AskForSaveDialog::ReturnCode(state);
  
}

//_____________________________________________
void EditFrame::displayTitle( void )
{
  Debug::Throw( "EditFrame::displayTitle.\n" );

  LogEntry* entry( EditFrame::entry() );
  title_->setText( ( entry && entry->title().size() ) ? entry->title().c_str(): LogEntry::UNTITLED.c_str()  );
  title_->setCursorPosition( 0 );
  return;
}

//_____________________________________________
void EditFrame::displayColor( void )
{
  Debug::Throw( "EditFrame::DisplayColor.\n" );
    
  string colorname( EditFrame::entry()->color() );
  QColor color;
  if( colorname != ColorMenu::NONE ) color = QColor( colorname.c_str() );

  if( !color.isValid() )
  {
  
    // if a colored frame existed
    // delete it and set it to 0
    if( color_frame_ ) 
    {
      delete color_frame_;
      color_frame_ = 0;
    }
  
  } else {
    
    // if color frame does not exist, create it
    // color label
    if( !color_frame_ )
    {
      color_frame_ = new QFrame( title_->parentWidget() );
      color_frame_->setFrameStyle( QFrame::NoFrame );
      // color_frame_->setFixedSize( ColorMenu::PixmapSize );
      color_frame_->setFixedSize( QSize( title_->height()-2, title_->height()-2 ) );
      color_frame_->setAutoFillBackground( true );
      
      title_layout_->addWidget( color_frame_ );
    }
    
    // create gradient for nice look and fill
    QLinearGradient linearGrad(QPointF(0, 0), color_frame_->rect().bottomRight());
    linearGrad.setColorAt(0, color);
    linearGrad.setColorAt(1, color.light());
    
    // modify palette with create gradient
    QPalette palette( color );
    palette.setBrush( QPalette::Window, linearGrad );
    color_frame_->setPalette( palette );
    color_frame_->update();
    
  }
  
}

//______________________________________________________
void EditFrame::setModified( const bool& value )
{
  Debug::Throw( "EditFrame::setModified.\n" );
  title_->setModified( value );
  editor().document()->setModified( value );
}

//_____________________________________________
void EditFrame::save( bool update_selection )
{
  
  Debug::Throw( "EditFrame::save.\n" );
  if( isReadOnly() ) return;

  // retrieve associated entry
  LogEntry *entry( EditFrame::entry() );

  // see if entry is new
  bool entry_is_new( !entry || BASE::KeySet<Logbook>( entry ).empty() );
  
  // create entry if none set
  if( !entry ) entry = new LogEntry();

  // check logbook
  SelectionFrame *frame( _selectionFrame() );
  Logbook *logbook( frame->logbook() );
  if( !logbook ) {
    QtUtil::infoDialog( this, "No logbook opened. <Save> canceled." );
    return;
  }

  //! update entry text
  entry->setText( qPrintable( editor().toPlainText() ) );
  entry->setFormats( format_toolbar_->get() );

  //! update entry title
  entry->setTitle( qPrintable( title_->text() ) );

  // update author
  entry->setAuthor( XmlOptions::get().raw( "USER" ) );

  // add _now_ to entry modification timestamps
  entry->modified();

  // status bar
  statusbar_->label().setText(  "writting entry to logbook ..." );

  // add entry to logbook, if needed
  if( entry_is_new ) Key::associate( entry, logbook->latestChild() );

  // update this window title, set unmodified.
  setModified( false );
  updateWindowTitle();

  // update selection frame
  frame->updateEntry( entry, update_selection );
  frame->setWindowTitle( MainFrame::MAIN_TITLE_MODIFIED );

  // set logbook as modified
  BASE::KeySet<Logbook> logbooks( entry );
  for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter!= logbooks.end(); iter++ )
  (*iter)->setModified( true );

  // Save logbook
  if( frame->logbook()->file().size() ) frame->save();

  statusbar_->label().setText( "" );

  return;

}

//_____________________________________________
void EditFrame::newEntry( void )
{

  Debug::Throw( "EditFrame::newEntry.\n" );

  // check if entry is modified
  if( modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

  // create new entry, set author, set keyword
  LogEntry* entry = new LogEntry();
  entry->setAuthor( XmlOptions::get().raw( "USER" ) );
  entry->setKeyword( _selectionFrame()->keywordList().current() );

  // add logbook parent if any
  Logbook *logbook( _selectionFrame()->logbook() );
  if( logbook ) Key::associate( entry, logbook->latestChild() );

  // display new entry
  displayEntry( entry );

}

//____________________________________________
void EditFrame::closeEvent( QCloseEvent *event )
{
  Debug::Throw( "EditFrame::closeEvent.\n" );
  
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
void EditFrame::enterEvent( QEvent *event )
{
  Debug::Throw( "EditFrame::enterEvent.\n" );

  // base class enterEvent
  QMainWindow::enterEvent( event );
  _selectionFrame()->checkLogbookModified();

}


//_____________________________________________
void EditFrame::_updateConfiguration( void )
{
  
  Debug::Throw( "EditFrame::_updateConfiguration.\n" );
  
  CustomMainWindow::_updateConfiguration();
  
  // window size
  resize( XmlOptions::get().get<int>( "EDIT_FRAME_WIDTH" ), XmlOptions::get().get<int>( "EDIT_FRAME_HEIGHT" ) );
  
  // set splitter default size
  QList<int> sizes;
  sizes << XmlOptions::get().get<int>( "EDT_HEIGHT" );
  sizes << XmlOptions::get().get<int>( "ATC_HEIGHT" );
  splitter_->setSizes( sizes );

  // toolbars visibility and location
  for( list< pair<QToolBar*, string> >::iterator iter = toolbars_.begin(); iter != toolbars_.end(); iter++ )
  {
    
    QToolBar* toolbar( iter->first );
    string option_name( iter->second );
    string location_name( option_name + "_LOCATION" );
    
    bool visibility( XmlOptions::get().find( option_name ) ? XmlOptions::get().get<bool>( option_name ):true );
    bool current_visibility( toolbar->isVisible() );
    
    ToolBarArea location( (XmlOptions::get().find( location_name )) ? (ToolBarArea) CustomToolBar::nameToArea( XmlOptions::get().get<string>( location_name ) ):LeftToolBarArea );
    ToolBarArea current_location( toolBarArea( toolbar ) );
    
    Debug::Throw() << "EditFrame::_updateConfiguration - " << option_name << " visibility: " << visibility << " location: " << (int)location << endl;
    
    if( visibility )
    {
      if( !( current_visibility && location == current_location ) ) 
      {
        addToolBar( location, toolbar );
        toolbar->show();
      }
    } else toolbar->hide();
   
    XmlOptions::get().set<bool>( option_name, !toolbar->isHidden() );
    XmlOptions::get().set<string>( location_name, CustomToolBar::areaToName( toolBarArea( toolbar ) ) );
  }

}

//_____________________________________________
void EditFrame::_saveConfiguration( void )
{
  
  Debug::Throw( "EditFrame::_saveConfiguration.\n" );
  XmlOptions::get().set<int>( "EDIT_FRAME_HEIGHT", height() );
  XmlOptions::get().set<int>( "EDIT_FRAME_WIDTH", width() );
  
  // retrieve attachment list
  AttachmentList* atc_list( *BASE::KeySet<AttachmentList>(this).begin() );
  if( !atc_list->isHidden() )
  {
    XmlOptions::get().set<int>( "EDT_HEIGHT", editor().height() );
    XmlOptions::get().set<int>( "ATC_HEIGHT", (*BASE::KeySet<AttachmentList>(this).begin())->height() );
  }
  
  // save toolbars location and visibility
  for( list< pair<QToolBar*, string> >::iterator iter = toolbars_.begin(); iter != toolbars_.end(); iter++ )
  {
    
    QToolBar* toolbar( iter->first );
    string option_name( iter->second );
    string location_name( option_name + "_LOCATION" );
    XmlOptions::get().set<bool>( option_name, !toolbar->isHidden() );
    XmlOptions::get().set<string>( location_name, CustomToolBar::areaToName( toolBarArea( toolbar ) ) );
  }
  

}

//_______________________________________________
void EditFrame::_previousEntry( void )
{
  Debug::Throw( "EditFrame::_previousEntry.\n" );

  //if( isReadOnly() ) return;
    
  SelectionFrame *frame( _selectionFrame() );
  LogEntry* entry( frame->previousEntry( EditFrame::entry(), true ) );
  if( !( entry  && frame->lockEntry( entry ) ) ) return;
  displayEntry( entry );
  setReadOnly( false );

}

//_______________________________________________
void EditFrame::_nextEntry( void )
{
  Debug::Throw( "EditFrame::_nextEntry.\n" );

  //if( isReadOnly() ) return;

  SelectionFrame *frame( _selectionFrame() );
  LogEntry* entry( frame->nextEntry( EditFrame::entry(), true ) );
  if( !( entry && frame->lockEntry( entry ) ) ) return;
  displayEntry( entry );
  setReadOnly( false );

}

//_____________________________________________
void EditFrame::_entryInfo( void )
{

  Debug::Throw( "EditFrame::_EntryInfo.\n" );

  // check entry
  LogEntry *entry( EditFrame::entry() );
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
void EditFrame::_undo( void )
{
  Debug::Throw( "EditFrame::_undo.\n" );
  if( editor().hasFocus() ) editor().document()->undo();
  else if( title_->hasFocus() ) title_->undo();
  return;
}

//_____________________________________________
void EditFrame::_redo( void )
{
  Debug::Throw( "EditFrame::_redo.\n" );
  if( editor().hasFocus() ) editor().document()->redo();
  else if( title_->hasFocus() ) title_->redo();
  return;
}

//_____________________________________________
void EditFrame::_updateUndoAction( void )
{ 
  Debug::Throw( "EditFrame::_updateUndoAction.\n" );
  if( title_->hasFocus() ) undo_action_->setEnabled( title_->isUndoAvailable() );
  if( editor().hasFocus() ) undo_action_->setEnabled( editor().document()->isUndoAvailable() );
}

//_____________________________________________
void EditFrame::_updateRedoAction( void )
{ 
  Debug::Throw( "EditFrame::_updateRedoAction.\n" );
  if( title_->hasFocus() ) redo_action_->setEnabled( title_->isRedoAvailable() );
  if( editor().hasFocus() ) redo_action_->setEnabled( editor().document()->isRedoAvailable() );
}

//_____________________________________________
void EditFrame::_updateUndoRedoActions( QWidget*, QWidget* current )
{
  Debug::Throw( "EditFrame::_updateUndoRedoAction.\n" );
  if( current == title_ )
  {
    undo_action_->setEnabled( title_->isUndoAvailable() );
    redo_action_->setEnabled( title_->isRedoAvailable() );
  }

  if( current == text_ )
  {
    undo_action_->setEnabled( editor().document()->isUndoAvailable() );
    redo_action_->setEnabled( editor().document()->isRedoAvailable() );
  }

}

//_____________________________________________
void EditFrame::_deleteEntry( void )
{

  Debug::Throw( "EditFrame::_deleteEntry.\n" );

  // check current entry
  LogEntry *entry( EditFrame::entry() );

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
void EditFrame::_spellCheck( void )
{
#if WITH_ASPELL
  Debug::Throw( "EditFrame::_spellCheck.\n" );
  
  // create dialog
  SPELLCHECK::SpellDialog dialog( text_ );
  
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
void EditFrame::_newWindow( void )
{
  
  Debug::Throw( "EditFrame::_newWindow.\n" );
  LogEntry *entry( EditFrame::entry() );
  if( !entry ) {
    QtUtil::infoDialog( this, "No valid entry found. <New window> canceled." );
    return;
  }

  // retrieve selection frame
  SelectionFrame *frame( _selectionFrame() );

  // create new EditFrame
  EditFrame *edit_frame( new EditFrame( frame ) );
  Key::associate( edit_frame, frame );
  edit_frame->displayEntry( entry );

  // raise EditFrame
  edit_frame->show();

  return;
}

//_____________________________________________
void EditFrame::_viewHtml( void )
{
  Debug::Throw( "EditFrame::_viewHtml.\n" );

  // check logbook entry
  LogEntry *entry( EditFrame::entry() );
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
  what << Util::tmp() << "/_eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
  dialog.setFile( File( what.str() ) );

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
void EditFrame::_unlock( void )
{
  
  Debug::Throw( "EditFrame::_unlock.\n" );
  
  if( !isReadOnly() ) return;
  LogEntry *entry( EditFrame::entry() );
  
  if( entry && ! _selectionFrame()->lockEntry( entry ) ) return;
  setReadOnly( false );
  
  return;
  
}

//_____________________________________________
void EditFrame::_titleModified( bool state )
{
  Debug::Throw() << "EditFrame::_titleModified - state: " << (state ? "true":"false" ) << endl;

  // check readonly status
  if( isReadOnly() ) return;

  bool text_modified( editor().document()->isModified() );
  if( state && !text_modified ) updateWindowTitle();
  if( !(state || text_modified ) ) updateWindowTitle();

  return;
}

//_____________________________________________
void EditFrame::_textModified( bool state )
{
  Debug::Throw() << "EditFrame::_textModified - state: " << (state ? "true":"false" ) << endl;

  // check readonly status
  if( isReadOnly() ) return;
  
  bool title_modified( title_->isModified() );
  if( state && !title_modified ) updateWindowTitle();
  if( !(state || title_modified ) ) updateWindowTitle();
  
}

//_____________________________________________
void EditFrame::_displayCursorPosition( const TextPosition& position)
{
  Debug::Throw( "EditFrame::_DisplayCursorPosition.\n" );
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
SelectionFrame* EditFrame::_selectionFrame( void ) const
{
  Debug::Throw( "EditFrame::_selectionFrame.\n" );
  BASE::KeySet<SelectionFrame> frames( this );
  Exception::check( frames.size()==1, DESCRIPTION( "wrong association to SelectionFrame" ) );
  return *frames.begin();
}

//_____________________________________________
void EditFrame::_displayText( void )
{
  Debug::Throw( "EditFrame::_displayText.\n" );
  if( !text_ ) return;

  LogEntry* entry( EditFrame::entry() );
  editor().setPlainText( (entry) ? entry->text().c_str() : "" );
  format_toolbar_->load( entry->formats() );

  return;
}

//_____________________________________________
void EditFrame::_displayAttachments( void )
{
  Debug::Throw( "EditFrame::_DisplayAttachments.\n" );

  AttachmentList &attachment_list( attachmentList() );
  attachment_list.clear();

  LogEntry* entry( EditFrame::entry() );
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
  attachment_list.show();

  return;
}
