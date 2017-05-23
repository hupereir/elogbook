#ifndef ProgressBar_h
#define ProgressBar_h

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

#include "BaseProgressBar.h"
#include "Counter.h"

//* display command progress and remaining time
class ProgressBar:public BaseProgressBar, private Base::Counter<ProgressBar>
{

    Q_OBJECT

    public:

    //* constructor
    ProgressBar( QWidget* parent = nullptr ):
        BaseProgressBar( parent ),
        Counter( "ProgressBar" )
    {}

    public Q_SLOTS:

    //* set maximum
    virtual void setMaximum( int value )
    {
        BaseProgressBar::setMaximum( value );
        current_ = 0;
    }

    //* add to progress
    void addToProgress( int value )
    { setValue( current_ += value ); }

    private:

    //* current progress
    int current_ = 0;

};

#endif
