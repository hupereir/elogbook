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

/*!
  \file DefaultOptions.h
  \brief Default options
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "LogEntryModel.h"
#include "AttachmentModel.h"
#include "XmlOptions.h"
#include "Config.h"
#include "Util.h"

using namespace std;

//_____________________________________________________
//! Default options installer
void installDefaultOptions( void )
{
  // set options default values
  XmlOptions::get().setAutoDefault( true );
  
  // COLOR options are special. Keep the full list
  XmlOptions::get().keep( "COLOR" );
  XmlOptions::get().add( "COLOR", Option( "#aa0000" , "entry display color"  ) );
  XmlOptions::get().add( "COLOR", Option( "#FF9900" , "entry display color"  ) );
  XmlOptions::get().add( "COLOR", Option( "#FF8C00" , "entry display color"  ) );
  XmlOptions::get().add( "COLOR", Option( "#009900" , "entry display color"  ) );
  XmlOptions::get().add( "COLOR", Option( "#3333FF" , "entry display color"  ) );
  XmlOptions::get().add( "COLOR", Option( "#993399" , "entry display color"  ) );
 
    // COLOR options are special. Keep the full list
  XmlOptions::get().keep( "TEXT_COLOR" );
  XmlOptions::get().add( "TEXT_COLOR", Option( "#aa0000" , "entry display color"  ) );
  XmlOptions::get().add( "TEXT_COLOR", Option( "#FF9900" , "entry display color"  ) );
  XmlOptions::get().add( "TEXT_COLOR", Option( "#FF8C00" , "entry display color"  ) );
  XmlOptions::get().add( "TEXT_COLOR", Option( "#009900" , "entry display color"  ) );
  XmlOptions::get().add( "TEXT_COLOR", Option( "#3333FF" , "entry display color"  ) );
  XmlOptions::get().add( "TEXT_COLOR", Option( "#993399" , "entry display color"  ) );
  
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
  
  XmlOptions::get().set( "AUTO_BACKUP", Option( "1" , "1 to make a backup of logbook file prior to any writting"  ) );
  XmlOptions::get().set( "AUTO_SAVE", Option( "0" , "1 to save logbook automaticaly every given interval"  ) );
  XmlOptions::get().set( "AUTO_SAVE_ITV", Option( "60" , "interval between two consecutive automatic save [seconds]"  ) );
  XmlOptions::get().set( "BACKUP_ITV", Option( "30" , "interval between two consecutive auto backup [days]"  ) );
  XmlOptions::get().set( "CASE_SENSITIVE", Option( "0" , "1 to distinguish upper and lower case when sorting/selecting text"  ) );
  XmlOptions::get().set( "DB_SIZE", Option( "10" , "max number of files stored in the open previous menu"  ) );

  XmlOptions::get().set( "SIDE_EDITOR_TOOLBAR", Option( "1" , "if true, editor toolbar is on the left instead of top"  ) );
  
  XmlOptions::get().set( "SHOW_KEYWORD", Option( "0" , "show keyword in editor"  ) );
  XmlOptions::get().set( "SHOW_EDITFRAME_MENU", Option( "1", "show menu in EditionWindow windows" ) );

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
  string user( Util::user( ) );
  string host( Util::host() );
  XmlOptions::get().set( "USER", Option( user+"@"+host, Option::NONE ) );  
  XmlOptions::get().set( "HELP_FILE", Option(  File( ".elogbook_help" ).addPath( Util::home() ), Option::NONE ) );  
  XmlOptions::get().set( "DB_FILE", Option( File( ".elogbook_db" ).addPath( Util::home() ), Option::NONE ) );
  XmlOptions::get().set( "RC_FILE", Option(  File(".elogbookrc").addPath(Util::home()), Option::NONE ) );
  XmlOptions::get().set( "APP_NAME", Option(  "ELOGBOOK", Option::NONE ) );
  XmlOptions::get().setAutoDefault( false );
  
};
