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
#include "AttachmentFrame.h"
#include "AttachmentWindow.h"
#include "BaseContextMenu.h"
#include "BaseFindWidget.h"
#include "BaseIconNames.h"
#include "BaseReplaceWidget.h"
#include "BaseStatusBar.h"
#include "ColorMenu.h"
#include "Command.h"
#include "CppUtil.h"
#include "EditionWindow_p.h"
#include "File.h"
#include "FormatBar.h"
#include "HtmlDialog.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "IconSize.h"
#include "InformationDialog.h"
#include "InsertLinkDialog.h"
#include "LogEntry.h"
#include "LogEntryHtmlHelper.h"
#include "LogEntryInformationDialog.h"
#include "LogEntryPrintHelper.h"
#include "LogEntryPrintOptionWidget.h"
#include "Logbook.h"
#include "MainWindow.h"
#include "MenuBar.h"
#include "MessageWidget.h"
#include "OpenWithDialog.h"
#include "Options.h"
#include "PrintPreviewDialog.h"
#include "PrinterOptionWidget.h"
#include "QtUtil.h"
#include "QuestionDialog.h"
#include "RecentFilesMenu.h"
#include "SelectLineWidget.h"
#include "Singleton.h"
#include "ToolBar.h"
#include "ToolBarSpacerItem.h"
#include "Util.h"
#include "moc_EditionWindow_p.cpp"


#if WITH_ASPELL
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
#include <QUrl>

//_______________________________________________
EditionWindow::EditionWindow( QWidget* parent, bool readOnly ):
    BaseMainWindow( parent ),
    Counter( QStringLiteral("EditionWindow") ),
    readOnly_( readOnly )
{
    Debug::Throw(QStringLiteral("EditionWindow::EditionWindow.\n") );
    setOptionName( QStringLiteral("editionWindow") );
    setObjectName( QStringLiteral("EDITFRAME") );

    auto main( new QWidget( this ) );
    setCentralWidget( main );

    auto layout = new QVBoxLayout;
    QtUtil::setMargin(layout, 0);
    layout->setSpacing(0);
    main->setLayout( layout );

    // message widget
    messageWidget_ = new MessageWidget( main );
    layout->addWidget( messageWidget_ );
    messageWidget_->setText( tr( "This entry is open in read-only mode." ) );
    messageWidget_->setDirection( QBoxLayout::LeftToRight );
    {
        auto button = messageWidget_->addButton( IconEngine::get( IconNames::Edit ), tr( "Edit entry" ) );
        connect( button, &QAbstractButton::clicked, this, &EditionWindow::_unlock );
        connect( button, &QAbstractButton::clicked, messageWidget_, &MessageWidget::animatedHide );
    }

    messageWidget_->hide();

    // header layout
    auto gridLayout( new QGridLayout );
    gridLayout->setSpacing(0);
    QtUtil::setMargin(gridLayout, 0);
    layout->addLayout( gridLayout );

    // title label and editor
    gridLayout->addWidget( titleLabel_ = new QLabel( tr( "Subject: " ), main ), 0, 0, 1, 1 );
    gridLayout->addWidget( titleEditor_ = new Editor( main ), 0, 1, 1, 1 );

    titleEditor_->setPlaceholderText( tr( "Entry subject" ) );
    titleLabel_->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    titleLabel_->setBuddy( titleEditor_ );

    // colorWidget
    colorWidget_ = new Private::ColorWidget;
    colorWidget_->setToolTip( tr( "Change entry color.\nThis is used to tag entries in the main window list" ) );
    colorWidget_->setAutoRaise( true );
    colorWidget_->setPopupMode( QToolButton::InstantPopup );
    titleEditor_->addRightWidget(colorWidget_);

    // keywoard label and editor
    gridLayout->addWidget( keywordLabel_ = new QLabel( tr( " Keyword: " ), main ), 1, 0, 1, 1 );
    gridLayout->addWidget( keywordEditor_ = new Editor( main ), 1, 1, 1, 1 );
    QtUtil::setWidgetSides( keywordEditor_, Qt::TopEdge|Qt::LeftEdge);

    keywordEditor_->setPlaceholderText( tr( "Entry keyword" ) );
    keywordLabel_->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    keywordLabel_->setBuddy( keywordEditor_ );

    gridLayout->setColumnStretch( 1, 1 );

    // hide everything
    _setKeywordVisible( false );
    colorWidget_->setVisible( false );

    // splitter for EditionWindow and attachment list
    auto splitter = new QSplitter( main );
    splitter->setOrientation( Qt::Vertical );
    layout->addWidget( splitter, 1 );

    // create text
    auto splitterWidget = new QWidget;
    splitterWidget->setLayout( new QVBoxLayout );
    QtUtil::setMargin(splitterWidget->layout(), 0);
    splitterWidget->layout()->setSpacing(0);
    splitter->addWidget( splitterWidget );

    auto editorContainer = new QWidget( splitterWidget );
    editorContainer->setLayout( new QVBoxLayout );
    QtUtil::setMargin(editorContainer->layout(), 0);
    editorContainer->layout()->setSpacing(0);
    static_cast<QVBoxLayout*>(splitterWidget->layout())->addWidget( editorContainer, 1 );

    // container for embedded widgets
    container_ = new QWidget( splitterWidget );
    container_->setLayout( new QVBoxLayout );
    QtUtil::setMargin(container_->layout(), 0);
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

    connect( splitter, &QSplitter::splitterMoved, this, &EditionWindow::_splitterMoved );
    connect( keywordEditor_, &Editor::modificationChanged, this, &EditionWindow::_textModified );
    connect( keywordEditor_, &Editor::cursorPositionChanged, this, QOverload<>::of( &EditionWindow::_displayCursorPosition ) );
    connect( titleEditor_, &Editor::modificationChanged, this, &EditionWindow::_textModified );
    connect( titleEditor_, &Editor::cursorPositionChanged, this, QOverload<>::of( &EditionWindow::_displayCursorPosition ) );
    connect( activeEditor_->document(), &QTextDocument::modificationChanged, this, &EditionWindow::_textModified );

    // create attachment list
    auto frame = new AttachmentFrame( 0, readOnly_ );
    frame->visibilityAction().setChecked( false );
    frame->setDefaultHeight( XmlOptions::get().get<int>( QStringLiteral("ATTACHMENT_FRAME_HEIGHT") ) );
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

    // main toolbar
    auto toolbar = new ToolBar( tr( "Main Toolbar" ), this, QStringLiteral("MAIN_TOOLBAR") );
    toolbar->addAction( newEntryAction_ );
    toolbar->addAction( saveAction_ );
    toolbar->addAction( deleteEntryAction_ );
    readOnlyActions_.append( deleteEntryAction_ );
    toolbar->addAction( &frame->newAction() );

    // format bar
    formatBar_ = new FormatBar( this, QStringLiteral("FORMAT_TOOLBAR") );
    formatBar_->setTarget( editor );
    formatBar_->addAction( insertLinkAction_ );
    readOnlyActions_.append( insertLinkAction_ );

    const auto& actions( formatBar_->actions() );
    std::copy( actions.begin(), actions.end(), std::back_inserter( readOnlyActions_ ) );

    // set proper connection for first editor
    // (because it could not be performed in _newTextEditor)
    connect(
        &editor, &QTextEdit::currentCharFormatChanged,
        formatBar_, &FormatBar::updateState );

    // edition toolbars
    toolbar = new ToolBar( tr( "History" ), this, QStringLiteral("EDITION_TOOLBAR") );
    toolbar->addAction( undoAction_ );
    toolbar->addAction( redoAction_ );

    readOnlyActions_.append( { undoAction_, redoAction_ } );

    // undo/redo connections
    connect( keywordEditor_, &Editor::textChanged, this, [this](const QString&){ _updateUndoRedoActions(); } );
    connect( keywordEditor_, &Editor::textChanged, this, &EditionWindow::_updateSaveAction );
    connect( titleEditor_, &Editor::textChanged, this, [this](const QString&){ _updateUndoRedoActions(); } );
    connect( titleEditor_, &Editor::textChanged, this, &EditionWindow::_updateSaveAction );
    connect( qApp, &QApplication::focusChanged, this, [this](QWidget*,QWidget*){ _updateUndoRedoActions(); } );

    // extra toolbar
    toolbar = new ToolBar( tr( "Tools" ), this, QStringLiteral("EXTRA_TOOLBAR") );

    #if WITH_ASPELL
    toolbar->addAction( spellcheckAction_ );
    #endif

    toolbar->addAction( printAction_ );
    toolbar->addAction( entryInformationAction_ );

    // extra toolbar
    toolbar = new ToolBar( tr( "Multiple Views" ), this, QStringLiteral("MULTIPLE_VIEW_TOOLBAR") );
    toolbar->addAction( splitViewHorizontalAction_ );
    toolbar->addAction( splitViewVerticalAction_ );
    toolbar->addAction( cloneWindowAction_ );
    toolbar->addAction( closeAction_ );

    // extra toolbar
    toolbar = new ToolBar( tr( "Navigation" ), this, QStringLiteral("NAVIGATION_TOOLBAR") );
    toolbar->addAction( &application->mainWindow().uniconifyAction() );
    toolbar->addAction( previousEntryAction_ );
    toolbar->addAction( nextEntryAction_ );

    // application menu
    toolbar =  new ToolBar( tr( "Application Menu" ), this, QStringLiteral("APPMENU_TOOLBAR") );
    toolbar->addWidget( new ToolBarSpacerItem );
    addApplicationMenu( toolbar );

    // create menu if requested
    menuBar_ = new MenuBar( this, &application->mainWindow() );
    setMenuBar( menuBar_ );

    // changes display according to readOnly flag
    setReadOnly( readOnly_ );

    // update modifiers
    _modifiersChanged( activeEditor_->modifiers() );

    // configuration
    connect( application, &Application::configurationChanged, this, &EditionWindow::_updateConfiguration );
    _updateConfiguration();

}

