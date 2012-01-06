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

#include "AskForSaveDialog.h"
#include "Counter.h"
#include "BaseMainWindow.h"
#include "Debug.h"
#include "FileCheck.h"
#include "FileRecord.h"
#include "Key.h"
#include "KeywordModel.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryModel.h"
#include "LogEntryPrintSelectionWidget.h"

#include "AnimatedTreeView.h"

#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>
#include <QtGui/QContextMenuEvent>

class ColorMenu;
class CustomToolBar;
class EditionWindow;
class FileCheck;
class Menu;
class SearchPanel;
class SelectionStatusBar;

//! display a set of log entries, allows selection of one
class MainWindow: public BaseMainWindow, public Counter, public BASE::Key
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
    SearchPanel& searchPanel( void ) const
    {
        assert( searchPanel_ );
        return *searchPanel_;
    }

    //! retrive state frame
    SelectionStatusBar& statusBar( void ) const
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

    //! deletes old logbook, if any
    virtual void reset( void );

    //! returns pointer to selected Logbook, if any
    virtual Logbook* logbook( void ) const
    { return logbook_; }

    //! creates dialog to ask for Logbook save.
    AskForSaveDialog::ReturnCode askForSave( const bool& enable_cancel = true );

    //! retrieve working directory
    virtual const File& workingDirectory( void ) const
    { return workingDirectory_; }

    //! clear entry selection
    virtual void clearSelection( void );

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
    AnimatedTreeView& keywordList( void ) const
    { return *keywordList_; }

    //! log entry list
    virtual AnimatedTreeView& logEntryList( void ) const
    { return *entryList_; }

    //! current keyword
    Keyword currentKeyword( void ) const;

    //! file checker
    FileCheck& fileCheck( void ) const
    { return *fileCheck_; }

    //!@name toolbars
    //@{

    //! keyword toolbar
    CustomToolBar& keywordToolBar( void ) const
    {
        assert( keywordToolBar_ );
        return *keywordToolBar_;
    }

    //! entry toolbar
    CustomToolBar& entryToolBar( void ) const
    {
        assert( entryToolBar_ );
        return *entryToolBar_;
    }
    //@}

    //!@name actions
    //@{

    //! uniconify window
    QAction& uniconifyAction( void ) const
    { return *uniconifyAction_; }

    //! new keyword action
    QAction& newKeywordAction( void ) const
    { return *newKeywordAction_; }

    //! edit keyword action
    QAction& editKeywordAction( void ) const
    { return *editKeywordAction_; }

    //! delete keyword action
    QAction& deleteKeywordAction( void ) const
    { return *deleteKeywordAction_; }

    //! find entries action
    QAction& findEntriesAction( void ) const
    { return *findEntriesAction_; }

    //! new entry action
    QAction& newEntryAction( void ) const
    { return *newEntryAction_; }

    //! edit entry action
    QAction& editEntryAction( void ) const
    { return *editEntryAction_; }

    //! edit entry action
    QAction& editEntryTitleAction( void ) const
    { return *editEntryTitleAction_; }

    //! delete entry action
    QAction& deleteEntryAction( void ) const
    { return *deleteEntryAction_; }

    //! change entry color action
    QAction& entryColorAction( void ) const
    { return *entryColorAction_; }

    //! change entry keyword action
    QAction& entryKeywordAction( void ) const
    { return *entryKeywordAction_; }

    //! create new logbook
    QAction& newLogbookAction( void ) const
    { return *newLogbookAction_; }

    //! open existing logbook
    QAction& openAction( void ) const
    { return *openAction_; }

    //! synchronize logbooks
    QAction& synchronizeAction( void ) const
    { return *synchronizeAction_; }

    //! reorganize logbook
    QAction& reorganizeAction( void ) const
    { return *reorganizeAction_; }

    //! save logbook
    QAction& saveAction( void ) const
    { return *saveAction_; }

    //! save logbook
    QAction& saveForcedAction( void ) const
    { return *saveForcedAction_; }

    //! save logbook with a different name
    QAction& saveAsAction( void ) const
    { return *saveAsAction_; }

    //! save logbook backup
    QAction& saveBackupAction( void ) const
    { return *saveBackupAction_; }

    //! backup manager
    QAction& backupManagerAction( void ) const
    { return *backupManagerAction_; }

    //! revert logbook to saved version
    QAction& revertToSaveAction( void ) const
    { return *revertToSaveAction_; }

    //! print
    QAction& printAction( void ) const
    { return *printAction_; }

    //! print preview
    QAction& printPreviewAction( void ) const
    { return *printPreviewAction_; }

    //! export to html
    QAction& htmlAction( void ) const
    { return *htmlAction_; }

    //! logbook information
    QAction& logbookInformationsAction( void ) const
    { return *logbookInformationsAction_; }

    //! logbook information
    QAction& logbookStatisticsAction( void ) const
    { return *logbookStatisticsAction_; }

    //! close editionwindows
    QAction& closeFramesAction( void ) const
    { return *closeFramesAction_; }

    //! show duplicates
    QAction& showDuplicatesAction( void ) const
    { return *showDuplicatesAction_; }

    //! monitored files
    QAction& monitoredFilesAction( void ) const
    { return *monitoredFilesAction_; }

    //! tree mode action
    QAction& treeModeAction( void ) const
    { return *treeModeAction_; }

    //@}

    //! set modified
    void setModified( bool );

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
    virtual void save( const bool& confirmEntries = true );

    //! select entry
    virtual void selectEntry( LogEntry* );

    //! select entries using selection criterions
    virtual void selectEntries( QString, unsigned int );

    //! show all entries
    virtual void showAllEntries( void );

    protected:

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
    { return keywordModel_; }

    //! keyword model
    const KeywordModel& _keywordModel( void ) const
    { return keywordModel_; }

    //! log entry model
    LogEntryModel& _logEntryModel( void )
    { return entryModel_; }

    //! clear list and reinitialize from logbook entries
    virtual void _resetLogEntryList( void );

    //! clear list and reinitialize from logbook entries
    virtual void _resetKeywordList( void );

    //! load colors (from current logbook)
    void _loadColors( void );

    //! enable state
    void _setEnabled( bool );

    //! returns true if logbook has modified entries
    bool _hasModifiedEntries( void ) const;

    //! perform autoSave
    void _autoSave( void );

    //! check modified entries
    AskForSaveDialog::ReturnCode _checkModifiedEntries( BASE::KeySet<EditionWindow>, const bool& ) const;

    enum Mask
    {
        NONE = 0,
        TITLE_MASK = 1<<0,
        KEYWORD_MASK = 1<<1
    };

    //! update frames associated to given entry
    void _updateEntryFrames( LogEntry*, unsigned int );

    //! get entries matching a given entry selection mode
    LogEntryModel::List _entries( LogEntryPrintSelectionWidget::Mode mode );

    protected slots:

    //! files modified
    void _filesModified( FileCheck::DataSet );

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
    bool _saveAs( File defaultFile = File(""), bool registerLogbook = true );

    //! save current logbook with a given filename
    void _saveBackup( void );

    //! manage backups
    void _manageBackups( void );

    //! revert logbook to saved version
    void _revertToSaved( void );

    //! Print current document
    void _print( void );

    //! Print current document
    void _printPreview( void );

    //! export to html
    void _toHtml( void );

    //! opens a logbook merge it to the existing onecomments
    void _synchronize( void );

    //! remove backup
    void _removeBackup( Logbook::Backup );

    //! restore backup
    void _restoreBackup( Logbook::Backup );

    //! merge backup
    void _mergeBackup( Logbook::Backup );

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
    virtual void _closeEditionWindows( bool askForSave = true ) const;

    //! find entries
    void _findEntries( void ) const;

    //! create new entry
    virtual void _newEntry( void );

    //! edit selected entries
    virtual void _editEntries( void );

    //! delete selected entries
    virtual void _deleteEntries ( void );

    //! show EditionWindow associated to a given name
    virtual void _displayEntry( LogEntry* );

    //! rename entry with current title
    virtual void _changeEntryTitle( LogEntry*, QString );

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
    void _renameKeyword( Keyword oldKeyword, Keyword newKeyword, bool updateSelection = true );

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
    void _renameEntryKeyword( Keyword newKeyword, bool updateSelection = true );

    //! keyword selection changed
    void _keywordSelectionChanged( const QModelIndex& );

    //! update keyword-list related actions
    void _updateKeywordActions( void );

    //! update entry-list related actions
    void _updateEntryActions( void );

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

    //! monitored files
    void _showMonitoredFiles( void );

    //! tree mode
    void _toggleTreeMode( bool );

    private slots:

    //! configuration
    void _updateConfiguration( void );

    private:

    //! main menu
    Menu* menu_;

    //! search panel
    SearchPanel *searchPanel_;

    //! state frame
    SelectionStatusBar* statusbar_;

    //! keyword model
    KeywordModel keywordModel_;

    //! entry model
    LogEntryModel entryModel_;

    //! logEntry list
    AnimatedTreeView* entryList_;

    //! file check
    FileCheck* fileCheck_;

    //! local TreeView to store size hint
    class KeywordList: public AnimatedTreeView
    {

        public:

        //! constructor
        KeywordList( QWidget* parent = 0 ):
            AnimatedTreeView( parent ),
            defaultWidth_( -1 )
        {}

        //! default size
        void setDefaultWidth( const int& );

        //! default width
        const int& defaultWidth( void ) const
        { return defaultWidth_; }


        //! size
        QSize sizeHint( void ) const;

        private:

        //! default width;
        int defaultWidth_;

    };

    //! Keyword list
    KeywordList *keywordList_;

    //! color menu
    ColorMenu* colorMenu_;

    //! autoSaveTimer
    QBasicTimer autosaveTimer_;

    //! autosave interval
    int autoSaveDelay_;

    //!@name item edition
    //@{

    //! edition timer
    QBasicTimer editionTimer_;

    //! edit_item delay (ms)
    int editionDelay_;

    //@}

    //! maximum number of recent entries
    unsigned int maxRecentEntries_;

    //! associated logbook
    Logbook* logbook_;

    //! last directory in which logbook was opened, saved.
    File workingDirectory_;

    //! ignore file modified warnings if true
    bool ignoreWarnings_;

    //! ask entries for confirmation before saving
    bool confirmEntries_;

    //! keyword container
    QWidget* keywordContainer_;

    //@name toolbars
    //@{
    //! keywords
    CustomToolBar *keywordToolBar_;

    //! entries
    CustomToolBar *entryToolBar_;
    //@}

    //!@name actions
    //@{

    //! uniconify action
    QAction* uniconifyAction_;

    //! add new keyword
    QAction* newKeywordAction_;

    //! edit keyword
    QAction* editKeywordAction_;

    //! delete keyword
    QAction* deleteKeywordAction_;

    //! find entries
    QAction* findEntriesAction_;

    //! new entry
    QAction* newEntryAction_;

    //! edit entry
    QAction* editEntryAction_;

    //! edit entry title
    QAction* editEntryTitleAction_;

    //! delete entry
    QAction* deleteEntryAction_;

    //! change selected entries color
    QAction* entryColorAction_;

    //! change selected entries keyword
    QAction* entryKeywordAction_;

    //! create new logbook
    QAction* newLogbookAction_;

    //! open existing logbook
    QAction* openAction_;

    //! synchronize logbooks
    QAction* synchronizeAction_;

    //! reorganize logbook
    QAction* reorganizeAction_;

    //! save logbook
    QAction* saveAction_;

    //! save logbook
    QAction* saveForcedAction_;

    //! save logbook with different name
    QAction* saveAsAction_;

    //! save logbook backup
    QAction* saveBackupAction_;

    //! configure backups
    QAction* backupManagerAction_;

    //! revert logbook to saved version
    QAction* revertToSaveAction_;

    //! print preview
    QAction* printAction_;

    //! print preview
    QAction* printPreviewAction_;

    //! export to html
    QAction* htmlAction_;

    //! logbook information
    QAction* logbookInformationsAction_;

    //! logbook information
    QAction* logbookStatisticsAction_;

    //! close editionwindows
    QAction* closeFramesAction_;

    //! show duplicates
    QAction* showDuplicatesAction_;

    //! show monitored files
    QAction* monitoredFilesAction_;

    //! tree mode
    QAction* treeModeAction_;

    //@}

    //! resize timer
    /*! needed to store Keyword list width */
    QBasicTimer resizeTimer_;

};

#endif
