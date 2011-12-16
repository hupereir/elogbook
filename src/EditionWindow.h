#ifndef EditionWindow_h
#define EditionWindow_h

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

#include "AskForSaveDialog.h"
#include "AnimatedLineEditor.h"
#include "AnimatedTextEditor.h"
#include "AttachmentFrame.h"
#include "BaseMainWindow.h"
#include "Counter.h"
#include "Debug.h"

#include "Config.h"
#include "Key.h"
#include "LogEntry.h"
#include "TextPosition.h"

#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>
#include <QtGui/QLabel>
#include <QtGui/QPrinter>
#include <QtGui/QPushButton>
#include <QtGui/QSplitter>
#include <QtGui/QToolButton>

#include <set>
#include <map>
#include <list>

class Attachment;
class ColorMenu;
class CustomToolBar;
class FormatBar;
class MainWindow;
class Menu;
class StatusBar;

//! log entry edition/creation object
/*!
Note:
though EditionWindows are TopLevel widgets, they are not deleted at window closure
to avoid crash when object is deleted when still within one of its methods.
On the contrary, a close event hides the window, and the MainWindow will delete it
because of that next time it is asked to create a new EditionWindow, thus acting like a
garbage collector
*/

class EditionWindow: public BaseMainWindow, public Counter, public BASE::Key
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    typedef AnimatedLineEditor Editor;

    //! creator
    EditionWindow( QWidget* parent, bool readOnly = true );

    //! destructor
    ~EditionWindow( void );

    //! display all entries informations
    void displayEntry( LogEntry *entry = 0 );

    //! returns current entry
    LogEntry* entry( void ) const
    {
        BASE::KeySet<LogEntry> entries( this );
        assert( entries.size() <= 1 );
        return( entries.size() ) ? *entries.begin():0;
    }

    //! retrieve attachment list
    AttachmentFrame& attachmentFrame( void )
    {
        BASE::KeySet<AttachmentFrame> frames( this );
        assert( frames.size() == 1 );
        return **frames.begin();
    }

    //! status bar
    StatusBar& statusBar( void )
    {
        assert( statusBar_ );
        return *statusBar_;
    }

    //!@name active editor
    //@{

    //! retrieve active display
    AnimatedTextEditor& activeEditor( void )
    { return *activeEditor_; }

    //! retrieve active display
    const AnimatedTextEditor& activeEditor( void ) const
    { return *activeEditor_; }

    //@}

    //! check if this editor is readOnly or not
    bool isReadOnly( void ) const
    { return readOnly_; }

    //! set readOnly state of the EditionWindow
    void setReadOnly( bool );

    //! color menu
    void setColorMenu( ColorMenu* );

    //! closed flag
    bool isClosed( void ) const
    { return closed_; }

    //! closed flag
    void setIsClosed( bool value )
    { closed_ = value; }

    //! check if current entry has been modified or not
    bool modified( void ) const
    {
        return
            keywordEditor_->isModified() ||
            titleEditor_->isModified() ||
            activeEditor().document()->isModified();
    }

    //! computes window title
    QString windowTitle() const;

    //! change window title
    void updateWindowTitle()
    { setWindowTitle( windowTitle() ); }

    //! creates dialog to ask for LogEntry save.
    AskForSaveDialog::ReturnCode askForSave( bool = true );

    //! update keyword Widget from current entry
    void displayKeyword( void );

    //! update title Widget from current entry
    void displayTitle( void );

    //! update color Widget from current entry
    void displayColor( void );

    //! check if current entry has been modified or not
    void setModified( bool );

    //! used to count modified EditionWindows
    class ModifiedFTor
    {
        public:

        //! predicate
        bool operator() (const EditionWindow* frame )
        { return frame->modified() && !frame->isReadOnly() && !frame->isClosed(); }

    };

    //! used to count alive frames, that are not subject to delayed deletion
    class aliveFTor
    {
        public:

        //! predicate
        bool operator() (const EditionWindow* frame )
        { return !frame->isClosed(); }

    };

    //!@name actions
    //@{

    //! new entry action
    QAction& newEntryAction( void ) const
    { return *newEntryAction_; }

    //! previous entry action
    QAction& previousEntryAction( void ) const
    { return *previousEntryAction_; }

    //! next entry action
    QAction& nextEntryAction( void ) const
    { return *nextEntryAction_; }

    //! save
    QAction& saveAction( void ) const
    { return *saveAction_; }

    #if WITH_ASPELL
    //! check spelling of current entry
    QAction& spellCheckAction( void ) const
    { return *spellcheckAction_; }
    #endif

    //! entry information
    QAction& entryInfoAction( void ) const
    { return *entryInfoAction_; }

    //! view html
    QAction& printAction( void ) const
    { return *printAction_; }

    //! split view horizontal
    QAction& splitViewHorizontalAction( void ) const
    { return *splitViewHorizontalAction_; }

    //! split view vertical
    QAction& splitViewVerticalAction( void ) const
    { return *splitViewVerticalAction_; }

    //! split view vertical
    QAction& cloneWindowAction( void ) const
    { return *cloneWindowAction_; }

    //! close view
    QAction& closeAction( void ) const
    { return *closeAction_; }

    //! uniconify
    QAction& uniconifyAction( void ) const
    { return *uniconifyAction_; }

    //! show keyword
    QAction& showKeywordAction( void ) const
    { return *showKeywordAction_; }

    //@}

    //! force keyword visibility
    void setForceShowKeyword( bool value );

    //! close view
    /*! Ask for save if view is modified */
    void closeEditor( AnimatedTextEditor& );

    //! change active display manualy
    void setActiveEditor( AnimatedTextEditor& );

    protected:

    //! close window event handler
    virtual void closeEvent( QCloseEvent *event );

    //! timer event
    virtual void timerEvent( QTimerEvent* );

    //! install actions
    void _installActions( void );

    //!@name display management
    //@{

    //! split view
    AnimatedTextEditor& _splitView( const Qt::Orientation& );

    //! create new splitter
    QSplitter& _newSplitter( const Qt::Orientation&  );

    //! create new TextEditor
    AnimatedTextEditor& _newTextEditor( QWidget* parent );

    //@}

    //! display cursor position
    void _displayCursorPosition( const TextPosition& position );

    //! retrieve associated MainWindow
    MainWindow& _mainWindow( void ) const;

    //! menu
    Menu& _menu( void ) const
    {
        assert( menu_ );
        return *menu_;
    }

    //! update text Widget from current entry
    void _displayText( void );

    //! update attachment list Widget from current entry
    void _displayAttachments( void );

    //! true if status bar is set
    bool _hasStatusBar( void ) const
    { return (bool) statusBar_; }

    //! change keyword (and other widgets) visibility
    void _setKeywordVisible( bool );

    protected slots:

    //! Save Current entry
    void _save( bool updateSelection = true );

    //! creates a new entry
    void _newEntry( void );

    //! splitter moved
    void _splitterMoved( void );

    //! select previous entry
    void _previousEntry( void );

    //! select next entry
    void _nextEntry( void );

    //! show entry info
    void _entryInfo( void );

    //! Delete Current entry
    void _deleteEntry( void );

    //! check spelling of current entry
    void _spellCheck( void );

    //! undo in focused editor (text/title/keyword)
    void _undo( void );

    //! redo in focused editor (text/title/keyword);
    void _redo( void );

    //! clone editor
    void _cloneWindow( void );

    //! unlock read-only editors
    void _unlock( void );

    //! update (enable/disable) undo action
    void _updateUndoAction( void );

    //! update (enable/disable) redo action
    void _updateRedoAction( void );

    //! update (enable/disable) save action
    void _updateSaveAction( void );

    /*!
    \brief update (enable/disable) undo/redo action
    based on the widget that currently has focus
    */
    void _updateUndoRedoActions( QWidget* old, QWidget* current );

    //! Set entry as modified, change window title
    void _textModified( bool );

    //! display cursor position
    void _displayCursorPosition( void )
    { _displayCursorPosition( activeEditor().textPosition() ); }

    //! display cursor position
    void _displayCursorPosition( int, int new_position )
    { _displayCursorPosition( TextPosition( 0, new_position ) ); }

    //! close
    void _close( void )
    {
        Debug::Throw( "EditionWindow::_closeView (SLOT)\n" );
        BASE::KeySet< AnimatedTextEditor > editors( this );
        if( editors.size() > 1 ) closeEditor( activeEditor() );
        else close();
    }

    //! clone current file
    void _splitView( void )
    { _splitView( Qt::Vertical ); }

    //! clone current file horizontal
    void _splitViewHorizontal( void )
    { _splitView( Qt::Horizontal ); }

    //! clone current file horizontal
    void _splitViewVertical( void )
    { _splitView( Qt::Vertical ); }

    //! display focus changed
    void _displayFocusChanged( TextEditor* );

    //! overwrite mode changed
    void _modifiersChanged( unsigned int );

    //! toggle show keyword
    void _toggleShowKeyword( bool );

    private slots:

    //! configuration
    void _updateConfiguration( void );

    private:

    //! if true, LogEntry associated to EditionWindow cannot be modified
    bool readOnly_;

    //! list of buttons to disactivate in case of read-only
    typedef std::vector< QAction* > ActionList;

    //! list of buttons to disactivate in case of read-only
    ActionList readOnlyActions_;

    //! "closed" flag
    /*! this flag is used for delayed deletion of EditionWindows, when direct deletion might cause flags */
    bool closed_;

    //! true if keyword is forced visible
    bool forceShowKeyword_;

    //!@name stored actions to toggle visibility
    //@{

    //! lock toolbar
    CustomToolBar* lock_;

    //@}

    //! local QSplitter object, derived from Counter
    /*! helps keeping track of how many splitters are created/deleted */
    class LocalSplitter: public QSplitter, public Counter
    {

        public:

        //! constructor
        LocalSplitter( QWidget* parent ):
            QSplitter( parent ),
            Counter( "LocalSplitter" )
        { Debug::Throw( "LocalSplitter::LocalSplitter.\n" ); }

        //! destructor
        virtual ~LocalSplitter( void )
        { Debug::Throw( "LocalSplitter::~LocalSplitter.\n" ); }

    };

    //! main widget (that contains first editor)
    QWidget *main_;

    //! labels
    QLabel* keywordLabel_;

    //! labels
    QLabel* titleLabel_;

    //! Keyword object
    Editor* keywordEditor_;

    //! LogEntry title Object
    Editor *titleEditor_;

    //! color menu
    ColorMenu* colorMenu_;

    //! color widget
    class ColorWidget: public QToolButton, public Counter
    {

        public:

        //! constructor
        ColorWidget( QWidget* parent );

        //! color
        void setColor( const QColor& color );

        //! size hint
        QSize sizeHint( void ) const;

        //! size hint
        QSize minimumSizeHint( void ) const;

        protected:

        //! paint event
        void paintEvent( QPaintEvent* );

    };

    //! color widget
    ColorWidget* colorWidget_;

    //! LogEntry text Object
    AnimatedTextEditor *activeEditor_;

    //! text format bar
    FormatBar* formatBar_;

    //! statusbar
    StatusBar* statusBar_;

    //! menu
    Menu* menu_;

    //!@name actions
    //@{

    //! undo
    QAction* undoAction_;

    //! redo
    QAction* redoAction_;

    //! new entry
    QAction* newEntryAction_;

    //! previous entry action
    QAction* previousEntryAction_;

    //! next entry action
    QAction* nextEntryAction_;

    //! save
    QAction* saveAction_;

    #if WITH_ASPELL
    QAction* spellcheckAction_;
    #endif

    //! entry information
    QAction* entryInfoAction_;

    //! view html
    QAction* printAction_;

    //! split view horizontal
    QAction* splitViewHorizontalAction_;

    //! split view vertical
    QAction* splitViewVerticalAction_;

    //! new window action
    QAction* cloneWindowAction_;

    //! close view (or window) action
    QAction* closeAction_;

    //! uniconify
    QAction* uniconifyAction_;

    //! show keyword
    QAction* showKeywordAction_;

    //@}

    QBasicTimer resizeTimer_;

};

#endif
