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

    Debug::Throw( QStringLiteral("ConfigurationDialog::ConfigurationDialog.\n") );

    baseConfiguration();

    // generic objects
    OptionCheckBox *checkbox = nullptr;
    OptionSpinBox* spinbox = nullptr;

    // edition
    auto page = textEditConfiguration( nullptr, Flag::TabEmulation|Flag::ParagraphHighlight )->parentWidget();
    {
      // misc
      QGroupBox* box;
      page->layout()->addWidget( box = new QGroupBox( tr( "Options" ), page ) );
      QVBoxLayout* layout = new QVBoxLayout;
      box->setLayout( layout );

      OptionCheckBox* checkbox = new OptionCheckBox( tr( "Wrap text " ), box, QStringLiteral("WRAP_TEXT") );
      checkbox->setToolTip( tr( "Turn on/off line wrapping at editor border" ) );
      layout->addWidget( checkbox );
      addOptionWidget( checkbox );

      layout->addWidget( checkbox = new OptionCheckBox( tr( "Show line numbers" ), box, QStringLiteral("SHOW_LINE_NUMBERS") ) );
      checkbox->setToolTip( tr( "Turn on/off display of line numbers" ) );
      addOptionWidget( checkbox );

      layout->addWidget( checkbox = new OptionCheckBox( tr( "Insert hyperlinks automatically" ), box, QStringLiteral("AUTO_INSERT_LINK") ) );
      addOptionWidget( checkbox );

      QLabel* label;
      OptionSpinBox* spinbox;
      QHBoxLayout* hLayout = new QHBoxLayout;
      layout->addLayout( hLayout );
      hLayout->setMargin(0);
      hLayout->addWidget( label = new QLabel( tr( "Automatically hide mouse cursor after: " ), box ) );
      hLayout->addWidget( spinbox = new OptionSpinBox( box, QStringLiteral("AUTOHIDE_CURSOR_DELAY") ) );
      spinbox->setSuffix( tr( "s" ) );
      addOptionWidget( spinbox );

      spinbox->setSpecialValueText( tr( "Never" ) );
      spinbox->setMinimum( 0 );
      spinbox->setMaximum( 10 );

    }

    // saving and backup
    page = &addPage( IconEngine::get( IconNames::PreferencesBackup ), tr( "Saving and Backup" ), tr( "Logbook saving and backup configuration" ) );
    {
        auto gridLayout = new GridLayout;
        gridLayout->setSpacing(5);
        gridLayout->setMargin(0);
        page->layout()->addItem( gridLayout );

        int row = 0;
        gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Compress logbook files" ), page, QStringLiteral("USE_COMPRESSION") ), row++, 0, 1, 2 );
        addOptionWidget( checkbox );

        gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Make backup of files when saving modifications" ), page, QStringLiteral("FILE_BACKUP") ), row++, 0, 1, 2 );
        checkbox->setToolTip( tr( "Make backup of the file prior to saving modifications" ) );
        addOptionWidget( checkbox );

        gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Automatically save logbook every" ), page, QStringLiteral("AUTO_SAVE") ), row, 0, 1, 1 );
        addOptionWidget( checkbox );

        gridLayout->addWidget( spinbox = new OptionSpinBox( page, QStringLiteral("AUTO_SAVE_ITV") ), row++, 1, 1, 1 );
        spinbox->setSuffix( tr( "s" ) );
        spinbox->setMinimum( 0 );
        spinbox->setMaximum( 3600 );
        addOptionWidget( spinbox );

        checkbox->setChecked( false );
        spinbox->setEnabled( false );
        connect( checkbox, &QAbstractButton::toggled, spinbox, &QWidget::setEnabled );

        gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Backup logbook every" ), page, QStringLiteral("AUTO_BACKUP") ), row, 0, 1, 1 );
        addOptionWidget( checkbox );

        gridLayout->addWidget( spinbox = new OptionSpinBox( page, QStringLiteral("BACKUP_ITV") ), row++, 1, 1, 1 );
        spinbox->setSuffix( tr( " days" ) );
        spinbox->setMinimum( 0 );
        spinbox->setMaximum( 365 );
        addOptionWidget( spinbox );

        checkbox->setChecked( false );
        spinbox->setEnabled( false );
        connect( checkbox, &QAbstractButton::toggled, spinbox, &QWidget::setEnabled );

    }


    // printing
    page = &addPage( IconEngine::get( IconNames::PreferencesPrinting ), tr( "Printing" ), tr( "Logbook and logbook entries printing configuration" ) );
    {
        auto hLayout = new QHBoxLayout;
        hLayout->setMargin(0);
        hLayout->setSpacing(5);
        page->layout()->addItem( hLayout );

        auto box = new QGroupBox( tr( "Logbook" ), page );
        hLayout->addWidget( box );
        box->setLayout( new QVBoxLayout );
        LogbookPrintOptionWidget* logbookPrintOptionWidget = new LogbookPrintOptionWidget( box );
        logbookPrintOptionWidget->layout()->setMargin(0);
        box->layout()->addWidget( logbookPrintOptionWidget );
        addOptionWidget( logbookPrintOptionWidget );

        hLayout->addWidget( box = new QGroupBox( tr( "Logbook Entries" ), page ) );
        box->setLayout( new QVBoxLayout );
        LogEntryPrintOptionWidget* logEntryPrintOptionWidget = new LogEntryPrintOptionWidget( box );
        logEntryPrintOptionWidget->layout()->setMargin(0);
        box->layout()->addWidget( logEntryPrintOptionWidget );
        addOptionWidget( logEntryPrintOptionWidget );
    }

    // colors
    page = &addPage( IconEngine::get( IconNames::PreferencesColors ), tr( "Colors" ), tr( "Color settings for entry tagging and text highlighting" ) );
    {
        auto box = new QGroupBox( tr( "Logbook Entry Colors" ), page );
        box->setLayout( new QVBoxLayout );
        box->layout()->setMargin(5);
        box->layout()->setSpacing(5);
        page->layout()->addWidget( box );

        OptionListBox* listbox;
        box->layout()->addWidget( listbox = new ColorOptionListBox( box, QStringLiteral("COLOR") ) );
        listbox->setToolTip( tr( "Colors used for logbook entry display" ) );
        addOptionWidget( listbox );

        box = new QGroupBox( tr( "Text Colors" ), page );
        box->setLayout( new QVBoxLayout );
        box->layout()->setMargin(5);
        box->layout()->setSpacing(5);
        page->layout()->addWidget( box );

        box->layout()->addWidget( listbox = new ColorOptionListBox( box, QStringLiteral("TEXT_COLOR") ) );
        listbox->setToolTip(tr(  "Colors used for text formatting" ) );
        addOptionWidget( listbox );
    }

    // recent files
    page = &addPage( IconEngine::get( IconNames::PreferencesRecentFiles ), tr( "Recent Files" ), tr( "Recent files list settings" ), true );
    {
        auto recentFilesConfiguration = new RecentFilesConfiguration( page, Base::Singleton::get().application<Application>()->recentFiles() );
        page->layout()->addWidget( recentFilesConfiguration );
        addOptionWidget( recentFilesConfiguration );

        recentFilesConfiguration->read();
        connect( this, &ConfigurationDialog::ok, recentFilesConfiguration, QOverload<>::of( &RecentFilesConfiguration::write ) );
        connect( this, &ConfigurationDialog::apply, recentFilesConfiguration, QOverload<>::of( &RecentFilesConfiguration::write ) );
        connect( this, &ConfigurationDialog::reset, recentFilesConfiguration, &RecentFilesConfiguration::reload );
    }

    // misc
    page = &addPage( IconEngine::get( IconNames::PreferencesUnsorted ), tr( "Unsorted" ), tr( "Additional unsorted settings" ) );
    {
        auto box = new QWidget( page );
        auto gridLayout = new GridLayout;
        gridLayout->setSpacing(5);
        gridLayout->setMargin(0);
        gridLayout->setMaxCount( 2 );
        box->setLayout( gridLayout );
        page->layout()->addWidget( box );

        gridLayout->addWidget( new QLabel( tr( "Maximum number of stored recent entries:" ), box ) );
        gridLayout->addWidget( spinbox = new OptionSpinBox( box, QStringLiteral("MAX_RECENT_ENTRIES") ) );
        spinbox->setToolTip( tr( "Maximum number of entries that appear in the <i>Recent Entries</i> menu" ) );
        addOptionWidget( spinbox );

        gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Case sensitive text/entry finding" ), box, QStringLiteral("CASE_SENSITIVE") ), 1, 0, 1, 2 );
        checkbox->setToolTip( tr( "Toggle case sensitive text search" ) );
        addOptionWidget( checkbox );

        gridLayout->addWidget( checkbox = new OptionCheckBox( tr( "Show tooltips" ), box, QStringLiteral("SHOW_TOOLTIPS") ), 2, 0, 1, 2 );
        addOptionWidget( checkbox );
    }

    // load initial configuration
    read();

}
