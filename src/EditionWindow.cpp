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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "EditionWindow.h"

#include "Application.h"
#include "AttachmentWindow.h"
#include "AttachmentFrame.h"
#include "BaseContextMenu.h"
#include "BaseIcons.h"
#include "BaseStatusBar.h"
#include "ColorMenu.h"
#include "Command.h"
#include "CustomToolBar.h"
#include "File.h"
#include "FormatBar.h"
#include "HtmlDialog.h"
#include "Icons.h"
#include "IconSize.h"
#include "IconEngine.h"
#include "InformationDialog.h"
#include "InsertLinkDialog.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryHtmlHelper.h"
#include "LogEntryInformationDialog.h"
#include "LogEntryPrintHelper.h"
#include "LogEntryPrintOptionWidget.h"
#include "MainWindow.h"
#include "Menu.h"
#include "Options.h"
#include "PrinterOptionWidget.h"
#include "PrintPreviewDialog.h"
#include "QuestionDialog.h"
#include "RecentFilesMenu.h"
#include "Singleton.h"
// #include "Str.h"
#include "OpenLinkDialog.h"
#include "Util.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellDialog.h"
#endif

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QPrintDialog>
#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QTextLayout>

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
    statusBar_( 0 ),
    insertLinkAction_( 0 )
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
    gridLayout->addWidget( keywordLabel_ = new QLabel( tr( " Keyword:" ), main ), 0, 0, 1, 1 );
    gridLayout->addWidget( keywordEditor_ = new Editor( main ), 0, 1, 1, 2 );
    keywordEditor_->setPlaceholderText( tr( "Entry keyword" ) );
    keywordLabel_->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    keywordLabel_->setBuddy( keywordEditor_ );

    // title label and editor
    gridLayout->addWidget( titleLabel_ = new QLabel( tr( "Subject:" ), main ), 1, 0, 1, 1 );
    gridLayout->addWidget( titleEditor_ = new Editor( main ), 1, 1, 1, 1 );
    titleEditor_->setPlaceholderText( tr( "Entry subject" ) );
    titleLabel_->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    titleLabel_->setBuddy( titleEditor_ );

    // colorWidget
    gridLayout->addWidget( colorWidget_ = new ColorWidget( main ), 1, 2, 1, 1 );
    colorWidget_->setToolTip( tr( "Change entry color.\nThis is used to tag entries in the main window list." ) );
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
    LocalTextEditor& editor( _newTextEditor( main_ ) );
    main_->layout()->addWidget( &editor );

    connect( keywordEditor_, SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );
    connect( keywordEditor_, SIGNAL( cursorPositionChanged( int, int ) ), SLOT( _displayCursorPosition( int, int ) ) );

    connect( titleEditor_, SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );
    connect( titleEditor_, SIGNAL( cursorPositionChanged( int, int ) ), SLOT( _displayCursorPosition( int, int ) ) );

    connect( activeEditor().document(), SIGNAL( modificationChanged( bool ) ), SLOT( _textModified( bool ) ) );

    // create attachment list
    AttachmentFrame *frame = new AttachmentFrame( 0, readOnly_ );
    frame->visibilityAction().setChecked( false );
    frame->setDefaultHeight( XmlOptions::get().get<int>( "ATTACHMENT_FRAME_HEIGHT" ) );
    splitter->addWidget( frame );
    Key::associate( this, frame );

    // status bar for tooltips
    setStatusBar( statusBar_ = new BaseStatusBar( this ) );
    statusBar().addLabel( 2, true );
    statusBar().addLabels( 3, 0 );
    statusBar().addClock();

    // actions
    _installActions();
    Application& application( *Singleton::get().application<Application>() );
    addAction( &application.closeAction() );

    // toolbars
    // lock toolbar is visible only when window is not editable
    lock_ = new CustomToolBar( tr( "Lock" ), this, "LOCK_TOOLBAR" );
    lock_->setMovable( false );

    // hide lock_ visibility action because the latter should not be checkable in any menu
    lock_->visibilityAction().setVisible( false );

    QAction *action;
    lock_->addAction( action = new QAction( IconEngine::get( ICONS::LOCK ), tr( "Unlock" ), this ) );
    connect( action, SIGNAL( triggered() ), SLOT( _unlock() ) );
    action->setToolTip( tr( "Remove read-only lock for current editor." ) );

    // main toolbar
    CustomToolBar* toolbar;
    toolbar = new CustomToolBar( tr( "Main" ), this, "MAIN_TOOLBAR" );

    // new entry
    toolbar->addAction( &newEntryAction() );
    toolbar->addAction( &saveAction() );

    // delete_entry button
    toolbar->addAction( action = new QAction( IconEngine::get( ICONS::DELETE ), tr( "Delete Entry" ), this ) );
    connect( action, SIGNAL( triggered() ), SLOT( _deleteEntry() ) );
    readOnlyActions_ << action;

    // add_attachment button
    toolbar->addAction( &frame->newAction() );

    // format bar
    formatBar_ = new FormatBar( this, "FORMAT_TOOLBAR" );
    formatBar_->setTarget( activeEditor() );
    formatBar_->addAction( &insertLinkAction() );
    readOnlyActions_ << &insertLinkAction();

    const FormatBar::ActionMap& actions( formatBar_->actions() );
    for( FormatBar::ActionMap::const_iterator iter = actions.begin(); iter != actions.end(); ++iter )
    { readOnlyActions_ << iter.value(); }

    // set proper connection for first editor
    // (because it could not be performed in _newTextEditor)
    connect(
        &editor, SIGNAL( currentCharFormatChanged( const QTextCharFormat& ) ),
        formatBar_, SLOT( updateState( const QTextCharFormat& ) ) );

    // edition toolbars
    toolbar = new CustomToolBar( tr( "History" ), this, "EDITION_TOOLBAR" );
    toolbar->addAction( undoAction_ );
    toolbar->addAction( redoAction_ );
    readOnlyActions_ << undoAction_;
    readOnlyActions_ << redoAction_;

    // undo/redo connections
    connect( keywordEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateUndoRedoActions() ) );
    connect( keywordEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateSaveAction() ) );

    connect( titleEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateUndoRedoActions() ) );
    connect( titleEditor_, SIGNAL( textChanged( const QString& ) ), SLOT( _updateSaveAction() ) );

    connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), SLOT( _updateUndoRedoActions( QWidget*, QWidget*) ) );

    // extra toolbar
    toolbar = new CustomToolBar( tr( "Tools" ), this, "EXTRA_TOOLBAR" );

    #if WITH_ASPELL
    toolbar->addAction( &spellcheckAction() );
    #endif

    toolbar->addAction( &printAction() );
    toolbar->addAction( &entryInfoAction() );

    // extra toolbar
    toolbar = new CustomToolBar( tr( "Multiple Views" ), this, "MULTIPLE_VIEW_TOOLBAR" );
    toolbar->addAction( &splitViewHorizontalAction() );
    toolbar->addAction( &splitViewVerticalAction() );
    toolbar->addAction( &cloneWindowAction() );
    toolbar->addAction( &closeAction() );

    // extra toolbar
    toolbar = new CustomToolBar( tr( "Navigation" ), this, "NAVIGATION_TOOLBAR" );
    toolbar->addAction( &application.mainWindow().uniconifyAction() );
    toolbar->addAction( &previousEntryAction() );
    toolbar->addAction( &nextEntryAction() );

    // create menu if requested
    menu_ = new Menu( this, &Singleton::get().application<Application>()->mainWindow() );
    setMenuBar( menu_ );

    // changes display according to readOnly flag
    setReadOnly( readOnly_ );

    // update modifiers
    _modifiersChanged( activeEditor().modifiers() );

    // configuration
    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    _updateConfiguration();

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
    MainWindow &mainWindow( _mainWindow() );
    _menu().recentFilesMenu().setCurrentFile( mainWindow.menu().recentFilesMenu().currentFile() );

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
    previousEntryAction().setEnabled( mainWindow.previousEntry(entry, false) );
    nextEntryAction().setEnabled( mainWindow.nextEntry(entry, false) );

    // reset modify flag; change title accordingly
    setModified( false );

    _updateReadOnlyActions();
    _updateSaveAction();
    updateWindowTitle();

    Debug::Throw( "EditionWindow::displayEntry - done.\n" );
    return;
}

