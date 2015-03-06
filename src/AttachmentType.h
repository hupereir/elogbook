#ifndef AttachmentType_h
#define AttachmentType_h

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

#include "Counter.h"

#include <QMap>

//! Attached file types for file manipulations
class AttachmentType: public Counter
{

    public:

    //! unknown file type
    static AttachmentType Unknown;

    //! postscript file type
    static AttachmentType Postscript;

    //! image file type
    static AttachmentType Image;

    //! plain text file type
    static AttachmentType PlainText;

    //! html file type
    static AttachmentType Html;

    //! url (pseudo) file type
    static AttachmentType Url;

    /*!
    retrieves predefined attachment type from key
    returns atcUNKNOWN if key is not found
    */
    static AttachmentType get( const QString& key );

    //! equal to operator
    bool operator == (const AttachmentType& type ) const
    { return key_ == type.key_; }

    //! creator
    AttachmentType(
        const QString& key,
        const QString& name,
        const QString& icon,
        const QString& options = QString()  );

    //! retrieves attachment key
    const QString& key( void ) const
    { return key_; }

    //! retrieves attachment name
    const QString& name( void ) const
    { return name_; }

    //! icon
    const QString& icon( void ) const
    { return icon_; }

    //! retrieves command used to edit the attachment
    QString editCommand( void ) const;

    //! attachment types map
    using Map = QMap< QString, AttachmentType >;

    //! returns predefined attachment types
    static const Map& types( void );

    private:

    //! install static type map
    static Map _install( void );

    //! used to identify the file type
    QString key_;

    //! used to display the file type in readable format
    QString name_;

    //! icon
    QString icon_;

    //! option name for looking up command
    QString option_;

};

#endif
