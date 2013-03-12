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
#include "Icons.h"
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

#include <QHeaderView>
#include <QShortcut>

//_____________________________________________
AttachmentFrame::AttachmentFrame( QWidget *parent, bool readOnly ):
    QWidget( parent ),
    readOnly_( readOnly ),
    defaultHeight_( -1 ),
    thread_( this )
{
    Debug::Throw( "AttachmentFrame::AttachmentFrame.\n" );

    // tell validFile thread not to check duplicates
    // this is needed when checking files that are links
    thread_.setCheckDuplicates( false );
    connect( &thread_, SIGNAL( recordsAvailable( const FileRecord::List&, bool ) ), this, SLOT( _processRecords( const FileRecord::List&, bool ) ) );

    // default layout
    setLayout( new QVBoxLayout() );
    layout()->setMargin(0);
    layout()->setSpacing(5);

    // create list
    layout()->addWidget( list_ = new TreeView( this ) );
    list_->setItemMargin( 2 );
    list_->setModel( &_model() );
    list_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    list_->setOptionName( "ATTACHMENTLIST" );
    list_->setTextElideMode ( Qt::ElideMiddle );

    // install actions
    _installActions();

    contextMenu_ = new ContextMenu( list_ );
    contextMenu_->addAction( &newAction() );
    contextMenu_->addAction( &openAction() );
    contextMenu_->addAction( &saveAsAction() );
    contextMenu_->addAction( &editAction() );
    contextMenu_->addAction( &deleteAction() );
    contextMenu_->addAction( &reloadAction() );
    contextMenu_->addSeparator();
    contextMenu_->addAction( &cleanAction() );

    // connections
    connect( list_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions( void ) ) );
    connect( list_->selectionModel(), SIGNAL( currentRowChanged(const QModelIndex &, const QModelIndex &) ), SLOT( _itemSelected( const QModelIndex& ) ) );
    connect( list_, SIGNAL( activated( const QModelIndex& ) ), SLOT( _open( void ) ) );

    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    _updateConfiguration();
    _updateActions();
}


//______________________________________________________________________
void AttachmentFrame::setDefaultHeight( const int& value )
{ defaultHeight_ = value; }

//____________________________________________
QSize AttachmentFrame::sizeHint( void ) const
{ return (defaultHeight_ ) >= 0 ? QSize( 0, defaultHeight_ ):QWidget::sizeHint(); }

//_____________________________________________
void AttachmentFrame::add( const AttachmentModel::List& attachments )
{

    Debug::Throw( "AttachmentFrame::add.\n" );
    foreach( Attachment* attachment, attachments )
    { BASE::Key::associate( this, attachment ); }

    model_.add( attachments );
    list_->resizeColumns();

}

//_____________________________________________
void AttachmentFrame::update( Attachment& attachment )
{

    Debug::Throw( "AttachmentFrame::update.\n" );
    Q_ASSERT( attachment.isAssociated( this ) );
    model_.add( &attachment );
    list_->resizeColumns();

}

//_____________________________________________
void AttachmentFrame::select( Attachment& attachment )
{

    Debug::Throw( "AttachmentFrame::select.\n" );
    Q_ASSERT( attachment.isAssociated( this ) );

    // get matching model index
    QModelIndex index( model_.index( &attachment ) );

    // check if index is valid and not selected
    if( ( !index.isValid() ) || list_->selectionModel()->isSelected( index ) ) return;

    // select
    list_->selectionModel()->select( index,  QItemSelectionModel::Clear|QItemSelectionModel::Select|QItemSelectionModel::Rows );

    return;

}

