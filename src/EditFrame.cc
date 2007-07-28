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
#include <QSplitter>
#include <QToolBar>

#include "AttachmentFrame.h"
#include "AttachmentList.h"
#include "BrowsedLineEdit.h"
// #include "ColorMenu.h"
#include "CustomLineEdit.h"
#include "CustomPixmap.h"
#include "CustomTextEdit.h"
#include "CustomToolButton.h"
#include "EditFrame.h"
#include "File.h"
//#include "FormatBar.h"
#include "HtmlUtil.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryInfoDialog.h"
#include "LogEntryList.h"
#include "MainFrame.h"
#include "Menu.h"
#include "Options.h"
#include "QtUtil.h"
#include "SelectionFrame.h"
#include "StateFrame.h"
#include "Str.h"
#include "Util.h"
#include "ViewHtmlEntryDialog.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellDialog.h"
#endif

using namespace std;
using namespace BASE;

//! (modified) window title name
const string EditFrame::EDIT_TITLE_MODIFIED = "Electronic logbook editor (modified)";

//! (read only) window title name
const string EditFrame::EDIT_TITLE_READONLY = "Electronic logbook editor (read only)";

//! window title name
const string EditFrame::EDIT_TITLE = "Electronic logbook editor";

//_______________________________________________
EditFrame::EditFrame( QWidget* parent, LogEntry* entry, bool read_only ):
  CustomMainWindow( parent ),
  Counter( "EditFrame" ),
  read_only_( read_only ),
  modified_( false )
{
  Debug::Throw("EditFrame::EditFrame.\n" );

  QWidget* main_widget( new QWidget( this ) ); 
  QLayout *layout = new QLayout();
  layout->setMargin(2);
  layout->setSpacing( 2 );
  main_widget->setLayout( layout );
  setCentralWidget( main_widget );

  // create menu if requested
  Menu* menu = new Menu( this, &static_cast<MainFrame*>(qApp)->selectionFrame() ); 
  setMenuBar( menu );
  
  connect( menu, SIGNAL( save() ), SLOT( saveEntry() ) );
  connect( menu, SIGNAL( closeWindow(), SLOT( close ) );
  
  QHBoxLayout* h_layout = new QHBoxLayout( this );
  h_layout->setMargin(0);
  h_layout->setSpacing(2);
  layout->addLayout( h_layout );
  
  // title label and line
  h_layout->addWidget( new QLabel( " Title: ", this ) );
  h_layout->addWidget( title_ = new CustomLineEdit( this ), 1 );
  connect( title_, SIGNAL( textChanged( const QString&) ), SLOT( _modified() ) );
  
//   color_label_ = new CustomLabel( h_box );
//   color_label_->setAlignment( Qt::AlignCenter );
//   color_label_->setFrameStyle( QFrame::Panel | QFrame::Sunken );
//   QtUtil::Expand( color_label_, "    " );

  // splitter for EditFrame and attachment list
  QSplitter *splitter( new QSplitter( this ) );
  splitter->setOrientation( Qt::Vertical );
  layout->addWidget( splitter, 1 );
  
  // create text
  text_ = new CustomTextEdit( splitter, "edit_frame_text" );
  connect( text_, SIGNAL( textChanged() ), SLOT( _Modified() ) );

  // connect cursor position changed signals
  connect( text_, SIGNAL( cursorPositionChanged() ), SLOT( _displayCursorPosition() ) );
  connect( text_, SIGNAL( cursorPositionChanged( int, int ) ), SLOT( _displayCursorPosition( int, int ) ) );

  // create attachment list
  AttachmentList *attachment_list = new AttachmentList( splitter, isReadOnly() );
  attachment_list->hide();

  // associate EditFrame and attachment list
  Key::associate( this, attachment_list );

  // set splitter default size
  QValueList<int> sizes;
  sizes << Options::Get<int>( "EDIT_FRAME_HEIGHT" );
  sizes << Options::Get<int>( "ATC_HEIGHT" );
  splitter->setSizes( sizes );

  // state frame for tooltips
  layout->addWidget( statusbar_ = new StatusBar( this ) );
  statusBar().addLabel( 2 );
  statusBar().addLabels( 2, 0 );
  statusBar().addClock();

  // toolbars
  QToolBar* toolbar( new QToolBar( "Editor main toolbar", this ) );
  addToolBar( Qt::TopToolBarArea, toolbar );
  
  // toolbar buttons pixmaps
  list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
  if( !path_list.size() ) throw runtime_error( DESCRIPTION( "no path to pixmaps" ) );

  // lock button. is visible only when window is not editable
  lock_ = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::LOCK, path_list ) ), "Unlock current editor", &statusbar_->label() );
  connect( lock_, SIGNAL( clicked() ), SLOT( _unlock() ) );
  lock_->setText("Unlock");
  toolbar->addWidget( lock_ );
  
  // generic tool button
  CustomToolButton *button;
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::NEW, path_list ) ), "Create a new entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( newEntry() ) );
  button->setText("New");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // save_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::SAVE, path_list ) ), "Save the current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( saveEntry() ) );
  button->setText("Save");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // delete_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::DELETE, path_list ) ), "Delete the current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _deleteEntry() ) );
  button->setText("Delete");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // add_attachment button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::ATTACH, path_list ) ), "Add an attachment to the current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), attachment_list, SLOT( newAttachment() ) );
  button->setText("Attach");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  #if WITH_ASPELL
  // spellcheck button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::SPELLCHECK, path_list ) ), "Check spelling of current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _spellCheck() ) );
  read_only_widgets_.push_back( button );
  toolbar->addWidget( button );
  button->setText("Spell");
  #endif

