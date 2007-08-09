// $Id$
#ifndef SelectionFrame_h
#define SelectionFrame_h

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
  \file SelectionFrame.h
  \brief base class to display entries and keyword
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QSplitter>

#include "AskForSaveDialog.h"
#include "Counter.h"
#include "CustomMainWindow.h"
#include "Debug.h"
#include "Exception.h"
#include "FileRecord.h"
#include "Key.h"
#include "KeywordList.h"
#include "LogEntry.h"
#include "LogEntryList.h"
#include "QtUtil.h"

// class ColorMenu;
class CustomLineEdit;
class EditFrame;
class Logbook;
class Menu;
class SearchPanel;
class StatusBar;
class ColorMenu;

//! display a set of log entries, allows selection of one
class SelectionFrame: public CustomMainWindow, public Counter, public BASE::Key
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! constructor
  SelectionFrame( QWidget *parent );

  //! destructor
  virtual ~SelectionFrame( void )
  { Debug::Throw(0, "SelectionFrame::~SelectionFrame.\n" ); }

  //! retrive menu
  Menu& menu( void )
  {
    Exception::checkPointer( menu_, DESCRIPTION( "menu_ not initialized" ) );
    return *menu_;
  }

  //! retrive search panel
  SearchPanel& searchPanel( void )
  {
    Exception::checkPointer( search_panel_, DESCRIPTION( "search_panel_ not initialized" ) );
    return *search_panel_;
  }

  //! retrive state frame
  StatusBar& statusBar( void )
  {
    Exception::checkPointer( statusbar_, DESCRIPTION( "statusbar_ not initialized" ) );
    return *statusbar_;
  }

  //! deletes old logbook, if any. Set the new one an display
  /* 
  note: don't pass a const File& here cause it crashes the application
  when the file that is passed is from the currently opened logbook,
  since the later gets deleted in the method and the file is being re-used
  */
  virtual void setLogbook( File file );

  //! check if logbook needs a backup, ask for it if needed
  virtual void checkLogbookBackup( void );

  //! check if logbook has been externally modified
  virtual void checkLogbookModified( void );
  
  //! deletes old logbook, if any
  virtual void reset( void );

  //! returns pointer to selected Logbook, if any
  virtual Logbook* logbook( void ) const
  { return logbook_; }

  //! creates dialog to ask for Logbook save.
  AskForSaveDialog::ReturnCode askForSave( const bool& enable_cancel = true );

  //! retrieve working directory
  virtual const File& workingDirectory( void ) const
  { return working_directory_; }

  //! clear entry selection
  virtual void clearSelection( void )
  { logEntryList().clearSelection(); }

  //! select entry
  virtual void selectEntry( LogEntry *entry );

  //! update entry (create new if not found )
  virtual void updateEntry( LogEntry *entry, const bool& update_selection );

  //! delete entry
  virtual void deleteEntry( LogEntry* entry, const bool& save = true );

  //! look for EditFrames matching entry, set readonly
  virtual bool lockEntry( LogEntry* entry ) const;

  //! retrieve previous entry (if any)
  virtual LogEntry* previousEntry( LogEntry* entry, const bool& update_selection );

  //! retrieve next entry (if any)
  virtual LogEntry* nextEntry( LogEntry* entry, const bool& update_selection );

  //! reset attachment frame
  virtual void resetAttachmentFrame( void ) const;

  //! return keyword list
  KeywordList& keywordList( void )
  {
    Exception::checkPointer( keyword_list_, DESCRIPTION( "keyword_list_ not initialized" ) );
    return *keyword_list_;
  }

  //! return entry list
  LogEntryList& logEntryList( void )
  {
    Exception::checkPointer( list_, DESCRIPTION( "list_ not initialized" ) );
    return *list_;
  }
    
  //!@name actions 
  //@{
  
  //! uniconify window
  QAction& uniconifyAction( void )
  { return *uniconify_action_; }
  
  //! create new logbook
  QAction& newLogbookAction( void )
  { return *new_logbook_action_; }
  
  //! open existing logbook
  QAction& openAction( void )
  { return *open_action_; }
  
  //! synchronize logbooks
  QAction& synchronizeAction( void )
  { return *synchronize_action_; }
  
  //! reorganize logbook
  QAction& reorganizeAction( void )
  { return *reorganize_action_; }
  
  //! save logbook
  QAction& saveAction( void )
  { return *save_action_; }

  //! save logbook
  QAction& saveForcedAction( void )
  { return *save_forced_action_; }

  //! save logbook with a different name
  QAction& saveAsAction( void )
  { return *save_as_action_; }

  //! save logbook backup
  QAction& saveBackupAction( void )
  { return *save_backup_action_; }

  //! revert logbook to saved version
  QAction& revertToSaveAction( void )
  { return *revert_to_save_action_; }
  
  //! convert logbook to html
  QAction& viewHtmlAction( void )
  { return *view_html_action_; }

  //! logbook information
  QAction& logbookInformationsAction( void )
  { return *logbook_informations_action_; }
  
  //! logbook information
  QAction& logbookStatisticsAction( void )
  { return *logbook_statistics_action_; }

  //! close editframes
  QAction& closeFramesAction( void )
  { return *close_frames_action_; }
  
  //! show duplicates
  QAction& showDuplicatesAction( void )
  { return *show_duplicates_action_; }
  
  //@}
  
  signals:

  //! emmited when a message is available from logbook
  void messageAvailable( const QString& );

  //! emmited at the end of SetLogbook
  void ready( void );

  public slots:

  //! configuration
  virtual void updateConfiguration( void );

  //! save configuration
  void saveConfiguration( void );

  //! open existing logbook
  virtual void open( FileRecord file = FileRecord() );

  //! save current logbook
  /*! if argument is false, all modified entries will be saved without asking */
  virtual void save( const bool& confirm_entries = true );

  //! select entries using selection criterions
  virtual void selectEntries( QString text, unsigned int mode );

  //! show all entries
  virtual void showAllEntries( void );
    
  protected:

  //! enter event handler
  virtual void enterEvent( QEvent *event );
  
  //! close event
  virtual void closeEvent( QCloseEvent *event );
  
  //! install actions
  virtual void _installActions( void );
  
  //! clear list and reinitialize from logbook entries
  virtual void _resetList( void );

  //! clear list and reinitialize from logbook entries
  virtual void _resetKeywordList( void );
  
  //! load colors (from current logbook)
  void _loadColors( void );    

  protected slots:
  
  //! uniconify
  void _uniconify( void )
  { 
  
    Debug::Throw( "SelectionFrame::_uniconify.\n" );
    QtUtil::uniconify( this ); 
    
  }
  
  //! create a new logbook
  virtual void _newLogbook( void );

  //! save logbook and children whether they are modified or not
  virtual void _saveForced( void );
  
  /*! \brief
    save current logbook with a given filename
    returns true if logbook was saved
  */
  virtual bool _saveAs( File default_file = File("") );

  //! save current logbook with a given filename
  virtual void _saveBackup( void );

  //! revert logbook to saved version
  virtual void _revertToSaved( void );

  //! opens a logbook merge it to the existing onecomments
  virtual void _synchronize( void );

  //! reorganize logbook to entries associations
  virtual void _reorganize( void );

  /*! \brief
    show all entries which have equal creation time
    is needed to remove duplicate entries in case of
    wrong logbook merging. This is a Debugging tool
  */
  virtual void _showDuplicatedEntries( void );

  //! view logbook statistics
  virtual void _viewLogbookStatistics( void );

  //! edit current logbook informations
  virtual void _editLogbookInformations( void );

  //! close EditFrames
  virtual void _closeEditFrames( void ) const;

  //! create new entry
  virtual void _newEntry( void );

  //! edit selected entries
  virtual void _editEntries( void );

  //! delete selected entries
  virtual void _deleteEntries ( void );

  //! show EditFrame associated to a given name
  virtual void _displayEntry( LogEntry* );
  
  //! rename entry with current title
  virtual void _changeEntryTitle( LogEntry*, std::string );

  //! change selected entries color
  virtual void _changeEntryColor( QColor );
  
  //! create new keyword
  void _newKeyword( void );
  
  //! delete keyword from keyword list using dialog
  void _deleteKeyword( void );

  //! change selected entries keyword using custom dialog
  void _changeEntryKeyword( void );
  
  //! change selected entries keyword using argument
  void _changeEntryKeyword( std::string new_keyword );
  
  //! rename keyword from keyword list using arguments
  void _changeEntryKeyword( std::string old_keyword, std::string new_keyword );
   
  //! keyword selection changed
  void _keywordSelectionChanged( QTreeWidgetItem*, QTreeWidgetItem* );
    
  //! create HTML file from logbook
  virtual void _viewHtml( void );
  
  //! store sorting method when changed via list header
  virtual void _storeSortMethod( void )
  { _storeSortMethod( logEntryList().sortColumn() ); }

  //! store sorting method when changed via list header
  virtual void _storeSortMethod( int column );

  //! perform autoSave
  void _autoSave( void );

  private:
  
  //! main menu
  Menu* menu_;

  //! search panel
  SearchPanel *search_panel_;

  //! state frame
  StatusBar* statusbar_;

  //! main splitter
  QSplitter* splitter_;
  
  //! logEntry list
  LogEntryList* list_;

  //! Keyword list
  KeywordList *keyword_list_;

  //! color menu
  ColorMenu* color_menu_;
  
  //! autoSaveTimer
  QTimer* autosave_timer_;

  //! associated logbook
  Logbook* logbook_;

  //! last directory in which logbook was opened, saved.
  File working_directory_;
  
  //! ignore file modified warnings if true
  bool ignore_warnings_;
  
  //! ask entries for confirmation before saving
  bool confirm_entries_;

  //!@name actions
  ///@{
  
  //! uniconify action
  QAction* uniconify_action_;
  
  //! create new logbook
  QAction* new_logbook_action_;
  
  //! open existing logbook
  QAction* open_action_;
  
  //! synchronize logbooks
  QAction* synchronize_action_;
  
  //! reorganize logbook
  QAction* reorganize_action_;
  
  //! save logbook
  QAction* save_action_;

  //! save logbook
  QAction* save_forced_action_;

  //! save logbook with different name
  QAction* save_as_action_;

  //! save logbook backup
  QAction* save_backup_action_;

  //! revert logbook to saved version
  QAction* revert_to_save_action_;
  
  //! convert logbook to html
  QAction* view_html_action_;

  //! logbook information
  QAction* logbook_informations_action_;
  
  //! logbook information
  QAction* logbook_statistics_action_;

  //! close editframes
  QAction* close_frames_action_;
  
  //! show duplicates
  QAction* show_duplicates_action_;
    
  //@}
  
};

#endif
