#ifndef LogEntryPrintOptionWidget_h
#define LogEntryPrintOptionWidget_h

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

#include <QHash>
#include <QWidget>
#include <QCheckBox>

class LogEntryPrintOptionWidget: public QWidget, public OptionWidget
{

    Q_OBJECT

    public:

    //* constructor
    explicit LogEntryPrintOptionWidget( QWidget* = nullptr );

    //* read
    void read( const Options& ) override;

    //* write
    void write( Options& ) const override;

    //* mask
    LogEntry::Mask mask() const;

    Q_SIGNALS:

    //* modified
    void modified();

    //* mask changed
    void maskChanged( LogEntry::Mask );

    private:

    //* update mask
    void _updateMask()
    {
        emit maskChanged( mask() );
        if( _connected() ) emit modified();
    }

    //* checkboxes
    using CheckBoxMap = QHash<LogEntry::MaskFlag, QCheckBox* >;
    CheckBoxMap checkBoxes_;

};
#endif
