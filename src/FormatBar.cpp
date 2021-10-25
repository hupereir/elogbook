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

#include "FormatBar.h"

#include "Application.h"
#include "BaseIconNames.h"
#include "ColorMenu.h"
#include "CppUtil.h"
#include "ToolButton.h"
#include "Debug.h"
#include "FormatBarIconNames.h"
#include "IconEngine.h"
#include "PixmapEngine.h"
#include "Singleton.h"
#include "TextBlockRange.h"
#include "TextEditor.h"
#include "TextPosition.h"
#include "XmlOptions.h"

#include <QPainter>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextFragment>

// used for customized color button
class FormatColorButton: public ToolButton
{

    Q_OBJECT

    public:

    //* constructor
    explicit FormatColorButton( QWidget* parent ):
        ToolButton( parent )
    {}

    //* set color
    void setColor( QColor color )
    {
        color_ = color;
        update();
    }

    protected:

    //* paint
    void paintEvent( QPaintEvent* event );

    private:

    // associated color
    QColor color_;

};


// needs to be included after definition of FormatColorButton
#include "FormatBar.moc"

//________________________________________
FormatBar::FormatBar( QWidget* parent, const QString& optionName ):
    ToolBar( QStringLiteral("Text format"), parent, optionName )
{

    Debug::Throw( QStringLiteral("ToolBar::ToolBar.\n") );

    // bold
    QAction* foreground = nullptr;
    QAction* background = nullptr;
    actions_ =
    {
        { ActionId::Bold, addAction( IconEngine::get( IconNames::Bold ), tr( "Bold" ), this, &FormatBar::_bold ) },
        { ActionId::Italic, addAction( IconEngine::get( IconNames::Italic ), tr( "Italic" ), this, &FormatBar::_italic ) },
        { ActionId::Underline, addAction( IconEngine::get( IconNames::Underline ), tr( "Underline" ), this, &FormatBar::_underline ) },
        { ActionId::Strike, addAction( IconEngine::get( IconNames::Strike ), tr( "Strike" ), this, &FormatBar::_strike ) },
        { ActionId::Foreground, foreground = new QAction( IconEngine::get( IconNames::Foreground ), tr( "Text Color" ), this ) },
        { ActionId::Background, background = new QAction( IconEngine::get( IconNames::Background ), tr( "Background Color" ), this ) }
    };

    for( auto&& iter = actions_.begin(); iter != actions_.end(); ++iter )
    { iter.value()->setCheckable( true ); }

    // foreground color action
    {
        foreground->setCheckable( false );
        foreground->setIconText( tr( "Color" ) );
        connect( foreground, &QAction::triggered, this, &FormatBar::_lastForegroundColor );
    
        // color menu
        foregroundColorMenu_ = new ColorMenu( this );
        foreground->setMenu( foregroundColorMenu_ );
        connect( foregroundColorMenu_, &ColorMenu::selected, this, &FormatBar::_foreground );
        foreground->setMenu( foregroundColorMenu_ );

        // color button
        auto button( new FormatColorButton( this ) );
        button->setText( tr( "Text Color" ) );
        addWidget( button );
        button->setDefaultAction( foreground );
        button->setPopupMode( QToolButton::MenuButtonPopup );
        connect( foregroundColorMenu_, &ColorMenu::selected, button, &FormatColorButton::setColor );
    }

    
    // background color action
    {
        background->setCheckable( false );
        background->setIconText( tr( "Color" ) );
        connect( background, &QAction::triggered, this, &FormatBar::_lastBackgroundColor );
    
        // color menu
        backgroundColorMenu_ = new ColorMenu( this );
        background->setMenu( backgroundColorMenu_ );
        connect( backgroundColorMenu_, &ColorMenu::selected, this, &FormatBar::_background );
        background->setMenu( backgroundColorMenu_ );

        // color button
        auto button( new FormatColorButton( this ) );
        button->setText( tr( "Text Color" ) );
        addWidget( button );
        button->setDefaultAction( background );
        button->setPopupMode( QToolButton::MenuButtonPopup );
        connect( backgroundColorMenu_, &ColorMenu::selected, button, &FormatColorButton::setColor );
    }

    // configuration
    connect( Base::Singleton::get().application<Application>(), &Application::configurationChanged, this, &FormatBar::_updateConfiguration );
    connect( Base::Singleton::get().application<Application>(), &Application::saveConfiguration, this, &FormatBar::_saveConfiguration );
    _updateConfiguration();

}

