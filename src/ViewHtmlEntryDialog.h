#ifndef ViewHtmlEntryDialog_h
#define ViewHtmlEntryDialog_h

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
   \file ViewHtmlEntryDialog.h
   \brief new logbook popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QCheckBox>
#include <map>
#include <string>

#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "ImprovedLineEditor.h"
#include "File.h"
#include "QtUtil.h"

//! new attachment popup dialog
class ViewHtmlEntryDialog: public CustomDialog
{
  
  public:
      
  //! constructor
  ViewHtmlEntryDialog( QWidget* parent );
  
  //! destructor
  virtual ~ViewHtmlEntryDialog( void )
  {}

  //! mask
  void setLogbookMask( const unsigned int& mask )
  {
    for( CheckBoxMap::iterator iter = logbook_check_boxes_.begin(); iter != logbook_check_boxes_.end(); iter++ )
    iter->second->setChecked( mask & iter->first );  
  }

  //! mask
  unsigned int logbookMask( void ) const
  {
    unsigned int out( 0 );
    for( CheckBoxMap::const_iterator iter = logbook_check_boxes_.begin(); iter != logbook_check_boxes_.end(); iter++ )
    if( iter->second->isChecked() ) out |= iter->first;
    return out;
  }

  //! mask
  void setEntryMask( const unsigned int& mask )
  {
    for( CheckBoxMap::iterator iter = entry_check_boxes_.begin(); iter != entry_check_boxes_.end(); iter++ )
    iter->second->setChecked( mask & iter->first );
  }

  //! mask
  unsigned int entryMask( void ) const
  {
    unsigned int out( 0 );
    for( CheckBoxMap::const_iterator iter = entry_check_boxes_.begin(); iter != entry_check_boxes_.end(); iter++ )
    if( iter->second->isChecked() ) out |= iter->first;
    return out;
  }
    
  //! command
  void setCommand( const std::string& file )
  { 
    command_->setFile( file ); 
    QtUtil::expand( &command_->editor() );
  } 
  
  //! command
  std::string command( void ) const
  { return qPrintable( command_->editor().text() ); }
  
  //! file
  void setFile( const File& file )
  { 
    file_->setFile( file ); 
    QtUtil::expand( &file_->editor() );
  }
  
  //! file
  File file( void ) const
  { return File( qPrintable( file_->editor().text() ) ); }
  
  private:
  
  //! map mask bits to checkboxes
  typedef std::map<unsigned int, QCheckBox*> CheckBoxMap;
      
  //! checkboxes
  CheckBoxMap logbook_check_boxes_;
  
  //! checkboxes
  CheckBoxMap entry_check_boxes_;
  
  //! command
  BrowsedLineEditor *command_;
  
  //! file
  BrowsedLineEditor *file_;  
      
};

#endif
