#ifndef LogbookPrintHelper_h
#define LogbookPrintHelper_h

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
#include "BasePrintHelper.h"
#include "Debug.h"
#include "Logbook.h"
#include "LogEntryModel.h"
#include "LogEntry.h"
#include "LogEntryPrintSelectionWidget.h"

#include <QtCore/QObject>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>
#include <QtGui/QProgressDialog>

//! printing utilityclass
class LogbookPrintHelper: public BasePrintHelper, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    LogbookPrintHelper( QObject* parent = 0 ):
        BasePrintHelper( parent ),
        Counter( "LogbookPrintHelper" ),
        mask_( Logbook::LOGBOOK_ALL ),
        entryMask_( LogEntry::ENTRY_ALL ),
        selectionMode_( LogEntryPrintSelectionWidget::VISIBLE_ENTRIES ),
        logbook_( 0 ),
        progressDialog_( 0 ),
        progress_( 0 ),
        aborted_( false )
    { Debug::Throw( "LogbookPrintHelper::LogbookPrintHelper.\n" ); };

    //! destructor
    virtual ~LogbookPrintHelper( void )
    {}

    //! logbook
    void setLogbook( Logbook* logbook )
    { logbook_ = logbook; }

    //! entries
    void setEntries(
        const LogEntryModel::List& allEntries,
        const LogEntryModel::List& visibleEntries,
        const LogEntryModel::List& selectedEntries )
    {
        allEntries_ = allEntries;
        visibleEntries_ = visibleEntries;
        selectedEntries_ = selectedEntries;
        _updateEntries();
    }

    //! abort
    void setAborted( bool value )
    { aborted_ = value; }

    //! abort
    void abort( void )
    { aborted_ = true; }

    //! abort
    bool isAborted( void ) const
    { return aborted_; }

    public slots:


    //! mask
    void setMask( unsigned int value )
    { mask_ = value; }

    //! entry mask
    void setEntryMask( unsigned int value )
    { entryMask_ = value; }

    //! set entry selection mode
    void setSelectionMode( LogEntryPrintSelectionWidget::Mode mode )
    {
        selectionMode_ = mode;
        _updateEntries();
    }

    //! print
    void print( QPrinter* );

    protected:

    //! update entries
    void _updateEntries( void );

    //! print header
    void _printHeader( QPrinter*, QPainter*, QPointF& );

    //! print table
    void _printTable( QPrinter*, QPainter*, QPointF& );

    //! print contents
    void _printEntries( QPrinter*, QPainter*, QPointF& );

    private:

    //! mask
    unsigned int mask_;

    //! entry mask
    unsigned int entryMask_;

    //! selection mode
    LogEntryPrintSelectionWidget::Mode selectionMode_;

    //! logbook
    Logbook* logbook_;

    //!@name logbook entries
    //@{

    LogEntryModel::List allEntries_;
    LogEntryModel::List visibleEntries_;
    LogEntryModel::List selectedEntries_;
    LogEntryModel::List entries_;

    //@}

    //! progress dialog
    QProgressDialog* progressDialog_;

    //! progress
    int progress_;

    //! true when aborted
    bool aborted_;

};

#endif
