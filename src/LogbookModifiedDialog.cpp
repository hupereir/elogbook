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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "AnimatedTreeView.h"
#include "Icons.h"
#include "IconEngine.h"
#include "LogbookModifiedDialog.h"
#include "QtUtil.h"
#include "XmlOptions.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

//________________________________________________________
LogbookModifiedDialog::LogbookModifiedDialog( QWidget* parent, const FileCheck::DataSet& files ):
BaseDialog( parent ),
Counter( "LogbookModifiedDialog" )
{

    Debug::Throw( "LogbookModifiedDialog::LogbookModifiedDialog.\n" );
    setOptionName( "LOGBOOK_MODIFIED_DIALOG" );

    assert( !files.empty() );

    // create vbox layout
    QVBoxLayout* layout=new QVBoxLayout();
    layout->setSpacing(10);
    layout->setMargin(10);
    setLayout( layout );

    // create message
    QString buffer;
    QTextStream what( &buffer );
    what
        << "Following files have been modified"
        << " been modified by by another application: "
        << endl;

    QHBoxLayout *hLayout( new QHBoxLayout() );
    layout->addLayout( hLayout, 0 );
    QLabel* label = new QLabel( this );
    label->setPixmap( IconEngine::get( ICONS::WARNING ).pixmap( iconSize() ) );
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
    buttonLayout->setSpacing(5);
    buttonLayout->setMargin(0);
    layout->addLayout( buttonLayout );
    buttonLayout->addStretch(1);

    // resave button
    QPushButton* button;
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::SAVE ), "&Save Again", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _reSave() ) );
    button->setToolTip( "Save file again. Disc modifications will be lost" );

    // save as button
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::SAVE_AS ), "Save &As", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _saveAs() ) );
    button->setToolTip( "Save file with a different name" );

    // reload button.
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::RELOAD ), "&Reload", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _reLoad() ) );
    button->setToolTip( "Reload file from disc. Modifications will be lost" );

    // ignore button.
    buttonLayout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Ignore", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _ignore() ) );
    button->setToolTip( "Ignore warning" );

}
