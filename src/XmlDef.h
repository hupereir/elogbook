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

//* Some Xml definitions
namespace Xml
{

    static const QString Logbook( QStringLiteral("Logbook") );
    static const QString Attachment( QStringLiteral("Attachment") );
    static const QString RecentEntries( QStringLiteral("RecentEntries") );
    static const QString Entry( QStringLiteral("Entry") );
    static const QString Child( QStringLiteral("Child") );
    static const QString Type( QStringLiteral("type") );
    static const QString File( QStringLiteral("file") );
    static const QString SourceFile( QStringLiteral("orig") );
    static const QString ParentFile( QStringLiteral("parent_file") );
    static const QString Valid( QStringLiteral("valid") );
    static const QString IsLink( QStringLiteral("is_link") );
    static const QString IsUrl( QStringLiteral("is_url") );
    static const QString Directory( QStringLiteral("directory") );
    static const QString Comments( QStringLiteral("comments") );
    static const QString Creation( QStringLiteral("Creation") );
    static const QString Modification( QStringLiteral("Modification") );
    static const QString Backup( QStringLiteral("Backup") );
    static const QString BackupMask( QStringLiteral("Logbook_backup") );
    static const QString Title( QStringLiteral("title") );
    static const QString Author( QStringLiteral("author") );
    static const QString SortMethod( QStringLiteral("sort_method") );
    static const QString SortOrder( QStringLiteral("sort_order") );
    static const QString Entries( QStringLiteral("entries") );
    static const QString Children( QStringLiteral("children") );
    static const QString Text( QStringLiteral("Text") );
    static const QString Keyword( QStringLiteral("key") );
    static const QString KeywordValue( QStringLiteral("value") );
    static const QString Color( QStringLiteral("color") );
    static const QString ReadOnly( QStringLiteral("ReadOnly") );

};

#endif
