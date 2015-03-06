#ifndef XmlDef_h
#define XmlDef_h

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

    static const QString Logbook( "Logbook" );
    static const QString Attachment( "Attachment" );
    static const QString RecentEntries( "RecentEntries" );
    static const QString Entry( "Entry" );
    static const QString Child( "Child" );
    static const QString Type( "type" );
    static const QString File( "file" );
    static const QString SourceFile( "orig" );
    static const QString ParentFile( "parent_file" );
    static const QString Valid( "valid" );
    static const QString IsLink( "is_link" );
    static const QString Directory( "directory" );
    static const QString Comments( "comments" );
    static const QString Creation( "Creation" );
    static const QString Modification( "Modification" );
    static const QString Backup( "Backup" );
    static const QString BackupMask( "Logbook_backup" );
    static const QString Title( "title" );
    static const QString Author( "author" );
    static const QString SortMethod( "sort_method" );
    static const QString SortOrder( "sort_order" );
    static const QString Entries( "entries" );
    static const QString Children( "children" );
    static const QString Text( "Text" );
    static const QString Keyword( "key" );
    static const QString KeywordValue( "value" );
    static const QString Color( "color" );
    static const QString ReadOnly( "ReadOnly" );

};

#endif
