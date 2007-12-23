#ifndef LogEntryDelegate_h
#define LogEntryDelegate_h

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
  \file    LogEntryDelegate.h
  \brief   LogEntry item delegate for edition
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <QItemDelegate>
#include "Counter.h"

//! LogEntry item delegate for edition
class LogEntryDelegate : public QItemDelegate, public Counter
{

  public:
  
  //! constructor
  LogEntryDelegate(QObject *parent = 0);
  
  //! create editor
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  
  //! set editor data
  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  
  //! set model data from editor
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
  
  //! editor geometry
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
  
};

#endif
