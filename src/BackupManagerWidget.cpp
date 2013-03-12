// $Id$

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public license as published by the Free Software
* Foundation; either version 2 of the license, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public license
* for more details.
*
* You should have received a copy of the GNU General Public license along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "BackupManagerWidget.h"

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
    list_->setSelectionMode( QAbstractItemView::SingleSelection );
    list_->setOptionName( "BACKUP_MANAGER_LIST" );
    list_->setItemMargin( 2 );

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

    Logbook::Backup::List backups( logbook->backupFiles() );
    backups.checkValidity();

    // clear model
    model_.set( backups );
    list_->resizeColumns();

    // update clean button
    cleanButton_->setEnabled( std::find_if( backups.begin(), backups.end(), Logbook::Backup::InvalidFTor() ) != backups.end() );

}

//____________________________________________________________________________
void BackupManagerWidget::_clean( void )
{
    Debug::Throw( "BackupManagerWidget::_clean.\n" );

    Logbook* logbook( _logbook() );
    if( !logbook ) return;

    Logbook::Backup::List backups( model_.get() );
    backups.erase( std::remove_if( backups.begin(), backups.end(), Logbook::Backup::InvalidFTor() ), backups.end() );
    logbook->setBackupFiles( backups );

    // notify logbook modification
    if( logbook->modified() && !logbook->file().isEmpty() )
    { emit saveLogbookRequested(); }

}

//____________________________________________________________________________
void BackupManagerWidget::_remove( void )
{
    Debug::Throw( "BackupManagerWidget::_remove.\n" );
    QModelIndex index( list_->selectionModel()->currentIndex() );
    if( !index.isValid() ) return;

    const Logbook::Backup& backup( model_.get( index ) );

    // ask confirmation
    QString buffer = QString(
        tr( "Remove backup file named '%1' ?\n"
        "Warning: this will permanently delete the files on disk and the corresponding data." ) )
        .arg( backup.file().localName() );

    if( !QuestionDialog( this, buffer ).exec() ) return;
    emit removeBackupRequested( backup );
}

//____________________________________________________________________________
void BackupManagerWidget::_restore( void )
{
    Debug::Throw( "BackupManagerWidget::_restore.\n" );
    QModelIndex index( list_->selectionModel()->currentIndex() );
    if( !index.isValid() ) return;

    const Logbook::Backup& backup( model_.get( index ) );

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

    const Logbook::Backup& backup( model_.get( index ) );

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

//_______________________________________________
const QString BackupManagerWidget::Model::columnTitles_[ BackupManagerWidget::Model::nColumns ] =
{
    tr( "File" ),
    tr( "Path" ),
    tr( "Created" )
};

//__________________________________________________________________
Qt::ItemFlags BackupManagerWidget::Model::flags(const QModelIndex &index) const
{

    // default flags
    Qt::ItemFlags flags;
    if( !index.isValid() ) return flags;

    // check associated record validity
    const Logbook::Backup& backup( get(index) );
    if( !backup.isValid() ) return flags;

    // default flags
    flags |=  Qt::ItemIsEnabled |  Qt::ItemIsSelectable;

    return flags;

}

//_______________________________________________
QVariant BackupManagerWidget::Model::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    const Logbook::Backup backup( get( index ) );
    if( role == Qt::DisplayRole )
    {
        switch( index.column() )
        {
            case FILE: return backup.file().localName();
            case PATH: return backup.file().path();
            case CREATION: return backup.creation().toString();

            default:
            return QVariant();
        }
    }

    return QVariant();

}

//__________________________________________________________________
QVariant BackupManagerWidget::Model::headerData(int section, Qt::Orientation orientation, int role) const
{

    if(
        orientation == Qt::Horizontal &&
        role == Qt::DisplayRole &&
        section >= 0 &&
        section < nColumns )
    { return columnTitles_[section]; }

    // return empty
    return QVariant();

}

//____________________________________________________________
void BackupManagerWidget::Model::_sort( int column, Qt::SortOrder order )
{ std::sort( _get().begin(), _get().end(), SortFTor( (ColumnType) column, order ) ); }

//________________________________________________________
bool BackupManagerWidget::Model::SortFTor::operator () ( Logbook::Backup first, Logbook::Backup second ) const
{

    if( order_ == Qt::DescendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        case FILE: return first.file().localName() < second.file().localName();
        case PATH: return first.file().path() < second.file().path();
        case CREATION: return first.creation() < second.creation();
        default: return true;

    }

}
