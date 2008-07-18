
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
  \file    KeywordDelegate.cpp
  \brief   Keyword item delegate for edition
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QAbstractItemModel>

#include "KeywordDelegate.h"
#include "LineEditor.h"
#include "Debug.h"

using namespace std;

//______________________________________________________________
KeywordDelegate::KeywordDelegate( QObject *parent ):
  QItemDelegate( parent ),
  Counter( "KeywordDelegate" )
{ Debug::Throw( "KeywordDelegate::KeywordDelegate.\n" ); }

//______________________________________________________________
QWidget* KeywordDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{ 
  Debug::Throw( "KeywordDelegate::createEditor.\n" );
  LineEditor *editor = new LineEditor( parent ); 
  editor->setFrame( false );
  return editor;
}

//______________________________________________________________
void KeywordDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  Debug::Throw( "KeywordDelegate::setEditorData.\n" );
  QString text = index.model()->data(index, Qt::DisplayRole).toString();
  LineEditor *line_editor = static_cast<LineEditor*>(editor);
  line_editor->setText( text );
}

//______________________________________________________________
void KeywordDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  Debug::Throw( "KeywordDelegate::setModelData.\n" );
  LineEditor *line_editor = static_cast<LineEditor*>(editor);
  QString value( line_editor->text() );
  model->setData( index, value, Qt::EditRole);
}

//______________________________________________________________
void KeywordDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{ 
  Debug::Throw( "KeywordDelegate::updateEditorGeometry.\n" );
  editor->setGeometry(option.rect);
}
