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

#include "EditionWindow.h"

#include "Application.h"
#include "AttachmentWindow.h"
#include "AttachmentFrame.h"
#include "BaseIcons.h"
#include "ColorMenu.h"
#include "Command.h"
#include "CustomToolBar.h"
#include "File.h"
#include "FormatBar.h"
#include "HtmlHeaderNode.h"
#include "Icons.h"
#include "IconSize.h"
#include "IconEngine.h"
#include "InformationDialog.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryInformationDialog.h"
#include "LogEntryPrintHelper.h"
#include "MainWindow.h"
#include "Menu.h"
#include "Options.h"
#include "PrintPreviewDialog.h"
#include "QuestionDialog.h"
#include "RecentFilesMenu.h"
#include "Singleton.h"
#include "ScratchFileMonitor.h"
#include "StatusBar.h"
#include "Str.h"
#include "Util.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellDialog.h"
#endif

#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPrintDialog>
#include <QtGui/QStylePainter>
#include <QtGui/QStyleOptionToolButton>
#include <QtGui/QTextLayout>

//_______________________________________________
EditionWindow::EditionWindow( QWidget* parent, bool readOnly ):
    BaseMainWindow( parent ),
    Counter( "EditionWindow" ),
    readOnly_( readOnly ),
    closed_( false ),
    forceShowKeyword_( false ),
    colorMenu_( 0 ),
    colorWidget_( 0 ),
    activeEditor_( 0 ),
    formatBar_( 0 ),
    statusBar_( 0 )
{
    Debug::Throw("EditionWindow::EditionWindow.\n" );
    setOptionName( "EDITION_WINDOW" );
    setObjectName( "EDITFRAME" );

    QWidget* main( new QWidget( this ) );
    setCentralWidget( main );

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(2);
    main->setLayout( layout );

    // header layout
    QGridLayout* gridLayout( new QGridLayout() );
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    layout->addLayout( gridLayout );

    // keywoard label and editor
    gridLayout->addWidget( keywordLabel_ = new QLabel( " Keyword: ", main ), 0, 0, 1, 1 );
    gridLayout->addWidget( keywordEditor_ = new Editor( main ), 0, 1, 1, 2 );
    keywordLabel_->setAlignment( Qt::AlignVCenter|Qt::AlignRight );

    // title label and editor
    gridLayout->addWidget( titleLabel_ = new QLabel( " Title: ", main ), 1, 0, 1, 1 );
    gridLayout->addWidget( titleEditor_ = new Editor( main ), 1, 1, 1, 1 );
    titleLabel_->setAlignment( Qt::AlignVCenter|Qt::AlignRight );

    // colorWidget
    gridLayout->addWidget( colorWidget_ = new ColorWidget( main ), 1, 2, 1, 1 );
    colorWidget_->setToolTip( "Change entry color.\nThis is used to tag entries in the main window list." );
    colorWidget_->setAutoRaise( true );
    colorWidget_->setPopupMode( QToolButton::InstantPopup );

    gridLayout->setColumnStretch( 1, 1 );

    // hide everything
    _setKeywordVisible( false );
    colorWidget_->hide();

    // splitter for EditionWindow and attachment list
    QSplitter* splitter = new QSplitter( main );
    splitter->setOrientation( Qt::Vertical );
    layout->addWidget( splitter, 1 );

    // create text
    splitter->addWidget( main_ = new QWidget() );
    main_->setLayout( new QVBoxLayout() );
    main_->layout()->setMargin(0);
    main_->layout()->setSpacing(0);

    // assign stretch factors
    splitter->setStretchFactor( 0, 1 );
    splitter->setStretchFactor( 1, 0 );

    connect( splitter, SIGNAL( splitterMoved( int, int ) ), SLOT( _splitterMoved( void ) ) );

    // create editor
    AnimatedTextEditor& editor( _newTextEditor( main_ ) );
    main_->layout()->addWidget( &editor );

    connect( keywordEditor_, SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );
    connect( keywordEditor_, SIGNAL( cursorPositionChanged( int, int ) ), SLOT( _displayCursorPosition( int, int ) ) );

    connect( titleEditor_, SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );
    connect( titleEditor_, SIGNAL( cursorPositionChanged( int, int ) ), SLOT( _displayCursorPosition( int, int ) ) );

    connect( activeEditor().document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );

    // create attachment list
    AttachmentFrame *frame = new AttachmentFrame( 0, isReadOnly() );
    frame->visibilityAction().setChecked( false );
    frame->setDefaultHeight( XmlOptions::get().get<int>( "ATTACHMENT_FRAME_HEIGHT" ) );
    splitter->addWidget( frame );
    Key::associate( this, frame );

    // status bar for tooltips
    setStatusBar( statusBar_ = new StatusBar( this ) );
    statusBar().addLabel( 2, true );
    statusBar().addLabels( 3, 0 );
    statusBar().addClock();

    // actions
    _installActions();
    Application& application( *Singleton::get().application<Application>() );
    addAction( &application.closeAction() );

    // toolbars
    // lock toolbar is visible only when window is not editable
    lock_ = new CustomToolBar( "Lock", this, "LOCK_TOOLBAR" );
    lock_->setMovable( false );

    // hide lock_ visibility action because the latter should not be checkable in any menu
    lock_->visibilityAction().setVisible( false );

    QAction *action;
    lock_->addAction( action = new QAction( IconEngine::get( ICONS::LOCK ), "&Unlock", this ) );
    connect( action, SIGNAL( triggered() ), SLOT( _unlock() ) );
    action->setToolTip( "Remove read-only lock for current editor." );

    // main toolbar
    CustomToolBar* toolbar;
    toolbar = new CustomToolBar( "Main", this, "MAIN_TOOLBAR" );

    // new entry
    toolbar->addAction( &newEntryAction() );
    toolbar->addAction( &saveAction() );

    // delete_entry button
    toolbar->addAction( action = new QAction( IconEngine::get( ICONS::DELETE ), "&Delete Entry", this ) );
    connect( action, SIGNAL( triggered() ), SLOT( _deleteEntry() ) );
    readOnlyActions_.push_back( action );

    // add_attachment button
    toolbar->addAction( &frame->newAction() );

    // format bar
    formatBar_ = new FormatBar( this, "FORMAT_TOOLBAR" );
    formatBar_->setTarget( activeEditor() );
    const FormatBar::ActionMap& actions( formatBar_->actions() );
    for( FormatBar::ActionMap::const_iterator iter = actions.begin(); iter != actions.end(); ++iter )
    { readOnlyActions_.push_back( iter->second ); }

    // set proper connection for first editor
    // (because it could not be performed in _newTextEditor)
    connect(
        &editor, SIGNAL( currentCharFormatChanged( const QTextCharFormat& ) ),
        formatBar_, SLOT( updateState( const QTextCharFormat& ) ) );

    // edition toolbars
    toolbar = new CustomToolBar( "History", this, "EDITION_TOOLBAR" );
    toolbar->addAction( undoAction_ );
    toolbar->addAction( redoAction_ );
    readOnlyActions_.push_back( undoAction_ );
    readOnlyActions_.push_back( redoAction_ );

    // undo/redo connections
    connect( keywordEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateUndoAction() ) );
    connect( keywordEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateRedoAction() ) );
    connect( keywordEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateSaveAction() ) );

    connect( titleEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateUndoAction() ) );
    connect( titleEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateRedoAction() ) );
    connect( titleEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateSaveAction() ) );

    connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );
    connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );

    // extra toolbar
    toolbar = new CustomToolBar( "Tools", this, "EXTRA_TOOLBAR" );

    #if WITH_ASPELL
    toolbar->addAction( &spellCheckAction() );
    #endif

    toolbar->addAction( &printAction() );
    toolbar->addAction( &entryInfoAction() );

    // extra toolbar
    toolbar = new CustomToolBar( "Multiple Views", this, "MULTIPLE_VIEW_TOOLBAR" );
    toolbar->addAction( &splitViewHorizontalAction() );
    toolbar->addAction( &splitViewVerticalAction() );
    toolbar->addAction( &cloneWindowAction() );
    toolbar->addAction( &closeAction() );

    // extra toolbar
    toolbar = new CustomToolBar( "Navigation", this, "NAVIGATION_TOOLBAR" );
    toolbar->addAction( &application.mainWindow().uniconifyAction() );
    toolbar->addAction( &previousEntryAction() );
    toolbar->addAction( &nextEntryAction() );

    // create menu if requested
    menu_ = new Menu( this, &Singleton::get().application<Application>()->mainWindow() );
    setMenuBar( menu_ );

    // changes display according to readOnly flag
    setReadOnly( readOnly_ );

    // configuration
    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    _updateConfiguration();
    Debug::Throw("EditionWindow::EditionWindow - done.\n" );

}

