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

/*!
  \file LogbookModifiedDialog.cpp
  \brief QDialog used when a file has been modified on disk
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <cassert>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "AnimatedTreeView.h"
#include "Icons.h"
#include "IconEngine.h"
#include "LogbookModifiedDialog.h"
#include "PixmapEngine.h"
#include "QtUtil.h"
#include "XmlOptions.h"

using namespace std;

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

  //! try load Question icon
  QPixmap question_pixmap( PixmapEngine::get( ICONS::WARNING ) );
  if( question_pixmap.isNull() )
  { layout->addWidget( new QLabel( buffer, this ), 0, Qt::AlignHCenter ); }
  else
  {

    QHBoxLayout *h_layout( new QHBoxLayout() );
    layout->addLayout( h_layout, 0 );
    QLabel* label = new QLabel( this );
    label->setPixmap( question_pixmap );
    h_layout->addWidget( label, 0, Qt::AlignHCenter );
    h_layout->addWidget( new QLabel( buffer, this ), 1, Qt::AlignHCenter );

  }

  // list
  list_ = new AnimatedTreeView( this );
  list_->setModel( &model_ );
  layout->addWidget( list_, 1 );

  model_.add( FileCheck::Model::List( files.begin(), files.end() ) );
  list_->resizeColumns();

  // button layout
  QHBoxLayout *button_layout = new QHBoxLayout();
  button_layout->setSpacing(5);
  button_layout->setMargin(0);
  layout->addLayout( button_layout );
  button_layout->addStretch(1);

  // resave button
  QPushButton* button;
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::SAVE ), "&Save Again", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _reSave() ) );
  button->setToolTip( "Save file again. Disc modifications will be lost" );

  // save as button
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::SAVE_AS ), "Save &As", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _saveAs() ) );
  button->setToolTip( "Save file with a different name" );

  // reload button.
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::RELOAD ), "&Reload", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _reLoad() ) );
  button->setToolTip( "Reload file from disc. Modifications will be lost" );

  // ignore button.
  button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Ignore", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _ignore() ) );
  button->setToolTip( "Ignore warning" );

}
