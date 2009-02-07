#ifndef PrintDialog_h
#define PrintDialog_h

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
   \file PrintDialog.h
   \brief convert logbook/logEntry to HTML
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QString>

#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "CustomComboBox.h"
#include "LineEditor.h"
#include "File.h"

//! new attachment popup dialog
class PrintDialog: public CustomDialog
{
  
  Q_OBJECT
  
  public:
      
  //! constructor
  PrintDialog( QWidget* parent );
  
  //! destructor
  virtual ~PrintDialog( void )
  {}

  //! set command manually
  void setCommand( QString command )
  { _commandEditor().setEditText( command ); }
  
  //! add commands to the combo-box list
  void addCommand( QString command )
  { _commandEditor().addItem( command ); }
    
  //! command
  QString command( void ) const
  { return _commandEditor().currentText(); }

  //! file
  void setFile( const File& file )
  { _destinationEditor().setFile( file ); }
  
  //! file
  File file( void ) const
  { return _destinationEditor().editor().text(); }
  
  protected:
  
  //! destination
  BrowsedLineEditor& _destinationEditor( void ) const
  { return *destination_editor_; }
  
  //! command editor
  CustomComboBox& _commandEditor( void ) const
  { return *command_editor_; }  
  
  protected slots:
  
  //! browse print command
  void _browseCommand( void );

  private:
  
  //! print command
  CustomComboBox* command_editor_;
  
  //! file
  BrowsedLineEditor *destination_editor_;  
      
};

#endif
