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
#include "RoundedRegion.h"
#include "SvgEngine.h"
#include "XmlOptions.h"
#include "WinUtil.h"

using namespace std;
using namespace Qt;

//_______________________________________________________________________
SplashScreen::SplashScreen( QWidget* parent ):
  TRANSPARENCY::TransparentWidget( parent, Qt::Window|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint ),
  icon_size_(0),
  margin_(0),
  minimum_width_(0)
{
  
  Debug::Throw( "SplashScreen::SplashScreen.\n" );
  setAttribute( Qt::WA_DeleteOnClose );
  
  round_corners_ = XmlOptions::get().get<bool>( "ROUND_SPLASH_SCREEN" );
  use_svg_ = XmlOptions::get().get<bool>( "USE_SVG" );
  if( _transparent() || use_svg_ )
  {
    
    QPalette palette( this->palette() );
    
    palette.setColor( QPalette::Window, Qt::black );
    palette.setColor( QPalette::Base, Qt::black );

    palette.setColor( QPalette::WindowText, Qt::white );
    palette.setColor( QPalette::Text, Qt::white );
    
    setPalette( palette );
    
  }
  
  #ifdef Q_WS_WINDOW
  if( !TRANSPARENCY::CompositeEngine::get().isEnabled() )
  #endif
  { if( _transparent() ) setWindowOpacity( 0.9 ); }
    
  // title
  setTitle( "elogbook" );

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

  QPalette palette( TRANSPARENCY::TransparentWidget::palette() );
  palette.setBrush( QPalette::Background, pixmap );
  setPalette( palette );
  setMinimumWidth( pixmap.width() );
  
}
    
//_______________________________________________________________________
void SplashScreen::setIcon( QPixmap pixmap )
{
  icon_ = CustomPixmap( pixmap ).scaled( QSize( icon_size_, icon_size_ ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
  TRANSPARENCY::TransparentWidget::setWindowIcon( QIcon(pixmap) );
}

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
  
  TRANSPARENCY::TransparentWidget::setMinimumSize( size ); 
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
  
  TRANSPARENCY::TransparentWidget::resizeEvent( event );
  if( round_corners_ ) setMask( RoundedRegion( rect() ) );
}

//________________________________________________
void SplashScreen::_updateBackgroundPixmap( void )
{

  Debug::Throw( "SplashScreen::_updateBackgroundPixmap.\n" );
  TransparentWidget::_updateBackgroundPixmap();
  
  // check if svg is selected
  if( use_svg_ && SVG::SvgEngine::get().isValid() )
  {  
    // top svg
    QPainter painter( &_backgroundPixmap() );
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver );
    painter.drawPixmap( QPoint( 0, 0 ), SVG::SvgEngine::get().get( size() ) );
    painter.end();
  }
  
}

//_____________________________________________________________________
void SplashScreen::_paint( QPaintDevice& device, const QRect& clip )
{
  QPainter painter( &device );
  painter.setClipRect( clip );
  painter.setRenderHints(QPainter::Antialiasing );
    
  // prepare rect for drawing
  QRect pixmap_rect;
  QRect title_rect( rect() );
  QRect message_rect( rect() );
  QRect bounding_rect;
  
  if( !icon_.isNull() ) 
  {
    
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
  painter.setPen( palette().color( QPalette::Text ) );
  painter.setFont( large_font_ );
  painter.drawText( title_rect, Qt::AlignCenter, title_, &bounding_rect );
  
  // comments
  message_rect.setTop( title_rect.bottom() + margin_/2 );
  message_rect.setBottom( height() - margin_ );
  painter.setFont( normal_font_ );
  painter.drawText( message_rect, Qt::AlignCenter, message_ );
  painter.end();

}
