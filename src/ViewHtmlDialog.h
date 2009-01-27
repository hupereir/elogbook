#ifndef ViewHtmlDialog_h
#define ViewHtmlDialog_h

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
   \file ViewHtmlDialog.h
   \brief convert logbook/logEntry to HTML
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QString>
#include <QStringList>

#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "LineEditor.h"
#include "File.h"

//! new attachment popup dialog
class ViewHtmlDialog: public CustomDialog
{
  
  public:
      
  //! constructor
  ViewHtmlDialog( QWidget* parent );
  
  //! destructor
  virtual ~ViewHtmlDialog( void )
  {}

  //! set command manually
  void setCommand( QString command )
  { _commandEditor().setEditText( command ); }
  
  //! add commands to the combo-box list
  void addCommand( const std::string& command )
  { _commandEditor().addItem( command ); }
  
  //! commands
  QStringList commands( void ) const
  { 
    Debug::Throw() << "PrintDialog::commands - maxCount: " << _commandEditor().QComboBox::count() << std::endl;
    QStringList out;
    for( int row = 0; row < _commandEditor().QComboBox::count(); row++ )
    { out.push_back( _commandEditor().itemText( row ) ); }
    
    return out;
  }
  
  //! command
  QString command( void ) const
  { return _commandEditor().currentText(); }

  //! file
  void setFile( const File& file )
  { _destinationEditor().setFile( file ); }
  
  //! file
  File file( void ) const
  { return File( qPrintable( _destinationEditor().editor().text() ) ); }
  
  protected:
  
  //! destination
  BrowsedLineEditor& _destinationEditor( void ) const
  { return *destination_editor_; }
  
  //! command editor
  CustomComboBox& _commandEditor( void ) const
  { return *command_editor_; }  
  
  private:
  
  //! print command
  CustomComboBox* command_editor_;
  
  //! file
  BrowsedLineEditor *destination_editor_;  
      
};

#endif
