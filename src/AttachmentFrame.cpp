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

#include <QDesktopServices>
#include <QHeaderView>
#include <QShortcut>
#include <QUrl>

//_____________________________________________
AttachmentFrame::AttachmentFrame( QWidget *parent, bool readOnly ):
    QWidget( parent ),
    readOnly_( readOnly ),
    thread_( this )
{
    Debug::Throw( "AttachmentFrame::AttachmentFrame.\n" );

    // tell validFile thread not to check duplicates
    // this is needed when checking files that are links
    thread_.setCheckDuplicates( false );
    connect( &thread_, SIGNAL(recordsAvailable(const FileRecord::List&,bool)), this, SLOT(_processRecords(const FileRecord::List&,bool)) );

    // default layout
    setLayout( new QVBoxLayout() );
    layout()->setMargin(0);
    layout()->setSpacing(5);

    // create list
    treeView_ = new TreeView();
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
    connect( treeView_->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(_updateActions()) );
    connect( treeView_->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(_itemSelected(QModelIndex)) );
    connect( treeView_, SIGNAL(activated(QModelIndex)), SLOT(_open()) );

    connect( Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    _updateConfiguration();
    _updateActions();
}


//______________________________________________________________________
void AttachmentFrame::setDefaultHeight( int value )
{ defaultHeight_ = value; }

//____________________________________________
QSize AttachmentFrame::sizeHint( void ) const
{ return (defaultHeight_ ) >= 0 ? QSize( 0, defaultHeight_ ):QWidget::sizeHint(); }

//_____________________________________________
void AttachmentFrame::add( const AttachmentModel::List& attachments )
{

    Debug::Throw( "AttachmentFrame::add.\n" );
    for( const auto& attachment:attachments )
    { Base::Key::associate( this, attachment ); }

    model_.add( attachments );
    treeView_->resizeColumns();

}

//_____________________________________________
void AttachmentFrame::update( Attachment& attachment )
{

    Debug::Throw( "AttachmentFrame::update.\n" );
    Q_ASSERT( attachment.isAssociated( this ) );
    model_.add( &attachment );
    treeView_->resizeColumns();

}

//_____________________________________________
void AttachmentFrame::select( Attachment& attachment )
{

    Debug::Throw( "AttachmentFrame::select.\n" );
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
void AttachmentFrame::_new( void )
{

    Debug::Throw( "AttachmentFrame::_new.\n" );

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
        MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
        if( mainwindow.logbook() && !mainwindow.logbook()->directory().isEmpty() )
        { dialog.setDestinationDirectory( mainwindow.logbook()->directory() ); }
    }

    // action
    dialog.setAction( Attachment::CopyVersion );
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

            InformationDialog( this, QString( tr( "File '%1' is not a directory." ) ).arg( fullDirectory ) ).exec();

        } else if( !fullDirectory.exists() && QuestionDialog( this, QString( tr( "Directory '%1' does not exist. Create ?" ) ).arg( fullDirectory ) ).exec() ) {

            ( Command( "mkdir" ) << fullDirectory ).run();
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
    Attachment *attachment = new Attachment( file );
    attachment->setIsUrl( isUrl );
    attachment->setComments( dialog.comments() );

    // retrieve command
    Attachment::Command command( dialog.action() );

    // process attachment command
    Attachment::ErrorCode error = attachment->copy( command, fullDirectory );
    QString buffer;
    switch (error)
    {

        case Attachment::SourceNotFound:
        InformationDialog( this, QString( tr( "Cannot find file '%1'. <Add Attachment> canceled." ) ).arg( file ) ).exec();
        delete attachment;
        break;

        case Attachment::DestNotFound:
        InformationDialog( this, QString( tr( "Cannot find directory '%1'. <Add Attachment> canceled." ) ).arg( fullDirectory ) ).exec();
        delete attachment;
        break;

        case Attachment::SourceIsDir:
        InformationDialog( this, QString( tr( "File '%1' is a directory. <Add Attachment> canceled." ) ).arg( file ) ).exec();
        delete attachment;
        break;

        case Attachment::DestExist:
        InformationDialog( this, QString( tr( "File '%1' is already in list." ) ).arg( file ) ).exec();
        delete attachment;
        break;

        case Attachment::Success:

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
        Singleton::get().application<Application>()->attachmentWindow().frame().add( *attachment );

        // update logbooks destination directory
        for( const auto& logbook:logbooks )
        {
            logbook->setModified( true );
            logbook->setDirectory( fullDirectory );
        }

        // change Application window title
        Singleton::get().application<Application>()->mainWindow().updateWindowTitle();

        // save EditionWindow entry
        window.saveAction().trigger();

        break;

        default: delete attachment; break;

    }

}

//_____________________________________________
void AttachmentFrame::enterEvent( QEvent* event )
{

    Debug::Throw( "AttachmentFrame::enterEvent.\n" );
    if( thread_.isRunning() || !hasList() ) return;

    // create file records
    FileRecord::List records;

    // retrieve all attachments from model
    AttachmentModel::List attachments( model_.get() );
    for( const auto& attachment:attachments )
    {

        if( attachment->isUrl() ) continue;
        if( attachment->file().isEmpty() ) continue;

        records << FileRecord( attachment->file() );

        if( attachment->isLink() == Attachment::Yes || attachment->isLink() == Attachment::Unknown )
        { records << FileRecord( attachment->sourceFile() ); }

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
        auto found = std::find_if( records.begin(), records.end(), FileRecord::SameFileFTor( attachment->file() ) );
        if( found != records.end() ) { isValid = found->isValid(); }
        else { Debug::Throw() << "AttachmentFrame::_processRecords - not found." << endl; }

        // check link status
        if( isValid && isLink == Attachment::Unknown )
        {
            // check if destination is a link
            QFileInfo fileInfo( attachment->file() );
            isLink = fileInfo.isSymLink() ? Attachment::Yes : Attachment::No;
        }

        // check source file
        if( isValid && isLink == Attachment::Yes )
        {
            found = std::find_if( records.begin(), records.end(), FileRecord::SameFileFTor( attachment->sourceFile() ) );
            if( found != records.end() ) { isValid &= found->isValid(); }
            else { Debug::Throw() << "AttachmentFrame::_processRecords - not found." << endl; }
        }

        // update validity flag and set parent logbook as modified if needed
        Debug::Throw() << "AttachmentFrame::_processRecords - valid: " << isValid << " link: " << isLink << endl;
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
        MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
        mainwindow.updateWindowTitle();
        if( mainwindow.logbook()->file().size() ) mainwindow.save();

    }

    cleanAction_->setEnabled( hasInvalidRecords );
    return;

}

//_____________________________________________
void AttachmentFrame::_updateConfiguration( void )
{

    Debug::Throw( "AttachmentFrame::_updateConfiguration.\n" );
    int icon_size( XmlOptions::get().get<int>( "ATTACHMENT_LIST_ICON_SIZE" ) );
    treeView_->setIconSize( QSize( icon_size, icon_size ) );

}

//_____________________________________________
void AttachmentFrame::_updateActions( void )
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
void AttachmentFrame::_open( void )
{
    Debug::Throw( "AttachmentFrame::_open.\n" );

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

        if( attachment->updateTimeStamps() ) modifiedAttachments << attachment;

        const bool isUrl( attachment->isUrl() );
        File fullname( isUrl ? attachment->file():attachment->file().expand() );
        if( !( isUrl || fullname.exists() ) )
        {
            InformationDialog( this, QString( tr( "Cannot find file '%1'. <Open Attachment> canceled." ) ).arg( fullname ) ).exec();
            continue;
        }

        OpenAttachmentDialog dialog( this, *attachment );
        if( dialog.centerOnWidget( window() ).exec() == QDialog::Accepted )
        {
            if( dialog.action() == OpenAttachmentDialog::Open )
            {

                if( !dialog.isCommandValid() || (dialog.command().isEmpty() && !dialog.isCommandDefault() ) )
                {
                    InformationDialog( this, QString( tr( "Specified command is invalid. <Open Attachment> canceled." ) ).arg( fullname ) ).exec();
                    continue;
                }

                if( dialog.isCommandDefault() )
                {

                    if( isUrl ) QDesktopServices::openUrl( QUrl::fromEncoded( fullname.toLatin1() ) );
                    else QDesktopServices::openUrl( QUrl::fromEncoded( QString( "file://%1" ).arg( fullname ).toLatin1() ) );

                } else {

                    ( Command( dialog.command() ) << fullname ).run();

                }

            } else  {

                // create and configure SaveAs dialog
                FileDialog dialog( this );
                dialog.setFileMode( QFileDialog::AnyFile );
                dialog.setAcceptMode( QFileDialog::AcceptSave );
                dialog.selectFile( fullname.localName().addPath( dialog.workingDirectory() ) );
                File destname( dialog.getFile() );

                // check filename and copy if accepted
                if( destname.isNull() || (destname.exists() && !QuestionDialog( this, tr( "Selected file already exists. Overwrite ?" ) ).exec() ) ) return;
                else ( Command("cp") << fullname << destname ).run();

            }

        }

    }

    // need to save attachments because timeStamps might have been updated
    _saveAttachments( modifiedAttachments );

    return;

}

