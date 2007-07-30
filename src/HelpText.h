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

#ifndef HelpText_h
#define HelpText_h

/*!
  \file    HelpText.h
  \brief   eLogbook help text
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

//! eLogbook help text
static const char* HelpText[] = {
  
  //=========
  "Introduction",
  "<B><U><FONT color=\"blue\">Introduction</FONT></U></B>"
  ""
  "<P>eLogbook is a Qt based application used to keep track of "
  "user text entries using tags."
  ""
  "<P>eLogbook offers the following features:"
  ""
  "<UL><LI>A basic text editor. Each text entry is tagged using a title, several keywords and its "
  "creation time. The user may add new entries, modify/delete existing entries. "
  "The time of the last modifications of existing entries is also stored for tagging "
  "purposes."
  ""
  "<LI>A compact display of all entry tags to allow fast browsing of the entries whose "
  "text is to be edited."
  ""  
  "<LI>A basic search engine to select a fraction of the existing entries according " 
  "to their title/keyword or full text."
  ""
  "<LI>The possibility to attach files to the entries, for later editing. Attached file "
  "edition is based on the file type. Following types are supported: postscript, "
  "HTML and plain text. An <i>unknown</i> type is also used for all unsupported file types."   
  "</UL>"
  "<P>The text information are stored together with tags and attached file names/types "
  "in a collection of XML files. XML files are created automatically and store a finite number "
  "of entries to keep their size small and reduce the amount of disk access when saving modifications. "
  "A new file is generated each time the last filled file is full. The files are referred to in "
  "a <i>master</i> file which gets loaded first. "
  "<P>The attached files are either copied or linked to a "
  "specific directory, whose name is also stored, to avoid irresponsible file deletion."
  ""
  "<P>The application has been designed to be as self-explaining as possible. " 
  "For specific troubles/questions, one may consult the corresponding section "
  "in this help. The section names refer to the corresponding area in one of the two "
  "windows displayed when running eLogbook. The <B>Getting started</B> section is aimed to "
  "help creating a new logbook and adding in the first text entry.",

  "Definitions",
  "<B><U><FONT color=\"blue\">Definitions</FONT></U></B>"
  ""
  "<UL><LI><B>Logbook</B>: "
  "A logbook consists in a collection of logbook entries (see below), stored into a single file, "
  "a collection of attachments (see below), and a directory where the corresponding files are stored."
  ""
  "<LI><B>Logbook entry</B>: "
  "An entry consists in a title and a keyword, to enable fast searching through the "
  "associated logbook; a text, stored in the logbook file; a collection of attachments whose corresponding file "
  "is stored in the logbook directory; a creation time stamp, referring to the time at which the entry was created "
  "and a modification time stamp referring to the time at which the entry was modified for the last time. These "
  "stamps are also used for fast searching through the logbook."
  ""
  "<LI><B>Attachments</B>: "
  "An attachment consists in a file, stored in the logbook directory; a type, to enable the editing of the file using "
  "the correct application. At the moment plain text, HTML, URL, postscript and image "
  "files are recognized. Other file types "
  "are handled as <i>unknown</i>. An attachment is always linked to a given logbook entry and may be explained or referred to "
  "in the entry text." 
  "</UL>",

  "Getting Started",
  "<B><U><FONT color=\"blue\">Getting Started</FONT></U></B>"
  ""
  "<P>When running eLogbook for the first time, the user needs to create a new logbook where the entries will be stored. "
  "This is performed by clicking on the <B>File::New Logbook</B> button. A pop-up dialog "
  "is opened where the title of the logbook and the directory where attachments will be saved can be specified. "
  "The default value for the directory is the one from which the application was launched. When giving a non existing directory "
  "the user is asked if he wants to create it new. If no valid directory is given, attachments adding is disabled. The "
  "file where the logbook will be written to disk is not specified yet. It will be asked the first time the logbook is saved, "
  "when clicking either on the <B>File::Save Logbook</B> or on the <B>File::Save As</B> button. If an existing file name is "
  "given then, it will be overwritten. The same procedure is to be used every time the user wants to create a new logbook."
  ""
  "<P>The <B>Electronic logbook editor</B> window is used to add/remove/modify entries to the selected logbook. The entries modifications "
  "are saved using the <B>Save entry</B> tool button. The logbook "
  "is automatically rewritten to disk each time the entry modifications are saved, provided a valid file name is given. Otherwise, this is "
  "done when clicking on the <B>File::Save Logbook</B> or the <B>File::Save As</B> buttons."
  ""
  "<P>Before exiting the application or creating/opening a different logbook, the user is asked to save or discard the changes made to the current "
  "one, to avoid loss of modifications."
  ""
  "<P>A more detailed description of logbook/entries/attachment manipulation tools is given in the other sections of this help."
  "",

  "Main window",
  "<B><U><FONT color=\"blue\">Main Window</FONT></U></B>"
  ""
  "<P>The main window is named \"Electronic logbook\". "
  "It contains the following frames:"
  ""
  "<UL><LI>A menu, which allows to open/save/create logbooks; to change some of the application options; "
  "fast access to the other application windows"
  ""
  "<LI>The entries table, which list all entries to enable fast browsing and searching through the logbook. It is separated in two lists. "
  "The first encloses all keywords in a tree structure. The second displays entries associated to a given keyword."
  ""
  "<LI>The entry selection frame, to select subsets of the logbook using various basic selection criteria."
  "</UL>"
  "<P>For more detailed information on one of the previous frames, refer to the corresponding section in this help."
  ""
  "<P>Related: Menu/Logbook information/Entry selection/Logbook Entries Table."
  "", 
  
  "File Menu",
  "<B><U><FONT color=\"blue\">File Menu</FONT></U></B>"
  ""
  "<P>The file menu is located in the main menu bar. It contains the following buttons:"
  ""
  "<UL><LI><B>New</B>: "
  "to create a new (empty) logbook. A pop-up is opened to set the title of the new logbook and "
  "its attachment directory."
  ""
  "<LI><B>Open</B>: "
  "to open an existing logbook. The logbook file name is selected using a file selection dialog."
  ""
  "<LI><B>Open Previous</B>: "
  "displays the name of the recently opened logbook files. Selecting one of these results in opening "
  "the corresponding logbook."
  ""
  "<LI><B>Synchronize</B>: "
  "opens a new logbook and merge it with the current one; save the two logbooks, now identical. "
  "<B>Warning</B>: when entries are found with the same creation time, the entry with the latest modification is kept. "
  "This can result in a loss of information when entries have been modified independently in the two logbooks after the "
  "last synchronization."
  ""
  "<LI><B>Reorganize</B>: "
  "Rearrange logbook entries so that each file except the last one gets the maximum number of entries (default is 50)."
  ""
  "<LI><B>Save</B>: "
  "to save the logbook modifications to disk. If no file name exists, a file selection dialog is opened to select one. "
  "This is the case when saving a freshly created logbook for the first time. Logbooks are split into several sub-files "
  "of given maximum size (default is 50) to limit "
  "the size of the files and increase security in case of application crash. Only the modified sub-files are saved."
  ""
  "<LI><B>Save (forced)</B>: "
  "to save the logbook modifications to disk. All sub-files are saved whether they have been modified or not. Consequently the operation takes "
  "longer than the standard save."
  ""
  "<LI><B>Save As</B>: "
  "to save the logbook using a user specified file name. The new file name is chosen using a file "
  "selection dialog. Only the <B>main</B> file name must be specified. All sub files are saved with an automatically "
  "generated file name."
  ""
  "<LI><B>Save Backup</B>: "
  "to make a copy of the current logbook into a tagged backup. By default the backup file is saved in the same directory as "
  "the original file. It's name is the original name appended to the current date. Backup files may also be created "
  "automatically at given time interval. The default interval between two automatic backups is one month."
  ""
  "<LI><B>Revert to Saved</B>: "
  "to discard changes applied on the current logbook since last save and redisplay the saved version."
  ""
  "<LI><B>View HTML</B>: "
  "to generate a HTML formatted file from the logbook. The user can select either the full logbook (default) or the table of content. "
  "The generated HTML file name may also be changed in the corresponding pop-up window. The created HTML file is opened using the command "
  "specified in the pop-up window."
  ""
  "<LI><B>Exit</B>: "
  "to exit the application. If the last logbook modifications have "
  "not been saved, the user is asked if he wants to save/discard the modifications before exiting."
  "</UL>"
  "<P>Related: Entry selection/Logbook Entries Table/Editor Window."
  "",
  
  "Preferences Menu",
  "<B><U><FONT color=\"blue\">Preferences Menu</FONT></U></B>"
  ""
  "<P>The Preferences menu is located in the main menu bar. It contains the following items:"
  ""
  "<UL><LI><B>Configuration</B>: "
  "opens the configuration dialog that contains all options for the logbook." 
  "",
    
  "Windows Menu",
  "<B><U><FONT color=\"blue\">Windows Menu</FONT></U></B>"
  ""
  "<P>The Windows menu is located in the main menu bar. It contains the following buttons:"
  ""
  "<UL><LI><B>Editors</B>: opens a list of all opened editor windows, allows to select one or close them all."
  ""
  "<LI><B>Attachments</B>: raises the Attachments Window."
  ""
  "<LI><B>Logbook information</B>: raises the logbook information dialog."
  "</UL>"
  "<P>Related: Editor Window/Attachments Window."
  "",
  
  "Logbook information",
  "<B><U><FONT color=\"blue\">Logbook Information</FONT></U></B>"
  ""
  "<P>The logbook information dialog displays all logbook information including:"
  ""
  "<UL><LI>the logbook title;"
  "<LI>the logbook author name;"
  "<LI>the logbook default attachment directory;"
  "<LI>the logbook file name;"
  "<LI>the logbook last modification time;"
  "<LI>the last backup time;"
  "<LI>statistics concerning the number of entries and attachments in the logbook and each of its files."
  "</UL>"
  "<P>Some of the information can be modified in this window, namely: "
  "<UL><LI>the logbook title, "
  "<LI>the logbook author name, "
  "<LI>the logbook default attachment directory."
  "</UL> "
  "<P>The modifications are taken into account when pressing the <B>OK</B> button."
  ""
  "<P>Related: Attachments." 
  "",
  
  "Logbook Entries Table",
  "<B><U><FONT color=\"blue\">Logbook Entries Table</FONT></U></B>"
  ""
  "<P>The logbook entries table consists in two lists. "
  ""
  "The <b>keyword list</b> encloses all logbook keywords in a tree structure. A menu bar is located at the top of the list. It allows to add a new keyword to the tree or rename an existing keyword."
  "New keywords are placed in the tree using the \"/\" key to separate folders. Non existing keywords will be created recursively by parsing the full path."
  "Keywords can be edited directly by clicking on a given keyword and waiting for it to become editable. A keyword and all its associated entries can be moved"
  "through the tree using usual drag and drop."
  "<p>A popup menu is opened when right clicking in the list. It allows to create a new keyword or rename the current selection."
  "<p>The <b>entry list</b> encloses all logbook entries matching the keyword selected in the left list. It displays basic information on each entries "
  "such as: the entry title, its creation time, its last modification time. A menu bar is located at the top of the list. It allows to "
  "<ul>"
  "<li>create a new entry; "
  "<li>edit all selected entries; "
  "<li>delete all selected entries; "
  "<li>display all selected entries in an html file;"
  "<li>save all edited entries."
  "</ul>"
  "Entries can be dragged and dropped in the keyword list to change their keyword."
  "An entry gets edited in its dedicated Editor window by double-clicking on it in the list."
  "<p>A popup menu is opened when right clicking in the list. It allows to perform all actions available in the logbook. Additionally, it allows to change the selected "
  "entries color in the list, for highlighting."  
  "<P>A header band specifies the meaning of each column in the list located below. Clicking on one item of this header sorts the logbook "
  "list of entries according to the corresponding criterion. The last column, which has no label "
  "is used to sort the entries according to their color."
  ""
  "<P>Related: Entry selection/Editor window.",

  "Entry selection",
  "<B><U><FONT color=\"blue\">Entry Selection</FONT></U></B>"
  ""
  "<P>Some basic criteria are used to select a fraction of the logbook entries to be displayed in the main window. "
  "Namely: "
  "<UL><LI>their title, "
  "<LI>their keyword, "
  "<LI>their text, " 
  "<LI>their associated attachments"
  "</UL> "
  "<P>To choose the criteria to be used, one activates the corresponding "
  "toggle button in the <B>Entry selection</B> frame of the main window. The text entry located in the same frame is used for the selection. "
  "Selected are the entries whose title/keyword/text (depending on the activated toggle button) match the corresponding text. "
  ""
  "<P>To perform the selection "
  "one must either press the <B>return</B> key or click on the <B>Find</B> button. To reset the selection and display all entries, "
  "one must clean the text entry and press <B>return</B> key or click on the <B>Show All</B> button."
  ""
  "<P>By default a new selection always applies on top of previous selection. To apply it to the full selection, "
  "press <B>Show All</B> then <B>Find</B>."
  ""
  "<P>Related: Logbook Entries Table."
  "",
  
  "Editor Window",
  "<B><U><FONT color=\"blue\">Editor Window</FONT></U></B>"
  ""
  "<P>The editor window is the second window of the application and it is named from the entry displayed inside. "
  "It cannot be destroyed unless when exiting the application. It displays detailed information on selected logbook entries "
  "or freshly created ones. It contains the following frames:"
  ""
  "<UL><LI>Title frames;"
  "<LI>Actions frame;"
  "<LI>Text frame;"
  "<LI>Attachment frame."
  "</UL>"
  "<P>Related: Logbook Entries Table/Window Menu."
  "",

  "Actions",
  "<B><U><FONT color=\"blue\">Actions</FONT></U></B>"
  ""
  "<P>The action frame enables logbook entry manipulation and navigation. Following actions are available, represented by "
  "self-explaining pixmaps:"
  "<UL>"
  "<LI>display the entry located immediately before the current entry, if any. The button has no effect when the current entry "
  "is freshly created;"
  "<LI>display the entry located immediately after the current entry, if any. The button has no effect when the current entry "
  "is freshly created;"
  "<LI>display current entry information;"
  "<LI>undo last modification in current editor (title, keyword or text);"
  "<LI>redo last undone modification in current editor (title, keyword or text);"
  "<LI>create a new entry. The entry title, keyword, text and attachment list either have default values or "
  "are empty. They all can be modified by the user. The new entry is added to the logbook only when the \"Save Entry\" "
  "button is clicked on (see below), or when the user replies \"yes\" to the corresponding pop-ups."  
  "<LI>delete the current entry from the logbook. When clicking on this button, the user is asked for confirmation. The "
  "deletion is not propagated to the possible attached files written on disk. It is therefore recommended to erase "
  "all entry attachments before removing the entry."
  "<LI>save the current entry. The modifications performed on the edited entry title, keyword or text are written to the logbook. "
  "The entry last modification time stamp is updated. If the edited entry is new, a new line is added to the logbook entries table. "
  "Added/modified entries are then automatically written to the logbook file."
  "<LI>adds a new attachment to the current entry."
  "<LI>create an HTML page from the current entry."
  "<LI>create a new read-only editor window with current entry."
  ""
  "<LI>create an HTML page from the current entry."
  ""
  "</UL>"
  "Additionally a <b>lock</b> button is displayed on the right for read-only editors. Clicking on the button makes the current editor "
  "editable. All other editors matching the same entry are made read-only. They are saved automatically in case of modifications to avoid conflicts."
  "<P>Related: Main Window/Menu/Read-only Editor/Attachments Window/Logbook Entries Table."
  "",
  
  "Entry Title",
  "<B><U><FONT color=\"blue\">Entry Title</FONT></U></B>"
  ""
  "<P>The selected entry title is displayed at the top of the Editor Window and can be modified by user. "
  "Freshly created entries have a default \"untitled\" title."
  "The title modifications are saved to the logbook when "
  "the <B>Save Entry</B> tool button is clicked on either in the editor or in the main window, or by clicking Ctrl+S in the window."
  "<P>Related: Actions"
  "",
  
  "Text ",
  "<B><U><FONT color=\"blue\">Text</FONT></U></B>"
  ""
  "<P>The text frame is located in the editor window. This is a basic text editor which allows to modify the text corresponding to the selected/"
  "freshly created entry."
  ""
  "<P>Related: accelerators."
  "",
  
  "Attachments",
  "<B><U><FONT color=\"blue\">Attachments</FONT></U></B>"
  ""
  "<P>The attachment frame is located at the bottom of the editor window. It is visible only when the current entry as at least one associated "
  "attachment. The frame contains the list of the attachment files corresponding to "
  "the selected entry. Each line in the attachment "
  "list corresponds to an attachment file name. The presence of the attachment file on the disk is checked when updating "
  "the list. When found, the type of the file (Unknown/Postscript/Plain Text/HTML/URL) is given. This list is updated "
  "when the selected entry is changed; when a new entry is created or when an attachment is added to/deleted from the list."
  ""
  "Clicking in the attachment frame with third buttons opens a menu with following buttons:"
  ""
  "<UL><LI><B>New attachment</B>: "
  "to add a new file to the attachment list. A pop-up dialog is opened where the user can specify the following:"
  ""
  "<UL><LI>the file name to be attached (it can also be selected from a file selection dialog);"
  "<LI>the type of the file (Unknown/Postscript/Plain Text/HTML/URL);"
  "<LI>the destination directory where the attachment is to be saved (the default value is the logbook default attachment directory);"
  "<LI>the action to be performed (either a copy to logbook directory or a symbolic link)."
  ""
  "<LI>The type of the file determines the application to be used "
  "when editing the attachment file. The applications associated to different file types is specified in the configuration dialog. The actions "
  "(copy/link) are disabled when the type of the file is URL, as the address of the URL is kept in the logbook file and nothing is written "
  "to the disk."
  "</UL>"
  "<LI><B>View attachment</B>: "
  "to view the attachment selected in the list. A pop-up dialog is opened where the file name is written, as well as the name of the application "
  "to be used for editing and the attachment associated comments. The application name is selected using the type of the attachment, " 
  "as given at creation. The line is left blank when the type is \"Unknown\" but still can be modified manually by the user before editing."
  ""
  "<LI><B>Edit attachment</B>: "
  "To display modify the comments and/or type associated to the attachment selected in the list. The changes are saved when clicking on "
  "the <B>OK</B> button in the corresponding pop-up."
  "<LI><B>Delete attachment</B>"
  "to deleted the attachment selected in the list. A pop-up dialog is opened to ask the user if the attachment is to be removed from the logbook "
  "entry only or from the logbook and the disk."
  "</UL>"
  "Modifications of the list of attachment are automatically written to the logbook."
  ""
  "<P>Related: Preferences menu/Logbook entry table/eLogbook configuration dialog."   
  "",
  
  "Read-only Editor",
  "<B><U><FONT color=\"blue\">Read-only Editor</FONT></U></B>"
  ""
  "<P>Read-only editors are secondary windows of the application which can be destroyed at will. They are opened when clicking on the "
  "\"Read only editor\" button of the Actions frame in the editor window. The window contains a copy of the logbook entry information "
  "in the same layout as for the editor window, except that all action buttons have been removed/disabled. The text field may be selected "
  "or copied but not edited. It is useful to make a copy of part of an entry to another one. The opened Read-only editor windows are "
  "updated every time the corresponding entry is saved, when selected in the (editable) editor window. They are destroyed automatically when "
  "the logbook is changed or when the corresponding entry is deleted from the logbook. Read-only editor windows have an additional <b>lock</b> "
  "button in the menu. The window is made editable when clicking on that button. Any other editor window matching the same entry is made read-only "
  "at the same time to avoid conflicts."
  ""
  "<P>Related: File Menu/Editor Window/Actions"  
  "",
  
  "Attachment Window",
  "<B><U><FONT color=\"blue\">Attachment Window</FONT></U></B>"
  ""
  "<P>The Attachment Window is opened via the <B>Windows::Attachment</B> Menu button. "
  "It contains a list of all attachments present in the current "
  "logbook, if any. The file name and its type are written. The presence of the file "
  "on the disk is checked. Whenever an attachment is selected, the corresponding entry is "
  "displayed in the Editor. Clicking in the "
  "list with the mouse third buttons opens a pop-up with the following buttons:"
  ""
  "<UL><LI><B>View attachment</B>: "
  "Opens a pop-up to view the selected attachment file according to its type. The same action is performed "
  "when double clicking on an item in the list."
  ""
  "<P>Related: Editor Window."
  "",
  
  "Accelerators",
  "<B><U><FONT color=\"blue\">Accelerators</FONT></U></B>"
  ""
  "<P>Accelerators are combinations of keys used to activate some of the buttons using "
  "the keyboard. Available accelerators are displayed "
  "close to the buttons to which they are associated."
  "All accelerators which may alter the entry text are disabled in the Read-Only editor window."
  ""
  "<P>Related: Main window/Menu/Editor Menu"
  "",
  
    
  "Configuration Dialog",
  "<B><U><FONT color=\"blue\">Configuration dialog</FONT></U></B>"
  ""
  "<P>The configuration dialog is opened via the preference menu located in the main menu bar. It contains the"
  " following tabs:"
  ""
  "<UL><LI><B>Base</B>: "
  "handles the base configuration of the application, such as "
  "<ul>"
  "<li>the fonts;"
  "<li>the application icon;"
  "<li>the debugging verbosity;"
  "<li>the icon search path;"
  "<li>the list item color."
  "</ul>" 
  ""
  "<LI><B>window size</B>: handles the application window size;"
  "<LI><B>list configuration</B>: handles which columns are shown/hidden in the main window entry list;"
  "<LI><B>toolbars</B>: handles toolbars location and which toolbars are to be shown/hidden in the editor window;"
  "<LI><B>backup</B>: configure the logbook autosave and auto backup options;"
  "<LI><B>misc</B>: handles additional options such as"
  "<UL>"
  "<LI> the spell checker configuration;"
  "<LI> the available colors used to mark entries importance;"
  "<LI> the text colors available for the entry edition;"
  "<LI> the visibility of the keyword edition text entry in the editor window;"
  "<LI> the case sensitivity of the search bar;"
  "<LI> the visibility of the startup splash screen;"
  "</UL>"
  "</UL>" 
  "<P>Related: Attachment/Editor Window/Entry Selection/Tools Menu."
  "",
  0 };


#endif