//____________________________________________
EditionWindow::~EditionWindow( void )
{ Debug::Throw( "EditionWindow::~EditionWindow.\n" ); }

//____________________________________________
void EditionWindow::displayEntry( LogEntry *entry )
{
    Debug::Throw( "EditionWindow::displayEntry.\n" );

    // disassociate with existing entries, if any
    clearAssociations<LogEntry>();

    // retrieve selection frame
    MainWindow &mainwindow( _mainWindow() );
    _menu().recentFilesMenu().setCurrentFile( mainwindow.menu().recentFilesMenu().currentFile() );

    // check entry
    if( !entry ) return;

    // update current entry
    Key::associate( entry, this );

    // update all display
    displayKeyword();
    displayTitle();
    displayColor();
    _displayText();
    _displayAttachments();

    // update previous and next action states
    Debug::Throw( "EditionWindow::displayEntry - setting button states.\n" );
    previousEntryAction().setEnabled( mainwindow.previousEntry(entry, false) );
    nextEntryAction().setEnabled( mainwindow.nextEntry(entry, false) );

    // reset modify flag; change title accordingly
    setModified( false );
    updateWindowTitle();

    Debug::Throw( "EditionWindow::displayEntry - done.\n" );
    return;
}

//____________________________________________
void EditionWindow::setReadOnly( bool value )
{

    Debug::Throw() << "EditionWindow::setReadOnly - " << (value ? "true":"false" ) << endl;

    // update readOnly value
    readOnly_ = value;

    // changes button state
    for( ActionList::iterator it=readOnlyActions_.begin(); it != readOnlyActions_.end(); it++ )
    { (*it)->setEnabled( !isReadOnly() ); }

    // changes lock button state
    if( isReadOnly() && lock_->isHidden() )
    {

        Qt::ToolBarArea current_location = toolBarArea( lock_ );
        if( current_location == Qt::NoToolBarArea ) { addToolBar( Qt::LeftToolBarArea, lock_ ); }
        lock_->show();

    } else if( !(isReadOnly() || lock_->isHidden() ) ) { lock_->hide(); }

    // changes TextEdit readOnly status
    keywordEditor_->setReadOnly( isReadOnly() );
    titleEditor_->setReadOnly( isReadOnly() );

    // update editors
    BASE::KeySet<AnimatedTextEditor> editors( this );
    for( BASE::KeySet<AnimatedTextEditor>::iterator iter = editors.begin(); iter != editors.end(); ++iter )
    { (*iter)->setReadOnly( isReadOnly() ); }

    // changes attachment list status
    attachmentFrame().setReadOnly( isReadOnly() );

    // changes window title
    _updateSaveAction();
    updateWindowTitle();

}

//_____________________________________________
void EditionWindow::setColorMenu( ColorMenu* menu )
{
    colorMenu_ = menu;
    if( colorWidget_ && !colorWidget_->menu() )
    { colorWidget_->setMenu( menu ); }
}

