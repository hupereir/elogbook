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

#include "SearchWidget.h"

#include "Color.h"
#include "CustomComboBox.h"
#include "Debug.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "Singleton.h"
#include "XmlOptions.h"

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QToolButton>

//___________________________________________________________
SearchWidget::SearchWidget( QWidget* parent ):
    QWidget( parent ),
    Counter( "SearchWidget" )
{
    Debug::Throw( "SearchWidget::SearchWidget.\n" );

    // update palette
    _updateNotFoundPalette();

    // editor layout
    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setMargin(2);
    gridLayout->setSpacing(5);
    setLayout( gridLayout );

    // first row
    QLabel *label = new QLabel( tr( "Text to find:" ), this );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    gridLayout->addWidget( label, 0, 0, 1, 1 );

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(5);
    gridLayout->addLayout( hLayout, 0, 1, 1, 1 );

    // editor
    editor_ = new CustomComboBox( this );
    editor_->setEditable( true );
    editor_->setAutoCompletion( true );
    editor_->setToolTip( tr( " Text to be found in logbook" ) );
    editor_->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    editor_->setAutoCompletion( true, Qt::CaseSensitive );
    editor_->setNavigationEnabled( false );

    hLayout->addWidget( editor_, 1 );

    connect( editor_, SIGNAL(activated(QString)), SLOT(_selectionRequest()) );
    connect( editor_, &QComboBox::editTextChanged, this, &SearchWidget::_updateFindButton );
    connect( editor_->lineEdit(), &QLineEdit::textChanged, this, &SearchWidget::_restorePalette );

    // find selection button
    findButton_ = new QPushButton( IconEngine::get( IconNames::Find ), tr( "Find" ), this );
    findButton_->setToolTip( tr( "Find logbook entries matching selected text" ) );
    findButton_->setEnabled( false );
    hLayout->addWidget( findButton_ );

    connect( findButton_, &QAbstractButton::clicked, this, &SearchWidget::_selectionRequest );

    // show all button
    allEntriesButton_ = new QPushButton( tr( "Show All" ), this );
    allEntriesButton_->setIcon( IconEngine::get( IconNames::DialogCancel ) );
    allEntriesButton_->setToolTip( tr( "Show all logbook entries" ) );
    allEntriesButton_->setEnabled( false );
    hLayout->addWidget( allEntriesButton_ );

    connect( allEntriesButton_, &QAbstractButton::clicked, this, &SearchWidget::showAllEntries );
    connect( allEntriesButton_, &QAbstractButton::clicked, this, &SearchWidget::_disableAllEntriesButton );

    // close button
    QToolButton* closeButton = new QToolButton( this );
    closeButton->setAutoRaise( true );
    closeButton->setIcon( IconEngine::get( IconNames::DialogClose ) );
    closeButton->setText( tr( "Close" ) );
    hLayout->addWidget( closeButton );
    connect( closeButton, &QAbstractButton::clicked,this,  &SearchWidget::showAllEntries );
    connect( closeButton, &QAbstractButton::clicked,this,  &QWidget::hide );

    // second row
    label = new QLabel( tr( "In:" ), this );
    label->setAlignment( Qt::AlignRight|Qt::AlignVCenter );
    gridLayout->addWidget( label, 1, 0, 1, 1 );

    hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(5);
    gridLayout->addLayout( hLayout, 1, 1, 1, 1 );

    // checkboxes
    hLayout->addWidget( checkboxes_[Title] = new QCheckBox( tr( "Title" ), this ) );
    hLayout->addWidget( checkboxes_[Keyword] = new QCheckBox( tr( "Keyword" ), this ) );
    hLayout->addWidget( checkboxes_[Text]  = new QCheckBox( tr( "Text" ), this ) );
    hLayout->addWidget( checkboxes_[Attachment] = new QCheckBox( tr( "Attachment" ), this ) );
    hLayout->addWidget( checkboxes_[Color] = new QCheckBox( tr( "Color" ), this ) );
    hLayout->addStretch(1);

    checkboxes_[Text]->setChecked( true );

    for( auto&& iter = checkboxes_.begin(); iter !=checkboxes_.end(); ++iter )
    { connect( iter.value(), &QAbstractButton::toggled, this, &SearchWidget::_saveMask ); }

    // configuration
    connect( Base::Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    _updateConfiguration();

}

//________________________________________________________________________
void SearchWidget::setText( const QString& text )
{
    editor_->setEditText( text );
    _restorePalette();
}

//________________________________________________________________________
void SearchWidget::matchFound()
{
    allEntriesButton_->setEnabled(true);
    _restorePalette();
}

//________________________________________________________________________
void SearchWidget::noMatchFound()
{
    if( !editor_->currentText().isEmpty() )
    { editor_->setPalette( notFoundPalette_ ); }
}

//________________________________________________________________________
void SearchWidget::changeEvent( QEvent* event )
{
    switch( event->type() )
    {
        case QEvent::PaletteChange:
        _updateNotFoundPalette();
        break;

        default: break;
    }

    return QWidget::changeEvent( event );
}

//___________________________________________________________
void SearchWidget::_restorePalette()
{ editor_->setPalette( palette() ); }

//___________________________________________________________
void SearchWidget::_updateFindButton( const QString& value )
{ findButton_->setEnabled( !value.isEmpty() ); }

//___________________________________________________________
void SearchWidget::_updateConfiguration()
{

    Debug::Throw( "SearchWidget::_updateConfiguration.\n" );

    // load mask
    if( XmlOptions::get().contains( "SEARCH_PANEL_MASK" ) )
    {
        int mask( XmlOptions::get().get<int>( "SEARCH_PANEL_MASK" ) );
        for( auto&& iter = checkboxes_.begin(); iter != checkboxes_.end(); ++iter )
        { iter.value()->setChecked( mask & iter.key() ); }
    }

}

//___________________________________________________________
void SearchWidget::_saveMask()
{

    Debug::Throw( "SearchWidget::_saveMask.\n" );

    // store mask
    int mask(0);
    for( auto&& iter = checkboxes_.begin(); iter != checkboxes_.end(); ++iter )
    { if( iter.value()->isChecked() ) mask |= iter.key(); }

    XmlOptions::get().set<int>( "SEARCH_PANEL_MASK", mask );

}

//___________________________________________________________
void SearchWidget::_selectionRequest()
{
    Debug::Throw( "SearchWidget::_selectionRequest.\n" );

    // build mode
    SearchModes mode = None;
    for( auto&& iter = checkboxes_.begin(); iter != checkboxes_.end(); ++iter )
    { if( iter.value()->isChecked() ) mode |= iter.key(); }

    // text selection
    emit selectEntries( editor_->currentText(), mode );

}

//________________________________________________________________________
void SearchWidget::_updateNotFoundPalette()
{
    notFoundPalette_ = palette();
    notFoundPalette_.setColor( QPalette::Base,
        Base::Color( palette().color( QPalette::Base ) ).merge(
        Qt::red, 0.95 ) );
}
