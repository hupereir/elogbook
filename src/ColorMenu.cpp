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

#include "ColorMenu.h"

#include "BaseIconNames.h"
#include "CppUtil.h"
#include "IconEngine.h"
#include "IconSize.h"

#include <QColorDialog>
#include <QPainter>

//_______________________________________________
ColorMenu::ColorMenu( QWidget* parent ):
    QMenu( parent ),
    Counter( "ColorMenu" )
{
    Debug::Throw( "ColorMenu::ColorMenu.\n" );

    // new color action
    addAction( IconEngine::get( IconNames::Add ), tr( "New" ), this, &ColorMenu::_new );
    addSeparator();

    // default color action
    addAction( tr( "Default" ), this, &ColorMenu::_default );

    connect( this, &QMenu::triggered, this, &ColorMenu::_selected );
    connect( this, &QMenu::aboutToShow, this, &ColorMenu::_display );
}

//_______________________________________________
void ColorMenu::add( const QColor& color )
{
    if( !color.isValid() ) return;

    Base::Color copy( color );
    auto iter = colors_.lowerBound( copy );
    if( iter == colors_.end() || !Base::areEquivalent( copy, *iter ) )
    { colors_.insert( iter, copy ); }

    return;
}

//_______________________________________________
void ColorMenu::paintEvent( QPaintEvent* event )
{

    static const int margin = 5;

    // default paint
    QMenu::paintEvent( event );

    // loop over actions associated to existing colors
    QPainter painter( this );
    painter.setClipRect( event->rect() );

    painter.setPen( Qt::NoPen );
    for( auto&& iter = actions_.begin(); iter != actions_.end(); ++iter )
    {
        auto actionRect( actionGeometry( iter.key() ) );
        if( !event->rect().intersects( actionRect ) ) continue;
        actionRect.adjust( 2*margin, margin+1, -2*margin-1, -margin );
        painter.setBrush( iter.value() );
        painter.setRenderHints(QPainter::Antialiasing );
        painter.drawRoundedRect( actionRect, 4, 4 );
    }

    painter.end();

}

//_______________________________________________
void ColorMenu::_display()
{

    Debug::Throw( "ColorMenu::_display.\n" );

    // clear actions
    for( auto&& iter = actions_.begin(); iter != actions_.end(); ++iter )
    { delete iter.key(); }

    actions_.clear();

    for( const auto& color:colors_ )
    {

        // create action
        auto action = new QAction( this );
        actions_.insert( action, color );
        addAction( action );

    };

    return;

}

//_______________________________________________
void ColorMenu::_new()
{

    Debug::Throw( "ColorMenu::_new.\n" );
    QColor color( QColorDialog::getColor( Qt::white, this ) );
    if( color.isValid() )
    {
        add( color );
        lastColor_ = color;
        emit selected( color );
    }

}

//_______________________________________________
void ColorMenu::_default()
{

    Debug::Throw( "ColorMenu::_default.\n" );
    lastColor_ = QColor();
    emit selected( QColor() );

}

//_______________________________________________
void ColorMenu::_selected( QAction* action )
{
    Debug::Throw( "ColorMenu::_selected.\n" );
    auto iter = actions_.find( action );
    if( iter != actions_.end() )
    {
        lastColor_ = iter.value();
        emit selected( iter.value() );
    }
}
