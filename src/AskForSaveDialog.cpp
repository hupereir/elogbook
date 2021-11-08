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
#include "IconNames.h"
#include "IconEngine.h"
#include "XmlOptions.h"
#include "QtUtil.h"

#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
AskForSaveDialog::AskForSaveDialog( QWidget* parent, const QString& message, ReturnCodes buttons ):
    BaseDialog( parent ),
    Counter( QStringLiteral("AskForSaveDialog") )
{

    Debug::Throw( QStringLiteral("AskForSaveDialog::AskForSaveDialog.\n") );
    setWindowModality( Qt::ApplicationModal );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout;
    layout->setSpacing(5);
    QtUtil::setMargin(layout, 10);
    setLayout( layout );

    QHBoxLayout *hLayout( new QHBoxLayout );
    hLayout->setSpacing(10);
    QtUtil::setMargin(hLayout, 10);
    layout->addLayout( hLayout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::DialogWarning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( message, this ), 1, Qt::AlignHCenter );

    // horizontal separator
    QFrame* frame( new QFrame( this ) );
    frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    layout->addWidget( frame );

    // button box
    QDialogButtonBox* buttonBox = new QDialogButtonBox( this );
    layout->addWidget( buttonBox );

    // yes button
    QPushButton* button;
    if( buttons & Yes )
    {
        button = buttonBox->addButton( QDialogButtonBox::Yes );
        connect( button, &QAbstractButton::clicked, this, &AskForSaveDialog::_yes );
        #if defined(Q_OS_WIN)
        button->setIcon( IconEngine::get( IconNames::DialogOk ) );
        #endif
    }

    // yes to all button
    if( buttons & All )
    {
        button = buttonBox->addButton( QDialogButtonBox::YesToAll );
        connect( button, &QAbstractButton::clicked, this, &AskForSaveDialog::_all );
        button->setIcon( IconEngine::get( IconNames::DialogAccept ) );
    }

    // no button
    if( buttons & No )
    {
        button = buttonBox->addButton( QDialogButtonBox::No );
        connect( button, &QAbstractButton::clicked, this, &AskForSaveDialog::_no );
        button->setIcon( IconEngine::get( IconNames::DialogClose ) );
    }

    // cancel button
    if( buttons & Cancel )
    {
        button = buttonBox->addButton( QDialogButtonBox::Cancel );
        button->setShortcut( Qt::Key_Escape );
        connect( button, &QAbstractButton::clicked, this, &AskForSaveDialog::_cancel );
    }

    adjustSize();

}
