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
#include <QTextBlock>
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
  editor_(0)
{
  
  Debug::Throw( "ToolBar::ToolBar.\n" );

  // generic button
  CustomToolButton *button;
  
  // bold 
  button = new CustomToolButton( this, IconEngine::get( BOLD_ICON ), "change current font to bold" );
  button->setText("Bold");
  button->setCheckable( true );
  buttons_.insert( make_pair( BOLD, button ) );
  connect( button, SIGNAL( clicked() ), SLOT( _bold() ) );
  addWidget( button );
  
  // underline 
  button = new CustomToolButton( this, IconEngine::get( ITALIC_ICON ), "change current font to italic" );
  button->setText("Italic");
  button->setCheckable( true );
  buttons_.insert( make_pair( ITALIC, button ) );
  connect( button, SIGNAL( clicked() ), SLOT( _italic() ) );
  addWidget( button );

  // underline 
  button = new CustomToolButton( this, IconEngine::get( UNDERLINE_ICON ), "change current font to underline" );
  button->setText("Underline");
  button->setCheckable( true );
  buttons_.insert( make_pair( UNDERLINE, button ) );
  connect( button, SIGNAL( clicked() ), SLOT( _underline() ) );
  addWidget( button );

  // strike 
  button = new CustomToolButton( this, IconEngine::get( STRIKE_ICON ), "change current font to strike" );
  button->setText("Strike");
  button->setCheckable( true );
  buttons_.insert( make_pair( STRIKE, button ) );
  connect( button, SIGNAL( clicked() ), SLOT( _strike() ) );
  addWidget( button );
 
  // color
  button = new CustomToolButton( this, IconEngine::get( ICONS::COLOR ), "change current font color" );
  button->setText("Text color");
  buttons_.insert( make_pair( COLOR, button ) );
  addWidget( button );
  
  // color menu
  color_menu_ = new ColorMenu( button );
  button->setMenu( color_menu_ );
  connect( color_menu_, SIGNAL( selected( QColor ) ), SLOT( _updateColorPixmap( QColor ) ) );
  connect( color_menu_, SIGNAL( selected( QColor ) ), SLOT( _color( QColor ) ) );
  connect( button, SIGNAL( pressed() ), SLOT( _lastColor() ) );
  _updateColorPixmap();
    
  // configuration
  connect( qApp, SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), SLOT( _saveConfiguration() ) );
  _updateConfiguration();
  
}

//________________________________________
void FormatBar::setTarget( TextEditor& editor )
{
  
  Debug::Throw() << "FormatBar::setTarget - key: " << editor.key() << endl;

//   if( editor_ )
//   { 
//     Debug::Throw( "FormatBar::setTarget - disconnecting old target\n" );
//     disconnect( editor_, SIGNAL( currentCharFormatChanged( const QTextCharFormat& ) ), this, SLOT( _updateState( const QTextCharFormat& ) ) ); 
//   }

  editor_ = &editor;
  Debug::Throw( "FormatBar::setTarget - editor updated\n" );

  connect( editor_, 
    SIGNAL( currentCharFormatChanged( const QTextCharFormat& ) ), 
    SLOT( _updateState( const QTextCharFormat& ) ) );
  
  Debug::Throw( "FormatBar::setTarget - connection set\n" );
  
  // first update
  _updateState( editor_->currentCharFormat() );
  Debug::Throw( "FormatBar::setTarget - done.\n" );
  
}

//________________________________________
void FormatBar::load( const FORMAT::TextFormatBlock::List& format_list ) const
{
  
  Debug::Throw( "FormatBar::loadFormats.\n" );
  assert( editor_ );
  for( FORMAT::TextFormatBlock::List::const_iterator iter = format_list.begin(); iter != format_list.end(); iter++ )
  {
        
    // check if paragraphs are set to 0 or not. If non 0, need to convert to absolute index
    TextPosition begin( iter->parBegin(), iter->begin() );
    int index_begin = iter->parBegin() ? editor_->indexFromPosition( begin ) : iter->begin();

    TextPosition end( iter->parEnd(), iter->end() );
    int index_end = iter->parEnd() ? editor_->indexFromPosition( end ) : iter->end();
    
    // define cursor
    QTextCursor cursor( editor_->document() );
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
void FormatBar::_bold( void )
{
  Debug::Throw( "FormatBar::_bold.\n" );
  if( editor_ ) editor_->setFontWeight( buttons_[BOLD]->isChecked() ? QFont::Bold : QFont::Normal);
}
  
//________________________________________
void FormatBar::_italic( void )
{
  Debug::Throw( "FormatBar::_italic.\n" );
  if( editor_ ) editor_->setFontItalic(buttons_[ITALIC]->isChecked());
}
  
//________________________________________
void FormatBar::_underline( void )
{
  Debug::Throw( "FormatBar::_underline.\n" );
  if( editor_ ) editor_->setFontUnderline(buttons_[UNDERLINE]->isChecked());
}

//________________________________________
void FormatBar::_strike( void )
{
  Debug::Throw( "FormatBar::_strike.\n" );
  if( editor_ ) 
  {
    // strike out fonts cannot be set directly in the editor
    // one must retrieve the current font, strike it and reassign
    QFont font( editor_->currentFont() );
    font.setStrikeOut(buttons_[STRIKE]->isChecked());
    editor_->setCurrentFont( font );
  }
}

//________________________________________
void FormatBar::_color( QColor color )
{
  Debug::Throw( "FormatBar::_color.\n" );
  if( !editor_ ) return;
  editor_->setTextColor( color.isValid() ? color:editor_->palette().color( QPalette::Text ) );
}

//______________________________________
void FormatBar::_lastColor( void )
{ 
  Debug::Throw( "FormatBar::_lastColor.\n" );
  _color( color_menu_->lastColor() ); 
}
  
//________________________________________
void FormatBar::_updateState( const QTextCharFormat& format )
{
  Debug::Throw( "FormatBar::_updateState.\n" );
  buttons_[BOLD]->setChecked( format.fontWeight() == QFont::Bold );
  buttons_[ITALIC]->setChecked( format.fontItalic() );
  buttons_[UNDERLINE]->setChecked( format.fontUnderline() );
  buttons_[STRIKE]->setChecked( format.fontStrikeOut() );
}
  
//________________________________________
void FormatBar::_updateColorPixmap( QColor color )
{
  Debug::Throw( "FormatBar::_updateColorPixmap.\n" );
  
  // retrieve button
  CustomToolButton* button( buttons_[COLOR] );
  assert( button );

  
  QPixmap base( PixmapEngine::get( ICONS::COLOR ) );
  assert( !base.isNull() );
  
  // create new empty pixmap
  static const double ratio = 1.15;
  QSize size( base.width(), int( ratio*base.height() ) );
  CustomPixmap new_pixmap = CustomPixmap().empty( size );
  QPainter painter( &new_pixmap );
  painter.drawPixmap( QPoint(0, 0), base, base.rect() );
  
  if( color.isValid() )
  {
    QLinearGradient gradient(QPointF(0, 0), QPointF( base.width(), int( 0.4*base.height() ) ) );
    gradient.setColorAt(0, color);
    gradient.setColorAt(1, color.light(135));
    
    painter.setPen( Qt::NoPen );
    painter.setBrush( gradient );
    painter.drawRect( QRectF( 0, base.height(), base.width(), (ratio-1)*base.height() ) );
  }
  painter.end();
  
  button->setIcon( new_pixmap.scaleHeight( button->iconSize().height() ) );
  
  
}
