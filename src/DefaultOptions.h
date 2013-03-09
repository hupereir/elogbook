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

#include "LogEntryModel.h"
#include "AttachmentModel.h"
#include "XmlOptions.h"
#include "Color.h"
#include "Config.h"
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
    XmlOptions::get().add( "COLOR", Option().set<BASE::Color>( QColor( "#aa0000" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<BASE::Color>( QColor( "#FF9900" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<BASE::Color>( QColor( "#FF8C00" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<BASE::Color>( QColor( "#009900" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<BASE::Color>( QColor( "#3333FF" ) ) );
    XmlOptions::get().add( "COLOR", Option().set<BASE::Color>( QColor( "#993399" ) ) );

    // COLOR options are special. Keep the full list
    XmlOptions::get().keep( "TEXT_COLOR" );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<BASE::Color>( QColor( "#aa0000" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<BASE::Color>( QColor( "#FF9900" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<BASE::Color>( QColor( "#FF8C00" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<BASE::Color>( QColor( "#009900" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<BASE::Color>( QColor( "#3333FF" ) ) );
    XmlOptions::get().add( "TEXT_COLOR", Option().set<BASE::Color>( QColor( "#993399" ) ) );

    // icon
    XmlOptions::get().set( "ICON_PIXMAP", Option( ":/icon.png" , "application icon"  ) );

    // normal (overwritten) options

    // window sizes
    XmlOptions::get().set( "ATTACHMENT_WINDOW_HEIGHT", Option( "400" , "requested AttachmentWindow height [pixels]"  ) );
    XmlOptions::get().set( "ATTACHMENT_WINDOW_WIDTH", Option( "600" , "requested AttachmentWindow width [pixels]"  ) );
    XmlOptions::get().set( "ATTACHMENT_FRAME_HEIGHT", Option( "150" , "requested height of attachment list in editor [pixels]"  ) );

    XmlOptions::get().set( "EDITION_WINDOW_HEIGHT", Option( "750" , "requested EditionWindow height [pixels]"  ) );
    XmlOptions::get().set( "EDITION_WINDOW_WIDTH", Option( "700" , "requested EditionWindow width [pixels]"  ) );

    XmlOptions::get().set( "MAIN_WINDOW_HEIGHT", Option( "750" , "requested MainWindow height [pixels]"  ) );
    XmlOptions::get().set( "MAIN_WINDOW_WIDTH", Option( "700" , "requested MainWindow width [pixels]"  ) );
    XmlOptions::get().set( "LIST_ICON_SIZE", Option( "10", "default icon size in lists" ) );
    XmlOptions::get().set( "ATTACHMENT_LIST_ICON_SIZE", Option( "22", "default icon size in lists" ) );

    XmlOptions::get().set( "FILE_BACKUP", "0" );
    XmlOptions::get().set( "AUTO_BACKUP", Option( "1" , "1 to make a backup of logbook file prior to any writting"  ) );
    XmlOptions::get().set( "AUTO_SAVE", Option( "0" , "1 to save logbook automaticaly every given interval"  ) );
    XmlOptions::get().set( "AUTO_SAVE_ITV", Option( "60" , "interval between two consecutive automatic save [seconds]"  ) );
    XmlOptions::get().set( "BACKUP_ITV", Option( "30" , "interval between two consecutive auto backup [days]"  ) );
    XmlOptions::get().set( "CASE_SENSITIVE", Option( "0" , "1 to distinguish upper and lower case when sorting/selecting text"  ) );
    XmlOptions::get().set( "DB_SIZE", Option( "10" , "max number of files stored in the open previous menu"  ) );

    XmlOptions::get().set( "SIDE_EDITOR_TOOLBAR", Option( "1" , "if true, editor toolbar is on the left instead of top"  ) );

    XmlOptions::get().set( "SEARCH_PANEL", Option( "0" , "toolbar visibility" ) );
    XmlOptions::get().set( "SEARCH_PANEL_LOCATION", Option( "bottom" , "toolbar location" ) );

    XmlOptions::get().set( "SEARCH_PANEL_ICON_SIZE", "16" , "text label in tool buttons" );
    XmlOptions::get().set( "SEARCH_PANEL_TEXT_POSITION", "0" , "text label in tool buttons" );

    XmlOptions::get().set( "MAIN_TOOLBAR", Option( "1" , "toolbar visibility" ) );
    XmlOptions::get().set( "FORMAT_TOOLBAR", Option( "1" , "toolbar visibility" ) );
    XmlOptions::get().set( "EDITION_TOOLBAR", Option( "0" , "toolbar visibility" ) );
    XmlOptions::get().set( "EXTRA_TOOLBAR", Option( "0" , "toolbar visibility" ) );
    XmlOptions::get().set( "NAVIGATION_TOOLBAR", Option( "0" , "toolbar visibility" ) );
    XmlOptions::get().set( "MULTIPLE_VIEW_TOOLBAR", Option( "1" , "toolbar visibility" ) );

    XmlOptions::get().set( "LOCK_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
    XmlOptions::get().set( "MAIN_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
    XmlOptions::get().set( "FORMAT_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
    XmlOptions::get().set( "EDITION_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
    XmlOptions::get().set( "EXTRA_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
    XmlOptions::get().set( "NAVIGATION_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );
    XmlOptions::get().set( "MULTIPLE_VIEW_TOOLBAR_LOCATION", Option( "top" , "toolbar location" ) );

    XmlOptions::get().set( "LOCK_TOOLBAR", Option( "0" , "keywords toolbar visibility" ) );
    XmlOptions::get().set( "ENTRY_TOOLBAR", Option( "1" , "entries toolbar visibility" ) );
    XmlOptions::get().set( "KEYWORD_TOOLBAR", Option( "1" , "entries toolbar visibility" ) );
    XmlOptions::get().set( "SHOW_SEARCHPANEL", Option( "1" , "search panel visibility" ) );

    XmlOptions::get().set( "MAX_RECENT_ENTRIES", Option( "30", "maximum number of recent entries stored in logbook" ) );


    XmlOptions::get().set( "SHOW_KEYWORD", "0" );
    XmlOptions::get().set( "USE_TREE", "1" );

    XmlOptions::get().set( "LOGENTRY_PRINT_OPTION_MASK", QString().setNum( LogEntry::ENTRY_ALL ) );
    XmlOptions::get().set( "LOGENTRY_PRINT_SELECTION", QString().setNum( LogEntryPrintSelectionWidget::ALL_ENTRIES ) );
    XmlOptions::get().set( "LOGBOOK_PRINT_OPTION_MASK", QString().setNum( Logbook::LOGBOOK_ALL ) );

    // masks
    XmlOptions::get().set<unsigned int>( "ENTRY_LIST_MASK",
        (1<< LogEntryModel::COLOR)|
        (1<< LogEntryModel::TITLE)|
        (1<< LogEntryModel::CREATION)|
        (1<< LogEntryModel::MODIFICATION)|
        (1<< LogEntryModel::AUTHOR) );

    XmlOptions::get().set<unsigned int>( "ATTACHMENT_LIST_MASK",
        (1<< AttachmentModel::FILE)|
        (1<< AttachmentModel::SIZE)|
        (1<< AttachmentModel::TYPE)|
        (1<< AttachmentModel::MODIFICATION) );

    // add run-time non recordable options
    QString user( Util::user( ) );
    QString host( Util::host() );
    XmlOptions::get().set( "USER", Option( user+"@"+host, Option::None ) );
    XmlOptions::get().set( "RC_FILE", Option(  File(".elogbookrc").addPath(Util::home()), Option::None ) );
    XmlOptions::get().set( "APP_NAME", Option(  "ELOGBOOK", Option::None ) );
    XmlOptions::get().setAutoDefault( false );

};

#endif
