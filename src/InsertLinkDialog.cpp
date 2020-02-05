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

#include "InsertLinkDialog.h"
#include "Debug.h"

#include <QLabel>
#include <QLayout>

//_________________________________________________________
InsertLinkDialog::InsertLinkDialog( QWidget* parent, QString value ):
CustomDialog( parent, OkButton|CancelButton|Separator )
{
    Debug::Throw( QStringLiteral("InsertLinkDialog::InsertLinkDialog.\n") );
    setOptionName( "INSERT_LINK_DIALOG" );
    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    mainLayout().addLayout( hLayout );
    QLabel* label;

    hLayout->addWidget( label = new QLabel( tr( "Link:" ), this ) );
    hLayout->addWidget( editor_ = new BrowsedLineEditor( this ) );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    label->setBuddy( editor_ );

    editor_->editor().setText( value );
}
