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

#include "NewLogbookDialog.h"
#include "Debug.h"
#include "QtUtil.h"

#include <QLabel>
#include <QLayout>

//_______________________________________________________
NewLogbookDialog::NewLogbookDialog( QWidget* parent ):
Dialog( parent )
{

    Debug::Throw( QStringLiteral("NewLogbookDialog::NewLogbookDialog.\n") );

    setWindowTitle( tr( "New Logbook" ) );
    setOptionName( QStringLiteral("NEW_LOGBOOK_DIALOG") );

    layout()->setSpacing(0);
    QtUtil::setMargin(layout(), 0);
    QtUtil::setMargin(&buttonLayout(), defaultMargin());

    auto gridLayout( new QGridLayout );
    QtUtil::setMargin(gridLayout, 0);
    gridLayout->setSpacing(5);
    mainLayout().addLayout( gridLayout, 0 );

    // title
    QLabel* label;
    gridLayout->addWidget( label = new QLabel( tr( " Title:" ), this ), 0, 0 );
    gridLayout->addWidget( title_ = new LineEditor( this ), 0, 1 );
    title_->setToolTip( tr( "Logbook title" ) );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    label->setBuddy( title_ );

    // logbook author
    gridLayout->addWidget( label = new QLabel( tr( " Author:" ), this ), 1, 0 );
    gridLayout->addWidget( author_ = new LineEditor( this ), 1, 1 );
    author_->setToolTip( tr("Logbook author" ) );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    label->setBuddy( author_ );

    // attachment directory
    gridLayout->addWidget( label = new QLabel( tr( " Attachment directory:" ), this ), 3, 0 );
    gridLayout->addWidget( attachmentDirectory_ = new BrowsedLineEditor( this ), 3, 1 );
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    attachmentDirectory_->setFileMode( QFileDialog::DirectoryOnly );
    #else
    attachmentDirectory_->setFileMode( QFileDialog::Directory );
    #endif
    attachmentDirectory_->setAcceptMode( QFileDialog::AcceptSave );
    attachmentDirectory_->setToolTip( tr( "Default directory where attached files are stored (either copied or linked)" ) );
    label->setAlignment( Qt::AlignVCenter|Qt::AlignRight );
    label->setBuddy( attachmentDirectory_ );

    gridLayout->setColumnStretch( 1, 1 );

    // comments
    mainLayout().addWidget( label = new QLabel( tr( " Comments:" ), this ), 0 );
    mainLayout().addWidget( comments_ = new TextEditor( this ), 1 );
    comments_->setToolTip( tr( "Logbook comments" ) );
    QtUtil::setWidgetSides(comments_, Qt::TopEdge|Qt::BottomEdge);
    label->setBuddy( comments_ );

    adjustSize();

}