//_____________________________________________
QString EditionWindow::windowTitle( void ) const
{

    Debug::Throw( "EditionWindow::windowTitle.\n" );
    LogEntry* entry( EditionWindow::entry() );

    QString buffer;
    QTextStream what( &buffer );
    if( entry )
    {

        what << entry->title();
        if( entry->keyword() != Keyword::NO_KEYWORD ) what << " - " << entry->keyword().get();

    } else what << "Electronic Logbook Editor";

    if( isReadOnly() ) what << " (read only)";
    else if( modified()  ) what << " (modified)";
    return buffer;

}

//____________________________________________
AskForSaveDialog::ReturnCode EditionWindow::askForSave( bool enableCancel )
{

    Debug::Throw( "EditionWindow::askForSave.\n" );

    // retrieve other editFrames
    BASE::KeySet<EditionWindow> editionwindows( &_mainWindow() );
    unsigned int count( count_if( editionwindows.begin(), editionwindows.end(), ModifiedFTor() ) );

    // create dialog
    unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
    if( enableCancel ) buttons |= AskForSaveDialog::CANCEL;
    if( count > 1 ) buttons |= AskForSaveDialog::ALL;
    AskForSaveDialog dialog( this, "Entry has been modified. Save ?", buttons );

    // exec and check return code
    int state = dialog.centerOnParent().exec();
    if( state == AskForSaveDialog::YES ) _save( false );
    else if( state == AskForSaveDialog::ALL )
    {
        /*
        save_all: if the logbook has no valid file one save the modified editionwindows one by one
        otherwise one directly save the loogbook, while disabling the confirmation for modified
        entries
        */
        if( _mainWindow().logbook()->file().isEmpty() )
        {
            for( BASE::KeySet<EditionWindow>::iterator iter = editionwindows.begin(); iter!= editionwindows.end(); ++iter )
            { if( (*iter)->modified() && !(*iter)->isReadOnly() ) (*iter)->_save(enableCancel); }
        } else _mainWindow().save( false );
    }

    return AskForSaveDialog::ReturnCode(state);

}

//_____________________________________________
void EditionWindow::displayKeyword( void )
{
    Debug::Throw( "EditionWindow::displayKeyword.\n" );

    LogEntry* entry( EditionWindow::entry() );
    keywordEditor_->setText( ( entry && entry->keyword().get().size() ) ? entry->keyword().get(): LogEntry::UNTITLED  );
    keywordEditor_->setCursorPosition( 0 );
    return;
}

//_____________________________________________
void EditionWindow::displayTitle( void )
{
    Debug::Throw( "EditionWindow::displayTitle.\n" );

    LogEntry* entry( this->entry() );
    titleEditor_->setText( ( entry && entry->title().size() ) ? entry->title(): LogEntry::UNTITLED  );
    titleEditor_->setCursorPosition( 0 );
    return;
}

//_____________________________________________
void EditionWindow::displayColor( void )
{
    Debug::Throw( "EditionWindow::DisplayColor.\n" );

    // try load entry color
    QColor color;
    Str colorname( entry()->color() );
    if( colorname.compare( ColorMenu::NONE, Qt::CaseInsensitive ) == 0 || !( color = QColor( colorname ) ).isValid() )
    {

        colorWidget_->hide();

    } else {

        colorWidget_->setColor( color );
        colorWidget_->show();

    }

    return;

}

//______________________________________________________
void EditionWindow::setModified( bool value )
{
    Debug::Throw( "EditionWindow::setModified.\n" );
    keywordEditor_->setModified( value );
    titleEditor_->setModified( value );
    activeEditor().document()->setModified( value );
}

//___________________________________________________________
void EditionWindow::setForceShowKeyword( bool value )
{
    Debug::Throw( "EditionWindow::setForceShowKeyword.\n" );
    _setKeywordVisible( value || showKeywordAction().isChecked() );
    showKeywordAction().setEnabled( !value );
}

//___________________________________________________________
void EditionWindow::closeEditor( AnimatedTextEditor& editor )
{
    Debug::Throw( "EditionWindow::closeEditor.\n" );

    // retrieve number of editors
    // if only one display, close the entire window
    BASE::KeySet<AnimatedTextEditor> editors( this );
    if( editors.size() < 2 )
    {
        Debug::Throw() << "EditionWindow::closeEditor - full close." << endl;
        close();
        return;
    }

    // retrieve parent and grandparent of current display
    QWidget* parent( editor.parentWidget() );
    QSplitter* parent_splitter( qobject_cast<QSplitter*>( parent ) );

    // retrieve editors associated to current
    editors = BASE::KeySet<AnimatedTextEditor>( &editor );

    // check how many children remain in parent_splitter if any
    // take action if it is less than 2 (the current one to be deleted, and another one)
    if( parent_splitter && parent_splitter->count() <= 2 )
    {

        // retrieve child that is not the current editor
        // need to loop over existing widgets because the editor above has not been deleted yet
        QWidget* child(0);
        for( int index = 0; index < parent_splitter->count(); index++ )
        {
            if( parent_splitter->widget( index ) != &editor )
            {
                child = parent_splitter->widget( index );
                break;
            }
        }
        assert( child );
        Debug::Throw( "EditionWindow::closeEditor - found child.\n" );

        // retrieve splitter parent
        QWidget* grand_parent( parent_splitter->parentWidget() );

        // try cast to a splitter
        QSplitter* grand_parent_splitter( qobject_cast<QSplitter*>( grand_parent ) );

        // move child to grand_parent_splitter if any
        if( grand_parent_splitter )
        {

            grand_parent_splitter->insertWidget( grand_parent_splitter->indexOf( parent_splitter ), child );

        }  else {

            child->setParent( grand_parent );
            grand_parent->layout()->addWidget( child );

        }

        // delete parent_splitter, now that it is empty
        parent_splitter->deleteLater();
        Debug::Throw( "EditionWindow::closeEditor - deleted splitter.\n" );

    } else {

        // the editor is deleted only if its parent splitter is not
        // otherwise this will trigger double deletion of the editor
        // which will then crash
        editor.deleteLater();

    }

    // update activeEditor
    bool active_found( false );
    for( BASE::KeySet<AnimatedTextEditor>::reverse_iterator iter = editors.rbegin(); iter != editors.rend(); ++iter )
    {
        if( (*iter) != &editor ) {
            setActiveEditor( **iter );
            active_found = true;
            break;
        }
    }
    assert( active_found );

    // change focus
    activeEditor().setFocus();
    Debug::Throw( "EditionWindow::closeEditor - done.\n" );

}

