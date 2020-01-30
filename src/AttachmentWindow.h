#ifndef AttachmentWindow_h
#define AttachmentWindow_h

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
#include "Counter.h"
#include "AttachmentFrame.h"
#include "Debug.h"

#include <QEvent>

class Attachment;

//* popup window to list/edit all attachments independantly from entries
class AttachmentWindow: public CustomDialog
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    explicit AttachmentWindow( QWidget* = nullptr );

    //* display widget
    void show();

    //* retrieve associated List
    AttachmentFrame& frame()
    { return *frame_; }

    //* uniconify window
    QAction& uniconifyAction()
    { return *uniconifyAction_; }

    //* uniconify
    void uniconify();

    Q_SIGNALS:

    void entrySelected( LogEntry* );

    private:

    //* display entry associated to selected attachment when selection changes
    void _displayEntry( Attachment& );

    private:

    //* associated attachment list
    AttachmentFrame* frame_ = nullptr;

    //* uniconify action
    QAction* uniconifyAction_ = nullptr;

};

#endif
