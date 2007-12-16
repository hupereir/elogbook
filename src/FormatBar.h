// $Id$
#ifndef _FormatBar_h_
#define _FormatBar_h_
 
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
  \file FormatBar.h
  \brief text formating bar
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <string>
#include <map>
#include <QFont>   
#include <QTextCharFormat>   

#include "Counter.h"
#include "CustomToolBar.h"
#include "CustomToolButton.h"
#include "Exception.h"
#include "TextFormatBlock.h"
#include "TextFormat.h"

class ColorMenu;
class CustomTextEdit;

//! text formating bar
class FormatBar: public CustomToolBar
{

  //! Qt meta object declaration
  Q_OBJECT
  
  public:
  
  //! bold icon name
  static const std::string BOLD_ICON;

  //! italic icon name
  static const std::string ITALIC_ICON;

  //! strike icon name
  static const std::string STRIKE_ICON;

  //! underline icon name
  static const std::string UNDERLINE_ICON;
  
  //! button id enumeration
  enum ButtonId
  {
    BOLD,
    ITALIC,
    STRIKE,
    UNDERLINE,
    COLOR
  };
      
  //! constructor
  FormatBar( QWidget* parent, const std::string& option_name );

  //! destructor
  virtual ~FormatBar( void )
  { Debug::Throw( "FormatBar::~FormatBar.\n" ); }
  
  //! set destination label
  void setToolTipLabel( QLabel* tooltip_label )
  { 
    for( ButtonMap::iterator iter = buttons_.begin(); iter != buttons_.end(); iter++ )
    iter->second->setToolTipLabel( tooltip_label );
  }
          
  //! set target editor
  void setTarget( CustomTextEdit* editor );
  
  //! load text formats
  void load( const FORMAT::TextFormatBlock::List& ) const;
  
  //! get text formats
  FORMAT::TextFormatBlock::List get( void ) const;
  
  private slots:
  
  //! update configuration
  void _updateConfiguration( void );
  
  //! save configuration
  void _saveConfiguration( void );
    
  //! update button state
  void _updateState( const QTextCharFormat& );
    
  //! bold
  void _bold( void );
  
  //! italic
  void _italic( void );
  
  //! underline
  void _underline( void );
  
  //! strike
  void _strike( void );
  
  //! color
  void _color( QColor );
  
  //! last selected color
  void _lastColor( void );
  
  //! modify ColorButton pixmap to change the current color
  void _updateColorPixmap( QColor color = QColor() );
 
  private:
   
  //! target text editor
  CustomTextEdit* editor_;
  
  //! button map
  typedef std::map< ButtonId, CustomToolButton* > ButtonMap;
  
  //! button map
  ButtonMap buttons_; 
      
  //! color menu
  ColorMenu* color_menu_;
    
};

#endif
