#ifndef IconNames_h
#define IconNames_h

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

#include "BaseIconNames.h"
#include <QString>

//* namespace for icon static name wrappers
namespace IconNames
{

    static const QString Attach = QStringLiteral("mail-attachment");
    static const QString Close = QStringLiteral("document-close");
    static const QString Home = QStringLiteral("go-home");
    static const QString PreviousEntry = QStringLiteral("go-previous");
    static const QString NextEntry = QStringLiteral("go-next");
    static const QString New = QStringLiteral("document-new");
    static const QString Save = QStringLiteral("document-save");
    static const QString SaveAs = QStringLiteral("document-save-as");
    static const QString SpellCheck = QStringLiteral("tools-check-spelling");
    static const QString ViewLeftRight = QStringLiteral("view-split-left-right");
    static const QString ViewTopBottom = QStringLiteral("view-split-top-bottom");
    static const QString ViewRemove = QStringLiteral("view-close");
    static const QString ViewClone = QStringLiteral("view-fullscreen");
    static const QString Tree = QStringLiteral("view-list-tree");
    static const QString ConfigureBackups = QStringLiteral("configure-backups");
    static const QString InsertSymbolicLink = QStringLiteral("insert-link");

    static const QString PreferencesSpellCheck = QStringLiteral("tools-check-spelling");
    static const QString PreferencesBackup = QStringLiteral("document-save");
    static const QString Merge = QStringLiteral("merge");

    static const QString Move = QStringLiteral("go-jump");
    static const QString Link = QStringLiteral("edit-link");

};

#endif
