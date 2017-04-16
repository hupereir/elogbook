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

namespace Private
{

    //* color widget
    class ColorWidget: public QToolButton, public Counter
    {

        public:

        //* constructor
        ColorWidget( QWidget* );

        //* color
        void setColor( const QColor& );

        //* size hint
        QSize sizeHint( void ) const;

        //* size hint
        QSize minimumSizeHint( void ) const;

        protected:

        //* paint event
        void paintEvent( QPaintEvent* );

    };

    //* local QSplitter object, derived from Counter
    class LocalSplitter: public QSplitter, public Counter
    {

        public:

        //* constructor
        LocalSplitter( QWidget* parent ):
            QSplitter( parent ),
            Counter( "LocalSplitter" )
        {}

    };

    //* local text editor, to deal with HTML edition
    class LocalTextEditor: public TextEditor
    {

        public:

        //* constructor
        LocalTextEditor( QWidget* );

        //* insert link action
        QAction& insertLinkAction( void ) const
        { return *insertLinkAction_; }

        //* edit link action
        QAction& editLinkAction( void ) const
        { return *editLinkAction_; }

        //* remove link action
        QAction& removeLinkAction( void ) const
        { return *removeLinkAction_; }

        //* view link action
        QAction& openLinkAction( void ) const
        { return *openLinkAction_; }

        //* return cursor at context menu
        QTextCursor cursorAtContextMenu( void ) const
        { return cursorForPosition( _contextMenuPosition() );  }

        protected:

        //* insert from mime data
        virtual void insertFromMimeData( const QMimeData* );

        //* install actions in context menu
        virtual void installContextMenuActions( BaseContextMenu*, bool = true );

        private:

        //* install actions
        void _installActions( void );

        //* insert link
        QAction* insertLinkAction_ = nullptr;

        //* edit link
        QAction* editLinkAction_ = nullptr;

        //*  remove
        QAction* removeLinkAction_ = nullptr;

        //* open link
        QAction* openLinkAction_ = nullptr;

    };

}

#endif
