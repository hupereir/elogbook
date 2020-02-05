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

#include "AttachmentFrame.h"

#include "Application.h"
#include "Attachment.h"
#include "AttachmentWindow.h"
#include "Command.h"
#include "ContextMenu.h"
#include "Debug.h"
#include "DeleteAttachmentDialog.h"
#include "EditAttachmentDialog.h"
#include "EditionWindow.h"
#include "File.h"
#include "FileDialog.h"
#include "FileRecord.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "MainWindow.h"
#include "NewAttachmentDialog.h"
#include "OpenAttachmentDialog.h"
#include "QuestionDialog.h"
#include "Singleton.h"
#include "TreeView.h"
#include "InformationDialog.h"

#include <memory>

#include <QDesktopServices>
#include <QHeaderView>
#include <QUrl>

//_____________________________________________
AttachmentFrame::AttachmentFrame( QWidget *parent, bool readOnly ):
    QWidget( parent ),
    readOnly_( readOnly ),
    thread_( this )
{
    Debug::Throw( QStringLiteral("AttachmentFrame::AttachmentFrame.\n") );

    // tell validFile thread not to check duplicates
    // this is needed when checking files that are links
    thread_.setCheckDuplicates( false );
    connect( &thread_, &ValidFileThread::recordsAvailable, this, &AttachmentFrame::_processRecords );

    // default layout
    setLayout( new QVBoxLayout );
    layout()->setMargin(0);
    layout()->setSpacing(5);

    // create list
    treeView_ = new TreeView;
    layout()->addWidget( new TreeView::Container( this, treeView_ ) );
    treeView_->setModel( &_model() );
    treeView_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    treeView_->setOptionName( "ATTACHMENTLIST" );
    treeView_->setTextElideMode ( Qt::ElideMiddle );

    // install actions
    _installActions();

    contextMenu_ = new ContextMenu( treeView_ );
    contextMenu_->addAction( newAction_ );
    contextMenu_->addAction( openAction_ );
    contextMenu_->addAction( saveAsAction_ );
    contextMenu_->addAction( editAction_ );
    contextMenu_->addAction( deleteAction_ );
    contextMenu_->addAction( reloadAction_ );
    contextMenu_->addSeparator();
    contextMenu_->addAction( cleanAction_ );

    // connections
    connect( treeView_->selectionModel(), &QItemSelectionModel::selectionChanged, this, &AttachmentFrame::_updateActions );
    connect( treeView_->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &AttachmentFrame::_itemSelected );
    connect( treeView_, &QAbstractItemView::activated, this, &AttachmentFrame::_open );

    connect( Base::Singleton::get().application<Application>(), &Application::configurationChanged, this, &AttachmentFrame::_updateConfiguration );
    _updateConfiguration();
    _updateActions();
}


//______________________________________________________________________
void AttachmentFrame::setDefaultHeight( int value )
{ defaultHeight_ = value; }

//____________________________________________
QSize AttachmentFrame::sizeHint() const
{ return (defaultHeight_ ) >= 0 ? QSize( 0, defaultHeight_ ):QWidget::sizeHint(); }

//_____________________________________________
void AttachmentFrame::add( const AttachmentModel::List& attachments )
{

    Debug::Throw( QStringLiteral("AttachmentFrame::add.\n") );
    for( const auto& attachment:attachments )
    { Base::Key::associate( this, attachment ); }

    model_.add( attachments );
    treeView_->resizeColumns();

}

//_____________________________________________
void AttachmentFrame::update( Attachment& attachment )
{

    Debug::Throw( QStringLiteral("AttachmentFrame::update.\n") );
    Q_ASSERT( attachment.isAssociated( this ) );
    model_.add( &attachment );
    treeView_->resizeColumns();

}

