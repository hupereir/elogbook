#ifndef LogbookHtmlHelper_h
#define LogbookHtmlHelper_h
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
#include "Logbook.h"
#include "LogEntryModel.h"
#include "LogEntry.h"

#include <QObject>
#include <QIODevice>
#include <QDomDocument>
#include <QDomElement>

//* printing utilityclass
class LogbookHtmlHelper: public QObject, public Counter
{

    Q_OBJECT

    public:

    //* constructor
    LogbookHtmlHelper( QObject* parent = nullptr ):
        QObject( parent ),
        Counter( "LogbookHtmlHelper" )
    { Debug::Throw( "LogbookHtmlHelper::LogbookHtmlHelper.\n" ); };

    //* logbook
    void setLogbook( Logbook* logbook )
    { logbook_ = logbook; }

    //* entries
    void setEntries( const LogEntryModel::List& entries )
    { entries_ = entries; }

    //* mask
    void setMask( Logbook::Mask value )
    { mask_ = value; }

    //* entry mask
    void setEntryMask( LogEntry::Mask value )
    { entryMask_ = value; }

    public Q_SLOTS:

    //* print
    void print( QIODevice* );

    protected:

    //* print header
    void _appendHeader( QDomDocument&, QDomElement& );

    //* print table
    void _appendTable( QDomDocument&, QDomElement& );

    //* print contents
    void _appendEntries( QDomDocument&, QDomElement& );

    private:

    //* mask
    Logbook::Mask mask_ = Logbook::All;

    //* entry mask
    LogEntry::Mask entryMask_ = LogEntry::All;

    //* logbook
    Logbook* logbook_ = nullptr;

    //* logbook entries
    LogEntryModel::List entries_;

};

#endif
