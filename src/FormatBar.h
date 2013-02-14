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

#include "Counter.h"
#include "CustomToolBar.h"
#include "TextFormatBlock.h"
#include "TextFormat.h"

#include <QAction>
#include <QFont>
#include <QTextCharFormat>
#include <QToolButton>
#include <QMap>

class ColorMenu;
class TextEditor;

//! used for customized color button
class FormatColorButton: public QToolButton, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    FormatColorButton( QWidget* parent ):
        QToolButton( parent ),
        Counter( "FormatColorButton" )
    {}

    public slots:

    //! set color
    void setColor( QColor color )
    {
        color_ = color;
        update();
    }

    protected:

    //! paint
    void paintEvent( QPaintEvent* event );

    private:

    // associated color
    QColor color_;

};


//! text formating bar
class FormatBar: public CustomToolBar
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! bold icon name
    static const QString BOLD_ICON;

    //! italic icon name
    static const QString ITALIC_ICON;

    //! strike icon name
    static const QString STRIKE_ICON;

    //! underline icon name
    static const QString UNDERLINE_ICON;

    //! action id enumeration
    enum ActionId
    {
        Bold,
        Italic,
        Strike,
        Underline,
        Color
    };

    //! constructor
    FormatBar( QWidget* parent, const QString& option_name );

    //! destructor
    virtual ~FormatBar( void )
    { Debug::Throw( "FormatBar::~FormatBar.\n" ); }

    //! set target editor
    void setTarget( TextEditor& editor );

    //! load text formats
    void load( const FORMAT::TextFormatBlock::List& ) const;

    //! get text formats
    FORMAT::TextFormatBlock::List get( void ) const;

    //! button map
    typedef QMap< ActionId, QAction* > ActionMap;

    //! actions
    const ActionMap& actions( void ) const
    { return actions_; }

    public slots:

    //! update button state
    void updateState( const QTextCharFormat& );

    private slots:

    //! update configuration
    void _updateConfiguration( void );

    //! save configuration
    void _saveConfiguration( void );

    //! bold
    void _bold( bool );

    //! italic
    void _italic( bool );

    //! underline
    void _underline( bool );

    //! strike
    void _strike( bool );

    //! color
    void _color( QColor );

    //! last selected color
    void _lastColor( void );

    private:

    //! target text editor
    TextEditor* editor_;

    // enabled
    bool enabled_;

    //! button map
    ActionMap actions_;

    //! color menu
    ColorMenu* colorMenu_;

};

#endif
