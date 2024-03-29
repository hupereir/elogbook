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

#include "LogbookModifiedDialog.h"
#include "CppUtil.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "QtUtil.h"
#include "TreeView.h"
#include "XmlOptions.h"


#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
LogbookModifiedDialog::LogbookModifiedDialog( QWidget* parent, const FileCheck::DataSet& files ):
  BaseDialog( parent ),
Counter( QStringLiteral("LogbookModifiedDialog") )
{

    Debug::Throw( QStringLiteral("LogbookModifiedDialog::LogbookModifiedDialog.\n") );
    setOptionName( QStringLiteral("LOGBOOK_MODIFIED_DIALOG") );

    Q_ASSERT( !files.empty() );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout;
    layout->setSpacing(10);
    QtUtil::setMargin(layout, 10);
    setLayout( layout );

    // create message
    QString buffer( tr( "Following files have been modified by another application: \n" ) );

    QHBoxLayout *hLayout( new QHBoxLayout );
    layout->addLayout( hLayout, 0 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::DialogWarning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );

    // list
    list_ = new TreeView( this );
    list_->setModel( &model_ );
    layout->addWidget( list_, 1 );

    model_.add( Base::makeT<FileCheck::Model::List>( files ) );
    list_->resizeColumns();

    // button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(5);
    QtUtil::setMargin(buttonLayout, 0);
    layout->addLayout( buttonLayout );
    buttonLayout->addStretch(1);

    // resave button
    QPushButton* button;
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::Save ), tr( "Save Again" ), this ) );
    connect( button, &QAbstractButton::clicked, this, &LogbookModifiedDialog::_reSave );
    button->setToolTip( tr( "Save file again. Disc modifications will be lost" ) );

    // save as button
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::SaveAs ), tr( "Save As" ), this ) );
    connect( button, &QAbstractButton::clicked, this, &LogbookModifiedDialog::_saveAs );
    button->setToolTip( tr( "Save file with a different name" ) );

    // reload button.
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this ) );
    connect( button, &QAbstractButton::clicked, this, &LogbookModifiedDialog::_reLoad );
    button->setToolTip( tr( "Reload file from disc. Modifications will be lost" ) );

    // ignore button.
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogClose ), tr( "Ignore" ), this ) );
    connect( button, &QAbstractButton::clicked, this, &LogbookModifiedDialog::_ignore );
    button->setToolTip( tr( "Ignore warning" ) );

}

//___________________________________________________________________
void LogbookModifiedDialog::addFiles( const FileCheck::DataSet& files )
{ model_.add( Base::makeT<FileCheck::Model::List>( files ) ); }
