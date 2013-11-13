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

#include "BackupManagerWidget.h"
#include "BackupManagerWidget.moc"

#include "Icons.h"
#include "IconEngine.h"
#include "QuestionDialog.h"
#include "TreeView.h"

//____________________________________________________________________________
BackupManagerWidget::BackupManagerWidget( QWidget* parent, Logbook* logbook ):
    QWidget( parent ),
    Counter( "BackupManagerWidget" )
{
    Debug::Throw( "BackupManagerWidget::BackupManagerWidget" );
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    setLayout( hLayout );

    // add tree
    hLayout->addWidget( list_ = new TreeView( this ) );
    list_->setModel( &model_ );
    list_->setSelectionMode( QAbstractItemView::ContiguousSelection );
    list_->setOptionName( "BACKUP_MANAGER_LIST" );

    // buttons
    buttonLayout_ = new QVBoxLayout();
    buttonLayout_->setMargin(0);
    hLayout->addLayout( buttonLayout_ );

    buttonLayout_->addWidget( newBackupButton_ = new QPushButton( IconEngine::get( ICONS::ADD ), tr( "New" ), this ) );
    buttonLayout_->addWidget( removeButton_ = new QPushButton( IconEngine::get( ICONS::REMOVE ), tr( "Remove" ), this ) );
    buttonLayout_->addWidget( restoreButton_ = new QPushButton( IconEngine::get( ICONS::UNDO ), tr( "Restore" ), this ) );
    buttonLayout_->addWidget( mergeButton_ = new QPushButton( IconEngine::get( ICONS::MERGE ), tr( "Merge" ), this ) );

    QFrame* frame = new QFrame( this );
    frame->setFrameStyle( QFrame::HLine );
    buttonLayout_->addWidget( frame );

    buttonLayout_->addWidget( cleanButton_ = new QPushButton( IconEngine::get( ICONS::DELETE ), tr( "Clean" ), this ) );

    buttonLayout_->addStretch( 1 );

    // connections
    connect( list_->selectionModel(), SIGNAL( selectionChanged(const QItemSelection &, const QItemSelection &) ), SLOT( _updateActions( void ) ) );
    connect( cleanButton_, SIGNAL( clicked( void ) ), SLOT( _clean( void ) ) );
    connect( cleanButton_, SIGNAL( clicked( void ) ), SLOT( updateBackups( void ) ) );

    connect( removeButton_, SIGNAL( clicked( void ) ), SLOT( _remove( void ) ) );
    connect( removeButton_, SIGNAL( clicked( void ) ), SLOT( updateBackups( void ) ) );

    connect( restoreButton_, SIGNAL( clicked( void ) ), SLOT( _restore( void ) ) );
    connect( restoreButton_, SIGNAL( clicked( void ) ), SLOT( updateBackups( void ) ) );

    connect( mergeButton_, SIGNAL( clicked( void ) ), SLOT( _merge( void ) ) );
    connect( mergeButton_, SIGNAL( clicked( void ) ), SLOT( updateBackups( void ) ) );

    connect( newBackupButton_, SIGNAL( clicked( void ) ), SIGNAL( backupRequested( void ) ) );
    connect( newBackupButton_, SIGNAL( clicked( void ) ), SLOT( updateBackups( void ) ) );

    if( logbook ) Key::associate( this, logbook );

    _updateActions();

}

//____________________________________________________________________________
void BackupManagerWidget::updateBackups( void )
{
    Debug::Throw( "BackupManagerWidget::updateBackups" );
    model_.clear();

    // check associated logbook
    Logbook* logbook( _logbook() );
    if( !logbook ) return;

    Backup::List backups( logbook->backupFiles() );
    backups.checkValidity();

    // clear model
    model_.set( backups );
    list_->resizeColumns();

    // update clean button
    cleanButton_->setEnabled( std::find_if( backups.begin(), backups.end(), Backup::InvalidFTor() ) != backups.end() );

}

//____________________________________________________________________________
void BackupManagerWidget::_clean( void )
{
    Debug::Throw( "BackupManagerWidget::_clean.\n" );

    Logbook* logbook( _logbook() );
    if( !logbook ) return;

    Backup::List backups( model_.get() );
    backups.erase( std::remove_if( backups.begin(), backups.end(), Backup::InvalidFTor() ), backups.end() );
    logbook->setBackupFiles( backups );

    // notify logbook modification
    if( logbook->modified() && !logbook->file().isEmpty() )
    { emit saveLogbookRequested(); }

}

//____________________________________________________________________________
void BackupManagerWidget::_remove( void )
{
    Debug::Throw( "BackupManagerWidget::_remove.\n" );
    QModelIndexList selectedIndexes( list_->selectionModel()->selectedRows() );
    if( selectedIndexes.isEmpty() ) return;

    QString buffer;
    if( selectedIndexes.size() == 1 )
    {

        buffer = QString(
            tr( "Permanently delete the backup named '%1' ?\n"
            "Warning: this will permanently delete the files on disk and the corresponding data." ) )
            .arg( model_.get( selectedIndexes.front() ).file().localName() );

    } else {

        buffer = QString(
            tr( "Permanently delete %1 backups ?\n"
            "Warning: this will permanently delete the files on disk and the corresponding data." ) )
            .arg( selectedIndexes.size() );

    }

    if( !QuestionDialog( this, buffer ).exec() ) return;

    // ask for deletion of all selected backups
    foreach( const QModelIndex& index, selectedIndexes )
    { emit removeBackupRequested( model_.get( index ) ); }

}

//____________________________________________________________________________
void BackupManagerWidget::_restore( void )
{
    Debug::Throw( "BackupManagerWidget::_restore.\n" );
    QModelIndex index( list_->selectionModel()->currentIndex() );
    if( !index.isValid() ) return;

    const Backup& backup( model_.get( index ) );

    // ask confirmation
    QString buffer = QString(
        tr( "Restore data from backup file named '%1' ?\n"
        "Warning: this will permanently erase all data from the current logbook." ) )
        .arg( backup.file().localName() );
    if( !QuestionDialog( this, buffer ).exec() ) return;
    emit restoreBackupRequested( backup );
}

//____________________________________________________________________________
void BackupManagerWidget::_merge( void )
{
    Debug::Throw( "BackupManagerWidget::_merge.\n" );
    QModelIndex index( list_->selectionModel()->currentIndex() );
    if( !index.isValid() ) return;

    const Backup& backup( model_.get( index ) );

    // ask confirmation
    QString buffer = QString(
        tr( "Merge backup file named '%1' to current logbook ?\n"
        "Warning: this will add all unmatched entry in the backup to the current logbook." ) )
        .arg( backup.file().localName()  );
    if( !QuestionDialog( this, buffer ).exec() ) return;
    emit mergeBackupRequested( backup );
}

//____________________________________________________________________________
void BackupManagerWidget::_updateActions( void )
{
    Debug::Throw( "BackupManagerWidget::_updateActions" );
    bool hasSelection( !list_->selectionModel()->selectedRows().isEmpty() );
    removeButton_->setEnabled( hasSelection );
    restoreButton_->setEnabled( hasSelection );
    mergeButton_->setEnabled( hasSelection );
}
