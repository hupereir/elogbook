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

#include "FormatBar.h"
#include "BaseIcons.h"
#include "ColorMenu.h"
#include "CustomPixmap.h"
#include "Debug.h"
#include "IconEngine.h"
#include "PixmapEngine.h"
#include "RoundedPath.h"
#include "Singleton.h"
#include "TextEditor.h"
#include "TextPosition.h"

#include <QPainter>
#include <QStylePainter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextFragment>

//________________________________________
const QString FormatBar::BOLD_ICON = "format-text-bold.png";
const QString FormatBar::ITALIC_ICON = "format-text-italic.png";
const QString FormatBar::STRIKE_ICON = "format-text-strikethrough.png";
const QString FormatBar::UNDERLINE_ICON = "format-text-underline.png";

//________________________________________
FormatBar::FormatBar( QWidget* parent, const QString& option_name ):
    CustomToolBar( "Text format", parent, option_name ),
    editor_(0),
    enabled_( true )
{

    Debug::Throw( "ToolBar::ToolBar.\n" );

    // bold
    QAction* action;
    addAction( action = new QAction( IconEngine::get( BOLD_ICON ), "&Bold", this ) );
    action->setCheckable( true );
    actions_.insert( Bold, action );
    connect( action, SIGNAL( toggled( bool ) ), SLOT( _bold( bool ) ) );

    // underline
    addAction( action = new QAction( IconEngine::get( ITALIC_ICON ), "&Italic", this ) );
    action->setCheckable( true );
    actions_.insert( Italic, action );
    connect( action, SIGNAL( toggled( bool ) ), SLOT( _italic( bool ) ) );

    // underline
    addAction( action = new QAction( IconEngine::get( UNDERLINE_ICON ), "&Underline", this ) );
    action->setCheckable( true );
    actions_.insert( Underline, action );
    connect( action, SIGNAL( toggled( bool ) ), SLOT( _underline( bool ) ) );

    // strike
    addAction( action = new QAction( IconEngine::get( STRIKE_ICON ), "&Strike", this ) );
    action->setCheckable( true );
    actions_.insert( Strike, action );
    connect( action, SIGNAL( toggled( bool ) ), SLOT( _strike( bool ) ) );

    // color
    action = new QAction( IconEngine::get( ICONS::COLOR ), "&Color", this );
    connect( action, SIGNAL( triggered() ), SLOT( _lastColor() ) );
    actions_.insert( Color, action );

    // color menu
    colorMenu_ = new ColorMenu( this );
    action->setMenu( colorMenu_ );
    connect( colorMenu_, SIGNAL( selected( QColor ) ), SLOT( _color( QColor ) ) );
    action->setMenu( colorMenu_ );

    // color button
    FormatColorButton *button( new FormatColorButton( this ) );
    addWidget( button );
    button->setDefaultAction( action );
    button->setPopupMode( QToolButton::DelayedPopup );
    connect( colorMenu_, SIGNAL( selected( QColor ) ), button, SLOT( setColor( QColor ) ) );

    // configuration
    connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
    connect( Singleton::get().application(), SIGNAL( saveConfiguration() ), SLOT( _saveConfiguration() ) );
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
void FormatBar::load( const FORMAT::TextFormatBlock::List& formatList ) const
{

    Debug::Throw( "FormatBar::loadFormats.\n" );
    Q_CHECK_PTR( editor_ );

    // get base text color name
    const QColor baseTextColor( editor_->palette().color( QPalette::Text ) );

    QTextCursor cursor( editor_->document() );
    cursor.beginEditBlock();
    foreach( const FORMAT::TextFormatBlock& block, formatList )
    {

        // define cursor
        cursor.setPosition( block.begin(), QTextCursor::MoveAnchor );
        cursor.setPosition( block.end(), QTextCursor::KeepAnchor );

        // define format
        QTextCharFormat textFormat;
        textFormat.setFontWeight( block.format() & FORMAT::Bold ? QFont::Bold : QFont::Normal );
        textFormat.setFontItalic( block.format() & FORMAT::Italic );
        textFormat.setFontUnderline( block.format() & FORMAT::Underline );
        textFormat.setFontStrikeOut( block.format() & FORMAT::Strike );
        textFormat.setFontOverline( block.format() & FORMAT::Overline );

        // load color
        if( block.color().isValid() && !(block.color() == baseTextColor) )
        { textFormat.setForeground( block.color() ); }

        // load href
        if( !block.href().isEmpty() )
        { textFormat.setAnchorHref( block.href() ); }

        cursor.setCharFormat( textFormat );

    }

    cursor.endEditBlock();

}
//________________________________________
FORMAT::TextFormatBlock::List FormatBar::get( void ) const
{
    Debug::Throw( "FormatBar::get.\n" );
    Q_CHECK_PTR( editor_ );

    FORMAT::TextFormatBlock::List out;

    // iterator over blocks
    for( QTextBlock block = editor_->document()->begin(); block.isValid(); block = block.next() )
    {

        // iterator over text fragments
        for( QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it)
        {
            QTextFragment fragment = it.fragment();
            if( !fragment.isValid() ) continue;

            // retrieve fragments position
            int begin( fragment.position() );
            int end( fragment.position() + fragment.length() );

            // retrieve text format
            QTextCharFormat textFormat( fragment.charFormat() );

            FORMAT::TextFormatFlags format( FORMAT::Default );
            if( textFormat.fontWeight() == QFont::Bold ) format |= FORMAT::Bold;
            if( textFormat.fontItalic() ) format |= FORMAT::Italic;
            if( textFormat.fontUnderline() ) format |= FORMAT::Underline;
            if( textFormat.fontStrikeOut() ) format |= FORMAT::Strike;
            if( textFormat.fontOverline() ) format |= FORMAT::Overline;

            // retrieve text color
            QColor color( textFormat.foreground().color() );
            if( color == editor_->palette().color( QPalette::Text ) ) color = QColor();

            const QString href( textFormat.anchorHref() );

            // skip format if corresponds to default
            if( format == FORMAT::Default && color == editor_->palette().color( QPalette::Text ) && href.isEmpty() ) continue;

            // store new TextFormatBlock
            FORMAT::TextFormatBlock textFormatBlock( begin, end, format, color );
            if( !href.isEmpty() ) textFormatBlock.setHRef( href );

            out << textFormatBlock;

        }
    }

    return out;
}

//________________________________________
void FormatBar::_updateConfiguration( void )
{
    Debug::Throw( "FormatBar::_updateConfiguration.\n" );

    // retrieve colors from options
    Options::List colors( XmlOptions::get().specialOptions( "TEXT_COLOR" ) );
    if( colors.empty() )
    {

        // add default colors
        QList<QColor> defaultColors;
        defaultColors
            << "#aa0000"
            << "green"
            << "blue"
            << "grey"
            << "black";

        XmlOptions::get().keep( "TEXT_COLOR" );
        foreach( const QColor& color, defaultColors )
        {
            XmlOptions::get().add( "TEXT_COLOR", Option().set<BASE::Color>( color ) );
            colorMenu_->add( color );
        }

    } else {

        foreach( const Option& color, colors )
        { colorMenu_->add( color.get<BASE::Color>() ); }

    }
}


//________________________________________
void FormatBar::_saveConfiguration( void )
{
    Debug::Throw( "FormatBar::_saveConfiguration.\n" );
    XmlOptions::get().keep( "TEXT_COLOR" );

    const ColorMenu::ColorSet colors( colorMenu_->colors() );
    foreach( const BASE::Color& color, colors )
    { XmlOptions::get().add( "TEXT_COLOR", Option().set<BASE::Color>( color ) ); }

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
    _color( colorMenu_->lastColor() );
}

//________________________________________
void FormatBar::updateState( const QTextCharFormat& format )
{
    Debug::Throw( "FormatBar::updateState.\n" );
    enabled_ = false;
    actions_[Bold]->setChecked( format.fontWeight() == QFont::Bold );
    actions_[Italic]->setChecked( format.fontItalic() );
    actions_[Underline]->setChecked( format.fontUnderline() );
    actions_[Strike]->setChecked( format.fontStrikeOut() );
    enabled_ = true;
}

//________________________________________
void FormatColorButton::paintEvent( QPaintEvent* event )
{
    if( color_.isValid() )
    {

        QPainter painter( this );
        painter.setClipRect( event->rect() );
        painter.setRenderHint( QPainter::Antialiasing );
        painter.setBrush( color_ );
        painter.setPen( Qt::NoPen );
        QRectF tmp_rect( FormatColorButton::rect() );
        tmp_rect.setWidth( 0.5*qMin( rect().width(), rect().height() ) );
        tmp_rect.setHeight( 0.5*qMin( rect().width(), rect().height() ) );
        tmp_rect.translate( rect().width() - tmp_rect.width(), rect().height() - tmp_rect.height() );
        painter.drawEllipse( tmp_rect );
        painter.end();

    }

    // default handling if color is invalid
    QToolButton::paintEvent( event );

    return;

}
