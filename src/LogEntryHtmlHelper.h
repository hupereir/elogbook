#ifndef LogEntryHtmlHelper_h
#define LogEntryHtmlHelper_h

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
#include "LogEntry.h"

#include <QtCore/QObject>
#include <QtCore/QIODevice>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>

//! printing utility
class LogEntryHtmlHelper: public QObject, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    LogEntryHtmlHelper( QObject* parent = 0 ):
        QObject( parent ),
        Counter( "LogEntryHtmlHelper" ),
        mask_( LogEntry::ENTRY_ALL ),
        entry_( 0 )
    { Debug::Throw( "LogEntryHtmlHelper::LogEntryHtmlHelper.\n" ); };

    //! destructor
    virtual ~LogEntryHtmlHelper( void )
    {}

    //! entry
    void setEntry( LogEntry* entry )
    { entry_ = entry; }

    //! mask
    void setMask( unsigned int value )
    { mask_ = value; }

    //! print header
    void appendEntry( QDomDocument&, QDomElement& );

    public slots:

    //! print
    void print( QIODevice* );

    protected:

    //! print header
    void _appendHeader( QDomDocument&, QDomElement& );

    //! print body
    void _appendBody( QDomDocument&, QDomElement& );

    //! print attachments
    void _appendAttachments( QDomDocument&, QDomElement& );

    private:

    //! mask
    unsigned int mask_;

    //! log entry
    LogEntry* entry_;

};

#endif