//_____________________________________________
void AttachmentFrame::select( Attachment& attachment )
{

    Debug::Throw( QStringLiteral("AttachmentFrame::select.\n") );
    Q_ASSERT( attachment.isAssociated( this ) );

    // get matching model index
    QModelIndex index( model_.index( &attachment ) );

    // check if index is valid and not selected
    if( ( !index.isValid() ) || treeView_->selectionModel()->isSelected( index ) ) return;

    // select
    treeView_->selectionModel()->select( index,  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );

    return;

}

//_____________________________________________
void AttachmentFrame::_new()
{

    Debug::Throw( QStringLiteral("AttachmentFrame::_new.\n") );

    // retrieve/check associated EditionWindow/LogEntry
    Base::KeySet<EditionWindow> windows( this );
    Q_ASSERT( windows.size() == 1 );

    EditionWindow &window( **windows.begin() );

    Base::KeySet<LogEntry> entries( window );
    if( entries.size() != 1 )
    {
        InformationDialog( this, tr( "No valid entry found. <New Attachment> canceled." ) ).exec();
        return;
    }

    LogEntry *entry( *entries.begin() );
    Base::KeySet<Logbook> logbooks( entry );

    // create dialog
    NewAttachmentDialog dialog( this );

    // update destination directory
    if( logbooks.size() && !(*logbooks.begin())->directory().isEmpty() ) { dialog.setDestinationDirectory( (*logbooks.begin())->directory() ); }
    else {
        MainWindow& mainwindow( Base::Singleton::get().application<Application>()->mainWindow() );
        if( mainwindow.logbook() && !mainwindow.logbook()->directory().isEmpty() )
        { dialog.setDestinationDirectory( mainwindow.logbook()->directory() ); }
    }

    // action
    dialog.setAction( Attachment::Command::CopyVersion );
    dialog.resize( 400, 350 );
    if( dialog.centerOnWidget( AttachmentFrame::window() ).exec() == QDialog::Rejected ) return;

    // retrieve Attachment type
    const bool isUrl( dialog.isUrl() );

    // retrieve destination directory
    File fullDirectory = dialog.destinationDirectory();

    // check destination directory (if file is not URL)
    if( !isUrl )
    {
        // check if destination directory is not a non directory existsing file
        if( fullDirectory.exists() && !fullDirectory.isDirectory() )
        {

            InformationDialog( this, tr( "File '%1' is not a directory." ).arg( fullDirectory ) ).exec();

        } else if( !fullDirectory.exists() && QuestionDialog( this, tr( "Directory '%1' does not exist. Create ?" ).arg( fullDirectory ) ).exec() ) {

            ( Base::Command( "mkdir" ) << fullDirectory ).run();
        }

    }

    // retrieve check attachment filename
    File file( dialog.file() );
    if( file.isEmpty() )
    {
        InformationDialog( this, tr( "Invalid name. <New Attachment> canceled." ) ).exec();
        return;
    }

    // create attachment with correct type
    std::unique_ptr<Attachment> attachmentPtr( new Attachment( file ) );
    attachmentPtr->setIsUrl( isUrl );
    attachmentPtr->setComments( dialog.comments() );

    // retrieve command
    Attachment::Command command( dialog.action() );

    // process attachment command
    Attachment::ErrorCode error = attachmentPtr->copy( command, fullDirectory );
    QString buffer;
    switch (error)
    {

        case Attachment::ErrorCode::SourceNotFound:
        InformationDialog( this, tr( "Cannot find file '%1'. <Add Attachment> canceled." ).arg( file ) ).exec();
        break;

        case Attachment::ErrorCode::DestNotFound:
        InformationDialog( this, tr( "Cannot find directory '%1'. <Add Attachment> canceled." ).arg( fullDirectory ) ).exec();
        break;

        case Attachment::ErrorCode::SourceIsDir:
        InformationDialog( this, tr( "File '%1' is a directory. <Add Attachment> canceled." ).arg( file ) ).exec();
        break;

        case Attachment::ErrorCode::DestExist:
        InformationDialog( this, tr( "File '%1' is already in list." ).arg( file ) ).exec();
        break;

        case Attachment::ErrorCode::Success:
        {

            auto attachment( attachmentPtr.release() );

            // associate attachment to entry
            Base::Key::associate( entry, attachment );

            // update all windows edition windows associated to entry
            windows = Base::KeySet<EditionWindow>( entry );
            for( const auto& window:windows )
            {

                window->attachmentFrame().visibilityAction_->setChecked( true );
                window->attachmentFrame().add( *attachment );

            }

            // update attachment frame
            Base::Singleton::get().application<Application>()->attachmentWindow().frame().add( *attachment );

            // update logbooks destination directory
            for( const auto& logbook:logbooks )
            {
                logbook->setModified( true );
                logbook->setDirectory( fullDirectory );
            }

            // change Application window title
            Base::Singleton::get().application<Application>()->mainWindow().updateWindowTitle();

            // save EditionWindow entry
            window.saveAction().trigger();
        }
        break;

        default: break;

    }

}

