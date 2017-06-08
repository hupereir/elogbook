#ifndef LogbookStatisticsDialog_h
#define LogbookStatisticsDialog_h

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

#include "Counter.h"
#include "CustomDialog.h"
#include "LogbookModel.h"

class Logbook;

class LogbookStatisticsDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //* constructor
    explicit LogbookStatisticsDialog( QWidget*, Logbook* );

    private:

    //* model
    LogbookModel model_;

};

#endif
