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
#include "EditionWindow_p.h"

#include "Application.h"
#include "AttachmentWindow.h"
#include "AttachmentFrame.h"
#include "BaseContextMenu.h"
#include "BaseFindWidget.h"
#include "BaseIconNames.h"
#include "BaseReplaceWidget.h"
#include "BaseStatusBar.h"
#include "ColorMenu.h"
#include "Command.h"
#include "CppUtil.h"
#include "CustomToolBar.h"
#include "File.h"
#include "FormatBar.h"
#include "HtmlDialog.h"
#include "IconNames.h"
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
#include "MenuBar.h"
#include "Options.h"
#include "PrinterOptionWidget.h"
#include "PrintPreviewDialog.h"
#include "QuestionDialog.h"
#include "RecentFilesMenu.h"
#include "SelectLineWidget.h"
#include "Singleton.h"
#include "OpenWithDialog.h"
#include "Util.h"

#if USE_ASPELL
#include "SpellDialog.h"
#endif

#include <QApplication>
#include <QDesktopServices>
#include <QLabel>
#include <QLayout>
#include <QPrintDialog>
#include <QStylePainter>
#include <QStyleOptionToolButton>
#include <QTextFragment>
#include <QTextLayout>
#include <QUrl>

//_______________________________________________
EditionWindow::EditionWindow( QWidget* parent, bool readOnly ):
    BaseMainWindow( parent ),
    Counter( "EditionWindow" ),
    readOnly_( readOnly )
{
    Debug::Throw("EditionWindow::EditionWindow.\n" );
    setOptionName( "editionWindow" );
    setObjectName( "EDITFRAME" );

    auto main( new QWidget( this ) );
    setCentralWidget( main );

    auto layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(2);
    main->setLayout( layout );

    // header layout
    auto gridLayout( new QGridLayout );
    gridLayout->setSpacing(0);
    gridLayout->setMargin(0);
    layout->addLayout( gridLayout );

    // title label and editor
    gridLayout->addWidget( titleLabel_ = new QLabel( tr( "Subject:" ), main ), 0, 0, 1, 1 );
    gridLayout->addWidget( titleEditor_ = new Editor( main ), 0, 1, 1, 1 );
    titleEditor_->setPlaceholderText( tr( "Entry subject" ) );
    titleLabel_->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    titleLabel_->setBuddy( titleEditor_ );

    // colorWidget
    gridLayout->addWidget( colorWidget_ = new Private::ColorWidget( main ), 0, 2, 1, 1 );
    colorWidget_->setToolTip( tr( "Change entry color.\nThis is used to tag entries in the main window list" ) );
    colorWidget_->setAutoRaise( true );
    colorWidget_->setPopupMode( QToolButton::InstantPopup );

    // keywoard label and editor
    gridLayout->addWidget( keywordLabel_ = new QLabel( tr( " Keyword:" ), main ), 1, 0, 1, 1 );
    gridLayout->addWidget( keywordEditor_ = new Editor( main ), 1, 1, 1, 2 );
    keywordEditor_->setPlaceholderText( tr( "Entry keyword" ) );
    keywordLabel_->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    keywordLabel_->setBuddy( keywordEditor_ );

    gridLayout->setColumnStretch( 1, 1 );

    // hide everything
    _setKeywordVisible( false );
    colorWidget_->hide();

    // splitter for EditionWindow and attachment list
    auto splitter = new QSplitter( main );
    splitter->setOrientation( Qt::Vertical );
    layout->addWidget( splitter, 1 );

    // create text
    auto splitterWidget = new QWidget;
    splitterWidget->setLayout( new QVBoxLayout );
    splitterWidget->layout()->setMargin(0);
    splitterWidget->layout()->setSpacing(0);
    splitter->addWidget( splitterWidget );

    auto editorContainer = new QWidget( splitterWidget );
    editorContainer->setLayout( new QVBoxLayout );
    editorContainer->layout()->setMargin(0);
    editorContainer->layout()->setSpacing(0);
    static_cast<QVBoxLayout*>(splitterWidget->layout())->addWidget( editorContainer, 1 );

    // container for embedded widgets
    container_ = new QWidget( splitterWidget );
    container_->setLayout( new QVBoxLayout );
    container_->layout()->setMargin(0);
    container_->layout()->setSpacing(0);
    static_cast<QVBoxLayout*>(splitterWidget->layout())->addWidget( container_, 0 );

    // embedded widgets
    _createFindWidget();
    _createReplaceWidget();
    _createSelectLineWidget();

    // first editor
    auto& editor( _newTextEditor( editorContainer ) );
    editorContainer->layout()->addWidget( &editor );

    // assign stretch factors to splitters
    splitter->setStretchFactor( 0, 1 );
    splitter->setStretchFactor( 1, 0 );

    connect( splitter, SIGNAL(splitterMoved(int,int)), SLOT(_splitterMoved()) );
    connect( keywordEditor_, SIGNAL(modificationChanged(bool)), SLOT(_textModified(bool)) );
    connect( keywordEditor_, SIGNAL(cursorPositionChanged(int,int)), SLOT(_displayCursorPosition(int,int)) );
    connect( titleEditor_, SIGNAL(modificationChanged(bool)), SLOT(_textModified(bool)) );
    connect( titleEditor_, SIGNAL(cursorPositionChanged(int,int)), SLOT(_displayCursorPosition(int,int)) );
    connect( activeEditor_->document(), SIGNAL(modificationChanged(bool)), SLOT(_textModified(bool)) );

    // create attachment list
    auto frame = new AttachmentFrame( 0, readOnly_ );
    frame->visibilityAction().setChecked( false );
    frame->setDefaultHeight( XmlOptions::get().get<int>( "ATTACHMENT_FRAME_HEIGHT" ) );
    splitter->addWidget( frame );
    Base::Key::associate( this, frame );

    // status bar for tooltips
    setStatusBar( statusBar_ = new BaseStatusBar( this ) );
    statusBar_->addLabel( 2 );
    statusBar_->addLabels( 3, 0 );
    statusBar_->addClock();

    // actions
    _installActions();
    auto application( Base::Singleton::get().application<Application>() );
    addAction( &application->closeAction() );

    // toolbars
    // lock toolbar is visible only when window is not editable
    lock_ = new CustomToolBar( tr( "Lock" ), this, "LOCK_TOOLBAR" );
    lock_->setMovable( false );

    // hide lock_ visibility action because the latter should not be checkable in any menu
    lock_->visibilityAction().setVisible( false );

    QAction *action;
    lock_->addAction( action = new QAction( IconEngine::get( IconNames::Lock ), tr( "Unlock" ), this ) );
    connect( action, SIGNAL(triggered()), SLOT(_unlock()) );
    action->setToolTip( tr( "Remove read-only lock for current editor" ) );

    // main toolbar
    auto toolbar = new CustomToolBar( tr( "Main Toolbar" ), this, "MAIN_TOOLBAR" );
    toolbar->addAction( newEntryAction_ );
    toolbar->addAction( saveAction_ );
    toolbar->addAction( deleteEntryAction_ );
    readOnlyActions_.append( deleteEntryAction_ );
    toolbar->addAction( &frame->newAction() );

    // format bar
    formatBar_ = new FormatBar( this, "FORMAT_TOOLBAR" );
    formatBar_->setTarget( editor );
    formatBar_->addAction( insertLinkAction_ );
    readOnlyActions_.append( insertLinkAction_ );

    const auto& actions( formatBar_->actions() );
    std::copy( actions.begin(), actions.end(), std::back_inserter( readOnlyActions_ ) );

    // set proper connection for first editor
    // (because it could not be performed in _newTextEditor)
    connect(
        &editor, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
        formatBar_, SLOT(updateState(QTextCharFormat)) );

    // edition toolbars
    toolbar = new CustomToolBar( tr( "History" ), this, "EDITION_TOOLBAR" );
    toolbar->addAction( undoAction_ );
    toolbar->addAction( redoAction_ );

    using ActionList = QList<QAction*>;
    readOnlyActions_.append( Base::makeT<ActionList>({ undoAction_, redoAction_ }) );

    // undo/redo connections
    connect( keywordEditor_, SIGNAL(textChanged(QString)), SLOT(_updateUndoRedoActions()) );
    connect( keywordEditor_, SIGNAL(textChanged(QString)), SLOT(_updateSaveAction()) );

    connect( titleEditor_, SIGNAL(textChanged(QString)), SLOT(_updateUndoRedoActions()) );
    connect( titleEditor_, SIGNAL(textChanged(QString)), SLOT(_updateSaveAction()) );

    connect( qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), SLOT(_updateUndoRedoActions(QWidget*,QWidget*)) );

    // extra toolbar
    toolbar = new CustomToolBar( tr( "Tools" ), this, "EXTRA_TOOLBAR" );

    #if USE_ASPELL
    toolbar->addAction( spellcheckAction_ );
    #endif

    toolbar->addAction( printAction_ );
    toolbar->addAction( entryInformationAction_ );

    // extra toolbar
    toolbar = new CustomToolBar( tr( "Multiple Views" ), this, "MULTIPLE_VIEW_TOOLBAR" );
    toolbar->addAction( splitViewHorizontalAction_ );
    toolbar->addAction( splitViewVerticalAction_ );
    toolbar->addAction( cloneWindowAction_ );
    toolbar->addAction( closeAction_ );

    // extra toolbar
    toolbar = new CustomToolBar( tr( "Navigation" ), this, "NAVIGATION_TOOLBAR" );
    toolbar->addAction( &application->mainWindow().uniconifyAction() );
    toolbar->addAction( previousEntryAction_ );
    toolbar->addAction( nextEntryAction_ );

    // create menu if requested
    menuBar_ = new MenuBar( this, &application->mainWindow() );
    setMenuBar( menuBar_ );

    // changes display according to readOnly flag
    setReadOnly( readOnly_ );

    // update modifiers
    _modifiersChanged( activeEditor_->modifiers() );

    // configuration
    connect( application, SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    _updateConfiguration();

}

