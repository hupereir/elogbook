
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
   \file ViewHtmlEntryDialog.cc
   \brief new logbook popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QPushButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLayout>

#include "ViewHtmlEntryDialog.h"
#include "Logbook.h"
#include "LogEntry.h"

using namespace std;

//___________________________________________________
ViewHtmlEntryDialog::ViewHtmlEntryDialog( QWidget* parent ):
    CustomDialog( parent )
{
  Debug::Throw( "ViewHtmlEntryDialog::ViewHtmlEntryDialog.\n" );

  setWindowTitle( "eLogbook - HTML" );
  mainLayout().setSpacing(2);

  // command
  mainLayout().addWidget( new QLabel( "Command:", this ) );
  mainLayout().addWidget( command_ = new BrowsedLineEdit( this ) );
  command_->setMode( QFileDialog::ExistingFile );
  
  // file
  mainLayout().addWidget( new QLabel( "File:", this ) );
  mainLayout().addWidget( file_ = new BrowsedLineEdit( this ) );
  file_->setMode( QFileDialog::AnyFile );

  QPushButton* button = new QPushButton( "more ... ", this );
  button->setCheckable( true );
  mainLayout().addWidget( button );
  connect( button, SIGNAL( toggled( bool ) ), SLOT( showExtension( bool ) ) );

  // extension widget
  QWidget *extension = new QWidget( this );
  extension->setLayout( new QHBoxLayout() );
  extension->layout()->setMargin(10);
  extension->layout()->setSpacing(5);
  setExtension( extension );
  setOrientation( Qt::Vertical );
  
  QGroupBox* group_box = new QGroupBox( "logbook configuration", extension );
  group_box->setLayout( new QVBoxLayout() );
  group_box->layout()->setSpacing( 5 );
  group_box->layout()->setMargin( 5 );
  extension->layout()->addWidget( group_box );
    
  group_box->layout()->addWidget( logbook_check_boxes_[Logbook::HTML_TITLE] = new QCheckBox( "Title", group_box ) );
  group_box->layout()->addWidget( logbook_check_boxes_[Logbook::HTML_AUTHOR] = new QCheckBox( "Author", group_box ) );
  group_box->layout()->addWidget( logbook_check_boxes_[Logbook::HTML_CREATION] = new QCheckBox( "Creation", group_box ) );
  group_box->layout()->addWidget( logbook_check_boxes_[Logbook::HTML_MODIFICATION] = new QCheckBox( "Last modification", group_box ) );
  group_box->layout()->addWidget( logbook_check_boxes_[Logbook::HTML_BACKUP] = new QCheckBox( "Last backup", group_box ) );
  group_box->layout()->addWidget( logbook_check_boxes_[Logbook::HTML_FILE] = new QCheckBox( "File", group_box ) );
  group_box->layout()->addWidget( logbook_check_boxes_[Logbook::HTML_DIRECTORY] = new QCheckBox( "Attachment directory", group_box ) );
  group_box->layout()->addWidget( logbook_check_boxes_[Logbook::HTML_COMMENTS] = new QCheckBox( "Comments", group_box ) );
      
  
  group_box = new QGroupBox( "Entry configuration", extension );
  group_box->setLayout( new QVBoxLayout() );
  group_box->layout()->setSpacing( 5 );
  group_box->layout()->setMargin( 5 );
  extension->layout()->addWidget( group_box );
  
  group_box->layout()->addWidget( entry_check_boxes_[LogEntry::HTML_TITLE] = new QCheckBox( "Title", group_box ) );
  group_box->layout()->addWidget( entry_check_boxes_[LogEntry::HTML_KEYWORD] = new QCheckBox( "Keyword", group_box ) );
  group_box->layout()->addWidget( entry_check_boxes_[LogEntry::HTML_CREATION] = new QCheckBox( "Creation", group_box ) );
  group_box->layout()->addWidget( entry_check_boxes_[LogEntry::HTML_MODIFICATION] = new QCheckBox( "Last modification", group_box ) );
  group_box->layout()->addWidget( entry_check_boxes_[LogEntry::HTML_AUTHOR] = new QCheckBox( "Author", group_box ) );
  group_box->layout()->addWidget( entry_check_boxes_[LogEntry::HTML_ATTACHMENT] = new QCheckBox( "Attachment", group_box ) );
  group_box->layout()->addWidget( entry_check_boxes_[LogEntry::HTML_TEXT] = new QCheckBox( "Text", group_box ) );
  
}