//________________________________________________________________
void EditionWindow::setActiveEditor( AnimatedTextEditor& editor )
{
    Debug::Throw() << "EditionWindow::setActiveEditor - key: " << editor.key() << endl;
    assert( editor.isAssociated( this ) );

    activeEditor_ = &editor;
    if( !activeEditor().isActive() )
    {

        BASE::KeySet<AnimatedTextEditor> editors( this );
        for( BASE::KeySet<AnimatedTextEditor>::iterator iter = editors.begin(); iter != editors.end(); ++iter )
        { (*iter)->setActive( false ); }

        activeEditor().setActive( true );

    }

    // associate with toolbar
    if( formatBar_ ) formatBar_->setTarget( activeEditor() );

    Debug::Throw( "EditionWindow::setActiveEditor - done.\n" );

}

//____________________________________________
void EditionWindow::closeEvent( QCloseEvent *event )
{
    Debug::Throw( "EditionWindow::closeEvent.\n" );

    // ask for save if entry is modified
    if( !(isReadOnly() || isClosed() ) && modified() && askForSave() == AskForSaveDialog::CANCEL ) event->ignore();
    else
    {
        setIsClosed( true );
        event->accept();
    }

    return;
}

//_______________________________________________________
void EditionWindow::timerEvent( QTimerEvent* event )
{

    if( event->timerId() == resizeTimer_.timerId() )
    {

        // stop timer
        resizeTimer_.stop();

        // save size
        if( attachmentFrame().visibilityAction().isChecked() )
        { XmlOptions::get().set<int>( "ATTACHMENT_FRAME_HEIGHT", attachmentFrame().height() ); }

    } else return BaseMainWindow::timerEvent( event );

}

//_____________________________________________
void EditionWindow::_installActions( void )
{
    Debug::Throw( "EditionWindow::_installActions.\n" );

    // undo action
    addAction( undoAction_ = new QAction( IconEngine::get( ICONS::UNDO ), "Undo", this ) );
    undoAction_->setToolTip( "Undo last modification" );
    connect( undoAction_, SIGNAL( triggered() ), SLOT( _undo() ) );

    // redo action
    addAction( redoAction_ = new QAction( IconEngine::get( ICONS::REDO ), "Redo", this ) );
    redoAction_->setToolTip( "Redo last undone modification" );
    connect( redoAction_, SIGNAL( triggered() ), SLOT( _redo() ) );

    // new entry
    addAction( newEntryAction_ = new QAction( IconEngine::get( ICONS::NEW ), "New Entry", this ) );
    newEntryAction_->setShortcut( Qt::CTRL + Qt::Key_N );
    newEntryAction_->setToolTip( "Create new entry in current editor" );
    connect( newEntryAction_, SIGNAL( triggered() ), SLOT( _newEntry() ) );

    // previous_entry action
    addAction( previousEntryAction_ = new QAction( IconEngine::get( ICONS::PREV ), "Previous Entry", this ) );
    previousEntryAction_->setToolTip( "Display previous entry in current list" );
    connect( previousEntryAction_, SIGNAL( triggered() ), SLOT( _previousEntry() ) );

    // previous_entry action
    addAction( nextEntryAction_ = new QAction( IconEngine::get( ICONS::NEXT ), "Next Entry", this ) );
    nextEntryAction_->setToolTip( "Display next entry in current list" );
    connect( nextEntryAction_, SIGNAL( triggered() ), SLOT( _nextEntry() ) );
    nextEntryAction_->setShortcut( Qt::CTRL+Qt::Key_N );

    // save
    addAction( saveAction_ = new QAction( IconEngine::get( ICONS::SAVE ), "Save Entry", this ) );
    saveAction_->setToolTip( "Save current entry" );
    connect( saveAction_, SIGNAL( triggered() ), SLOT( _save() ) );
    saveAction_->setShortcut( Qt::CTRL+Qt::Key_S );

    #if WITH_ASPELL
    addAction( spellcheckAction_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "Spellcheck", this ) );
    spellcheckAction_->setToolTip( "Check spelling of current entry" );
    connect( spellcheckAction_, SIGNAL( triggered() ), SLOT( _spellCheck() ) );
    #endif

    // entry_info
    addAction( entryInfoAction_ = new QAction( IconEngine::get( ICONS::INFO ), "Entry Information", this ) );
    entryInfoAction_->setToolTip( "Show current entry information" );
    connect( entryInfoAction_, SIGNAL( triggered() ), SLOT( _entryInfo() ) );

    // print
    addAction( printAction_ = new QAction( IconEngine::get( ICONS::PRINT ), "Print", this ) );
    printAction_->setToolTip( "Print current logbook entry" );
    printAction_->setShortcut( Qt::CTRL + Qt::Key_P );
    connect( printAction_, SIGNAL( triggered() ), SLOT( _print() ) );

    // print preview
    addAction( printPreviewAction_ = new QAction( IconEngine::get( ICONS::PRINT_PREVIEW ), "Print Preview", this ) );
    printPreviewAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_P );
    connect( printPreviewAction_, SIGNAL( triggered() ), SLOT( _printPreview() ) );

    // split action
    addAction( splitViewHorizontalAction_ =new QAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM ), "Split View Top/Bottom", this ) );
    splitViewHorizontalAction_->setToolTip( "Split current text editor vertically" );
    connect( splitViewHorizontalAction_, SIGNAL( triggered() ), SLOT( _splitViewVertical() ) );

    addAction( splitViewVerticalAction_ =new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), "Split View Left/Right", this ) );
    splitViewVerticalAction_->setToolTip( "Split current text editor horizontally" );
    connect( splitViewVerticalAction_, SIGNAL( triggered() ), SLOT( _splitViewHorizontal() ) );

    // clone window action
    addAction( cloneWindowAction_ = new QAction( IconEngine::get( ICONS::VIEW_CLONE ), "Clone Window", this ) );
    cloneWindowAction_->setToolTip( "Create a new edition window displaying the same entry" );
    connect( cloneWindowAction_, SIGNAL( triggered() ), SLOT( _cloneWindow() ) );

    // close window action
    addAction( closeAction_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE ), "Close View", this ) );
    closeAction_->setShortcut( Qt::CTRL+Qt::Key_W );
    closeAction_->setToolTip( "Close current view" );
    connect( closeAction_, SIGNAL( triggered() ), SLOT( _close() ) );

    // uniconify
    addAction( uniconifyAction_ = new QAction( "Uniconify", this ) );
    connect( uniconifyAction_, SIGNAL( triggered() ), SLOT( uniconify() ) );

    // show/hide keyword
    addAction( showKeywordAction_ = new QAction( "Show Keyword", this ) );
    showKeywordAction_->setCheckable( true );
    showKeywordAction_->setChecked( false );
    connect( showKeywordAction_, SIGNAL( toggled( bool ) ), SLOT( _toggleShowKeyword( bool ) ) );

}

