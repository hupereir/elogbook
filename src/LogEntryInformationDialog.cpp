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
  \file LogEntryInformationDialog.cc
  \brief  logbook entry informations
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>
#include <QPushButton>

#include "CustomPixmap.h"
#include "Debug.h"
#include "Icons.h"
#include "LogEntryInformationDialog.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "Options.h"
#include "QtUtil.h"


using namespace std;
using namespace BASE;

//_________________________________________________________
LogEntryInformationDialog::LogEntryInformationDialog( QWidget* parent, LogEntry* entry ):
    QDialog( parent ),
    Counter( "LogEntryInformationDialog" )
{
  Debug::Throw( "LogEntryInformationDialog::LogEntryInformationDialog.\n" );

  //! try load Question icon
  static CustomPixmap pixmap;
  static bool first( true );
  if( first )
  {
    first = false;
    list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
    pixmap.find( ICONS::INFO, path_list );
  }
  
  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(10);
  layout->setSpacing(10);
  setLayout( layout );
  
  QHBoxLayout* h_layout = new QHBoxLayout();
  h_layout->setSpacing(10);
  h_layout->setMargin(0);
  layout->addLayout( h_layout );
  
  QLabel* label = new QLabel(this);
  label->setPixmap( pixmap );
  h_layout->addWidget( label );
  
  QGridLayout *grid_layout = new QGridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  h_layout->addLayout( grid_layout );
  
  grid_layout->addWidget( new QLabel( "Title: ", this ), 0, 0 );
  grid_layout->addWidget( new QLabel( entry->title().c_str(), this ), 0, 1 );

  grid_layout->addWidget( new QLabel( "Keyword: ", this ), 1, 0 );
  grid_layout->addWidget( new QLabel( entry->keyword().c_str(), this ), 1, 1 );

  grid_layout->addWidget( new QLabel( "Author: ", this ), 2, 0 );
  grid_layout->addWidget( new QLabel( entry->author().c_str(), this ), 2, 1 );

  grid_layout->addWidget( new QLabel( "Creation: ", this ), 3, 0 );
  grid_layout->addWidget( new QLabel( entry->creation().string().c_str(), this ), 3, 1 );

  grid_layout->addWidget( new QLabel( "Modification: ", this ), 4, 0 );
  grid_layout->addWidget( new QLabel( entry->modification().string().c_str(), this ), 4, 1 );
  
  // retrieve associated logbook
  int i=5;
  KeySet<Logbook> logbooks( entry );
  for( KeySet<Logbook>::iterator iter = logbooks.begin(); iter!=logbooks.end(); iter++, i++ )
  {
    
    grid_layout->addWidget( new QLabel( "File: ", this ), i, 0 );
    grid_layout->addWidget( new QLabel( File( (*iter)->file() ).localName().c_str(), this ), i, 1);
    
  }
  
  QPushButton* button = new QPushButton( "&Ok", this );
  layout->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( close() ) );

  adjustSize();
  
}
