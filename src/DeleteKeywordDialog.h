#ifndef DeleteKeywordDialog_h
#define DeleteKeywordDialog_h

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

#include "Dialog.h"
#include "Keyword.h"

#include <QRadioButton>

//* delete keyword popup dialog
class DeleteKeywordDialog: public Dialog
{

    Q_OBJECT

    public:

    //* constructor
    explicit DeleteKeywordDialog( QWidget* parent, const Keyword::List&, bool hasEntries );

    //* entry action
    enum class Action
    {
        MoveEntries,
        DeleteEntries
    };

    //* get action
    Action action() const
    { return moveRadioButton_->isChecked() ? Action::MoveEntries : Action::DeleteEntries; }

    //* move netries
    bool moveEntries() const
    { return action() == Action::MoveEntries; }

    //* delete entries
    bool deleteEntries() const
    { return action() == Action::DeleteEntries; }

    private:


    //* open with radio button
    QRadioButton* moveRadioButton_ = nullptr;

    //* save as radio button
    QRadioButton* deleteRadioButton_ = nullptr;

};

#endif
