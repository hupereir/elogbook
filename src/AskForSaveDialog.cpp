
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
  \file AskForSaveDialog.cpp
  \brief QDialog used to ask if modifications of a file should be saved
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/


#include <QFrame>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "AskForSaveDialog.h"
#include "PixmapEngine.h"
#include "Icons.h"
#include "IconEngine.h"
#include "XmlOptions.h"
#include "QtUtil.h"

using namespace std;

//________________________________________________________
AskForSaveDialog::AskForSaveDialog( QWidget* parent, const QString& message, const unsigned int& buttons ):
  BaseDialog( parent ),
  Counter( "AskForSaveDialog" )
{

  Debug::Throw( "AskForSaveDialog::AskForSaveDialog.\n" );

  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout();
  layout->setSpacing(5);
  layout->setMargin(10);
  setLayout( layout );

  //! try load Question icon
  QPixmap question_pixmap( PixmapEngine::get( ICONS::WARNING ) );
  if( question_pixmap.isNull() )
  { layout->addWidget( new QLabel( message, this ), 1, Qt::AlignHCenter ); }
  else
  {

    QHBoxLayout *h_layout( new QHBoxLayout() );
    h_layout->setSpacing(10);
    h_layout->setMargin(10);
    layout->addLayout( h_layout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( question_pixmap );
    h_layout->addWidget( label, 0, Qt::AlignHCenter );
    h_layout->addWidget( new QLabel( message, this ), 1, Qt::AlignHCenter );

  }

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
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_OK ), "&Yes", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _yes() ) );
  }

  // yes to all button
  if( buttons & ALL )
  {
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_OK ), "Yes to &All", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _all() ) );
  }

  // no button
  if( buttons & NO )
  {
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&No", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _no() ) );
  }

  // cancel button
  if( buttons & CANCEL )
  {
    button_layout->addWidget( button = new QPushButton( IconEngine::get( ICONS::DIALOG_CANCEL ), "&Cancel", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _cancel() ) );
  }

  adjustSize();

}
