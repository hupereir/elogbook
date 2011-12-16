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

#include <QtCore/QObject>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>

class LogEntry;

//! printing utility
class LogEntryPrintHelper: public QObject, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    LogEntryPrintHelper( QObject* parent, LogEntry* entry ):
        QObject( parent ),
        Counter( "LogEntryPrintHelper" ),
        entry_( entry )
    { Debug::Throw( "LogEntryPrintHelper::LogEntryPrintHelper.\n" ); };

    //! destructor
    virtual ~LogEntryPrintHelper( void )
    {}

    public slots:

    //! print
    void print( QPrinter* );

    protected:

    //! print header
    void _printHeader( QPrinter*, QPointF = QPointF() ) const;

    //! print body
    void _printBody( QPrinter*, QPointF = QPointF() ) const;

    //! print attachments
    void _printAttachments( QPrinter*, QPointF = QPointF() ) const;

    private:

    //! log entry
    LogEntry* entry_;

};

#endif
