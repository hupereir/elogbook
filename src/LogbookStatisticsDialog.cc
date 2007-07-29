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
   \file LogbookStatisticsDialog.cc
   \brief  logbook informations
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLayout>

#include "CustomListView.h"
#include "CustomLineEdit.h" 
#include "Debug.h"
#include "Logbook.h"
#include "LogbookStatisticsDialog.h"
#include "LogEntry.h"
#include "QtUtil.h"
#include "Util.h"

using namespace std;
using namespace BASE;

//_________________________________________________________
LogbookStatisticsDialog::LogbookStatisticsDialog( QWidget* parent, Logbook* logbook ):
    QDialog( parent ),
    Counter( "LogbookStatisticsDialog" )
{
  Debug::Throw( "LogbookStatisticsDialog::LogbookStatisticsDialog.\n" );

  setWindowTitle( "eLogbook - logbook statistics" );
  
  QVBoxLayout* layout( new QVBoxLayout() );
  layout->setMargin( 10 );
  layout->setSpacing( 10 );
  setLayout( layout );
  
  QGridLayout* grid_layout = new QGridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  layout->addLayout( grid_layout, 0 );
  
  // file
  grid_layout->addWidget( new QLabel( "File: ", this ), 0, 0 );
  
  // create a readonly line editor for the file name
  CustomLineEdit* edit = new CustomLineEdit( this );
  grid_layout->addWidget( edit, 0, 1 );
  edit->setReadOnly( true );
  edit->setText( logbook->file().expand().c_str() );
  QtUtil::expand( edit, logbook->file().expand() );
  
  // creation time
  if( logbook->creation().isValid() ) {
    grid_layout->addWidget( new QLabel( "Created: ", this ), 1, 0 );
    grid_layout->addWidget( new QLabel( logbook->creation().string().c_str(), this ), 1, 1 );
  }
  
  // modification time
  if( logbook->modification().isValid() ) {
    grid_layout->addWidget( new QLabel( "Last modified: ", this ), 2, 0 );
    grid_layout->addWidget( new QLabel( logbook->modification().string().c_str(), this ), 2, 1 );
  }
   
  // backup time
  if( logbook->backup().isValid() ) {
    grid_layout->addWidget( new QLabel( "Last backup: ", this ), 3, 0 );
    grid_layout->addWidget( new QLabel( logbook->backup().string().c_str(), this ), 3, 1 );
  }
  
  // stores all children
    
  // total number of entries
  grid_layout->addWidget( new QLabel( "Entries: ", this ), 4, 0 );
  grid_layout->addWidget( new QLabel( Str().assign<unsigned int>(logbook->entries().size()) .c_str(), this ), 4, 1 );
  
  // total number of attachments
  grid_layout->addWidget( new QLabel( "Attachments: ", this ), 5, 0 );
  grid_layout->addWidget( new QLabel( Str().assign<unsigned int>( logbook->attachments().size() ).c_str(), this ), 5, 1 );

  grid_layout->setColumnStretch( 1, 1 );
  
  
  // detail
  list< Logbook* > all( logbook->children() );
  all.push_front( logbook ); 

  CustomListView *list_view( new CustomListView( this ) );
  layout->addWidget( list_view, 1 );
  
  enum Columns{ FILE, ENTRIES, MODIFIED }; 
  list_view->setColumnCount( 3 );
  list_view->setColumnName( FILE, "file" );
  list_view->setColumnName( ENTRIES, "entries" );
  list_view->setColumnName( MODIFIED, "last modified" );
  
  list_view->setColumnType( ENTRIES, CustomListView::NUMBER );
  
  for( list< Logbook* >::iterator it=all.begin(); it!= all.end(); it++ )
  {
    
    CustomListView::Item* item( new CustomListView::Item( list_view ) );
        
    // second column get the file name
    item->setText( FILE, (*it)->file().localName().c_str() );
    
    // third column get the number of entries
    item->setText( ENTRIES, Str().assign<unsigned int>( KeySet<LogEntry>(*it).size() ).c_str() );

    // last modificatino
    item->setText( MODIFIED, (*it)->modification().string().c_str() );
    
  }

  resize( 350, 350 );
  list_view->resizeColumnToContents( ENTRIES );
  list_view->resizeColumnToContents( MODIFIED );
  list_view->resizeColumnToContents( FILE );
  
}