//________________________________________
void FormatBar::setTarget( TextEditor& editor )
{

    Debug::Throw() << "FormatBar::setTarget - key: " << editor.key() << endl;
    editor_ = &editor;

    // first update
    updateState( editor_->currentCharFormat() );

}

//________________________________________
void FormatBar::load( const TextFormat::Block::List& formatList ) const
{

    Debug::Throw( QStringLiteral("FormatBar::loadFormats.\n") );
    Q_CHECK_PTR( editor_ );

    // get base text color name
    const QColor baseTextColor( editor_->palette().color( QPalette::Text ) );
    const QColor baseViewColor( editor_->palette().color( QPalette::Base ) );

    QTextCursor cursor( editor_->document() );
    cursor.beginEditBlock();
    for( const auto& block:formatList )
    {

        // define cursor
        cursor.setPosition( block.begin(), QTextCursor::MoveAnchor );
        cursor.setPosition( block.end(), QTextCursor::KeepAnchor );

        // define format
        QTextCharFormat textFormat;
        textFormat.setFontWeight( block.format() & TextFormat::Bold ? QFont::Bold : QFont::Normal );
        textFormat.setFontItalic( block.format() & TextFormat::Italic );
        textFormat.setFontUnderline( block.format() & TextFormat::Underline );
        textFormat.setFontStrikeOut( block.format() & TextFormat::Strike );
        textFormat.setFontOverline( block.format() & TextFormat::Overline );

        // load foreground
        if( block.foreground().isValid() && !(block.foreground() == baseTextColor) )
        { textFormat.setForeground( block.foreground() ); }

        // load background
        if( block.background().isValid() && !(block.background() == baseViewColor) )
        { textFormat.setBackground( block.background() ); }

        // load href
        if( !block.href().isEmpty() )
        { textFormat.setAnchorHref( block.href() ); }

        cursor.setCharFormat( textFormat );

    }

    cursor.endEditBlock();

}
//________________________________________
TextFormat::Block::List FormatBar::get() const
{
    Debug::Throw( QStringLiteral("FormatBar::get.\n") );

    TextFormat::Block::List out;

    // get base text color name
    const QColor baseTextColor( editor_->palette().color( QPalette::Text ) );
    const QColor baseViewColor( editor_->palette().color( QPalette::Base ) );

    // iterator over blocks
    for( const auto& block:TextBlockRange( editor_->document() ) )
    {

        // iterator over text fragments
        for( auto&& it = block.begin(); !(it.atEnd()); ++it)
        {
            const auto fragment = it.fragment();
            if( !fragment.isValid() ) continue;

            // retrieve fragments position
            const int begin( fragment.position() );
            const int end( fragment.position() + fragment.length() );

            // retrieve text format
            const QTextCharFormat textFormat( fragment.charFormat() );

            TextFormat::Flags format( TextFormat::Default );
            if( textFormat.fontWeight() == QFont::Bold ) format |= TextFormat::Bold;
            if( textFormat.fontItalic() ) format |= TextFormat::Italic;
            if( textFormat.fontUnderline() ) format |= TextFormat::Underline;
            if( textFormat.fontStrikeOut() ) format |= TextFormat::Strike;
            if( textFormat.fontOverline() ) format |= TextFormat::Overline;

            // retrieve text color
            QColor foreground( textFormat.foreground().color() );
            if( foreground == baseTextColor ) foreground = QColor();

            // retrieve text color
            QColor background( textFormat.background().color() );
            if( background == baseViewColor ) background = QColor();

            const QString href( textFormat.anchorHref() );

            // skip format if corresponds to default
            if( format == TextFormat::Default && !foreground.isValid() && !background.isValid() && href.isEmpty() ) continue;

            // store new TextFormatBlock
            TextFormat::Block textFormatBlock( begin, end, format );
            textFormatBlock.setForeground( foreground );
            textFormatBlock.setBackground( background );
            
            if( !href.isEmpty() ) textFormatBlock.setHRef( href );

            out.append( textFormatBlock );

        }
    }

    return out;
}

