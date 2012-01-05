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
#include "TreeView.h"

#include <QtGui/QLayout>

//____________________________________________________________________________
BackupManagerWidget::BackupManagerWidget( QWidget* parent, Logbook* logbook ):
    QWidget( parent ),
    Counter( "BackupManagerWidget" )
{
    Debug::Throw( "BackupManagerWidget::BackupManagerWidget" );
    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    setLayout( vLayout );

    // add tree
    vLayout->addWidget( list_ = new TreeView( this ) );
    list_->setModel( &model_ );
    list_->setSelectionMode( QAbstractItemView::ContiguousSelection );

    if( logbook ) Key::associate( this, logbook );

}

//____________________________________________________________________________
void BackupManagerWidget::updateBackups( void )
{
    Debug::Throw( "BackupManagerWidget::updateBackups" );
    model_.clear();

    // check associated logbook
    Logbook* logbook( _logbook() );
    if( !logbook ) return;

    // clear model
    model_.set( logbook->backupFiles() );
    list_->resizeColumns();
}

//_______________________________________________
const QString BackupManagerWidget::Model::columnTitles_[ BackupManagerWidget::Model::nColumns ] =
{
    "File",
    "Path",
    "Created"
};

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

    if( order_ == Qt::AscendingOrder ) std::swap( first, second );

    switch( type_ )
    {

        case FILE: return first.file().localName() < second.file().localName();
        case PATH: return first.file().path() < second.file().path();
        case CREATION: return first.creation() < second.creation();
        default: return true;

    }

}