//____________________________________________
void EditionWindow::displayEntry( Keyword keyword, LogEntry *entry )
{

    Debug::Throw( "EditionWindow::displayEntry.\n" );
    keyword_ =  keyword;
    displayEntry( entry );

}

//____________________________________________
void EditionWindow::displayEntry( LogEntry *entry )
{
    Debug::Throw( "EditionWindow::displayEntry.\n" );

    // disassociate with existing entries, if any
    clearAssociations<LogEntry>();

    // update recentFiles menu
    auto&& mainWindow( _mainWindow() );
    menuBar_->recentFilesMenu().setCurrentFile( mainWindow.menuBar().recentFilesMenu().currentFile() );

    // check entry and associate to this editor
    if( !entry ) return;
    Base::Key::associate( entry, this );

    // check saved keyword
    if( !entry->keywords().contains( keyword_ ) )
    { keyword_ = entry->hasKeywords() ? *entry->keywords().begin():Keyword::Default; }

    // update all display
    displayKeyword();
    displayTitle();
    displayColor();
    _displayText();
    _displayAttachments();

    // update previous and next action states
    Debug::Throw( "EditionWindow::displayEntry - setting button states.\n" );
    previousEntryAction_->setEnabled( mainWindow.previousEntry(entry, false) );
    nextEntryAction_->setEnabled( mainWindow.nextEntry(entry, false) );

    // reset modify flag; change title accordingly
    setModified( false );

    _updateReadOnlyActions();
    _updateSaveAction();
    updateWindowTitle();

    Debug::Throw( "EditionWindow::displayEntry - done.\n" );
    return;
}

//____________________________________________
TextEditor& EditionWindow::activeEditor()
{ return *activeEditor_; }

//____________________________________________
const TextEditor& EditionWindow::activeEditor() const
{ return *activeEditor_; }

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
        auto entry( this->entry() );
        if( entry && Base::KeySet<Logbook>( entry ).empty() )
        { delete entry; }

    }
}

//_____________________________________________
QString EditionWindow::windowTitle() const
{

    Debug::Throw( "EditionWindow::windowTitle.\n" );
    const auto entry( this->entry() );

    // read only flag
    const bool readOnly( readOnly_ || (_hasMainWindow() && _mainWindow().logbookIsReadOnly() ) );

    if( entry && !entry->title().isEmpty() )
    {

        if( readOnly ) return tr( "%1 (read only)" ).arg( entry->title() );
        else if( modified()  ) return tr( "%1 (modified)" ).arg( entry->title() );
        else return entry->title();

    } else {

        if( readOnly ) return tr( "Untitled Logbook Entry (read only)" );
        else if( modified() ) return tr( "Untitled Logbook Entry (modified)" );
        else return tr( "Untitled Logbook Entry" );

    }

}

//_____________________________________________
void EditionWindow::displayKeyword()
{
    Debug::Throw( "EditionWindow::displayKeyword.\n" );

    keywordEditor_->setText( keyword_.get() );
    keywordEditor_->setCursorPosition( 0 );
    return;

}

//_____________________________________________
void EditionWindow::displayTitle()
{
    Debug::Throw( "EditionWindow::displayTitle.\n" );

    auto entry( this->entry() );
    if( entry ) titleEditor_->setText( entry->title() );
    titleEditor_->setCursorPosition( 0 );
    return;
}

//_____________________________________________
void EditionWindow::displayColor()
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
    activeEditor_->document()->setModified( value );
}

//___________________________________________________________
void EditionWindow::setForceShowKeyword( bool value )
{
    Debug::Throw( "EditionWindow::setForceShowKeyword.\n" );
    _setKeywordVisible( value || showKeywordAction_->isChecked() );
    showKeywordAction_->setEnabled( !value );
}

//___________________________________________________________
void EditionWindow::closeEditor( TextEditor& editor )
{
    Debug::Throw( "EditionWindow::closeEditor.\n" );

    // retrieve number of editors
    // if only one display, close the entire window
    Base::KeySet<TextEditor> editors( this );
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
    editors = Base::KeySet<TextEditor>( &editor );

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
    Base::KeySetIterator<TextEditor> iterator( editors.get() );
    iterator.toBack();
    while( iterator.hasPrevious() )
    {
        TextEditor* current( iterator.previous() );
        if( current != &editor )
        {
            setActiveEditor( *current );
            activeFound = true;
            break;
        }
    }
    Q_UNUSED( activeFound );
    Q_ASSERT( activeFound );

    // change focus
    activeEditor_->setFocus();
    Debug::Throw( "EditionWindow::closeEditor - done.\n" );

}

//________________________________________________________________
void EditionWindow::setActiveEditor( TextEditor& editor )
{
    Debug::Throw() << "EditionWindow::setActiveEditor - key: " << editor.key() << endl;
    Q_ASSERT( editor.isAssociated( this ) );

    activeEditor_ = static_cast<Private::LocalTextEditor*>(&editor);
    if( !activeEditor_->isActive() )
    {

        for( const auto& editor:Base::KeySet<TextEditor>( this ) )
        { editor->setActive( false ); }

        activeEditor_->setActive( true );

    }

    // associate with toolbar
    if( formatBar_ ) formatBar_->setTarget( *activeEditor_ );

}

