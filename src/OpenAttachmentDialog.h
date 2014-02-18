#ifndef OpenAttachmentDialog_h
#define OpenAttachmentDialog_h

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

#include "Attachment.h"
#include "BrowsedLineEditor.h"
#include "CustomDialog.h"

#include <QRadioButton>

//! open attachment popup dialog
class OpenAttachmentDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //! constructor
    OpenAttachmentDialog( QWidget*, const Attachment& );

    //! destructor
    virtual ~OpenAttachmentDialog( void )
    {}

    //! get command
    QString command( void ) const;

    //! action
    enum Action
    {
        Open,
        SaveAs
    };

    //! get action
    Action action( void ) const;

    private:

    //! command browsed line editor
    BrowsedLineEditor *commandEditor_;

    //! open with radio button
    QRadioButton* openRadioButton_;

    //! save as radio button
    QRadioButton* saveRadioButton_;

};

#endif
