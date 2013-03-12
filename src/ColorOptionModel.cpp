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

#include "ColorOptionModel.h"

#include "CustomPixmap.h"
#include "IconSize.h"

#include <QPainter>

//_______________________________________________
ColorOptionModel::IconCache& ColorOptionModel::_icons( void )
{
    static IconCache singleton;
    return singleton;
}

//__________________________________________________________________
QVariant ColorOptionModel::data( const QModelIndex& index, int role ) const
{

    // check index, role and column
    if( !index.isValid() ) return QVariant();

    // retrieve associated color
    const BASE::Color& color( get( index ).second.get<BASE::Color>() );

    if( role == Qt::DisplayRole && index.column() == VALUE ) return color.name();
    else if( role == Qt::DecorationRole && index.column() == VALUE ) return _icon( color );
    else return OptionModel::data( index, role );

}

//______________________________________________________
QIcon ColorOptionModel::_icon( const BASE::Color& color )
{

    IconCache::iterator iter( _icons().find( color ) );
    if( iter != _icons().end() ) return iter.value();
    else {

        QPixmap pixmap( CustomPixmap().empty( IconSize( IconSize::Large ) ) );
        QPainter painter( &pixmap );
        painter.setPen( Qt::black );
        painter.setBrush( color );
        painter.setRenderHints(QPainter::Antialiasing );

        QRect rect( pixmap.rect() );
        rect.adjust( 2, 2, -2, -2 );

        painter.drawEllipse( rect );

        QIcon icon( pixmap );
        _icons()[color] = icon;
        return icon;

    }

}