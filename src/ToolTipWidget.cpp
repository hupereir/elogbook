/******************************************************************************
*
* Copyright (C) 2017 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
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

#include "ToolTipWidget.h"
#include "Application.h"
#include "Debug.h"
#include "GridLayout.h"
#include "GridLayoutItem.h"
#include "QtUtil.h"
#include "Singleton.h"
#include "TimeStamp.h"
#include "XmlOptions.h"


#include <QFrame>
#include <QLayout>

//_______________________________________________________
ToolTipWidget::ToolTipWidget( QWidget* parent ):
    BaseToolTipWidget( parent )
{

    Debug::Throw( QStringLiteral("ToolTipWidget::ToolTipWidget.\n") );

    setFollowMouse( true );
 
    auto vLayout = new QVBoxLayout;
    QtUtil::setMargin(vLayout, 10);
    vLayout->setSpacing( 5 );
    setLayout( vLayout );
 
    // title
    vLayout->addWidget( titleLabel_ = new QLabel( this ) );
    titleLabel_->setAlignment( Qt::AlignCenter );
    titleLabel_->setFont( QtUtil::titleFont( titleLabel_->font() ) );
    QtUtil::setMargin(titleLabel_, 1);

    // separator
    auto separator = new QFrame( this );
    separator->setFrameStyle( QFrame::HLine );
    vLayout->addWidget( separator );

    // grid layout
    auto gridLayout = new GridLayout;
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
    gridLayout->setColumnAlignment( 1, Qt::AlignVCenter|Qt::AlignLeft );
    QtUtil::setMargin(gridLayout, 0);
    gridLayout->setSpacing( 5 );
    vLayout->addLayout( gridLayout );

    // items
    ( authorItem_ = new GridLayoutItem( this, gridLayout ) )->setKey( tr( "Author:" ) );
    ( createdItem_ = new GridLayoutItem( this, gridLayout ) )->setKey( tr( "Created:" ) );
    ( modifiedItem_ = new GridLayoutItem( this, gridLayout ) )->setKey( tr( "Modified:" ) );
}

//_______________________________________________________
void ToolTipWidget::setLogEntry( LogEntry* entry )
{
    Debug::Throw( QStringLiteral("ToolTipWidget::setLogEntry.\n") );
    if( entry )
    {
        titleLabel_->setText( entry->title() );
        authorItem_->setText( entry->author() );
        createdItem_->setText( entry->creation().toString() );
        modifiedItem_->setText( entry->modification().toString() );
    }
}
