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
  \file ProgressFrame.cc
  \brief display command progress
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>

#include "Debug.h"
#include "ProgressFrame.h" 

using namespace std;

//___________________________________________________________
ProgressFrame::ProgressFrame( QWidget* parent ):
  QWidget( parent ),
  Counter( "ProgressFrame" ),
  current_( 0 )
{
  
  Debug::Throw( "ProgressFrame::ProgressFrame.\n" );
  
  // layout
  QHBoxLayout *layout = new QHBoxLayout();
  layout->setSpacing( 5 );
  layout->setMargin(0);
  setLayout( layout );

  // progress bar
  layout->addWidget( progress_bar_ = new QProgressBar( this ) );
  
}

//___________________________________________________________ 
void ProgressFrame::setMaximumProgress( unsigned int value )
{
  Debug::Throw( "ProgressFrame::setMaximumProgress.\n" );
  _progressBar().setRange( 0, value );
  _progressBar().setValue( (current_ = 0 ) );
}

//___________________________________________________________ 
void ProgressFrame::addToProgress( unsigned int value )
{
  Debug::Throw( "ProgressFrame::addToProgress.\n" );
  _progressBar().setValue( (current_ += value ) );
}
