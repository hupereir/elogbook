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
   \file LogbookStatisticsDialog.cpp
   \brief  logbook informations
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLayout>
#include <QLabel>
#include <QPushButton>

#include "TreeView.h"
#include "LineEditor.h" 
#include "Debug.h"
#include "Logbook.h"
#include "LogbookStatisticsDialog.h"
#include "LogEntry.h"
#include "Util.h"

using namespace std;

//_________________________________________________________
LogbookStatisticsDialog::LogbookStatisticsDialog( QWidget* parent, Logbook* logbook ):
  CustomDialog( parent, CustomDialog::OK_BUTTON )
{
  Debug::Throw( "LogbookStatisticsDialog::LogbookStatisticsDialog.\n" );

  setWindowTitle( "eLogbook - logbook statistics" );
  _setSizeOptionName( "LOGBOOK_STATISTICS_DIALOG" );
  
  QGridLayout* grid_layout = new QGridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  mainLayout().addLayout( grid_layout, 0 );
  
  // file
  grid_layout->addWidget( new QLabel( "File: ", this ), 0, 0 );
  
  // create a readonly line editor for the file name
  LineEditor* edit = new LineEditor( this );
  grid_layout->addWidget( edit, 0, 1 );
  edit->setReadOnly( true );
  edit->setText( logbook->file().expand().c_str() );
  
  // creation time
  if( logbook->creation().isValid() ) 
  {
    grid_layout->addWidget( new QLabel( "Created: ", this ), 1, 0 );
    grid_layout->addWidget( new QLabel( logbook->creation().string().c_str(), this ), 1, 1 );
  }
  
  // modification time
  if( logbook->modification().isValid() ) 
  {
    grid_layout->addWidget( new QLabel( "Last modified: ", this ), 2, 0 );
    grid_layout->addWidget( new QLabel( logbook->modification().string().c_str(), this ), 2, 1 );
  }
   
  // backup time
  if( logbook->backup().isValid() ) 
  {
    grid_layout->addWidget( new QLabel( "Last backup: ", this ), 3, 0 );
    grid_layout->addWidget( new QLabel( logbook->backup().string().c_str(), this ), 3, 1 );
  }
  
  // stores all children
    
  // total number of entries
  grid_layout->addWidget( new QLabel( "Entries: ", this ), 4, 0 );
  grid_layout->addWidget( new QLabel( Str().assign<unsigned int>(logbook->entries().size()).c_str(), this ), 4, 1 );
  
  // total number of attachments
  grid_layout->addWidget( new QLabel( "Attachments: ", this ), 5, 0 );
  grid_layout->addWidget( new QLabel( Str().assign<unsigned int>( logbook->attachments().size() ).c_str(), this ), 5, 1 );

  grid_layout->setColumnStretch( 1, 1 );
  
  
  // detail
  TreeView *list_view( new TreeView( this ) );
  list_view->setModel( &model_ );
  list_view->setSortingEnabled( false );
  mainLayout().addWidget( list_view, 1 );

  list< Logbook* > all( logbook->children() );
  all.push_front( logbook ); 
  model_.add( Model::List( all.begin(), all.end() ) );
  
  list_view->resizeColumns();
  
}


//_______________________________________________
const char* LogbookStatisticsDialog::Model::column_titles_[ LogbookStatisticsDialog::Model::n_columns ] =
{ 
  "file",
  "entries",
  "created",
  "modified"
};


//_______________________________________________________________________________________
QVariant LogbookStatisticsDialog::Model::data( const QModelIndex& index, int role ) const
{
  
  // check index, role and column
  if( !index.isValid() ) return QVariant();
  
  // retrieve associated file info
  Logbook& logbook( *get()[index.row()] );
     // return text associated to file and column
  if( role == Qt::DisplayRole ) 
  {
    
    switch( index.column() )
    {
      
      case FILE: return logbook.file().localName().c_str();
      case ENTRIES: return BASE::KeySet<LogEntry>(&logbook).size();
      case CREATED: return logbook.creation().string().c_str();
      case MODIFIED: return logbook.modification().string().c_str();
      default: return QVariant();
    }
  }
  
  return QVariant();
  
}