//____________________________________________
void EditionWindow::setReadOnly( bool value )
{
    readOnly_ = value;
    _updateReadOnlyActions();
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
void EditionWindow::setIsClosed( bool value )
{
    closed_ = value;
    if( closed_ )
    {

        // discard modifications, if any
        setModified( false );

        // delete associated entry if it is 'new'
        LogEntry *entry( this->entry() );
        if( entry && BASE::KeySet<Logbook>( entry ).empty() )
        { delete entry; }

    }
}

//_____________________________________________
QString EditionWindow::windowTitle( void ) const
{

    Debug::Throw( "EditionWindow::windowTitle.\n" );
    LogEntry* entry( this->entry() );

    // read only flag
    const bool readOnly( readOnly_ || (_hasMainWindow() && _mainWindow().logbook()->isReadOnly() ) );

    QString buffer;
    if( entry )
    {

        if( readOnly ) buffer = QString( tr( "%1 (read only) - Elogbook" ) ).arg( entry->title() );
        else if( modified()  ) buffer = QString( tr( "%1 (modified) - Elogbook" ) ).arg( entry->title() );
        else buffer = QString( "%1 - Elogbook" ).arg( entry->title() );

    } else buffer = "Elogbook";

    return buffer;

}

//____________________________________________
AskForSaveDialog::ReturnCode EditionWindow::askForSave( bool enableCancel )
{

    Debug::Throw( "EditionWindow::askForSave.\n" );

    // retrieve other editFrames
    BASE::KeySet<EditionWindow> editionwindows( &_mainWindow() );
    unsigned int count( std::count_if( editionwindows.begin(), editionwindows.end(), ModifiedFTor() ) );

    // create dialog
    unsigned int buttons = AskForSaveDialog::YES | AskForSaveDialog::NO;
    if( enableCancel ) buttons |= AskForSaveDialog::CANCEL;
    if( count > 1 ) buttons |= AskForSaveDialog::ALL;
    AskForSaveDialog dialog( this, tr( "Entry has been modified. Save ?" ), buttons );

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
            foreach( EditionWindow* window, editionwindows )
            { if( window->modified() && !window->isReadOnly() ) window->_save(enableCancel); }
        } else _mainWindow().save( false );
    }

    return AskForSaveDialog::ReturnCode(state);

}

//_____________________________________________
void EditionWindow::displayKeyword( void )
{
    Debug::Throw( "EditionWindow::displayKeyword.\n" );

    LogEntry* entry( EditionWindow::entry() );
    if( entry ) keywordEditor_->setText( entry->keyword().get() );
    keywordEditor_->setCursorPosition( 0 );
    return;
}

