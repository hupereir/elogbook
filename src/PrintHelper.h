#ifndef PrintHelper_h
#define PrintHelper_h

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

#include "TimeStamp.h"
#include "Debug.h"
#include "File.h"

#include <QtCore/QObject>
#include <QtGui/QPainter>
#include <QtGui/QPrinter>

class LogEntry;

//! printing utility
class PrintHelper: public QObject
{

    public:

    //! constructor
    PrintHelper( QObject* parent = 0 ):
        QObject( parent ),
        pageNumber_(0),
        now_( TimeStamp::now() )
    { Debug::Throw( "PrintHelper::PrintHelper.\n" ); };

    //! destructor
    virtual ~PrintHelper( void )
    {}

    //! setup pages
    void setupPage( QPrinter* );

    //! file
    void setFile( const File& file )
    { file_ = file; }

    //! set page number manually
    void setPageNumber( int value )
    { pageNumber_ = value; }

    protected:

    //! return pageRect
    const QRect& _pageRect( void ) const
    { return pageRect_; }

    //! page number
    int _pageNumber( void ) const
    { return pageNumber_; }

    //! increment page
    void _newPage( QPrinter*, QPainter* );

    private:

    //! rects
    QRect pageRect_;
    QRect headerRect_;
    QRect footerRect_;

    //! page
    int pageNumber_;

    //! time stamp
    TimeStamp now_;

    //! file
    File file_;

};

#endif
