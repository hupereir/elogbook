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
#include <QLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>

#include "SplashScreen.h"
#include "CompositeEngine.h"
#include "CustomPixmap.h"
#include "Debug.h"
#include "QtUtil.h"
#include "SvgEngine.h"
#include "XmlOptions.h"

using namespace std;
using namespace Qt;

//_______________________________________________________________________
SplashScreen::SplashScreen( QWidget* parent ):
  QWidget( parent, Qt::Window|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint ),
  Counter( "SplashScreen" ),
  icon_size_(0),
  margin_(0),
  minimum_width_(0)
{
  
  Debug::Throw( "SplashScreen::SplashScreen.\n" );
  setAttribute( Qt::WA_DeleteOnClose );
  
  // colors and transparency
  bool transparent( XmlOptions::get().get<bool>( "TRANSPARENT_SPLASH_SCREEN" ) );
  use_svg_ = XmlOptions::get().get<bool>( "USE_SVG" );
  if( transparent || use_svg_ )
  {
    
    QPalette palette( this->palette() );
    palette.setColor( QPalette::Window, Qt::black );
    palette.setColor( QPalette::WindowText, Qt::white );
    setPalette( palette );
    
  }
  
  if( transparent ) setOpacity( 0.9 );
    
  if( use_svg_ && TRANSPARENCY::CompositeEngine::get().isEnabled() ) 
  { 
    setAttribute( Qt::WA_OpaquePaintEvent );
    setAttribute( Qt::WA_NoSystemBackground ); 
  }
  
  // title
  ostringstream what;
  what << "eLogbook";
  setTitle( what.str().c_str() );

  // icon size
  setIconSize( 64 );
  setMargin( 20 );
  setMinimumWidth( 300 );
  
  // pixmap
  QPixmap pixmap( (File( XmlOptions::get().raw( "ICON_PIXMAP" ) )).expand().c_str() );
  setIcon( pixmap );
  
  // fonts
  large_font_ = font();
  large_font_.setBold( true );
  setTitleFontSize( 20 );
  normal_font_ = font();
  
  setMinimumSize( QSize( 300, icon_size_ + 2*margin_ ) );
  setMouseTracking( true );
  
  SVG::SvgEngine::get().reload();
  
}

//_______________________________________________________________________
void SplashScreen::setSplash( QPixmap pixmap )
{
  if( pixmap.isNull() ) return;

  QPalette palette( QWidget::palette() );
  palette.setBrush( QPalette::Background, pixmap );
  setPalette( palette );
  setMinimumWidth( pixmap.width() );
  
}
    
//_______________________________________________________________________
void SplashScreen::setIcon( QPixmap pixmap )
{
  icon_ = CustomPixmap( pixmap ).scaled( QSize( icon_size_, icon_size_ ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
  QWidget::setWindowIcon( QIcon(pixmap) );
}

//_______________________________________________________________________
void SplashScreen::setOpacity( const double& opacity )
{ QtUtil::setOpacity( this, opacity ); }

//_______________________________________________________________________
void SplashScreen::setTitleFontSize( int value )
{ large_font_.setPointSize( value ); }

//_______________________________________________________________________
void SplashScreen::setIconSize( int value )
{
  // check value changed
  if( icon_size_ == value ) return;

  // store new size
  icon_size_ = value;
  
  // resize icon
  if( !icon_.isNull() ) setIcon( icon_ );
  
  // resize widget
  if( minimum_width_ > 0 ) setMinimumWidth( minimum_width_ );
  
}

//_______________________________________________________________________
void SplashScreen::setMargin( int value )
{
  
  // check value changed
  if( margin_ == value  ) return;
  margin_ = value;
 
  // resize widget
  if( minimum_width_ > 0 ) setMinimumWidth( minimum_width_ );
 
}

//_______________________________________________________________________
void SplashScreen::setMinimumWidth( int value )
{ 
  
  QSize size( value, max( icon_size_+2*margin_, 
    3*margin_ + 
    QFontMetrics( normal_font_ ).lineSpacing() +
    QFontMetrics( large_font_ ).lineSpacing() )
    );
  
  QWidget::setMinimumSize( size ); 
  resize( size );
}

//_______________________________________________________________________
void SplashScreen::displayMessage( const QString& text )
{
  message_ = text;
  update();
}

//_____________________________________________________________________
void SplashScreen::mousePressEvent( QMouseEvent* event )
{ close(); }

//_____________________________________________________________________
void SplashScreen::resizeEvent( QResizeEvent* event )
{
  QWidget::resizeEvent( event );
  setMask( QtUtil::round( QRect( QPoint(0,0), event->size() ) ) );
}

//_____________________________________________________________________
void SplashScreen::paintEvent( QPaintEvent* event )
{

  QWidget::paintEvent( event );
  QPainter painter( this );
  
  if( use_svg_ && TRANSPARENCY::CompositeEngine::get().isEnabled() ) 
  { 
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_Source );
    painter.fillRect( rect(), Qt::transparent );
  }

  // draw background
  if( use_svg_ ) painter.drawPixmap( QPoint(0,0), SVG::SvgEngine::get().get( size() ) );

  painter.setCompositionMode(QPainter::CompositionMode_SourceOver );
    
  // prepare rect for drawing
  QRect pixmap_rect;
  QRect title_rect( rect() );
  QRect message_rect( rect() );
  QRect bounding_rect;
  
  if( !icon_.isNull() ) { 
    pixmap_rect = QRect( 0, 0, icon_.width()+margin_, height() ); 
    painter.drawPixmap( 
      pixmap_rect.left()+(pixmap_rect.width() - icon_.width() )/2, 
      pixmap_rect.top()+(pixmap_rect.height() - icon_.height() )/2, 
      icon_ );
    
    title_rect.setLeft( pixmap_rect.right() );
    message_rect.setLeft( pixmap_rect.right() );
    
  }
  
  // font metrics
  QFontMetrics normal_metrics( normal_font_ );

  // title
  title_rect.setTop( margin_ );
  title_rect.setBottom( height() - 3*margin_/2 - normal_metrics.lineSpacing() );
  painter.setFont( large_font_ );
  painter.drawText( title_rect, Qt::AlignCenter, title_, &bounding_rect );
  
  // comments
  message_rect.setTop( title_rect.bottom() + margin_/2 );
  message_rect.setBottom( height() - margin_ );
  painter.setFont( normal_font_ );
  painter.drawText( message_rect, Qt::AlignCenter, message_ );
  painter.end();

}
