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

#ifndef Menu_h
#define Menu_h

/*!
  \file Menu.h
  \brief  main menu
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <cassert>
#include <QMenuBar>


#include "Debug.h"
#include "Counter.h"

class LogEntry;
class MainWindow;
class RecentFilesMenu;

/*!
  \class Menu
  \brief main menu
*/

class Menu:public QMenuBar, public Counter
{

  //! Qt meta object declaration
  Q_OBJECT

  public:

  //! creator
  Menu( QWidget* parent, MainWindow* frame );

  //! destructor
  ~Menu( void )
  { Debug::Throw() << "Menu::~Menu.\n"; }

  //! recent files menu
  RecentFilesMenu& recentFilesMenu( void ) const
  { return *recent_files_menu_; }

  signals:

  //! triggered when an entry is selected in recent entries list
  void entrySelected( LogEntry* );

  private slots:

  //! recent entries
  void _updateRecentEntriesMenu( void );

  //! select entry from recent entries menu
  void _selectEntry( QAction* );

  //! get list of editor windows into menu
  void _updateEditorMenu( void );

  //! update preference menu
  void _updatePreferenceMenu( void );

  private:

  //! recent entries
  QMenu* recent_entries_menu_;

  //! editor windows menu
  QMenu* editor_menu_;

  //! preference menu
  QMenu* preference_menu_;

  //! recent files menu
  RecentFilesMenu* recent_files_menu_;

  //! associates actions and recent entries
  typedef std::map< QAction*, LogEntry* > ActionMap;

  //! associates actions and recent entries
  ActionMap actions_;

};

#endif