//________________________________________________________________________
AskForSaveDialog::ReturnCode EditionWindow::askForSave()
{

    Debug::Throw( "EditionWindow::askForSave.\n" );

    // create dialog
    AskForSaveDialog::ReturnCodes buttons( AskForSaveDialog::Yes | AskForSaveDialog::No  | AskForSaveDialog::Cancel );
    AskForSaveDialog dialog( this, tr( "Entry has been modified. Save ?" ), buttons );

    // exec and check return code
    int state = dialog.centerOnParent().exec();
    if( state == AskForSaveDialog::Yes ) _save();

    return AskForSaveDialog::ReturnCode(state);

}

//_____________________________________________
void EditionWindow::writeEntryToLogbook( bool updateSelection )
{
    if( readOnly_ ) return;

    // retrieve associated entry
    auto entry( this->entry() );

    // see if entry is new
    const bool entryIsNew( !entry || Base::KeySet<Logbook>( entry ).empty() );

    // create entry if none set
    if( !entry ) entry = new LogEntry;

    // check logbook
    auto&& mainWindow( _mainWindow() );
    auto&& logbook( mainWindow.logbook() );
    if( !logbook )
    {
        InformationDialog( this, tr( "No logbook opened. <Save> canceled." ) ).exec();
        return;
    }

    // update entry text
    entry->setText( activeEditor_->toPlainText() );
    entry->setFormats( formatBar_->get() );

    // update entry keyword
    Keyword newKeyword( keywordEditor_->text() );
    if( keyword_ != newKeyword )
    {
        if( entry->keywords().contains( keyword_ ) ) entry->replaceKeyword( keyword_, newKeyword );
        else entry->addKeyword( newKeyword );
        keyword_ = newKeyword;
    }

    // update entry title
    entry->setTitle( titleEditor_->text() );

    // update author
    entry->setAuthor( XmlOptions::get().raw( "USER" ) );

    // add _now_ to entry modification timestamps
    entry->setModified();

    // status bar
    statusBar_->label().setText( tr( "writting entry to logbook..." ) );

    // add entry to logbook, if needed
    if( entryIsNew ) Base::Key::associate( entry, logbook->latestChild().get() );

    // update this window title, set unmodified.
    setModified( false );
    updateWindowTitle();

    // update associated EditionWindows
    for( const auto& window:Base::KeySet<EditionWindow> ( entry ) )
    { if( window != this ) window->displayEntry( entry ); }

    // update main window
    mainWindow.updateEntry( keyword_, entry, updateSelection );
    mainWindow.updateWindowTitle();

    // set logbook as modified
    for( const auto& logbook:Base::KeySet<Logbook>( entry ) )
    { logbook->setModified( true ); }

    // add to main logbook recent entries
    logbook->addRecentEntry( entry );

    statusBar_->label().clear();

    return;

}

//_____________________________________________
void EditionWindow::updateReadOnlyState()
{

    Debug::Throw( "EditionWindow::updateReadOnlyState\n" );
    _updateReadOnlyActions();
    _updateSaveAction();
    _updateUndoRedoActions();
    _updateInsertLinkActions();
    updateWindowTitle();

}

//_____________________________________________________________________
void EditionWindow::findFromDialog()
{
    Debug::Throw( "EditionWindow::findFromDialog.\n" );

    // create find widget
    if( !findWidget_ ) _createFindWidget();
    findWidget_->show();
    findWidget_->editor().setFocus();
    activeEditor().ensureCursorVisible();

    /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
    */
    QString text( activeEditor().selection().text() );
    if( !text.isEmpty() )
    {
        const int maxLength( 1024 );
        text = text.left( maxLength );
    }

    findWidget_->enableRegExp( true );
    findWidget_->synchronize();
    findWidget_->matchFound();
    findWidget_->setText( text );

    return;
}

//_____________________________________________________________________
void EditionWindow::replaceFromDialog()
{
    Debug::Throw( "EditionWindow::replaceFromDialog.\n" );

    // create replace widget
    if( !replaceWidget_ ) _createReplaceWidget();

    // show replace widget and set focus
    replaceWidget_->show();
    replaceWidget_->editor().setFocus();
    activeEditor().ensureCursorVisible();

    /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
    */

    // synchronize combo-boxes
    replaceWidget_->synchronize();
    replaceWidget_->matchFound();

    // update find text
    QString text;
    if( !( text = qApp->clipboard()->text( QClipboard::Selection) ).isEmpty() ) replaceWidget_->setText( text );
    else if( activeEditor().textCursor().hasSelection() ) replaceWidget_->setText( activeEditor().textCursor().selectedText() );
    else if( !( text = TextEditor::lastSelection().text() ).isEmpty() ) replaceWidget_->setText( text );

    // update replace text
    if( !TextEditor::lastSelection().replaceText().isEmpty() ) replaceWidget_->setReplaceText( TextEditor::lastSelection().replaceText() );

    return;
}

//________________________________________________
void EditionWindow::selectLineFromDialog()
{

    Debug::Throw( "TextEditor::selectLineFromDialog.\n" );

    // create select line widget
    if( !selectLineWidget_ ) _createSelectLineWidget();

    selectLineWidget_->show();
    selectLineWidget_->matchFound();
    selectLineWidget_->editor().clear();
    selectLineWidget_->editor().setFocus();

}

