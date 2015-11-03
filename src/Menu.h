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

/*!
\class Menu
\brief main menu
*/

class Menu:public QMenuBar, public Counter
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* creator
    Menu( QWidget*, MainWindow* );

    //* destructor
    ~Menu( void ) = default;

    //* recent files menu
    RecentFilesMenu& recentFilesMenu( void ) const
    { return *recentFilesMenu_; }

    Q_SIGNALS:

    //* triggered when an entry is selected in recent entries list
    void entrySelected( LogEntry* );

    private Q_SLOTS:

    //* recent entries
    void _updateRecentEntriesMenu( void );

    //* select entry from recent entries menu
    void _selectEntry( QAction* );

    //* get list of editor windows into menu
    void _updateEditorMenu( void );

    //* update preference menu
    void _updatePreferenceMenu( void );

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
