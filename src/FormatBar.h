#ifndef FormatBar_h
#define FormatBar_h

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

#include "Counter.h"
#include "TextFormat.h"
#include "TextFormatBlock.h"
#include "ToolBar.h"

#include <QAction>
#include <QFont>
#include <QTextCharFormat>
#include <QToolButton>
#include <QHash>

class ColorMenu;
class TextEditor;

//* text formating bar
class FormatBar: public ToolBar
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* action id enumeration
    enum class ActionId
    {
        Bold,
        Italic,
        Strike,
        Underline,
        Foreground,
        Background
    };

    //* constructor
    explicit FormatBar( QWidget*, const QString& );

    //*@name accessors
    //@{

    //* load text formats
    void load( const TextFormat::Block::List& ) const;

    //* get text formats
    TextFormat::Block::List get() const;

    //* button map
    using ActionMap = QHash< ActionId, QAction* >;

    //* actions
    const ActionMap& actions() const
    { return actions_; }

    //@}

    //*@name modifiers
    //@{

    //* set target editor
    void setTarget( TextEditor& );

    //* update button state
    void updateState( const QTextCharFormat& );

    //@}

    private:

    //* update configuration
    void _updateConfiguration();

    //* save configuration
    void _saveConfiguration();

    //* bold
    void _bold( bool );

    //* italic
    void _italic( bool );

    //* underline
    void _underline( bool );

    //* strike
    void _strike( bool );

    //* color
    void _foreground( QColor );

    //* color
    void _background( QColor );

    //* last selected color
    void _lastForegroundColor();

    //* last selected color
    void _lastBackgroundColor();

    //* target text editor
    TextEditor* editor_ = nullptr;

    // enabled
    bool enabled_ = true;

    //* button map
    ActionMap actions_;

    //* color menu
    ColorMenu* foregroundColorMenu_ = nullptr;

    //* color menu
    ColorMenu* backgroundColorMenu_ = nullptr;

};

#endif
