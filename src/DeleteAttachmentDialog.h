#ifndef DeleteAttachmentDialog_h
#define DeleteAttachmentDialog_h

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
#include "CustomDialog.h"

#include <QRadioButton>

//! delete attachment popup dialog
class DeleteAttachmentDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //! constructor
    DeleteAttachmentDialog( QWidget* parent, const Attachment& attachment );

    //! destructor
    virtual ~DeleteAttachmentDialog( void )
    {}

    //! action
    enum Action
    {
        DeleteFromDisk,
        DeleteFromLogbook
    };

    //! get action
    Action action( void ) const;

    private:

    //! open with radio button
    QRadioButton* fromDiskButton_;

    //! save as radio button
    QRadioButton* fromLogbookButton_;


};

#endif
