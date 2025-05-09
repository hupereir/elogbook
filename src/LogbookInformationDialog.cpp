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

#include "LogbookInformationDialog.h"
#include "Debug.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "Logbook.h"
#include "QtUtil.h"
#include "TextEditor.h"
#include "Util.h"


#include <QLayout>
#include <QLabel>

//_________________________________________________________
LogbookInformationDialog::LogbookInformationDialog( QWidget* parent, Logbook* logbook ):
    Dialog( parent )
{
    Debug::Throw( QStringLiteral("LogbookInformationDialog::LogbookInformationDialog.\n") );

    setWindowTitle( tr( "Logbook Informations" ) );
    setOptionName( QStringLiteral("LOGBOOK_INFORMATION_DIALOG") );

    layout()->setSpacing(0);
    QtUtil::setMargin(layout(), 0);
    QtUtil::setMargin(&buttonLayout(), defaultMargin());

    auto hLayout = new QHBoxLayout;
    QtUtil::setMargin(hLayout, defaultMargin());
    mainLayout().addLayout( hLayout );

    auto label = new QLabel(this);
    label->setPixmap( IconEngine::get( IconNames::DialogInformation ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignTop );

    auto gridLayout = new QGridLayout;
    QtUtil::setMargin(gridLayout, 0);
    hLayout->addLayout( gridLayout, 0 );

    gridLayout->addWidget( label = new QLabel( tr( "Title:" ), this ), 0, 0 );
    gridLayout->addWidget( title_ = new LineEditor( this ), 0, 1 );
    title_->setText( logbook->title().isEmpty() ?   Logbook::NoTitle:logbook->title()  );
    title_->setToolTip( tr( "Logbook title" ) );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    label->setBuddy( title_ );

    // logbook author
    gridLayout->addWidget( label = new QLabel( tr( "Author:" ), this ), 1, 0 );
    gridLayout->addWidget( author_ = new LineEditor( this ), 1, 1 );
    author_->setText( logbook->author().isEmpty() ? Logbook::NoAuthor:logbook->author() );
    author_->setToolTip( tr( "Logbook author" ) );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    label->setBuddy( author_ );

    // attachment directory
    gridLayout->addWidget( label = new QLabel( tr( "Attachment Directory:" ), this ), 2, 0 );
    gridLayout->addWidget( attachmentDirectory_ = new BrowsedLineEditor( this ), 2, 1 );
    attachmentDirectory_->setFile( logbook->directory().isEmpty() ? File(Util::workingDirectory()) : logbook->directory() );
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    attachmentDirectory_->setFileMode( QFileDialog::DirectoryOnly );
    #else
    attachmentDirectory_->setFileMode( QFileDialog::Directory );
    #endif
    attachmentDirectory_->setToolTip( tr( "Default directory where attached files are stored (either copied or linked)" ) );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    label->setBuddy( attachmentDirectory_ );

    // readonly
    gridLayout->addWidget( readOnlyCheckBox_ = new QCheckBox( tr( "Read-only" ), this ), 3, 1 );
    readOnlyCheckBox_->setToolTip( tr( "Mark logbook as read-only. Further write to the logbook will be forbidden" ) );

    gridLayout->setColumnStretch( 1, 1 );

    // comments
    mainLayout().addWidget( label = new QLabel( tr( " Comments:" ), this ), 0 );
    mainLayout().addWidget( comments_ = new TextEditor( this ), 1 );
    comments_->setPlainText( logbook->comments() );
    comments_->setToolTip( tr( "Logbook comments" ) );
    QtUtil::setWidgetSides(comments_, Qt::TopEdge|Qt::BottomEdge);
    label->setBuddy( comments_ );

    // connections
    connect( readOnlyCheckBox_, &QAbstractButton::toggled, title_, &QWidget::setDisabled );
    connect( readOnlyCheckBox_, &QAbstractButton::toggled, author_, &QWidget::setDisabled );
    connect( readOnlyCheckBox_, &QAbstractButton::toggled, attachmentDirectory_, &QWidget::setDisabled );
    connect( readOnlyCheckBox_, &QAbstractButton::toggled, comments_, &QWidget::setDisabled );
    readOnlyCheckBox_->setChecked( logbook->isReadOnly() );

}
