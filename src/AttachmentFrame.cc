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
  \file AttachmentFrame.cc
  \brief popup window to list/edit all attachments independantly from entries
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QShortcut>
#include <QLayout>

#include "AttachmentFrame.h"
#include "EditFrame.h"
#include "KeywordList.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryList.h"
#include "MainFrame.h"
#include "Options.h"
#include "QtUtil.h"
#include "SelectionFrame.h"

using namespace std;
using namespace Qt;

//________________________________________
AttachmentFrame::AttachmentFrame( QWidget* parent ):
  QWidget( parent ),
  Counter( "AttachmentFrame" )
{
  
  Debug::Throw( "AttachmentFrame::AttachmentFrame.\n" );
  setWindowTitle( MainFrame::ATTACHMENT_TITLE );
  
  // create vbox layout
  QVBoxLayout* layout=new QVBoxLayout(this);
  layout->setMargin( 10 );
  layout->setSpacing( 10 );
  setLayout( layout );
  
  layout->addWidget( list_ = new AttachmentList( this, true ) );
  connect( list_, SIGNAL( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem* ) ), SLOT( _displayEntry( QTreeWidgetItem*, QTreeWidgetItem* ) ) );
  
  // shortcuts
  connect( new QShortcut( CTRL+Key_Q, this ), SIGNAL( activated() ), qApp, SLOT( closeAllWindows() ) );
  connect( new QShortcut( CTRL+Key_W, this ), SIGNAL( activated() ), SLOT( close() ) );
  
  // configuration
  updateConfiguration();
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( updateConfiguration() ) );

};

//________________________________________
void AttachmentFrame::updateConfiguration( void )
{
  
  Debug::Throw( "AttachmentFrame::updateConfiguration.\n" );
  
  // resize window
  resize( XmlOptions::get().get<int>( "ATC_FRAME_WIDTH" ), XmlOptions::get().get<int>( "ATC_FRAME_HEIGHT" ) );  
  
}    

//________________________________________
void AttachmentFrame::show( void )
{
  Debug::Throw( "AttachmentFrame::show.\n" );
  QtUtil::centerOnPointer( this, false );
  QWidget::show();
  QWidget::raise();
}
  
//____________________________________________
void AttachmentFrame::enterEvent( QEvent *event )
{
  Debug::Throw( "SelectionFrame::enterEvent.\n" );
  
  // base class enterEvent
  QWidget::enterEvent( event );
  static_cast<MainFrame*>(qApp)->selectionFrame().checkLogbookModified();
    
  return;
}

//_______________________________________________
void AttachmentFrame::uniconify( void )
{
  Debug::Throw( "AttachmentFrame::uniconify.\n" );
  show();
  QtUtil::uniconify( window() );
  return;
}

//________________________________________
void AttachmentFrame::_displayEntry( QTreeWidgetItem *current, QTreeWidgetItem* old )
{
  
  Debug::Throw( "AttachmentFrame::_displayEntry.\n");
  
  if( !current ) current = old;
  
  AttachmentList::Item *item = static_cast<AttachmentList::Item*>( current );
  Exception::checkPointer( item, DESCRIPTION( "wrong item" ) );
  
  // retrieve associated entry
  LogEntry *entry( item->attachment()->entry() );
  
  // check if entry is visible
  static_cast<MainFrame*>(qApp)->selectionFrame().clearSelection();
  
  if( entry && !entry->isSelected() ) entry->setFindSelected( true );  
  static_cast<MainFrame*>(qApp)->selectionFrame().selectEntry( entry );
  
}
