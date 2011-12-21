#ifndef LogEntryPrintHelper_h
#define LogEntryPrintHelper_h

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

#include "Counter.h"
#include "Debug.h"
#include "BasePrintHelper.h"
#include "LogEntry.h"

#include <QtCore/QObject>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>

//! printing utility
class LogEntryPrintHelper: public BasePrintHelper, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    LogEntryPrintHelper( QObject* parent = 0 ):
        BasePrintHelper( parent ),
        Counter( "LogEntryPrintHelper" ),
        mask_( LogEntry::ENTRY_ALL ),
        entry_( 0 )
    { Debug::Throw( "LogEntryPrintHelper::LogEntryPrintHelper.\n" ); };

    //! destructor
    virtual ~LogEntryPrintHelper( void )
    {}

    //! entry
    void setEntry( LogEntry* entry )
    { entry_ = entry; }

    //! mask
    void setMask( unsigned int value )
    { mask_ = value; }

    // print entry
    void printEntry( QPrinter*, QPainter*, QPointF& );

    public slots:

    //! print
    void print( QPrinter* );

    protected:

    //! print header
    void _printHeader( QPrinter*, QPainter*, QPointF& );

    //! print body
    void _printBody( QPrinter*, QPainter*, QPointF& );

    //! print attachments
    void _printAttachments( QPrinter*, QPainter*, QPointF& );

    private:

    //! mask
    unsigned int mask_;

    //! log entry
    LogEntry* entry_;

};

#endif
