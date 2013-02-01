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
* Place, Suite 330, Boston, MA 02111-1307 USA
*
*
*******************************************************************************/

#include "LogbookThread.h"
#include "Logbook.h"

//_____________________________________________________________________________________________
LogbookThread::LogbookThread( QObject* parent ):
    QThread( parent ),
    Counter( "LogbookThread" ),
    logbook_( 0 )
{}

//_____________________________________________________________________________________________
void LogbookThread::run( void )
{

    // check logbook
    if( logbook_ ) logbook_->read();



}