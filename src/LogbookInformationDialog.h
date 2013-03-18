#ifndef LogbookInformationDialog_h
#define LogbookInformationDialog_h
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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "AnimatedLineEditor.h"
#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "File.h"
#include "TextEditor.h"

#include <QCheckBox>

class Logbook;

class LogbookInformationDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //! constructor
    LogbookInformationDialog( QWidget* parent, Logbook* logbook  );

    //! title
    QString title( void ) const
    { return title_->text(); }

    //! author
    QString author( void ) const
    { return author_->text(); }

    //! attachment directory
    File attachmentDirectory( void ) const
    { return attachmentDirectory_->editor().text(); }

    //! read only
    bool readOnly( void ) const
    { return readOnlyCheckBox_->isChecked(); }

    //! comments
    QString comments( void ) const
    { return comments_->toPlainText(); }

    private:

    //! title line edit
    AnimatedLineEditor* title_;

    //! author
    AnimatedLineEditor* author_;

    //! attachment directory
    BrowsedLineEditor* attachmentDirectory_;

    //! read only checkbox
    QCheckBox* readOnlyCheckBox_;

    //! comments
    TextEditor* comments_;

};

#endif