//_____________________________________________
void EditionWindow::displayTitle( void )
{
    Debug::Throw( "EditionWindow::displayTitle.\n" );

    LogEntry* entry( this->entry() );
    if( entry ) titleEditor_->setText( entry->title() );
    titleEditor_->setCursorPosition( 0 );
    return;
}

//_____________________________________________
void EditionWindow::displayColor( void )
{
    Debug::Throw( "EditionWindow::DisplayColor.\n" );

    // try load entry color
    const QColor color( entry()->color() );
    if( !color.isValid() )
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
void EditionWindow::closeEditor( LocalTextEditor& editor )
{
    Debug::Throw( "EditionWindow::closeEditor.\n" );

    // retrieve number of editors
    // if only one display, close the entire window
    BASE::KeySet<LocalTextEditor> editors( this );
    if( editors.size() < 2 )
    {
        Debug::Throw() << "EditionWindow::closeEditor - full close." << endl;
        close();
        return;
    }

    // retrieve parent and grandparent of current display
    QWidget* parent( editor.parentWidget() );
    QSplitter* parentSplitter( qobject_cast<QSplitter*>( parent ) );

    // retrieve editors associated to current
    editors = BASE::KeySet<LocalTextEditor>( &editor );

    // check how many children remain in parentSplitter if any
    // take action if it is less than 2 (the current one to be deleted, and another one)
    if( parentSplitter && parentSplitter->count() <= 2 )
    {

        // retrieve child that is not the current editor
        // need to loop over existing widgets because the editor above has not been deleted yet
        QWidget* child(0);
        for( int index = 0; index < parentSplitter->count(); index++ )
        {
            if( parentSplitter->widget( index ) != &editor )
            {
                child = parentSplitter->widget( index );
                break;
            }
        }
        Q_CHECK_PTR( child );
        Debug::Throw( "EditionWindow::closeEditor - found child.\n" );

        // retrieve splitter parent
        QWidget* grandParent( parentSplitter->parentWidget() );

        // try cast to a splitter
        QSplitter* grandParentSplitter( qobject_cast<QSplitter*>( grandParent ) );

        // move child to grandParentSplitter if any
        if( grandParentSplitter )
        {

            grandParentSplitter->insertWidget( grandParentSplitter->indexOf( parentSplitter ), child );

        }  else {

            child->setParent( grandParent );
            grandParent->layout()->addWidget( child );

        }

        // delete parentSplitter, now that it is empty
        parentSplitter->deleteLater();
        Debug::Throw( "EditionWindow::closeEditor - deleted splitter.\n" );

    } else {

        // the editor is deleted only if its parent splitter is not
        // otherwise this will trigger double deletion of the editor
        // which will then crash
        editor.deleteLater();

    }

    // update activeEditor
    bool activeFound( false );
    BASE::KeySetIterator<LocalTextEditor> iterator( editors );
    iterator.toBack();
    while( iterator.hasPrevious() )
    {
        LocalTextEditor* current( iterator.previous() );
        if( current != &editor )
        {
            setActiveEditor( *current );
            activeFound = true;
            break;
        }
    }
    Q_ASSERT( activeFound );

    // change focus
    activeEditor().setFocus();
    Debug::Throw( "EditionWindow::closeEditor - done.\n" );

}

//________________________________________________________________
void EditionWindow::setActiveEditor( LocalTextEditor& editor )
{
    Debug::Throw() << "EditionWindow::setActiveEditor - key: " << editor.key() << endl;
    Q_ASSERT( editor.isAssociated( this ) );

    activeEditor_ = &editor;
    if( !activeEditor().isActive() )
    {

        foreach( LocalTextEditor* editor, BASE::KeySet<LocalTextEditor>( this ) )
        { editor->setActive( false ); }

        activeEditor().setActive( true );

    }

    // associate with toolbar
    if( formatBar_ ) formatBar_->setTarget( activeEditor() );

}

//_____________________________________________
void EditionWindow::updateReadOnlyState( void )
{

    Debug::Throw( "EditionWindow::updateReadOnlyState\n" );
    _updateReadOnlyActions();
    _updateSaveAction();
    _updateUndoRedoActions();
    _updateInsertLinkActions();
    updateWindowTitle();

}

//____________________________________________
void EditionWindow::closeEvent( QCloseEvent *event )
{
    Debug::Throw( "EditionWindow::closeEvent.\n" );

    // ask for save if entry is modified
    if( !(readOnly_ || closed_ ) && modified() && askForSave() == AskForSaveDialog::CANCEL ) event->ignore();
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
    addAction( undoAction_ = new QAction( IconEngine::get( ICONS::UNDO ), tr( "Undo" ), this ) );
    undoAction_->setToolTip( tr( "Undo last modification" ) );
    connect( undoAction_, SIGNAL( triggered() ), SLOT( _undo() ) );

    // redo action
    addAction( redoAction_ = new QAction( IconEngine::get( ICONS::REDO ), tr( "Redo" ), this ) );
    redoAction_->setToolTip( tr( "Redo last undone modification" ) );
    connect( redoAction_, SIGNAL( triggered() ), SLOT( _redo() ) );

    // new entry
    addAction( newEntryAction_ = new QAction( IconEngine::get( ICONS::NEW ), tr( "New Entry" ), this ) );
    newEntryAction_->setShortcut( QKeySequence::New );
    newEntryAction_->setToolTip( tr( "Create new entry in current editor" ) );
    connect( newEntryAction_, SIGNAL( triggered() ), SLOT( _newEntry() ) );

    // previous entry action
    addAction( previousEntryAction_ = new QAction( IconEngine::get( ICONS::PREVIOUS ), tr( "Previous Entry" ), this ) );
    previousEntryAction_->setToolTip( tr( "Display previous entry in current list" ) );
    connect( previousEntryAction_, SIGNAL( triggered() ), SLOT( _previousEntry() ) );

    // next entry action
    addAction( nextEntryAction_ = new QAction( IconEngine::get( ICONS::NEXT ), tr( "Next Entry" ), this ) );
    nextEntryAction_->setToolTip( tr( "Display next entry in current list" ) );
    connect( nextEntryAction_, SIGNAL( triggered() ), SLOT( _nextEntry() ) );

    // save
    addAction( saveAction_ = new QAction( IconEngine::get( ICONS::SAVE ), tr( "Save Entry" ), this ) );
    saveAction_->setToolTip( tr( "Save current entry" ) );
    connect( saveAction_, SIGNAL( triggered() ), SLOT( _save() ) );
    saveAction_->setShortcut( QKeySequence::Save );

    #if WITH_ASPELL
    addAction( spellcheckAction_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), tr( "Spellcheck..." ), this ) );
    spellcheckAction_->setToolTip( tr( "Check spelling of current entry" ) );
    connect( spellcheckAction_, SIGNAL( triggered() ), SLOT( _spellCheck() ) );

    // disable action if there is no dictionary
    spellcheckAction_->setEnabled( !SPELLCHECK::SpellInterface().dictionaries().empty() );
    #endif

    // entry_info
    addAction( entryInfoAction_ = new QAction( IconEngine::get( ICONS::INFORMATION ), tr( "Entry Properties..." ), this ) );
    entryInfoAction_->setToolTip( tr( "Show current entry properties" ) );
    connect( entryInfoAction_, SIGNAL( triggered() ), SLOT( _entryInfo() ) );

    // print
    addAction( printAction_ = new QAction( IconEngine::get( ICONS::PRINT ), tr( "Print..." ), this ) );
    printAction_->setToolTip( tr( "Print current logbook entry" ) );
    printAction_->setShortcut( QKeySequence::Print );
    connect( printAction_, SIGNAL( triggered() ), SLOT( _print() ) );

    // print preview
    addAction( printPreviewAction_ = new QAction( IconEngine::get( ICONS::PRINT_PREVIEW ), tr( "Print Preview..." ), this ) );
    connect( printPreviewAction_, SIGNAL( triggered() ), SLOT( _printPreview() ) );

    // print
    addAction( htmlAction_ = new QAction( IconEngine::get( ICONS::HTML ), tr( "Export to HTML..." ), this ) );
    htmlAction_->setToolTip( tr( "Export current logbook entry to HTML" ) );
    connect( htmlAction_, SIGNAL( triggered() ), SLOT( _toHtml() ) );

    // split action
    addAction( splitViewHorizontalAction_ =new QAction( IconEngine::get( ICONS::VIEW_TOPBOTTOM ), tr( "Split View Top/Bottom" ), this ) );
    splitViewHorizontalAction_->setToolTip( tr( "Split current text editor vertically" ) );
    connect( splitViewHorizontalAction_, SIGNAL( triggered() ), SLOT( _splitViewVertical() ) );

    addAction( splitViewVerticalAction_ =new QAction( IconEngine::get( ICONS::VIEW_LEFTRIGHT ), tr( "Split View Left/Right" ), this ) );
    splitViewVerticalAction_->setToolTip( tr( "Split current text editor horizontally" ) );
    connect( splitViewVerticalAction_, SIGNAL( triggered() ), SLOT( _splitViewHorizontal() ) );

    // clone window action
    addAction( cloneWindowAction_ = new QAction( IconEngine::get( ICONS::VIEW_CLONE ), tr( "Clone Window" ), this ) );
    cloneWindowAction_->setToolTip( tr( "Create a new edition window displaying the same entry" ) );
    connect( cloneWindowAction_, SIGNAL( triggered() ), SLOT( _cloneWindow() ) );

    // close window action
    addAction( closeAction_ = new QAction( IconEngine::get( ICONS::VIEW_REMOVE ), tr( "Close View" ), this ) );
    closeAction_->setShortcut( QKeySequence::Close );
    closeAction_->setToolTip( tr( "Close current view" ) );
    connect( closeAction_, SIGNAL( triggered() ), SLOT( _close() ) );

    // uniconify
    addAction( uniconifyAction_ = new QAction( tr( "Uniconify" ), this ) );
    connect( uniconifyAction_, SIGNAL( triggered() ), SLOT( uniconify() ) );

    // show/hide keyword
    addAction( showKeywordAction_ = new QAction( tr( "Show Keyword" ), this ) );
    showKeywordAction_->setCheckable( true );
    showKeywordAction_->setChecked( false );
    connect( showKeywordAction_, SIGNAL( toggled( bool ) ), SLOT( _toggleShowKeyword( bool ) ) );

    // insert link
    addAction( insertLinkAction_ = new QAction( IconEngine::get( ICONS::INSERT_LINK ), tr( "Insert Link" ), this ) );
    connect( insertLinkAction_, SIGNAL( triggered( void ) ), SLOT( _insertLink( void ) ) );
    insertLinkAction_->setEnabled( false );
}