//   // format bar
//   FormatBar *format_bar = new FormatBar( this, "format", "FORMAT_TOOLBAR_LOCATION" );
//   format_bar->SetToolTipLabel( &statusbar_->label() );
//   connect( text_, SIGNAL( currentFontChanged ( const QFont& ) ), format_bar, SLOT( UpdateState( const QFont& ) ) );
//   read_only_widgets_.push_back( format_bar );
//   format_bar->SetVisibilityOption( "FORMAT_TOOLBAR" );
// 
//   text_format_ = new FORMAT::TextFormat( text_ );
//   connect( &format_bar->Button(FormatBar::BOLD), SIGNAL( toggled( bool ) ), text_format_, SLOT( Bold( bool ) ) );
//   connect( &format_bar->Button(FormatBar::ITALIC), SIGNAL( toggled( bool ) ), text_format_, SLOT( Italic( bool ) ) );
//   connect( &format_bar->Button(FormatBar::STRIKE), SIGNAL( toggled( bool ) ), text_format_, SLOT( Strike( bool ) ) );
//   connect( &format_bar->Button(FormatBar::UNDERLINE), SIGNAL( toggled( bool ) ), text_format_, SLOT( Underline( bool ) ) );
//   connect( &format_bar->GetColorMenu(), SIGNAL( ColorSelected( const std::string& ) ), text_format_, SLOT( ColorChanged( const std::string& ) ) );

