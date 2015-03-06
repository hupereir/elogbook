#ifndef LogEntryPrintSelectionWidget_h
#define LogEntryPrintSelectionWidget_h

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

#include "OptionWidget.h"

#include <QMap>
#include <QWidget>
#include <QRadioButton>

class LogEntryPrintSelectionWidget: public QWidget, public OptionWidget
{

    Q_OBJECT

    public:

    //* constructor
    LogEntryPrintSelectionWidget( QWidget* = 0 );

    //* destructor
    virtual ~LogEntryPrintSelectionWidget( void )
    {}

    //* read
    virtual void read( const Options& );

    //* write
    virtual void write( Options& ) const;

    //* mask
    enum Mode
    {
        AllEntries,
        VisibleEntries,
        SelectedEntries
    };

    Mode mode( void ) const;

    Q_SIGNALS:

    //* modified
    void modified( void );

    //* emited when selection mode is changed
    void modeChanged( LogEntryPrintSelectionWidget::Mode );

    protected Q_SLOTS:

    //* update mode
    void _updateMode( void )
    {
        emit modeChanged( mode() );
        if( _connected() ) emit modified();
    }

    private:

    //* checkboxes
    using RadioButtonMap = QMap<Mode, QRadioButton* >;
    RadioButtonMap radioButtons_;

};
#endif