//___________________________________________________________
EditionWindow::LocalTextEditor& EditionWindow::_splitView( const Qt::Orientation& orientation )
{
    Debug::Throw( "EditionWindow::_splitView.\n" );

    // keep local pointer to current active display
    LocalTextEditor& activeEditorLocal( activeEditor() );

    // compute desired dimension of the new splitter
    // along its splitting direction
    int dimension( (orientation == Qt::Horizontal) ? activeEditorLocal.width():activeEditorLocal.height() );

    // create new splitter
    QSplitter& splitter( _newSplitter( orientation ) );

    // create new display
    LocalTextEditor& editor( _newTextEditor(0) );

    // insert in splitter, at correct position
    splitter.insertWidget( splitter.indexOf( &activeEditorLocal )+1, &editor );

    // recompute dimension
    // take the max of active display and splitter,
    // in case no new splitter was created.
    dimension = qMax( dimension, (orientation == Qt::Horizontal) ? splitter.width():splitter.height() );

    // assign equal size to all splitter children
    QList<int> sizes;
    for( int i=0; i<splitter.count(); i++ )
    { sizes << dimension/splitter.count(); }
    splitter.setSizes( sizes );

    // synchronize both editors, if cloned
    /*
    if there exists no clone of active display,
    backup text and register a new Sync object
    */
    BASE::KeySet<LocalTextEditor> editors( &activeEditorLocal );

    // clone new display
    editor.synchronize( &activeEditorLocal );

    // perform associations
    // check if active editors has associates and propagate to new
    foreach( LocalTextEditor* iter, editors )
    { BASE::Key::associate( &editor, iter ); }

    // associate new display to active
    BASE::Key::associate( &editor, &activeEditorLocal );

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
    QSplitter *parentSplitter( qobject_cast<QSplitter*>( parent ) );
    if( parentSplitter && parentSplitter->orientation() == orientation ) {

        Debug::Throw( "EditionWindow::_newSplitter - orientation match. No need to create new splitter.\n" );
        splitter = parentSplitter;

    } else {


        // move splitter to the first place if needed
        if( parentSplitter )
        {

            Debug::Throw( "EditionWindow::_newSplitter - found parent splitter with incorrect orientation.\n" );
            // create a splitter with correct orientation
            // give him no parent, because the parent is set in QSplitter::insertWidget()
            splitter = new LocalSplitter(0);
            splitter->setOrientation( orientation );
            parentSplitter->insertWidget( parentSplitter->indexOf( &activeEditor() ), splitter );

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
        if( parentSplitter )
        {
            int dimension = ( parentSplitter->orientation() == Qt::Horizontal) ?
                parentSplitter->width():
                parentSplitter->height();

            QList<int> sizes;
            for( int i=0; i<parentSplitter->count(); i++ )
            { sizes << dimension/parentSplitter->count(); }
            parentSplitter->setSizes( sizes );

        }

    }

    // return created splitter
    return *splitter;

}

//_____________________________________________________________
EditionWindow::LocalTextEditor& EditionWindow::_newTextEditor( QWidget* parent )
{
    Debug::Throw( "EditionWindow::_newTextEditor.\n" );

    // create textDisplay
    LocalTextEditor* editor = new LocalTextEditor( parent );

    // connections
    connect( &editor->insertLinkAction(), SIGNAL( triggered( void ) ), SLOT( _insertLink( void ) ) );
    connect( &editor->openLinkAction(), SIGNAL( triggered( void ) ), SLOT( _openLink( void ) ) );
    connect( editor, SIGNAL( hasFocus( TextEditor* ) ), SLOT( _displayFocusChanged( TextEditor* ) ) );
    connect( editor, SIGNAL( cursorPositionChanged( void ) ), SLOT( _displayCursorPosition( void ) ) );
    connect( editor, SIGNAL( modifiersChanged( TextEditor::Modifiers ) ), SLOT( _modifiersChanged( TextEditor::Modifiers ) ) );
    connect( editor, SIGNAL( undoAvailable( bool ) ), SLOT( _updateUndoRedoActions( void ) ) );
    connect( editor, SIGNAL( selectionChanged( void ) ), SLOT( _updateInsertLinkActions( void ) ) );
    connect( editor->document(), SIGNAL( modificationChanged( bool ) ), SLOT( _updateSaveAction( void ) ) );

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

    // update insert Link actions
    _updateInsertLinkActions();

    return *editor;

}

//_____________________________________________
void EditionWindow::_displayCursorPosition( const TextPosition& position)
{
    Debug::Throw( "EditionWindow::_DisplayCursorPosition.\n" );
    if( !_hasStatusBar() ) return;

    statusBar().label(2).setText( QString( tr( "Line: %1" ) ).arg( position.paragraph()+1 ), false );
    statusBar().label(3).setText( QString( tr( "Column: %1" ) ).arg( position.index()+1 ), true );

    return;
}

//_______________________________________________
bool EditionWindow::_hasMainWindow( void ) const
{ return BASE::KeySet<MainWindow>( this ).size() > 0; }

//_______________________________________________
MainWindow& EditionWindow::_mainWindow( void ) const
{
    Debug::Throw( "EditionWindow::_mainWindow.\n" );
    BASE::KeySet<MainWindow> mainWindows( this );
    Q_ASSERT( mainWindows.size()==1 );
    return **mainWindows.begin();
}

//_____________________________________________
void EditionWindow::_displayText( void )
{
    Debug::Throw( "EditionWindow::_displayText.\n" );
    if( !&activeEditor() ) return;

    LogEntry* entry( EditionWindow::entry() );
    activeEditor().setCurrentCharFormat( QTextCharFormat() );
    activeEditor().setPlainText( (entry) ? entry->text() : QString() );
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
        frame.add( attachments.toList() );

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
    if( readOnly_ ) return;

    // retrieve associated entry
    LogEntry *entry( this->entry() );

    // see if entry is new
    const bool entryIsNew( !entry || BASE::KeySet<Logbook>( entry ).empty() );

    // create entry if none set
    if( !entry ) entry = new LogEntry();

    // check logbook
    MainWindow &mainWindow( _mainWindow() );
    Logbook *logbook( mainWindow.logbook() );
    if( !logbook ) {
        InformationDialog( this, tr( "No logbook opened. <Save> canceled." ) ).exec();
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
    statusBar().label().setText( tr( "writting entry to logbook..." ) );

    // add entry to logbook, if needed
    if( entryIsNew ) Key::associate( entry, logbook->latestChild() );

    // update this window title, set unmodified.
    setModified( false );
    updateWindowTitle();

    // update associated EditionWindows
    BASE::KeySet<EditionWindow> windows( entry );
    foreach( EditionWindow* window, windows )
    {
        Q_ASSERT( window == this || window->isReadOnly() || window->isClosed() );
        if( window != this ) window->displayEntry( entry );
    }

    // update main window
    mainWindow.updateEntry( entry, updateSelection );
    mainWindow.updateWindowTitle();

    // set logbook as modified
    BASE::KeySet<Logbook> logbooks( entry );
    foreach( Logbook* logbook, logbooks )
    { logbook->setModified( true ); }

    // add to main logbook recent entries
    mainWindow.logbook()->addRecentEntry( entry );

    // Save logbook
    if( !mainWindow.logbook()->file().isEmpty() ) mainWindow.save();

    statusBar().label().setText( "" );

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

    // generate document name
    QString buffer;
    QTextStream( &buffer )  << "elogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid();
    printer.setDocName( buffer );

    // create helper
    LogEntryPrintHelper helper( this );
    helper.setEntry( entry() );

    // create option widget
    PrinterOptionWidget* optionWidget( new PrinterOptionWidget() );
    optionWidget->setHelper( &helper );

    connect( optionWidget, SIGNAL( orientationChanged( QPrinter::Orientation ) ), &helper, SLOT( setOrientation( QPrinter::Orientation ) ) );
    connect( optionWidget, SIGNAL( pageModeChanged( BasePrintHelper::PageMode ) ), &helper, SLOT( setPageMode( BasePrintHelper::PageMode ) ) );

    LogEntryPrintOptionWidget* logEntryOptionWidget = new LogEntryPrintOptionWidget();
    connect( logEntryOptionWidget, SIGNAL( maskChanged( unsigned int ) ), &helper, SLOT( setMask( unsigned int ) ) );
    logEntryOptionWidget->read();

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setOptionTabs( QList<QWidget *>() << optionWidget << logEntryOptionWidget );
    dialog.setWindowTitle( tr( "Print Logbook Entry - Elogbook" ) );
    if( !dialog.exec() ) return;

    // add output file to scratch files, if any
    if( !printer.outputFileName().isEmpty() )
    { emit scratchFileCreated( printer.outputFileName() ); }

    // write options
    logEntryOptionWidget->write();

    // print
    helper.print( &printer );

    return;

}

//___________________________________________________________
void EditionWindow::_printPreview( void )
{
    Debug::Throw( "EditionWindow::_printPreview.\n" );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create helper
    LogEntryPrintHelper helper( this );
    helper.setEntry( entry() );
    helper.setMask( (LogEntry::Mask) XmlOptions::get().get<int>( "LOGENTRY_PRINT_OPTION_MASK" ) );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this );
    dialog.setWindowTitle( tr( "Print Preview - Elogbook" ) );
    dialog.setHelper( &helper );
    dialog.exec();

}

//___________________________________________________________
void EditionWindow::_toHtml( void )
{
    Debug::Throw( "EditionWindow::_toHtml.\n" );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::CANCEL ) return;

    // create option widget
    LogEntryPrintOptionWidget* optionWidget = new LogEntryPrintOptionWidget();
    optionWidget->read();

    // create dialog
    HtmlDialog dialog( this );
    dialog.setOptionWidgets( QList<QWidget *>() << optionWidget );
    dialog.setWindowTitle( tr( "Export to HTML - Elogbook" ) );

    // generate file name
    QString buffer;
    QTextStream( &buffer )  << "eLogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid() << ".html";
    dialog.setFile( File( buffer ).addPath( Util::tmp() ) );

    // execute dialog
    if( !dialog.exec() ) return;

    // retrieve/check file
    File file( dialog.file() );
    if( file.isEmpty() ) {
        InformationDialog(this, tr( "No output file specified. <View HTML> canceled." ) ).exec();
        return;
    }

    QFile out( file );
    if( !out.open( QIODevice::WriteOnly ) )
    {
        InformationDialog( this, QString( tr( "Cannot write to file '%1'. <View HTML> canceled." ) ).arg( file ) ).exec();
        return;
    }

    // add as scratch file
    emit scratchFileCreated( file );

    // write options
    optionWidget->write();

    LogEntryHtmlHelper helper;
    helper.setEntry( entry() );
    helper.setMask( optionWidget->mask() );

    helper.print( &out );
    out.close();

    // get command and execute
    QString command( dialog.command() );
    if( !command.isEmpty() )
    { ( Command( command ) << file ).run(); }

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

    MainWindow &mainWindow( _mainWindow() );
    LogEntry* entry( mainWindow.previousEntry( EditionWindow::entry(), true ) );
    if( !( entry  && mainWindow.lockEntry( entry ) ) ) return;
    displayEntry( entry );
    setReadOnly( false );

}

