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
  \file SearchPanel.cpp
  \brief selects entries from keyword/title/text/...
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLayout>

#include "TransitionWidget.h"
#include "Debug.h"
#include "Icons.h"
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



//___________________________________________________________
SearchPanel::SearchPanel( const QString& title, QWidget* parent, const QString& option_name ):
  CustomToolBar( title, parent, option_name ),
  transitionWidget_( new TransitionWidget(parent) )
{
  Debug::Throw( "SearchPanel::SearchPanel.\n" );

  // find selection button
  QPushButton* button;
  addWidget( button = new QPushButton( IconEngine::get( ICONS::FIND ), "&Find", this ) );
  connect( button, SIGNAL( clicked() ), SLOT( _selectionRequest() ) );
  button->setToolTip( "Find logbook entries matching selected text" );

  // selection text
  editor_ = new CustomComboBox( this );
  editor().setEditable( true );
  editor().setAutoCompletion( true );
  editor().setToolTip( "Text to be found in logbook" );
  editor().setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  addWidget( editor_ );
  connect( &editor(), SIGNAL( activated( const QString& ) ), SLOT( _selectionRequest() ) );

  addWidget( new QLabel( " in ", this ) );

  addWidget( checkboxes_[TITLE] = new QCheckBox( "&Title", this ) );
  addWidget( checkboxes_[KEYWORD] = new QCheckBox( "&Keyword", this ) );
  addWidget( checkboxes_[TEXT]  = new QCheckBox( "&Text", this ) );
  addWidget( checkboxes_[ATTACHMENT] = new QCheckBox( "&Attachment", this ) );
  addWidget( checkboxes_[COLOR] = new QCheckBox( "&Color", this ) );
  checkboxes_[TEXT]->setChecked( true );

  for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter !=checkboxes_.end(); ++iter )
  { connect( iter.value(), SIGNAL( toggled( bool ) ), SLOT( _saveMask() ) ); }

  // show_all button
  addWidget( button = new QPushButton( "&Show All", this ) );
  connect( button, SIGNAL( clicked() ), this, SIGNAL( showAllEntries() ) );
  button->setToolTip( "Show all logbook entries" );

  // close button
  QAction* hide_action;
  addAction( hide_action = new QAction( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Close", this ) );
  connect( hide_action, SIGNAL( triggered() ), SLOT( hide() ) );
  connect( hide_action, SIGNAL( triggered() ), this, SIGNAL( showAllEntries() ) );
  hide_action->setToolTip( "Show all entries and hide search bar." );

  // configuration
  connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  _updateConfiguration();

  // transition widget
  _transitionWidget().setFlag( TransitionWidget::FROM_PARENT, true );
  _transitionWidget().hide();
  connect( &_transitionWidget().timeLine(), SIGNAL( finished() ),  &_transitionWidget(), SLOT( hide() ) );


}

//_______________________________________________________________
void SearchPanel::show( void )
{
  Debug::Throw( "SearchPanel::show.\n" );

  // check transition enability
  if( !( _transitionWidget().isEnabled() && parentWidget()->isVisible() ) ) return CustomToolBar::show();

  // transition
  _transitionWidget().initialize();
  CustomToolBar::show();
  _transitionWidget().start();

}

//_______________________________________________________________
void SearchPanel::hide( void )
{
  Debug::Throw( "SearchPanel::hide.\n" );

  // check transition enability
  if( !( _transitionWidget().isEnabled() && parentWidget()->isVisible() ) )  return CustomToolBar::hide();

  // transition
  _transitionWidget().initialize();
  CustomToolBar::hide();
  _transitionWidget().start();

}

//_______________________________________________________________
void SearchPanel::setVisible( bool state )
{

  Debug::Throw( "SearchPanel::setVisible.\n" );

  // check state and transition enability
  if( state == isVisible() || !( _transitionWidget().isEnabled() && parentWidget()->isVisible() ) )
  { return CustomToolBar::setVisible( state ); }

  // transition
  _transitionWidget().initialize();
  CustomToolBar::setVisible( state );
  _transitionWidget().start();

}

//_______________________________________________________________
void SearchPanel::_toggleVisibility( bool state )
{

  Debug::Throw( "SearchPanel::_toggleVisibility.\n" );
  CustomToolBar::_toggleVisibility( state );
  if( state ) {
    editor().lineEdit()->selectAll();
    editor().setFocus();
  }

}

//___________________________________________________________
void SearchPanel::_updateConfiguration( void )
{

  Debug::Throw( "SearchPanel::_updateConfiguration.\n" );

  // icon size
  IconSize icon_size( IconSize( this, (IconSize::Size)XmlOptions::get().get<int>( "SEARCH_PANEL_ICON_SIZE" ) ) );
  setIconSize( icon_size );

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
  unsigned int mode = NONE;
  for( CheckBoxMap::iterator iter = checkboxes_.begin(); iter != checkboxes_.end(); ++iter )
  { if( iter.value()->isChecked() ) mode |= iter.key(); }

  // text selection
  emit selectEntries( editor().currentText(), mode );

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

  CustomToolBar::connect( &menu.toolButtonStyleMenu(), SIGNAL( styleSelected( Qt::ToolButtonStyle ) ), SLOT( _updateToolButtonStyle( Qt::ToolButtonStyle ) ) );
  CustomToolBar::connect( &menu.iconSizeMenu(), SIGNAL( iconSizeSelected( IconSize::Size ) ), SLOT( _updateToolButtonIconSize( IconSize::Size ) ) );

  // move and show menu
  menu.adjustSize();
  menu.exec( event->globalPos() );
  menu.deleteLater();

}
