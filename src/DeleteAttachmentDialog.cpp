
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
* ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA   02111-1307 USA
*
*
*******************************************************************************/

#include "DeleteAttachmentDialog.h"
#include "Debug.h"
#include "Icons.h"
#include "IconEngine.h"
#include "PixmapEngine.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

//_____________________________________________________
DeleteAttachmentDialog::DeleteAttachmentDialog( QWidget* parent, const Attachment& attachment ):
    CustomDialog( parent, OkButton | CancelButton| Separator )
{

    Debug::Throw( "DeleteAttachmentDialog::DeleteAttachmentDialog.\n" );

    // radio buttons
    QButtonGroup* group = new QButtonGroup( this );
    group->setExclusive( true );

    //! try load Question icon
    QPixmap questionPixmap = PixmapEngine::get( ICONS::WARNING );
    if( questionPixmap.isNull() )
    {

        mainLayout().addWidget( new QLabel( "Delete attachment ?", this ) );

    } else {

        QHBoxLayout *hLayout( new QHBoxLayout() );
        hLayout->setSpacing(10);
        hLayout->setMargin(0);
        mainLayout().addLayout( hLayout );

        QLabel* label = new QLabel( this );
        label->setPixmap( questionPixmap );
        hLayout->addWidget( label, 0, Qt::AlignHCenter );

        hLayout->addWidget( new QLabel( "Delete attachment ?", this ) );

    }

    QWidget *groupBox = new QWidget( this );
    mainLayout().addWidget( groupBox );
    groupBox->setLayout( new QVBoxLayout() );
    groupBox->layout()->setMargin(5);
    groupBox->layout()->setSpacing(5);

    groupBox->layout()->addWidget( fromDiskButton_ = new QRadioButton( "From disk", groupBox ) );
    fromDiskButton_->setChecked( true );
    fromDiskButton_->setToolTip( "Select this button to remove attachment file from disk and logbook." );
    group->addButton( fromDiskButton_ );

    groupBox->layout()->addWidget( fromLogbookButton_ = new QRadioButton( "From frame", groupBox ) );
    fromLogbookButton_->setToolTip( "Select this button to remove attachment file from logbook only (attachment is kept on disk)." );
    group->addButton( fromLogbookButton_ );

    if( attachment.type() == AttachmentType::URL )
    { groupBox->setEnabled( false ); }

    okButton().setText( "Delete" );
    okButton().setIcon( IconEngine::get( ICONS::DELETE ) );

    adjustSize();

}

//______________________________________________________
DeleteAttachmentDialog::Action DeleteAttachmentDialog::action( void ) const
{ return fromDiskButton_->isChecked() ? FROM_DISK:FROM_LOGBOOK; }
