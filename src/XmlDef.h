#ifndef XmlDef_h
#define XmlDef_h

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

#include <QString>

//! Some Xml definitions
namespace Xml
{

    //! logbook tag
    static const QString Logbook( "Logbook" );

    //! attachment tag
    static const QString Attachment( "Attachment" );

    //! recent entries
    static const QString RecentEntries( "RecentEntries" );

    //! entry tag
    static const QString Entry( "Entry" );

    //! logbook child tag
    static const QString Child( "Child" );

    //! attachment type
    static const QString Type( "type" );

    //! attachment destination file
    static const QString File( "file" );

    //! attachment source file
    static const QString SourceFile( "orig" );

    //! parent file
    static const QString ParentFile( "parent_file" );

    //! attachment validity
    static const QString Valid( "valid" );

    //! attachement is link
    static const QString IsLink( "is_link" );

    //! directory
    static const QString Directory( "directory" );

    //! comments
    static const QString Comments( "comments" );

    //! creation time
    static const QString Creation( "Creation" );

    //! modification time
    static const QString Modification( "Modification" );

    //! backup time
    static const QString Backup( "Backup" );

    //! backup file
    static const QString BackupMask( "Logbook_backup" );

    //! title
    static const QString Title( "title" );

    //! logbook author
    static const QString Author( "author" );

    //! logbook sort method
    static const QString SortMethod( "sort_method" );

    //! logbook sort order
    static const QString SortOrder( "sort_order" );

    //! logbook number of entries
    static const QString Entries( "entries" );

    //! logbook number of children
    static const QString Children( "children" );

    //! entry text
    static const QString Text( "Text" );

    //! entry key
    static const QString Keyword( "key" );

    //! entry color
    static const QString Color( "color" );

    //! true if logbook is read only
    static const QString ReadOnly( "ReadOnly" );

};

#endif
