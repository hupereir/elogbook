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
class MessageWidget;
class MainWindow;
class MenuBar;
class SelectLineWidget;

namespace Private
{

    class ColorWidget;
    class LocalTextEditor;
}

//* log entry edition/creation object
class EditionWindow: public BaseMainWindow, private Base::Counter<EditionWindow>, public Base::Key
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    using Editor = LineEditor;

    //* creator
    explicit EditionWindow( QWidget*, bool readOnly = true );

    //*@name accessors
    //@{

    //* returns current entry
    LogEntry* entry() const
    {
        Base::KeySet<LogEntry> entries( this );
        return( entries.size() ) ? *entries.begin():nullptr;
    }

    //* entry title
    QString entryTitle() const
    { return titleEditor_ ? titleEditor_->text():QString(); }

    //* retrieve active display
    const TextEditor& activeEditor() const;

    //* check if this editor is readOnly or not
    bool isReadOnly() const
    { return readOnly_; }

    //* closed flag
    bool isClosed() const
    { return closed_; }

    //* check if current entry has been modified or not
    bool modified() const
    {
        return
            keywordEditor_->isModified() ||
            titleEditor_->isModified() ||
            activeEditor().document()->isModified();
    }

    //* computes window title
    QString windowTitle() const;

    //@}

    //*@name modifiers
    //@{

    //* display all entries informations
    void displayEntry( Keyword, LogEntry* = nullptr );

    //* display all entries informations
    void displayEntry( LogEntry* = nullptr );

    //* retrieve attachment list
    AttachmentFrame& attachmentFrame()
    {
        Base::KeySet<AttachmentFrame> frames( this );
        Q_ASSERT( frames.size() == 1 );
        return **frames.begin();
    }

    //* status bar
    BaseStatusBar& statusBar()
    { return *statusBar_; }

    //* retrieve active display
    TextEditor& activeEditor();

    //* set readOnly state of the EditionWindow
    void setReadOnly( bool );

    //* color menu
    void setColorMenu( ColorMenu* );

    //* closed flag
    void setIsClosed( bool );

    //* update keyword Widget from current entry
    void displayKeyword();

    //* update title Widget from current entry
    void displayTitle();

    //* update color Widget from current entry
    void displayColor();

    //* check if current entry has been modified or not
    void setModified( bool );

    //* force keyword visibility
    void setForceShowKeyword( bool value );

    //* close view
    /** Ask for save if view is modified */
    void closeEditor( TextEditor& );

    //* change active display manualy
    void setActiveEditor( TextEditor& );

    //* creates dialog to ask for LogEntry save.
    AskForSaveDialog::ReturnCode askForSave();

    //* save to logbook
    /** logbook is updated with the content of the current entry,
    but the logbook itself is not saved */
    void writeEntryToLogbook( bool updateSelection );

    //@}

    //* used to count modified EditionWindows
    class ModifiedFTor
    {
        public:

        //* predicate
        bool operator() (const EditionWindow* frame )
        { return frame->modified() && !frame->isReadOnly() && !frame->isClosed(); }

    };

    //* used to count alive frames, that are not subject to delayed deletion
    using AliveFTor = Base::Functor::UnaryFalse<EditionWindow, &EditionWindow::isClosed>;

    //*@name actions
    //@{

    //* new entry action
    QAction& newEntryAction() const
    { return *newEntryAction_; }

    //* previous entry action
    QAction& previousEntryAction() const
    { return *previousEntryAction_; }

    //* next entry action
    QAction& nextEntryAction() const
    { return *nextEntryAction_; }

    //* save
    QAction& saveAction() const
    { return *saveAction_; }

    #if WITH_ASPELL
    //* check spelling of current entry
    QAction& spellcheckAction() const
    { return *spellcheckAction_; }
    #endif

    //* entry information
    QAction& entryInformationAction() const
    { return *entryInformationAction_; }

    //* revert logbook to saved version
    QAction& reloadAction() const
    { return *reloadAction_; }

    //* print
    QAction& printAction() const
    { return *printAction_; }

    //* print preview
    QAction& printPreviewAction() const
    { return *printPreviewAction_; }

    //* html
    QAction& htmlAction() const
    { return *htmlAction_; }

    //* split view horizontal
    QAction& splitViewHorizontalAction() const
    { return *splitViewHorizontalAction_; }

    //* split view vertical
    QAction& splitViewVerticalAction() const
    { return *splitViewVerticalAction_; }

    //* split view vertical
    QAction& cloneWindowAction() const
    { return *cloneWindowAction_; }

    //* close view
    QAction& closeAction() const
    { return *closeAction_; }

    //* uniconify
    QAction& uniconifyAction() const
    { return *uniconifyAction_; }

    //* show keyword
    QAction& showKeywordAction() const
    { return *showKeywordAction_; }

    //* add hyperLink
    QAction& insertLinkAction() const
    { return *insertLinkAction_; }

    //@}

    Q_SIGNALS:

    //* emitted when new scratch file is created
    void scratchFileCreated( const File& );

    //*@name re-implemented from text editor
    //@{

    //* emitted from TextDisplay when no match is found for find/replace request
    void noMatchFound();

    //* emitted from TextDisplay when no match is found for find/replace request
    void matchFound();

    //* emitted when selected line is not found
    void lineNotFound();

    //* emitted when selected line is found
    void lineFound();

    //@}

    public Q_SLOTS:

    //* update read-only state
    void updateReadOnlyState();

    //* change window title
    void updateWindowTitle()
    { setWindowTitle( windowTitle() ); }

    //*@name reimplemented from TextEditor
    //@{

    //* find text from dialog
    void findFromDialog();

    //* replace text from dialog
    void replaceFromDialog();

    //* select line from dialog
    void selectLineFromDialog();

    //@}

    protected:

    //*@name event filters
    //@{

    //* close window event handler
    void closeEvent( QCloseEvent* ) override;

    //* timer event
    void timerEvent( QTimerEvent* ) override;

    //@}

    private Q_SLOTS:

    //* Save Current entry
    void _save( bool updateSelection = true );

    //* reload entry
    void _reloadEntry();

    //* Print current document
    void _print();

    //* Print current document
    void _print( LogEntryPrintHelper& );

    //* Print current document
    void _printPreview();

    //* export to html
    void _toHtml();

    //* creates a new entry
    void _newEntry();

    //* splitter moved
    void _splitterMoved();

    //* select previous entry
    void _previousEntry();

    //* select next entry
    void _nextEntry();

    //* show entry info
    void _entryInformation();

    //* Delete Current entry
    void _deleteEntry();

    //* check spelling of current entry
    void _spellCheck();

    //* undo in focused editor (text/title/keyword)
    void _undo();

    //* redo in focused editor (text/title/keyword);
    void _redo();

    //* clone editor
    void _cloneWindow();

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
    void _restoreFocus()
    { activeEditor().setFocus(); }

    //* unlock read-only editors
    void _unlock();

    //* insert link
    void _insertLink();

    //* insert link
    void _editLink();

    //* insert link
    void _removeLink();

    //* view link
    void _openLink();

    //* view link
    void _openLink( QString );

    //* update replace in selection action
    void _updateReplaceInSelection();

    //* read only actions
    void _updateReadOnlyActions();

    //* update (enable/disable) save action
    void _updateSaveAction();

    //* update (enable/disable) redo action
    void _updateUndoRedoActions();

    //* update (enable/disable) redo action
    void _updateUndoRedoActions( QWidget*, QWidget* );

    //* update (enable/disable) insert link action
    void _updateInsertLinkActions();

    //* Set entry as modified, change window title
    void _textModified( bool );

    //* display cursor position
    void _displayCursorPosition()
    { _displayCursorPosition( activeEditor().textPosition() ); }

    //* display cursor position
    void _displayCursorPosition( int, int new_position )
    { _displayCursorPosition( TextPosition( 0, new_position ) ); }

    //* close
    void _close();

    //* clone current file
    void _splitView()
    { _splitView( Qt::Vertical ); }

    //* clone current file horizontal
    void _splitViewHorizontal()
    { _splitView( Qt::Horizontal ); }

    //* clone current file horizontal
    void _splitViewVertical()
    { _splitView( Qt::Vertical ); }

    //* display focus changed
    void _displayFocusChanged( TextEditor* );

    //* overwrite mode changed
    void _modifiersChanged( TextEditor::Modifiers );

    //* toggle show keyword
    void _toggleShowKeyword( bool );

    //* configuration
    void _updateConfiguration();

    private:

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
    bool _hasMainWindow() const;

    //* retrieve associated MainWindow
    MainWindow& _mainWindow() const;

    //* update text Widget from current entry
    void _displayText();

    //* update attachment list Widget from current entry
    void _displayAttachments();

    //* true if status bar is set
    bool _hasStatusBar() const
    { return (bool) statusBar_; }

    //* change keyword (and other widgets) visibility
    void _setKeywordVisible( bool );

    //* install actions
    void _installActions();

    //* create find dialog
    void _createFindWidget();

    //* create replace dialog
    void _createReplaceWidget();

    //* create select line widget
    void _createSelectLineWidget();

    //* if true, LogEntry associated to EditionWindow cannot be modified
    bool readOnly_ = false;

    //* "closed" flag
    /** this flag is used for delayed deletion of EditionWindows, when direct deletion might cause flags */
    bool closed_ = false;

    //* true if keyword is forced visible
    bool forceShowKeyword_ = false;

    //* keyword
    Keyword keyword_;

    //* message widget
    MessageWidget* messageWidget_ = nullptr;

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
    MenuBar* menuBar_ = nullptr;

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

    #if WITH_ASPELL
    QAction* spellcheckAction_ = nullptr;
    #endif

    //* entry information
    QAction* entryInformationAction_ = nullptr;

    //* reload
    QAction* reloadAction_ = nullptr;

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
