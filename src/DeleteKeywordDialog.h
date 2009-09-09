#ifndef DeleteKeywordDialog_h
#define DeleteKeywordDialog_h

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
   \file DeleteKeywordDialog.h
   \brief Delete keyword popup dialog
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QRadioButton>

#include <vector>

#include "CustomDialog.h"
#include "Keyword.h"

//! delete keyword popup dialog
class DeleteKeywordDialog: public CustomDialog
{

  public:

  //! constructor
  DeleteKeywordDialog( QWidget* parent, const std::vector<Keyword>&, const bool& has_entries );

  //! destructor
  virtual ~DeleteKeywordDialog( void )
  {}

  //! entry action
  enum Action
  {
    //! delete file from disk
    MOVE,

    //! delete file from logbook
    DELETE

  };

  //! get action
  Action action( void ) const
  { return move_radio_button_->isChecked() ? MOVE:DELETE; }

  //! move netries
  bool moveEntries( void ) const
  { return action() == MOVE; }

  //! delete entries
  bool deleteEntries( void ) const
  { return action() == DELETE; }

  private:


  //! open with radio button
  QRadioButton* move_radio_button_;

  //! save as radio button
  QRadioButton* delete_radio_button_;


};

#endif