//_______________________________________________
void EditionWindow::_nextEntry( void )
{
    Debug::Throw( "EditionWindow::_nextEntry.\n" );

    MainWindow &mainWindow( _mainWindow() );
    LogEntry* entry( mainWindow.nextEntry( EditionWindow::entry(), true ) );
    if( !( entry && mainWindow.lockEntry( entry ) ) ) return;
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
        InformationDialog( this, tr( "No valid entry." ) ).exec();
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
void EditionWindow::_insertLink( void )
{
    Debug::Throw( "EditionWindow::_insertLink.\n" );

    // check readonly and selection
    const QTextCursor cursor( activeEditor().textCursor() );
    if( readOnly_ || !cursor.hasSelection() ) return;

    const QTextCharFormat format( cursor.charFormat() );
    const QString selection( format.anchorHref().isEmpty() ? cursor.selectedText() : format.anchorHref() );

    // create dialog
    InsertLinkDialog dialog( this, selection );
    if( !dialog.exec() ) return;

    // update format
    QTextCharFormat outputFormat;
    outputFormat.setFontUnderline( true );
    outputFormat.setForeground( activeEditor().palette().color( QPalette::Link ) );
    outputFormat.setAnchorHref( dialog.link() );
    outputFormat.setAnchor( true );
    activeEditor().mergeCurrentCharFormat( outputFormat );

}

//_____________________________________________
void EditionWindow::_openLink( void )
{
    Debug::Throw( "EditionWindow::_openLink.\n" );
    QString anchor( activeEditor().anchor() );
    if( anchor.isEmpty() ) return;

    OpenLinkDialog dialog( this, anchor );
    dialog.setWindowTitle( tr( "Open Link - Elogbook" ) );

    // retrieve applications from options
    Options::List applications( XmlOptions::get().specialOptions( "OPEN_LINK_APPLICATIONS" ) );
    foreach( const Option& option, applications )
    { dialog.actionComboBox().addItem( option.raw() ); }

    if( dialog.centerOnParent().exec() == QDialog::Rejected ) return;

    // retrieve application from combobox and add as options
    QString command( dialog.actionComboBox().currentText() );
    if( command.isEmpty() )
    {
        InformationDialog( this, tr( "No command specified to open the selected files. <Open> canceled." ) ).exec();
        return;
    }

    // update options
    XmlOptions::get().add( "OPEN_LINK_APPLICATIONS", Option( command, Option::Recordable|Option::Current ) );

    // execute
    ( Command( command ) << anchor ).run();

}

//_____________________________________________
void EditionWindow::_deleteEntry( void )
{

    Debug::Throw( "EditionWindow::_deleteEntry.\n" );

    // check current entry
    LogEntry *entry( EditionWindow::entry() );

    if( !entry )
    {
        InformationDialog( this, tr( "No entry selected. <Delete Entry> canceled." ) ).exec();
        return;
    }

    // ask confirmation
    if( !QuestionDialog( this, tr( "Delete current entry ?" ) ).exec() ) return;

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
        InformationDialog( this, tr( "No valid entry found. <New window> canceled." ) ).exec();
        return;
    }

    // retrieve selection frame
    MainWindow &mainWindow( _mainWindow() );

    // create new EditionWindow
    EditionWindow *edition_window( new EditionWindow( &mainWindow ) );
    Key::associate( edition_window, &mainWindow );
    edition_window->displayEntry( entry );

    // raise EditionWindow
    edition_window->show();

    return;
}

