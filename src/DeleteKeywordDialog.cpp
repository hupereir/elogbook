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

#include "DeleteKeywordDialog.h"
#include "Debug.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "QtUtil.h"

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>

//_____________________________________________________
DeleteKeywordDialog::DeleteKeywordDialog( QWidget* parent, const Keyword::List& keywords, bool hasEntries ):
    Dialog( parent, OkButton|CancelButton|Separator )
{

    Debug::Throw( QStringLiteral("DeleteKeywordDialog::DeleteKeywordDialog.\n") );
    setWindowTitle( tr( "Delete Keyword" ) );

    // create label
    QString buffer;
    if( keywords.size() == 1 ) buffer = tr( "Delete keyword '%1' ?" ).arg( keywords.front().get() );
    else {

        buffer = tr( "Delete following keywords ?\n  " );
        int maxKeywords = 10;
        int index(0);
        for( const auto& keyword:keywords )
        {
            buffer += keyword.get() + " ";
            index++;
            if( index >= maxKeywords )
            {
                index = 0;
                buffer += QLatin1String("\n ");
            }
        }

    }

    //! try load Question icon
    auto hLayout( new QHBoxLayout );
    hLayout->setSpacing(10);
    QtUtil::setMargin(hLayout, 0);
    mainLayout().addLayout( hLayout );

    auto label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::DialogWarning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( buffer, this ) );

    auto box = new QWidget( this );
    mainLayout().addWidget( box );
    box->setLayout( new QVBoxLayout );
    QtUtil::setMargin(box->layout(), 5);
    box->layout()->setSpacing(5);


    // radio buttons
    auto group = new QButtonGroup( this );

    box->layout()->addWidget( moveRadioButton_ = new QRadioButton( tr( "Move entries to parent keyword" ), box ) );
    moveRadioButton_->setToolTip( tr( "Select this button to move entries associated to this keyword to the parent keyword" ) );
    group->addButton( moveRadioButton_ );

    box->layout()->addWidget( deleteRadioButton_ = new QRadioButton( tr( "Delete entries" ), box ) );
    deleteRadioButton_->setToolTip( tr( "Select this button to delete entries associated to this keyword" ) );
    group->addButton( deleteRadioButton_ );

    group->setExclusive( true );
    moveRadioButton_->setChecked( true );

    if( !hasEntries ) box->setEnabled( false );

    okButton().setText( tr( "Delete" ) );
    okButton().setIcon( IconEngine::get( IconNames::Delete ) );

    adjustSize();

}