//   // toolbars
//   QToolBar* toolbar( new QToolBar( "Editor edition toolbar", this ) );
//   addToolBar( Qt::TopToolBarArea, toolbar );
// 
//   // undo button
//   button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::UNDO, path_list ) ), "Undo last text modification", &statusbar_->label() );
//   connect( button, SIGNAL( clicked() ), SLOT( _undo() ) );
//   button->setText("Undo");
//   toolbar->addWidget( button );
//   read_only_widgets_.push_back( button );
// 
//   // redo button
//   button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::REDO, path_list ) ), "Redo last text modification", &statusbar_->label() );
//   connect( button, SIGNAL( clicked() ), SLOT( _redo() ) );
//   button->setText("Redo");
//   toolbar->addWidget( button );
//   read_only_widgets_.push_back( button );

  // extra toolbar
  QToolBar* toolbar( new QToolBar( "Editor extra toolbar (1)", this ) );
  addToolBar( Qt::TopToolBarArea, toolbar );

  // main_window button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::HOME, path_list ) ), "Raise the main window", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), &static_cast<MainFrame*>(qApp)->selectionFrame(), SLOT( uniconify() ) );
  button->setText("Home");
  toolbar->addWidget( button );

  // view_html button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::HTML, path_list ) ), "Convert the current entry to HTML", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _viewHtml() ) );
  button->setText("Html");
  toolbar->addWidget( button );

  // new_window button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::COPY, path_list ) ), "Open a read-only editor for the current entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _newWindow() ) );
  button->setText("Clone");
  toolbar->addWidget( button );

  // extra toolbar
  QToolBar* toolbar( new QToolBar( "Editor extra toolbar (2)", this ) );
  addToolBar( Qt::TopToolBarArea, toolbar );

  // previous_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::PREV, path_list ) ), "Display the previous entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _previousEntry() ) );
  button->setText("Previous");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // next_entry button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::NEXT, path_list ) ), "Display the next entry", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _nextEntry() ) );
  button->setText("Next");
  toolbar->addWidget( button );
  read_only_widgets_.push_back( button );

  // entry_info button
  button = new CustomToolButton( toolbar, IconEngine::get( CustomPixmap().find( ICONS::INFO, path_list ) ), "Display the current entry informations", &statusbar_->label() );
  connect( button, SIGNAL( clicked() ), SLOT( _entryInfo() ) );
  button->setText("Info");
  toolbar->addWidget( button );

  // changes display according to read_only flag
  setReadOnly( read_only_ );
 
  // configuration
  connect( qApp, SIGNAL( configurationChanged ), SLOT( updateConfiguration() ) );
  updateConfiguration();
  
  // display selected entry
  displayEntry( entry );

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

  // reset modify flag; change title accordingly
  modified_ = false;
  updateWindowTitle();

  return;
}


//____________________________________________
void EditFrame::setReadOnly( const bool& value )
{

  // update read_only value
  read_only_ = value;

  // changes button state
  for( list< QWidget* >::iterator it=read_only_widgets_.begin(); it != read_only_widgets_.end(); it++ )
  { (*it)->setEnabled( !isReadOnly() ); }

  // changes lock button state
  if( isReadOnly() ) lock_->show();
  else lock_->hide();

  // changes TextEdit readOnly status
  title_->setReadOnly( isReadOnly() );
  text_->setReadOnly( isReadOnly() );

  // changes attachment list status
  attachmentList().setReadOnly( isReadOnly() );

  // changes window title
  updateWindowTitle();
  
}
  
//_____________________________________________
string EditFrame::windowTitle( void ) const
{

  LogEntry* entry( EditFrame::entry() );

  ostringstream title;
  if( entry )
  {
  
    title << entry->title();
    if( entry->keyword() != LogEntry::NO_KEYWORD ) title << " - " << entry->keyword();

  } else title << "Electronic Logbook Editor";

  if( isReadOnly() ) title << " (read only)";
  else if( modified_  ) title << " (modified)";
  return title.str();

}

//____________________________________________
AskForSaveDialog::ReturnCode EditFrame::askForSave( const bool& enable_cancel )
{
  
  // create dialog
  unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
  if( enable_cancel ) buttons |= AskForSaveDialog::CANCEL;
  AskForSaveDialog dialog( this, "Entry has been modified. Save ?", buttons );
  
  // exec and check return code
  int state = dialog.exec();
  if( state == AskForSaveDialog::YES ) saveEntry( false );
  return AskForSaveDialog::ReturnCode(state);
  
}

//_____________________________________________
void EditFrame::displayTitle( void )
{
  Debug::Throw( "EditFrame::displayTitle.\n" );

  LogEntry* entry( EditFrame::entry() );
  title_->setText( entry && entry->title().size() ) ? entry->title().c_str(): LogEntry::UNTITLED.c_str()  );
  title_->setCursorPosition( 0 );
  return;
}

