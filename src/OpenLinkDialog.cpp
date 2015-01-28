
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

#include "OpenLinkDialog.h"
#include "CustomComboBox.h"
#include "Debug.h"
#include "ElidedLabel.h"
#include "FileDialog.h"
#include "IconEngine.h"
#include "IconNames.h"

#include <QGridLayout>
#include <QButtonGroup>
#include <QToolButton>
#include <QLabel>

//___________________________________________________________________________
OpenLinkDialog::OpenLinkDialog( QWidget* parent, const QString& file ):
    CustomDialog( parent, OkButton|CancelButton|Separator )
{

    Debug::Throw( "OpenLinkDialog::OpenLinkDialog.\n" );
    setOptionName( "OPEN_LINK_DIALOG" );

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setMargin(0);
    mainLayout().addLayout( gridLayout );

    QLabel* label;
    gridLayout->addWidget( label = new QLabel( tr( "Opening link:" ), this ), 0, 0, 1, 1 );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );

    gridLayout->addWidget( label = new ElidedLabel( file, this ), 0, 1, 1, 2 );

    // change font
    QFont font( label->font() );
    font.setStyle( QFont::StyleItalic );
    label->setFont( font );

    gridLayout->addWidget( label = new QLabel( tr( "Command:" ), this ), 1, 0, 1, 1 );
    gridLayout->addWidget( actionComboBox_ = new CustomComboBox( this ), 1, 1, 1, 1 );
    actionComboBox_->setEditable( true );
    actionComboBox_->setAutoCompletion( true );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    label->setBuddy( actionComboBox_ );

    QToolButton* button;
    gridLayout->addWidget( button = new QToolButton( this ), 1, 2, 1, 1 );
    button->setIcon( IconEngine::get( IconNames::Open ) );
    button->setAutoRaise( true );
    connect( button, SIGNAL(clicked()), SLOT(_browse()) );
    gridLayout->setColumnStretch( 1, 1 );

}

//______________________________________________________________________
void OpenLinkDialog::_browse( void )
{
    Debug::Throw( "OpenLinkDialog::_browse.\n" );

    // create file dialog
    QString file( FileDialog( this ).getFile() );
    if( !file.isNull() ) actionComboBox().setEditText( file );
    return;
}