//_____________________________________________
void AttachmentFrame::_new( void )
{

    Debug::Throw( "AttachmentFrame::_new.\n" );

    // retrieve/check associated EditionWindow/LogEntry
    BASE::KeySet<EditionWindow> windows( this );
    Q_ASSERT( windows.size() == 1 );

    EditionWindow &window( **windows.begin() );

    BASE::KeySet<LogEntry> entries( window );
    if( entries.size() != 1 )
    {
        InformationDialog( this, tr( "No valid entry found. <New Attachment> canceled." ) ).exec();
        return;
    }

    LogEntry *entry( *entries.begin() );
    BASE::KeySet<Logbook> logbooks( entry );

    // create dialog
    NewAttachmentDialog dialog( this );

    // update destination directory
    if( logbooks.size() && !(*logbooks.begin())->directory().isEmpty() ) { dialog.setDestinationDirectory( (*logbooks.begin())->directory() ); }
    else {
        MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
        if( mainwindow.logbook() && !mainwindow.logbook()->directory().isEmpty() )
        { dialog.setDestinationDirectory( mainwindow.logbook()->directory() ); }
    }

    // type and action
    dialog.setType( AttachmentType::UNKNOWN );
    dialog.setAction( Attachment::COPY_VERSION );
    dialog.resize( 400, 350 );
    if( dialog.centerOnWidget( AttachmentFrame::window() ).exec() == QDialog::Rejected ) return;

    // retrieve Attachment type
    AttachmentType type( dialog.type() );

    // retrieve destination directory
    File fullDirectory = dialog.destinationDirectory();

    // check destination directory (if file is not URL)
    if( !(type == AttachmentType::URL) )
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
    Attachment *attachment = new Attachment( file, type );

    // retrieve check comments
    attachment->setComments( dialog.comments() );

    // retrieve command
    Attachment::Command command( dialog.action() );

    // process attachment command
    Attachment::ErrorCode error = attachment->copy( command, fullDirectory );
    QString buffer;
    switch (error)
    {

        case Attachment::SOURCE_NOT_FOUND:
        InformationDialog( this, QString( tr( "Cannot find file '%1'. <Add Attachment> canceled." ) ).arg( file ) ).exec();
        delete attachment;
        break;

        case Attachment::DEST_NOT_FOUND:
        InformationDialog( this, QString( tr( "Cannot find directory '%1'. <Add Attachment> canceled." ) ).arg( fullDirectory ) ).exec();
        delete attachment;
        break;

        case Attachment::SOURCE_IS_DIR:
        InformationDialog( this, QString( tr( "File '%1' is a directory. <Add Attachment> canceled." ) ).arg( file ) ).exec();
        delete attachment;
        break;

        case Attachment::DEST_EXIST:
        InformationDialog( this, QString( tr( "File '%1' is already in list." ) ).arg( file ) ).exec();
        delete attachment;
        break;

        case Attachment::SUCCESS:

        // associate attachment to entry
        Key::associate( entry, attachment );

        // update all windows edition windows associated to entry
        windows = BASE::KeySet<EditionWindow>( entry );
        foreach( EditionWindow* window, windows )
        {

            window->attachmentFrame().visibilityAction().setChecked( true );
            window->attachmentFrame().add( *attachment );

        }

        // update attachment frame
        Singleton::get().application<Application>()->attachmentWindow().frame().add( *attachment );

        // update logbooks destination directory
        foreach( Logbook* logbook, logbooks )
        {
            logbook->setModified( true );
            logbook->setDirectory( fullDirectory );
        }

        // change Application window title
        Singleton::get().application<Application>()->mainWindow().setModified( true );

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
    foreach( Attachment* attachment, attachments )
    {

        if( attachment->type() == AttachmentType::URL ) continue;
        if( attachment->file().isEmpty() ) continue;

        records << FileRecord( attachment->file() );

        if( attachment->isLink() == Attachment::YES || attachment->isLink() == Attachment::UNKNOWN )
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
    foreach( Attachment* attachment, model_.get() )
    {

        if( attachment->type() == AttachmentType::URL ) continue;
        if( attachment->file().isEmpty() ) continue;

        Debug::Throw() << "AttachmentFrame::_processRecords - checking: " << attachment->file() << endl;

        bool isValid( attachment->isValid() );
        Attachment::LinkState isLink( attachment->isLink() );

        // check destination file
        FileRecord::List::const_iterator found = std::find_if( records.begin(), records.end(), FileRecord::SameFileFTor( attachment->file() ) );
        if( found != records.end() ) { isValid = found->isValid(); }
        else { Debug::Throw() << "AttachmentFrame::_processRecords - not found." << endl; }

        // check link status
        if( isValid && isLink == Attachment::UNKNOWN )
        {
            // check if destination is a link
            QFileInfo fileInfo( attachment->file() );
            isLink = fileInfo.isSymLink() ? Attachment::YES : Attachment::NO;
        }

        // check source file
        if( isValid && isLink == Attachment::YES )
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
            BASE::KeySet<LogEntry> entries( attachment );
            Q_ASSERT( entries.size() == 1 );
            LogEntry& entry( **entries.begin() );
            entry.modified();

            // get associated logbooks
            BASE::KeySet<Logbook> logbooks( &entry );
            foreach( Logbook* logbook, logbooks ) logbook->setModified( true );

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
        mainwindow.setModified( true );
        if( mainwindow.logbook()->file().size() ) mainwindow.save();

    }

    cleanAction().setEnabled( hasInvalidRecords );
    return;

}

//_____________________________________________
void AttachmentFrame::_updateConfiguration( void )
{

    Debug::Throw( "AttachmentFrame::_updateConfiguration.\n" );
    int icon_size( XmlOptions::get().get<int>( "ATTACHMENT_LIST_ICON_SIZE" ) );
    list_->setIconSize( QSize( icon_size, icon_size ) );

}

//_____________________________________________
void AttachmentFrame::_updateActions( void )
{

    bool hasSelection( !list_->selectionModel()->selectedRows().isEmpty() );
    newAction().setEnabled( !readOnly() );
    openAction().setEnabled( hasSelection );
    saveAsAction().setEnabled( hasSelection );
    reloadAction().setEnabled( hasSelection );
    editAction().setEnabled( hasSelection );
    deleteAction().setEnabled( hasSelection && !readOnly() );
    return;

}

//_____________________________________________
void AttachmentFrame::_open( void )
{
    Debug::Throw( "AttachmentFrame::_open.\n" );

    // get selection
    AttachmentModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Open Attachment> canceled." ) ).exec();
        return;
    }

    // loop over attachments
    AttachmentModel::List modifiedAttachments;
    foreach( Attachment* attachment, selection )
    {

        if( attachment->updateTimeStamps() ) modifiedAttachments << attachment;

        AttachmentType type = attachment->type();
        File fullname( ( type == AttachmentType::URL ) ? attachment->file():attachment->file().expand() );
        if( !( type == AttachmentType::URL || fullname.exists() ) )
        {
            InformationDialog( this, QString( tr( "Cannot find file '%1'. <Open Attachment> canceled." ) ).arg( fullname ) ).exec();
            continue;
        }

        OpenAttachmentDialog dialog( this, *attachment );
        if( dialog.centerOnWidget( window() ).exec() == QDialog::Accepted )
        {
            if( dialog.action() == OpenAttachmentDialog::OPEN ) ( Command( dialog.command() ) << fullname ).run();
            else  {

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
    AttachmentModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Edit Attachment> canceled." ) ).exec();
        return;
    }

    // loop over attachments
    AttachmentModel::List modifiedAttachments;
    foreach( Attachment* attachment, selection )
    {

        // update time stamps
        bool attachmentChanged( false );
        attachmentChanged |= attachment->updateTimeStamps();
        EditAttachmentDialog dialog( this, *attachment );

        // map dialog
        if( dialog.centerOnWidget( window() ).exec() == QDialog::Accepted )
        {

            // change attachment type
            attachmentChanged |= attachment->setType( dialog.type() );

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
    AttachmentModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Delete Attachment> canceled." ) ).exec();
        return;
    }

    // retrieve/check associated EditionWindow/LogEntry
    BASE::KeySet<EditionWindow> windows( this );
    Q_ASSERT( windows.size() == 1 );
    EditionWindow &window( **windows.begin() );

    // loop over attachments
    bool logbookChanged( false );
    foreach( Attachment* attachment, selection )
    {

        // dialog
        DeleteAttachmentDialog dialog( this, *attachment );
        if( dialog.centerOnWidget( AttachmentFrame::window() ).exec() == QDialog::Accepted )
        {

            logbookChanged = true;

            // retrieve action
            bool fromDisk( dialog.action() == DeleteAttachmentDialog::FROM_DISK );

            // retrieve associated attachment frames and remove item
            BASE::KeySet<AttachmentFrame> frames( attachment );
            foreach( AttachmentFrame* frame, frames ) frame->model_.remove( attachment );

            // retrieve associated entries
            BASE::KeySet<LogEntry> entries( attachment );
            Q_ASSERT( entries.size() == 1 );
            LogEntry& entry( **entries.begin() );
            entry.modified();

            // retrieve associated logbooks
            BASE::KeySet<Logbook> logbooks( &entry );

            // check sharing attachments to avoid fromDisk deletion
            if( fromDisk && logbooks.size() )
            {

                BASE::KeySet<Attachment> attachments( (*logbooks.begin())->attachments() );
                unsigned int n_share = std::count_if( attachments.begin(), attachments.end(), Attachment::SameFileFTor( attachment ) );
                if( n_share > 1 ) {

                    InformationDialog( this, tr( "Attachment still in use by other entries. Kept on disk." ) ).exec();
                    fromDisk = false;

                }

            }

            // remove file from disk, if required
            File file( attachment->file().expand() );
            if( fromDisk && ( !( attachment->type() == AttachmentType::URL ) ) && file.isWritable() )
            { file.remove(); }

            // delete attachment
            delete attachment;

        }

    }

    if( logbookChanged )
    {
        Singleton::get().application<Application>()->mainWindow().setModified( true );
        window.saveAction().trigger();

        // resize columns
        list_->resizeColumns();
    }

    return;

}

//_____________________________________________
void AttachmentFrame::_reload( void )
{
    Debug::Throw( "AttachmentFrame::_reload.\n" );

    // get selection
    AttachmentModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Save Attachment As> canceled." ) ).exec();
        return;
    }

    // loop over attachments
    AttachmentModel::List modifiedAttachments;
    foreach( Attachment* attachment, selection )
    { if( attachment->updateTimeStamps() ) modifiedAttachments << attachment; }

    _saveAttachments( modifiedAttachments );

}

//_____________________________________________
void AttachmentFrame::_saveAs( void )
{
    Debug::Throw( "AttachmentFrame::_saveAs.\n" );

    // get selection
    AttachmentModel::List selection( model_.get( list_->selectionModel()->selectedRows() ) );

    // check items
    if( selection.empty() )
    {
        InformationDialog( this, tr( "No attachment selected. <Save Attachment As> canceled." ) ).exec();
        return;
    }

    // loop over attachments
    foreach( Attachment* attachment, selection )
    {

        const AttachmentType type = attachment->type();
        File fullname( ( type == AttachmentType::URL ) ? attachment->file():attachment->file().expand() );
        if( type == AttachmentType::URL )
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
    foreach( Attachment* attachment, model_.get() )
    {

        // skip attachment if valid
        if( attachment->isValid() ) continue;

        Debug::Throw() << "AttachmentFrame::_clean - removing: " << attachment->file() << endl;

        // retrieve associated attachment frames and remove item
        BASE::KeySet<AttachmentFrame> frames( attachment );
        foreach( AttachmentFrame* frame, frames ) frame->model_.remove( attachment );

        // retrieve associated entries
        BASE::KeySet<LogEntry> entries( attachment );
        Q_ASSERT( entries.size() == 1 );
        LogEntry& entry( **entries.begin() );
        entry.modified();

        // retrieve associated logbooks
        BASE::KeySet<Logbook> logbooks( &entry );
        foreach( Logbook* logbook, logbooks )
        { logbook->setModified( true ); }

        // delete attachment
        delete attachment;
        modified = true;
    }

    if( modified )
    {

        // set main window title
        MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
        mainwindow.setModified( true );
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

    addAction( visibilityAction_ = new QAction( IconEngine::get( ICONS::ATTACH ), tr( "Show &Attachment List" ), this ) );
    visibilityAction().setToolTip( tr( "Show/hide attachment list" ) );
    visibilityAction().setCheckable( true );
    visibilityAction().setChecked( true );
    connect( &visibilityAction(), SIGNAL( toggled( bool ) ), SLOT( setVisible( bool ) ) );

    addAction( newAction_ = new QAction( IconEngine::get( ICONS::ATTACH ), tr( "New" ), this ) );
    newAction().setToolTip( tr( "Attach a file/URL to the current entry" ) );
    connect( &newAction(), SIGNAL( triggered() ), SLOT( _new() ) );

    addAction( openAction_ = new QAction( IconEngine::get( ICONS::OPEN ), tr( "Open" ), this ) );
    openAction().setToolTip( tr( "Open selected attachments" ) );
    connect( &openAction(), SIGNAL( triggered() ), SLOT( _open() ) );

    addAction( editAction_ = new QAction( IconEngine::get( ICONS::EDIT ), tr( "Edit" ), this ) );
    editAction().setToolTip( tr( "Edit selected attachments informations" ) );
    connect( &editAction(), SIGNAL( triggered() ), SLOT( _edit() ) );

    addAction( deleteAction_ = new QAction( IconEngine::get( ICONS::DELETE ), tr( "Delete" ), this ) );
    deleteAction().setShortcut( QKeySequence::Delete );
    deleteAction().setToolTip( tr( "Delete selected attachments" ) );
    connect( &deleteAction(), SIGNAL( triggered() ), SLOT( _delete() ) );

    addAction( reloadAction_ = new QAction( IconEngine::get( ICONS::RELOAD ), tr( "Reload" ), this ) );
    reloadAction().setShortcut( QKeySequence::Refresh );
    reloadAction().setToolTip( tr( "Reload attachments timestamps" ) );
    connect( &reloadAction(), SIGNAL( triggered() ), SLOT( _reload() ) );

    addAction( saveAsAction_ = new QAction( IconEngine::get( ICONS::SAVE_AS ), tr( "Save As" ), this ) );
    saveAsAction().setToolTip( tr( "Save selected attachment with a different filename" ) );
    connect( &saveAsAction(), SIGNAL( triggered() ), SLOT( _saveAs() ) );


    cleanAction_ = new QAction( IconEngine::get( ICONS::DELETE ), tr( "Clean" ), this );
    cleanAction().setToolTip( tr( "Delete selected attachments" ) );
    connect( &cleanAction(), SIGNAL( triggered() ), SLOT( _clean() ) );
}

//_______________________________________________________________________
void AttachmentFrame::_saveAttachments( const AttachmentModel::List& attachments )
{
    Debug::Throw( "AttachmentFrame::_saveAttachments.\n" );
    if( attachments.empty() ) return;

    // associated lists
    BASE::KeySet<LogEntry> entries;

    // loop over attachments
    foreach( Attachment* attachment, attachments )
    {

        // get associated entries and store
        entries.unite( BASE::KeySet<LogEntry>( attachment ) );
        Debug::Throw( "AttachmentFrame::_saveAttachments - entries.\n" );

        // get associated attachment frames and store
        BASE::KeySet<AttachmentFrame> localFrames( attachment );
        foreach( AttachmentFrame* frame, localFrames )
        { frame->update( *attachment ); }

        Debug::Throw( "AttachmentFrame::_saveAttachments - frames.\n" );

    }

    BASE::KeySet<Logbook> logbooks;
    BASE::KeySet<EditionWindow> editionWindows;

    // loop over entries
    foreach( LogEntry* entry, entries )
    {

        // get associated logbooks and store
        logbooks.unite( BASE::KeySet<Logbook>( entry ) );

        Debug::Throw( "AttachmentFrame::_saveAttachments - logbooks.\n" );

        // get associated Edition windows
        editionWindows.unite( BASE::KeySet<EditionWindow>( entry ) );
        Debug::Throw( "AttachmentFrame::_saveAttachments - edition windows.\n" );

    }

    // loop over logbook and set modified
    foreach( Logbook* logbook, logbooks ) logbook->setModified( true );
    Debug::Throw( "AttachmentFrame::_saveAttachments - logbooks modified.\n" );

    // loop over edition windows and trigger save action
    foreach( EditionWindow* window, editionWindows ) window->saveAction().trigger();
    Debug::Throw( "AttachmentFrame::_saveAttachments - edition windows saved.\n" );

    MainWindow& mainwindow( Singleton::get().application<Application>()->mainWindow() );
    if( mainwindow.logbook()->file().size() ) mainwindow.save();
    Debug::Throw( "AttachmentFrame::_saveAttachments - logbook saved.\n" );

}
