
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

#include "AskForSaveDialog.h"
#include "AskForSaveDialog.moc"
#include "IconNames.h"
#include "IconEngine.h"
#include "XmlOptions.h"
#include "QtUtil.h"

#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
AskForSaveDialog::AskForSaveDialog( QWidget* parent, const QString& message, const unsigned int& buttons ):
    BaseDialog( parent ),
    Counter( "AskForSaveDialog" )
{

    Debug::Throw( "AskForSaveDialog::AskForSaveDialog.\n" );
    setWindowModality( Qt::ApplicationModal );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout();
    layout->setSpacing(5);
    layout->setMargin(10);
    setLayout( layout );

    QHBoxLayout *hLayout( new QHBoxLayout() );
    hLayout->setSpacing(10);
    hLayout->setMargin(10);
    layout->addLayout( hLayout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::Warning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( message, this ), 1, Qt::AlignHCenter );

    // horizontal separator
    QFrame* frame( new QFrame( this ) );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( frame );

    // button layout
    QHBoxLayout *button_layout = new QHBoxLayout();
    button_layout->setMargin(0);
    button_layout->setSpacing(5);
    layout->addLayout( button_layout );
    button_layout->addStretch(1);

    // yes button
    QPushButton* button;
    if( buttons & YES )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogOk ), tr( "Yes" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_yes()) );
    }

    // yes to all button
    if( buttons & ALL )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogOk ), tr( "Yes to All" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_all()) );
    }

    // no button
    if( buttons & NO )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogClose ), tr( "No" ), this ) );
        connect( button, SIGNAL(clicked()), SLOT(_no()) );
    }

    // cancel button
    if( buttons & CANCEL )
    {
        button_layout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogCancel ), tr( "Cancel" ), this ) );
        button->setShortcut( Qt::Key_Escape );
        connect( button, SIGNAL(clicked()), SLOT(_cancel()) );
    }

    adjustSize();

}
