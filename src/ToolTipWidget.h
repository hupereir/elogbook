#ifndef ToolTipWidget_h
#define ToolTipWidget_h

/******************************************************************************
*
* Copyright (C) 2017 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
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

#include "BaseToolTipWidget.h"

#include <QLabel>

class GridLayoutItem;
class LogEntry;

class ToolTipWidget: public BaseToolTipWidget
{

    Q_OBJECT

    public:

    //* constructor
    explicit ToolTipWidget( QWidget* = nullptr );

    //* assign logbook entry
    void setLogEntry( LogEntry* );

    private:

    //* title
    QLabel* titleLabel_ = nullptr;

    //*@name items
    //@{
    GridLayoutItem* authorItem_ = nullptr;
    GridLayoutItem* createdItem_ = nullptr;
    GridLayoutItem* modifiedItem_ = nullptr;
    //@}

};

#endif