//____________________________________________
void EditionWindow::displayEntry( const Keyword &keyword, LogEntry *entry )
{

    Debug::Throw( QStringLiteral("EditionWindow::displayEntry.\n") );
    keyword_ =  keyword;
    displayEntry( entry );

}

//____________________________________________
void EditionWindow::displayEntry( LogEntry *entry )
{
    Debug::Throw( QStringLiteral("EditionWindow::displayEntry.\n") );

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
    Debug::Throw( QStringLiteral("EditionWindow::displayEntry - setting button states.\n") );
    previousEntryAction_->setEnabled( mainWindow.previousEntry(entry, false) );
    nextEntryAction_->setEnabled( mainWindow.nextEntry(entry, false) );

    // reset modify flag; change title accordingly
    setModified( false );

    _updateReadOnlyActions();
    _updateSaveAction();
    updateWindowTitle();

    Debug::Throw( QStringLiteral("EditionWindow::displayEntry - done.\n") );
    return;
}

//____________________________________________
TextEditor& EditionWindow::activeEditor()
{ return *activeEditor_; }

//____________________________________________
const TextEditor& EditionWindow::activeEditor() const
{ return *activeEditor_; }

//_____________________________________________
bool EditionWindow::modified() const
{
    return
        keywordEditor_->isModified() ||
        titleEditor_->isModified() ||
        activeEditor_->document()->isModified();
}

//_____________________________________________
QString EditionWindow::windowTitle() const
{

    Debug::Throw( QStringLiteral("EditionWindow::windowTitle.\n") );
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

//____________________________________________
void EditionWindow::setReadOnly( bool value )
{
    readOnly_ = value;
    if( readOnly_ ) messageWidget_->animatedShow();
    else messageWidget_->animatedHide();
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

        // remove all editors but one
        for( auto&& editor:Base::KeySet< TextEditor >(this) )
        {
            if( editor != activeEditor_ )
            { _closeEditor( *editor ); }
        }

        // reset editor's font
        activeEditor_->setFont( qApp->font( activeEditor_ ) );

    }
}

//_____________________________________________
void EditionWindow::displayKeyword()
{
    Debug::Throw( QStringLiteral("EditionWindow::displayKeyword.\n") );

    keywordEditor_->setText( keyword_.get() );
    keywordEditor_->setCursorPosition( 0 );
    return;

}

