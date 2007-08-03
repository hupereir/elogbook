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

#ifndef SplashScreen_h
#define SplashScreen_h

/*!
   \file SplashScreen.h
   \brief application splash screen
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <string>

#include "Counter.h"
#include "Exception.h"

//! application splash screen
class SplashScreen: public QWidget, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:
      
  //! constructorc
  SplashScreen( QWidget* parent, const std::string& title );
    
  //! destructor
  virtual ~SplashScreen( void )
  { Debug::Throw( "SplashScreen::~SplashScreen.\n" ); }

  //! realize widget
  void realizeWidget( void );
  
  //! set application title
  virtual void setTitle( const std::string& title )
  {
    Exception::check( !realized_, DESCRIPTION( "widget already realized" ) );
    title_ = title;
  }
  
  //! set splash pixmap
  virtual void setSplash( const QPixmap& pixmap )
  {
    Exception::check( !realized_, DESCRIPTION( "widget already realized" ) );
    splash_ = pixmap; 
  }
  
  //! set splash screen displayed icon
  virtual void setIcon( const QPixmap& icon )
  {
    Exception::check( !realized_, DESCRIPTION( "widget already realized" ) );
    icon_ = icon;
    QWidget::setWindowIcon( QIcon(icon) );
  }
  
  //! set icon size
  virtual void setIconSize( const unsigned int& size )
  { 
    Exception::check( !realized_, DESCRIPTION( "widget already realized" ) );
    icon_size_ = size;
  }    
    
  //! change default title font
  virtual void SetTitleFontSize( const unsigned int value )
  {
    Exception::check( !realized_, DESCRIPTION( "widget already realized" ) );
    title_font_size_ = value;
  }
  
  //! set minimum size (to be used if splash is not specified
  virtual void setMinimumSize( const QSize& size )
  { 
    Exception::check( !realized_, DESCRIPTION( "widget already realized" ) );
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
  virtual void mousePressEvent ( QMouseEvent* )
  { close(); }
  
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
