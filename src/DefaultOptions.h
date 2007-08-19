// $Id$

/******************************************************************************
*                        
* Copyright (C) 2002 Hugo PEREIRA XmlOptions::get().add( Option("mailto: hugo.pereira@free.fr>            
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

#include "XmlOptions.h"
#include "Config.h"
#include "Util.h"

using namespace std;

//_____________________________________________________
//! Default options installer
void installDefaultOptions( void )
{
  // set options default values
  
  // COLOR options are special. Keep the full list
  XmlOptions::get().keep( "COLOR" );
  XmlOptions::get().add( Option( "COLOR", "#aa0000" , "entry display color"  ));
  XmlOptions::get().add( Option( "COLOR", "#FF9900" , "entry display color"  ));
  XmlOptions::get().add( Option( "COLOR", "#009900" , "entry display color"  ));
  XmlOptions::get().add( Option( "COLOR", "#3333FF" , "entry display color"  ));
  XmlOptions::get().add( Option( "COLOR", "#993399" , "entry display color"  ));
  
  // pixmap path options are special. Keep the full list
  XmlOptions::get().keep( "PIXMAP_PATH" );
  XmlOptions::get().add( Option( "PIXMAP_PATH", ":/pixmaps" , "directory where application pixmaps are taken from."  ));
  
  // icon
  XmlOptions::get().add( Option( "ICON_PIXMAP", ":/icon.png" , "application icon"  ));
  
  // normal (overwritten) options
  XmlOptions::get().add( Option( "ATC_FRAME_HEIGHT", "400" , "requested AttachmentFrame height [pixels]"  ));
  XmlOptions::get().add( Option( "ATC_FRAME_WIDTH", "600" , "requested AttachmentFrame width [pixels]"  ));
  XmlOptions::get().add( Option( "ATC_HEIGHT", "150" , "requested height of attachment list in editor [pixels]"  ));
  XmlOptions::get().add( Option( "EDT_HEIGHT", "600" , "requested EditFrame height [pixels]"  ));
  XmlOptions::get().add( Option( "EDIT_FRAME_HEIGHT", "750" , "requested EditFrame height [pixels]"  ));
  XmlOptions::get().add( Option( "EDIT_FRAME_WIDTH", "700" , "requested EditFrame width [pixels]"  ));
  XmlOptions::get().add( Option( "SELECTION_FRAME_HEIGHT", "750" , "requested SelectionFrame height [pixels]"  ));
  XmlOptions::get().add( Option( "SELECTION_FRAME_WIDTH", "700" , "requested SelectionFrame width [pixels]"  ));
  XmlOptions::get().add( Option( "ENTRYLIST_MASK", "94" ));
  
  XmlOptions::get().add( Option( "KEYWORD_LIST_WIDTH", "250" , "requested keyword list width [pixels]"  ));
  XmlOptions::get().add( Option( "ENTRY_LIST_WIDTH", "450" , "requested entry list width [pixels]"  ));

  XmlOptions::get().add( Option( "CHECK_ATTACHMENT", "0", "check if attachment exists at startup" ) );
  
  XmlOptions::get().add( Option( "AUTO_BACKUP", "1" , "1 to make a backup of logbook file prior to any writting"  ));
  XmlOptions::get().add( Option( "AUTO_SAVE", "0" , "1 to save logbook automaticaly every given interval"  ));
  XmlOptions::get().add( Option( "AUTO_SAVE_ITV", "60" , "interval between two consecutive automatic save [seconds]"  ));
  XmlOptions::get().add( Option( "BACKUP_ITV", "30" , "interval between two consecutive auto backup [days]"  ));
  XmlOptions::get().add( Option( "CASE_SENSITIVE", "0" , "1 to distinguish upper and lower case when sorting/selecting text"  ));
  XmlOptions::get().add( Option( "DB_SIZE", "10" , "max number of files stored in the open previous menu"  ));

  XmlOptions::get().add( Option( "SIDE_EDITOR_TOOLBAR", "1" , "if true, editor toolbar is on the left instead of top"  ));
  XmlOptions::get().add( Option( "SPLASH_SCREEN", "1" , "1 to show splash screen"  ));
  XmlOptions::get().add( Option( "SHOW_KEYWORD", "0" , "show keyword in editor"  ));
  XmlOptions::get().add( Option( "SHOW_EDITFRAME_MENU", "1", "show menu in EditFrame windows" ));

  #ifdef WITH_ASPELL
  XmlOptions::get().add( Option( "ASPELL", "@ASPELL@", "aspell command" ));
  XmlOptions::get().add( Option( "DICTIONARY", "en" , "default dictionary"  ));
  XmlOptions::get().add( Option( "DICTIONARY_FILTER", "none" , "default filter"  ));
  #endif
  
  XmlOptions::get().add( Option( "MAIN_TOOLBAR",   "1" , "toolbar visibility" ));
  XmlOptions::get().add( Option( "FORMAT_TOOLBAR",  "1" , "toolbar visibility" ));
  XmlOptions::get().add( Option( "EDITION_TOOLBAR", "0" , "toolbar visibility" ));
  XmlOptions::get().add( Option( "EXTRA_TOOLBAR", "1" , "toolbar visibility" ));
  XmlOptions::get().add( Option( "NAVIGATION_TOOLBAR", "0" , "toolbar visibility" ));

  XmlOptions::get().add( Option( "MAIN_TOOLBAR_LOCATION",   "left" , "toolbar location" ));
  XmlOptions::get().add( Option( "FORMAT_TOOLBAR_LOCATION",  "left" , "toolbar location" ));
  XmlOptions::get().add( Option( "EDITION_TOOLBAR_LOCATION", "left" , "toolbar location" ));
  XmlOptions::get().add( Option( "EXTRA_TOOLBAR_LOCATION", "left" , "toolbar location" ));
  XmlOptions::get().add( Option( "NAVIGATION_TOOLBAR_LOCATION", "left" , "toolbar location" ));
  
  // add run-time non recordable options
        
  // user name and host
  string user( Util::user( ) );
  string host( Util::host() );
  Option option( "USER", user+"@"+host );  
  option.setRecordable( false );
  XmlOptions::get().add( option );

  // help file
  option = Option( "HELP_FILE",  File( ".eLogbook_help" ).addPath( Util::home() ) );  
  option.setRecordable( false );
  XmlOptions::get().add( option );
  
  // DB file (for previously opened files
  option = Option( "DB_FILE", File( ".eLogbook_db" ).addPath( Util::home() ) );  
  option.setRecordable( false );
  XmlOptions::get().add( option );

  // user resource file
  option = Option( "RC_FILE", File(".elogbookrc").addPath(Util::home()));
  option.setRecordable( false );
  XmlOptions::get().add( option );
  
};
