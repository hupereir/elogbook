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

#include "DeleteAttachmentDialog.h"
#include "Debug.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "QtUtil.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

//_____________________________________________________
DeleteAttachmentDialog::DeleteAttachmentDialog( QWidget* parent, const Attachment& attachment ):
    Dialog( parent, OkButton | CancelButton| Separator )
{

    Debug::Throw( QStringLiteral("DeleteAttachmentDialog::DeleteAttachmentDialog.\n") );

    // radio buttons
    QButtonGroup* group = new QButtonGroup( this );
    group->setExclusive( true );

    QHBoxLayout *hLayout( new QHBoxLayout );
    hLayout->setSpacing(10);
    QtUtil::setMargin(hLayout, 0);
    mainLayout().addLayout( hLayout );

    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::DialogWarning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( tr( "Delete attachment ?" ), this ) );

    QWidget *groupBox = new QWidget( this );
    mainLayout().addWidget( groupBox );
    groupBox->setLayout( new QVBoxLayout );
    QtUtil::setMargin(groupBox->layout(), 5);
    groupBox->layout()->setSpacing(5);

    groupBox->layout()->addWidget( fromDiskButton_ = new QRadioButton( tr( "From disk" ), groupBox ) );
    fromDiskButton_->setChecked( true );
    fromDiskButton_->setToolTip( tr( "Select this button to remove attachment file from disk and logbook" ) );
    group->addButton( fromDiskButton_ );

    groupBox->layout()->addWidget( fromLogbookButton_ = new QRadioButton( QStringLiteral("From frame"), groupBox ) );
    fromLogbookButton_->setToolTip( tr( "Select this button to remove attachment file from logbook only (attachment is kept on disk)" ) );
    group->addButton( fromLogbookButton_ );

    if( attachment.isUrl() )
    { groupBox->setEnabled( false ); }

    okButton().setText( tr( "Delete" ) );
    okButton().setIcon( IconEngine::get( IconNames::Delete ) );

    adjustSize();

}

//______________________________________________________
DeleteAttachmentDialog::Action DeleteAttachmentDialog::action() const
{ return fromDiskButton_->isChecked() ? Action::DeleteFromDisk : Action::DeleteFromLogbook; }