//_____________________________________________
void AttachmentFrame::enterEvent( QEvent* event )
{

    Debug::Throw( QStringLiteral("AttachmentFrame::enterEvent.\n") );
    if( thread_.isRunning() || !hasList() ) return;

    // create file records
    FileRecord::List records;

    // retrieve all attachments from model
    AttachmentModel::List attachments( model_.get() );
    for( const auto& attachment:attachments )
    {

        if( attachment->isUrl() ) continue;
        if( attachment->file().isEmpty() ) continue;

        records.append( FileRecord( attachment->file() ) );

        if( attachment->isLink() == Attachment::LinkState::Yes || attachment->isLink() == Attachment::LinkState::Unknown )
        { records.append( FileRecord( attachment->sourceFile() ) ); }

    }

    // setup thread and start
    thread_.setRecords( records );
    thread_.start();

    return QWidget::enterEvent( event );

}

//_______________________________________________
void AttachmentFrame::_processRecords( const FileRecord::List& records, bool hasInvalidRecords )
{

    Debug::Throw() << "AttachmentFrame::_processRecords." << endl;

    // retrieve all attachments from model
    // true if some modifications are to be saved
    bool modified( false );
    for( const auto& attachment:model_.get() )
    {

        if( attachment->isUrl() ) continue;
        if( attachment->file().isEmpty() ) continue;

        Debug::Throw() << "AttachmentFrame::_processRecords - checking: " << attachment->file() << endl;

        bool isValid( attachment->isValid() );
        Attachment::LinkState isLink( attachment->isLink() );

        // check destination file
        auto found = std::find_if( records.begin(), records.end(), FileRecord::SameFileFTorUnary( attachment->file() ) );
        if( found != records.end() ) { isValid = found->isValid(); }
        else { Debug::Throw() << "AttachmentFrame::_processRecords - not found." << endl; }

        // check link status
        if( isValid && isLink == Attachment::LinkState::Unknown )
        {
            // check if destination is a link
            QFileInfo fileInfo( attachment->file() );
            isLink = fileInfo.isSymLink() ? Attachment::LinkState::Yes : Attachment::LinkState::No;
        }

        // check source file
        if( isValid && isLink == Attachment::LinkState::Yes )
        {
            found = std::find_if( records.begin(), records.end(), FileRecord::SameFileFTorUnary( attachment->sourceFile() ) );
            if( found != records.end() ) { isValid &= found->isValid(); }
            else { Debug::Throw() << "AttachmentFrame::_processRecords - not found." << endl; }
        }

        // update validity flag and set parent logbook as modified if needed
        Debug::Throw() << "AttachmentFrame::_processRecords - valid: " << isValid << " link: " << Base::toIntegralType( isLink ) << endl;
        if( attachment->setIsValid( isValid ) || attachment->setIsLink( isLink ) )
        {

            // get associated entry
            Base::KeySet<LogEntry> entries( attachment );
            Q_ASSERT( entries.size() == 1 );

            auto entry( *entries.begin() );
            entry->setModified();

            // get associated logbooks
            Base::KeySet<Logbook> logbooks( entry );
            for( const auto& logbook:logbooks ) logbook->setModified( true );

            modified = true;

        }

        // update attachment size
        attachment->updateSize();

    }

    // save logbooks
    if( modified )
    {

        // set main window title
        MainWindow& mainwindow( Base::Singleton::get().application<Application>()->mainWindow() );
        mainwindow.updateWindowTitle();
        if( !mainwindow.logbook()->file().isEmpty() ) mainwindow.save();

    }

    cleanAction_->setEnabled( hasInvalidRecords );
    return;

}

