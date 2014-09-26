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

#include "AnimatedTreeView.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "LogbookModifiedDialog.h"
#include "LogbookModifiedDialog.moc"
#include "QtUtil.h"
#include "XmlOptions.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

//________________________________________________________
LogbookModifiedDialog::LogbookModifiedDialog( QWidget* parent, const FileCheck::DataSet& files ):
  BaseDialog( parent ),
Counter( "LogbookModifiedDialog" )
{

    Debug::Throw( "LogbookModifiedDialog::LogbookModifiedDialog.\n" );
    setOptionName( "LOGBOOK_MODIFIED_DIALOG" );

    Q_ASSERT( !files.empty() );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout();
    setLayout( layout );

    // create message
    QString buffer( tr( "Following files have been modified by another application: \n" ) );

    QHBoxLayout *hLayout( new QHBoxLayout() );
    layout->addLayout( hLayout, 0 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( IconNames::Warning ).pixmap( iconSize() ) );
    hLayout->addWidget( label, 0, Qt::AlignHCenter );
    hLayout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );

    // list
    list_ = new AnimatedTreeView( this );
    list_->setModel( &model_ );
    layout->addWidget( list_, 1 );

    model_.add( files.toList() );
    list_->resizeColumns();

    // button layout
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setMargin(0);
    layout->addLayout( buttonLayout );
    buttonLayout->addStretch(1);

    // resave button
    QPushButton* button;
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::Save ), tr( "Save Again" ), this ) );
    connect( button, SIGNAL(clicked()), SLOT(_reSave()) );
    button->setToolTip( tr( "Save file again. Disc modifications will be lost" ) );

    // save as button
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::SaveAs ), tr( "Save As" ), this ) );
    connect( button, SIGNAL(clicked()), SLOT(_saveAs()) );
    button->setToolTip( tr( "Save file with a different name" ) );

    // reload button.
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::Reload ), tr( "Reload" ), this ) );
    connect( button, SIGNAL(clicked()), SLOT(_reLoad()) );
    button->setToolTip( tr( "Reload file from disc. Modifications will be lost" ) );

    // ignore button.
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( IconNames::DialogClose ), tr( "Ignore" ), this ) );
    connect( button, SIGNAL(clicked()), SLOT(_ignore()) );
    button->setToolTip( tr( "Ignore warning" ) );

}