//___________________________________________________________
AnimatedTextEditor& EditionWindow::_splitView( const Qt::Orientation& orientation )
{
    Debug::Throw( "EditionWindow::_splitView.\n" );

    // keep local pointer to current active display
    AnimatedTextEditor& activeEditor_local( activeEditor() );

    // compute desired dimension of the new splitter
    // along its splitting direction
    int dimension( (orientation == Qt::Horizontal) ? activeEditor_local.width():activeEditor_local.height() );

    // create new splitter
    QSplitter& splitter( _newSplitter( orientation ) );

    // create new display
    AnimatedTextEditor& editor( _newTextEditor(0) );

    // insert in splitter, at correct position
    splitter.insertWidget( splitter.indexOf( &activeEditor_local )+1, &editor );

    // recompute dimension
    // take the max of active display and splitter,
    // in case no new splitter was created.
    dimension = std::max( dimension, (orientation == Qt::Horizontal) ? splitter.width():splitter.height() );

    // assign equal size to all splitter children
    QList<int> sizes;
    for( int i=0; i<splitter.count(); i++ )
    { sizes.push_back( dimension/splitter.count() ); }
    splitter.setSizes( sizes );

    // synchronize both editors, if cloned
    /*
    if there exists no clone of active display,
    backup text and register a new Sync object
    */
    BASE::KeySet<AnimatedTextEditor> editors( &activeEditor_local );

    // clone new display
    editor.synchronize( &activeEditor_local );

    // perform associations
    // check if active editors has associates and propagate to new
    for( BASE::KeySet<AnimatedTextEditor>::iterator iter = editors.begin(); iter != editors.end(); ++iter )
    { BASE::Key::associate( &editor, *iter ); }

    // associate new display to active
    BASE::Key::associate( &editor, &activeEditor_local );

    return editor;

}

//____________________________________________________________
QSplitter& EditionWindow::_newSplitter( const Qt::Orientation& orientation )
{

    Debug::Throw( "EditionWindow::_newSplitter.\n" );
    QSplitter *splitter = 0;

    // retrieve parent of current display
    QWidget* parent( activeEditor().parentWidget() );

    // try cast to splitter
    // do not create a new splitter if the parent has same orientation
    QSplitter *parent_splitter( qobject_cast<QSplitter*>( parent ) );
    if( parent_splitter && parent_splitter->orientation() == orientation ) {

        Debug::Throw( "EditionWindow::_newSplitter - orientation match. No need to create new splitter.\n" );
        splitter = parent_splitter;

    } else {


        // move splitter to the first place if needed
        if( parent_splitter )
        {

            Debug::Throw( "EditionWindow::_newSplitter - found parent splitter with incorrect orientation.\n" );
            // create a splitter with correct orientation
            // give him no parent, because the parent is set in QSplitter::insertWidget()
            splitter = new LocalSplitter(0);
            splitter->setOrientation( orientation );
            parent_splitter->insertWidget( parent_splitter->indexOf( &activeEditor() ), splitter );

        } else {

            Debug::Throw( "EditionWindow::_newSplitter - no splitter found. Creating a new one.\n" );

            // create a splitter with correct orientation
            splitter = new LocalSplitter(parent);
            splitter->setOrientation( orientation );
            parent->layout()->addWidget( splitter );

        }

        // reparent current display
        splitter->addWidget( &activeEditor() );

        // resize parent splitter if any
        if( parent_splitter )
        {
            int dimension = ( parent_splitter->orientation() == Qt::Horizontal) ?
                parent_splitter->width():
                parent_splitter->height();

            QList<int> sizes;
            for( int i=0; i<parent_splitter->count(); i++ )
            { sizes.push_back( dimension/parent_splitter->count() ); }
            parent_splitter->setSizes( sizes );

        }

    }

    // return created splitter
    return *splitter;

}

//_____________________________________________________________
AnimatedTextEditor& EditionWindow::_newTextEditor( QWidget* parent )
{
    Debug::Throw( "EditionWindow::_newTextEditor.\n" );

    // create textDisplay
    AnimatedTextEditor* editor = new AnimatedTextEditor( parent );

    // connections
    connect( editor, SIGNAL( hasFocus( TextEditor* ) ), SLOT( _displayFocusChanged( TextEditor* ) ) );
    connect( editor, SIGNAL( cursorPositionChanged() ), SLOT( _displayCursorPosition() ) );
    connect( editor, SIGNAL( modifiersChanged( unsigned int ) ), SLOT( _modifiersChanged( unsigned int ) ) );
    connect( editor, SIGNAL( undoAvailable( bool ) ), SLOT( _updateUndoAction() ) );
    connect( editor, SIGNAL( redoAvailable( bool ) ), SLOT( _updateRedoAction() ) );
    connect( editor->document(), SIGNAL( modificationChanged( bool ) ), SLOT( _updateSaveAction() ) );

    if( formatBar_ )
    {
        connect(
            editor, SIGNAL( currentCharFormatChanged( const QTextCharFormat& ) ),
            formatBar_, SLOT( updateState( const QTextCharFormat& ) ) );
    }

    // associate display to this editFrame
    BASE::Key::associate( this, editor );

    // update current display and focus
    setActiveEditor( *editor );
    editor->setFocus();
    Debug::Throw() << "EditionWindow::_newTextEditor - key: " << editor->key() << endl;
    Debug::Throw( "EditionWindow::_newTextEditor - done.\n" );

    return *editor;

}

