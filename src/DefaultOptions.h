#ifndef DefaultOptions_h
#define DefaultOptions_h

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

#include "LogEntryModel.h"
#include "AttachmentModel.h"
#include "Color.h"
#include "CppUtil.h"
#include "File.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryPrintSelectionWidget.h"
#include "Util.h"
#include "XmlOptions.h"

#include <QString>

//_____________________________________________________
//* Default options installer
void installDefaultOptions()
{
    // set options default values
    XmlOptions::get().setAutoDefault( true );

    XmlOptions::get().keep( QStringLiteral("OPEN_LINK_APPLICATIONS") );
    XmlOptions::get().keep( QStringLiteral("OPEN_ATTACHMENT_APPLICATIONS") );

    // COLOR options are special. Keep the full list
    XmlOptions::get().keep( QStringLiteral("COLOR") );
    XmlOptions::get().add( QStringLiteral("COLOR"), Option().set( Base::Color( "#aa0000" ) ) );
    XmlOptions::get().add( QStringLiteral("COLOR"), Option().set( Base::Color( "#FF9900" ) ) );
    XmlOptions::get().add( QStringLiteral("COLOR"), Option().set( Base::Color( "#FF8C00" ) ) );
    XmlOptions::get().add( QStringLiteral("COLOR"), Option().set( Base::Color( "#009900" ) ) );
    XmlOptions::get().add( QStringLiteral("COLOR"), Option().set( Base::Color( "#3333FF" ) ) );
    XmlOptions::get().add( QStringLiteral("COLOR"), Option().set( Base::Color( "#993399" ) ) );

    // COLOR options are special. Keep the full list
    XmlOptions::get().keep( QStringLiteral("TEXT_COLOR") );
    XmlOptions::get().add( QStringLiteral("TEXT_COLOR"), Option().set( Base::Color( "#aa0000" ) ) );
    XmlOptions::get().add( QStringLiteral("TEXT_COLOR"), Option().set( Base::Color( "#FF9900" ) ) );
    XmlOptions::get().add( QStringLiteral("TEXT_COLOR"), Option().set( Base::Color( "#FF8C00" ) ) );
    XmlOptions::get().add( QStringLiteral("TEXT_COLOR"), Option().set( Base::Color( "#009900" ) ) );
    XmlOptions::get().add( QStringLiteral("TEXT_COLOR"), Option().set( Base::Color( "#3333FF" ) ) );
    XmlOptions::get().add( QStringLiteral("TEXT_COLOR"), Option().set( Base::Color( "#993399" ) ) );


    // window sizes
    XmlOptions::get().set<int>( QStringLiteral("ATTACHMENT_WINDOW_HEIGHT"),400 );
    XmlOptions::get().set<int>( QStringLiteral("ATTACHMENT_WINDOW_WIDTH"), 600 );
    XmlOptions::get().set<int>( QStringLiteral("ATTACHMENT_FRAME_HEIGHT"), 150 );

    XmlOptions::get().set<int>( QStringLiteral("EDITION_WINDOW_HEIGHT"), 750 );
    XmlOptions::get().set<int>( QStringLiteral("EDITION_WINDOW_WIDTH"), 700 );

    XmlOptions::get().set<int>( QStringLiteral("MAIN_WINDOW_HEIGHT"), 750 );
    XmlOptions::get().set<int>( QStringLiteral("MAIN_WINDOW_WIDTH"), 700 );
    XmlOptions::get().set<int>( QStringLiteral("LIST_ICON_SIZE"), 10 );
    XmlOptions::get().set<int>( QStringLiteral("ATTACHMENT_LIST_ICON_SIZE"), 22 );

    XmlOptions::get().set<bool>( QStringLiteral("USE_COMPRESSION"), true );
    XmlOptions::get().set<bool>( QStringLiteral("FILE_BACKUP"), false );
    XmlOptions::get().set<bool>( QStringLiteral("AUTO_BACKUP"), true );
    XmlOptions::get().set<bool>( QStringLiteral("AUTO_SAVE"), false );
    XmlOptions::get().set<int>( QStringLiteral("AUTO_SAVE_ITV"), 60 );
    XmlOptions::get().set<int>( QStringLiteral("BACKUP_ITV"), 30 );
    XmlOptions::get().set<bool>( QStringLiteral("CASE_SENSITIVE"), false );
    XmlOptions::get().set<int>( QStringLiteral("DB_SIZE"), 10 );
    XmlOptions::get().set<bool>( QStringLiteral("SEARCH_PANEL"), false );

    XmlOptions::get().set<bool>( QStringLiteral("MAIN_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("FORMAT_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("EDITION_TOOLBAR"), false );
    XmlOptions::get().set<bool>( QStringLiteral("EXTRA_TOOLBAR"), false );
    XmlOptions::get().set<bool>( QStringLiteral("NAVIGATION_TOOLBAR"), false );
    XmlOptions::get().set<bool>( QStringLiteral("MULTIPLE_VIEW_TOOLBAR"), true );

    XmlOptions::get().set<bool>( QStringLiteral("LOCK_TOOLBAR"), false );
    XmlOptions::get().set<bool>( QStringLiteral("ENTRY_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("KEYWORD_TOOLBAR"), true );
    XmlOptions::get().set<bool>( QStringLiteral("SHOW_SEARCHPANEL"), true );

    XmlOptions::get().set<int>( QStringLiteral("MAX_RECENT_ENTRIES"), 30 );
    XmlOptions::get().set<bool>( QStringLiteral("SHOW_KEYWORD"), false );
    XmlOptions::get().set<bool>( QStringLiteral("USE_TREE"), true );

    XmlOptions::get().set<int>( QStringLiteral("LOGENTRY_PRINT_OPTION_MASK"), LogEntry::All );
    XmlOptions::get().set<int>( QStringLiteral("LOGENTRY_PRINT_SELECTION"), Base::toIntegralType(LogEntryPrintSelectionWidget::Mode::AllEntries) );
    XmlOptions::get().set<int>( QStringLiteral("LOGBOOK_PRINT_OPTION_MASK"), Logbook::All );

    XmlOptions::get().set<bool>( QStringLiteral("AUTO_INSERT_LINK"), true );

    // masks
    XmlOptions::get().set<int>( QStringLiteral("ENTRY_LIST_MASK"),
        (1<< LogEntryModel::Color)|
        (1<< LogEntryModel::Title)|
        (1<< LogEntryModel::Modification) );

    XmlOptions::get().set<int>( QStringLiteral("ENTRY_LIST_SORT_COLUMN"), LogEntryModel::Modification );
    XmlOptions::get().set<int>( QStringLiteral("ENTRY_LIST_SORT_Order"), Qt::DescendingOrder );

    XmlOptions::get().set<int>( QStringLiteral("ATTACHMENT_LIST_MASK"),
        (1<< AttachmentModel::FileName)|
        (1<< AttachmentModel::Size)|
        (1<< AttachmentModel::Modification) );

    // add run-time non recordable options
    QString user( Util::user( ) );
    QString host( Util::host() );
    XmlOptions::get().set( QStringLiteral("USER"), Option( user+"@"+host, Option::Flag::None ) );

    // resource file
    XmlOptions::get().set( QStringLiteral("OLD_RC_FILE"), Option(  File(".elogbookrc").addPath(Util::home()), Option::Flag::None ) );
    XmlOptions::get().set( QStringLiteral("RC_FILE"), Option(  File("elogbookrc").addPath(Util::config()), Option::Flag::None ) );

    XmlOptions::get().setAutoDefault( false );

};

#endif
