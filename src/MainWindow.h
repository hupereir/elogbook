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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "AskForSaveDialog.h"
#include "Counter.h"
#include "BaseMainWindow.h"
#include "Debug.h"
#include "FileCheck.h"
#include "FileRecord.h"
#include "Key.h"
#include "KeywordList.h"
#include "KeywordModel.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryList.h"
#include "LogEntryModel.h"
#include "LogEntryPrintSelectionWidget.h"
#include "SearchWidget.h"
#include "TreeView.h"

#include <QBasicTimer>
#include <QTimerEvent>
#include <QContextMenuEvent>

class ColorMenu;
class CustomToolBar;
class EditionWindow;
class FileCheck;
class LogbookPrintHelper;
class Menu;
class ProgressStatusBar;

//* display a set of log entries, allows selection of one
class MainWindow: public BaseMainWindow, public Counter, public Base::Key
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    MainWindow( QWidget* = nullptr );

    //* destructor
    virtual ~MainWindow( void );

    //* retrive menu
    Menu& menu( void )
    { return *menu_; }

    //* retrive state frame
    ProgressStatusBar& statusBar( void ) const
    { return *statusbar_; }

    //* creates a new default logbook
    virtual void createDefaultLogbook( void );

    //* deletes old logbook, if any. Set the new one an display
    /*
    note: don't pass a const File& here cause it crashes the application
    when the file that is passed is from the currently opened logbook,
    since the later gets deleted in the method and the file is being re-used
    */
    virtual bool setLogbook( File );

    //* check if logbook needs a backup, ask for it if needed
    virtual void checkLogbookBackup( void );

    //* deletes old logbook, if any
    virtual void reset( void );

    //* returns pointer to selected Logbook, if any
    virtual Logbook* logbook( void ) const
    { return logbook_; }

    //* creates dialog to ask for Logbook save.
    AskForSaveDialog::ReturnCode askForSave( bool enableCancel = true );

    //* retrieve working directory
    virtual const File& workingDirectory( void ) const
    { return workingDirectory_; }

    //* clear entry selection
    virtual void clearSelection( void );

    //* update entry (create new if not found )
    virtual void updateEntry( Keyword, LogEntry*, bool );

    //* delete entry
    virtual void deleteEntry( LogEntry*, bool save = true );

    //* look for EditionWindows matching entry, set readonly
    virtual bool lockEntry( LogEntry* ) const;

    //* retrieve previous entry (if any)
    virtual LogEntry* previousEntry( LogEntry*, bool );

    //* retrieve next entry (if any)
    virtual LogEntry* nextEntry( LogEntry*, bool );

    //* reset attachment frame
    virtual void resetAttachmentWindow( void ) const;

    //* return keyword list
    TreeView& keywordList( void ) const
    { return *keywordList_; }

    //* log entry list
    virtual TreeView& logEntryList( void ) const
    { return *entryList_; }

    //* current keyword
    Keyword currentKeyword( void ) const;

    //* file checker
    FileCheck& fileCheck( void ) const
    { return *fileCheck_; }

    //*@name toolbars
    //@{

    //* keyword toolbar
    CustomToolBar& keywordToolBar( void ) const
    { return *keywordToolBar_; }

    //* entry toolbar
    CustomToolBar& entryToolBar( void ) const
    { return *entryToolBar_; }

    //@}

    //*@name actions
    //@{

    //* uniconify window
    QAction& uniconifyAction( void ) const
    { return *uniconifyAction_; }

    //* new keyword action
    QAction& newKeywordAction( void ) const
    { return *newKeywordAction_; }

    //* edit keyword action
    QAction& editKeywordAction( void ) const
    { return *editKeywordAction_; }

    //* delete keyword action
    QAction& deleteKeywordAction( void ) const
    { return *deleteKeywordAction_; }

    //* find entries action
    QAction& findEntriesAction( void ) const
    { return *findEntriesAction_; }

    //* new entry action
    QAction& newEntryAction( void ) const
    { return *newEntryAction_; }

    //* edit entry action
    QAction& editEntryAction( void ) const
    { return *editEntryAction_; }

    //* edit entry action
    QAction& editEntryTitleAction( void ) const
    { return *editEntryTitleAction_; }

    //* delete entry action
    QAction& deleteEntryAction( void ) const
    { return *deleteEntryAction_; }

    //* change entry color action
    QAction& entryColorAction( void ) const
    { return *entryColorAction_; }

    //* change entry keyword action
    QAction& entryKeywordAction( void ) const
    { return *entryKeywordAction_; }

    //* show entry properties
    QAction& entryInformationAction( void ) const
    { return *entryInformationAction_; }

    //* create new logbook
    QAction& newLogbookAction( void ) const
    { return *newLogbookAction_; }

    //* open existing logbook
    QAction& openAction( void ) const
    { return *openAction_; }

    //* synchronize logbooks
    QAction& synchronizeAction( void ) const
    { return *synchronizeAction_; }

    //* reorganize logbook
    QAction& reorganizeAction( void ) const
    { return *reorganizeAction_; }

    //* save logbook
    QAction& saveAction( void ) const
    { return *saveAction_; }

    //* save logbook
    QAction& saveForcedAction( void ) const
    { return *saveForcedAction_; }

    //* save logbook with a different name
    QAction& saveAsAction( void ) const
    { return *saveAsAction_; }

    //* save logbook backup
    QAction& saveBackupAction( void ) const
    { return *saveBackupAction_; }

    //* backup manager
    QAction& backupManagerAction( void ) const
    { return *backupManagerAction_; }

    //* revert logbook to saved version
    QAction& revertToSaveAction( void ) const
    { return *revertToSaveAction_; }

    //* print
    QAction& printAction( void ) const
    { return *printAction_; }

    //* print preview
    QAction& printPreviewAction( void ) const
    { return *printPreviewAction_; }

    //* export to html
    QAction& htmlAction( void ) const
    { return *htmlAction_; }

    //* logbook information
    QAction& logbookInformationsAction( void ) const
    { return *logbookInformationsAction_; }

    //* logbook information
    QAction& logbookStatisticsAction( void ) const
    { return *logbookStatisticsAction_; }

    //* close editionwindows
    QAction& closeFramesAction( void ) const
    { return *closeFramesAction_; }

    //* show duplicates
    QAction& showDuplicatesAction( void ) const
    { return *showDuplicatesAction_; }

    //* monitored files
    QAction& monitoredFilesAction( void ) const
    { return *monitoredFilesAction_; }

    //* tree mode action
    QAction& treeModeAction( void ) const
    { return *treeModeAction_; }

    //@}

    //* update window title
    void updateWindowTitle( void );

    Q_SIGNALS:

    //* emitted when new scratch file is created
    void scratchFileCreated( const File& );

    //* emitted when a message is available from logbook
    void messageAvailable( const QString& );

    //* emitted at the end of SetLogbook
    void ready( void );

    public Q_SLOTS:

    //* open existing logbook
    virtual void open( FileRecord file = FileRecord() );

    //* save current logbook
    /** if argument is false, all modified entries will be saved without asking */
    virtual void save( bool confirmEntries = true );

    //* select entry
    virtual void selectEntry( LogEntry* );

    //* select entry
    virtual void selectEntry( const Keyword&, LogEntry* );

    //* select entries using selection criterions
    virtual void selectEntries( QString, SearchWidget::SearchModes );

    //* show all entries
    virtual void showAllEntries( void );

    protected:

    //* close event
    virtual void closeEvent( QCloseEvent* );

    //* timer event
    virtual void timerEvent( QTimerEvent* );

    //* context menu event [overloaded]
    virtual void contextMenuEvent( QContextMenuEvent* );

    //* clear list and reinitialize from logbook entries
    virtual void _resetKeywordList( void );

    //* clear list and reinitialize from logbook entries
    virtual void _resetLogEntryList( void );

    //* load colors (from current logbook)
    void _loadColors( void );

    //* enable state
    void _setEnabled( bool );

    //* returns true if logbook has modified entries
    bool _hasModifiedEntries( void ) const;

    //* perform autoSave
    void _autoSave( void );

    //* check modified entries
    AskForSaveDialog::ReturnCode _checkModifiedEntries( Base::KeySet<EditionWindow>, bool ) const;

    enum MaskFlag
    {
        None = 0,
        TitleMask = 1<<0,
        KeywordMask = 1<<1
    };

    Q_DECLARE_FLAGS( Mask, MaskFlag )

    //* update frames associated to given entry
    void _updateEntryFrames( LogEntry*, Mask );

    //* get entries matching a given entry selection mode
    LogEntryModel::List _entries( LogEntryPrintSelectionWidget::Mode mode );

    protected Q_SLOTS:

    //* files modified
    void _filesModified( FileCheck::DataSet );

    //* splitter moved
    void _splitterMoved( void );

    //* create a new logbook
    void _newLogbook( void );

    //* save logbook and children whether they are modified or not
    void _saveForced( void );

    /** \brief
    save current logbook with a given filename
    returns true if logbook was saved
    */
    bool _saveAs( File defaultFile = File(), bool registerLogbook = true );

    //* save current logbook with a given filename
    void _saveBackup( void );

    //* manage backups
    void _manageBackups( void );

    //* revert logbook to saved version
    void _revertToSaved( void );

    //* Print current document
    void _print( void );

    //* Print current document
    void _print( LogbookPrintHelper& );

    //* Print current document
    void _printPreview( void );

    //* export to html
    void _toHtml( void );

    //* opens a logbook merge it to the existing onecomments
    void _synchronize( void );

    //* remove backup
    void _removeBackup( Backup );

    //* remove backups
    void _removeBackups( Backup::List );

    //* restore backup
    void _restoreBackup( Backup );

    //* merge backup
    void _mergeBackup( Backup );

    //* reorganize logbook to entries associations
    void _reorganize( void );

    /** \brief
    show all entries which have equal creation time
    is needed to remove duplicate entries in case of
    wrong logbook merging. This is a Debugging tool
    */
    virtual void _showDuplicatedEntries( void );

    //* view logbook statistics
    virtual void _viewLogbookStatistics( void );

    //* edit current logbook informations
    virtual void _editLogbookInformations( void );

    //* close EditionWindows
    virtual void _closeEditionWindows( bool askForSave = true ) const;

    //* find entries
    void _findEntries( void ) const;

    //* create new entry
    virtual void _newEntry( void );

    //* edit selected entries
    virtual void _editEntries( void );

    //* delete selected entries
    virtual void _deleteEntries ( void );

    //* show EditionWindow associated to a given name
    virtual void _displayEntry( LogEntry* );

    //* rename entry with current title
    virtual void _changeEntryTitle( LogEntry*, QString );

    //* change selected entries color
    virtual void _changeEntryColor( QColor );

    //* show entry information
    virtual void _entryInformation( void );

    //* create new keyword
    void _newKeyword( void );

    //* delete keyword from keyword list using dialog
    void _deleteKeyword( void );

    //* change selected entries keyword using dialog
    /**
    this is triggered by the rename keyword action from
    the keyword list
    */
    void _renameKeyword( void );

    //* rename keyword for all entries that match old keyword.
    /**
    this is triggered by drag and drop in the keyword list,
    **/
    void _confirmRenameKeyword( const Keyword& oldKeyword, const Keyword& newKeyword );

    //* rename keyword for all entries that match old keyword.
    /**
    this is triggered by renaming a keyword directly from the keyword list,
    or by deleting a keyword in the list, and moving entries to the parent.
    It is also called by the renameKeyword slot above.
    */
    void _renameKeyword( const Keyword& oldKeyword, const Keyword& newKeyword, bool updateSelection = true );

    //* rename keyword for selected entries using dialog
    /**
    this is triggered by the rename entry keyword action in the
    logEntry list.
    */
    void _renameEntryKeyword( void );

    //* change selected entries keyword using argument
    void _confirmRenameEntryKeyword( Keyword );

    //* keyword selection changed
    void _keywordSelectionChanged( const QModelIndex& );

    //* update keyword-list related actions
    void _updateKeywordActions( void );

    //* update entry-list related actions
    void _updateEntryActions( void );

    //* read-only actions
    void _updateReadOnlyState( void );

    //* store sorting method when changed via list header
    virtual void _storeSortMethod( void )
    { _storeSortMethod( entryModel_.sortColumn(), entryModel_.sortOrder() ); }

    //* store sorting method when changed via list header
    virtual void _storeSortMethod( int, Qt::SortOrder );

    //* item clicked
    virtual void _entryItemClicked( const QModelIndex& index );

    //* activare item
    void _entryItemActivated( const QModelIndex& index );

    //* item data changed
    void _entryDataChanged( const QModelIndex& index );

    //* edit entry title
    void _startEntryEdition( void );

    //* monitored files
    void _showMonitoredFiles( void );

    //* tree mode
    void _toggleTreeMode( bool );

    private Q_SLOTS:

    //* configuration
    void _updateConfiguration( void );

    private:

    //* install actions
    void _installActions( void );

    //* change selected entries keyword using argument
    /** it is called by the renameEntry keyword slots **/
    void _renameEntryKeyword( Keyword );

    //* change selected entries keyword using argument
    /** it is called by the confirmRenameEntry keyword slots **/
    void _linkEntryKeyword( Keyword );

    //* update selection and entries
    void _updateSelection( Keyword, Base::KeySet<LogEntry> );

    //* main menu
    Menu* menu_ = nullptr;

    //* search panel
    SearchWidget *searchWidget_ = nullptr;

    //* state frame
    ProgressStatusBar* statusbar_ = nullptr;

    //* keyword model
    KeywordModel keywordModel_;

    //* entry model
    LogEntryModel entryModel_;

    //* logEntry list
    LogEntryList* entryList_ = nullptr;

    //* file check
    FileCheck* fileCheck_ = nullptr;

    //* Keyword list
    KeywordList *keywordList_ = nullptr;

    //* color menu
    ColorMenu* colorMenu_ = nullptr;

    //* autoSaveTimer
    QBasicTimer autosaveTimer_;

    //* autosave interval
    int autoSaveDelay_ = 60*1000;

    //*@name item edition
    //@{

    //* edition timer
    QBasicTimer editionTimer_;

    //* edition delay (ms)
    int editionDelay_ = 200;

    //@}

    //* maximum number of recent entries
    int maxRecentEntries_ = 0;

    //* associated logbook
    Logbook* logbook_ = nullptr;

    //* last directory in which logbook was opened, saved.
    File workingDirectory_;

    //* ignore file modified warnings if true
    bool ignoreWarnings_ = false;

    //* ask entries for confirmation before saving
    bool confirmEntries_ = true;

    //* keyword container
    QWidget* keywordContainer_ = nullptr;

    //@name toolbars
    //@{
    //* keywords
    CustomToolBar *keywordToolBar_ = nullptr;

    //* entries
    CustomToolBar *entryToolBar_ = nullptr;
    //@}

    //*@name actions
    //@{

    //* uniconify action
    QAction* uniconifyAction_ = nullptr;

    //* add new keyword
    QAction* newKeywordAction_ = nullptr;

    //* edit keyword
    QAction* editKeywordAction_ = nullptr;

    //* delete keyword
    QAction* deleteKeywordAction_ = nullptr;

    //* find entries
    QAction* findEntriesAction_ = nullptr;

    //* new entry
    QAction* newEntryAction_ = nullptr;

    //* edit entry
    QAction* editEntryAction_ = nullptr;

    //* edit entry title
    QAction* editEntryTitleAction_ = nullptr;

    //* delete entry
    QAction* deleteEntryAction_ = nullptr;

    //* change selected entries color
    QAction* entryColorAction_ = nullptr;

    //* change selected entries keyword
    QAction* entryKeywordAction_ = nullptr;

    //* entry information action
    QAction* entryInformationAction_ = nullptr;

    //* create new logbook
    QAction* newLogbookAction_ = nullptr;

    //* open existing logbook
    QAction* openAction_ = nullptr;

    //* synchronize logbooks
    QAction* synchronizeAction_ = nullptr;

    //* reorganize logbook
    QAction* reorganizeAction_ = nullptr;

    //* save logbook
    QAction* saveAction_ = nullptr;

    //* save logbook
    QAction* saveForcedAction_ = nullptr;

    //* save logbook with different name
    QAction* saveAsAction_ = nullptr;

    //* save logbook backup
    QAction* saveBackupAction_ = nullptr;

    //* configure backups
    QAction* backupManagerAction_ = nullptr;

    //* revert logbook to saved version
    QAction* revertToSaveAction_ = nullptr;

    //* print preview
    QAction* printAction_ = nullptr;

    //* print preview
    QAction* printPreviewAction_ = nullptr;

    //* export to html
    QAction* htmlAction_ = nullptr;

    //* logbook information
    QAction* logbookInformationsAction_ = nullptr;

    //* logbook information
    QAction* logbookStatisticsAction_ = nullptr;

    //* close editionwindows
    QAction* closeFramesAction_ = nullptr;

    //* show duplicates
    QAction* showDuplicatesAction_ = nullptr;

    //* show monitored files
    QAction* monitoredFilesAction_ = nullptr;

    //* tree mode
    QAction* treeModeAction_ = nullptr;

    //* entry color button
    QToolButton* entryColorButton_ = nullptr;

    //* static menu actions
    QList<QAction*> keywordChangedMenuActions_;

    //* static menu actions
    QList<QAction*> entryKeywordChangedMenuActions_;

    //@}

    //* resize timer
    /** needed to store Keyword list width */
    QBasicTimer resizeTimer_;

};

#endif
