#ifndef LogEntryHtmlHelper_h
#define LogEntryHtmlHelper_h
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

#include "Counter.h"
#include "Debug.h"
#include "LogEntry.h"

#include <QObject>
#include <QIODevice>
#include <QDomDocument>
#include <QDomElement>

//* printing utility
class LogEntryHtmlHelper: public QObject, private Base::Counter<LogEntryHtmlHelper>
{

    Q_OBJECT

    public:

    //* constructor
    explicit LogEntryHtmlHelper( QObject* parent = nullptr ):
        QObject( parent ),
        Counter( "LogEntryHtmlHelper" )
    { Debug::Throw( "LogEntryHtmlHelper::LogEntryHtmlHelper.\n" ); };

    //* entry
    void setEntry( LogEntry* entry )
    { entry_ = entry; }

    //* mask
    void setMask( LogEntry::Mask value )
    { mask_ = value; }

    //* print header
    void appendEntry( QDomDocument&, QDomElement& );

    public Q_SLOTS:

    //* print
    void print( QIODevice* );

    protected:

    //* print header
    void _appendHeader( QDomDocument&, QDomElement& );

    //* print body
    void _appendBody( QDomDocument&, QDomElement& );

    //* print attachments
    void _appendAttachments( QDomDocument&, QDomElement& );

    private:

    //* mask
    LogEntry::Mask mask_ = LogEntry::All;

    //* log entry
    LogEntry* entry_ = nullptr;

};

#endif
