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

#include "BackupManagerDialog.h"
#include "BackupManagerWidget.h"
#include "Icons.h"
#include "IconEngine.h"

//__________________________________________________________________________________
BackupManagerDialog::BackupManagerDialog( QWidget* parent ):
   BaseDialog( parent )
{
    Debug::Throw( "BackupManagerDialog::BackupManagerDialog.\n" );
    setWindowTitle( tr( "Backup Manager - Elogbook" ) );
    setOptionName( "BACKUP_MANAGER_DIALOG" );

    setLayout( new QVBoxLayout() );
    layout()->setMargin(5);
    layout()->addWidget( managerWidget_ = new BackupManagerWidget( this ) );

    // add close button
    QPushButton* closeButton = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), tr( "Close" ), managerWidget_ );
    connect( closeButton, SIGNAL( clicked() ), this, SLOT( close() ) );
    managerWidget_->buttonLayout().addWidget( closeButton );

}
