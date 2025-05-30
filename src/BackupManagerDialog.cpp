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
#include "IconEngine.h"
#include "IconNames.h"
#include "QtUtil.h"
#include "TreeView.h"

//__________________________________________________________________________________
BackupManagerDialog::BackupManagerDialog( QWidget* parent ):
   BaseDialog( parent )
{
    Debug::Throw( QStringLiteral("BackupManagerDialog::BackupManagerDialog.\n") );
    setWindowTitle( tr( "Backup Manager" ) );
    setOptionName( QStringLiteral("BACKUP_MANAGER_DIALOG") );

    setLayout( new QVBoxLayout );
    QtUtil::setMargin(layout(), 0);
    layout()->addWidget( managerWidget_ = new BackupManagerWidget( this ) );

    QtUtil::setMargin(managerWidget_->layout(),0);
    QtUtil::setWidgetSides(&managerWidget_->list(), Qt::RightEdge|Qt::TopEdge);

    managerWidget_->layout()->setSpacing(0);
    QtUtil::setMargin(&managerWidget_->buttonLayout(),5);

    // add close button
    auto closeButton = new QPushButton( IconEngine::get( IconNames::DialogClose ), tr( "Close" ), managerWidget_ );
    connect( closeButton, &QAbstractButton::clicked, this, &QWidget::close );
    managerWidget_->buttonLayout().addWidget( closeButton );

}
