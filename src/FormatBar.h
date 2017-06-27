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
#include "CustomToolBar.h"
#include "TextFormatBlock.h"
#include "TextFormat.h"

#include <QAction>
#include <QFont>
#include <QTextCharFormat>
#include <QToolButton>
#include <QHash>

class ColorMenu;
class TextEditor;

//* text formating bar
class FormatBar: public CustomToolBar
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* action id enumeration
    enum ActionId
    {
        Bold,
        Italic,
        Strike,
        Underline,
        Color
    };

    //* constructor
    explicit FormatBar( QWidget*, const QString& );

    //* set target editor
    void setTarget( TextEditor& );

    //* load text formats
    void load( const Format::TextFormatBlock::List& ) const;

    //* get text formats
    Format::TextFormatBlock::List get() const;

    //* button map
    using ActionMap = QHash< ActionId, QAction* >;

    //* actions
    const ActionMap& actions() const
    { return actions_; }

    public Q_SLOTS:

    //* update button state
    void updateState( const QTextCharFormat& );

    private Q_SLOTS:

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
    void _color( QColor );

    //* last selected color
    void _lastColor();

    private:

    //* target text editor
    TextEditor* editor_ = nullptr;

    // enabled
    bool enabled_ = true;

    //* button map
    ActionMap actions_;

    //* color menu
    ColorMenu* colorMenu_ = nullptr;

};

#endif