//_____________________________________________
void EditionWindow::_displayCursorPosition( const TextPosition& position)
{
    Debug::Throw( "EditionWindow::_DisplayCursorPosition.\n" );
    if( !_hasStatusBar() ) return;

    QString buffer;
    QTextStream( &buffer ) << "Line: " << position.paragraph()+1;
    statusBar().label(2).setText( buffer, false );

    buffer.clear();
    QTextStream( &buffer )  << "Column: " << position.index()+1;
    statusBar().label(3).setText( buffer, true );

    return;
}

//_______________________________________________
MainWindow& EditionWindow::_mainWindow( void ) const
{
    Debug::Throw( "EditionWindow::_mainWindow.\n" );
    BASE::KeySet<MainWindow> mainwindows( this );
    assert( mainwindows.size()==1 );
    return **mainwindows.begin();
}

//_____________________________________________
void EditionWindow::_displayText( void )
{
    Debug::Throw( "EditionWindow::_displayText.\n" );
    if( !&activeEditor() ) return;

    LogEntry* entry( EditionWindow::entry() );
    activeEditor().setCurrentCharFormat( QTextCharFormat() );
    activeEditor().setPlainText( (entry) ? entry->text() : "" );
    formatBar_->load( entry->formats() );

    // reset undo/redo stack
    activeEditor().resetUndoRedoStack();

    return;
}

//_____________________________________________
void EditionWindow::_displayAttachments( void )
{
    Debug::Throw( "EditionWindow::_DisplayAttachments.\n" );

    AttachmentFrame &frame( attachmentFrame() );
    frame.clear();

    LogEntry* entry( EditionWindow::entry() );
    if( !entry ) {

        frame.visibilityAction().setChecked( false );
        return;

    }

    // get associated attachments
    BASE::KeySet<Attachment> attachments( entry );
    if( attachments.empty() ) {

        frame.visibilityAction().setChecked( false );
        return;

    } else {

        frame.visibilityAction().setChecked( true );
        frame.add( AttachmentModel::List( attachments.begin(), attachments.end() ) );

    }

    return;

}

//_____________________________________________
void EditionWindow::_setKeywordVisible( bool value )
{
    Debug::Throw( "EditionWindow::_setKeywordVisible.\n" );
    keywordLabel_->setVisible( value );
    keywordEditor_->setVisible( value );
    titleLabel_->setVisible( value );
}

//_____________________________________________
void EditionWindow::_save( bool updateSelection )
{

    Debug::Throw( "EditionWindow::_save.\n" );
    if( isReadOnly() ) return;

    // retrieve associated entry
    LogEntry *entry( this->entry() );

    // see if entry is new
    bool entryIsNew( !entry || BASE::KeySet<Logbook>( entry ).empty() );

    // create entry if none set
    if( !entry ) entry = new LogEntry();

    // check logbook
    MainWindow &mainwindow( _mainWindow() );
    Logbook *logbook( mainwindow.logbook() );
    if( !logbook ) {
        InformationDialog( this, "No logbook opened. <Save> canceled." ).exec();
        return;
    }
    Debug::Throw( "EditionWindow::_save - logbook checked.\n" );

    //! update entry text
    entry->setText( activeEditor().toPlainText() );
    entry->setFormats( formatBar_->get() );

    //! update entry keyword
    entry->setKeyword( keywordEditor_->text() );

    //! update entry title
    entry->setTitle( titleEditor_->text() );

    // update author
    entry->setAuthor( XmlOptions::get().raw( "USER" ) );

    // add _now_ to entry modification timestamps
    entry->modified();

    // status bar
    statusBar().label().setText(  "writting entry to logbook ..." );
    Debug::Throw( "EditionWindow::_save - statusbar set.\n" );

    // add entry to logbook, if needed
    if( entryIsNew ) Key::associate( entry, logbook->latestChild() );

    // update this window title, set unmodified.
    setModified( false );
    updateWindowTitle();
    Debug::Throw( "EditionWindow::_save - modified state saved.\n" );

    // update associated EditionWindows
    BASE::KeySet<EditionWindow> editors( entry );
    for( BASE::KeySet<EditionWindow>::iterator iter = editors.begin(); iter != editors.end(); ++iter )
    {
        assert( *iter == this || (*iter)->isReadOnly() || (*iter)->isClosed() );
        if( *iter == this ) continue;
        (*iter)->displayEntry( entry );
    }
    Debug::Throw( "EditionWindow::_save - editFrames updated.\n" );

    // update selection frame
    mainwindow.updateEntry( entry, updateSelection );
    mainwindow.setWindowTitle( Application::MAIN_TITLE_MODIFIED );
    Debug::Throw( "EditionWindow::_save - mainWindow updated.\n" );

    // set logbook as modified
    BASE::KeySet<Logbook> logbooks( entry );
    for( BASE::KeySet<Logbook>::iterator iter = logbooks.begin(); iter!= logbooks.end(); ++iter )
    { (*iter)->setModified( true ); }

    Debug::Throw( "EditionWindow::_save - loogbook modified state updated.\n" );

    // add to main logbook recent entries
    mainwindow.logbook()->addRecentEntry( entry );

    // Save logbook
    if( mainwindow.logbook()->file().size() )
    { mainwindow.save(); }

    Debug::Throw( "EditionWindow::_save - mainWindow saved.\n" );

    statusBar().label().setText( "" );
    Debug::Throw( "EditionWindow::_save - done.\n" );

    return;

}

