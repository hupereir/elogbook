
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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "EditKeywordDialog.h"
#include "Debug.h"

#include <QLabel>

//_____________________________________________________
EditKeywordDialog::EditKeywordDialog( QWidget* parent ):
    CustomDialog( parent, OkButton|CancelButton|Separator )
{

    Debug::Throw( "EditKeywordDialog::EditKeywordDialog.\n" );

    setWindowTitle( tr( "Edit Keyword - Elogbook" ) );
    mainLayout().addWidget( combobox_ = new CustomComboBox( this ) );
    combobox_->setEditable( true );
    combobox_->setAutoCompletion( true );

    mainLayout().addWidget( new QLabel( tr( "use \"/\" characters to add keyword to a specific branch" ), this ) );

}
