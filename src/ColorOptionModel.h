#ifndef ColorOptionModel_h
#define ColorOptionModel_h

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

#include "OptionModel.h"

#include "Color.h"
#include <QIcon>
#include <QMap>

//! Color option model. Stores Color option information for display in lists
class ColorOptionModel: public OptionModel
{

    public:

    //! constructor
    ColorOptionModel( QObject* parent = 0 ):
        OptionModel( parent )
    {}

    //! destructor
    virtual ~ColorOptionModel( void )
    {}

    // return job for a given index
    virtual QVariant data( const QModelIndex&, int ) const;

    private:

    //! icon cache
    using IconCache = QMap<Base::Color, QIcon>;

    //! icon cache
    static IconCache& _icons( void );

    //! icon matching color
    static QIcon _icon( const Base::Color& );


};

#endif
