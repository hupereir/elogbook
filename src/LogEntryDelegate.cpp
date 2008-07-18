
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
  \file    LogEntryDelegate.cpp
  \brief   LogEntry item delegate for edition
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QAbstractItemModel>

#include "LogEntryDelegate.h"
#include "LineEditor.h"
#include "Debug.h"

using namespace std;

//______________________________________________________________
LogEntryDelegate::LogEntryDelegate( QObject *parent ):
  QItemDelegate( parent ),
  Counter( "LogEntryDelegate" )
{ Debug::Throw( "LogEntryDelegate::LogEntryDelegate.\n" ); }

//______________________________________________________________
QWidget* LogEntryDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{ 
  Debug::Throw( "LogEntryDelegate::createEditor.\n" );
  LineEditor *editor = new LineEditor( parent ); 
  editor->setFrame( false );
  return editor;
}

//______________________________________________________________
void LogEntryDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  Debug::Throw( "LogEntryDelegate::setEditorData.\n" );
  QString text = index.model()->data(index, Qt::DisplayRole).toString();
  LineEditor *line_editor = static_cast<LineEditor*>(editor);
  line_editor->setText( text );
}

//______________________________________________________________
void LogEntryDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  Debug::Throw( "LogEntryDelegate::setModelData.\n" );
  LineEditor *line_editor = static_cast<LineEditor*>(editor);
  QString value( line_editor->text() );
  model->setData( index, value, Qt::EditRole);
}

//______________________________________________________________
void LogEntryDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{ 
  Debug::Throw( "LogEntryDelegate::updateEditorGeometry.\n" );
  editor->setGeometry(option.rect);
}
