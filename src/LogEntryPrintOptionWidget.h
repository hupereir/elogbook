#ifndef LogEntryPrintOptionWidget_h
#define LogEntryPrintOptionWidget_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "LogEntry.h"
#include "OptionWidget.h"

#include <QMap>
#include <QWidget>
#include <QCheckBox>

class LogEntryPrintOptionWidget: public QWidget, public OptionWidget
{

    Q_OBJECT

    public:

    //! constructor
    LogEntryPrintOptionWidget( QWidget* = 0 );

    //! destructor
    virtual ~LogEntryPrintOptionWidget( void )
    {}

    //! read
    virtual void read( const Options& );

    //! write
    virtual void write( Options& ) const;

    //! mask
    LogEntry::Mask mask( void ) const;

    Q_SIGNALS:

    //! modified
    void modified( void );

    //! mask changed
    void maskChanged( LogEntry::Mask );

    protected Q_SLOTS:

    //! update mask
    void _updateMask( void )
    {
        emit maskChanged( mask() );
        if( _connected() ) emit modified();
    }

    private:

    //! checkboxes
    typedef QMap<LogEntry::MaskFlag, QCheckBox* > CheckBoxMap;
    CheckBoxMap checkBoxes_;

};
#endif
