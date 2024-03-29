#ifndef LogEntryPrintHelper_h
#define LogEntryPrintHelper_h
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

#include "BasePrintHelper.h"
#include "Counter.h"
#include "Debug.h"
#include "LogEntry.h"

#include <QObject>
#include <QPainter>
#include <QPrinter>

//* printing utility
class LogEntryPrintHelper: public BasePrintHelper, private Base::Counter<LogEntryPrintHelper>
{

    Q_OBJECT

    public:

    //* constructor
    explicit LogEntryPrintHelper( QObject* parent = nullptr ):
        BasePrintHelper( parent ),
        Counter( QStringLiteral("LogEntryPrintHelper") )
    { Debug::Throw( QStringLiteral("LogEntryPrintHelper::LogEntryPrintHelper.\n") ); };

    //* entry
    void setEntry( LogEntry* entry )
    { entry_ = entry; }

    // print entry
    void printEntry( QPrinter*, QPainter*, QPointF& );

    //* mask
    void setMask( LogEntry::Mask value )
    { mask_ = value; }

    //* print
    void print( QPrinter* ) override;

    protected:

    //* print header
    void _printHeader( QPrinter*, QPainter*, QPointF& );

    //* print body
    void _printBody( QPrinter*, QPainter*, QPointF& );

    //* print attachments
    void _printAttachments( QPrinter*, QPainter*, QPointF& );

    private:

    //* mask
    LogEntry::Mask mask_ = LogEntry::All;

    //* log entry
    LogEntry* entry_ = nullptr;

};

#endif