//_____________________________________________
void EditionWindow::_unlock( void )
{

    Debug::Throw( "EditionWindow::_unlock.\n" );

    if( !readOnly_ ) return;
    LogEntry *entry( EditionWindow::entry() );

    if( entry && ! _mainWindow().lockEntry( entry ) ) return;
    setReadOnly( false );

    return;

}


//_____________________________________________
void EditionWindow::_updateReadOnlyActions( void )
{

    Debug::Throw( "EditionWindow::_updateReadOnlyActions.\n" );

    // add flag from logbook read only
    const bool logbookReadOnly( _hasMainWindow() && _mainWindow().logbook()->isReadOnly() );
    const bool readOnly( readOnly_ || logbookReadOnly );

    // changes button state
    foreach( QAction* action, readOnlyActions_ )
    { action->setEnabled( !readOnly ); }

    // changes lock button state
    if( readOnly_ && lock_->isHidden() )
    {

        Qt::ToolBarArea currentLocation = toolBarArea( lock_ );
        if( currentLocation == Qt::NoToolBarArea ) { addToolBar( Qt::LeftToolBarArea, lock_ ); }
        lock_->show();

    } else if( !(readOnly_ || lock_->isHidden() ) ) { lock_->hide(); }

    // changes TextEdit readOnly status
    keywordEditor_->setReadOnly( readOnly );
    titleEditor_->setReadOnly( readOnly );

    // update editors
    foreach( LocalTextEditor* editor, BASE::KeySet<LocalTextEditor>( this ) )
    { editor->setReadOnly( readOnly ); }

    // changes attachment list status
    attachmentFrame().setReadOnly( readOnly );

    // new entry
    newEntryAction_->setEnabled( !logbookReadOnly );

    #if WITH_ASPELL
    spellcheckAction_->setEnabled( !( readOnly || SPELLCHECK::SpellInterface().dictionaries().empty() ) );
    #endif

}