//_____________________________________________
void AttachmentFrame::_updateConfiguration()
{

    Debug::Throw( QStringLiteral("AttachmentFrame::_updateConfiguration.\n") );
    int icon_size( XmlOptions::get().get<int>( QStringLiteral("ATTACHMENT_LIST_ICON_SIZE") ) );
    treeView_->setIconSize( QSize( icon_size, icon_size ) );

}

//_____________________________________________
void AttachmentFrame::_updateActions()
{

    bool hasSelection( !treeView_->selectionModel()->selectedRows().isEmpty() );
    newAction_->setEnabled( !readOnly_ );
    openAction_->setEnabled( hasSelection );
    saveAsAction_->setEnabled( hasSelection );
    reloadAction_->setEnabled( hasSelection );
    editAction_->setEnabled( hasSelection && !readOnly_ );
    deleteAction_->setEnabled( hasSelection && !readOnly_ );
    return;

}

//_____________________________________________
void AttachmentFrame::_open()
{
    Debug::Throw( QStringLiteral("AttachmentFrame::_open.\n") );

    // get selection
    AttachmentModel::List selection( model_.get( treeView_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Open Attachment> canceled." ) ).exec();
        return;
    }

    // loop over attachments
    AttachmentModel::List modifiedAttachments;
    for( const auto& attachment:selection )
    {

        if( attachment->updateTimeStamps() ) modifiedAttachments.append( attachment );

        const bool isUrl( attachment->isUrl() );
        File fullname( isUrl ? attachment->file():attachment->file().expanded() );
        if( !( isUrl || fullname.exists() ) )
        {
            InformationDialog( this, tr( "Cannot find file '%1'. <Open Attachment> canceled." ).arg( fullname ) ).exec();
            continue;
        }

        OpenAttachmentDialog dialog( this, *attachment );
        if( dialog.centerOnWidget( window() ).exec() == QDialog::Accepted )
        {
            if( dialog.action() == OpenAttachmentDialog::Action::Open )
            {

                if( !dialog.isCommandValid() || (dialog.command().isEmpty() && !dialog.isCommandDefault() ) )
                {
                    InformationDialog( this, tr( "Specified command is invalid. <Open Attachment> canceled." ).arg( fullname ) ).exec();
                    continue;
                }

                if( dialog.isCommandDefault() )
                {

                    if( isUrl ) QDesktopServices::openUrl( QUrl::fromEncoded( fullname.get().toLatin1() ) );
                    else QDesktopServices::openUrl( QUrl::fromEncoded( QString( "file://%1" ).arg( fullname ).toLatin1() ) );

                } else {

                    ( Base::Command( dialog.command() ) << fullname ).run();

                }

            } else  {

                // create and configure SaveAs dialog
                FileDialog dialog( this );
                dialog.setFileMode( QFileDialog::AnyFile );
                dialog.setAcceptMode( QFileDialog::AcceptSave );
                dialog.selectFile( fullname.localName().addPath( dialog.workingDirectory() ) );
                File destname( dialog.getFile() );

                // check filename and copy if accepted
                if( destname.isEmpty() || (destname.exists() && !QuestionDialog( this, tr( "Selected file already exists. Overwrite ?" ) ).exec() ) ) return;
                else ( Base::Command("cp") << fullname << destname ).run();

            }

        }

    }

    // need to save attachments because timeStamps might have been updated
    _saveAttachments( modifiedAttachments );

    return;

}

//_____________________________________________
void AttachmentFrame::_edit()
{
    Debug::Throw( QStringLiteral("AttachmentFrame::_edit.\n") );

    // store selected item locally
    // get selection
    AttachmentModel::List selection( model_.get( treeView_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Edit Attachment> canceled." ) ).exec();
        return;
    }

    // loop over attachments
    AttachmentModel::List modifiedAttachments;
    for( const auto& attachment:selection )
    {

        // update time stamps
        bool attachmentChanged( false );
        attachmentChanged |= attachment->updateTimeStamps();
        EditAttachmentDialog dialog( this, *attachment );

        // map dialog
        if( dialog.centerOnWidget( window() ).exec() == QDialog::Accepted )
        {

            // retrieve comments
            attachmentChanged |= attachment->setComments( dialog.comments() );

            // update time stamps
            attachmentChanged |= attachment->updateTimeStamps();

        }

        if( !attachmentChanged ) continue;
        modifiedAttachments.append( attachment );

    }

    // save
    _saveAttachments( modifiedAttachments );

}

//_____________________________________________
void AttachmentFrame::_delete()
{
    Debug::Throw( QStringLiteral("AttachmentFrame::_delete.\n") );

    // store selected item locally
    // get selection
    AttachmentModel::List selection( model_.get( treeView_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Delete Attachment> canceled." ) ).exec();
        return;
    }

    // retrieve/check associated EditionWindow/LogEntry
    Base::KeySet<EditionWindow> windows( this );
    Q_ASSERT( windows.size() == 1 );
    EditionWindow &window( **windows.begin() );

    // loop over attachments
    bool logbookChanged( false );
    for( const auto& attachment:selection )
    {

        // dialog
        DeleteAttachmentDialog dialog( this, *attachment );
        if( dialog.centerOnWidget( AttachmentFrame::window() ).exec() == QDialog::Accepted )
        {

            logbookChanged = true;

            // retrieve action
            bool fromDisk( dialog.action() == DeleteAttachmentDialog::Action::DeleteFromDisk );

            // retrieve associated attachment frames and remove item
            Base::KeySet<AttachmentFrame> frames( attachment );
            for( const auto& frame:frames )
            { frame->model_.remove( attachment ); }

            // retrieve associated entries
            Base::KeySet<LogEntry> entries( attachment );
            Q_ASSERT( entries.size() == 1 );

            auto entry( *entries.begin() );
            entry->setModified();

            // retrieve associated logbooks
            Base::KeySet<Logbook> logbooks( entry );

            // check sharing attachments to avoid fromDisk deletion
            if( fromDisk && logbooks.size() )
            {

                Base::KeySet<Attachment> attachments( (*logbooks.begin())->attachments() );
                auto nShare = std::count_if( attachments.begin(), attachments.end(), Attachment::SameFileFTor( attachment ) );
                if( nShare > 1 ) {

                    InformationDialog( this, tr( "Attachment still in use by other entries. Kept on disk." ) ).exec();
                    fromDisk = false;

                }

            }

            // remove file from disk, if required
            File file( attachment->file().expanded() );
            if( fromDisk && !attachment->isUrl() && file.isWritable() )
            { file.remove(); }

            // delete attachment
            delete attachment;

        }

    }

    if( logbookChanged )
    {
        Base::Singleton::get().application<Application>()->mainWindow().updateWindowTitle();
        window.saveAction().trigger();

        // resize columns
        treeView_->resizeColumns();
    }

    return;

}

//_____________________________________________
void AttachmentFrame::_reload()
{
    Debug::Throw( QStringLiteral("AttachmentFrame::_reload.\n") );

    // get selection
    AttachmentModel::List selection( model_.get( treeView_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Save Attachment As> canceled." ) ).exec();
        return;
    }

    // loop over attachments
    AttachmentModel::List modifiedAttachments;
    for( const auto& attachment:selection )
    { if( attachment->updateTimeStamps() ) modifiedAttachments.append( attachment ); }

    _saveAttachments( modifiedAttachments );

}

//_____________________________________________
void AttachmentFrame::_saveAs()
{
    Debug::Throw( QStringLiteral("AttachmentFrame::_saveAs.\n") );

    // get selection
    AttachmentModel::List selection( model_.get( treeView_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Save Attachment As> canceled." ) ).exec();
        return;
    }

    // loop over attachments
    for( const auto& attachment:selection )
    {

        const bool isUrl( attachment->isUrl() );
        File fullname( isUrl ? attachment->file():attachment->file().expanded() );
        if( isUrl )
        {

            InformationDialog( this, tr( "Selected attachement is URL. <Save Attachment As> canceled." ) ).exec();
            continue;

        } else if( !fullname.exists() ) {

            QString buffer;
            InformationDialog( this, tr( "Cannot find file '%1'. <Save Attachment As> canceled." ).arg( fullname ) ).exec();
            continue;

        } else {

            // create and configure SaveAs dialog
            FileDialog dialog( this );
            dialog.setFileMode( QFileDialog::AnyFile );
            dialog.setAcceptMode( QFileDialog::AcceptSave );
            dialog.selectFile( fullname.localName().addPath( dialog.workingDirectory() ) );
            File destname( dialog.getFile() );

            // check filename and copy if accepted
            if( destname.isEmpty() || (destname.exists() && !QuestionDialog( this, tr( "selected file already exists. Overwrite ?" ) ).exec() ) ) return;
            else ( Base::Command("cp") << fullname << destname ).run();

        }

    }

    return;

}


//_________________________________________________________________________
void AttachmentFrame::_clean()
{
    Debug::Throw( QStringLiteral("AttachmentFrame::clean.\n") );

    // ask for confirmation
    if( !QuestionDialog( this, tr( "Remove all invalid attachments ?" ) ).exec() ) return;

    // retrieve all attachments from model
    // true if some modifications are to be saved
    bool modified( false );
    for( const auto& attachment:model_.get() )
    {

        // skip attachment if valid
        if( attachment->isValid() ) continue;

        Debug::Throw() << "AttachmentFrame::_clean - removing: " << attachment->file() << endl;

        // retrieve associated attachment frames and remove item
        Base::KeySet<AttachmentFrame> frames( attachment );
        for( const auto& frame:frames )
        { frame->model_.remove( attachment ); }

        // retrieve associated entries
        Base::KeySet<LogEntry> entries( attachment );
        Q_ASSERT( entries.size() == 1 );

        auto entry( *entries.begin() );
        entry->setModified();

        // retrieve associated logbooks
        Base::KeySet<Logbook> logbooks( entry );
        for( const auto& logbook:logbooks )
        { logbook->setModified( true ); }

        // delete attachment
        delete attachment;
        modified = true;
    }

    if( modified )
    {

        // set main window title
        MainWindow& mainwindow( Base::Singleton::get().application<Application>()->mainWindow() );
        mainwindow.updateWindowTitle();
        if( !mainwindow.logbook()->file().isEmpty() ) mainwindow.save();

    }

}

//_________________________________________________________________________
void AttachmentFrame::_itemSelected( const QModelIndex& index )
{
    if( !index.isValid() ) return;
    Attachment& attachment( *model_.get( index ) );
    Debug::Throw() << "AttachmentFrame::_itemSelected - " << attachment.file() << endl;
    emit attachmentSelected( attachment );

}

//_______________________________________________________________________
void AttachmentFrame::_installActions()
{
    Debug::Throw( QStringLiteral("AttachmentFrame::_installActions.\n") );

    addAction( visibilityAction_ = new QAction( IconEngine::get( IconNames::Attach ), tr( "Show &Attachment List" ), this ) );
    visibilityAction_->setToolTip( tr( "Show/hide attachment list" ) );
    visibilityAction_->setCheckable( true );
    visibilityAction_->setChecked( true );
    connect( &visibilityAction(), &QAction::toggled, this, &QWidget::setVisible );

    addAction( newAction_ = new QAction( IconEngine::get( IconNames::Attach ), tr( "New" ), this ) );
    newAction_->setToolTip( tr( "Attach a file/URL to the current entry" ) );
    newAction_->setIconText( tr( "Attach" ) );
    connect( newAction_, &QAction::triggered, this, &AttachmentFrame::_new );

    addAction( openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open" ), this ) );
    openAction_->setToolTip( tr( "Open selected attachments" ) );
    connect( openAction_, &QAction::triggered, this, &AttachmentFrame::_open );

    addAction( editAction_ = new QAction( IconEngine::get( IconNames::Edit ), tr( "Edit" ), this ) );
    editAction_->setToolTip( tr( "Edit selected attachments informations" ) );
    connect( editAction_, &QAction::triggered, this, &AttachmentFrame::_edit );

    addAction( deleteAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete" ), this ) );
    deleteAction_->setShortcut( QKeySequence::Delete );
    deleteAction_->setToolTip( tr( "Delete selected attachments" ) );
    connect( deleteAction_, &QAction::triggered, this, &AttachmentFrame::_delete );

    addAction( reloadAction_ = new QAction( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this ) );
    reloadAction_->setShortcut( QKeySequence::Refresh );
    reloadAction_->setToolTip( tr( "Reload attachments timestamps" ) );
    connect( reloadAction_, &QAction::triggered, this, &AttachmentFrame::_reload );

    addAction( saveAsAction_ = new QAction( IconEngine::get( IconNames::SaveAs ), tr( "Save As" ), this ) );
    saveAsAction_->setToolTip( tr( "Save selected attachment with a different filename" ) );
    connect( saveAsAction_, &QAction::triggered, this, &AttachmentFrame::_saveAs );


    cleanAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Clean" ), this );
    cleanAction_->setToolTip( tr( "Delete selected attachments" ) );
    connect( cleanAction_, &QAction::triggered, this, &AttachmentFrame::_clean );
}

//_______________________________________________________________________
void AttachmentFrame::_saveAttachments( const AttachmentModel::List& attachments )
{
    Debug::Throw( QStringLiteral("AttachmentFrame::_saveAttachments.\n") );
    if( attachments.empty() ) return;

    // associated lists
    Base::KeySet<LogEntry> entries;

    // loop over attachments
    for( const auto& attachment:attachments )
    {

        // get associated entries and store
        entries.unite( Base::KeySet<LogEntry>( attachment ) );
        Debug::Throw( QStringLiteral("AttachmentFrame::_saveAttachments - entries.\n") );

        // get associated attachment frames and store
        Base::KeySet<AttachmentFrame> localFrames( attachment );
        for( const auto& frame:localFrames )
        { frame->update( *attachment ); }

        Debug::Throw( QStringLiteral("AttachmentFrame::_saveAttachments - frames.\n") );

    }

    Base::KeySet<Logbook> logbooks;
    Base::KeySet<EditionWindow> editionWindows;

    // loop over entries
    for( const auto& entry:entries )
    {

        // get associated logbooks and store
        logbooks.unite( Base::KeySet<Logbook>( entry ) );

        Debug::Throw( QStringLiteral("AttachmentFrame::_saveAttachments - logbooks.\n") );

        // get associated Edition windows
        editionWindows.unite( Base::KeySet<EditionWindow>( entry ) );
        Debug::Throw( QStringLiteral("AttachmentFrame::_saveAttachments - edition windows.\n") );

    }

    // loop over logbook and set modified
    for( const auto& logbook:logbooks )
    { logbook->setModified( true ); }

    // loop over edition windows and trigger save action
    for( const auto& window:editionWindows )
    { window->saveAction().trigger(); }

    MainWindow& mainwindow( Base::Singleton::get().application<Application>()->mainWindow() );
    if( !mainwindow.logbook()->file().isEmpty() )
    { mainwindow.save(); }

}
