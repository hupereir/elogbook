#ifndef EditAttachmentDialog_h
#define EditAttachmentDialog_h

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
#include "TextEditor.h"
#include "CustomDialog.h"

//* edit attachment popup dialog
class EditAttachmentDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //* constructor
    explicit EditAttachmentDialog( QWidget*, const Attachment& );

    //* get comments
    QString comments( void ) const;

    private:

    //* comments editor
    TextEditor *commentsEditor_ = nullptr;

};

#endif
