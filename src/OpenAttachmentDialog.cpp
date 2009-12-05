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
   \file OpenAttachmentDialog.cpp
   \brief open attachment popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QTextStream>

#include "AnimatedLineEditor.h"
#include "GridLayout.h"
#include "TextEditor.h"
#include "Debug.h"
#include "File.h"
#include "OpenAttachmentDialog.h"

using namespace std;

//_____________________________________________________
OpenAttachmentDialog::OpenAttachmentDialog( QWidget* parent, const Attachment& attachment ):
  CustomDialog( parent )
{

  Debug::Throw( "OpenAttachmentDialog::OpenAttachmentDialog.\n" );

  GridLayout* grid_layout = new GridLayout();
  grid_layout->setMargin(0);
  grid_layout->setMaxCount(2);
  grid_layout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
  mainLayout().addLayout( grid_layout, 0 );

  // file name
  grid_layout->addWidget(new QLabel( "File: ", this ) );
  AnimatedLineEditor* file_line_edit = new AnimatedLineEditor( this );
  grid_layout->addWidget( file_line_edit );
  file_line_edit->setReadOnly( true );
  file_line_edit->setToolTip( "Attachment file/URL. (read-only)" );

  File fullname( ( attachment.type() == AttachmentType::URL ) ? attachment.file() : attachment.file().expand() );
  file_line_edit->setText( fullname );

  // attachment type
  grid_layout->addWidget(new QLabel( "Type: ", this ) );
  grid_layout->addWidget( new QLabel( attachment.type().name(), this ) );

  // creation
  grid_layout->addWidget( new QLabel( "Created: ", this ) );
  grid_layout->addWidget( new QLabel( attachment.creation().isValid() ? attachment.creation().toString():"-", this ) );

  // modification
  grid_layout->addWidget( new QLabel( "Modified: ", this ) );
  grid_layout->addWidget( new QLabel( attachment.modification().isValid() ? attachment.modification().toString():"-", this ) );

  // horizontal separator
  QFrame* frame;
  mainLayout().addWidget( frame = new QFrame( this ) );
  frame->setFrameStyle( QFrame::HLine | QFrame::Sunken );

  // radio buttons
  QButtonGroup* group = new QButtonGroup( this );
  group->setExclusive( true );

  grid_layout = new GridLayout();
  mainLayout().addLayout( grid_layout );
  grid_layout->setMargin(0);
  grid_layout->setMaxCount(2);

  grid_layout->addWidget( openRadioButton_ = new QRadioButton( "Open using: ", this ) );
  openRadioButton_->setToolTip( "Select this button to open attachment using the selected application." );
  group->addButton( openRadioButton_ );

  grid_layout->addWidget( commandEditor_ = new BrowsedLineEditor( this ) );
  commandEditor_->setFile( attachment.type().editCommand() );
  commandEditor_->setToolTip( "Application to be used to display the attachment." );

  grid_layout->addWidget( saveRadioButton_ = new QRadioButton( "save to disk ", this ) );
  saveRadioButton_->setToolTip( "Select this button to save a copy of the attachment on disk." );
  group->addButton( saveRadioButton_ );

  if( attachment.type() == AttachmentType::URL )
  {

    openRadioButton_->setChecked( true );
    saveRadioButton_->setChecked( false );
    saveRadioButton_->setEnabled( false );

  } else {

    openRadioButton_->setChecked( true );
    saveRadioButton_->setChecked( false );
    saveRadioButton_->setEnabled( true );
  }

  // comments
  if( !( attachment.comments().isEmpty() || attachment.comments()== Attachment::NO_COMMENTS ) )
  {
    mainLayout().addWidget( new QLabel( "Comments:", this ), 0 );
    TextEditor* comments_text_edit = new TextEditor( this );
    mainLayout().addWidget( comments_text_edit, 1 );
    comments_text_edit->setReadOnly( true );
    comments_text_edit->setPlainText( attachment.comments() );
    comments_text_edit->setToolTip( "Attachment comments. (read-only)" );
  }

  adjustSize();

}

//______________________________________________________
QString OpenAttachmentDialog::command( void ) const
{ return commandEditor_->editor().text(); }

//______________________________________________________
OpenAttachmentDialog::Action OpenAttachmentDialog::action( void ) const
{ return openRadioButton_->isChecked() ? OPEN:SAVE_AS; }