//_____________________________________________
void EditionWindow::_updateSaveAction( void )
{ saveAction().setEnabled( !readOnly_ && !( _hasMainWindow() && _mainWindow().logbook()->isReadOnly() ) && modified() ); }

//_____________________________________________
void EditionWindow::_updateUndoRedoActions( void )
{

    Debug::Throw( "EditionWindow::_updateRedoAction.\n" );
    if( keywordEditor_->hasFocus() )
    {
        undoAction_->setEnabled( keywordEditor_->isUndoAvailable() && !keywordEditor_->isReadOnly() );
        redoAction_->setEnabled( keywordEditor_->isRedoAvailable() && !keywordEditor_->isReadOnly() );

    } else if( titleEditor_->hasFocus() ) {

        undoAction_->setEnabled( titleEditor_->isUndoAvailable() && !titleEditor_->isReadOnly() );
        redoAction_->setEnabled( titleEditor_->isRedoAvailable() && !titleEditor_->isReadOnly() );

    } else if( activeEditor().QWidget::hasFocus() ) {

        undoAction_->setEnabled( activeEditor().document()->isUndoAvailable() && !activeEditor().isReadOnly() );
        redoAction_->setEnabled( activeEditor().document()->isRedoAvailable() && !activeEditor().isReadOnly() );

    }
}