//_____________________________________________
void AttachmentFrame::_edit( void )
{
    Debug::Throw( "AttachmentFrame::_edit.\n" );

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
        modifiedAttachments << attachment;

    }

    // save
    _saveAttachments( modifiedAttachments );

}

//_____________________________________________
void AttachmentFrame::_delete( void )
{
    Debug::Throw( "AttachmentFrame::_delete.\n" );

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
            bool fromDisk( dialog.action() == DeleteAttachmentDialog::DeleteFromDisk );

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
            File file( attachment->file().expand() );
            if( fromDisk && !attachment->isUrl() && file.isWritable() )
            { file.remove(); }

            // delete attachment
            delete attachment;

        }

    }

    if( logbookChanged )
    {
        Singleton::get().application<Application>()->mainWindow().updateWindowTitle();
        window.saveAction().trigger();

        // resize columns
        treeView_->resizeColumns();
    }

    return;

}

//_____________________________________________
void AttachmentFrame::_reload( void )
{
    Debug::Throw( "AttachmentFrame::_reload.\n" );

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
    { if( attachment->updateTimeStamps() ) modifiedAttachments << attachment; }

    _saveAttachments( modifiedAttachments );

}

//_____________________________________________
void AttachmentFrame::_saveAs( void )
{
    Debug::Throw( "AttachmentFrame::_saveAs.\n" );

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
        File fullname( isUrl ? attachment->file():attachment->file().expand() );
        if( isUrl )
        {

            InformationDialog( this, QString( tr( "Selected attachement is URL. <Save Attachment As> canceled." ) ) ).exec();
            continue;

        } else if( !fullname.exists() ) {

            QString buffer;
            InformationDialog( this, QString( tr( "Cannot find file '%1'. <Save Attachment As> canceled." ) ).arg( fullname ) ).exec();
            continue;

        } else {

            // create and configure SaveAs dialog
            FileDialog dialog( this );
            dialog.setFileMode( QFileDialog::AnyFile );
            dialog.setAcceptMode( QFileDialog::AcceptSave );
            dialog.selectFile( fullname.localName().addPath( dialog.workingDirectory() ) );
            File destname( dialog.getFile() );

            // check filename and copy if accepted
            if( destname.isNull() || (destname.exists() && !QuestionDialog( this, tr( "selected file already exists. Overwrite ?" ) ).exec() ) ) return;
            else ( Command("cp") << fullname << destname ).run();

        }

    }

    return;

}


