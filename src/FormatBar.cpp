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
  \file FormatBar.cpp
  \brief text formating bar
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QApplication>
#include <QPainter>
#include <QStylePainter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextFragment>
#include <list>

#include "BaseIcons.h"
#include "ColorMenu.h"
#include "CustomPixmap.h"
#include "Debug.h"
#include "FormatBar.h"
#include "IconEngine.h"
#include "PixmapEngine.h"
#include "TextEditor.h"
#include "TextPosition.h"

using namespace std;

//________________________________________
const std::string FormatBar::BOLD_ICON = "text_bold.png";
const std::string FormatBar::ITALIC_ICON = "text_italic.png";
const std::string FormatBar::STRIKE_ICON = "text_strike.png";
const std::string FormatBar::UNDERLINE_ICON = "text_under.png";

//________________________________________
FormatBar::FormatBar( QWidget* parent, const std::string& option_name ):
  CustomToolBar( "Text format", parent, option_name ),
  editor_(0),
  enabled_( true )
{
  
  Debug::Throw( "ToolBar::ToolBar.\n" );

  // bold 
  QAction* action;
  addAction( action = new QAction( IconEngine::get( BOLD_ICON ), "&Bold", this ) );
  action->setCheckable( true );
  actions_.insert( make_pair( BOLD, action ) );
  connect( action, SIGNAL( toggled( bool ) ), SLOT( _bold( bool ) ) );
  
  // underline 
  addAction( action = new QAction( IconEngine::get( ITALIC_ICON ), "&Italic", this ) );
  action->setCheckable( true );
  actions_.insert( make_pair( ITALIC, action ) );
  connect( action, SIGNAL( toggled( bool ) ), SLOT( _italic( bool ) ) );

  // underline 
  addAction( action = new QAction( IconEngine::get( UNDERLINE_ICON ), "&Underline", this ) );
  action->setCheckable( true );
  actions_.insert( make_pair( UNDERLINE, action ) );
  connect( action, SIGNAL( toggled( bool ) ), SLOT( _underline( bool ) ) );

  // strike 
  addAction( action = new QAction( IconEngine::get( STRIKE_ICON ), "&Strike", this ) );
  action->setCheckable( true );
  actions_.insert( make_pair( STRIKE, action ) );
  connect( action, SIGNAL( toggled( bool ) ), SLOT( _strike( bool ) ) );
 
  // color
  action = new QAction( IconEngine::get( ICONS::COLOR ), "&Color", this );
  connect( action, SIGNAL( triggered() ), SLOT( _lastColor() ) );
  actions_.insert( make_pair( COLOR, action ) );

  // color menu
  color_menu_ = new ColorMenu( this );
  action->setMenu( color_menu_ );
  connect( color_menu_, SIGNAL( selected( QColor ) ), SLOT( _color( QColor ) ) );
  action->setMenu( color_menu_ );
  
  // color button
  FormatColorButton *button( new FormatColorButton( this ) );
  addWidget( button );
  button->setDefaultAction( action );
  button->setPopupMode( QToolButton::DelayedPopup );
  connect( color_menu_, SIGNAL( selected( QColor ) ), button, SLOT( setColor( QColor ) ) );
  
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( saveConfiguration() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();
  
}

//________________________________________
void FormatBar::setTarget( TextEditor& editor )
{
  
  Debug::Throw() << "FormatBar::setTarget - key: " << editor.key() << endl;
  editor_ = &editor;
  
  // first update
  updateState( editor_->currentCharFormat() );
  Debug::Throw( "FormatBar::setTarget - done.\n" );
  
}

//________________________________________
void FormatBar::load( const FORMAT::TextFormatBlock::List& format_list ) const
{
  
  Debug::Throw( "FormatBar::loadFormats.\n" );
  assert( editor_ );
  QTextCursor cursor( editor_->document() );
  cursor.beginEditBlock();
  for( FORMAT::TextFormatBlock::List::const_iterator iter = format_list.begin(); iter != format_list.end(); iter++ )
  {
        
    // check if paragraphs are set to 0 or not. If non 0, need to convert to absolute index
    TextPosition begin( iter->parBegin(), iter->begin() );
    int index_begin = iter->parBegin() ? editor_->indexFromPosition( begin ) : iter->begin();

    TextPosition end( iter->parEnd(), iter->end() );
    int index_end = iter->parEnd() ? editor_->indexFromPosition( end ) : iter->end();
    
    // define cursor
    cursor.setPosition( index_begin, QTextCursor::MoveAnchor );
    cursor.setPosition( index_end, QTextCursor::KeepAnchor );
    
    // define format
    QTextCharFormat text_format;
    text_format.setFontWeight( iter->format() & FORMAT::BOLD ? QFont::Bold : QFont::Normal );
    text_format.setFontItalic( iter->format() & FORMAT::ITALIC );
    text_format.setFontUnderline( iter->format() & FORMAT::UNDERLINE );
    text_format.setFontStrikeOut( iter->format() & FORMAT::STRIKE );
    text_format.setFontOverline( iter->format() & FORMAT::OVERLINE );
    
    // load color
    if( iter->color() != ColorMenu::NONE )
    { text_format.setForeground( QColor( iter->color().c_str() ) ); }
    
    cursor.setCharFormat( text_format );
    
  }
  
  cursor.endEditBlock();
  
}
 //________________________________________
FORMAT::TextFormatBlock::List FormatBar::get( void ) const
{
  Debug::Throw( "FormatBar::get.\n" );
  assert( editor_ );
  
  FORMAT::TextFormatBlock::List out;
  
  // iterator over blocks
  for( QTextBlock block = editor_->document()->begin(); block.isValid(); block = block.next() )
  {
    
    // iterator over text fragments
    for( QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it) 
    {
      QTextFragment fragment = it.fragment();
      if (fragment.isValid())    
      {
        
        // retrieve fragments position
        int begin( fragment.position() );
        int end( fragment.position() + fragment.length() );
        
        // retrieve text format
        QTextCharFormat text_format( fragment.charFormat() );
        unsigned int format( FORMAT::DEFAULT );
        if( text_format.fontWeight() == QFont::Bold ) format |= FORMAT::BOLD;
        if( text_format.fontItalic() ) format |= FORMAT::ITALIC;
        if( text_format.fontUnderline() ) format |= FORMAT::UNDERLINE;
        if( text_format.fontStrikeOut() ) format |= FORMAT::STRIKE;
        if( text_format.fontOverline() ) format |= FORMAT::OVERLINE;
        
        // retrieve text color
        QColor color( text_format.foreground().color() );
        string colorname = (color == editor_->palette().color( QPalette::Text ) ) ? ColorMenu::NONE:qPrintable( color.name() );
        
        // skip format if corresponds to default
        if( format == FORMAT::DEFAULT && color == editor_->palette().color( QPalette::Text ) ) continue;
        
        // store new TextFormatBlock
        out.push_back( FORMAT::TextFormatBlock( begin, end, format, colorname ) );
        
      }
    }
  }
  
  return out;
}
  
//________________________________________
void FormatBar::_updateConfiguration( void )
{
  Debug::Throw( "FormatBar::_updateConfiguration.\n" );
  // retrieve colors from options
  list<string> text_colors( XmlOptions::get().specialOptions<string>( "TEXT_COLOR" ) );
  if( text_colors.empty() ) 
  {
    
    // add default colors
    string default_colors[] = 
    { 
      "None",
      "#aa0000",
      "green",
      "blue",
      "grey",
      "black",
      "" 
    };
    
    XmlOptions::get().keep( "TEXT_COLOR" );
    for( unsigned int i=0; default_colors[i].size(); i++ )
    {
      XmlOptions::get().add( Option( "TEXT_COLOR", default_colors[i] , "text color"  ));
      color_menu_->add( default_colors[i] );
    }
        
  } else {
    
    for( list<string>::iterator iter = text_colors.begin(); iter != text_colors.end(); iter++ )
    { color_menu_->add( *iter ); }

  }
}
  

//________________________________________
void FormatBar::_saveConfiguration( void )
{
  Debug::Throw( "FormatBar::_saveConfiguration.\n" );
  XmlOptions::get().keep( "TEXT_COLOR" );
  
  const ColorMenu::ColorSet colors( color_menu_->colors() );
  for( ColorMenu::ColorSet::const_iterator iter = colors.begin(); iter != colors.end(); iter++ )
  { XmlOptions::get().add( Option( "TEXT_COLOR", qPrintable( iter->name() ) , "text color"  )); }

  return;
}
    
//________________________________________
void FormatBar::_bold( bool state )
{
  Debug::Throw( "FormatBar::_bold.\n" );
  if( editor_ && enabled_ ) 
  {
    QTextCharFormat format;
    format.setFontWeight( state ? QFont::Bold : QFont::Normal );
    editor_->mergeCurrentCharFormat( format );
  }
}
  
//________________________________________
void FormatBar::_italic( bool state )
{
  Debug::Throw( "FormatBar::_italic.\n" );
  if( editor_ && enabled_ ) 
  {
    QTextCharFormat format;
    format.setFontItalic( state );
    editor_->mergeCurrentCharFormat( format );
  }
}
  
//________________________________________
void FormatBar::_underline( bool state )
{
  Debug::Throw( "FormatBar::_underline.\n" );
  if( editor_ && enabled_ ) 
  {
    QTextCharFormat format;
    format.setFontUnderline( state );
    editor_->mergeCurrentCharFormat( format );
  }
}

//________________________________________
void FormatBar::_strike( bool state )
{
  Debug::Throw( "FormatBar::_strike.\n" );
  if( editor_ && enabled_ ) 
  {
    QTextCharFormat format;
    format.setFontStrikeOut( state );
    editor_->mergeCurrentCharFormat( format );
  }
}

//________________________________________
void FormatBar::_color( QColor color )
{
  Debug::Throw( "FormatBar::_color.\n" );
  if( editor_ && enabled_ ) 
  {
    QTextCharFormat format;
    format.setForeground( color );
    editor_->mergeCurrentCharFormat( format );
  }
}

//______________________________________
void FormatBar::_lastColor( void )
{ 
  Debug::Throw( "FormatBar::_lastColor.\n" );
  _color( color_menu_->lastColor() ); 
}
  
//________________________________________
void FormatBar::updateState( const QTextCharFormat& format )
{
  Debug::Throw( "FormatBar::updateState.\n" );
  enabled_ = false;
  actions_[BOLD]->setChecked( format.fontWeight() == QFont::Bold );
  actions_[ITALIC]->setChecked( format.fontItalic() );
  actions_[UNDERLINE]->setChecked( format.fontUnderline() );
  actions_[STRIKE]->setChecked( format.fontStrikeOut() );
  enabled_ = true;
}
  
//________________________________________
void FormatColorButton::paintEvent( QPaintEvent* event )
{
  
  // default handling if color is invalid
  QToolButton::paintEvent( event );
  if( !color_.isValid() ) return;
  
  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing );
  
  QLinearGradient gradient( rect().topLeft(), rect().bottomLeft() );

  QColor first( color_ );
  first.setAlpha( 50 );
  gradient.setColorAt(0, first );
  
  QColor second( color_ );
  gradient.setColorAt(1, second );

  
  QPen pen;
  pen.setWidth( 2 );
  // pen.setBrush( color_ );
  pen.setBrush( gradient );
  pen.setJoinStyle( Qt::RoundJoin );
  painter.setPen( pen );
  painter.setBrush( Qt::transparent );
  
  
  painter.drawRoundedRect( rect().adjusted( 1, 1, -1, -1 ), 5, 5 );
  // painter.drawRect( rect().adjusted( 1, 1, -1, -1 ) );
  painter.end();
  return;
  
}
