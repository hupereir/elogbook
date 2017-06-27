#ifndef EditKeywordDialog_h
#define EditKeywordDialog_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "CustomComboBox.h"
#include "CustomDialog.h"
#include "Keyword.h"

//* Edit/create keyword popup dialog
class EditKeywordDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //* constructor
    explicit EditKeywordDialog( QWidget* );

    //* keyword
    void add( const Keyword& keyword )
    { combobox_->addItem( keyword.get() ); }

    //* keyword
    void setKeyword( const Keyword& keyword )
    { combobox_->setEditText( keyword.get() ); }

    //* keyword
    Keyword keyword() const
    { return Keyword( combobox_->currentText() ); }

    private:

    //*keyword combo box
    CustomComboBox *combobox_ = nullptr;

};

#endif
