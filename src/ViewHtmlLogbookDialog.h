#ifndef ViewHtmlLogbookDialog_h
#define ViewHtmlLogbookDialog_h

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
   \file ViewHtmlLogbookDialog.h
   \brief new logbook popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QCheckBox>
#include <QRadioButton>
#include <map>
#include <string>

#include "BrowsedLineEdit.h"
#include "CustomDialog.h"
#include "LineEditor.h"
#include "File.h"
#include "QtUtil.h"

//! new attachment popup dialog
class ViewHtmlLogbookDialog: public CustomDialog
{
  
  public:

  //! selection mode
  enum Selection
  {
    ALL,
    VISIBLE,
    SELECTED
  };
          
  //! constructor
  ViewHtmlLogbookDialog( QWidget* parent );
  
  //! destructor
  virtual ~ViewHtmlLogbookDialog( void )
  {}

  //! selection
  void setSelection( const Selection& selection )
  { radio_buttons_[selection]->setChecked( true ); }
  
  //! selection
  Selection selection( void ) const
  {
    for( RadioButtonMap::const_iterator iter = radio_buttons_.begin(); iter != radio_buttons_.end(); iter++ )
    if( iter->second->isChecked() ) return iter->first;
    return ALL;
  }  

  //! true if selection is ALL
  bool allEntries( void ) const
  { return selection() == ALL; }
  
  //! true if selection is VISIBLE
  bool visibleEntries( void ) const
  { return selection() == VISIBLE; }

  //! true if selection is VISIBLE
  bool selectedEntries( void ) const
  { return selection() == SELECTED; }
  
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
  
  //! shortcut to map of Radio buttons
  typedef std::map<Selection, QRadioButton*> RadioButtonMap;

  //! shortcut to map of check boxes    
  typedef std::map<unsigned int, QCheckBox*> CheckBoxMap;
        
  //! selection radio button
  RadioButtonMap radio_buttons_;    
      
  //! checkboxes
  CheckBoxMap logbook_check_boxes_;
  
  //! checkboxes
  CheckBoxMap entry_check_boxes_;
  
  //! command
  BrowsedLineEdit *command_;
  
  //! file
  BrowsedLineEdit *file_;  
      
};

#endif
