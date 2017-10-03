#ifndef KeywordList_h
#define KeywordList_h


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

#include "TreeView.h"

//* local TreeView to store size hint
class KeywordList: public TreeView
{

    Q_OBJECT

    public:

    //* constructor
    explicit KeywordList( QWidget* parent = nullptr ):
        TreeView( parent )
        {}

    //*@name accessors
    //@{

    //* size
    QSize sizeHint() const override;

    //@}


    //*@name modifiers
    //@{

    //* default size
    void setDefaultWidth( int );

    //@}

    private:

    //* default width;
    int defaultWidth_ = -1;

};

#endif
