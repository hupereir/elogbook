#ifndef LogbookStatisticsDialog_h
#define LogbookStatisticsDialog_h
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
  \file LogbookStatisticsDialog.h
  \brief  logbook informations
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include "Counter.h"
#include "CustomDialog.h"
#include "ListModel.h"

class Logbook;

class LogbookStatisticsDialog: public CustomDialog
{

    public:

    //! constructor
    LogbookStatisticsDialog( QWidget* parent, Logbook* logbook  );

    private:

    //! logbook information model
    class Model: public ListModel<Logbook*>, public Counter
    {

        public:

        //! number of columns
        enum { nColumns = 5 };

        //! column type enumeration
        enum ColumnType {
            FILE,
            ENTRIES,
            CREATED,
            MODIFIED
        };

        //! constructor
        Model( QObject* parent = 0 ):
            ListModel<Logbook*>( parent ),
            Counter( "LogbookStatisticsDialog::Model" )
        {}

        //!@name methods reimplemented from base class
        //@{

        //! flags
        virtual Qt::ItemFlags flags(const QModelIndex &index) const
        { return Qt::ItemIsEnabled |  Qt::ItemIsSelectable; }

        //! return data
        virtual QVariant data(const QModelIndex &index, int role) const;

        //! header data
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const
        {
            if( orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < nColumns )
            { return columnTitles_[section]; }

            // return empty
            return QVariant();

        }

        //! number of columns for a given index
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
        { return nColumns; }

        //@}

        protected:

        //! sort
        virtual void _sort( int column, Qt::SortOrder order = Qt::AscendingOrder )
        {}

        //! list column names
        static const QString columnTitles_[nColumns];

    };

    //! model
    Model model_;

};

#endif
