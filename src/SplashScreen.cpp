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
   \file SplashScreen.cpp
   \brief application splash screen
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QApplication>
#include <QFrame>
#include <QLayout>
#include <QLabel>

#include "SplashScreen.h"
#include "CustomPixmap.h"
#include "Exception.h"
#include "QtUtil.h"

using namespace std;
using namespace Qt;

//_______________________________________________________________________
SplashScreen::SplashScreen( QWidget* parent, const std::string& title ):
  QWidget( parent, Qt::Window|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint ),
  Counter( "SplashScreen" ),
  title_( title ),
  title_font_size_( 20 ),
  icon_size_(64),
  minimum_size_( 300, 200 ),
  opacity_(1.0),
  realized_( false )
{
  Debug::Throw( "SplashScreen::SplashScreen.\n" );
  setAttribute( Qt::WA_DeleteOnClose );

}

//_______________________________________________________________________
void SplashScreen::realizeWidget( void )
{
  Exception::check( !realized_, DESCRIPTION( "widget already realized" ) );
  
  if( !splash_.isNull() )
  {
    minimum_size_ = splash_.size();
    QPalette palette( QWidget::palette() );
    palette.setBrush( QPalette::Background, splash_ );
    setPalette( palette );
  }
  
  setMinimumSize( minimum_size_ );    
  resize( minimum_size_ );

  QtUtil::setOpacity( this, opacity_ );
  QVBoxLayout *layout = new QVBoxLayout(); 
  setLayout( layout );
  
  QFrame *frame = new QFrame( this );
  if( opacity_ == 1 ) frame->setFrameStyle( QFrame::Panel | QFrame::Raised );
  frame->setFixedSize( minimum_size_ );
  layout->addWidget( frame );
    
  QHBoxLayout* h_layout = new QHBoxLayout();
  h_layout->setSpacing(10);
  h_layout->setMargin(0);
  frame->setLayout( h_layout );
    
  if( !icon_.isNull() )
  { 
    QLabel *icon_label = new QLabel( frame );
    icon_label->setPixmap( CustomPixmap( icon_ ).scale( icon_size_, icon_size_ ) );
    QtUtil::fixSize( icon_label, QtUtil::WIDTH );
    h_layout->addWidget( icon_label );
  }
  
  layout = new QVBoxLayout();
  layout->setSpacing(10);
  layout->setMargin(10);
  h_layout->addLayout( layout );

  QLabel* title = new QLabel( frame );
  title->setAlignment( Qt::AlignCenter );
  title->setText( title_.c_str() );
  layout->addWidget( title );
  
  {
    // change title font
    QFont font( title->font() );
    font.setPointSize( title_font_size_ );
    font.setWeight( QFont::Bold );
    title->setFont( font );
  }
  
  message_ = new QLabel( frame );
  message_->setAlignment( AlignCenter );
  QtUtil::fixSize( message_, QtUtil::HEIGHT );
  layout->addWidget( message_ );
  
  realized_ = true;
  
}
    
//_______________________________________________________________________
void SplashScreen::displayMessage( const QString& text )
{
  Exception::check( realized_, DESCRIPTION( "widget not realized" ) );
  message_->setText( text );
  qApp->processEvents();
}
