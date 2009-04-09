// $Id$
#ifndef SearchPanel_h
#define SearchPanel_h

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
  \file SearchPanel.h
  \brief selects entries from keyword/title/text/...
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QCheckBox>
#include <QContextMenuEvent>

#include <map>

#include "CustomToolBar.h"
#include "IconSize.h"

class CustomComboBox;
class TransitionWidget;

//! selects entries from keyword/title/text/...
class SearchPanel: public CustomToolBar
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:
      
  //! constructor
  SearchPanel( const QString&, QWidget*, const QString& = QString() );
  
  //! search mode enumeration
  enum SearchMode 
  {
    NONE = 0,
    TITLE = 1<<0,
    KEYWORD = 1<<1,
    TEXT = 1<<2,
    ATTACHMENT = 1<<3,
    COLOR = 1<<4
  };
   
  //! editor
  CustomComboBox& editor( void ) const
  { return *editor_; }

  signals:
  
  //! emitted when the Find button is pressed
  void selectEntries( QString text, unsigned int mode );    

  //! emitted when the Show All button is pressed
  void showAllEntries( void );  

  public slots:
  
  //! show
  virtual void show( void );
  
  //! hide
  virtual void hide( void );
  
  //! visibility
  virtual void setVisible( bool );
  
  protected:
  
  //! context menu
  virtual void contextMenuEvent( QContextMenuEvent* );
 
  //! transition widget
  TransitionWidget& _transitionWidget( void ) const
  { return *transition_widget_; }
  
  protected slots:
  
  //! toggle visibility [overloaded]
  virtual void _toggleVisibility( bool );
   
  private slots:
 
  //! configuration
  void _updateConfiguration( void );
 
  //! toolbar text position
  void _updateToolButtonStyle( Qt::ToolButtonStyle );
 
  //! toolbar text position
  void _updateToolButtonIconSize( IconSize::Size );

  //! save configuration
  void _saveMask( void );
  
  //! send SelectEntries request
  void _selectionRequest( void );

  private:
  
  //! checkboxes
  typedef std::map<SearchMode, QCheckBox* > CheckBoxMap;
  
  //! transition widget
  TransitionWidget* transition_widget_;
  
  //! checkboxes
  CheckBoxMap checkboxes_;
  
  //! select LogEntry according to title
  QCheckBox *title_selection_;  
  
  //! select LogEntry according to key
  QCheckBox *keyword_selection_;    
  
  //! select LogEntry according to text
  QCheckBox *text_selection_;  
  
  //! select LogEntry according to attachment title
  QCheckBox *attachment_selection_;    
  
  //! select LogEntry according to color
  QCheckBox *color_selection_;    
  
  //! selection text widget
  CustomComboBox *editor_; 
  
};
#endif
