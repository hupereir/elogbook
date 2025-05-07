#ifndef LogbookInformationDialog_h
#define LogbookInformationDialog_h

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

#include "BrowsedLineEditor.h"
#include "Dialog.h"
#include "File.h"
#include "LineEditor.h"
#include "TextEditor.h"

#include <QCheckBox>

class Logbook;

class LogbookInformationDialog: public Dialog
{

    Q_OBJECT

    public:

    //* constructor
    explicit LogbookInformationDialog( QWidget* parent, Logbook* logbook  );

    //* title
    QString title() const
    { return title_->text(); }

    //* author
    QString author() const
    { return author_->text(); }

    //* attachment directory
    File attachmentDirectory() const
    { return File( attachmentDirectory_->text() ); }

    //* read only
    bool readOnly() const
    { return readOnlyCheckBox_->isChecked(); }

    //* comments
    QString comments() const
    { return comments_->toPlainText(); }

    private:

    //* title line edit
    LineEditor* title_ = nullptr;

    //* author
    LineEditor* author_ = nullptr;

    //* attachment directory
    BrowsedLineEditor* attachmentDirectory_ = nullptr;

    //* read only checkbox
    QCheckBox* readOnlyCheckBox_ = nullptr;

    //* comments
    TextEditor* comments_ = nullptr;

};

#endif