//_____________________________________________
void EditionWindow::displayTitle()
{
    Debug::Throw( QStringLiteral("EditionWindow::displayTitle.\n") );

    auto entry( this->entry() );
    if( entry ) titleEditor_->setText( entry->title() );
    titleEditor_->setCursorPosition( 0 );
    return;
}

//_____________________________________________
void EditionWindow::displayColor()
{
    Debug::Throw( QStringLiteral("EditionWindow::DisplayColor.\n") );

    // try load entry color
    const QColor color( entry()->color() );
    if( !color.isValid() )
    {
        colorWidget_->setVisible( false );

    } else {

        colorWidget_->setColor( color );
        colorWidget_->setVisible( true );

    }

    return;

}

//______________________________________________________
void EditionWindow::setModified( bool value )
{
    Debug::Throw( QStringLiteral("EditionWindow::setModified.\n") );
    keywordEditor_->setModified( value );
    titleEditor_->setModified( value );
    activeEditor_->document()->setModified( value );
}

//___________________________________________________________
void EditionWindow::setForceShowKeyword( bool value )
{
    Debug::Throw( QStringLiteral("EditionWindow::setForceShowKeyword.\n") );
    _setKeywordVisible( value || showKeywordAction_->isChecked() );
    showKeywordAction_->setEnabled( !value );
}

//___________________________________________________________
void EditionWindow::_closeEditor( TextEditor& editor )
{
    Debug::Throw( QStringLiteral("EditionWindow::_closeEditor.\n") );

    // retrieve number of editors
    // if only one display, close the entire window
    Base::KeySet<TextEditor> editors( this );
    if( editors.size() < 2 )
    {
        Debug::Throw() << "EditionWindow::_closeEditor - full close." << Qt::endl;
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
        Debug::Throw( QStringLiteral("EditionWindow::_closeEditor - found child.\n") );

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
        Debug::Throw( QStringLiteral("EditionWindow::_closeEditor - deleted splitter.\n") );

    } else {

        // the editor is deleted only if its parent splitter is not
        // otherwise this will trigger double deletion of the editor
        // which will then crash
        editor.deleteLater();

    }

    // update activeEditor
    TextEditor* active = nullptr;
    for( const auto& current:editors.get() )
    { if( current != &editor ) active = current; }
    if( active ) setActiveEditor( *active );

    // change focus
    activeEditor_->setFocus();
    Debug::Throw( QStringLiteral("EditionWindow::_closeEditor - done.\n") );

}

//________________________________________________________________
void EditionWindow::setActiveEditor( TextEditor& editor )
{
    Debug::Throw() << "EditionWindow::setActiveEditor - key: " << editor.key() << Qt::endl;
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

    Debug::Throw( QStringLiteral("EditionWindow::askForSave.\n") );

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
    entry->setAuthor( XmlOptions::get().raw( QStringLiteral("USER") ) );

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

    Debug::Throw( QStringLiteral("EditionWindow::updateReadOnlyState\n") );
    _updateReadOnlyActions();
    _updateSaveAction();
    _updateUndoRedoActions();
    _updateInsertLinkActions();
    updateWindowTitle();

}

//_____________________________________________________________________
void EditionWindow::findFromDialog()
{
    Debug::Throw( QStringLiteral("EditionWindow::findFromDialog.\n") );

    // create find widget
    if( !findWidget_ ) _createFindWidget();
    findWidget_->show();
    findWidget_->editor().setFocus();
    activeEditor_->ensureCursorVisible();

    /*
    setting the default text values
    must be done after the dialog is shown
    otherwise it may be automatically resized
    to very large sizes due to the input text
    */
    QString text( activeEditor_->selection().text() );
    if( !text.isEmpty() )
    {
        const int maxLength( 1024 );
        text.truncate( maxLength );
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
    Debug::Throw( QStringLiteral("EditionWindow::replaceFromDialog.\n") );

    // create replace widget
    if( !replaceWidget_ ) _createReplaceWidget();

    // show replace widget and set focus
    replaceWidget_->show();
    replaceWidget_->editor().setFocus();
    activeEditor_->ensureCursorVisible();

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
    else if( activeEditor_->textCursor().hasSelection() ) replaceWidget_->setText( activeEditor_->textCursor().selectedText() );
    else if( !( text = TextEditor::lastSelection().text() ).isEmpty() ) replaceWidget_->setText( text );

    // update replace text
    if( !TextEditor::lastSelection().replaceText().isEmpty() ) replaceWidget_->setReplaceText( TextEditor::lastSelection().replaceText() );

    return;
}

//________________________________________________
void EditionWindow::selectLineFromDialog()
{

    Debug::Throw( QStringLiteral("TextEditor::selectLineFromDialog.\n") );

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
    Debug::Throw( QStringLiteral("EditionWindow::closeEvent.\n") );

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
        { XmlOptions::get().set<int>( QStringLiteral("ATTACHMENT_FRAME_HEIGHT"), attachmentFrame().height() ); }

    } else return BaseMainWindow::timerEvent( event );

}

