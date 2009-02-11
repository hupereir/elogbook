
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
 * ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or            
 * FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License            
 * for more details.                        
 *                             
 * You should have received a copy of the GNU General Public License along with 
 * software; if not, write to the Free Software Foundation, Inc., 59 Temple       
 * Place, Suite 330, Boston, MA   02111-1307 USA                                      
 *                            
 *                            
 *******************************************************************************/

/*!
   \file PrintDialog.cpp
   \brief new logbook popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QToolButton>

#include "FileDialog.h"
#include "GridLayout.h"
#include "Icons.h"
#include "IconEngine.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "QtUtil.h"
#include "PrintDialog.h"

using namespace std;

//___________________________________________________
PrintDialog::PrintDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "PrintDialog::PrintDialog.\n" );
  
  setWindowTitle( "Print - elogbook" );
  mainLayout().setSpacing(5);
  
  // file
  GridLayout *grid_layout = new GridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  grid_layout->setMaxCount( 2 );
  mainLayout().addItem( grid_layout );
  
  grid_layout->addWidget( new QLabel( "File:", this ) );
  grid_layout->addWidget( destination_editor_ = new BrowsedLineEditor( this ) );
  _destinationEditor().setFileMode( QFileDialog::AnyFile );

  grid_layout->addWidget( new QLabel( "Command:", this ) );
  
  QHBoxLayout* h_layout = new QHBoxLayout();
  h_layout->setSpacing(2);
  h_layout->setMargin(0);
  grid_layout->addLayout( h_layout );
  
  // command
  h_layout->addWidget( command_editor_ = new CustomComboBox( this ) );
  command_editor_->setEditable( true );
  
  // browse command button associated to the CustomComboBox
  QToolButton* button = new QToolButton( this );
  button->setIcon( IconEngine::get( ICONS::OPEN ) );
  button->setAutoRaise( false );
  h_layout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( _browseCommand() ) );
 
  // change button text
  okButton().setText( "&Print" );
  okButton().setIcon( IconEngine::get( ICONS::PRINT ));

}

//__________________________________________________ 
void PrintDialog::_browseCommand( void )
{
 
  Debug::Throw( "PrintDialog::_browseCommand.\n" );
  
  // open FileDialog
  QString file( FileDialog( this ).getFile() );
  if( !file.isNull() )
  {
    _commandEditor().setEditText( file );
    _commandEditor().addItem( file );
  }
  
  return;

}
