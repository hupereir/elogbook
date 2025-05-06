#ifndef EditionWindow_p_h
#define EditionWindow_p_h

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

#include "LineEditorButton.h"

namespace Private
{

    //! color widget
    class ColorWidget: public LineEditorButton
    {

        Q_OBJECT

        public:

        //! constructor
        explicit ColorWidget( QWidget* = nullptr );

        //! color
        void setColor( const QColor& );

    };

    //! local QSplitter object, derived from Counter
    class LocalSplitter: public QSplitter, private Base::Counter<LocalSplitter>
    {

        Q_OBJECT

        public:

        //! constructor
        explicit LocalSplitter( QWidget* parent ):
            QSplitter( parent ),
            Counter( QStringLiteral("LocalSplitter") )
        {}

    };

    //! local text editor, to deal with HTML edition
    class LocalTextEditor: public TextEditor
    {

        Q_OBJECT

        public:

        //! constructor
        explicit LocalTextEditor( QWidget* );

        //! insert link action
        QAction& insertLinkAction() const
        { return *insertLinkAction_; }

        //! edit link action
        QAction& editLinkAction() const
        { return *editLinkAction_; }

        //! remove link action
        QAction& removeLinkAction() const
        { return *removeLinkAction_; }

        //! view link action
        QAction& openLinkAction() const
        { return *openLinkAction_; }

        //! return cursor at context menu
        QTextCursor cursorAtContextMenu() const
        { return cursorForPosition( _contextMenuPosition() );  }

        protected:

        //! insert from mime data
        void insertFromMimeData( const QMimeData* ) override;

        //! install actions in context menu
        void installContextMenuActions( BaseContextMenu*, bool = true ) override;

        private:

        //! configuration
        void _updateConfiguration();

        //! install actions
        void _installActions();

        //! true when links are to be inserted automatically
        bool autoInsertLinks_ = true;

        //! insert link
        QAction* insertLinkAction_ = nullptr;

        //! edit link
        QAction* editLinkAction_ = nullptr;

        //!  remove
        QAction* removeLinkAction_ = nullptr;

        //! open link
        QAction* openLinkAction_ = nullptr;

    };

}

#endif