//_____________________________________________
void EditionWindow::_installActions()
{
    Debug::Throw( QStringLiteral("EditionWindow::_installActions.\n") );

    // undo action
    addAction( undoAction_ = new QAction( IconEngine::get( IconNames::Undo ), tr( "Undo" ), this ) );
    undoAction_->setToolTip( tr( "Undo last modification" ) );
    connect( undoAction_, &QAction::triggered, this, &EditionWindow::_undo );

    // redo action
    addAction( redoAction_ = new QAction( IconEngine::get( IconNames::Redo ), tr( "Redo" ), this ) );
    redoAction_->setToolTip( tr( "Redo last undone modification" ) );
    connect( redoAction_, &QAction::triggered, this, &EditionWindow::_redo );

    // new entry
    addAction( newEntryAction_ = new QAction( IconEngine::get( IconNames::New ), tr( "New Entry" ), this ) );
    newEntryAction_->setIconText( tr( "New" ) );
    newEntryAction_->setShortcut( QKeySequence::New );
    newEntryAction_->setToolTip( tr( "Create new entry in current editor" ) );
    connect( newEntryAction_, &QAction::triggered, this, &EditionWindow::_newEntry );

    // previous entry action
    addAction( previousEntryAction_ = new QAction( IconEngine::get( IconNames::PreviousEntry ), tr( "Previous Entry" ), this ) );
    previousEntryAction_->setIconText( tr( "Previous" ) );
    previousEntryAction_->setToolTip( tr( "Display previous entry in current list" ) );
    connect( previousEntryAction_, &QAction::triggered, this, &EditionWindow::_previousEntry );

    // next entry action
    addAction( nextEntryAction_ = new QAction( IconEngine::get( IconNames::NextEntry ), tr( "Next Entry" ), this ) );
    nextEntryAction_->setIconText( tr( "Next" ) );
    nextEntryAction_->setToolTip( tr( "Display next entry in current list" ) );
    connect( nextEntryAction_, &QAction::triggered, this, &EditionWindow::_nextEntry );

    // save
    addAction( saveAction_ = new QAction( IconEngine::get( IconNames::Save ), tr( "Save Entry" ), this ) );
    saveAction_->setIconText( tr( "Save" ) );
    saveAction_->setToolTip( tr( "Save current entry" ) );
    connect( saveAction_, &QAction::triggered, this, &EditionWindow::_save );
    saveAction_->setShortcut( QKeySequence::Save );

    #if WITH_ASPELL
    addAction( spellcheckAction_ = new QAction( IconEngine::get( IconNames::SpellCheck ), tr( "Check Spelling..." ), this ) );
    spellcheckAction_->setToolTip( tr( "Check spelling of current entry" ) );
    connect( spellcheckAction_, &QAction::triggered, this, &EditionWindow::_spellCheck );
    spellcheckAction_->setEnabled( !SpellCheck::SpellInterface().dictionaries().empty() );
    #endif

    // entry information
    addAction( entryInformationAction_ = new QAction( IconEngine::get( IconNames::Information ), tr( "Entry Properties..." ), this ) );
    entryInformationAction_->setIconText( tr( "Properties" ) );
    entryInformationAction_->setToolTip( tr( "Show current entry properties" ) );
    connect( entryInformationAction_, &QAction::triggered, this, &EditionWindow::_entryInformation );

    // reload
    addAction( reloadAction_ = new QAction( IconEngine::get( IconNames::Reload ), tr( "Reload Entry" ), this ) );
    reloadAction_->setToolTip( tr( "ReloadEntry" ) );
    reloadAction_->setShortcut( QKeySequence::Refresh );
    connect( reloadAction_, &QAction::triggered, this, &EditionWindow::_reloadEntry );

    // print
    addAction( printAction_ = new QAction( IconEngine::get( IconNames::Print ), tr( "Print..." ), this ) );
    printAction_->setToolTip( tr( "Print current logbook entry" ) );
    printAction_->setShortcut( QKeySequence::Print );
    connect( printAction_, &QAction::triggered, this, QOverload<>::of( &EditionWindow::_print ) );

    // print preview
    addAction( printPreviewAction_ = new QAction( IconEngine::get( IconNames::PrintPreview ), tr( "Print Preview..." ), this ) );
    connect( printPreviewAction_, &QAction::triggered, this, &EditionWindow::_printPreview );

    // print
    addAction( htmlAction_ = new QAction( IconEngine::get( IconNames::Html ), tr( "Export to HTML..." ), this ) );
    htmlAction_->setToolTip( tr( "Export current logbook entry to HTML" ) );
    connect( htmlAction_, &QAction::triggered, this, &EditionWindow::_toHtml );

    // split action
    addAction( splitViewHorizontalAction_ =new QAction( IconEngine::get( IconNames::ViewTopBottom ), tr( "Split View Top/Bottom" ), this ) );
    splitViewHorizontalAction_->setToolTip( tr( "Split current text editor vertically" ) );
    connect( splitViewHorizontalAction_, &QAction::triggered, this, &EditionWindow::_splitViewVertical );

    addAction( splitViewVerticalAction_ =new QAction( IconEngine::get( IconNames::ViewLeftRight ), tr( "Split View Left/Right" ), this ) );
    splitViewVerticalAction_->setToolTip( tr( "Split current text editor horizontally" ) );
    connect( splitViewVerticalAction_, &QAction::triggered, this, &EditionWindow::_splitViewHorizontal );

    // clone window action
    addAction( cloneWindowAction_ = new QAction( IconEngine::get( IconNames::ViewClone ), tr( "Clone Window" ), this ) );
    cloneWindowAction_->setToolTip( tr( "Create a new edition window displaying the same entry" ) );
    connect( cloneWindowAction_, &QAction::triggered, this, &EditionWindow::_cloneWindow );

    // close window action
    addAction( closeAction_ = new QAction( IconEngine::get( IconNames::ViewRemove ), tr( "Close View" ), this ) );
    closeAction_->setShortcut( QKeySequence::Close );
    closeAction_->setToolTip( tr( "Close current view" ) );
    connect( closeAction_, &QAction::triggered, this, &EditionWindow::_close );

    addAction( deleteEntryAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete Entry" ), this ) );
    deleteEntryAction_->setIconText( tr( "Delete" ) );
    connect( deleteEntryAction_, &QAction::triggered, this, &EditionWindow::_deleteEntry );

    // uniconify
    addAction( uniconifyAction_ = new QAction( tr( "Uniconify" ), this ) );
    connect( uniconifyAction_, &QAction::triggered, this, &BaseMainWindow::uniconify );

    // show/hide keyword
    addAction( showKeywordAction_ = new QAction( tr( "Show Keyword" ), this ) );
    showKeywordAction_->setCheckable( true );
    showKeywordAction_->setChecked( false );
    connect( showKeywordAction_, &QAction::toggled, this, &EditionWindow::_toggleShowKeyword );

    // insert link
    addAction( insertLinkAction_ = new QAction( IconEngine::get( IconNames::InsertSymbolicLink ), tr( "Insert Link" ), this ) );
    connect( insertLinkAction_, &QAction::triggered, this, &EditionWindow::_insertLink );
    insertLinkAction_->setEnabled( false );
}


//______________________________________________________________________
void EditionWindow::_createFindWidget()
{

    Debug::Throw( QStringLiteral("EditionWindow::_createFindWidget.\n") );
    if( !findWidget_ )
    {

        findWidget_ = new BaseFindWidget( container_ );
        findWidget_->enableHighlightAll(false);
        container_->layout()->addWidget( findWidget_ );
        connect( findWidget_, &BaseFindWidget::find, this, &EditionWindow::_find );
        connect( this, &EditionWindow::matchFound, findWidget_, &BaseFindWidget::matchFound );
        connect( this, &EditionWindow::noMatchFound, findWidget_, &BaseFindWidget::noMatchFound );
        connect( &findWidget_->closeButton(), &QAbstractButton::clicked, this, &EditionWindow::_restoreFocus );
        findWidget_->hide();

    }

    return;

}

//_____________________________________________________________________
void EditionWindow::_createReplaceWidget()
{
    Debug::Throw( QStringLiteral("EditionWindow::_CreateReplaceDialog.\n") );
    if( !( replaceWidget_ ) )
    {

        replaceWidget_ = new BaseReplaceWidget( container_ );
        container_->layout()->addWidget( replaceWidget_ );
        connect( replaceWidget_, &BaseReplaceWidget::find, this, &EditionWindow::_find );
        connect( replaceWidget_, &BaseReplaceWidget::replace, this, &EditionWindow::_replace );
        connect( replaceWidget_, &BaseReplaceWidget::replaceInWindow, this, &EditionWindow::_replaceInWindow );
        connect( replaceWidget_, &BaseReplaceWidget::replaceInSelection, this, &EditionWindow::_replaceInSelection );
        connect( replaceWidget_, &BaseReplaceWidget::menuAboutToShow, this, &EditionWindow::_updateReplaceInSelection );
        connect( &replaceWidget_->closeButton(), &QAbstractButton::clicked, this, &EditionWindow::_restoreFocus );
        replaceWidget_->hide();

        connect( this, &EditionWindow::matchFound, replaceWidget_, &BaseReplaceWidget::matchFound );
        connect( this, &EditionWindow::noMatchFound, replaceWidget_, &BaseReplaceWidget::noMatchFound );

    }

}

//_________________________________________________________________
void EditionWindow::_createSelectLineWidget()
{
    if( !selectLineWidget_ )
    {
        selectLineWidget_ = new SelectLineWidget( this, true );
        container_->layout()->addWidget( selectLineWidget_ );
        connect( selectLineWidget_, &SelectLineWidget::lineSelected, this, &EditionWindow::_selectLine );
        connect( this, &EditionWindow::lineFound, selectLineWidget_, &SelectLineWidget::matchFound );
        connect( this, &EditionWindow::lineNotFound, selectLineWidget_, &SelectLineWidget::noMatchFound );
        connect( &selectLineWidget_->closeButton(), &QAbstractButton::clicked, this, &EditionWindow::_restoreFocus );
        selectLineWidget_->hide();
    }
}

//___________________________________________________________
Private::LocalTextEditor& EditionWindow::_splitView( Qt::Orientation orientation )
{
    Debug::Throw( QStringLiteral("EditionWindow::_splitView.\n") );

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

    // synchronize both editors
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
QSplitter& EditionWindow::_newSplitter( Qt::Orientation orientation )
{

    Debug::Throw( QStringLiteral("EditionWindow::_newSplitter.\n") );
    QSplitter *splitter = nullptr;

    // retrieve parent of current display
    auto parent( activeEditor_->parentWidget() );

    // try cast to splitter
    // do not create a new splitter if the parent has same orientation
    auto parentSplitter( qobject_cast<QSplitter*>( parent ) );
    if( parentSplitter && parentSplitter->orientation() == orientation ) {

        Debug::Throw( QStringLiteral("EditionWindow::_newSplitter - orientation match. No need to create new splitter.\n") );
        splitter = parentSplitter;

    } else {


        // move splitter to the first place if needed
        if( parentSplitter )
        {

            Debug::Throw( QStringLiteral("EditionWindow::_newSplitter - found parent splitter with incorrect orientation.\n") );
            // create a splitter with correct orientation
            // give him no parent, because the parent is set in QSplitter::insertWidget()
            splitter = new Private::LocalSplitter( nullptr );
            splitter->setOrientation( orientation );
            parentSplitter->insertWidget( parentSplitter->indexOf( activeEditor_ ), splitter );

        } else {

            Debug::Throw( QStringLiteral("EditionWindow::_newSplitter - no splitter found. Creating a new one.\n") );

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
    Debug::Throw( QStringLiteral("EditionWindow::_newTextEditor.\n") );

    // create TextEditor
    auto editor = new Private::LocalTextEditor( parent );
    QtUtil::setWidgetSides(editor, Qt::TopEdge);

    // connections
    connect( &editor->insertLinkAction(), &QAction::triggered, this, &EditionWindow::_insertLink );
    connect( &editor->editLinkAction(), &QAction::triggered, this, &EditionWindow::_editLink );
    connect( &editor->removeLinkAction(), &QAction::triggered, this, &EditionWindow::_removeLink );
    connect( &editor->openLinkAction(), &QAction::triggered, this, QOverload<>::of( &EditionWindow::_openLink) );
    connect( editor, &Private::LocalTextEditor::linkActivated, this, QOverload<const QString&>::of( &EditionWindow::_openLink ) );
    connect( editor, &Private::LocalTextEditor::hasFocus, this, &EditionWindow::_displayFocusChanged );
    connect( editor, &Private::LocalTextEditor::cursorPositionChanged, this, QOverload<>::of( &EditionWindow::_displayCursorPosition ) );
    connect( editor, &TextEditor::modifiersChanged, this, &EditionWindow::_modifiersChanged );
    connect( editor, &Private::LocalTextEditor::undoAvailable, [this](bool){ _updateUndoRedoActions(); } );
    connect( editor, &QTextEdit::selectionChanged, this, &EditionWindow::_updateInsertLinkActions );
    connect( editor->document(), &QTextDocument::modificationChanged, this, &EditionWindow::_updateSaveAction );

    // customize display actions
    /* this is needed to be able to handle a single dialog for stacked windows */
    // goto line number
    editor->gotoLineAction().disconnect();
    connect( &editor->gotoLineAction(), &QAction::triggered, this, &EditionWindow::selectLineFromDialog );

    // find
    editor->findAction().disconnect();
    connect( &editor->findAction(), &QAction::triggered, this, &EditionWindow::findFromDialog );
    connect( editor, &TextEditor::noMatchFound, this, &EditionWindow::noMatchFound );
    connect( editor, &TextEditor::matchFound, this, &EditionWindow::matchFound );
    connect( editor, &TextEditor::lineNotFound, this, &EditionWindow::lineNotFound );
    connect( editor, &TextEditor::lineFound, this, &EditionWindow::lineFound );

    // replace
    editor->replaceAction().disconnect();
    connect( &editor->replaceAction(), &QAction::triggered, this, &EditionWindow::replaceFromDialog );

    if( formatBar_ )
    {
        connect(
            editor, &QTextEdit::currentCharFormatChanged,
            formatBar_, &FormatBar::updateState );
    }

    // associate display to this editFrame
    Base::Key::associate( this, editor );

    // update current display and focus
    setActiveEditor( *editor );
    editor->setFocus();
    Debug::Throw() << "EditionWindow::_newTextEditor - key: " << editor->key() << Qt::endl;
    Debug::Throw( QStringLiteral("EditionWindow::_newTextEditor - done.\n") );

    // update insert Link actions
    _updateInsertLinkActions();

    return *editor;

}

//_____________________________________________
void EditionWindow::_displayCursorPosition( const TextPosition& position)
{
    Debug::Throw( QStringLiteral("EditionWindow::_DisplayCursorPosition.\n") );
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
    Debug::Throw( QStringLiteral("EditionWindow::_mainWindow.\n") );
    Base::KeySet<MainWindow> mainWindows( this );
    return **mainWindows.begin();
}

//_____________________________________________
void EditionWindow::_displayText()
{
    Debug::Throw( QStringLiteral("EditionWindow::_displayText.\n") );
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
    Debug::Throw( QStringLiteral("EditionWindow::_DisplayAttachments.\n") );

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
        frame.add( Base::makeT<AttachmentModel::List>( attachments ) );
    }
}

//_____________________________________________
void EditionWindow::_setKeywordVisible( bool value )
{
    Debug::Throw( QStringLiteral("EditionWindow::_setKeywordVisible.\n") );
    keywordLabel_->setVisible( value );
    keywordEditor_->setVisible( value );
    titleLabel_->setVisible( value );

    // get widget sides from properties
    Qt::Edges borders( QtUtil::widgetSides(titleEditor_) );

    // adjust sides
    borders.setFlag(Qt::LeftEdge, value );
    QtUtil::setWidgetSides( titleEditor_, borders );
}

//_____________________________________________
void EditionWindow::_save( bool updateSelection )
{

    Debug::Throw( QStringLiteral("EditionWindow::_save.\n") );

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
void EditionWindow::_reloadEntry()
{
    Debug::Throw( QStringLiteral("EditionWindow::_reloadEntry.\n") );
    auto entry( this->entry() );
    if( !entry )
    {
        InformationDialog( this, tr( "No valid entry opened. <Reload> canceled." ) ).exec();
        return;
    }

    if( modified() && !QuestionDialog( this, tr( "Discard changes to this entry ?" ) ).exec() )
    { return; }

    displayEntry( entry );

}

//___________________________________________________________
void EditionWindow::_print()
{
    Debug::Throw( QStringLiteral("EditionWindow::_print.\n") );

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

    connect( optionWidget, &PrinterOptionWidget::orientationChanged, &helper, &BasePrintHelper::setOrientation );
    connect( optionWidget, &PrinterOptionWidget::pageModeChanged, &helper, &BasePrintHelper::setPageMode );

    LogEntryPrintOptionWidget* logEntryOptionWidget = new LogEntryPrintOptionWidget;
    logEntryOptionWidget->setWindowTitle( tr( "Logbook Entry Configuration" ) );
    connect( logEntryOptionWidget, &LogEntryPrintOptionWidget::maskChanged, &helper, &LogEntryPrintHelper::setMask );
    logEntryOptionWidget->read( XmlOptions::get() );

    // create prind dialog and run.
    QPrintDialog dialog( &printer, this );
    dialog.setWindowTitle( Util::windowTitle( tr( "Print Logbook Entry" ) ) );

    dialog.setOptionTabs( { optionWidget, logEntryOptionWidget } );
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
    Debug::Throw( QStringLiteral("EditionWindow::_printPreview.\n") );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create helper
    LogEntryPrintHelper helper( this );
    helper.setEntry( entry() );
    helper.setMask( (LogEntry::Mask) XmlOptions::get().get<int>( QStringLiteral("LOGENTRY_PRINT_OPTION_MASK") ) );

    // create dialog, connect and execute
    PrintPreviewDialog dialog( this, Dialog::OkButton|Dialog::CancelButton );
    dialog.setWindowTitle( tr( "Print Preview" ) );
    dialog.setHelper( &helper );

    // print
    if( dialog.exec() ) _print( helper );

}

//___________________________________________________________
void EditionWindow::_toHtml()
{
    Debug::Throw( QStringLiteral("EditionWindow::_toHtml.\n") );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create option widget
    LogEntryPrintOptionWidget* optionWidget = new LogEntryPrintOptionWidget;
    optionWidget->read( XmlOptions::get() );

    // create dialog
    HtmlDialog dialog( this );

    dialog.setOptionWidgets( { optionWidget } );
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
    { ( Base::Command( command ) << file ).run(); }

}

//_____________________________________________
void EditionWindow::_newEntry()
{

    Debug::Throw( QStringLiteral("EditionWindow::_newEntry.\n") );

    // check if entry is modified
    if( modified() && askForSave() == AskForSaveDialog::Cancel ) return;

    // create new entry, set author, set keyword
    auto entry = new LogEntry;
    entry->setAuthor( XmlOptions::get().raw( QStringLiteral("USER") ) );
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
    Debug::Throw( QStringLiteral("MainWindow::_splitterMoved.\n") );
    resizeTimer_.start( 200, this );
}

//_______________________________________________
void EditionWindow::_previousEntry()
{
    Debug::Throw( QStringLiteral("EditionWindow::_previousEntry.\n") );

    MainWindow &mainWindow( _mainWindow() );
    auto entry( mainWindow.previousEntry( this->entry(), true ) );
    if( !( entry  && mainWindow.lockEntry( entry, this ) ) ) return;
    displayEntry( entry );
    setReadOnly( false );

}

//_______________________________________________
void EditionWindow::_nextEntry()
{
    Debug::Throw( QStringLiteral("EditionWindow::_nextEntry.\n") );

    MainWindow &mainWindow( _mainWindow() );
    auto entry( mainWindow.nextEntry( this->entry(), true ) );
    if( !( entry && mainWindow.lockEntry( entry, this ) ) ) return;
    displayEntry( entry );
    setReadOnly( false );

}

//_____________________________________________
void EditionWindow::_entryInformation()
{

    Debug::Throw( QStringLiteral("EditionWindow::_EntryInfo.\n") );

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
    Debug::Throw( QStringLiteral("EditionWindow::_undo.\n") );
    if( activeEditor_->QWidget::hasFocus() ) activeEditor_->document()->undo();
    else if( titleEditor_->hasFocus() ) titleEditor_->undo();
    else if( keywordEditor_->hasFocus() ) keywordEditor_->undo();
    return;
}

//_____________________________________________
void EditionWindow::_redo()
{
    Debug::Throw( QStringLiteral("EditionWindow::_redo.\n") );
    if( activeEditor_->QWidget::hasFocus() ) activeEditor_->document()->redo();
    else if( titleEditor_->hasFocus() ) titleEditor_->redo();
    else if( keywordEditor_->hasFocus() ) keywordEditor_->redo();
    return;
}

//_____________________________________________
void EditionWindow::_insertLink()
{
    Debug::Throw( QStringLiteral("EditionWindow::_insertLink.\n") );

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
    Debug::Throw( QStringLiteral("EditionWindow::_editLink.\n") );
    auto cursor( activeEditor_->cursorAtContextMenu() );
    auto block( cursor.block() );

    // loop over text fragments and find the one that matches cursor
    for( auto&& it = block.begin(); !(it.atEnd()); ++it)
    {
        auto fragment = it.fragment();
        if( !fragment.isValid() ) continue;
        if( fragment.position() > cursor.position() || fragment.position() + fragment.length() <= cursor.position() )
        { continue; }

        auto anchor( fragment.charFormat().anchorHref() );
        if( anchor.isEmpty() ) continue;

        // select the corresponding block
        auto cursor( activeEditor_->textCursor() );
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
    Debug::Throw( QStringLiteral("EditionWindow::_removeLink.\n") );
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
    Debug::Throw( QStringLiteral("EditionWindow::_openLink.\n") );
    QString anchor( activeEditor_->anchor() );
    if( anchor.isEmpty() ) return;

    OpenWithDialog dialog( this );
    dialog.setWindowTitle( tr( "Open Link" ) );
    dialog.setLink( File( anchor ) );
    dialog.setOptionName( QStringLiteral("OPEN_LINK_APPLICATIONS") );
    dialog.realizeWidget();
    dialog.exec();

}

//_____________________________________________
void EditionWindow::_openLink( const QString &anchor )
{
    Debug::Throw( QStringLiteral("EditionWindow::_openLink.\n") );
    if( !anchor.isEmpty() ) QDesktopServices::openUrl( QUrl::fromEncoded( anchor.toLatin1() ) );
}

//_____________________________________________
void EditionWindow::_deleteEntry()
{

    Debug::Throw( QStringLiteral("EditionWindow::_deleteEntry.\n") );

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
    #if WITH_ASPELL
    Debug::Throw( QStringLiteral("EditionWindow::_spellCheck.\n") );

    // create dialog
    SpellCheck::SpellDialog dialog( activeEditor_ );

    // set dictionary and filter
    dialog.setFilter( XmlOptions::get().raw(QStringLiteral("DICTIONARY_FILTER")) );
    dialog.setDictionary( XmlOptions::get().raw(QStringLiteral("DICTIONARY")) );
    dialog.nextWord();
    dialog.exec();

    // update dictionary and filter from dialog
    XmlOptions::get().setRaw( QStringLiteral("DICTIONARY_FILTER"), dialog.interface().filter() );
    XmlOptions::get().setRaw( QStringLiteral("DICTIONARY"), dialog.interface().dictionary() );

    #endif
}


//_____________________________________________
void EditionWindow::_cloneWindow()
{

    Debug::Throw( QStringLiteral("EditionWindow::_cloneWindow.\n") );
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
void EditionWindow::_find( const TextSelection &selection )
{ activeEditor_->find( selection ); }

//_____________________________________________
void EditionWindow::_replace( const TextSelection &selection )
{ activeEditor_->replace( selection ); }

//_____________________________________________
void EditionWindow::_replaceInSelection( const TextSelection &selection )
{ activeEditor_->replaceInSelection( selection ); }

//_____________________________________________
void EditionWindow::_replaceInWindow( const TextSelection &selection )
{ activeEditor_->replaceInWindow( selection ); }

//_____________________________________________
void EditionWindow::_selectLine( int value )
{ activeEditor_->selectLine( value ); }

//_____________________________________________
void EditionWindow::_restoreFocus()
{ activeEditor_->setFocus(); }

//_____________________________________________
void EditionWindow::_unlock()
{

    Debug::Throw( QStringLiteral("EditionWindow::_unlock.\n") );

    if( !readOnly_ ) return;
    auto entry( this->entry() );

    if( entry && ! _mainWindow().lockEntry( entry, this ) ) return;
    setReadOnly( false );

    return;

}

//_____________________________________________
void EditionWindow::_updateReplaceInSelection()
{ if( replaceWidget_ ) replaceWidget_->enableReplaceInSelection( activeEditor_->hasSelection() ); }

//_____________________________________________
void EditionWindow::_updateReadOnlyActions()
{

    Debug::Throw( QStringLiteral("EditionWindow::_updateReadOnlyActions.\n") );

    // add flag from logbook read only
    const bool logbookReadOnly( _hasMainWindow() && _mainWindow().logbookIsReadOnly() );
    const bool readOnly( readOnly_ || logbookReadOnly );

    // changes button state
    for( const auto& action:readOnlyActions_ )
    { action->setEnabled( !readOnly ); }

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

    #if WITH_ASPELL
    spellcheckAction_->setEnabled( !( readOnly || SpellCheck::SpellInterface().dictionaries().empty() ) );
    #endif

}

//_____________________________________________
void EditionWindow::_updateSaveAction()
{ saveAction_->setEnabled( !readOnly_ && !( _hasMainWindow() && _mainWindow().logbookIsReadOnly() ) && modified() ); }

//_____________________________________________
void EditionWindow::_updateUndoRedoActions()
{

    Debug::Throw( QStringLiteral("EditionWindow::_updateRedoAction.\n") );
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
    Debug::Throw( QStringLiteral("EditionWindow::_updateUndoRedoAction.\n") );
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
    Debug::Throw( QStringLiteral("EditionWindow::_updateInsertLinkActions.\n") );
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
    Debug::Throw() << "EditionWindow::_textModified - state: " << (state ? "true":"false" ) << Qt::endl;

    // check readonly status
    if( readOnly_ ) return;
    updateWindowTitle();
}

//_____________________________________________
void EditionWindow::_displayCursorPosition()
{ _displayCursorPosition( activeEditor_->textPosition() ); }

//_____________________________________________
void EditionWindow::_close()
{
    Debug::Throw( QStringLiteral("EditionWindow::_close\n") );
    Base::KeySet< Private::LocalTextEditor > editors( this );
    if( editors.size() > 1 ) _closeEditor( *activeEditor_ );
    else close();
}

//_____________________________________________
void EditionWindow::_displayFocusChanged( TextEditor* editor )
{

    Debug::Throw() << "EditionWindow::_DisplayFocusChanged - " << editor->key() << Qt::endl;
    setActiveEditor( *static_cast<Private::LocalTextEditor*>(editor) );

}

//________________________________________________________________
void EditionWindow::_modifiersChanged( TextEditor::Modifiers modifiers )
{
    if( !_hasStatusBar() ) return;
    QStringList buffer;
    if( modifiers & TextEditor::Modifier::Wrap ) buffer.append( QStringLiteral("WRAP") );
    if( modifiers & TextEditor::Modifier::Insert ) buffer.append( QStringLiteral("INS") );
    if( modifiers & TextEditor::Modifier::CapsLock ) buffer.append( QStringLiteral("CAPS") );
    if( modifiers & TextEditor::Modifier::NumLock ) buffer.append( QStringLiteral("NUM") );
    statusBar_->label(1).setText( buffer.join( QStringLiteral(" ") ) );
}

//________________________________________________________________
void EditionWindow::_toggleShowKeyword( bool value )
{

    Debug::Throw( QStringLiteral("EditionWindow::_toggleShowKeyword.\n") );
    _setKeywordVisible( value || forceShowKeyword_ );
    XmlOptions::get().set<bool>( QStringLiteral("SHOW_KEYWORD"), value );

}

//_____________________________________________
void EditionWindow::_updateConfiguration()
{

    // one should check whether this is needed or not.
    Debug::Throw( QStringLiteral("EditionWindow::_updateConfiguration.\n") );
    resize( sizeHint() );

    // show keyword
    showKeywordAction_->setChecked( XmlOptions::get().get<bool>( QStringLiteral("SHOW_KEYWORD") ) );

}

//______________________________________________________
Private::LocalTextEditor::LocalTextEditor( QWidget* parent ):
    TextEditor( parent )
{
    setTrackAnchors( true );
    _installActions();

    // configuration
    connect( Base::Singleton::get().application<Application>(), &Application::configurationChanged, this, &LocalTextEditor::_updateConfiguration );
    _updateConfiguration();

}

//___________________________________________________________________________________
void Private::LocalTextEditor::insertFromMimeData( const QMimeData* source )
{
    Debug::Throw( QStringLiteral("Private::LocalTextEditor::insertFromMimeData.\n") );

    // check option
    if( !autoInsertLinks_ )
    {
        TextEditor::insertFromMimeData( source );
        return;
    }

    // do nothing in case of rich text
    if( source->hasFormat(QStringLiteral("application/x-qrichtext") ) )
    {
        TextEditor::insertFromMimeData( source );
        return;
    }

    // get source text
    QString text;
    if( source->hasHtml() ) text = source->html();
    else text = source->text();
    if( text.isNull() )
    {
        TextEditor::insertFromMimeData( source );
        return;
    }

    // try build an url from text
    QUrl url( text );
    if( !url.isValid() || url.isRelative() )
    {
        TextEditor::insertFromMimeData( source );
        return;
    }
    // check scheme
    static const QStringList schemes( { "file", "ftp", "http", "https", "alien" } );
    if( !schemes.contains( url.scheme() ) )
    {
        TextEditor::insertFromMimeData( source );
        return;
    }

    // copy mime type
    // redo html addind the proper href
    QMimeData copy;
    copy.setText( source->text() );
    copy.setHtml( QStringLiteral( "<a href=\"%1\">%1</a> " ).arg( text ) );
    TextEditor::insertFromMimeData( &copy );

}

//___________________________________________________________________________________
void Private::LocalTextEditor::installContextMenuActions( BaseContextMenu* menu, bool allActions )
{
    Debug::Throw( QStringLiteral("Private::LocalTextEditor::installContextMenuActions.\n") );
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
{ autoInsertLinks_ = XmlOptions::get().get<bool>( QStringLiteral("AUTO_INSERT_LINK") ); }

//___________________________________________________________________________________
void Private::LocalTextEditor::_installActions()
{
    Debug::Throw( QStringLiteral("Private::LocalTextEditor::_installActions.\n") );
    addAction( insertLinkAction_ = new QAction( IconEngine::get( IconNames::InsertSymbolicLink ), tr( "Insert Link..." ), this ) );
    addAction( editLinkAction_ = new QAction( IconEngine::get( IconNames::Edit ), tr( "Edit Link..." ), this ) );
    addAction( removeLinkAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Remove Link..." ), this ) );
    addAction( openLinkAction_ = new QAction( IconEngine::get( IconNames::Find ), tr( "Open Link..." ), this ) );

    // disable insert link action by default
    insertLinkAction_->setEnabled( false );
}

//___________________________________________________________________________________
Private::ColorWidget::ColorWidget( QWidget* parent ):
    LineEditorButton( parent )
{ Debug::Throw( QStringLiteral("ColorWidget::ColorWidget.\n") ); }

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
