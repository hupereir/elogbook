#ifndef EditKeywordDialog_h
#define EditKeywordDialog_h

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
   \file EditKeywordDialog.h
   \brief Edit/create keyword popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/



#include "CustomComboBox.h"
#include "CustomDialog.h"
#include "Keyword.h"

//! Edit/create keyword popup dialog
class EditKeywordDialog: public CustomDialog
{

  public:

  //! constructor
  EditKeywordDialog( QWidget* parent );

  //! destructor
  virtual ~EditKeywordDialog( void )
  {}

  //! keyword
  void add( const Keyword& keyword )
  { combo_box_->addItem( keyword.get() ); }

  //! keyword
  void setKeyword( const Keyword& keyword )
  { combo_box_->setEditText( keyword.get() ); }

  //! keyword
  Keyword keyword( void ) const
  { return Keyword( combo_box_->currentText() ); }

  private:

  //!keyword combo box
  CustomComboBox *combo_box_;

};

#endif
