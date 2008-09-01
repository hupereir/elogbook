// $Id$
#ifndef MainWindow_h
#define MainWindow_h

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
  \file MainWindow.h
  \brief base class to display entries and keyword
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QBasicTimer>
#include <QContextMenuEvent>
#include <QSplitter>
#include <QTimer>
#include <QTimerEvent>

#include "AskForSaveDialog.h"
#include "Counter.h"
#include "CustomMainWindow.h"
#include "Debug.h"


#include "FileRecord.h"
#include "Key.h"
#include "KeywordModel.h"
#include "LogEntry.h"
#include "LogEntryModel.h"

#include "TreeView.h"

// class ColorMenu;
class CustomToolBar;
class EditionWindow;
class Logbook;
class Menu;
class SearchPanel;
class SelectionStatusBar;
class ColorMenu;

//! display a set of log entries, allows selection of one
class MainWindow: public CustomMainWindow, public Counter, public BASE::Key
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! constructor
  MainWindow( QWidget *parent = 0 );

  //! destructor
  virtual ~MainWindow( void );

  //! retrive menu
  Menu& menu( void )
  {
    assert( menu_ );
    return *menu_;
  }

  //! retrive search panel
  SearchPanel& searchPanel( void )
  {
    assert( search_panel_ );
    return *search_panel_;
  }

  //! retrive state frame
  SelectionStatusBar& statusBar( void )
  {
    assert( statusbar_ );
    return *statusbar_;
  }

  //! deletes old logbook, if any. Set the new one an display
  /* 
  note: don't pass a const File& here cause it crashes the application
  when the file that is passed is from the currently opened logbook,
  since the later gets deleted in the method and the file is being re-used
  */
  virtual bool setLogbook( File );

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
  virtual void clearSelection( void );

  //! select entry
  virtual void selectEntry( LogEntry* );

  //! update entry (create new if not found )
  virtual void updateEntry( LogEntry*, const bool& );

  //! delete entry
  virtual void deleteEntry( LogEntry*, const bool& save = true );

  //! look for EditionWindows matching entry, set readonly
  virtual bool lockEntry( LogEntry* ) const;

  //! retrieve previous entry (if any)
  virtual LogEntry* previousEntry( LogEntry*, const bool& );

  //! retrieve next entry (if any)
  virtual LogEntry* nextEntry( LogEntry*, const bool& );

  //! reset attachment frame
  virtual void resetAttachmentWindow( void ) const;

  //! return keyword list
  TreeView& keywordList( void ) const
  { return *keyword_list_; }
   
  //! log entry list
  virtual TreeView& logEntryList( void ) const
  { return *entry_list_; }
  
  //! current keyword
  Keyword currentKeyword( void ) const;
   
  //!@name toolbars
  //@{
  
  //! keyword toolbar
  CustomToolBar& keywordToolBar( void ) const
  { 
    assert( keyword_toolbar_ );
    return *keyword_toolbar_;
  }
  
  //! entry toolbar
  CustomToolBar& entryToolBar( void ) const
  { 
    assert( entry_toolbar_ );
    return *entry_toolbar_;
  }  
  //@}
    
  //!@name actions 
  //@{
  
  //! uniconify window
  QAction& uniconifyAction( void ) const
  { return *uniconify_action_; }
  
  //! new keyword action
  QAction& newKeywordAction( void ) const
  { return *new_keyword_action_; }

  //! edit keyword action
  QAction& editKeywordAction( void ) const
  { return *edit_keyword_action_; }

  //! delete keyword action
  QAction& deleteKeywordAction( void ) const
  { return *delete_keyword_action_; }
  //! new entry action
  QAction& newEntryAction( void ) const
  { return *new_entry_action_; }

  //! edit entry action
  QAction& editEntryAction( void ) const
  { return *edit_entry_action_; }

  //! delete entry action
  QAction& deleteEntryAction( void ) const
  { return *delete_entry_action_; }

  //! change entry color action
  QAction& entryColorAction( void ) const
  { return *entry_color_action_; }

  //! change entry keyword action
  QAction& entryKeywordAction( void ) const
  { return *entry_keyword_action_; }

  //! create new logbook
  QAction& newLogbookAction( void ) const
  { return *new_logbook_action_; }
  
  //! open existing logbook
  QAction& openAction( void ) const
  { return *open_action_; }
  
  //! synchronize logbooks
  QAction& synchronizeAction( void ) const
  { return *synchronize_action_; }
  
  //! reorganize logbook
  QAction& reorganizeAction( void ) const
  { return *reorganize_action_; }
  
  //! save logbook
  QAction& saveAction( void ) const
  { return *save_action_; }

  //! save logbook
  QAction& saveForcedAction( void ) const
  { return *save_forced_action_; }

  //! save logbook with a different name
  QAction& saveAsAction( void ) const
  { return *save_as_action_; }

  //! save logbook backup
  QAction& saveBackupAction( void ) const
  { return *save_backup_action_; }

  //! revert logbook to saved version
  QAction& revertToSaveAction( void ) const
  { return *revert_to_save_action_; }
  
  //! convert logbook to html
  QAction& viewHtmlAction( void ) const
  { return *view_html_action_; }

  //! logbook information
  QAction& logbookInformationsAction( void ) const
  { return *logbook_informations_action_; }
  
  //! logbook information
  QAction& logbookStatisticsAction( void ) const
  { return *logbook_statistics_action_; }

  //! close editionwindows
  QAction& closeFramesAction( void ) const
  { return *close_frames_action_; }
  
  //! show duplicates
  QAction& showDuplicatesAction( void ) const
  { return *show_duplicates_action_; }
  
  //@}
  
  signals:

  //! emmited when a message is available from logbook
  void messageAvailable( const QString& );

  //! emmited at the end of SetLogbook
  void ready( void );

  public slots:

  //! open existing logbook
  virtual void open( FileRecord file = FileRecord() );

  //! save current logbook
  /*! if argument is false, all modified entries will be saved without asking */
  virtual void save( const bool& confirm_entries = true );

  //! select entries using selection criterions
  virtual void selectEntries( QString, unsigned int );

  //! show all entries
  virtual void showAllEntries( void );
    
  protected:

  //! enter event handler
  virtual void enterEvent( QEvent* );
  
  //! close event
  virtual void closeEvent( QCloseEvent* );
  
  //! timer event
  virtual void timerEvent( QTimerEvent* );

  //! context menu event [overloaded]
  virtual void contextMenuEvent( QContextMenuEvent* );
  
  //! install actions
  virtual void _installActions( void );
  
  //! keyword model
  KeywordModel& _keywordModel( void )
  { return keyword_model_; }
  
  //! keyword model
  const KeywordModel& _keywordModel( void ) const
  { return keyword_model_; }
 
  //! log entry model
  LogEntryModel& _logEntryModel( void )
  { return entry_model_; }
    
  //! clear list and reinitialize from logbook entries
  virtual void _resetLogEntryList( void );

  //! clear list and reinitialize from logbook entries
  virtual void _resetKeywordList( void );
  
  //! load colors (from current logbook)
  void _loadColors( void );    

  private slots:
  
  //! configuration
  void _updateConfiguration( void );

  //! splitter moved
  void _splitterMoved( void );
  
  //! create a new logbook
  void _newLogbook( void );

  //! save logbook and children whether they are modified or not
  void _saveForced( void );
  
  /*! \brief
    save current logbook with a given filename
    returns true if logbook was saved
  */
  bool _saveAs( File default_file = File("") );

  //! save current logbook with a given filename
  void _saveBackup( void );

  //! revert logbook to saved version
  void _revertToSaved( void );

  //! opens a logbook merge it to the existing onecomments
  void _synchronize( void );

  //! reorganize logbook to entries associations
  void _reorganize( void );

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

  //! close EditionWindows
  virtual void _closeEditionWindows( void ) const;

  //! create new entry
  virtual void _newEntry( void );

  //! edit selected entries
  virtual void _editEntries( void );

  //! delete selected entries
  virtual void _deleteEntries ( void );

  //! show EditionWindow associated to a given name
  virtual void _displayEntry( LogEntry* );
  
  //! rename entry with current title
  virtual void _changeEntryTitle( LogEntry*, std::string );

  //! change selected entries color
  virtual void _changeEntryColor( QColor );
  
  //! create new keyword
  void _newKeyword( void );
  
  //! delete keyword from keyword list using dialog
  void _deleteKeyword( void );

  //! change selected entries keyword using dialog
  /*! 
  this is triggered by the rename keyword action from 
  the keyword list
  */
  void _renameKeyword( void );

  //! rename keyword for all entries that match old keyword.
  /*! 
  this is triggered by drag and drop in the keyword list,
  by renaming a keyword directly from the keyword list,
  or by deleting a keyword in the list, and moving entries to the parent.
  It is also called by the renameKeyword slot above.
  */
  void _renameKeyword( Keyword old_keyword, Keyword new_keyword, bool update_selection = true );
  
  //! rename keyword for selected entries using dialog
  /*! 
  this is triggered by the rename entry keyword action in the
  logEntry list.
  */
  void _renameEntryKeyword( void );
  
  //! change selected entries keyword using argument
  /*! 
  this is triggered by drag and drop from the logEntry list
  to the keyword list, and it is also called by the slot above 
  */
  void _renameEntryKeyword( Keyword new_keyword, bool update_selection = true );
  
  //! keyword selection changed
  void _keywordSelectionChanged( const QModelIndex& );
    
  //! update keyword-list related actions
  void _updateKeywordActions( void );
  
  //! update entry-list related actions
  void _updateEntryActions( void );
  
  //! create HTML file from logbook
  virtual void _viewHtml( void );
  
  //! store sorting method when changed via list header
  virtual void _storeSortMethod( void )
  { _storeSortMethod( _logEntryModel().sortColumn(), _logEntryModel().sortOrder() ); }

  //! store sorting method when changed via list header
  virtual void _storeSortMethod( int, Qt::SortOrder );
 
  //! item clicked
  virtual void _entryItemClicked( const QModelIndex& index );
      
  //! activare item
  void _entryItemActivated( const QModelIndex& index );
    
  //! item data changed
  void _entryDataChanged( const QModelIndex& index );
 
  //! edit entry title
  void _startEntryEdition( void );
 
  //! store selected jobs in model
  void _storeSelectedEntries( void );

  //! restore selected jobs from model
  void _restoreSelectedEntries( void );
 
  //! store selected jobs in model
  void _storeSelectedKeywords( void );

  //! restore selected jobs from model
  void _restoreSelectedKeywords( void );
 
  //! store expanded jobs in model
  void _storeExpandedKeywords( void );

  //! restore expanded jobs from model
  void _restoreExpandedKeywords( void );

  //! perform autoSave
  void _autoSave( void );
  
  private:
  
  //! returns true if logbook has modified entries
  bool _hasModifiedEntries( void ) const;
  
  //! check modified entries
  AskForSaveDialog::ReturnCode _checkModifiedEntries( BASE::KeySet<EditionWindow>, const bool& ) const;

  //! main menu
  Menu* menu_;

  //! search panel
  SearchPanel *search_panel_;

  //! state frame
  SelectionStatusBar* statusbar_;

  //! main splitter
  QSplitter* splitter_;

  //! keyword model
  KeywordModel keyword_model_;
  
  //! entry model
  LogEntryModel entry_model_;
  
  //! logEntry list
  TreeView* entry_list_;

  //! local TreeView to store size hint
  class KeywordList: public TreeView
  {
    
    public:
        
    //! constructor
    KeywordList( QWidget* parent = 0 ):
      TreeView( parent ),
      default_width_( -1 )
    {} 
   
    //! default size
    void setDefaultWidth( const int& );
    
    //! default width
    const int& defaultWidth( void ) const
    { return default_width_; }
    
  
    //! size
    QSize sizeHint( void ) const;  
    
    private:
    
    //! default width;
    int default_width_;
    
  };
  
  //! Keyword list
  KeywordList *keyword_list_;

  //! color menu
  ColorMenu* color_menu_;
  
  //! autoSaveTimer
  QTimer autosave_timer_;

  //!@name item edition
  //@{
  
  //! edition timer
  QTimer edition_timer_;

  //! edit_item delay (ms)
  enum { edition_delay_ = 500 };
  
  //@}
  
  //! associated logbook
  Logbook* logbook_;

  //! last directory in which logbook was opened, saved.
  File working_directory_;
  
  //! ignore file modified warnings if true
  bool ignore_warnings_;
  
  //! ask entries for confirmation before saving
  bool confirm_entries_;

  //@name toolbars
  //@{
  CustomToolBar *keyword_toolbar_;
  CustomToolBar *entry_toolbar_;
  //@}
  
  //!@name actions
  //@{
  
  //! uniconify action
  QAction* uniconify_action_;
  
  //! add new keyword
  QAction* new_keyword_action_;
 
  //! edit keyword
  QAction* edit_keyword_action_;
  
  //! delete keyword
  QAction* delete_keyword_action_;
  
  //! new entry
  QAction* new_entry_action_;
  
  //! edit entry
  QAction* edit_entry_action_;
  
  //! delete entry
  QAction* delete_entry_action_;
  
  //! change selected entries color
  QAction* entry_color_action_;
  
  //! change selected entries keyword
  QAction* entry_keyword_action_;
  
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

  //! close editionwindows
  QAction* close_frames_action_;
  
  //! show duplicates
  QAction* show_duplicates_action_;
    
  //@}
      
  //! timer
  /*! needed to store Keyword list width */
  QBasicTimer resize_timer_;
  
};

#endif
