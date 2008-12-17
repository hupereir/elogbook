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
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QResizeEvent>
#include <string>

#include "TransparentWidget.h"
#include "Counter.h"
#include "Debug.h"
#include "QtUtil.h"

//! application splash screen
class SplashScreen: public TRANSPARENCY::TransparentWidget
{

  //! Qt meta object declaration
  Q_OBJECT

  public:
      
  //! constructorc
  SplashScreen( QWidget* parent = 0 );
    
  //! destructor
  virtual ~SplashScreen( void )
  { Debug::Throw( "SplashScreen::~SplashScreen.\n" ); }
  
  //! set application title
  virtual void setTitle( const QString& title )
  { title_ = title; }
  
  //! set splash pixmap
  virtual void setSplash( QPixmap );
  
  //! set splash screen displayed icon
  virtual void setIcon( QPixmap );

  //! change default title font
  virtual void setTitleFontSize( int value );
  
  //! set icon size
  virtual void setIconSize( int );
  
  //! set icon size
  virtual void setMargin( int );

  //! set icon size
  virtual void setMinimumWidth( int );

  public slots:
  
  //! display new message
  virtual void displayMessage( const QString& );
  
  protected:
      
  //! mouse press event [overloaded]
  virtual void mousePressEvent( QMouseEvent* );
       
  //! resize event [overloaded]
  virtual void resizeEvent( QResizeEvent *e);
  
  //! paint on device
  virtual void _paint( QPaintDevice&, const QRect& );
  
  private:
  
  //! title
  QString title_;
  
  //! message
  QString message_;
    
  //! icon 
  QPixmap icon_;
              
  //! fonts
  QFont large_font_;

  //! fonts
  QFont normal_font_;
    
  //! round corners
  bool round_corners_;
  
  //! icon size
  int icon_size_;
  
  //! margin
  int margin_;
  
  //! minimum width
  int minimum_width_;
  
  //! svg
  bool use_svg_;
  
  #ifdef Q_WS_WIN
  //! widget pixmap
  /*! it is used as widget storage when using full translucency */
  QPixmap widget_pixmap_;
  #endif

};

#endif
