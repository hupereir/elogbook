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
    const Base::Color& color( get( index ).second.get<Base::Color>() );

    if( role == Qt::DisplayRole && index.column() == Value ) return color.name();
    else if( role == Qt::DecorationRole && index.column() == Value ) return _icon( color );
    else return OptionModel::data( index, role );

}

//______________________________________________________
QIcon ColorOptionModel::_icon( const Base::Color& color )
{

    IconCache::iterator iter( _icons().find( color ) );
    if( iter != _icons().end() ) return iter.value();
    else {


        CustomPixmap pixmap( IconSize( IconSize::Large ), CustomPixmap::AllFlags );
        QRect rect( QPoint(0,0), pixmap.size()/pixmap.devicePixelRatio() );
        rect.adjust( 2, 2, -2, -2 );

        QPainter painter( &pixmap );
        painter.setRenderHints(QPainter::Antialiasing );

        painter.setPen( Qt::NoPen );
        painter.setBrush( color );
        painter.drawEllipse( rect );

        painter.setPen( QPalette().color( QPalette::Text ) );
        painter.setBrush( Qt::NoBrush );
        painter.drawEllipse( rect );

        QIcon icon( pixmap );
        _icons()[color] = icon;
        return icon;

    }

}
