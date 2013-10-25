#ifndef LogbookPrintOptionWidget_h
#define LogbookPrintOptionWidget_h

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

#include "Logbook.h"
#include "OptionWidget.h"

#include <QMap>
#include <QWidget>
#include <QCheckBox>

class LogbookPrintOptionWidget: public QWidget, public OptionWidget
{

    Q_OBJECT

    public:

    //! constructor
    LogbookPrintOptionWidget( QWidget* = 0 );

    //! destructor
    virtual ~LogbookPrintOptionWidget( void )
    {}

    //! read
    virtual void read( void );

    //! write
    virtual void write( void ) const;

    //! mask
    Logbook::Mask mask( void ) const;

    Q_SIGNALS:

    //! mask changed
    void maskChanged( unsigned int );

    protected Q_SLOTS:

    //! update mask
    void _updateMask( void )
    { emit maskChanged( mask() ); }

    private:

    //! checkboxes
    typedef QMap<Logbook::MaskFlag, QCheckBox* > CheckBoxMap;
    CheckBoxMap checkBoxes_;

};
#endif
