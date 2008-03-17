#ifndef SplashScreen_h
#define SplashScreen_h

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
   \file SplashScreen.h
   \brief application splash screen
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <assert.h>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <string>

#include "Counter.h"
#include "Debug.h"
#include "QtUtil.h"

//! application splash screen
class SplashScreen: public QWidget, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:
      
  //! constructorc
  SplashScreen( QWidget* parent = 0 );
    
  //! destructor
  virtual ~SplashScreen( void )
  { Debug::Throw( "SplashScreen::~SplashScreen.\n" ); }

  //! realize widget
  void realizeWidget( void );
  
  //! set application title
  virtual void setTitle( const std::string& title )
  {
    assert( !realized_ );
    title_ = title;
  }
  
  //! set splash pixmap
  virtual void setSplash( const QPixmap& pixmap )
  {
    assert( !realized_ );
    splash_ = pixmap; 
  }
  
  //! set splash screen displayed icon
  virtual void setIcon( const QPixmap& icon )
  {
    assert( !realized_ );
    icon_ = icon;
    QWidget::setWindowIcon( QIcon(icon) );
  }
  
  //! set icon size
  virtual void setIconSize( const unsigned int& size )
  { 
    assert( !realized_ );
    icon_size_ = size;
  }    
    
  //! change default title font
  virtual void SetTitleFontSize( const unsigned int value )
  {
    assert( !realized_ );
    title_font_size_ = value;
  }
  
  //! set minimum size (to be used if splash is not specified
  virtual void setMinimumSize( const QSize& size )
  { 
    assert( !realized_ );
    minimum_size_ = size;
  }
      
  //! set (composite) opacity
  virtual void setOpacity( const double& opacity )
  { opacity_ = opacity; }
  
  public slots:
  
  //! display new message
  virtual void displayMessage( const QString& message );
  
  protected:
      
  //! mouse press event
  virtual void mousePressEvent( QMouseEvent* )
  { close(); }
  
  //! mouse move event [overloaded]
  virtual void resizeEvent( QResizeEvent *e)
  {
    setMask( QtUtil::round( rect() ) );
    return QWidget::resizeEvent( e );
  }

  private:
  
  //! title
  std::string title_;
  
  //! splash
  QPixmap splash_;
  
  //! icon 
  QPixmap icon_;
      
  //! title default font size
  unsigned int title_font_size_;
      
  //! icon size
  unsigned int icon_size_;

  //! minimum size (used if splash is not specified
  QSize minimum_size_;
        
  //! message label
  QLabel* message_;
  
  //! opacity
  double opacity_;
  
  //! true when RealizeWidget is called
  bool realized_;
  
};

#endif