// //_____________________________________________
// void EditFrame::DisplayColor( void )
// {
//   Debug::Throw( "EditFrame::DisplayColor.\n" );
//   if( !color_label_ ) return;
// 
//   Str color( EditFrame::entry()->GetColor() );
//   if( color.IsEqual( "None", false ) ) {
//     color_label_->hide();
//   } else {
// 
//     color_label_->SetColor( color );
//     color_label_->update();
//     color_label_->show();
// 
//   }
//   return;
// }

//_____________________________________________
void EditFrame::updateConfiguration( void )
{
  
  Debug::Throw( "EditFrame::updateConfiguration.\n" );
  
  // window size
  resize( XmlOptions::get().get<int>( "EDIT_FRAME_WIDTH" ), XmlOptions::get().get<int>( "EDIT_FRAME_HEIGHT" ) );
  
}

//_____________________________________________
void EditFrame::saveEntry( bool update_selection )
{
  
  Debug::Throw( "EditFrame::SaveEntry.\n" );
  if( isReadOnly() ) return;

  // retrieve associated entry
  LogEntry *entry( EditFrame::entry() );

  // see if entry is new
  bool entry_is_new( !entry || KeySet<Logbook>( entry ).empty() ) );
  
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
  entry->setText( qPrintable( text_->text() ) );
  //entry->SetTextFormat( text_format_->Write() );

  //! update entry title
  entry->SetTitle( qPrintable( title_->text() ) );

  // update author
  entry->setAuthor( XmlOptions::get().get<string>( "USER" ) );

  // add _now_ to entry modification timestamps
  entry->modified();

  // status bar
  statusbar_->label().setText(  "writting entry to logbook ..." );

  // add entry to logbook, if needed
  if( entry_is_new ) Key::associate( entry, logbook->latestChild() );

  // update this window title, set unmodified.
  modified_ = false;
  updateWindowTitle();

  // update selection frame
  frame->updateEntry( entry, update_selection );
  frame->setWindowTitle( MainFrame::MAIN_TITLE_MODIFIED );

  // set logbook as modified
  KeySet<Logbook> logbooks( entry );
  for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter!= logbooks.end(); iter++ )
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
  if( modified_ && askForSave() == AskForSaveDialog::CANCEL ) return;

  // create new entry, set author, set keyword
  LogEntry* entry = new LogEntry();
  entry->setAuthor( XmlOptions::get().get<string>( "USER" ) );
  entry->setKeyword( _selectionFrame()->currentKeyword() );

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
  if( !isReadOnly() && modified() && askForSave() == AskForSaveDialog::CANCEL ) event->ignore();
  else event->accept();
  return;
}

//____________________________________________
void EditFrame::enterEvent( QEvent *event )
{
  Debug::Throw( "EditFrame::enterEvent.\n" );

  // base class enterEvent
  QMainWindow::enterEvent( event );
  _selectionFrame().checkLogbookModified();

}

//_______________________________________________
void EditFrame::_previousEntry( void )
{
  Debug::Throw( "EditFrame::_previousEntry.\n" );

  if( isReadOnly() ) return;
    
  SelectionFrame *frame( _selectionFrame() );
  LogEntry* entry( frame->previousEntry( EditFrame::entry() ) );
  if( !( entry  || frame->lockEntry( entry ) ) ) return;
  displayEntry( entry );

}

//_______________________________________________
void EditFrame::_nextEntry( void )
{
  Debug::Throw( "EditFrame::_nextEntry.\n" );

  if( isReadOnly() ) return;

  SelectionFrame *frame( _selectionFrame() );
  LogEntry* entry( frame->NextEntry( EditFrame::entry() ) );
  if( !( entry || frame->LockEntry( entry ) ) ) return;
  displayEntry( entry );

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
  LogEntryInfoDialog( this, entry ).exec();

}