//_________________________________________________________________________
void AttachmentFrame::_clean( void )
{
    Debug::Throw( "AttachmentFrame::clean.\n" );

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
        MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
        mainwindow.updateWindowTitle();
        if( mainwindow.logbook()->file().size() ) mainwindow.save();

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
void AttachmentFrame::_installActions( void )
{
    Debug::Throw( "AttachmentFrame::_installActions.\n" );

    addAction( visibilityAction_ = new QAction( IconEngine::get( IconNames::Attach ), tr( "Show &Attachment List" ), this ) );
    visibilityAction_->setToolTip( tr( "Show/hide attachment list" ) );
    visibilityAction_->setCheckable( true );
    visibilityAction_->setChecked( true );
    connect( &visibilityAction(), SIGNAL(toggled(bool)), SLOT(setVisible(bool)) );

    addAction( newAction_ = new QAction( IconEngine::get( IconNames::Attach ), tr( "New" ), this ) );
    newAction_->setToolTip( tr( "Attach a file/URL to the current entry" ) );
    newAction_->setIconText( tr( "Attach" ) );
    connect( newAction_, SIGNAL(triggered()), SLOT(_new()) );

    addAction( openAction_ = new QAction( IconEngine::get( IconNames::Open ), tr( "Open" ), this ) );
    openAction_->setToolTip( tr( "Open selected attachments" ) );
    connect( openAction_, SIGNAL(triggered()), SLOT(_open()) );

    addAction( editAction_ = new QAction( IconEngine::get( IconNames::Edit ), tr( "Edit" ), this ) );
    editAction_->setToolTip( tr( "Edit selected attachments informations" ) );
    connect( editAction_, SIGNAL(triggered()), SLOT(_edit()) );

    addAction( deleteAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Delete" ), this ) );
    deleteAction_->setShortcut( QKeySequence::Delete );
    deleteAction_->setToolTip( tr( "Delete selected attachments" ) );
    connect( deleteAction_, SIGNAL(triggered()), SLOT(_delete()) );

    addAction( reloadAction_ = new QAction( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this ) );
    reloadAction_->setShortcut( QKeySequence::Refresh );
    reloadAction_->setToolTip( tr( "Reload attachments timestamps" ) );
    connect( reloadAction_, SIGNAL(triggered()), SLOT(_reload()) );

    addAction( saveAsAction_ = new QAction( IconEngine::get( IconNames::SaveAs ), tr( "Save As" ), this ) );
    saveAsAction_->setToolTip( tr( "Save selected attachment with a different filename" ) );
    connect( saveAsAction_, SIGNAL(triggered()), SLOT(_saveAs()) );


    cleanAction_ = new QAction( IconEngine::get( IconNames::Delete ), tr( "Clean" ), this );
    cleanAction_->setToolTip( tr( "Delete selected attachments" ) );
    connect( cleanAction_, SIGNAL(triggered()), SLOT(_clean()) );
}

//_______________________________________________________________________
void AttachmentFrame::_saveAttachments( const AttachmentModel::List& attachments )
{
    Debug::Throw( "AttachmentFrame::_saveAttachments.\n" );
    if( attachments.empty() ) return;

    // associated lists
    Base::KeySet<LogEntry> entries;

    // loop over attachments
    for( const auto& attachment:attachments )
    {

        // get associated entries and store
        entries.unite( Base::KeySet<LogEntry>( attachment ) );
        Debug::Throw( "AttachmentFrame::_saveAttachments - entries.\n" );

        // get associated attachment frames and store
        Base::KeySet<AttachmentFrame> localFrames( attachment );
        for( const auto& frame:localFrames )
        { frame->update( *attachment ); }

        Debug::Throw( "AttachmentFrame::_saveAttachments - frames.\n" );

    }

    Base::KeySet<Logbook> logbooks;
    Base::KeySet<EditionWindow> editionWindows;

    // loop over entries
    for( const auto& entry:entries )
    {

        // get associated logbooks and store
        logbooks.unite( Base::KeySet<Logbook>( entry ) );

        Debug::Throw( "AttachmentFrame::_saveAttachments - logbooks.\n" );

        // get associated Edition windows
        editionWindows.unite( Base::KeySet<EditionWindow>( entry ) );
        Debug::Throw( "AttachmentFrame::_saveAttachments - edition windows.\n" );

    }

    // loop over logbook and set modified
    for( const auto& logbook:logbooks )
    { logbook->setModified( true ); }

    // loop over edition windows and trigger save action
    for( const auto& window:editionWindows )
    { window->saveAction().trigger(); }

    MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
    if( mainwindow.logbook()->file().size() )
    { mainwindow.save(); }

}
