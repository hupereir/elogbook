#ifndef NewLogbookDialog_h
#define NewLogbookDialog_h

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

#include "AnimatedLineEditor.h"
#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "TextEditor.h"
#include "File.h"
#include "Debug.h"

#include <QString>

//! new attachment popup dialog
class NewLogbookDialog: public CustomDialog
{

    Q_OBJECT

    public:

    //! constructor
    NewLogbookDialog( QWidget* parent );

    //! destructor
    virtual ~NewLogbookDialog( void )
    { Debug::Throw( "NewLogbookDialog::~NewLogbookDialog\n" ); }

    //! title
    void setTitle( const QString& title )
    { title_->setText( title ); }

    //! title
    QString title( void ) const
    { return title_->text(); }

    //! author
    void setAuthor( const QString& author )
    { author_->setText( author ); }

    //! filename
    QString author( void ) const
    { return author_->text(); }

    //! attachment directory
    void setAttachmentDirectory( const File& file )
    { attachmentDirectory_->setFile( file ); }

    //! attachment directory
    File attachmentDirectory( void ) const
    { return File( attachmentDirectory_->editor().text() ).expand(); }

    //! comments
    void setComments( const QString& comments )
    {
        Debug::Throw( "NewLogbookDialog::setComments.\n" );
        comments_->setPlainText( comments );
    }

    //! comments
    QString comments( void ) const
    { return comments_->toPlainText(); }

    private:

    //! title line edit
    AnimatedLineEditor *title_;

    //! author line edit
    AnimatedLineEditor *author_;

    //! destination directory browsed line edti
    BrowsedLineEditor *attachmentDirectory_;

    //! comments
    TextEditor *comments_;

};

#endif
