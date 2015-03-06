#ifndef OpenLinkDialog_h
#define OpenLinkDialog_h

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

#include "CustomDialog.h"

class CustomComboBox;

class OpenLinkDialog: public CustomDialog
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    OpenLinkDialog( QWidget*, const QString& );

    //* destructor
    virtual ~OpenLinkDialog( void ) = default;

    //* combo box
    CustomComboBox& actionComboBox( void ) const
    { return *actionComboBox_; }

    protected Q_SLOTS:

    //* select action from fileSelection dialog.
    void _browse( void );

    private:

    CustomComboBox* actionComboBox_ = nullptr;

};

#endif
