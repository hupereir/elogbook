#ifndef EditionWindow_h
#define EditionWindow_h

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
#include "AttachmentFrame.h"
#include "BaseMainWindow.h"
#include "Counter.h"
#include "Debug.h"
#include "Key.h"
#include "LineEditor.h"
#include "LogEntry.h"
#include "TextEditor.h"
#include "TextPosition.h"

#include <QBasicTimer>
#include <QTimerEvent>
#include <QLabel>
#include <QPrinter>
#include <QPushButton>
#include <QSplitter>
#include <QToolButton>
#include <QList>

class Attachment;
class BaseContextMenu;
class BaseFindWidget;
class BaseReplaceWidget;
class BaseStatusBar;
class ColorMenu;
class CustomToolBar;
class FormatBar;
class LogEntryPrintHelper;
class MainWindow;
class Menu;
class SelectLineWidget;

namespace Private
{

    class ColorWidget;
    class LocalTextEditor;
}

//* log entry edition/creation object
class EditionWindow: public BaseMainWindow, public Counter, public Base::Key
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    using Editor = LineEditor;

    //* creator
    EditionWindow( QWidget*, bool readOnly = true );

    //* display all entries informations
    void displayEntry( Keyword, LogEntry* = nullptr );

    //* display all entries informations
    void displayEntry( LogEntry* = nullptr );

    //* returns current entry
    LogEntry* entry( void ) const
    {
        Base::KeySet<LogEntry> entries( this );
        Q_ASSERT( entries.size() <= 1 );
        return( entries.size() ) ? *entries.begin():nullptr;
    }

    //* retrieve attachment list
    AttachmentFrame& attachmentFrame( void )
    {
        Base::KeySet<AttachmentFrame> frames( this );
        Q_ASSERT( frames.size() == 1 );
        return **frames.begin();
    }

    //* status bar
    BaseStatusBar& statusBar( void )
    { return *statusBar_; }

    //* retrieve active display
    TextEditor& activeEditor( void );

    //* retrieve active display
    const TextEditor& activeEditor( void ) const;

    //@}

    //* check if this editor is readOnly or not
    bool isReadOnly( void ) const
    { return readOnly_; }

    //* set readOnly state of the EditionWindow
    void setReadOnly( bool );

    //* color menu
    void setColorMenu( ColorMenu* );

    //* closed flag
    bool isClosed( void ) const
    { return closed_; }

    //* closed flag
    void setIsClosed( bool );

    //* check if current entry has been modified or not
    bool modified( void ) const
    {
        return
            keywordEditor_->isModified() ||
            titleEditor_->isModified() ||
            activeEditor().document()->isModified();
    }

    //* computes window title
    QString windowTitle() const;

    //* creates dialog to ask for LogEntry save.
    AskForSaveDialog::ReturnCode askForSave( bool = true );

    //* update keyword Widget from current entry
    void displayKeyword( void );

    //* update title Widget from current entry
    void displayTitle( void );

    //* update color Widget from current entry
    void displayColor( void );

    //* check if current entry has been modified or not
    void setModified( bool );

    //* used to count modified EditionWindows
    class ModifiedFTor
    {
        public:

        //* predicate
        bool operator() (const EditionWindow* frame )
        { return frame->modified() && !frame->isReadOnly() && !frame->isClosed(); }

    };

    //* used to count alive frames, that are not subject to delayed deletion
    class aliveFTor
    {
        public:

        //* predicate
        bool operator() (const EditionWindow* frame )
        { return !frame->isClosed(); }

    };

    //*@name actions
    //@{

    //* new entry action
    QAction& newEntryAction( void ) const
    { return *newEntryAction_; }

    //* previous entry action
    QAction& previousEntryAction( void ) const
    { return *previousEntryAction_; }

    //* next entry action
    QAction& nextEntryAction( void ) const
    { return *nextEntryAction_; }

    //* save
    QAction& saveAction( void ) const
    { return *saveAction_; }

    #if USE_ASPELL
    //* check spelling of current entry
    QAction& spellcheckAction( void ) const
    { return *spellcheckAction_; }
    #endif

    //* entry information
    QAction& entryInformationAction( void ) const
    { return *entryInformationAction_; }

    //* print
    QAction& printAction( void ) const
    { return *printAction_; }

    //* print preview
    QAction& printPreviewAction( void ) const
    { return *printPreviewAction_; }

    //* html
    QAction& htmlAction( void ) const
    { return *htmlAction_; }

    //* split view horizontal
    QAction& splitViewHorizontalAction( void ) const
    { return *splitViewHorizontalAction_; }

    //* split view vertical
    QAction& splitViewVerticalAction( void ) const
    { return *splitViewVerticalAction_; }

    //* split view vertical
    QAction& cloneWindowAction( void ) const
    { return *cloneWindowAction_; }

    //* close view
    QAction& closeAction( void ) const
    { return *closeAction_; }

    //* uniconify
    QAction& uniconifyAction( void ) const
    { return *uniconifyAction_; }

    //* show keyword
    QAction& showKeywordAction( void ) const
    { return *showKeywordAction_; }

    //* add hyperLink
    QAction& insertLinkAction( void ) const
    { return *insertLinkAction_; }

    //@}

    //* force keyword visibility
    void setForceShowKeyword( bool value );

    //* close view
    /** Ask for save if view is modified */
    void closeEditor( TextEditor& );

    //* change active display manualy
    void setActiveEditor( TextEditor& );

    Q_SIGNALS:

    //* emitted when new scratch file is created
    void scratchFileCreated( const File& );

    //*@name re-implemented from text editor
    //@{

    //* emitted from TextDisplay when no match is found for find/replace request
    void noMatchFound( void );

    //* emitted from TextDisplay when no match is found for find/replace request
    void matchFound( void );

    //* emitted when selected line is not found
    void lineNotFound( void );

    //* emitted when selected line is found
    void lineFound( void );

    //@}

    public Q_SLOTS:

    //* update read-only state
    void updateReadOnlyState( void );

    //* change window title
    void updateWindowTitle()
    { setWindowTitle( windowTitle() ); }

    //*@name reimplemented from TextEditor
    //@{

    //* find text from dialog
    virtual void findFromDialog( void );

    //* replace text from dialog
    virtual void replaceFromDialog( void );

    //* select line from dialog
    virtual void selectLineFromDialog( void );

    //@}

    protected:

    //*@name event filters
    //@{

    //* close window event handler
    virtual void closeEvent( QCloseEvent* );

    //* timer event
    virtual void timerEvent( QTimerEvent* );

    //@}

    //*@name display management
    //@{

    //* split view
    Private::LocalTextEditor& _splitView( const Qt::Orientation& );

    //* create new splitter
    QSplitter& _newSplitter( const Qt::Orientation&  );

    //* create new TextEditor
    Private::LocalTextEditor& _newTextEditor( QWidget* parent );

    //@}

    //* display cursor position
    void _displayCursorPosition( const TextPosition& position );

    //* true if has associated main window
    bool _hasMainWindow( void ) const;

    //* retrieve associated MainWindow
    MainWindow& _mainWindow( void ) const;

    //* update text Widget from current entry
    void _displayText( void );

    //* update attachment list Widget from current entry
    void _displayAttachments( void );

    //* true if status bar is set
    bool _hasStatusBar( void ) const
    { return (bool) statusBar_; }

    //* change keyword (and other widgets) visibility
    void _setKeywordVisible( bool );

    protected Q_SLOTS:

    //* Save Current entry
    void _save( bool updateSelection = true );

    //* Print current document
    void _print( void );

    //* Print current document
    void _print( LogEntryPrintHelper& );

    //* Print current document
    void _printPreview( void );

    //* export to html
    void _toHtml( void );

    //* creates a new entry
    void _newEntry( void );

    //* splitter moved
    void _splitterMoved( void );

    //* select previous entry
    void _previousEntry( void );

    //* select next entry
    void _nextEntry( void );

    //* show entry info
    void _entryInformation( void );

    //* Delete Current entry
    void _deleteEntry( void );

    //* check spelling of current entry
    void _spellCheck( void );

    //* undo in focused editor (text/title/keyword)
    void _undo( void );

    //* redo in focused editor (text/title/keyword);
    void _redo( void );

    //* clone editor
    void _cloneWindow( void );

    //* find
    void _find( TextSelection selection )
    { activeEditor().find( selection ); }

    //* find
    void _replace( TextSelection selection )
    { activeEditor().replace( selection ); }

    //* find
    void _replaceInSelection( TextSelection selection )
    { activeEditor().replaceInSelection( selection ); }

    //* find
    void _replaceInWindow( TextSelection selection )
    { activeEditor().replaceInWindow( selection ); }

    //* select line
    void _selectLine( int value )
    { activeEditor().selectLine( value ); }

    //* restore focus on active display, when closing embedded dialog
    void _restoreFocus( void )
    { activeEditor().setFocus(); }

    //* unlock read-only editors
    void _unlock( void );

    //* insert link
    void _insertLink( void );

    //* insert link
    void _editLink( void );

    //* insert link
    void _removeLink( void );

    //* view link
    void _openLink( void );

    //* view link
    void _openLink( QString );

    //* update replace in selection action
    void _updateReplaceInSelection( void );

    //* read only actions
    void _updateReadOnlyActions( void );

    //* update (enable/disable) save action
    void _updateSaveAction( void );

    //* update (enable/disable) redo action
    void _updateUndoRedoActions( void );

    //* update (enable/disable) redo action
    void _updateUndoRedoActions( QWidget*, QWidget* );

    //* update (enable/disable) insert link action
    void _updateInsertLinkActions( void );

    //* Set entry as modified, change window title
    void _textModified( bool );

    //* display cursor position
    void _displayCursorPosition( void )
    { _displayCursorPosition( activeEditor().textPosition() ); }

    //* display cursor position
    void _displayCursorPosition( int, int new_position )
    { _displayCursorPosition( TextPosition( 0, new_position ) ); }

    //* close
    void _close( void );

    //* clone current file
    void _splitView( void )
    { _splitView( Qt::Vertical ); }

    //* clone current file horizontal
    void _splitViewHorizontal( void )
    { _splitView( Qt::Horizontal ); }

    //* clone current file horizontal
    void _splitViewVertical( void )
    { _splitView( Qt::Vertical ); }

    //* display focus changed
    void _displayFocusChanged( TextEditor* );

    //* overwrite mode changed
    void _modifiersChanged( TextEditor::Modifiers );

    //* toggle show keyword
    void _toggleShowKeyword( bool );

    private Q_SLOTS:

    //* configuration
    void _updateConfiguration( void );

    private:

    //* install actions
    void _installActions( void );

    //* create find dialog
    void _createFindWidget( void );

    //* create replace dialog
    void _createReplaceWidget( void );

    //* create select line widget
    void _createSelectLineWidget( void );

    //* if true, LogEntry associated to EditionWindow cannot be modified
    bool readOnly_ = false;

    //* "closed" flag
    /** this flag is used for delayed deletion of EditionWindows, when direct deletion might cause flags */
    bool closed_ = false;

    //* true if keyword is forced visible
    bool forceShowKeyword_ = false;

    //* keyword
    Keyword keyword_;

    //* lock toolbar
    CustomToolBar* lock_ = nullptr;

    //* keyword label
    QLabel* keywordLabel_ = nullptr;

    //* title label
    QLabel* titleLabel_ = nullptr;

    //* Keyword object
    Editor* keywordEditor_ = nullptr;

    //* LogEntry title Object
    Editor *titleEditor_ = nullptr;

    //* color menu
    ColorMenu* colorMenu_ = nullptr;

    //* color widget
    Private::ColorWidget* colorWidget_ = nullptr;

    //* LogEntry text Object
    Private::LocalTextEditor* activeEditor_ = nullptr;

    //* embedded widgets container
    QWidget* container_ = nullptr;

    //* text format bar
    FormatBar* formatBar_ = nullptr;

    //* statusbar
    BaseStatusBar* statusBar_ = nullptr;

    //* menu
    Menu* menu_ = nullptr;

    //*@name widgets (re-implemented from TextEditor)
    //@{

    //* find widget
    BaseFindWidget* findWidget_ = nullptr;

    //* replace widget
    BaseReplaceWidget* replaceWidget_ = nullptr;

    //* line number dialog
    SelectLineWidget* selectLineWidget_ = nullptr;

    //@}

    //*@name actions
    //@{

    //* list of buttons to disactivate in case of read-only
    using ActionList = QList< QAction* >;

    //* list of buttons to disactivate in case of read-only
    ActionList readOnlyActions_;

    //* undo
    QAction* undoAction_ = nullptr;

    //* redo
    QAction* redoAction_ = nullptr;

    //* new entry
    QAction* newEntryAction_ = nullptr;

    //* previous entry action
    QAction* previousEntryAction_ = nullptr;

    //* next entry action
    QAction* nextEntryAction_ = nullptr;

    //* save
    QAction* saveAction_ = nullptr;

    #if USE_ASPELL
    QAction* spellcheckAction_ = nullptr;
    #endif

    //* entry information
    QAction* entryInformationAction_ = nullptr;

    //* print
    QAction* printAction_ = nullptr;

    //* print preview
    QAction* printPreviewAction_ = nullptr;

    //* export to html
    QAction* htmlAction_ = nullptr;

    //* split view horizontal
    QAction* splitViewHorizontalAction_ = nullptr;

    //* split view vertical
    QAction* splitViewVerticalAction_ = nullptr;

    //* new window action
    QAction* cloneWindowAction_ = nullptr;

    //* close view (or window) action
    QAction* closeAction_ = nullptr;

    //* delete enty
    QAction* deleteEntryAction_ = nullptr;

    //* uniconify
    QAction* uniconifyAction_ = nullptr;

    //* show keyword
    QAction* showKeywordAction_ = nullptr;

    //* hyperlink action
    QAction* insertLinkAction_ = nullptr;

    //@}

    QBasicTimer resizeTimer_;

};

#endif
