#ifndef LogbookThread_h
#define LogbookThread_h

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

#include "Counter.h"

#include <QtCore/QThread>

class Logbook;


//! independent thread used to automatically save file
class LogbookThread: public QThread, public Counter
{

    public:

    //! constructor
    LogbookThread( QObject* = 0 );

    //! set logbook
    void setLogbook( Logbook* logbook )
    { logbook_ = logbook; }

    protected:

    //! Check files validity. Post a ValidFileEvent when finished
    virtual void run( void );

    private:

    //! logbook
    Logbook* logbook_;

};
#endif
