#ifndef Menu_h
#define Menu_h

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
#include "Counter.h"

#include <QMenuBar>
#include <QHash>

class LogEntry;
class MainWindow;
class RecentFilesMenu;

/**
\class Menu
\brief main menu
*/

class MenuBar:public QMenuBar, private Base::Counter<MenuBar>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    explicit MenuBar( QWidget*, MainWindow* );

    //* recent files menu
    RecentFilesMenu& recentFilesMenu() const
    { return *recentFilesMenu_; }

    Q_SIGNALS:

    //* triggered when an entry is selected in recent entries list
    void entrySelected( LogEntry* );

    private Q_SLOTS:

    //* recent entries
    void _updateRecentEntriesMenu();

    //* select entry from recent entries menu
    void _selectEntry( QAction* );

    //* get list of editor windows into menu
    void _updateEditorMenu();

    //* update preference menu
    void _updatePreferenceMenu();

    private:

    //* recent entries
    QMenu* recentEntriesMenu_ = nullptr;

    //* editor windows menu
    QMenu* windowsMenu_ = nullptr;

    //* preference menu
    QMenu* preferenceMenu_ = nullptr;

    //* recent files menu
    RecentFilesMenu* recentFilesMenu_;

    //* associates actions and recent entries
    using ActionMap = QHash< QAction*, LogEntry* >;

    //* action group
    QActionGroup* actionGroup_ = nullptr;

    //* associates actions and recent entries
    ActionMap actions_;

};

#endif