// //_____________________________________________
// void EditFrame::_Undo( void )
// {
//   Debug::Throw( "EditFrame::_Undo.\n" );
//   if( text_->hasFocus() ) text_->undo();
//   else if( title_->hasFocus() ) title_->undo();
//   return;
// }
// 
// //_____________________________________________
// void EditFrame::_Redo( void )
// {
//   Debug::Throw( "EditFrame::_Redo.\n" );
//   if( text_->hasFocus() ) text_->redo();
//   else if( title_->hasFocus() ) title_->redo();
//   return;
// }

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
  SPELLCHECK::SpellDialog( text_ ).exec();
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
  EditFrame *edit_frame( new EditFrame( frame, entry ) );
  Key::associate( edit_frame, frame );

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
  what << "/tmp/_eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
  dialog.setFile( File( what.str() ) );

  // map dialog
  QtUtil::centerOnParent( &dialog, false );
  if( dialog.exec() == QDialog::Rejected ) return;

  // retrieve/check file
  File file( dialog.file() );
  if( file.empty() ) {
    QtUtil::infoDialog(this, "No output file specified. <View HTML> canceled." );
    return;
  }

  QFile out( file.c_str() );
  if( !out.open( QIODevice::WriteOnly ) 
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
  HtmlUtil::Header( html, document );
  
  // body
  QDomElement body = html.appendChild( document.createElement( "body" ) ).toElement();
  
  // dump logbook header
  KeySet<Logbook> logbooks( entry );
  for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter != logbooks.end(); iter++ )
  { body.appendChild( (*iter)->htmlElement( document, html_log_mask ); }

  // dump entry
  body.appendChild( entry->htmlElement( document, html_entry_mask ) );

  out.write( document.toByteArray() );
  out.close();

  // retrieve command
  string command( dialog.command() );
  if( command.empty() ) return;

  // execute command
  what.str("");
  what << command << " " << file << " &";
  Util::run( what.str() );

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
void EditFrame::_modified( void )
{
  Debug::Throw( "EditFrame::_modified.\n" );

  // check readonly status
  if( isReadOnly() ) return;

  // check if already modified; update window title otherwise
  if( !modified_ ) {
    modified_ = true;
    updateWindowTitle();
  }

  return;
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
  statusbar_->label(2).setText( what.str(), true, 2 );
  
  return;
}

//_______________________________________________
SelectionFrame* EditFrame::_selectionFrame( void ) const
{
  Debug::Throw( "EditFrame::_selectionFrame.\n" );
  KeySet<SelectionFrame> frames( this );
  Exception::assert( frames.size()==1, DESCRIPTION( "wrong association to SelectionFrame" ) );
  return *frames.begin();
}

//_____________________________________________
void EditFrame::_displayText( void )
{
  Debug::Throw( "EditFrame::_displayText.\n" );
  if( !text_ ) return;

  LogEntry* entry( EditFrame::entry() );
  text_->setText( (entry) ? entry->GetText().c_str() : "" );
  // text_format_->Read( entry->GetTextFormat() );

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
  KeySet<Attachment> attachments( entry );
  if( attachments.empty() ) {
  
    attachment_list.hide();
    return;
  
  }

  // display associated attachments
  for( KeySet<Attachment>::iterator it = attachments.begin(); it != attachments.end(); it++ )
  attachment_list.addAttachment( *it );

  // show attachment list
  attachment_list.show();

  return;
}

// //_________________________________________________________
// void EditFrame::CustomLabel::drawContents( QPainter *painter )
// {
//   Debug::Throw( "EditFrame::CustomLabel::drawContents.\n" );
// 
//   if( color_ == "" || Str( color_ ).IsEqual( "None", false ) ) {
//     QLabel::drawContents( painter );
//     return;
//   }
// 
//   // create/check color
//   QColor color( color_.c_str() );
//   if( !color.isValid() ) QLabel::drawContents( painter );
// 
//   // paint background rectangle
//   painter->fillRect( contentsRect(), color );
//   QLabel::drawContents( painter );
// 
// }
