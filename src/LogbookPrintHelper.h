#ifndef LogbookPrintHelper_h
#define LogbookPrintHelper_h
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
#include "BasePrintHelper.h"
#include "Debug.h"
#include "Keyword.h"
#include "Logbook.h"
#include "LogEntryModel.h"
#include "LogEntry.h"
#include "LogEntryPrintSelectionWidget.h"

#include <QObject>
#include <QPainter>
#include <QPointer>
#include <QPrinter>
#include <QProgressDialog>

//* printing utilityclass
class LogbookPrintHelper: public BasePrintHelper, private Base::Counter<LogbookPrintHelper>
{

    Q_OBJECT

    public:

    //* constructor
    explicit LogbookPrintHelper( QObject* parent = nullptr ):
        BasePrintHelper( parent ),
        Counter( QStringLiteral("LogbookPrintHelper") )
    { Debug::Throw( QStringLiteral("LogbookPrintHelper::LogbookPrintHelper.\n") ); };

    //*@name accessors
    //@{

    //* abort
    bool isAborted() const
    { return aborted_; }

    //@}

    //*@name modifiers
    //@{

    //* logbook
    void setLogbook( Logbook* logbook )
    { logbook_ = logbook; }

    //* set entries
    void setEntries( const LogEntryModel::List& entries )
    { entries_ = entries; }

    //* abort
    void setAborted( bool value )
    { aborted_ = value; }

    //* abort
    void abort()
    { aborted_ = true; }

    //* current keyword
    void setCurrentKeyword( Keyword value )
    { currentKeyword_ = value; }

    //* mask
    void setMask( Logbook::Mask value )
    { mask_ = value; }

    //* entry mask
    void setEntryMask( LogEntry::Mask value )
    { entryMask_ = value; }

    //* print
    void print( QPrinter* ) override;

    //@}

    protected:

    //* update entries
    void _updateEntries();

    //* print header
    void _printHeader( QPrinter*, QPainter*, QPointF& );

    //* print table
    void _printTable( QPrinter*, QPainter*, QPointF& );

    //* print contents
    void _printEntries( QPrinter*, QPainter*, QPointF& );

    private:

    //* mask
    Logbook::Mask mask_ = Logbook::All;

    //* entry mask
    LogEntry::Mask entryMask_ = LogEntry::All;

    //* logbook
    Logbook* logbook_ = nullptr;

    //* logbook entries
    LogEntryModel::List entries_;

    //* current keyword, in tree mode
    Keyword currentKeyword_;

    //* progress dialog
    QPointer<QProgressDialog> progressDialog_;

    //* progress
    int progress_ = 0;

    //* true when aborted
    bool aborted_ = false;

};

#endif
