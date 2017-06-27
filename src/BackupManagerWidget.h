#ifndef BackupManagerWidget_h
#define BackupManagerWidget_h

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

#include "Backup.h"
#include "BackupModel.h"
#include "Counter.h"
#include "Key.h"
#include "Logbook.h"
#include "ListModel.h"

#include <QWidget>
#include <QPushButton>
#include <QLayout>

class TreeView;

class BackupManagerWidget: public QWidget, private Base::Counter<BackupManagerWidget>, public Base::Key
{

    Q_OBJECT

    public:

    //* constructor
    explicit BackupManagerWidget( QWidget*, Logbook* = nullptr );

    //* button layout
    QVBoxLayout& buttonLayout()
    { return *buttonLayout_; }

    Q_SIGNALS:

    //* emitted when backups are changed (from clean action)
    void saveLogbookRequested();

    //* emitted when backup is removed
    void removeBackupRequested( Backup );

    //* emitted when backup is removed
    void removeBackupsRequested( Backup::List );

    //* emitted when backup is restored
    void restoreBackupRequested( Backup );

    //* emitted when backup is merged
    void mergeBackupRequested( Backup );

    //* emitted when backup is requested
    void backupRequested();

    public Q_SLOTS:

    //* update
    void updateBackups();

    protected:

    //* get associated logbook
    Logbook* _logbook() const
    {
        Base::KeySet<Logbook> logbooks( this );
        return logbooks.empty() ? 0L:*logbooks.begin();
    }

    protected Q_SLOTS:

    //* update actions
    void _updateActions();

    //* clean
    void _clean();

    //* remove backup
    void _remove();

    //* restore
    void _restore();

    //* merge
    void _merge();

    private:

    //* model
    BackupModel model_;

    //* list
    TreeView* list_ = nullptr;

    //* button layout
    QVBoxLayout* buttonLayout_ = nullptr;

    //*@name buttons
    //@{

    QPushButton* cleanButton_ = nullptr;
    QPushButton* removeButton_ = nullptr;
    QPushButton* restoreButton_ = nullptr;
    QPushButton* mergeButton_ = nullptr;
    QPushButton* newBackupButton_ = nullptr;

    //@}

};

#endif
