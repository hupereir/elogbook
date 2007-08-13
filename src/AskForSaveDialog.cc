
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
  \file AskForSaveDialog.cc
  \brief QDialog used to ask if modifications of a file should be saved
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <sstream>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include "AskForSaveDialog.h"
#include "CustomPixmap.h"
#include "XmlOptions.h"
#include "QtUtil.h"

using namespace std;

//________________________________________________________
AskForSaveDialog::AskForSaveDialog( QWidget* parent, const std::string& message, const unsigned int& buttons ):
  QDialog( parent ),
  Counter( "AskForSaveDialog" )
{
  
  Debug::Throw( "AskForSaveDialog::AskForSaveDialog.\n" );
  
  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout();
  layout->setSpacing(10);
  layout->setMargin(10);
  setLayout( layout );
  
  //! try load Question icon
  static CustomPixmap question_pixmap;
  static bool first( true );
  if( first )
  {
    first = false;
    list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
    question_pixmap.find( "messagebox_warning.png", path_list );    
  }
  
  // insert main vertical box
  if( question_pixmap.isNull() )
  { layout->addWidget( new QLabel( message.c_str(), this ), 1, Qt::AlignHCenter ); }
  else
  {
    
    QHBoxLayout *h_layout( new QHBoxLayout() );
    h_layout->setSpacing(10);
    h_layout->setMargin(10);
    layout->addLayout( h_layout, 1 );
    QLabel* label = new QLabel( this );
    label->setPixmap( question_pixmap );
    h_layout->addWidget( label, 0, Qt::AlignHCenter );
    h_layout->addWidget( new QLabel( message.c_str(), this ), 1, Qt::AlignHCenter );
    
  }
    
  // button layout
  QHBoxLayout *button_layout = new QHBoxLayout();     
  button_layout->setMargin(0);
  button_layout->setSpacing(5);
  layout->addLayout( button_layout );

  // yes button
  QPushButton* button;
  if( buttons & YES )
  {
    button_layout->addWidget( button = new QPushButton( "&Yes", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _yes() ) );
  }
  
  // no button
  if( buttons & NO )
  {
    button_layout->addWidget( button = new QPushButton( "&No", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _no() ) );
  }
  
  // no button
  if( buttons & ALL )
  {
    button_layout->addWidget( button = new QPushButton( "Yes to &All", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _all() ) );
  }
  
  // cancel button
  if( buttons & CANCEL )
  {
    button_layout->addWidget( button = new QPushButton( "&Cancel", this ) );
    connect( button, SIGNAL( clicked() ), SLOT( _cancel() ) );
  }
  
  adjustSize();
  
}
