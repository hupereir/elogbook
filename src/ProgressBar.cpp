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
  \file ProgressBar.cc
  \brief display command progress
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QStyleOptionProgressBarV2>
#include <QStylePainter>

#include "Debug.h"
#include "ProgressBar.h" 

using namespace std;

//___________________________________________________________
ProgressBar::ProgressBar( QWidget* parent ):
  QProgressBar( parent ),
  Counter( "ProgressBar" ),
  current_( 0 )
{ 
  Debug::Throw( "ProgressBar::ProgressBar.\n" ); 
  QPalette palette( QProgressBar::palette() );
  palette.setColor( QPalette::HighlightedText, palette.color( QPalette::Text ) );
}

//___________________________________________________________ 
void ProgressBar::setText( const QString& text )
{
  text_ = text;
  update();
}

//___________________________________________________________ 
void ProgressBar::setMaximumProgress( unsigned int value )
{
  Debug::Throw( "ProgressBar::setMaximumProgress.\n" );
  setRange( 0, value );
  setValue( (current_ = 0 ) );
}

//___________________________________________________________ 
void ProgressBar::addToProgress( unsigned int value )
{
  Debug::Throw( "ProgressBar::addToProgress.\n" );
  setValue( (current_ += value ) );
}

//___________________________________________________________ 
void ProgressBar::paintEvent( QPaintEvent* event )
{
  QStylePainter paint(this);
  QStyleOptionProgressBarV2 opt;
  initStyleOption(&opt);

  opt.text = "";
  paint.drawControl(QStyle::CE_ProgressBar, opt);

  // need to draw the label separately 
  // because the "BASE" color is used by default, which might be too light,
  // when the selection color (used for the bar) is light
  // one use HighlightedText instead
  opt.text = text_;
  opt.palette.setColor( QPalette::Base, palette().color( QPalette::HighlightedText ) );
  paint.drawControl(QStyle::CE_ProgressBarLabel, opt);

}