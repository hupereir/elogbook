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

#include "ConfigurationDialog.h"

#include "Application.h"
#include "ColorOptionListBox.h"
#include "Debug.h"
#include "GridLayout.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "LogbookPrintOptionWidget.h"
#include "LogEntryPrintOptionWidget.h"
#include "OptionBrowsedLineEditor.h"
#include "OptionCheckBox.h"
#include "OptionSpinBox.h"
#include "Options.h"
#include "RecentFilesConfiguration.h"
#include "Singleton.h"
#include "TreeViewConfiguration.h"

#include <QLabel>
#include <QLayout>
#include <QGroupBox>

//_________________________________________________________
ConfigurationDialog::ConfigurationDialog( QWidget* parent ):
   BaseConfigurationDialog( parent )
{

    Debug::Throw( "ConfigurationDialog::ConfigurationDialog.\n" );
    setWindowTitle( "Configuration - Elogbook" );

    baseConfiguration();

    // generic objects
    QWidget *box;
    OptionCheckBox *checkbox;
    OptionSpinBox* spinbox;

    // attachment editors
    QWidget* page = &addPage( IconEngine::get( IconNames::PreferencesFileAssociations ), tr( "Applications" ), tr( "Third-party applications used to edit attachments" ) );
    page->layout()->addWidget( box = new QGroupBox( tr( "Attachment Editors" ), page ));

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setSpacing(5);
    gridLayout->setMargin(5);
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( gridLayout );

    OptionBrowsedLineEditor *editor;

    gridLayout->addWidget( new QLabel( tr( "Default:" ), box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_UNKNOWN_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( tr( "HTML:" ), box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_HTML_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( tr( "URL:" ), box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_URL_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( tr( "Text:" ), box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_PLAIN_TEXT_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( tr( "Postscript:" ), box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_POSTSCRIPT_ATC" ) );
    addOptionWidget( editor );

    gridLayout->addWidget( new QLabel( tr( "Image:" ), box ) );
    gridLayout->addWidget( editor =  new OptionBrowsedLineEditor( box, "EDIT_IMAGE_ATC" ) );
    addOptionWidget( editor );

    gridLayout->setColumnStretch( 1, 1 );

    gridLayout->addWidget( new QLabel( tr( "Icon size:" ), box ) );
    gridLayout->addWidget( spinbox = new OptionSpinBox( box, "ATTACHMENT_LIST_ICON_SIZE" ) );
    spinbox->setSuffix( tr( "px" ) );
    spinbox->setMinimum( 8 );
    spinbox->setMaximum( 96 );
    spinbox->setToolTip( tr( "Icon size in attachment lists" ) );
    addOptionWidget( spinbox );

    // colors
    page = &addPage( IconEngine::get( IconNames::PreferencesColors ), tr( "Colors" ), tr( "Color settings for entry tagging and text highlighting" ) );
    box = new QGroupBox( tr( "Logbook Entry Colors" ), page );
    box->setLayout( new QVBoxLayout() );
    box->layout()->setMargin(5);
    box->layout()->setSpacing(5);
    page->layout()->addWidget( box );

    OptionListBox* listbox;
    box->layout()->addWidget( listbox = new ColorOptionListBox( box, "COLOR" ) );
    listbox->setToolTip( tr( "Colors used for logbook entry display" ) );
    addOptionWidget( listbox );

    box = new QGroupBox( tr( "Text Colors" ), page );
    box->setLayout( new QVBoxLayout() );
    box->layout()->setMargin(5);
    box->layout()->setSpacing(5);
    page->layout()->addWidget( box );

    box->layout()->addWidget( listbox = new ColorOptionListBox( box, "TEXT_COLOR" ) );
    listbox->setToolTip(tr(  "Colors used for text formatting" ) );
    addOptionWidget( listbox );

    // auto save
    page = &addPage( IconEngine::get( IconNames::PreferencesBackup ), tr( "Backup" ), tr( "Logbook backup configuration" ) );
    gridLayout = new GridLayout();
    gridLayout->setSpacing(5);
    gridLayout->setMargin(0);
    page->layout()->addItem( gridLayout );

    gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Make backup of files when saving modifications" ), page, "FILE_BACKUP" ), 0, 0, 1, 2 );
    checkbox->setToolTip( tr( "Make backup of the file prior to saving modifications" ) );
    addOptionWidget( checkbox );

    gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Automatically save logbook every" ), page, "AUTO_SAVE" ), 1, 0, 1, 1 );
    addOptionWidget( checkbox );

    gridLayout->addWidget( spinbox = new OptionSpinBox( page, "AUTO_SAVE_ITV" ), 1, 1, 1, 1 );
    spinbox->setSuffix( tr( "s" ) );
    spinbox->setMinimum( 0 );
    spinbox->setMaximum( 3600 );
    addOptionWidget( spinbox );

    checkbox->setChecked( false );
    spinbox->setEnabled( false );
    connect( checkbox, SIGNAL(toggled(bool)), spinbox, SLOT(setEnabled(bool)) );

    gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Backup logbook every" ), page, "AUTO_BACKUP" ), 2, 0, 1, 1 );
    addOptionWidget( checkbox );

    gridLayout->addWidget( spinbox = new OptionSpinBox( page, "BACKUP_ITV" ), 2, 1, 1, 1 );
    spinbox->setSuffix( tr( " days" ) );
    spinbox->setMinimum( 0 );
    spinbox->setMaximum( 365 );
    addOptionWidget( spinbox );

    checkbox->setChecked( false );
    spinbox->setEnabled( false );
    connect( checkbox, SIGNAL(toggled(bool)), spinbox, SLOT(setEnabled(bool)) );

    // edition
    textEditConfiguration();

    // printing
    page = &addPage( IconEngine::get( IconNames::PreferencesPrinting ), tr( "Printing" ), tr( "Logbook and logbook entries printing configuration" ) );

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(5);
    page->layout()->addItem( hLayout );

    hLayout->addWidget( box = new QGroupBox( tr( "Logbook" ), page ) );
    box->setLayout( new QVBoxLayout() );
    LogbookPrintOptionWidget* logbookPrintOptionWidget = new LogbookPrintOptionWidget( box );
    box->layout()->addWidget( logbookPrintOptionWidget );
    addOptionWidget( logbookPrintOptionWidget );

    hLayout->addWidget( box = new QGroupBox( tr( "Logbook Entries" ), page ) );
    box->setLayout( new QVBoxLayout() );
    LogEntryPrintOptionWidget* logEntryPrintOptionWidget = new LogEntryPrintOptionWidget( box );
    box->layout()->addWidget( logEntryPrintOptionWidget );
    addOptionWidget( logEntryPrintOptionWidget );

    // recent files
    page = &addPage( IconEngine::get( IconNames::PreferencesRecentFiles ), tr( "Recent Files" ), tr( "Recent files list settings" ), true );
    RecentFilesConfiguration* recentFilesConfiguration = new RecentFilesConfiguration( page, Singleton::get().application<Application>()->recentFiles() );
    page->layout()->addWidget( recentFilesConfiguration );
    addOptionWidget( recentFilesConfiguration );

    recentFilesConfiguration->read();
    connect( this, SIGNAL(ok()), recentFilesConfiguration, SLOT(write()) );
    connect( this, SIGNAL(apply()), recentFilesConfiguration, SLOT(write()) );
    connect( this, SIGNAL(reset()), recentFilesConfiguration, SLOT(reload()) );

    // misc
    page = &addPage( IconEngine::get( IconNames::PreferencesUnsorted ), tr( "Unsorted" ), tr( "Additional unsorted settings" ) );

    box = new QWidget( page );
    gridLayout = new GridLayout();
    gridLayout->setSpacing(5);
    gridLayout->setMargin(0);
    gridLayout->setMaxCount( 2 );
    box->setLayout( gridLayout );
    page->layout()->addWidget( box );

    gridLayout->addWidget( new QLabel( tr( "Maximum number of stored recent entries:" ), box ) );
    gridLayout->addWidget( spinbox = new OptionSpinBox( box, "MAX_RECENT_ENTRIES" ) );
    spinbox->setToolTip( tr( "Maximum number of entries that appear in the <i>Recent Entries</i> menu" ) );
    addOptionWidget( spinbox );

    gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Case sensitive text/entry finding" ), box, "CASE_SENSITIVE" ), 1, 0, 1, 2 );
    checkbox->setToolTip( tr( "Toggle case sensitive text search" ) );
    addOptionWidget( checkbox );

    // load initial configuration
    read();

}
