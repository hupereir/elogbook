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

#include <memory>

class ColorMenu;
class CustomToolBar;
class EditionWindow;
class FileCheck;
class LogbookPrintHelper;
class Menu;
class ProgressStatusBar;

//* display a set of log entries, allows selection of one
class MainWindow: public BaseMainWindow, private Base::Counter<MainWindow>, public Base::Key
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit MainWindow( QWidget* = nullptr );

    //*@name accessors
    //@{

    //* retrive menu
    Menu& menu() const
    { return *menu_; }

    //* retrive state frame
    ProgressStatusBar& statusBar() const
    { return *statusbar_; }

    //* true if main window has logbook
    bool hasLogbook() const
    { return logbook_ != nullptr; }

    //* returns pointer to selected Logbook, if any
    using LogbookPointer = std::unique_ptr<Logbook>;
    const LogbookPointer& logbook() const
    { return logbook_; }

    //* true if logbook exists and is readonly
    bool logbookIsReadOnly()
    { return logbook_ && logbook_->isReadOnly(); }

    //* true if logbook exists and is modified
    bool logbookIsModified()
    { return logbook_ && logbook_->modified(); }

    //* retrieve working directory
    File workingDirectory() const
    { return workingDirectory_; }

    //* return keyword list
    TreeView& keywordList() const
    { return *keywordList_; }

    //* log entry list
    TreeView& logEntryList() const
    { return *entryList_; }

    //* current keyword
    Keyword currentKeyword() const;

    //* keyword toolbar
    CustomToolBar& keywordToolBar() const
    { return *keywordToolBar_; }

    //* entry toolbar
    CustomToolBar& entryToolBar() const
    { return *entryToolBar_; }

    //@}

    //*@name modifiers
    //@{

    //* creates a new default logbook
    void createDefaultLogbook();

    //* deletes old logbook, if any. Set the new one an display
    /*
    note: don't pass a const File& here cause it crashes the application
    when the file that is passed is from the currently opened logbook,
    since the later gets deleted in the method and the file is being re-used
    */
    bool setLogbook( File );

    //* check if logbook needs a backup, ask for it if needed
    void checkLogbookBackup();

    //* deletes old logbook, if any
    void reset();

    //* check modified entries
    AskForSaveDialog::ReturnCode checkModifiedEntries();

    //* creates dialog to ask for Logbook save.
    AskForSaveDialog::ReturnCode askForSave( bool enableCancel = true );

    //* clear entry selection
    void clearSelection();

    //* update entry (create new if not found )
    void updateEntry( Keyword, LogEntry*, bool );

    //* delete entry
    void deleteEntry( LogEntry*, bool save = true );

    //* look for EditionWindows matching entry, set readonly
    bool lockEntry( LogEntry* );

    //* retrieve previous entry (if any)
    LogEntry* previousEntry( LogEntry*, bool );

    //* retrieve next entry (if any)
    LogEntry* nextEntry( LogEntry*, bool );

    //* reset attachment frame
    void resetAttachmentWindow() const;

    //* update window title
    void updateWindowTitle();

    //@}

    //*@name actions
    //@{

    //* uniconify window
    QAction& uniconifyAction() const
    { return *uniconifyAction_; }

    //* new keyword action
    QAction& newKeywordAction() const
    { return *newKeywordAction_; }

    //* edit keyword action
    QAction& editKeywordAction() const
    { return *editKeywordAction_; }

    //* delete keyword action
    QAction& deleteKeywordAction() const
    { return *deleteKeywordAction_; }

    //* find entries action
    QAction& findEntriesAction() const
    { return *findEntriesAction_; }

    //* new entry action
    QAction& newEntryAction() const
    { return *newEntryAction_; }

    //* edit entry action
    QAction& editEntryAction() const
    { return *editEntryAction_; }

    //* edit entry action
    QAction& editEntryTitleAction() const
    { return *editEntryTitleAction_; }

    //* delete entry action
    QAction& deleteEntryAction() const
    { return *deleteEntryAction_; }

    //* change entry color action
    QAction& entryColorAction() const
    { return *entryColorAction_; }

    //* change entry keyword action
    QAction& entryKeywordAction() const
    { return *entryKeywordAction_; }

    //* show entry properties
    QAction& entryInformationAction() const
    { return *entryInformationAction_; }

    //* create new logbook
    QAction& newLogbookAction() const
    { return *newLogbookAction_; }

    //* open existing logbook
    QAction& openAction() const
    { return *openAction_; }

    //* synchronize logbooks
    QAction& synchronizeAction() const
    { return *synchronizeAction_; }

    //* reorganize logbook
    QAction& reorganizeAction() const
    { return *reorganizeAction_; }

    //* save logbook
    QAction& saveAction() const
    { return *saveAction_; }

    //* save logbook
    QAction& saveForcedAction() const
    { return *saveForcedAction_; }

    //* save logbook with a different name
    QAction& saveAsAction() const
    { return *saveAsAction_; }

    //* save logbook backup
    QAction& saveBackupAction() const
    { return *saveBackupAction_; }

    //* backup manager
    QAction& backupManagerAction() const
    { return *backupManagerAction_; }

    //* revert logbook to saved version
    QAction& revertToSaveAction() const
    { return *revertToSaveAction_; }

    //* print
    QAction& printAction() const
    { return *printAction_; }

    //* print preview
    QAction& printPreviewAction() const
    { return *printPreviewAction_; }

    //* export to html
    QAction& htmlAction() const
    { return *htmlAction_; }

    //* logbook information
    QAction& logbookInformationsAction() const
    { return *logbookInformationsAction_; }

    //* logbook information
    QAction& logbookStatisticsAction() const
    { return *logbookStatisticsAction_; }

    //* close editionwindows
    QAction& closeFramesAction() const
    { return *closeFramesAction_; }

    //* show duplicates
    QAction& showDuplicatesAction() const
    { return *showDuplicatesAction_; }

    //* monitored files
    QAction& monitoredFilesAction() const
    { return *monitoredFilesAction_; }

    //* tree mode action
    QAction& treeModeAction() const
    { return *treeModeAction_; }

    //@}

    Q_SIGNALS:

    //* emitted when new scratch file is created
    void scratchFileCreated( const File& );

    //* emitted when a message is available from logbook
    void messageAvailable( const QString& );

    //* emitted at the end of SetLogbook
    void ready();

    public Q_SLOTS:

    //* open existing logbook
    void open( FileRecord file = FileRecord() );

    //* save current logbook
    /** pending entry modifications are ignored */
    void saveUnchecked();

    //* save current logbook
    /**
    if there are pending enry modifications, they are first saved to the logbook,
    then the logbook is saved.
    if argument is false, all modified entries will be saved without asking
    */
    void save();

    //* select entry
    void selectEntry( LogEntry* );

    //* select entry
    void selectEntry( const Keyword&, LogEntry* );

    //* select entries using selection criterions
    void selectEntries( QString, SearchWidget::SearchModes );

    //* show all entries
    void showAllEntries();

    protected:

    //* close event
    void closeEvent( QCloseEvent* ) override;

    //* timer event
    void timerEvent( QTimerEvent* ) override;

    //* context menu event [overloaded]
    void contextMenuEvent( QContextMenuEvent* ) override;

    //* clear list and reinitialize from logbook entries
    void _resetKeywordList();

    //* clear list and reinitialize from logbook entries
    void _resetLogEntryList();

    //* load colors (from current logbook)
    void _loadColors();

    //* enable state
    void _setEnabled( bool );

    //* returns true if logbook has modified entries
    bool _hasModifiedEntries() const;

    //* perform autoSave
    void _autoSave();

    //* check modified entries
    AskForSaveDialog::ReturnCode _checkModifiedEntries( Base::KeySet<EditionWindow> );

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
    void _splitterMoved();

    //* create a new logbook
    void _newLogbook();

    //* save logbook and children whether they are modified or not
    void _saveForced();

    /** \brief
    save current logbook with a given filename
    returns true if logbook was saved
    */
    bool _saveAs( File defaultFile = File(), bool registerLogbook = true );

    //* save current logbook with a given filename
    void _saveBackup();

    //* manage backups
    void _manageBackups();

    //* revert logbook to saved version
    void _revertToSaved();

    //* Print current document
    void _print();

    //* Print current document
    void _print( LogbookPrintHelper& );

    //* Print current document
    void _printPreview();

    //* export to html
    void _toHtml();

    //* opens a logbook merge it to the existing onecomments
    void _synchronize();

    //* remove backup
    void _removeBackup( Backup );

    //* remove backups
    void _removeBackups( Backup::List );

    //* restore backup
    void _restoreBackup( Backup );

    //* merge backup
    void _mergeBackup( Backup );

    //* reorganize logbook to entries associations
    void _reorganize();

    /** \brief
    show all entries which have equal creation time
    is needed to remove duplicate entries in case of
    wrong logbook merging. This is a Debugging tool
    */
    void _showDuplicatedEntries();

    //* view logbook statistics
    void _viewLogbookStatistics();

    //* edit current logbook informations
    void _editLogbookInformations();

    //* close EditionWindows
    void _closeEditionWindows( bool askForSave = true );

    //* find entries
    void _findEntries() const;

    //* create new entry
    void _newEntry();

    //* edit selected entries
    void _editEntries();

    //* delete selected entries
    void _deleteEntries ();

    //* show EditionWindow associated to a given name
    void _displayEntry( LogEntry* );

    //* rename entry with current title
    void _changeEntryTitle( LogEntry*, QString );

    //* change selected entries color
    void _changeEntryColor( QColor );

    //* show entry information
    void _entryInformation();

    //* create new keyword
    void _newKeyword();

    //* delete keyword from keyword list using dialog
    void _deleteKeyword();

    //* change selected entries keyword using dialog
    /**
    this is triggered by the rename keyword action from
    the keyword list
    */
    void _renameKeyword();

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
    /** this is triggered by the rename entry keyword action in the logEntry list. */
    void _renameEntryKeyword();

    //* change selected entries keyword using argument
    void _confirmRenameEntryKeyword( Keyword );

    //* keyword selection changed
    void _keywordSelectionChanged( const QModelIndex& );

    //* update keyword-list related actions
    void _updateKeywordActions();

    //* update entry-list related actions
    void _updateEntryActions();

    //* read-only actions
    void _updateReadOnlyState();

    //* store sorting method when changed via list header
    void _storeSortMethod()
    { _storeSortMethod( entryModel_.sortColumn(), entryModel_.sortOrder() ); }

    //* store sorting method when changed via list header
    void _storeSortMethod( int, Qt::SortOrder );

    //* item clicked
    void _entryItemClicked( const QModelIndex& index );

    //* activare item
    void _entryItemActivated( const QModelIndex& index );

    //* item data changed
    void _entryDataChanged( const QModelIndex& index );

    //* edit entry title
    void _startEntryEdition();

    //* monitored files
    void _showMonitoredFiles();

    //* tree mode
    void _toggleTreeMode( bool );

    private Q_SLOTS:

    //* configuration
    void _updateConfiguration();

    private:

    //* install actions
    void _installActions();

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
    LogbookPointer logbook_;

    //* last directory in which logbook was opened, saved.
    File workingDirectory_;

    //* ignore file modified warnings if true
    bool ignoreWarnings_ = false;

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

    //* menu actions
    QList<QAction*> keywordChangedMenuActions_;

    //* menu actions
    QList<QAction*> entryKeywordChangedMenuActions_;

    //@}

    //* resize timer
    /** needed to store Keyword list width */
    QBasicTimer resizeTimer_;

};

#endif