//___________________________________________________________
void EditionWindow::_print( void )
{
    Debug::Throw( "EditionWindow::_print.\n" );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create printer
    QPrinter printer( QPrinter::HighResolution );

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setWindowTitle( "Print Document - qedit" );
    if( dialog.exec() == QDialog::Rejected ) return;

    // add output file to scratch files, if any
    if( !printer.outputFileName().isEmpty() )
    { Singleton::get().application<Application>()->scratchFileMonitor().add( printer.outputFileName() ); }

    // print
    LogEntryPrintHelper( this, entry() ).print( &printer );

    return;

}

//___________________________________________________________
void EditionWindow::_printPreview( void )
{
    Debug::Throw( "EditionWindow::_printPreview.\n" );

    // create helper
    LogEntryPrintHelper helper( this, entry() );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this );
    connect( dialog.previewWidget(), SIGNAL( paintRequested( QPrinter* ) ), &helper, SLOT( print( QPrinter* ) ) );
    dialog.exec();
}

//_____________________________________________
void EditionWindow::_newEntry( void )
{

    Debug::Throw( "EditionWindow::_newEntry.\n" );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create new entry, set author, set keyword
    LogEntry* entry = new LogEntry();
    entry->setAuthor( XmlOptions::get().raw( "USER" ) );
    entry->setKeyword( _mainWindow().currentKeyword() );

    // add logbook parent if any
    // Logbook *logbook( _mainWindow().logbook() );
    // if( logbook ) Key::associate( entry, logbook->latestChild() );

    // display new entry
    displayEntry( entry );

    // set focus to title bar
    titleEditor_->setFocus();
    titleEditor_->selectAll();

}

//_____________________________________________
void EditionWindow::_splitterMoved( void )
{
    Debug::Throw( "MainWindow::_splitterMoved.\n" );
    resizeTimer_.start( 200, this );
}

//_______________________________________________
void EditionWindow::_previousEntry( void )
{
    Debug::Throw( "EditionWindow::_previousEntry.\n" );

    MainWindow &mainwindow( _mainWindow() );
    LogEntry* entry( mainwindow.previousEntry( EditionWindow::entry(), true ) );
    if( !( entry  && mainwindow.lockEntry( entry ) ) ) return;
    displayEntry( entry );
    setReadOnly( false );

}

//_______________________________________________
void EditionWindow::_nextEntry( void )
{
    Debug::Throw( "EditionWindow::_nextEntry.\n" );

    MainWindow &mainwindow( _mainWindow() );
    LogEntry* entry( mainwindow.nextEntry( EditionWindow::entry(), true ) );
    if( !( entry && mainwindow.lockEntry( entry ) ) ) return;
    displayEntry( entry );
    setReadOnly( false );

}

//_____________________________________________
void EditionWindow::_entryInfo( void )
{

    Debug::Throw( "EditionWindow::_EntryInfo.\n" );

    // check entry
    LogEntry *entry( EditionWindow::entry() );
    if( !entry ) {
        InformationDialog( this, "No valid entry."  ).exec();
        return;
    }

    // create dialog
    LogEntryInformationDialog( this, entry ).centerOnParent().exec();

}

//_____________________________________________
void EditionWindow::_undo( void )
{
    Debug::Throw( "EditionWindow::_undo.\n" );
    if( activeEditor().QWidget::hasFocus() ) activeEditor().document()->undo();
    else if( titleEditor_->hasFocus() ) titleEditor_->undo();
    else if( keywordEditor_->hasFocus() ) keywordEditor_->undo();
    return;
}

//_____________________________________________
void EditionWindow::_redo( void )
{
    Debug::Throw( "EditionWindow::_redo.\n" );
    if( activeEditor().QWidget::hasFocus() ) activeEditor().document()->redo();
    else if( titleEditor_->hasFocus() ) titleEditor_->redo();
    else if( keywordEditor_->hasFocus() ) keywordEditor_->redo();
    return;
}

//_____________________________________________
void EditionWindow::_updateUndoAction( void )
{
    Debug::Throw( "EditionWindow::_updateUndoAction.\n" );
    if( keywordEditor_->hasFocus() ) undoAction_->setEnabled( keywordEditor_->isUndoAvailable() );
    else if( titleEditor_->hasFocus() ) undoAction_->setEnabled( titleEditor_->isUndoAvailable() );
    else if( activeEditor().QWidget::hasFocus() ) undoAction_->setEnabled( activeEditor().document()->isUndoAvailable() );
}

//_____________________________________________
void EditionWindow::_updateRedoAction( void )
{
    Debug::Throw( "EditionWindow::_updateRedoAction.\n" );
    if( keywordEditor_->hasFocus() ) redoAction_->setEnabled( keywordEditor_->isRedoAvailable() );
    else if( titleEditor_->hasFocus() ) redoAction_->setEnabled( titleEditor_->isRedoAvailable() );
    else if( activeEditor().QWidget::hasFocus() ) redoAction_->setEnabled( activeEditor().document()->isRedoAvailable() );
}

//_____________________________________________
void EditionWindow::_updateSaveAction( void )
{ saveAction().setEnabled( !isReadOnly() && modified() ); }

//_____________________________________________
void EditionWindow::_updateUndoRedoActions( QWidget*, QWidget* current )
{
    Debug::Throw( "EditionWindow::_updateUndoRedoAction.\n" );
    if( current == keywordEditor_ )
    {

        undoAction_->setEnabled( keywordEditor_->isUndoAvailable() );
        redoAction_->setEnabled( keywordEditor_->isRedoAvailable() );

    } else if( current == titleEditor_ ) {

        undoAction_->setEnabled( titleEditor_->isUndoAvailable() );
        redoAction_->setEnabled( titleEditor_->isRedoAvailable() );

    } else if( current == &activeEditor() ) {

        undoAction_->setEnabled( activeEditor().document()->isUndoAvailable() );
        redoAction_->setEnabled( activeEditor().document()->isRedoAvailable() );

    }

}

