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

#include "Debug.h"
#include "IconNames.h"
#include "IconEngine.h"
#include "IconSizeMenu.h"
#include "LineEditor.h"
#include "MainWindow.h"
#include "QtUtil.h"
#include "SearchPanel.h"
#include "Singleton.h"
#include "ToolBarMenu.h"
#include "ToolButtonStyleMenu.h"
#include "XmlOptions.h"

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

//___________________________________________________________
SearchPanel::SearchPanel( const QString& title, QWidget* parent, const QString& optionName ):
    CustomToolBar( title, parent, optionName )
{
    Debug::Throw( "SearchPanel::SearchPanel.\n" );

    // find selection button
    QPushButton* button;
    addWidget( button = new QPushButton( IconEngine::get( IconNames::Find ), tr( "Find" ), this ) );
    connect( button, SIGNAL(clicked()), SLOT(_selectionRequest()) );
    button->setToolTip( tr( "Find logbook entries matching selected text" ) );
    button->setEnabled( false );
    findButton_ = button;

    // selection text
    editor_ = new CustomComboBox( this );
    editor_->setEditable( true );
    editor_->setAutoCompletion( true );
    editor_->setToolTip( tr( "Text to be found in logbook" ) );
    editor_->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    addWidget( editor_ );
    connect( editor_, SIGNAL(activated(QString)), SLOT(_selectionRequest()) );
    connect( editor_, SIGNAL(editTextChanged(QString)), SLOT(_updateFindButton(QString)) );

    addWidget( new QLabel( " in ", this ) );

    addWidget( checkboxes_[Title] = new QCheckBox( tr( "Title" ), this ) );
    addWidget( checkboxes_[Keyword] = new QCheckBox( tr( "Keyword" ), this ) );
    addWidget( checkboxes_[Text]  = new QCheckBox( tr( "Text" ), this ) );
    addWidget( checkboxes_[Attachment] = new QCheckBox( tr( "Attachment" ), this ) );
    addWidget( checkboxes_[Color] = new QCheckBox( tr( "Color" ), this ) );
    checkboxes_[Text]->setChecked( true );

    for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter !=checkboxes_.end(); ++iter )
    { connect( iter.value(), SIGNAL(toggled(bool)), SLOT(_saveMask()) ); }

    // show_all button
    addWidget( button = new QPushButton( tr( "Show All" ), this ) );
    connect( button, SIGNAL(clicked()), SIGNAL(showAllEntries()) );
    connect( button, SIGNAL(clicked()), SLOT(_disableAllEntriesButton()) );
    button->setToolTip( tr( "Show all logbook entries" ) );
    button->setEnabled( false );
    allEntriesButton_ = button;

    connect( findButton_, SIGNAL(clicked()), SLOT(_enableAllEntriesButton()) );
    connect( editor_, SIGNAL(activated(QString)), SLOT(_enableAllEntriesButton()) );

    // close button
    QAction* hideAction;
    addAction( hideAction = new QAction( IconEngine::get( IconNames::DialogClose ), tr( "Close" ), this ) );
    connect( hideAction, SIGNAL(triggered()), SLOT(hide()) );
    connect( hideAction, SIGNAL(triggered()), SIGNAL(showAllEntries()) );
    hideAction->setToolTip( tr( "Show all entries and hide search bar" ) );

    // configuration
    connect( Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    _updateConfiguration();

}

//_______________________________________________________________
void SearchPanel::_toggleVisibility( bool state )
{

    Debug::Throw( "SearchPanel::_toggleVisibility.\n" );
    CustomToolBar::_toggleVisibility( state );
    if( state ) {
        editor_->lineEdit()->selectAll();
        editor_->setFocus();
    }

}

//___________________________________________________________
void SearchPanel::_updateFindButton( const QString& value )
{ findButton_->setEnabled( !value.isEmpty() ); }

//___________________________________________________________
void SearchPanel::_updateConfiguration( void )
{

    Debug::Throw( "SearchPanel::_updateConfiguration.\n" );

    // icon size
    const IconSize iconSize( IconSize( (IconSize::Size)XmlOptions::get().get<int>( "SEARCH_PANEL_ICON_SIZE" ) ) );
    setIconSize( iconSize );

    // text label for toolbars
    Qt::ToolButtonStyle style( (Qt::ToolButtonStyle) XmlOptions::get().get<int>( "SEARCH_PANEL_TEXT_POSITION" ) );
    setToolButtonStyle( style );

    // load mask
    if( XmlOptions::get().contains( "SEARCH_PANEL_MASK" ) )
    {
        unsigned int mask( XmlOptions::get().get<unsigned int>( "SEARCH_PANEL_MASK" ) );
        for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); ++iter )
        { iter.value()->setChecked( mask & iter.key() ); }
    }

}


//____________________________________________________________
void SearchPanel::_updateToolButtonStyle( Qt::ToolButtonStyle style )
{

    Debug::Throw( "SearchPanel::_updateToolButtonStyle.\n" );
    XmlOptions::get().set<int>( "SEARCH_PANEL_TEXT_POSITION", (int)style );
    _updateConfiguration();

}

//____________________________________________________________
void SearchPanel::_updateToolButtonIconSize( IconSize::Size size )
{

    Debug::Throw( "SearchPanel::_updateToolButtonIconSize.\n" );
    XmlOptions::get().set<int>( "SEARCH_PANEL_ICON_SIZE", size );
    _updateConfiguration();

}

//___________________________________________________________
void SearchPanel::_saveMask( void )
{

    Debug::Throw( "SearchPanel::_saveMask.\n" );

    // store mask
    unsigned int mask(0);
    for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); ++iter )
    { if( iter.value()->isChecked() ) mask |= iter.key(); }

    XmlOptions::get().set<unsigned int>( "SEARCH_PANEL_MASK", mask );

}

//___________________________________________________________
void SearchPanel::_selectionRequest( void )
{
    Debug::Throw( "SearchPanel::_selectionRequest.\n" );

    // build mode
    SearchModes mode = None;
    for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); ++iter )
    { if( iter.value()->isChecked() ) mode |= iter.key(); }

    // text selection
    emit selectEntries( editor_->currentText(), mode );

}


//______________________________________________________________________
void SearchPanel::contextMenuEvent( QContextMenuEvent* event )
{
    Debug::Throw( "SearchPanel::_raiseMenu.\n" );

    MainWindow* mainwindow( qobject_cast<MainWindow*>( window() ) );
    if( !mainwindow ) return;
    ToolBarMenu& menu( mainwindow->toolBarMenu() );

    menu.toolButtonStyleMenu().select( (Qt::ToolButtonStyle) XmlOptions::get().get<int>( "SEARCH_PANEL_TEXT_POSITION" ) );
    menu.iconSizeMenu().select( (IconSize::Size) XmlOptions::get().get<int>( "SEARCH_PANEL_ICON_SIZE" ) );

    CustomToolBar::connect( &menu.toolButtonStyleMenu(), SIGNAL(styleSelected(Qt::ToolButtonStyle)), SLOT(_updateToolButtonStyle(Qt::ToolButtonStyle)) );
    CustomToolBar::connect( &menu.iconSizeMenu(), SIGNAL(iconSizeSelected(IconSize::Size)), SLOT(_updateToolButtonIconSize(IconSize::Size)) );

    // move and show menu
    menu.adjustSize();
    menu.exec( event->globalPos() );
    menu.deleteLater();

}
