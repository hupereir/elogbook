#ifndef ColorOptionListBox_h
#define ColorOptionListBox_h

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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "OptionListBox.h"


class ColorOptionListBox: public OptionListBox
{

    Q_OBJECT

    public:

    //! constructor
    ColorOptionListBox( QWidget*, const QString& );

    //! destructor
    virtual ~ColorOptionListBox( void )
    {}

    protected slots:

    //! add
    virtual void _add( void );

    //! edit
    virtual void _edit( void );

};

#endif
