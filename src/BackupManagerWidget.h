#ifndef _BackupManagerWidget_h_
#define _BackupManagerWidget_h_
// $Id$

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public license as published by the Free Software
* Foundation; either version 2 of the license, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public license
* for more details.
*
* You should have received a copy of the GNU General Public license along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "Counter.h"
#include "Key.h"
#include "Logbook.h"
#include "ListModel.h"

#include <QWidget>
#include <QPushButton>
#include <QLayout>

class TreeView;

class BackupManagerWidget: public QWidget, public Counter, public BASE::Key
{

    Q_OBJECT

    public:

    //! constructor
    BackupManagerWidget( QWidget*, Logbook* = 0 );

    //! destructor
    virtual ~BackupManagerWidget( void )
    {}

    //! button layout
    QVBoxLayout& buttonLayout( void )
    { return *buttonLayout_; }

    signals:

    //! emitted when backups are changed (from "clean" action)
    void saveLogbookRequested( void );

    //! emitted when backup is remove
    void removeBackupRequested( Backup );

    //! emitted when backup is restored
    void restoreBackupRequested( Backup );

    //! emmitted when backup is merged
    void mergeBackupRequested( Backup );

    //! emitted when backup is requested
    void backupRequested( void );

    public slots:

    //! update
    void updateBackups( void );

    protected:

    //! get associated logbook
    Logbook* _logbook( void ) const
    {
        BASE::KeySet<Logbook> logbooks( this );
        return logbooks.empty() ? 0L:*logbooks.begin();
    }

    protected slots:

    //! update actions
    void _updateActions( void );

    //! clean
    void _clean( void );

    //! remove backup
    void _remove( void );

    //! restore
    void _restore( void );

    //! merge
    void _merge( void );

    private:

    //! model
    class Model: public ListModel< Backup >, public Counter
    {

        public:

        //! constructor
        Model( void ):
            Counter( "BackupManagerWidget::Model" )
        {}

        //! destructor
        virtual ~Model( void )
        {}

        //! number of columns
        enum { nColumns = 3 };

        //! column type enumeration
        enum ColumnType { FILE, PATH, CREATION };

        //! flags
        virtual Qt::ItemFlags flags( const QModelIndex& ) const;

        //! return data
        virtual QVariant data( const QModelIndex&, int ) const;

        //! header data
        virtual QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const;

        //! number of columns for a given index
        virtual int columnCount( const QModelIndex& = QModelIndex() ) const
        { return nColumns; }

        protected:

        //! sort
        virtual void _sort( int, Qt::SortOrder = Qt::AscendingOrder );

        private:

        //! list column names
        static const QString columnTitles_[nColumns];

        //! used to sort IconCaches
        class SortFTor: public ItemModel::SortFTor
        {

            public:

            //! constructor
            SortFTor( const int& type, Qt::SortOrder order = Qt::AscendingOrder ):
                ItemModel::SortFTor( type, order )
            {}

            //! prediction
            bool operator() ( Backup, Backup ) const;

        };

    };

    //! model
    Model model_;

    //! list
    TreeView* list_;

    //! button layout
    QVBoxLayout* buttonLayout_;

    //!@name buttons
    //@{

    QPushButton* cleanButton_;
    QPushButton* removeButton_;
    QPushButton* restoreButton_;
    QPushButton* mergeButton_;
    QPushButton* newBackupButton_;

    //@}

};

#endif