//_____________________________________________
void EditionWindow::_updateUndoRedoActions( QWidget*, QWidget* current )
{
    Debug::Throw( "EditionWindow::_updateUndoRedoAction.\n" );
    if( current == keywordEditor_ )
    {

        undoAction_->setEnabled( keywordEditor_->isUndoAvailable() && !keywordEditor_->isReadOnly() );
        redoAction_->setEnabled( keywordEditor_->isRedoAvailable() && !keywordEditor_->isReadOnly() );

    } else if( current == titleEditor_ ) {

        undoAction_->setEnabled( titleEditor_->isUndoAvailable() && !titleEditor_->isReadOnly() );
        redoAction_->setEnabled( titleEditor_->isRedoAvailable() && !titleEditor_->isReadOnly() );

    } else if( current == &activeEditor() ) {

        undoAction_->setEnabled( activeEditor().document()->isUndoAvailable() && !activeEditor().isReadOnly() );
        redoAction_->setEnabled( activeEditor().document()->isRedoAvailable() && !activeEditor().isReadOnly() );

    }

}

//_____________________________________________
void EditionWindow::_updateInsertLinkActions( void )
{
    Debug::Throw( "EditionWindow::_updateInsertLinkActions.\n" );
    const bool enabled( !readOnly_ && !( _hasMainWindow() && _mainWindow().logbook()->isReadOnly() ) && activeEditor().textCursor().hasSelection() );

    // disable main window action
    if( hasInsertLinkAction() ) insertLinkAction().setEnabled( enabled );

    // also disable editors action
    BASE::KeySet<LocalTextEditor> editors( this );
    foreach( LocalTextEditor* editor, editors )
    { editor->insertLinkAction().setEnabled( enabled ); }

}

//_____________________________________________
void EditionWindow::_textModified( bool state )
{
    Debug::Throw() << "EditionWindow::_textModified - state: " << (state ? "true":"false" ) << endl;

    // check readonly status
    if( readOnly_ ) return;
    updateWindowTitle();
}

//_____________________________________________
void EditionWindow::_displayFocusChanged( TextEditor* editor )
{

    Debug::Throw() << "EditionWindow::_DisplayFocusChanged - " << editor->key() << endl;
    setActiveEditor( *static_cast<LocalTextEditor*>(editor) );

}

//________________________________________________________________
void EditionWindow::_modifiersChanged( TextEditor::Modifiers modifiers )
{
    if( !_hasStatusBar() ) return;
    QStringList buffer;
    if( modifiers & TextEditor::ModifierWrap ) buffer << "WRAP";
    if( modifiers & TextEditor::ModifierInsert ) buffer << "INS";
    if( modifiers & TextEditor::ModifierCapsLock ) buffer << "CAPS";
    if( modifiers & TextEditor::ModifierNumLock ) buffer << "NUM";
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
void EditionWindow::LocalTextEditor::_installActions( void )
{
    Debug::Throw( "EditionWindow::LocalTextEditor::_installActions.\n" );
    addAction( insertLinkAction_ = new QAction( IconEngine::get( ICONS::INSERT_LINK ), tr( "Insert Link..." ), this ) );
    addAction( openLinkAction_ = new QAction( IconEngine::get( ICONS::FIND ), tr( "View Link..." ), this ) );

    // disable insert link action by default
    insertLinkAction_->setEnabled( false );

}

//___________________________________________________________________________________
void EditionWindow::LocalTextEditor::installContextMenuActions( BaseContextMenu* menu, const bool& allActions )
{
    Debug::Throw( "EditionWindow::LocalTextEditor::installContextMenuActions.\n" );
    AnimatedTextEditor::installContextMenuActions( menu, allActions );

    // insert link
    menu->insertAction( &showLineNumberAction(), &insertLinkAction() );

    // open link
    if( !anchorAt( _contextMenuPosition() ).isEmpty() )
    { menu->insertAction( &showLineNumberAction(), &openLinkAction() ); }

    // separator
    menu->insertSeparator( &showLineNumberAction() );

}

//___________________________________________________________________________________
EditionWindow::ColorWidget::ColorWidget( QWidget* parent ):
    QToolButton( parent ),
    Counter( "ColorWidget" )
{ Debug::Throw( "ColorWidget::ColorWidget.\n" ); }

//___________________________________________________________________________________
void EditionWindow::ColorWidget::setColor( const QColor& color )
{

    // create pixmap
    QPixmap pixmap = QPixmap( IconSize( IconSize::Huge ) );
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
