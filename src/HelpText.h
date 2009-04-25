#ifndef HelpText_h
#define HelpText_h

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
  \file    HelpText.h
  \brief   eLogbook help text
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

//! eLogbook help text
static const QString HelpText[] = 
{
  
  //_________________________________________________
  "Introduction",
  "<h2>Introduction</h2>"
  ""
  "<p>eLogbook is a Qt based application used to keep track of user text entries using tags.</p>"
  "<p>eLogbook offers the following features:</p>"
  "<ul>"
  "<li>A basic text editor. Each text entry is tagged using a title, several keywords and its creation time. The user may add new entries, modify/delete existing entries. The time of the last modifications of existing entries is also stored for tagging purposes.</li>"
  "<li>A compact display of all entry tags to allow fast browsing of the entries whose text is to be edited.</li>"
  "<li>A basic search engine to select a subset of the existing entries according to their title/keyword or full text.</li>"
  "<li>The possibility to attach files to the entries, for later editing. Attached file edition is based on the file type. Following types are supported: postscript, HTML and plain text. An <i>unknown</i> type is also used for all unsupported file types.</li>"   
  "</ul>"
  "<p>The text information are stored together with tags and attached file names/types in a collection of XML files. XML files are created automatically and store a finite number of entries to keep their size small and reduce the amount of disk access when saving modifications. A new file is generated each time the last filled file is full. The files are referred to in a <i>master</i> file which gets loaded first. </p>"
  "<p>The attached files are either copied or linked to a specific directory, whose name is also stored, to avoid irresponsible file deletion.</p>"
  "<p>The application has been designed to be as self-explaining as possible. For specific troubles/questions, one may consult the corresponding section in this help. The section names refer to the corresponding area in either the main (selection) window or one of the secondary (edition) windows. The <i>Getting started</i> section is aimed to help creating a new logbook and adding in the first text entry.</p>",

  //_________________________________________________
  "Definitions",
  "<h2>Definitions</h2>"
  "<h3>Logbook</h3>"
  "<p>A logbook consists in a collection of logbook entries (see below) and a collection of attachments (see below).</p>"
  "<p> On disk, a logbook is physically organized the following:</p>"
  "<ul>"
  "<li>a main file, named as was originally specified at the logbook creation;</li>"
  "<li>a collection of children files, of name derived from the main file name, that contains up to a fixed amount of entries.</li>"
  "</ul>"
  "<p>When the logbook is read from disk, the main file, then all its children are read. When a new entry is created it is added to the latest child file if the later is not full, otherwise a new child file is created and the entry is added to this one. The maximum number of entries per child file can be changed in the <i>Configuration</i> dialog. This structure aims to minimize file access for entries that have been written a long time ago and are rarely modified.</p>"
  "<h3>Logbook entry</h3>"
  "<p>An entry consists in a title and a keyword, to enable fast searching through the associated logbook; a text, stored in the logbook file; a collection of attachments whose corresponding file is stored in the logbook directory; a creation time stamp, referring to the time at which the entry was created and a modification time stamp referring to the time at which the entry was modified for the last time. These stamps are also used for fast searching through the logbook.</p>"
  "<h3>Attachments</h3>"
  "<p>An attachment consists in a file, stored in the logbook directory and a type, to enable the editing of the file using the correct application. At the moment plain text, HTML, URL, postscript and image files are supported. Other file types are handled as <i>unknown</i>. An attachment is always linked to a given logbook entry and may be explained or referred to in the entry text.</p>",
  
  //_________________________________________________
  "Getting Started",
  "<h2>Getting Started</h2>"
  "<p>When running eLogbook for the first time, the user needs to create a new logbook where the entries will be stored. This is performed by clicking on the <i>File::New Logbook</i> button. A pop-up dialog is opened, that allows to specify:</p>"
  "<ul>"
  "<li>the title of the logbook;</li>"
  "<li>the logbook author (default value is <i>username@hostname</i>);</li>"
  "<li>the logbook main file name. The default location of the file name is the application working directory. If no file is provided, the user will be asked to provide one the next time the logbook is saved.</li>"
  "<li>the directory where attachments will be saved. The default value for the directory is the application working directory;</li>"
  "</ul>"
  "<p>A more detailed description of logbook/entries/attachment manipulation tools is given elsewhere in this help.</p>",

  //_________________________________________________
  "Main window",
  "<h2>Main Window</h2>"
  "<p>The main window contains the following frames:</p>"
  "<ul>"
  "<li>a menu, which allows to open/save/create logbooks, to configure the application and to rapidly access the other application windows</li>"
  "<li>the list of current logbook keywords, presented using a tree structure;</li> "
  "<li>the list of entries that match the selected keyword;</li>"
  "<li>a search panel to select subsets of the logbook using various basic selection criteria.</li>"
  "</ul>"
  "<p>For more detailed information on one of the previous frames, refer to the corresponding section in this help.</p>", 
  
   //_________________________________________________
  "File Menu",
  "<h2>File Menu</h2>"
  "<p>The file menu is located in the menu bar of both the main window and the entry edition windows. It contains the following buttons:</p>"
  "<ul>"
  "<li><i>New</i>, used to create a new (empty) logbook. A pop-up is opened to set the title, author and file location of the new logbook as well as its attachment directory and some comments associated to the logbook;</li>"
  "<li><i>Open</i>, used to open an existing logbook. The logbook file name is selected using a file selection dialog;</li>"
  "<li><i>Open Previous</i> used to re-open a logbook that was already opened in previous session;</li>"
  "<li><i>Synchronize</i> used to synchronize the current logbook with another (remote) logbook. A pop-up dialog allows to select the remote logbook with which the current is to be synchronized. The synchronization is based on the entries creation and modification time stamps. When two entries are found with the same creation time stamp, the one that was modified the later is kept. <i>Warning</i>: when both logbook have been modified since the last modification, there is no way to merge the modifications applied concurrently on the same entries. Since only the entry that was modified the latest is kept, this effectively results in some loss of information.</li>"
  "<li><i>Reorganize</i>, used to rearrange logbook entries so that each file except the last one gets the maximum number of entries (default is 50). Entries are sorted by increasing creation time stamp, to minimize file access for entries that have been written a long time ago.</li>"
  ""
  "<li><i>Save</i>, used to save the logbook modifications to disk. If no file name exists, a file selection dialog is opened to select one. Among the main and children logbook files, only the files that contains entries that have been modified are saved; </li>"
  "<li><i>Save As</i>, used to save the logbook using a user specified file name. The new file name is chosen using a file "
  "selection dialog. Only the <i>main</i> file name must be specified. All children files are saved with an automatically "
  "generated file name; </li>"
  "<li><i>Save Backup</i>, used to make a copy of the current logbook into a tagged backup. By default the backup file is saved in the same directory as the original file. Its name is the original name appended to the current date. Backup files may also be created automatically at given time interval. The default interval between two automatic backups is one month, it can be modified in the <i>Configuration</i> dialog;</li>"
  "<li><i>Revert to Saved</i>, used to discard changes applied on the current logbook since last save and redisplay the saved version;</li>"
  "<li><i>View HTML</i>, used to generate a HTML formatted file from the logbook. A pop-up dialog is opened to configure the amount of information that is to be written to the HTML file, as well as the location of this file. The created HTML file can also be opened using the command specified in the pop-up window; </li>"
  "<li><i>Close</i>, used to close the current window. If this is the main window, the application also exits. If the last logbook modifications have not been saved, the user is asked if he wants to save/discard these modifications before exiting.</li>"
  "<li><i>Exit</i>, used to exit the application. If the last logbook modifications have not been saved, the user is asked if he wants to save/discard these modifications before exiting.</li>"
  "</ul>",
  
  //_________________________________________________
  "Settings Menu",
  "<h2>Settings Menu</h2>"
  "<p>The settings menu is located in the menu bar of both the main window and the entry edition windows and allows to open the application <i>Configuration</i> dialog.</p>",
  
  //_________________________________________________
  "Window Menu",
  "<h2>Window Menu</h2>"
  "<p>The <i>Window</i> menu is located in the menu bar of both the main window and the entry edition windows. It contains the following buttons:</p>"
  "<ul>"
  "<li><i>Main window</i>, used to raise the application main window;</li>"
  "<li><i>Editors</i>, to select one of the existing entry edition window and raise it. It also offers the possibility to close all existing editor windows;</li>"
  "<li><i>Attachments</i>, used to raise the attachment window;</li>"
  "<li><i>Logbook statistics</i>, used to raise the <i>logbook statistics</i> dialog that displays information about the current logbook, such as the main and children files, the creation, modification and backup timestamps, the total number of entries and attachments as well as the number of entries per child file. This is a read-only dialog.</li>"
  "<li><i>Logbook information</i>, used to raise the <i>logbook information</i> dialog that displays editable information about the current logbook, such as its title, author, default attachment directory and comments.</li>"
  "</ul>",
  
  //_________________________________________________
  "Help Menu",
  "<h2>Help Menu</h2>"
  "<p>The <i>Help</i> menu is located in the menu bar of both the main window and the entry edition windows. It gives access to this help; to information about the current version of <i>eLogbook</i> and the version of <i>Qt</i> that is used, and to a debugging menu that contains additional features used for maintenance and that the normal user should not need to access.</p>",
  
  //_________________________________________________  
  "Keyword/Entry lists",
  "<h2>Keyword/Entry lists</h2>"
  "<p>Keyword/Entry lists consist in two lists.</p>"
  "<h3>Keywords</h3>"
  "<p>The <i>keyword list</i> encloses all logbook keywords in a tree structure. A tool-bar is located at the top of the list that allows to add a new keyword to the tree and rename or delete an existing keyword. The same actions are available in a menu by right clicking on the list. Items in the keyword list can also be dragged and dropped, to modify the location of a given keyword and its associated entries in the tree structure;</p>"
  "<p>New keywords are placed in the tree using the \"/\" key to separate folders. Non existing keywords are created recursively by parsing the full path.</p>"
  "<p>Keywords can be edited directly by clicking on a given keyword and waiting for it to become editable.</p>"

  "<h3>Entries</h3>"
  "<p>The <i>entry list</i> is located to the right of the keyword list and displays basic information for the logbook entries associated the selected keyword such as: the entry title, its creation time and modification time as well as it author. A menu bar is located at the top of the list that allows to:</p>" 
  "<ul>"
  "<li>create a new entry;</li>"
  "<li>edit all selected entries;</li>"
  "<li>delete all selected entries;</li>"
  "<li>save all edited entries.</li>"
  "<li>display all selected entries in an HTML file;</li>"
  "<li>change the color of the selected entries. This color is used to display the entry in the list and can be used to tag entries for their importance, obsolescence, etc.</li>"
  "</ul>"
  "<p>The same actions are available in a menu when right-clicking in the list. Entries can be dragged and dropped in the keyword list to change their keyword. An entry gets edited in its dedicated Editor window by double-clicking on it in the list. Clicking twice on the title of an entry makes it editable.<p>",

  //_________________________________________________  
  "Entry selection",
  "<h2>Entry Selection</h2>"
  "<p>Some basic criteria can be used to select a fraction of the logbook entries to be displayed in the main window, namely: the entry title, keyword and text, the file name of its associated attachment, or the entry color. The color selection criteria is based on the color-name. Available color names are listed in the <i>Change Entry Color</i> menu obtained by right-clicking in the entry list or in the corresponding toolbar button.</p>"
  "<p>To choose the criteria to be used, one activates the corresponding toggle button at the bottom of the main window. The text entry located in the same frame is used for the selection. Selected are the entries whose title/keyword/text (depending on the activated toggle button) match the corresponding text.</p>"
  "<p>To perform the selection one must either press the <i>return</i> key or click on the <i>Find</i> button. To reset the selection and display all entries, one must either clean the text entry and press <i>return</i> key or click on the <i>Show All</i> button.</p>"
  "<p>By default a new selection always applies on top of previous selection. To apply it to the full selection, press <i>Show All</i> first, then <i>Find</i>.</p>",
  
  //_________________________________________________  
  "Edition Window",
  "<h2>Edition Window</h2>"
  ""
  "<p>The edition windows are opened by double clicking on an entry in the main-window or trigger the <i>Edit Selected Entries</i> button either in the toolbar or in the pop-up menu associated to the entry list. Each edition window is named from its associated entry. It displays detailed and editable information on its associated logbook entry, such as its title, text and list of associated attachments.</p>"
  "<h3>Toolbars</h3>"
  "<p>A number of actions related to the edition of the entry are available in toolbars located (by default) on the left of each edition window. The following toolbars are available:</p>"
  "<ul>"
  "<li>the <i>main toolbar</i>, used to create a new entry (and its associated empty edition window); save the current entry in the logbook, delete the current entry from the logbook, add an attachment to the current entry and run the spell-checker on the entry text (this option is available only if eLogbook was compiled with <i>aspell</i> support);</li>"
  "<li>the <i>text format toolbar</i>, used to apply basic formatting to the entry text (bold/underlined/italic font, font color, etc.). The text formats are stored together with the entry text in the logbook, and are propagated to HTML when converting the corresponding entry;</li>"
  "<li>the <i>history toolbar</i>, used to undo the last edition action or redo the last undone edition action;</li>"
  "<li>the <i>tools</i> toolbar, used to convert the current entry to an HTML file, clone the current editor (see below) or display basic information about the current entry, such as its title, keyword and author, creation and modification timestamps and the file to which it is effectively saved;</li>"
  "<li>the <i>navigation toolbar</i>, used to navigate through the logbook entries (namely display the entry located next or previous to the current entry) and raise the main window;</li>"
  "</ul>"
  "<h3>Attachment list</h3>"
  "<p>The <i>attachment list</i> is visible only if there is already at least one attachment associated to the current entry, and is located at the bottom of the edition window. A pop-up menu is opened when right clicking on this list, that allows to create a new attachment, and edit, display or delete selected existing attachments.</p>"
  "<p>See the <i>Attachments</i> dedicated section of this help for details on attachment types and associated actions</p>"
  "<h3>Read-only edition windows</h3>"
  "<p>There is always only one editable edition window opened for each entry, this to avoid conflicts when modifying entries. However each edition window can be cloned into read-only windows, that do not allow to modify the entry. Such windows have an additional <i>lock</i> button in the toolbars. Clicking on this button will unlock the edition window and allow the user to modify the associated entry, but all other existing windows associated to the same entry will simultaneously become read-only.</p>"
  "<p>The read-only edition windows associated to a given entries are update automatically every time the entry modifications performed in the editable window are saved to the logbook. When an edition window state is changed from editable to read-only, it is checked whether the corresponding entry has been modified and the user is asked whether these modifications should be saved or discarded.</p>",
  
  //_________________________________________________  
  "Attachments",
  "<h2>Attachments</h2>"
  "<p>An attachment consists in a file, stored in the logbook directory and a type, to enable the editing of the file using the correct application. An attachment is always linked to a given logbook entry and may be explained or referred to in the entry text.</p>"
  
  "<h3>Creating a new attachment</h3>"
  "<p>Attachments are created in entry edition windows by clicking on the <i>create attachment</i> button in the toolbars or in the pop-up menu associated to the corresponding attachment list. A pop-up dialog is opened to specify the attachment source file or URL, the directory in which the attachment is to be saved, the attachment type, the action to be performed to store the attachment, and some comments associated to this attachment.</p>"
  "<p>At the moment, only plain text, HTML, URL, postscript and image files are supported. Other file types are handled as <i>unknown</i>. URL attachments are handled differently from the other attachment types, since they do not require the attachment to be stored on disk. When selecting this type, the destination directory and the copy action selection box are disabled, since the URL of the attachment is stored unchanged in the body of the logbook.</p>"
  
  "<h3>Attachment lists</h3>"
  "<p>The list of existing attachments appear either in the corresponding entry edition windows, or in a separate <i>Attachment window</i> that can be opened via the <i>Window</i> menu. The first list only display the attachments associated to the current entry and allows the creation of new attachments or modifications of existing attachments whereas the second list displays all attachments available in the logbook but does not allow the creation of new attachments nor the modification of existing attachments.</p>"
  
  "<h3>Displaying attachments</h3>"
  "<p>To display an existing attachment, on must either double-click on it in one of the attachment lists, or select the corresponding button in the associated pop-up menu. A dialog is then opened to select the action to be taken. Attachments can either be displayed using a third-party application associated to the attachment type or saved on disk locally. The second option is naturally not available for attachments of type URL.</p>"
  
  "<h3>Editing attachments</h3>"
  "<p>The type of an existing attachment, its associated comments, or its URL can be modified by selecting <i>Edit attachment</i> in the pop-up menu associated to the attachment list.</p>"
  
  "<h3>Deleting attachments</h3>"
  "<p>Existing attachments can be deleted by selecting <i>Delete attachment</i> in the pop-up menu associated to the attachment list. A pop-up dialog is then opened to select the action to be taken. The attachment can be either deleted from the list of attachments associated to the current entry or effectively deleted from the disk. The second option is available only for attachments that are not of type URL and the actual deletion from the disk is preformed only if no other entry in the logbook refer to this attachment.</p>",
    
  //_________________________________________________  
  "Configuration Dialog",
  "<h2>Configuration dialog</h2>"
  "<p>The configuration dialog is opened via the preference menu located in the main menu bar and contains a list of tabs used to configure various features of the application. They are:</p>"
   "<h3>Base</h3>"
  "<p>The Base tab allows to modify the font used to display the widgets and the text; the application icon, the location where the toolbar icons are found (since the name of the toolbar icons match the one used for most of the desktop icon themes, this option allows to pick a set of icons that match the current desktop icon theme. Besides, it allows to try find additional icons that are foreseen in eLogbook, but not included in the distribution); the appearance of the toolbar buttons, and the background color of the items displayed in lists.</p>"
  "<h3>Attachments</h3>"
  "<p>The Attachments tab allows to select the third-party applications used to display the different attachment types supported by the application and to enable/disable automatic check of the existence of attached files when loading a logbook.</p>"
  "<h3>Window sizes</h3>"
  "<p>The Window sizes tab allows to select the default size of the various application windows. These options get automatically updated with the current values for the existing windows when closing the application.</p>"
  "<h3>List configuration</h3>"
  "<p>The List configuration tab allows to select which columns are displayed/hidden in the logbook entry list of the application main window.</p>"
  "<h3>Toolbars</h3>"
  "<p>The Toolbars tab allows to decide which toolbars should be visible/hidden by default in the entry edition windows, as well as the side of the window at which they should appear</p>"
  "<h3>Colors</h3>"
  "<p>The Colors tab defines which colors are available to tag entries in the main window entry list or the highlight text in the entry edition windows.</p>"
  "<h3>Backup</h3>"
  "<p>The Backup tab allows to enable/disable automatic save of the logbook modifications to its current file and associated children at fixed time intervals (in seconds), and the automatic backup of the logbook in a different file at fixed time interval (in days), for book-keeping. The auto-saved option is disabled by default. The auto-backup option is enabled by default and the backup time interval is 30 days.</p>"
  "<h3>Spell-checking</h3>"
  "<p>The Spell-checking tab is available only if eLogbook was compiled with <i>aspell</i> support. It allows to select the default dictionary and filter used for the spell-checking and the <i>aspell</i> command that is used to load the list of available dictionaries and filters.</p>"
  "<h3>Misc</h3>"
  "<p>the Misc tab allows to modify additional settings that don't enter the above categories. It covers tab emulation, paragraph highlighting and line-wrapping for the log-entry text edition window; case-sensitivity of the log-entry selection criteria, display of the menu bar in the entry edition windows (on by default) and visibility of eLogbook splash-screen at start-up.</p>",  
  0 };


#endif
