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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
****************************************************************************/

#include "PrintHelper.h"

#include <QtCore/QTextStream>
#include <QtGui/QTextDocument>

//__________________________________________________________________________
void PrintHelper::setupPage( QPrinter* printer )
{

    Debug::Throw( "PrintHelper::setupPage.\n" );

    pageNumber_ = 0;
    now_ = TimeStamp::now();

    // get font
    const QFont font( QTextDocument().defaultFont() );
    const QFontMetrics metrics( font, printer );
    const int leading( metrics.leading() );
    const int headerHeight( ( metrics.height() + leading ) * 1.5 );
    const int footerHeight( ( metrics.height() + leading ) * 1.5 );

    headerRect_ = QRect( 0, 0, printer->pageRect().width(), headerHeight );
    footerRect_ = QRect( 0, printer->pageRect().height() - footerHeight, printer->pageRect().width(), footerHeight );
    pageRect_ = QRect( headerRect_.bottomLeft(), footerRect_.topRight() );
    pageRect_.adjust( 0, headerRect_.height()/2, 0, -footerRect_.height()/2 );

    return;

}

//__________________________________________________________________________
void PrintHelper::_newPage( QPrinter* printer, QPainter* painter )
{

    Debug::Throw( "PrintHelper::_newPage.\n" );

    // increment printer page
    if( pageNumber_ ) printer->newPage();

    // increment page number
    pageNumber_++;

    // setup painter
    painter->save();
    painter->setPen( QColor( "#888888" ) );

    // header
    painter->drawLine( headerRect_.bottomLeft()+QPoint(0,1), headerRect_.bottomRight()+QPoint(0,1) );
    painter->drawText( headerRect_, Qt::AlignVCenter|Qt::AlignLeft, TimeStamp::now().toString( TimeStamp::DATE ) );
    painter->drawText( headerRect_, Qt::AlignVCenter|Qt::AlignRight, QString().setNum( pageNumber_ ) );

    if( !file_.isEmpty() )
    { painter->drawText( headerRect_, Qt::AlignCenter, file_.localName() ); }

    // footer
    painter->drawLine( footerRect_.topLeft()-QPoint(0,1), footerRect_.topRight()-QPoint(0,1) );
    painter->drawText( footerRect_, Qt::AlignVCenter|Qt::AlignRight, QString().setNum( pageNumber_ ) );
    if( !file_.isEmpty() )
    { painter->drawText( footerRect_, Qt::AlignVCenter|Qt::AlignLeft, file_ ); }

    // restore
    painter->restore();

}