//________________________________________
void FormatBar::_updateConfiguration()
{
    Debug::Throw( QStringLiteral("FormatBar::_updateConfiguration.\n") );

    // retrieve colors from options
    Options::List colors( XmlOptions::get().specialOptions( QStringLiteral("TEXT_COLOR") ) );
    if( colors.empty() )
    {

        // add default colors
        using ColorList = QList<Base::Color>;
        static const ColorList defaultColors(
        {
            Base::Color("#aa0000"),
            Base::Color("green"),
            Base::Color("blue"),
            Base::Color("grey"),
            Base::Color("black")
        });

        XmlOptions::get().keep( QStringLiteral("TEXT_COLOR") );
        for( const auto& color:defaultColors )
        {
            XmlOptions::get().add( QStringLiteral("TEXT_COLOR"), Option().set( color ) );
            foregroundColorMenu_->add( color );
            backgroundColorMenu_->add( color );
        }

    } else {

        for( const auto& color:colors )
        { 
            foregroundColorMenu_->add( color.get<Base::Color>() ); 
            backgroundColorMenu_->add( color.get<Base::Color>() ); 
        }

    }
}


//________________________________________
void FormatBar::_saveConfiguration()
{
    Debug::Throw( QStringLiteral("FormatBar::_saveConfiguration.\n") );
    XmlOptions::get().keep( QStringLiteral("TEXT_COLOR") );

    Base::Color::Set colors( foregroundColorMenu_->colors() );
    colors.unite( backgroundColorMenu_->colors() );
    
    for( const auto& color:colors )
    { XmlOptions::get().add( QStringLiteral("TEXT_COLOR"), Option().set( Base::Color( color ) ) ); }

    return;
}

//________________________________________
void FormatBar::_bold( bool state )
{
    Debug::Throw( QStringLiteral("FormatBar::_bold.\n") );
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
    Debug::Throw( QStringLiteral("FormatBar::_italic.\n") );
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
    Debug::Throw( QStringLiteral("FormatBar::_underline.\n") );
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
    Debug::Throw( QStringLiteral("FormatBar::_strike.\n") );
    if( editor_ && enabled_ )
    {
        QTextCharFormat format;
        format.setFontStrikeOut( state );
        editor_->mergeCurrentCharFormat( format );
    }
}

//________________________________________
void FormatBar::_foreground( QColor color )
{
    Debug::Throw( QStringLiteral("FormatBar::_foreground.\n") );
    if( editor_ && enabled_ )
    {

        if( !color.isValid() ) color = palette().color( QPalette::Active, QPalette::Text );

        QTextCharFormat format;
        format.setForeground( color );
        editor_->mergeCurrentCharFormat( format );
    }
}

//________________________________________
void FormatBar::_background( QColor color )
{
    Debug::Throw( QStringLiteral("FormatBar::_background.\n") );
    if( editor_ && enabled_ )
    {

        if( !color.isValid() ) color = palette().color( QPalette::Active, QPalette::Base );

        QTextCharFormat format;
        format.setBackground( color );
        editor_->mergeCurrentCharFormat( format );
    }
}

//______________________________________
void FormatBar::_lastForegroundColor()
{
    Debug::Throw( QStringLiteral("FormatBar::_lastForegroundColor.\n") );
    _foreground( foregroundColorMenu_->lastColor() );
}

//______________________________________
void FormatBar::_lastBackgroundColor()
{
    Debug::Throw( QStringLiteral("FormatBar::_lastBackgroundColor.\n") );
    _background( backgroundColorMenu_->lastColor() );
}

//________________________________________
void FormatBar::updateState( const QTextCharFormat& format )
{
    Debug::Throw( QStringLiteral("FormatBar::updateState.\n") );
    enabled_ = false;
    actions_[ActionId::Bold]->setChecked( format.fontWeight() == QFont::Bold );
    actions_[ActionId::Italic]->setChecked( format.fontItalic() );
    actions_[ActionId::Underline]->setChecked( format.fontUnderline() );
    actions_[ActionId::Strike]->setChecked( format.fontStrikeOut() );
    enabled_ = true;
}

//________________________________________
void FormatColorButton::paintEvent( QPaintEvent* event )
{
    if( color_.isValid() )
    {

        // prepare option
        QStyleOptionToolButton toolButtonOption;
        toolButtonOption.initFrom(this);
        toolButtonOption.features |= QStyleOptionToolButton::MenuButtonPopup;

        QRect subRect( style()->subControlRect(
            QStyle::CC_ToolButton,
            &toolButtonOption,
            QStyle::SC_ToolButton,
            this ) );


        QPainter painter( this );
        painter.setPen( color_ );
        painter.drawLine( subRect.bottomLeft() + QPoint( 5, -3 ), subRect.bottomRight() + QPoint( -5, -3 ) );
        painter.end();

    }

    // default handling if color is invalid
    ToolButton::paintEvent( event );

    return;

}
