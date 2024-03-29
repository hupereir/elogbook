#ifndef NewAttachmentDialog_h
#define NewAttachmentDialog_h

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
#include "Dialog.h"
#include "File.h"
#include "TextEditor.h"

#include <QCheckBox>
#include <QComboBox>

//* new attachment popup dialog
class NewAttachmentDialog: public Dialog
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit NewAttachmentDialog( QWidget* );

    //*@name accessors
    //@{

    //* filename
    File file() const;

    //* destination directory
    File destinationDirectory() const;

    //* url
    bool isUrl() const;

    //* Action
    Attachment::Command action() const;

    //* comments
    QString comments() const;

    //@}

    //*@name modifiers
    //@{

    //* filename
    void setFile( const File& );

    //* destination directory
    void setDestinationDirectory( const File& );

    //* attachment type
    void setIsUrl( bool );

    //* Action
    void setAction( Attachment::Command  );

    //* comments
    void setComments( const QString& comments );

    //@}

    private:

    //* called when url checkbox toggled
    void _urlChanged( bool );

    //* filename browsed line editor
    BrowsedLineEditor* fileEditor_ = nullptr;

    //* destination directory browsed line edti
    BrowsedLineEditor* destinationDirectoryEditor_ = nullptr;

    //* is url checkbox
    QCheckBox* urlCheckBox_ = nullptr;

    //* action combo box
    QComboBox* actionComboBox_ = nullptr;

    //* comments
    TextEditor* commentsEditor_ = nullptr;

};

#endif