//____________________________________________
void EditionWindow::closeEvent( QCloseEvent *event )
{
    Debug::Throw( "EditionWindow::closeEvent.\n" );

    // ask for save if entry is modified
    if( !(readOnly_ || closed_ ) && modified() && askForSave() == AskForSaveDialog::Cancel ) event->ignore();
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
void EditionWindow::_installActions()
{
    Debug::Throw( "EditionWindow::_installActions.\n" );

    // undo action
    addAction( undoAction_ = new QAction( IconEngine::get( IconNames::Undo ), tr( "Undo" ), this ) );
    undoAction_->setToolTip( tr( "Undo last modification" ) );
    connect( undoAction_, SIGNAL(triggered()), SLOT(_undo()) );

    // redo action
    addAction( redoAction_ = new QAction( IconEngine::get( IconNames::Redo ), tr( "Redo" ), this ) );
    redoAction_->setToolTip( tr( "Redo last undone modification" ) );
    connect( redoAction_, SIGNAL(triggered()), SLOT(_redo()) );

    // new entry
    addAction( newEntryAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New Entry" ), this ) );
    newEntryAction_->setIconText( tr( "New" ) );
    newEntryAction_->setShortcut( QKeySequence::New );
    newEntryAction_->setToolTip( tr( "Create new entry in current editor" ) );
    connect( newEntryAction_, SIGNAL(triggered()), SLOT(_newEntry()) );

    // previous entry action
    addAction( previousEntryAction_ = new QAction( IconEngine::get( IconNames::PreviousEntry ), tr( "Previous Entry" ), this ) );
    previousEntryAction_->setIconText( tr( "Previous" ) );
    previousEntryAction_->setToolTip( tr( "Display previous entry in current list" ) );
    connect( previousEntryAction_, SIGNAL(triggered()), SLOT(_previousEntry()) );

    // next entry action
    addAction( nextEntryAction_ = new QAction( IconEngine::get( IconNames::NextEntry ), tr( "Next Entry" ), this ) );
    nextEntryAction_->setIconText( tr( "Next" ) );
    nextEntryAction_->setToolTip( tr( "Display next entry in current list" ) );
    connect( nextEntryAction_, SIGNAL(triggered()), SLOT(_nextEntry()) );

    // save
    addAction( saveAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save Entry" ), this ) );
    saveAction_->setIconText( tr( "Save" ) );
    saveAction_->setToolTip( tr( "Save current entry" ) );
    connect( saveAction_, SIGNAL(triggered()), SLOT(_save()) );
    saveAction_->setShortcut( QKeySequence::Save );

    #if USE_ASPELL
    addAction( spellcheckAction_ = new QAction( IconEngine::get( IconNames::SpellCheck ), tr( "Check Spelling..." ), this ) );
    spellcheckAction_->setToolTip( tr( "Check spelling of current entry" ) );
    connect( spellcheckAction_, SIGNAL(triggered()), SLOT(_spellCheck()) );
    spellcheckAction_->setEnabled( !SpellCheck::SpellInterface().dictionaries().empty() );
    #endif

    // entry information
    addAction( entryInformationAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Entry Properties..." ), this ) );
    entryInformationAction_->setIconText( tr( "Properties" ) );
    entryInformationAction_->setToolTip( tr( "Show current entry properties" ) );
    connect( entryInformationAction_, SIGNAL(triggered()), SLOT(_entryInformation()) );

    // print
    addAction( printAction_ = new QAction( IconEngine::get( IconNames::Print ), tr( "Print..." ), this ) );
    printAction_->setToolTip( tr( "Print current logbook entry" ) );
    printAction_->setShortcut( QKeySequence::Print );
    connect( printAction_, SIGNAL(triggered()), SLOT(_print()) );

    // print preview
    addAction( printPreviewAction_ = new QAction( IconEngine::get( IconNames::PrintPreview ), tr( "Print Preview..." ), this ) );
    connect( printPreviewAction_, SIGNAL(triggered()), SLOT(_printPreview()) );

    // print
    addAction( htmlAction_ = new QAction( IconEngine::get( IconNames::Html ), tr( "Export to HTML..." ), this ) );
    htmlAction_->setToolTip( tr( "Export current logbook entry to HTML" ) );
    connect( htmlAction_, SIGNAL(triggered()), SLOT(_toHtml()) );

    // split action
    addAction( splitViewHorizontalAction_ =new QAction( IconEngine::get( IconNames::ViewTopBottom ), tr( "Split View Top/Bottom" ), this ) );
    splitViewHorizontalAction_->setToolTip( tr( "Split current text editor vertically" ) );
    connect( splitViewHorizontalAction_, SIGNAL(triggered()), SLOT(_splitViewVertical()) );

    addAction( splitViewVerticalAction_ =new QAction( IconEngine::get( IconNames::ViewLeftRight ), tr( "Split View Left/Right" ), this ) );
    splitViewVerticalAction_->setToolTip( tr( "Split current text editor horizontally" ) );
    connect( splitViewVerticalAction_, SIGNAL(triggered()), SLOT(_splitViewHorizontal()) );

    // clone window action
    addAction( cloneWindowAction_ = new QAction( IconEngine::get( IconNames::ViewClone ), tr( "Clone Window" ), this ) );
    cloneWindowAction_->setToolTip( tr( "Create a new edition window displaying the same entry" ) );
    connect( cloneWindowAction_, SIGNAL(triggered()), SLOT(_cloneWindow()) );

    // close window action
    addAction( closeAction_ = new QAction( IconEngine::get( IconNames::ViewRemove ), tr( "Close View" ), this ) );
    closeAction_->setShortcut( QKeySequence::Close );
    closeAction_->setToolTip( tr( "Close current view" ) );
    connect( closeAction_, SIGNAL(triggered()), SLOT(_close()) );

    addAction( deleteEntryAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete Entry" ), this ) );
    deleteEntryAction_->setIconText( tr( "Delete" ) );
    connect( deleteEntryAction_, SIGNAL(triggered()), SLOT(_deleteEntry()) );

    // uniconify
    addAction( uniconifyAction_ = new QAction( tr( "Uniconify" ), this ) );
    connect( uniconifyAction_, SIGNAL(triggered()), SLOT(uniconify()) );

    // show/hide keyword
    addAction( showKeywordAction_ = new QAction( tr( "Show Keyword" ), this ) );
    showKeywordAction_->setCheckable( true );
    showKeywordAction_->setChecked( false );
    connect( showKeywordAction_, SIGNAL(toggled(bool)), SLOT(_toggleShowKeyword(bool)) );

    // insert link
    addAction( insertLinkAction_ = new QAction( IconEngine::get( IconNames::InsertSymbolicLink ), tr( "Insert Link" ), this ) );
    connect( insertLinkAction_, SIGNAL(triggered()), SLOT(_insertLink()) );
    insertLinkAction_->setEnabled( false );
}


//______________________________________________________________________
void EditionWindow::_createFindWidget()
{

    Debug::Throw( "EditionWindow::_createFindWidget.\n" );
    if( !findWidget_ )
    {

        findWidget_ = new BaseFindWidget( container_ );
        container_->layout()->addWidget( findWidget_ );
        connect( findWidget_, SIGNAL(find(TextSelection)), SLOT(_find(TextSelection)) );
        connect( this, SIGNAL(matchFound()), findWidget_, SLOT(matchFound()) );
        connect( this, SIGNAL(noMatchFound()), findWidget_, SLOT(noMatchFound()) );
        connect( &findWidget_->closeButton(), SIGNAL(clicked()), SLOT(_restoreFocus()) );
        findWidget_->hide();

    }

    return;

}

//_____________________________________________________________________
void EditionWindow::_createReplaceWidget()
{
    Debug::Throw( "EditionWindow::_CreateReplaceDialog.\n" );
    if( !( replaceWidget_ ) )
    {

        replaceWidget_ = new BaseReplaceWidget( container_ );
        container_->layout()->addWidget( replaceWidget_ );
        connect( replaceWidget_, SIGNAL(find(TextSelection)), SLOT(_find(TextSelection)) );
        connect( replaceWidget_, SIGNAL(replace(TextSelection)), SLOT(_replace(TextSelection)) );
        connect( replaceWidget_, SIGNAL(replaceInWindow(TextSelection)), SLOT(_replaceInWindow(TextSelection)) );
        connect( replaceWidget_, SIGNAL(replaceInSelection(TextSelection)), SLOT(_replaceInSelection(TextSelection)) );
        connect( replaceWidget_, SIGNAL(menuAboutToShow()), SLOT(_updateReplaceInSelection()) );
        connect( &replaceWidget_->closeButton(), SIGNAL(clicked()), SLOT(_restoreFocus()) );
        replaceWidget_->hide();

        connect( this, SIGNAL(matchFound()), replaceWidget_, SLOT(matchFound()) );
        connect( this, SIGNAL(noMatchFound()), replaceWidget_, SLOT(noMatchFound()) );

    }

}

//_________________________________________________________________
void EditionWindow::_createSelectLineWidget()
{
    if( !selectLineWidget_ )
    {
        selectLineWidget_ = new SelectLineWidget( this, true );
        container_->layout()->addWidget( selectLineWidget_ );
        connect( selectLineWidget_, SIGNAL(lineSelected(int)), SLOT(_selectLine(int)) );
        connect( this, SIGNAL(lineFound()), selectLineWidget_, SLOT(matchFound()) );
        connect( this, SIGNAL(lineNotFound()), selectLineWidget_, SLOT(noMatchFound()) );
        connect( &selectLineWidget_->closeButton(), SIGNAL(clicked()), SLOT(_restoreFocus()) );
        selectLineWidget_->hide();
    }
}

//___________________________________________________________
Private::LocalTextEditor& EditionWindow::_splitView( const Qt::Orientation& orientation )
{
    Debug::Throw( "EditionWindow::_splitView.\n" );

    // keep local pointer to current active display
    auto& activeEditorLocal( *activeEditor_ );

    // compute desired dimension of the new splitter
    // along its splitting direction
    int dimension( (orientation == Qt::Horizontal) ? activeEditorLocal.width():activeEditorLocal.height() );

    // create new splitter
    QSplitter& splitter( _newSplitter( orientation ) );

    // create new display
    auto& editor( _newTextEditor(nullptr) );

    // insert in splitter, at correct position
    splitter.insertWidget( splitter.indexOf( &activeEditorLocal )+1, &editor );

    // recompute dimension
    // take the max of active display and splitter,
    // in case no new splitter was created.
    dimension = qMax( dimension, (orientation == Qt::Horizontal) ? splitter.width():splitter.height() );

    // assign equal size to all splitter children
    QList<int> sizes;
    for( int i=0; i<splitter.count(); i++ )
    { sizes.append( dimension/splitter.count() ); }
    splitter.setSizes( sizes );

    // synchronize both editors, if cloned
    /*
    if there exists no clone of active display,
    backup text and register a new Sync object
    */
    Base::KeySet<Private::LocalTextEditor> editors( &activeEditorLocal );

    // clone new display
    editor.synchronize( &activeEditorLocal );

    // perform associations
    // check if active editors has associates and propagate to new
    for( const auto& iter:editors )
    { Base::Key::associate( &editor, iter ); }

    // associate new display to active
    Base::Key::associate( &editor, &activeEditorLocal );

    return editor;

}

//____________________________________________________________
QSplitter& EditionWindow::_newSplitter( const Qt::Orientation& orientation )
{

    Debug::Throw( "EditionWindow::_newSplitter.\n" );
    QSplitter *splitter = nullptr;

    // retrieve parent of current display
    auto parent( activeEditor_->parentWidget() );

    // try cast to splitter
    // do not create a new splitter if the parent has same orientation
    auto parentSplitter( qobject_cast<QSplitter*>( parent ) );
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
            splitter = new Private::LocalSplitter( nullptr );
            splitter->setOrientation( orientation );
            parentSplitter->insertWidget( parentSplitter->indexOf( activeEditor_ ), splitter );

        } else {

            Debug::Throw( "EditionWindow::_newSplitter - no splitter found. Creating a new one.\n" );

            // create a splitter with correct orientation
            splitter = new Private::LocalSplitter(parent);
            splitter->setOrientation( orientation );
            parent->layout()->addWidget( splitter );

        }

        // reparent current display
        splitter->addWidget( activeEditor_ );

        // resize parent splitter if any
        if( parentSplitter )
        {
            int dimension = ( parentSplitter->orientation() == Qt::Horizontal) ?
                parentSplitter->width():
                parentSplitter->height();

            QList<int> sizes;
            for( int i=0; i<parentSplitter->count(); i++ )
            { sizes.append( dimension/parentSplitter->count() ); }
            parentSplitter->setSizes( sizes );

        }

    }

    // return created splitter
    return *splitter;

}

//_____________________________________________________________
Private::LocalTextEditor& EditionWindow::_newTextEditor( QWidget* parent )
{
    Debug::Throw( "EditionWindow::_newTextEditor.\n" );

    // create TextEditor
    auto editor = new Private::LocalTextEditor( parent );

    // connections
    connect( &editor->insertLinkAction(), SIGNAL(triggered()), SLOT(_insertLink()) );
    connect( &editor->editLinkAction(), SIGNAL(triggered()), SLOT(_editLink()) );
    connect( &editor->removeLinkAction(), SIGNAL(triggered()), SLOT(_removeLink()) );
    connect( &editor->openLinkAction(), SIGNAL(triggered()), SLOT(_openLink()) );
    connect( editor, SIGNAL(linkActivated(QString)), SLOT(_openLink(QString)) );
    connect( editor, SIGNAL(hasFocus(TextEditor*)), SLOT(_displayFocusChanged(TextEditor*)) );
    connect( editor, SIGNAL(cursorPositionChanged()), SLOT(_displayCursorPosition()) );
    connect( editor, SIGNAL(modifiersChanged(TextEditor::Modifiers)), SLOT(_modifiersChanged(TextEditor::Modifiers)) );
    connect( editor, SIGNAL(undoAvailable(bool)), SLOT(_updateUndoRedoActions()) );
    connect( editor, SIGNAL(selectionChanged()), SLOT(_updateInsertLinkActions()) );
    connect( editor->document(), SIGNAL(modificationChanged(bool)), SLOT(_updateSaveAction()) );

    // customize display actions
    /* this is needed to be able to handle a single dialog for stacked windows */
    // goto line number
    editor->gotoLineAction().disconnect();
    connect( &editor->gotoLineAction(), SIGNAL(triggered()), SLOT(selectLineFromDialog()) );

    // find
    editor->findAction().disconnect();
    connect( &editor->findAction(), SIGNAL(triggered()), SLOT(findFromDialog()) );
    connect( editor, SIGNAL(noMatchFound()), this, SIGNAL(noMatchFound()) );
    connect( editor, SIGNAL(matchFound()), this, SIGNAL(matchFound()) );
    connect( editor, SIGNAL(lineNotFound()), this, SIGNAL(lineNotFound()) );
    connect( editor, SIGNAL(lineFound()), this, SIGNAL(lineFound()) );

    // replace
    editor->replaceAction().disconnect();
    connect( &editor->replaceAction(), SIGNAL(triggered()), SLOT(replaceFromDialog()) );

    if( formatBar_ )
    {
        connect(
            editor, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            formatBar_, SLOT(updateState(QTextCharFormat)) );
    }

    // associate display to this editFrame
    Base::Key::associate( this, editor );

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

    statusBar_->label(2).setText( tr( "Line: %1" ).arg( position.paragraph()+1 ), false );
    statusBar_->label(3).setText( tr( "Column: %1" ).arg( position.index()+1 ), true );

    return;
}

//_______________________________________________
bool EditionWindow::_hasMainWindow() const
{ return Base::KeySet<MainWindow>( this ).size() > 0; }

//_______________________________________________
MainWindow& EditionWindow::_mainWindow() const
{
    Debug::Throw( "EditionWindow::_mainWindow.\n" );
    Base::KeySet<MainWindow> mainWindows( this );
    return **mainWindows.begin();
}

//_____________________________________________
void EditionWindow::_displayText()
{
    Debug::Throw( "EditionWindow::_displayText.\n" );
    if( !activeEditor_ ) return;

    auto entry( this->entry() );
    activeEditor_->setCurrentCharFormat( QTextCharFormat() );
    activeEditor_->setPlainText( (entry) ? entry->text() : QString() );
    formatBar_->load( entry->formats() );

    // reset undo/redo stack
    activeEditor_->resetUndoRedoStack();

    return;
}

//_____________________________________________
void EditionWindow::_displayAttachments()
{
    Debug::Throw( "EditionWindow::_DisplayAttachments.\n" );

    auto &frame( attachmentFrame() );
    frame.clear();

    auto entry( this->entry() );
    if( !entry )
    {

        frame.visibilityAction().setChecked( false );
        return;

    }

    // get associated attachments
    Base::KeySet<Attachment> attachments( entry );
    if( attachments.empty() ) {

        frame.visibilityAction().setChecked( false );

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

    // save this entry to the logbook
    writeEntryToLogbook( updateSelection );

    // Save logbook
    auto&& mainWindow( _mainWindow() );
    auto&& logbook( mainWindow.logbook() );
    if( logbook && !logbook->file().isEmpty() )
    { mainWindow.saveUnchecked(); }

    return;

}

//___________________________________________________________
void EditionWindow::_print()
{
    Debug::Throw( "EditionWindow::_print.\n" );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create helper
    LogEntryPrintHelper helper( this );
    helper.setEntry( entry() );

    _print( helper );

}

//___________________________________________________________
void EditionWindow::_print( LogEntryPrintHelper& helper )
{

    // create printer
    QPrinter printer( QPrinter::HighResolution );

    // generate document name
    QString buffer;
    QTextStream( &buffer )  << "elogbook_" << Util::user() << "_" << TimeStamp::now().unixTime() << "_" << Util::pid();
    printer.setDocName( buffer );

    // create option widget
    PrinterOptionWidget* optionWidget( new PrinterOptionWidget );
    optionWidget->setHelper( &helper );

    connect( optionWidget, SIGNAL(orientationChanged(QPrinter::Orientation)), &helper, SLOT(setOrientation(QPrinter::Orientation)) );
    connect( optionWidget, SIGNAL(pageModeChanged(BasePrintHelper::PageMode)), &helper, SLOT(setPageMode(BasePrintHelper::PageMode)) );

    LogEntryPrintOptionWidget* logEntryOptionWidget = new LogEntryPrintOptionWidget;
    logEntryOptionWidget->setWindowTitle( tr( "Logbook Entry Configuration" ) );
    connect( logEntryOptionWidget, SIGNAL(maskChanged(LogEntry::Mask)), &helper, SLOT(setMask(LogEntry::Mask)) );
    logEntryOptionWidget->read( XmlOptions::get() );

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setWindowTitle( Util::windowTitle( tr( "Print Logbook Entry" ) ) );

    using WidgetList = QList<QWidget*>;
    dialog.setOptionTabs( Base::makeT<WidgetList>({ optionWidget, logEntryOptionWidget }) );
    if( !dialog.exec() ) return;

    // add output file to scratch files, if any
    if( !printer.outputFileName().isEmpty() )
    { emit scratchFileCreated( File( printer.outputFileName() ) ); }

    // write options
    logEntryOptionWidget->write( XmlOptions::get() );

    // print
    helper.print( &printer );

    return;

}

//___________________________________________________________
void EditionWindow::_printPreview()
{
    Debug::Throw( "EditionWindow::_printPreview.\n" );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create helper
    LogEntryPrintHelper helper( this );
    helper.setEntry( entry() );
    helper.setMask( (LogEntry::Mask) XmlOptions::get().get<int>( "LOGENTRY_PRINT_OPTION_MASK" ) );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this, CustomDialog::OkButton|CustomDialog::CancelButton );
    dialog.setWindowTitle( tr( "Print Preview" ) );
    dialog.setHelper( &helper );

    // print
    if( dialog.exec() ) _print( helper );

}

//___________________________________________________________
void EditionWindow::_toHtml()
{
    Debug::Throw( "EditionWindow::_toHtml.\n" );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create option widget
    LogEntryPrintOptionWidget* optionWidget = new LogEntryPrintOptionWidget;
    optionWidget->read( XmlOptions::get() );

    // create dialog
    HtmlDialog dialog( this );

    using WidgetList = QList<QWidget*>;
    dialog.setOptionWidgets( Base::makeT<WidgetList>({ optionWidget }) );
    dialog.setWindowTitle( tr( "Export to HTML" ) );

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
        InformationDialog( this, tr( "Cannot write to file '%1'. <View HTML> canceled." ).arg( file ) ).exec();
        return;
    }

    // add as scratch file
    emit scratchFileCreated( file );

    // write options
    optionWidget->write( XmlOptions::get() );

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
void EditionWindow::_newEntry()
{

    Debug::Throw( "EditionWindow::_newEntry.\n" );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create new entry, set author, set keyword
    auto entry = new LogEntry;
    entry->setAuthor( XmlOptions::get().raw( "USER" ) );
    entry->addKeyword( _mainWindow().currentKeyword() );

    // display new entry
    displayEntry( entry );

    // set focus to title bar
    titleEditor_->setFocus();
    titleEditor_->selectAll();

}

//_____________________________________________
void EditionWindow::_splitterMoved()
{
    Debug::Throw( "MainWindow::_splitterMoved.\n" );
    resizeTimer_.start( 200, this );
}

//_______________________________________________
void EditionWindow::_previousEntry()
{
    Debug::Throw( "EditionWindow::_previousEntry.\n" );

    MainWindow &mainWindow( _mainWindow() );
    auto entry( mainWindow.previousEntry( this->entry(), true ) );
    if( !( entry  && mainWindow.lockEntry( entry ) ) ) return;
    displayEntry( entry );
    setReadOnly( false );

}

//_______________________________________________
void EditionWindow::_nextEntry()
{
    Debug::Throw( "EditionWindow::_nextEntry.\n" );

    MainWindow &mainWindow( _mainWindow() );
    auto entry( mainWindow.nextEntry( this->entry(), true ) );
    if( !( entry && mainWindow.lockEntry( entry ) ) ) return;
    displayEntry( entry );
    setReadOnly( false );

}

//_____________________________________________
void EditionWindow::_entryInformation()
{

    Debug::Throw( "EditionWindow::_EntryInfo.\n" );

    // check entry
    auto entry( this->entry() );
    if( !entry ) {
        InformationDialog( this, tr( "No valid entry." ) ).exec();
        return;
    }

    // create dialog
    LogEntryInformationDialog( this, entry ).centerOnParent().exec();

}

//_____________________________________________
void EditionWindow::_undo()
{
    Debug::Throw( "EditionWindow::_undo.\n" );
    if( activeEditor_->QWidget::hasFocus() ) activeEditor_->document()->undo();
    else if( titleEditor_->hasFocus() ) titleEditor_->undo();
    else if( keywordEditor_->hasFocus() ) keywordEditor_->undo();
    return;
}

//_____________________________________________
void EditionWindow::_redo()
{
    Debug::Throw( "EditionWindow::_redo.\n" );
    if( activeEditor_->QWidget::hasFocus() ) activeEditor_->document()->redo();
    else if( titleEditor_->hasFocus() ) titleEditor_->redo();
    else if( keywordEditor_->hasFocus() ) keywordEditor_->redo();
    return;
}

//_____________________________________________
void EditionWindow::_insertLink()
{
    Debug::Throw( "EditionWindow::_insertLink.\n" );

    // check readonly and selection
    const QTextCursor cursor( activeEditor_->textCursor() );
    if( readOnly_ || !cursor.hasSelection() ) return;

    const QTextCharFormat format( cursor.charFormat() );
    const QString selection( format.anchorHref().isEmpty() ? cursor.selectedText() : format.anchorHref() );

    // create dialog
    InsertLinkDialog dialog( this, selection );
    dialog.setWindowTitle( tr( "Insert link" ) );
    if( !dialog.exec() ) return;

    // update format
    QTextCharFormat outputFormat;
    outputFormat.setFontUnderline( true );
    outputFormat.setForeground( activeEditor_->palette().color( QPalette::Link ) );
    outputFormat.setAnchorHref( dialog.link() );
    outputFormat.setAnchor( true );
    activeEditor_->mergeCurrentCharFormat( outputFormat );

}

//_____________________________________________
void EditionWindow::_editLink()
{
    Debug::Throw( "EditionWindow::_editLink.\n" );
    auto cursor( activeEditor_->cursorAtContextMenu() );
    auto block( cursor.block() );

    // loop over text fragments and find the one that matches cursor
    for( auto&& it = block.begin(); !(it.atEnd()); ++it)
    {
        QTextFragment fragment = it.fragment();
        if( !fragment.isValid() ) continue;
        if( fragment.position() > cursor.position() || fragment.position() + fragment.length() <= cursor.position() )
        { continue; }

        QString anchor( fragment.charFormat().anchorHref() );
        if( anchor.isEmpty() ) continue;

        // select the corresponding block
        QTextCursor cursor( activeEditor_->textCursor() );
        cursor.setPosition( fragment.position() );
        cursor.setPosition( fragment.position() + fragment.length(), QTextCursor::KeepAnchor );
        activeEditor_->setTextCursor( cursor );

        // insert link
        _insertLink();

        break;
    }
}

//_____________________________________________
void EditionWindow::_removeLink()
{
    Debug::Throw( "EditionWindow::_removeLink.\n" );
    QTextCursor cursor( activeEditor_->cursorAtContextMenu() );
    auto block( cursor.block() );

    // loop over text fragments and find the one that matches cursor
    for( auto&& it = block.begin(); !(it.atEnd()); ++it)
    {
        QTextFragment fragment = it.fragment();
        if( !fragment.isValid() ) continue;
        if( fragment.position() > cursor.position() || fragment.position() + fragment.length() <= cursor.position() )
        { continue; }

        QString anchor( fragment.charFormat().anchorHref() );
        if( anchor.isEmpty() ) continue;

        // select the corresponding block
        QTextCursor cursor( activeEditor_->textCursor() );
        cursor.setPosition( fragment.position() );
        cursor.setPosition( fragment.position() + fragment.length(), QTextCursor::KeepAnchor );
        activeEditor_->setTextCursor( cursor );

        // insert link
        QTextCharFormat outputFormat;
        outputFormat.setFontUnderline( false );
        outputFormat.setAnchorHref( QString() );
        outputFormat.setAnchor( false );
        outputFormat.setForeground( QColor() );
        activeEditor_->mergeCurrentCharFormat( outputFormat );

        break;
    }
}
//_____________________________________________
void EditionWindow::_openLink()
{
    Debug::Throw( "EditionWindow::_openLink.\n" );
    QString anchor( activeEditor_->anchor() );
    if( anchor.isEmpty() ) return;

    OpenWithDialog dialog( this );
    dialog.setWindowTitle( tr( "Open Link" ) );
    dialog.setLink( File( anchor ) );
    dialog.setOptionName( "OPEN_LINK_APPLICATIONS" );
    dialog.realizeWidget();
    dialog.exec();

}

//_____________________________________________
void EditionWindow::_openLink( QString anchor )
{
    Debug::Throw( "EditionWindow::_openLink.\n" );
    if( !anchor.isEmpty() ) QDesktopServices::openUrl( QUrl::fromEncoded( anchor.toLatin1() ) );
}

//_____________________________________________
void EditionWindow::_deleteEntry()
{

    Debug::Throw( "EditionWindow::_deleteEntry.\n" );

    // check current entry
    auto entry( this->entry() );

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
void EditionWindow::_spellCheck()
{
    #if USE_ASPELL
    Debug::Throw( "EditionWindow::_spellCheck.\n" );

    // create dialog
    SpellCheck::SpellDialog dialog( activeEditor_ );

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
void EditionWindow::_cloneWindow()
{

    Debug::Throw( "EditionWindow::_cloneWindow.\n" );
    auto entry( this->entry() );
    if( !entry ) {
        InformationDialog( this, tr( "No valid entry found. <New window> canceled." ) ).exec();
        return;
    }

    // retrieve selection frame
    MainWindow &mainWindow( _mainWindow() );

    // create new EditionWindow
    EditionWindow *editionWindow( new EditionWindow( &mainWindow ) );
    Base::Key::associate( editionWindow, &mainWindow );
    editionWindow->displayEntry( entry );

    // raise EditionWindow
    editionWindow->show();

    return;
}

//_____________________________________________
void EditionWindow::_unlock()
{

    Debug::Throw( "EditionWindow::_unlock.\n" );

    if( !readOnly_ ) return;
    auto entry( this->entry() );

    if( entry && ! _mainWindow().lockEntry( entry ) ) return;
    setReadOnly( false );

    return;

}

//_____________________________________________
void EditionWindow::_updateReplaceInSelection()
{ if( replaceWidget_ ) replaceWidget_->enableReplaceInSelection( activeEditor().hasSelection() ); }

//_____________________________________________
void EditionWindow::_updateReadOnlyActions()
{

    Debug::Throw( "EditionWindow::_updateReadOnlyActions.\n" );

    // add flag from logbook read only
    const bool logbookReadOnly( _hasMainWindow() && _mainWindow().logbookIsReadOnly() );
    const bool readOnly( readOnly_ || logbookReadOnly );

    // changes button state
    for( const auto& action:readOnlyActions_ )
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
    for( const auto& editor:Base::KeySet<Private::LocalTextEditor>( this ) )
    { editor->setReadOnly( readOnly ); }

    // changes attachment list status
    attachmentFrame().setReadOnly( readOnly );

    // new entry
    newEntryAction_->setEnabled( !logbookReadOnly );

    #if USE_ASPELL
    spellcheckAction_->setEnabled( !( readOnly || SpellCheck::SpellInterface().dictionaries().empty() ) );
    #endif

}

//_____________________________________________
void EditionWindow::_updateSaveAction()
{ saveAction_->setEnabled( !readOnly_ && !( _hasMainWindow() && _mainWindow().logbookIsReadOnly() ) && modified() ); }

//_____________________________________________
void EditionWindow::_updateUndoRedoActions()
{

    Debug::Throw( "EditionWindow::_updateRedoAction.\n" );
    if( keywordEditor_->hasFocus() )
    {
        undoAction_->setEnabled( keywordEditor_->isUndoAvailable() && !keywordEditor_->isReadOnly() );
        redoAction_->setEnabled( keywordEditor_->isRedoAvailable() && !keywordEditor_->isReadOnly() );

    } else if( titleEditor_->hasFocus() ) {

        undoAction_->setEnabled( titleEditor_->isUndoAvailable() && !titleEditor_->isReadOnly() );
        redoAction_->setEnabled( titleEditor_->isRedoAvailable() && !titleEditor_->isReadOnly() );

    } else if( activeEditor_->QWidget::hasFocus() ) {

        undoAction_->setEnabled( activeEditor_->document()->isUndoAvailable() && !activeEditor_->isReadOnly() );
        redoAction_->setEnabled( activeEditor_->document()->isRedoAvailable() && !activeEditor_->isReadOnly() );

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

    } else if( current == activeEditor_ ) {

        undoAction_->setEnabled( activeEditor_->document()->isUndoAvailable() && !activeEditor_->isReadOnly() );
        redoAction_->setEnabled( activeEditor_->document()->isRedoAvailable() && !activeEditor_->isReadOnly() );

    }

}

//_____________________________________________
void EditionWindow::_updateInsertLinkActions()
{
    Debug::Throw( "EditionWindow::_updateInsertLinkActions.\n" );
    const bool enabled( !readOnly_ && !( _hasMainWindow() && _mainWindow().logbookIsReadOnly() ) && activeEditor_->textCursor().hasSelection() );

    // disable main window action
    if( insertLinkAction_ ) insertLinkAction_->setEnabled( enabled );

    // also disable editors action
    Base::KeySet<Private::LocalTextEditor> editors( this );
    for( const auto& editor:editors )
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
void EditionWindow::_close()
{
    Debug::Throw( "EditionWindow::_closeView (SLOT)\n" );
    Base::KeySet< Private::LocalTextEditor > editors( this );
    if( editors.size() > 1 ) closeEditor( activeEditor() );
    else close();
}

//_____________________________________________
void EditionWindow::_displayFocusChanged( TextEditor* editor )
{

    Debug::Throw() << "EditionWindow::_DisplayFocusChanged - " << editor->key() << endl;
    setActiveEditor( *static_cast<Private::LocalTextEditor*>(editor) );

}

//________________________________________________________________
void EditionWindow::_modifiersChanged( TextEditor::Modifiers modifiers )
{
    if( !_hasStatusBar() ) return;
    QStringList buffer;
    if( modifiers & TextEditor::Modifier::Wrap ) buffer.append( "WRAP" );
    if( modifiers & TextEditor::Modifier::Insert ) buffer.append( "INS" );
    if( modifiers & TextEditor::Modifier::CapsLock ) buffer.append( "CAPS" );
    if( modifiers & TextEditor::Modifier::NumLock ) buffer.append( "NUM" );
    statusBar_->label(1).setText( buffer.join( " " ) );
}

//________________________________________________________________
void EditionWindow::_toggleShowKeyword( bool value )
{

    Debug::Throw( "EditionWindow::_toggleShowKeyword.\n" );
    _setKeywordVisible( value || forceShowKeyword_ );
    XmlOptions::get().set<bool>( "SHOW_KEYWORD", value );

}

//_____________________________________________
void EditionWindow::_updateConfiguration()
{

    // one should check whether this is needed or not.
    Debug::Throw( "EditionWindow::_updateConfiguration.\n" );
    resize( sizeHint() );

    // show keyword
    showKeywordAction_->setChecked( XmlOptions::get().get<bool>( "SHOW_KEYWORD" ) );

}

//______________________________________________________
Private::LocalTextEditor::LocalTextEditor( QWidget* parent ):
    TextEditor( parent )
{
    setTrackAnchors( true );
    _installActions();

    // configuration
    auto application( Base::Singleton::get().application<Application>() );
    connect( application, SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    _updateConfiguration();

}

//___________________________________________________________________________________
void Private::LocalTextEditor::insertFromMimeData( const QMimeData* source )
{
    Debug::Throw( "Private::LocalTextEditor::insertFromMimeData.\n" );

    // check option
    if( !autoInsertLinks_ )
    { return TextEditor::insertFromMimeData( source ); }

    // do nothing in case of rich text
    if( source->hasFormat(QLatin1String("application/x-qrichtext") ) )
    { return TextEditor::insertFromMimeData( source ); }

    // get source text
    QString text;
    if( source->hasHtml() ) text = source->html();
    else text = source->text();
    if( text.isNull() ) return TextEditor::insertFromMimeData( source );

    // try build an url from text
    QUrl url( text );
    if( !url.isValid() || url.isRelative() ) return TextEditor::insertFromMimeData( source );

    // check scheme
    static const auto schemes = Base::makeT<QStringList>( { "file", "ftp", "http", "https" } );
    if( !schemes.contains( url.scheme() ) ) return TextEditor::insertFromMimeData( source );

    // copy mime type
    // redo html addind the proper href
    QMimeData copy;
    copy.setText( source->text() );
    copy.setHtml( QString( "<a href=\"%1\">%1</a> " ).arg( text ) );
    return TextEditor::insertFromMimeData( &copy );

}

//___________________________________________________________________________________
void Private::LocalTextEditor::installContextMenuActions( BaseContextMenu* menu, bool allActions )
{
    Debug::Throw( "Private::LocalTextEditor::installContextMenuActions.\n" );
    TextEditor::installContextMenuActions( menu, allActions );

    // insert link
    menu->insertAction( &showLineNumberAction(), insertLinkAction_ );

    // open link
    if( !anchorAt( _contextMenuPosition() ).isEmpty() )
    {
        menu->insertAction( &copyLinkAction(), openLinkAction_ );
        menu->insertAction( &copyLinkAction(), editLinkAction_ );
        menu->insertAction( &copyLinkAction(), removeLinkAction_ );
    }

    // separator
    menu->insertSeparator( &showLineNumberAction() );

}

//_____________________________________________
void Private::LocalTextEditor::_updateConfiguration()
{ autoInsertLinks_ = XmlOptions::get().get<bool>( "AUTO_INSERT_LINK" ); }

//___________________________________________________________________________________
void Private::LocalTextEditor::_installActions()
{
    Debug::Throw( "Private::LocalTextEditor::_installActions.\n" );
    addAction( insertLinkAction_ = new QAction( IconEngine::get( IconNames::InsertSymbolicLink ), tr( "Insert Link..." ), this ) );
    addAction( editLinkAction_ = new QAction( IconEngine::get( IconNames::Edit ), tr( "Edit Link..." ), this ) );
    addAction( removeLinkAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Remove Link..." ), this ) );
    addAction( openLinkAction_ = new QAction( IconEngine::get( IconNames::Find ), tr( "Open Link..." ), this ) );

    // disable insert link action by default
    insertLinkAction_->setEnabled( false );
}

//___________________________________________________________________________________
Private::ColorWidget::ColorWidget( QWidget* parent ):
    QToolButton( parent ),
    Counter( "ColorWidget" )
{ Debug::Throw( "ColorWidget::ColorWidget.\n" ); }

//___________________________________________________________________________________
void Private::ColorWidget::setColor( const QColor& color )
{

    // create pixmap
    QPixmap pixmap( IconSize::get( IconSize::Huge ) );
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
QSize Private::ColorWidget::sizeHint() const
{
    // the const_cast is use to temporarily remove the menu
    // in order to keep the size of the toolbutton minimum
    QMenu* menu( ColorWidget::menu() );
    const_cast<Private::ColorWidget*>( this )->setMenu( nullptr );
    QSize out( QToolButton::sizeHint() );
    const_cast<Private::ColorWidget*>( this )->setMenu(menu);
    return out;
}

//___________________________________________________________________________________
QSize Private::ColorWidget::minimumSizeHint() const
{
    // this is an ugly hack to keep the size of the toolbutton minimum
    QMenu* menu( ColorWidget::menu() );
    const_cast<Private::ColorWidget*>( this )->setMenu(0);
    QSize out( QToolButton::minimumSizeHint() );
    const_cast<Private::ColorWidget*>( this )->setMenu(menu);
    return out;
}

//___________________________________________________________________________________
void Private::ColorWidget::paintEvent( QPaintEvent* )
{
    QStylePainter painter(this);
    QStyleOptionToolButton option;
    initStyleOption(&option);

    // first draw normal frame and not text/icon
    option.features &= (~QStyleOptionToolButton::HasMenu);
    painter.drawComplexControl(QStyle::CC_ToolButton, option);

}

#include "moc_EditionWindow_p.cpp"
