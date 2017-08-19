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

#include "BaseIconNames.h"
#include "ColorMenu.h"
#include "CppUtil.h"
#include "CustomToolButton.h"
#include "Debug.h"
#include "FormatBarIconNames.h"
#include "IconEngine.h"
#include "PixmapEngine.h"
#include "Singleton.h"
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
class FormatColorButton: public CustomToolButton
{

    Q_OBJECT

    public:

    //* constructor
    explicit FormatColorButton( QWidget* parent ):
        CustomToolButton( parent )
    {}

    public Q_SLOTS:

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
    CustomToolBar( "Text format", parent, optionName )
{

    Debug::Throw( "ToolBar::ToolBar.\n" );

    // bold
    QAction* action;
    actions_ = Base::makeT<ActionMap>(
    {
        { Bold, addAction( IconEngine::get( IconNames::Bold ), tr( "Bold" ), this, SLOT(_bold(bool)) ) },
        { Italic, addAction( IconEngine::get( IconNames::Italic ), tr( "Italic" ), this, SLOT(_italic(bool)) ) },
        { Underline, addAction( IconEngine::get( IconNames::Underline ), tr( "Underline" ), this, SLOT(_underline(bool)) ) },
        { Strike, addAction( IconEngine::get( IconNames::Strike ), tr( "Strike" ), this, SLOT(_strike(bool)) ) },
        { Color, action = new QAction( IconEngine::get( IconNames::Color ), tr( "Color" ), this ) }
    });

    for( auto&& iter = actions_.begin(); iter != actions_.end(); ++iter )
    { iter.value()->setCheckable( true ); }

    // color
    action->setCheckable( false );
    action->setIconText( tr( "Color" ) );
    connect( action, SIGNAL(triggered()), SLOT(_lastColor()) );

    // color menu
    colorMenu_ = new ColorMenu( this );
    action->setMenu( colorMenu_ );
    connect( colorMenu_, SIGNAL(selected(QColor)), SLOT(_color(QColor)) );
    action->setMenu( colorMenu_ );

    // color button
    FormatColorButton *button( new FormatColorButton( this ) );
    button->setText( tr( "Color" ) );
    addWidget( button );
    button->setDefaultAction( action );
    button->setPopupMode( QToolButton::MenuButtonPopup );
    connect( colorMenu_, SIGNAL(selected(QColor)), button, SLOT(setColor(QColor)) );

    // configuration
    connect( Base::Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    connect( Base::Singleton::get().application(), SIGNAL(saveConfiguration()), SLOT(_saveConfiguration()) );
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
void FormatBar::load( const Format::TextFormatBlock::List& formatList ) const
{

    Debug::Throw( "FormatBar::loadFormats.\n" );
    Q_CHECK_PTR( editor_ );

    // get base text color name
    const QColor baseTextColor( editor_->palette().color( QPalette::Text ) );

    QTextCursor cursor( editor_->document() );
    cursor.beginEditBlock();
    for( const auto& block:formatList )
    {

        // define cursor
        cursor.setPosition( block.begin(), QTextCursor::MoveAnchor );
        cursor.setPosition( block.end(), QTextCursor::KeepAnchor );

        // define format
        QTextCharFormat textFormat;
        textFormat.setFontWeight( block.format() & Format::Bold ? QFont::Bold : QFont::Normal );
        textFormat.setFontItalic( block.format() & Format::Italic );
        textFormat.setFontUnderline( block.format() & Format::Underline );
        textFormat.setFontStrikeOut( block.format() & Format::Strike );
        textFormat.setFontOverline( block.format() & Format::Overline );

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
Format::TextFormatBlock::List FormatBar::get() const
{
    Debug::Throw( "FormatBar::get.\n" );
    Q_CHECK_PTR( editor_ );

    Format::TextFormatBlock::List out;

    // iterator over blocks
    for( QTextBlock block = editor_->document()->begin(); block.isValid(); block = block.next() )
    {

        // iterator over text fragments
        for( auto&& it = block.begin(); !(it.atEnd()); ++it)
        {
            QTextFragment fragment = it.fragment();
            if( !fragment.isValid() ) continue;

            // retrieve fragments position
            int begin( fragment.position() );
            int end( fragment.position() + fragment.length() );

            // retrieve text format
            QTextCharFormat textFormat( fragment.charFormat() );

            Format::TextFormatFlags format( Format::Default );
            if( textFormat.fontWeight() == QFont::Bold ) format |= Format::Bold;
            if( textFormat.fontItalic() ) format |= Format::Italic;
            if( textFormat.fontUnderline() ) format |= Format::Underline;
            if( textFormat.fontStrikeOut() ) format |= Format::Strike;
            if( textFormat.fontOverline() ) format |= Format::Overline;

            // retrieve text color
            QColor color( textFormat.foreground().color() );
            if( color == editor_->palette().color( QPalette::Text ) ) color = QColor();

            const QString href( textFormat.anchorHref() );

            // skip format if corresponds to default
            if( format == Format::Default && !color.isValid() && href.isEmpty() ) continue;

            // store new TextFormatBlock
            Format::TextFormatBlock textFormatBlock( begin, end, format, color );
            if( !href.isEmpty() ) textFormatBlock.setHRef( href );

            out.append( textFormatBlock );

        }
    }

    return out;
}

//________________________________________
void FormatBar::_updateConfiguration()
{
    Debug::Throw( "FormatBar::_updateConfiguration.\n" );

    // retrieve colors from options
    Options::List colors( XmlOptions::get().specialOptions( "TEXT_COLOR" ) );
    if( colors.empty() )
    {

        // add default colors
        using ColorList = QList<Base::Color>;
        static const auto defaultColors = Base::makeT<ColorList>(
        {
            Base::Color("#aa0000"),
            Base::Color("green"),
            Base::Color("blue"),
            Base::Color("grey"),
            Base::Color("black")
        });

        XmlOptions::get().keep( "TEXT_COLOR" );
        for( const auto& color:defaultColors )
        {
            XmlOptions::get().add( "TEXT_COLOR", Option().set( color ) );
            colorMenu_->add( color );
        }

    } else {

        for( const auto& color:colors )
        { colorMenu_->add( color.get<Base::Color>() ); }

    }
}


//________________________________________
void FormatBar::_saveConfiguration()
{
    Debug::Throw( "FormatBar::_saveConfiguration.\n" );
    XmlOptions::get().keep( "TEXT_COLOR" );

    const Base::Color::Set colors( colorMenu_->colors() );
    for( const auto& color:colors )
    { XmlOptions::get().add( "TEXT_COLOR", Option().set( Base::Color( color ) ) ); }

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

        if( !color.isValid() ) color = palette().color( QPalette::Active, QPalette::Text );

        QTextCharFormat format;
        format.setForeground( color );
        editor_->mergeCurrentCharFormat( format );
    }
}

//______________________________________
void FormatBar::_lastColor()
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
    CustomToolButton::paintEvent( event );

    return;

}