//_____________________________________________
void EditionWindow::_deleteEntry( void )
{

    Debug::Throw( "EditionWindow::_deleteEntry.\n" );

    // check current entry
    LogEntry *entry( EditionWindow::entry() );

    if( !entry ) {
        InformationDialog( this, "No entry selected. <Delete Entry> canceled." ).exec();
        return;
    }

    // ask confirmation
    if( !QuestionDialog( this, "Delete current entry ?" ).exec() ) return;

    // get associated attachments
    _mainWindow().deleteEntry( entry );

    return;

}

//_____________________________________________
void EditionWindow::_spellCheck( void )
{
    #if WITH_ASPELL
    Debug::Throw( "EditionWindow::_spellCheck.\n" );

    // create dialog
    SPELLCHECK::SpellDialog dialog( &activeEditor() );

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
void EditionWindow::_cloneWindow( void )
{

    Debug::Throw( "EditionWindow::_cloneWindow.\n" );
    LogEntry *entry( EditionWindow::entry() );
    if( !entry ) {
        InformationDialog( this, "No valid entry found. <New window> canceled." ).exec();
        return;
    }

    // retrieve selection frame
    MainWindow &mainwindow( _mainWindow() );

    // create new EditionWindow
    EditionWindow *edition_window( new EditionWindow( &mainwindow ) );
    Key::associate( edition_window, &mainwindow );
    edition_window->displayEntry( entry );

    // raise EditionWindow
    edition_window->show();

    return;
}

//_____________________________________________
void EditionWindow::_unlock( void )
{

    Debug::Throw( "EditionWindow::_unlock.\n" );

    if( !isReadOnly() ) return;
    LogEntry *entry( EditionWindow::entry() );

    if( entry && ! _mainWindow().lockEntry( entry ) ) return;
    setReadOnly( false );

    return;

}

//_____________________________________________
void EditionWindow::_textModified( bool state )
{
    Debug::Throw() << "EditionWindow::_textModified - state: " << (state ? "true":"false" ) << endl;

    // check readonly status
    if( isReadOnly() ) return;
    updateWindowTitle();
}

//_____________________________________________
void EditionWindow::_displayFocusChanged( TextEditor* editor )
{

    Debug::Throw() << "EditionWindow::_DisplayFocusChanged - " << editor->key() << endl;
    setActiveEditor( *static_cast<AnimatedTextEditor*>(editor) );

}

//________________________________________________________________
void EditionWindow::_modifiersChanged( unsigned int modifiers )
{
    if( !_hasStatusBar() ) return;
    QStringList buffer;
    if( modifiers & TextEditor::MODIFIER_WRAP ) buffer << "WRAP";
    if( modifiers & TextEditor::MODIFIER_INSERT ) buffer << "INS";
    if( modifiers & TextEditor::MODIFIER_CAPS_LOCK ) buffer << "CAPS";
    if( modifiers & TextEditor::MODIFIER_NUM_LOCK ) buffer << "NUM";
    statusBar().label(1).setText( buffer.join( " " ) );
}

//________________________________________________________________
void EditionWindow::_toggleShowKeyword( bool value )
{

    Debug::Throw( "EditionWindow::_toggleShowKeyword.\n" );
    _setKeywordVisible( value || forceShowKeyword_ );
    XmlOptions::get().set<bool>( "SHOW_KEYWORD", value );

}

//_____________________________________________
void EditionWindow::_updateConfiguration( void )
{

    // one should check whether this is needed or not.
    Debug::Throw( "EditionWindow::_updateConfiguration.\n" );
    resize( sizeHint() );

    // show keyword
    showKeywordAction().setChecked( XmlOptions::get().get<bool>( "SHOW_KEYWORD" ) );

}

//___________________________________________________________________________________
EditionWindow::ColorWidget::ColorWidget( QWidget* parent ):
    QToolButton( parent ),
    //QPushButton( parent ),
    Counter( "ColorWidget" )
{ Debug::Throw( "ColorWidget::ColorWidget.\n" ); }

//___________________________________________________________________________________
void EditionWindow::ColorWidget::setColor( const QColor& color )
{

    // create pixmap
    QPixmap pixmap( IconSize( IconSize::Huge ) );
    pixmap.fill( Qt::transparent );

    QPainter painter( &pixmap );
    painter.setPen( Qt::NoPen );
    painter.setRenderHints( QPainter::Antialiasing|QPainter::SmoothPixmapTransform );

    QRectF rect( pixmap.rect() );
    rect.adjust( 0.5, 0.5, -0.5, -0.5 );

    painter.setBrush( color );
    painter.setPen( Qt::NoPen );
    painter.drawEllipse( rect );
    painter.end();

    setIcon( QIcon( pixmap ) );
}

//___________________________________________________________________________________
QSize EditionWindow::ColorWidget::sizeHint( void ) const
{
    // the const_cast is use to temporarily remove the menu
    // in order to keep the size of the toolbutton minimum
    QMenu* menu( ColorWidget::menu() );
    const_cast<EditionWindow::ColorWidget*>( this )->setMenu(0);
    QSize out( QToolButton::sizeHint() );
    const_cast<EditionWindow::ColorWidget*>( this )->setMenu(menu);
    return out;
}

//___________________________________________________________________________________
QSize EditionWindow::ColorWidget::minimumSizeHint( void ) const
{
    // this is an ugly hack to keep the size of the toolbutton minimum
    QMenu* menu( ColorWidget::menu() );
    const_cast<EditionWindow::ColorWidget*>( this )->setMenu(0);
    QSize out( QToolButton::minimumSizeHint() );
    const_cast<EditionWindow::ColorWidget*>( this )->setMenu(menu);
    return out;
}

//___________________________________________________________________________________
void EditionWindow::ColorWidget::paintEvent( QPaintEvent* )
{
    // rotated paint
    QStylePainter painter(this);
    QStyleOptionToolButton option;
    initStyleOption(&option);

    // first draw normal frame and not text/icon
    option.features &= (~QStyleOptionToolButton::HasMenu);
    painter.drawComplexControl(QStyle::CC_ToolButton, option);

}
