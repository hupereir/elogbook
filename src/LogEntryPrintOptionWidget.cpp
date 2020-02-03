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


#include "LogEntryPrintOptionWidget.h"
#include "XmlOptions.h"

#include <QLayout>

//_________________________________________________________________
LogEntryPrintOptionWidget::LogEntryPrintOptionWidget( QWidget* parent ):
    QWidget( parent ),
    OptionWidget( "LOGENTRY_PRINT_OPTION_MASK" )
{

    setWindowTitle( tr( "Logbook Entry Configuration" ) );

    auto layout = new QVBoxLayout;
    setLayout( layout );

    // insert checkboxes
    checkBoxes_ =
    {
        { LogEntry::KeywordMask, new QCheckBox( tr( "Keyword" ), this ) },
        { LogEntry::TitleMask, new QCheckBox( tr( "Title" ), this ) },
        { LogEntry::AuthorMask, new QCheckBox( tr( "Author" ), this ) },
        { LogEntry::CreationMask, new QCheckBox( tr( "Creation time" ), this ) },
        { LogEntry::ModificationMask, new QCheckBox( tr( "Last modificaion time" ), this ) },
        { LogEntry::TextMask, new QCheckBox( tr( "Contents" ), this ) },
        { LogEntry::AttachmentsMask, new QCheckBox( tr( "Attachments" ), this ) }
    };

    // insert in layout
    for( auto&& iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    {
        connect( iter.value(), &QAbstractButton::toggled, this, &LogEntryPrintOptionWidget::_updateMask );
        layout->addWidget( iter.value() );
    }

    layout->addStretch(1);

}

//_________________________________________________________________
void LogEntryPrintOptionWidget::read( const Options& options )
{

    int mask( options.get<int>( optionName() ) );
    for( auto&& iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { iter.value()->setChecked( mask&iter.key() ); }

    _setConnected();

}

//_________________________________________________________________
void LogEntryPrintOptionWidget::write( Options& options ) const
{ options.set<int>( optionName(), mask() ); }

//_________________________________________________________________
LogEntry::Mask LogEntryPrintOptionWidget::mask() const
{
    LogEntry::Mask out(0);
    for( auto&& iter = checkBoxes_.begin(); iter != checkBoxes_.end(); iter++ )
    { if( iter.value()->isChecked() ) out |= iter.key(); }

    return out;

}
