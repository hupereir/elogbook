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
  \file AttachmentWindow.cpp
  \brief popup window to list/edit all attachments independantly from entries
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QShortcut>
#include <QLayout>
#include <QPushButton>
#include <list>

#include "AttachmentWindow.h"
#include "EditionWindow.h"
#include "IconEngine.h"
#include "Icons.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "Application.h"
#include "Options.h"
#include "MainWindow.h"

using namespace std;
using namespace Qt;

//________________________________________
AttachmentWindow::AttachmentWindow( QWidget* parent ):
  BaseDialog( parent ),
  Counter( "AttachmentWindow" )
{
  
  Debug::Throw( "AttachmentWindow::AttachmentWindow.\n" );
  setWindowTitle( Application::ATTACHMENT_TITLE );
  _setSizeOptionName( "ATTACHMENT_WINDOW" );
  
  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout(this);
  layout->setMargin(10);
  layout->setSpacing(10);
  setLayout( layout );
  
  layout->addWidget( frame_ = new AttachmentFrame( this, true ) );
  connect( frame_, SIGNAL( attachmentSelected( Attachment& ) ), SLOT( _displayEntry( Attachment& ) ) );
  
  // shortcuts
  connect( new QShortcut( CTRL+Key_Q, this ), SIGNAL( activated() ), qApp, SLOT( closeAllWindows() ) );
  connect( new QShortcut( CTRL+Key_W, this ), SIGNAL( activated() ), SLOT( close() ) );
  
  uniconify_action_ = new QAction( IconEngine::get( ICONS::ATTACH ), "&Attachments", this );
  uniconify_action_->setToolTip( "Raise application main window" );
  connect( uniconify_action_, SIGNAL( triggered() ), SLOT( uniconify() ) );
  
  // button layout
  QHBoxLayout *button_layout( new QHBoxLayout() );
  button_layout->setSpacing(10);
  button_layout->setMargin(0);
  layout->addLayout( button_layout );
  
  // close button
  QPushButton* button;
  button_layout->addWidget( button = new QPushButton( "&Close", this ) );
  button->setAutoDefault( false );
  connect( button, SIGNAL( clicked() ), SLOT( close() ) );
  
};

//________________________________________
void AttachmentWindow::show( void )
{
  Debug::Throw( "AttachmentWindow::show.\n" );
  centerOnWidget( qApp->activeWindow());
  QWidget::show();
  QWidget::raise();
}
  
//____________________________________________
void AttachmentWindow::enterEvent( QEvent *event )
{
  Debug::Throw( "MainWindow::enterEvent.\n" );
  
  // base class enterEvent
  QWidget::enterEvent( event );
  static_cast<Application*>(qApp)->mainWindow().checkLogbookModified();
    
  return;
}

//_______________________________________________
void AttachmentWindow::_uniconify( void )
{
  Debug::Throw( "AttachmentWindow::_uniconify.\n" );
  uniconify();
  show();
  return;
}

//________________________________________
void AttachmentWindow::_displayEntry( Attachment& attachment )
{ 
  
  Debug::Throw( "AttachmentWindow::_displayEntry.\n");
    
  // retrieve associated entry
  LogEntry *entry( attachment.entry() );
  
  // check if entry is visible
  if( entry && !entry->isSelected() ) entry->setFindSelected( true );  
  static_cast<Application*>(qApp)->mainWindow().selectEntry( entry );
  
}
