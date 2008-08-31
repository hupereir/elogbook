
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
   \file NewLogbookDialog.cpp
   \brief new logbook popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLabel>
#include <QLayout>

#include "Debug.h"
#include "NewLogbookDialog.h"
#include "QtUtil.h"

using namespace std;

//_______________________________________________________
NewLogbookDialog::NewLogbookDialog( QWidget* parent ):
  CustomDialog( parent )
{
  
  Debug::Throw( "NewLogbookDialog::NewLogbookDialog.\n" );
  setWindowTitle( "eLogbook - new logbook" );
  QGridLayout *grid_layout( new QGridLayout() );
  grid_layout->setMargin(0);
  grid_layout->setSpacing(5);
  mainLayout().addLayout( grid_layout, 0 );

  // title
  grid_layout->addWidget( new QLabel( "Title: ", this ), 0, 0 );
  grid_layout->addWidget( title_ = new LineEditor( this ), 0, 1 );
  title_->setToolTip( "Logbook title" );
  
  // logbook author
  grid_layout->addWidget( new QLabel( "Author: ", this ), 1, 0 );
  grid_layout->addWidget( author_ = new LineEditor( this ), 1, 1 );
  author_->setToolTip( "Logbook author." );
  
  // attachment directory
  grid_layout->addWidget( new QLabel( "File: ", this ), 2, 0 );
  grid_layout->addWidget( file_ = new BrowsedLineEditor( this ), 2, 1 );
  file_->setFileMode( QFileDialog::AnyFile );
  file_->setToolTip( "File to which logbook entries are saved." );
  
  // attachment directory
  grid_layout->addWidget( new QLabel( "Attachment directory: ", this ), 3, 0 );
  grid_layout->addWidget( attachment_directory_ = new BrowsedLineEditor( this ), 3, 1 );
  attachment_directory_->setFileMode( QFileDialog::DirectoryOnly );
  attachment_directory_->setToolTip( "Default directory where attached files are stored (either copied or linked)." );
  
  grid_layout->setColumnStretch( 1, 1 );  
  
  // comments
  mainLayout().addWidget( new QLabel( "Comments:", this ), 0 );  
  mainLayout().addWidget( comments_ = new TextEditor( this ), 1 );
  comments_->setToolTip( "Logbook comments." );
  Debug::Throw( "NewLogbookDialog::NewLogbookDialog - done.\n" );
  
  adjustSize();
  
}
