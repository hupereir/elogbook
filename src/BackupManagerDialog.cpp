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

#include "BackupManagerDialog.h"
#include "BackupManagerWidget.h"
#include "Icons.h"
#include "IconEngine.h"

//__________________________________________________________________________________
BackupManagerDialog::BackupManagerDialog( QWidget* parent ):
   BaseDialog( parent )
{
    Debug::Throw( "BackupManagerDialog::BackupManagerDialog.\n" );
    setWindowTitle( "Backup Manager - elogbook" );
    setOptionName( "BACKUP_MANAGER_DIALOG" );

    setLayout( new QVBoxLayout() );
    layout()->setMargin(5);
    layout()->addWidget( managerWidget_ = new BackupManagerWidget( this ) );

    // add close button
    QPushButton* closeButton = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "Close", managerWidget_ );
    connect( closeButton, SIGNAL( clicked() ), this, SLOT( close() ) );
    managerWidget_->buttonLayout().addWidget( closeButton );

}
