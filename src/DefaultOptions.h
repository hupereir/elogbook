#ifndef DefaultOptions_h
#define DefaultOptions_h

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

#include "LogEntryModel.h"
#include "AttachmentModel.h"
#include "XmlOptions.h"
#include "Color.h"
#include "File.h"
#include "Util.h"

#include "Logbook.h"
#include "LogEntry.h"
#include "LogEntryPrintSelectionWidget.h"

#include <QString>

//_____________________________________________________
//! Default options installer
void installDefaultOptions( void )
{
    // set options default values
    XmlOptions::get().setAutoDefault( true );

    XmlOptions::get().keep( "OPEN_LINK_APPLICATIONS" );

    // COLOR options are special. Keep the full list
    XmlOptions::get().keep( "COLOR" );
    XmlOptions::get().add( "COLOR", Option().set<Base::Color>( QColor( "#aa0000" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<Base::Color>( QColor( "#FF9900" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<Base::Color>( QColor( "#FF8C00" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<Base::Color>( QColor( "#009900" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<Base::Color>( QColor( "#3333FF" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<Base::Color>( QColor( "#993399" ) ) );

    // COLOR options are special. Keep the full list
    XmlOptions::get().keep( "TEXT_COLOR" );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<Base::Color>( QColor( "#aa0000" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<Base::Color>( QColor( "#FF9900" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<Base::Color>( QColor( "#FF8C00" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<Base::Color>( QColor( "#009900" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<Base::Color>( QColor( "#3333FF" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<Base::Color>( QColor( "#993399" ) ) );

    // normal (overwritten) options

    // window sizes
    XmlOptions::get().set<int>( "ATTACHMENT_WINDOW_HEIGHT",400 );
    XmlOptions::get().set<int>( "ATTACHMENT_WINDOW_WIDTH", 600 );
    XmlOptions::get().set<int>( "ATTACHMENT_FRAME_HEIGHT", 150 );

    XmlOptions::get().set<int>( "EDITION_WINDOW_HEIGHT", 750 );
    XmlOptions::get().set<int>( "EDITION_WINDOW_WIDTH", 700 );

    XmlOptions::get().set<int>( "MAIN_WINDOW_HEIGHT", 750 );
    XmlOptions::get().set<int>( "MAIN_WINDOW_WIDTH", 700 );
    XmlOptions::get().set<int>( "LIST_ICON_SIZE", 10 );
    XmlOptions::get().set<int>( "ATTACHMENT_LIST_ICON_SIZE", 22 );

    XmlOptions::get().set<bool>( "FILE_BACKUP", false );
    XmlOptions::get().set<bool>( "AUTO_BACKUP", true );
    XmlOptions::get().set<bool>( "AUTO_SAVE", false );
    XmlOptions::get().set<int>( "AUTO_SAVE_ITV", 60 );
    XmlOptions::get().set<int>( "BACKUP_ITV", 30 );
    XmlOptions::get().set<bool>( "CASE_SENSITIVE", false );
    XmlOptions::get().set<int>( "DB_SIZE", 10 );

    XmlOptions::get().set<bool>( "SIDE_EDITOR_TOOLBAR", true );

    XmlOptions::get().set<bool>( "SEARCH_PANEL", false );
    XmlOptions::get().set<int>( "SEARCH_PANEL_LOCATION", Qt::BottomToolBarArea );

    XmlOptions::get().set<int>( "SEARCH_PANEL_ICON_SIZE", 16 );
    XmlOptions::get().set<int>( "SEARCH_PANEL_TEXT_POSITION", 0 );

    XmlOptions::get().set<bool>( "MAIN_TOOLBAR", true );
    XmlOptions::get().set<bool>( "FORMAT_TOOLBAR", true );
    XmlOptions::get().set<bool>( "EDITION_TOOLBAR", false );
    XmlOptions::get().set<bool>( "EXTRA_TOOLBAR", false );
    XmlOptions::get().set<bool>( "NAVIGATION_TOOLBAR", false );
    XmlOptions::get().set<bool>( "MULTIPLE_VIEW_TOOLBAR", true );

    XmlOptions::get().set<bool>( "LOCK_TOOLBAR", false );
    XmlOptions::get().set<bool>( "ENTRY_TOOLBAR", true );
    XmlOptions::get().set<bool>( "KEYWORD_TOOLBAR", true );
    XmlOptions::get().set<bool>( "SHOW_SEARCHPANEL", true );

    XmlOptions::get().set<int>( "MAX_RECENT_ENTRIES", 30 );


    XmlOptions::get().set<bool>( "SHOW_KEYWORD", false );
    XmlOptions::get().set<bool>( "USE_TREE", true );

    XmlOptions::get().set<int>( "LOGENTRY_PRINT_OPTION_MASK", LogEntry::All );
    XmlOptions::get().set<int>( "LOGENTRY_PRINT_SELECTION", LogEntryPrintSelectionWidget::AllEntries );
    XmlOptions::get().set<int>( "LOGBOOK_PRINT_OPTION_MASK", Logbook::All );

    // masks
    XmlOptions::get().set<int>( "ENTRY_LIST_MASK",
        (1<< LogEntryModel::Color)|
        (1<< LogEntryModel::Title)|
        (1<< LogEntryModel::Creation)|
        (1<< LogEntryModel::Modification)|
        (1<< LogEntryModel::Author) );

    XmlOptions::get().set<int>( "ATTACHMENT_LIST_MASK",
        (1<< AttachmentModel::Filename)|
        (1<< AttachmentModel::Size)|
        (1<< AttachmentModel::Type)|
        (1<< AttachmentModel::Modification) );

    // add run-time non recordable options
    QString user( Util::user( ) );
    QString host( Util::host() );
    XmlOptions::get().set( "USER", Option( user+"@"+host, Option::None ) );
    XmlOptions::get().set( "RC_FILE", Option(  File(".elogbookrc").addPath(Util::home()), Option::None ) );
    XmlOptions::get().setAutoDefault( false );

};

#endif
