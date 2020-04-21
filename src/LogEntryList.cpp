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

#include "LogEntryList.h"

#include "Debug.h"
#include "LogEntryModel.h"
#include "ToolTipWidget.h"


//_______________________________________________________________
LogEntryList::LogEntryList( QWidget* parent ):
    TreeView( parent )
{
    Debug::Throw( QStringLiteral( "LogEntryList::LogEntryList.\n" ) );

    setMouseTracking( true );

    // tooltip widget
    toolTipWidget_ = new ToolTipWidget( this );
    connect( this, &LogEntryList::hovered, this, &LogEntryList::_showToolTip );

}


//________________________________________________________
void LogEntryList::_showToolTip( const QModelIndex& index )
{
    Debug::Throw( QStringLiteral("LogEntryList::_showToolTip.\n") );

    // check index and model
    const auto model = static_cast<LogEntryModel*>( this->model() );
    if( !( model && index.isValid() ) ) toolTipWidget_->hide();
    else {

        // assign item
        toolTipWidget_->setLogEntry( model->get( index ) );

        // rect
        toolTipWidget_->setIndexRect(
            visualRect( index ).
            translated( viewport()->mapToGlobal( QPoint( 0, 0 ) ) ) );

        // show
        toolTipWidget_->showDelayed();
    }
}
