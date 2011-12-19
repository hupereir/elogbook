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
\file ConfigurationDialog.cpp
\brief configuration dialog
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "Application.h"
#include "AttachmentWindow.h"
#include "ConfigurationDialog.h"
#include "CustomToolBar.h"
#include "Debug.h"
#include "FileList.h"
#include "GridLayout.h"
#include "LogbookPrintOptionWidget.h"
#include "LogEntryPrintOptionWidget.h"
#include "MainWindow.h"
#include "OptionBrowsedLineEditor.h"
#include "OptionCheckBox.h"
#include "OptionSpinBox.h"
#include "OptionListBox.h"
#include "Options.h"
#include "RecentFilesConfiguration.h"
#include "ServerConfiguration.h"
#include "Singleton.h"
#include "TreeViewConfiguration.h"

#include "Config.h"
#if WITH_ASPELL
#include "SpellCheckConfiguration.h"
#endif

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QGroupBox>

//_________________________________________________________
ConfigurationDialog::ConfigurationDialog( QWidget* parent ):
BaseConfigurationDialog( parent )
{

    Debug::Throw( "ConfigurationDialog::ConfigurationDialog.\n" );
    setWindowTitle( "Configuration - Elogbook" );

    baseConfiguration();

    // generic objects
    QGroupBox *box;
    OptionCheckBox *checkbox;
    OptionSpinBox* spinbox;

    // attachment editors
    QWidget* page = &addPage( "Commands", "System dependent commands used to edit attachments" );
    page->layout()->addWidget( box = new QGroupBox( "Attachment Editors", page ));

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setSpacing(5);
    gridLayout->setMargin(5);
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( gridLayout );

    OptionBrowsedLineEditor *editor;

    gridLayout->addWidget( new QLabel( "Default: ", box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_UNKNOWN_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( "HTML: ", box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_HTML_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( "URL: ", box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_URL_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( "Text: ", box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_PLAIN_TEXT_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( "Postscript: ", box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_POSTSCRIPT_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( "Image: ", box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_IMAGE_ATC" ) );
    addOptionWidget( editor );

    gridLayout->setColumnStretch( 1, 1 );

    gridLayout->addWidget( new QLabel( "Icon size: ", box ) );
    gridLayout->addWidget( spinbox = new OptionSpinBox( box, "ATTACHMENT_LIST_ICON_SIZE" ) );
    spinbox->setMinimum( 8 );
    spinbox->setMaximum( 96 );
    spinbox->setToolTip( "Icon size in attachment lists" );
    addOptionWidget( spinbox );

    page->layout()->addWidget( box = new QGroupBox( "Print Commands", page ) );

    box->setLayout( new QVBoxLayout() );
    box->layout()->setMargin(5);
    box->layout()->setSpacing(5);

    OptionListBox* listbox;
    listbox = new OptionListBox( box, "PRINT_COMMAND" );
    listbox->setBrowsable( true );
    addOptionWidget( listbox );
    listbox->setToolTip( "Available command for printing/editing converted files" );
    box->layout()->addWidget( listbox );

    // listview configuration
    page = &addPage( "List configuration", "Visible columns in logbook keywords and entries lists" );
    TreeViewConfiguration *listview_config = new TreeViewConfiguration(
        page,
        &Singleton::get().application<Application>()->mainWindow().logEntryList(),
        Singleton::get().application<Application>()->mainWindow().logEntryList().maskOptionName() );

    listview_config->setTitle( "Logbook entries list" );
    addOptionWidget( listview_config );
    page->layout()->addWidget( listview_config );

    // attachment configuration
    listview_config = new TreeViewConfiguration(
        page,
        &Singleton::get().application<Application>()->attachmentWindow().frame().list(),
        Singleton::get().application<Application>()->attachmentWindow().frame().list().maskOptionName() );
    listview_config->setTitle( "Attachments list" );
    addOptionWidget( listview_config );
    page->layout()->addWidget( listview_config );

    // toolbars
    page = &addPage( "Toolbars", "Toolbars visibility and location" );
    box = new QGroupBox( "Toolbars", page );
    page->layout()->addWidget( box );

    gridLayout = new GridLayout();
    gridLayout->setSpacing(5);
    gridLayout->setMargin(5);
    gridLayout->setMaxCount(2);
    box->setLayout( gridLayout );

    gridLayout->addWidget( new QLabel( "Visibility", box ) );
    gridLayout->addWidget( new QLabel( "Location", box ) );

    OptionComboBox* combobox;

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Main Toolbar", box, "MAIN_TOOLBAR" ));
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "MAIN_TOOLBAR_LOCATION" ) );
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Undo History Toolbar", box, "EDITION_TOOLBAR" ));
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EDITION_TOOLBAR_LOCATION" ));
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Format Toolbar", box, "FORMAT_TOOLBAR" ));
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "FORMAT_TOOLBAR_LOCATION" ));
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Tools", box, "EXTRA_TOOLBAR" ));
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "EXTRA_TOOLBAR_LOCATION" ));
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Navigation Toolbar", box, "NAVIGATION_TOOLBAR" ));
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "NAVIGATION_TOOLBAR_LOCATION" ));
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Multiple Views Toolbar", box, "MULTIPLE_VIEW_TOOLBAR" ));
    gridLayout->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "MULTIPLE_VIEW_TOOLBAR_LOCATION" ));
    addOptionWidget( checkbox );
    addOptionWidget( combobox );

    box = new QGroupBox( page );
    page->layout()->addWidget( box );

    box->setLayout( new QHBoxLayout() );
    box->layout()->setSpacing( 5 );
    box->layout()->setMargin( 5 );

    box->layout()->addWidget( new QLabel( "Lock position: ", box ));
    box->layout()->addWidget( combobox = new CustomToolBar::LocationComboBox( box, "LOCK_TOOLBAR_LOCATION" ) );
    addOptionWidget( combobox );

    // colors
    page = &addPage( "Colors", "Color settings for entry tagging and text highlighting" );
    box = new QGroupBox( "Logbook entry colors", page );
    box->setLayout( new QVBoxLayout() );
    box->layout()->setMargin(5);
    box->layout()->setSpacing(5);
    page->layout()->addWidget( box );

    box->layout()->addWidget( listbox = new OptionListBox( box, "COLOR" ) );
    listbox->setToolTip( "Colors used for logbook entry display" );
    addOptionWidget( listbox );

    box = new QGroupBox( "Text colors", page );
    box->setLayout( new QVBoxLayout() );
    box->layout()->setMargin(5);
    box->layout()->setSpacing(5);
    page->layout()->addWidget( box );

    box->layout()->addWidget( listbox = new OptionListBox( box, "TEXT_COLOR" ) );
    listbox->setToolTip( "Colors used for text formatting" );
    addOptionWidget( listbox );

    // auto save
    page = &addPage( "Backup", "Logbook backup configuration" );
    box = new QGroupBox( "Auto save", page );
    page->layout()->addWidget( box );

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(5);
    layout->setSpacing(5);
    box->setLayout( layout );

    layout->addWidget( checkbox = new OptionCheckBox( "Auto save", box, "AUTO_SAVE" ) );
    addOptionWidget( checkbox );
    checkbox->setToolTip( "Automatically save logbook at fixed time interval" );

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setSpacing(5);
    hLayout->setMargin(0);
    layout->addLayout( hLayout );

    hLayout->addWidget( new QLabel( "Auto save interval (seconds): ", box ) );
    hLayout->addWidget( spinbox = new OptionSpinBox( box, "AUTO_SAVE_ITV" ) );
    hLayout->addStretch(1);
    spinbox->setMinimum( 0 );
    spinbox->setMaximum( 3600 );
    spinbox->setToolTip( "Time interval between two automatic save" );
    addOptionWidget( spinbox );

    checkbox->setChecked( false );
    spinbox->setEnabled( false );
    connect( checkbox, SIGNAL( toggled( bool ) ), spinbox, SLOT( setEnabled( bool ) ) );

    // auto backup
    box = new QGroupBox( "Auto backup", page );
    layout = new QVBoxLayout();
    layout->setMargin(5);
    layout->setSpacing(5);
    box->setLayout( layout );
    page->layout()->addWidget( box );

    layout->addWidget( checkbox = new OptionCheckBox( "Auto backup", box, "AUTO_BACKUP" ) );
    checkbox->setToolTip( "Make a backup of the logbook at fixed time schedule" );
    addOptionWidget( checkbox );

    hLayout = new QHBoxLayout();
    hLayout->setSpacing(5);
    hLayout->setMargin(0);
    layout->addLayout( hLayout );

    hLayout->addWidget( new QLabel( "Backup interval (days): ", box ) );
    hLayout->addWidget( spinbox = new OptionSpinBox( box, "BACKUP_ITV" ) );
    hLayout->addStretch(1);
    spinbox->setMinimum( 0 );
    spinbox->setMaximum( 365 );
    spinbox->setToolTip( "Time interval between two backups" );
    addOptionWidget( spinbox );

    checkbox->setChecked( false );
    spinbox->setEnabled( false );
    connect( checkbox, SIGNAL( toggled( bool ) ), spinbox, SLOT( setEnabled( bool ) ) );

    // spelling
    #if WITH_ASPELL
    page = &addPage( "Spell checking", "Spell checking configuration" );
    SpellCheckConfiguration* spell_config = new SpellCheckConfiguration( page );
    page->layout()->addWidget( spell_config );
    addOptionWidget( spell_config );
    #endif

    // edition
    textEditConfiguration();

    // printing
    page = &addPage( "Printing", "Logbook and logbook entries printing configuration" );

    hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(5);
    page->layout()->addItem( hLayout );

    hLayout->addWidget( box = new QGroupBox( "Logbook", page ) );
    box->setLayout( new QVBoxLayout() );
    LogbookPrintOptionWidget* logbookPrintOptionWidget = new LogbookPrintOptionWidget( box );
    box->layout()->addWidget( logbookPrintOptionWidget );
    addOptionWidget( logbookPrintOptionWidget );

    hLayout->addWidget( box = new QGroupBox( "Logbook entries", page ) );
    box->setLayout( new QVBoxLayout() );
    LogEntryPrintOptionWidget* logEntryPrintOptionWidget = new LogEntryPrintOptionWidget( box );
    box->layout()->addWidget( logEntryPrintOptionWidget );
    addOptionWidget( logEntryPrintOptionWidget );

    // recent files
    page = &addPage( "Recent files", "Recent files list settings", true );
    RecentFilesConfiguration* recentFilesConfiguration = new RecentFilesConfiguration( page, Singleton::get().application<Application>()->recentFiles() );
    page->layout()->addWidget( recentFilesConfiguration );
    addOptionWidget( recentFilesConfiguration );

    // misc
    page = &addPage( "Misc", "Additional unsorted settings" );

    // server configuration
    SERVER::ServerConfiguration* server_configuration;
    page->layout()->addWidget( server_configuration = new SERVER::ServerConfiguration( page, "Server configuration" ));
    addOptionWidget( server_configuration );

    box = new QGroupBox( "Misc", page );
    gridLayout = new GridLayout();
    gridLayout->setSpacing(5);
    gridLayout->setMargin(5);
    gridLayout->setMaxCount( 2 );
    box->setLayout( gridLayout );
    page->layout()->addWidget( box );

    gridLayout->addWidget( new QLabel( "Maximum number of stored recent entries: ", box ) );
    gridLayout->addWidget( spinbox = new OptionSpinBox( box, "MAX_RECENT_ENTRIES" ) );
    spinbox->setToolTip( "Maximum number of entries that appear in the <i>Recent Entries</i> menu." );
    addOptionWidget( spinbox );

    gridLayout->addWidget( checkbox = new OptionCheckBox( "Case sensitive text/entry finding", box, "CASE_SENSITIVE" ), 1, 0, 1, 2 );
    checkbox->setToolTip( "Toggle case sensitive text search" );
    addOptionWidget( checkbox );

    // load initial configuration
    _read();

}
