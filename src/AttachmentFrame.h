#ifndef AttachmentFrame_h
#define AttachmentFrame_h

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

#include "Attachment.h"
#include "AttachmentModel.h"
#include "Debug.h"
#include "Key.h"
#include "ValidFileThread.h"

#include <QtGui/QWidget>
#include <QtGui/QMenu>

class TreeView;

/*!
\class  AttachmentFrame
\brief  handles attachment list
*/
class AttachmentFrame: public QWidget, public BASE::Key
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! constructor
    AttachmentFrame( QWidget *, bool );

    //! default size
    void setDefaultHeight( const int& );

    //! default height
    const int& defaultHeight( void ) const
    { return defaultHeight_; }

    //! size
    QSize sizeHint( void ) const;

    //! list
    bool hasList( void ) const
    { return (bool) list_; }

    //! list
    TreeView& list( void ) const
    { return *list_; }

    //! clear
    void clear( void )
    {
        // clear model and associations
        _model().clear();
        BASE::Key::clearAssociations<Attachment>();
    }

    //! add attachment to the list
    void add( Attachment& attachment )
    {
        AttachmentModel::List attachments;
        attachments << &attachment;
        add( attachments );
    }

    //! add attachments to list
    void add( const AttachmentModel::List& attachments );

    //! update attachment in the list
    void update( Attachment& attachment );

    //! select attachment in the list
    void select( Attachment& attachment );

    //! remove attachment from list
    void remove( Attachment& attachment )
    { _model().remove( &attachment ); }

    //! change read only status
    void setReadOnly( bool value )
    {
        readOnly_ = value;
        _updateActions();
    }

    //! read only state
    const bool& readOnly( void ) const
    { return readOnly_; }

    //! context menu
    QMenu& contextMenu( void ) const
    { return *contextMenu_; }

    //!@name actions
    //@{

    //! visibility action
    QAction& visibilityAction( void ) const
    { return *visibilityAction_; }

    //! new attachment action
    QAction& newAction( void ) const
    { return *newAction_; }

    //! view attachment action
    QAction& openAction( void ) const
    { return *openAction_; }

    //! edit attachment action
    QAction& editAction( void ) const
    { return *editAction_; }

    //! update attachment action
    QAction& reloadAction( void ) const
    { return *reloadAction_; }

    //! update attachment action
    QAction& saveAsAction( void ) const
    { return *saveAsAction_; }

    //! delete attachment action
    QAction& deleteAction( void ) const
    { return *deleteAction_; }

    //! clean action
    QAction& cleanAction( void ) const
    { return *cleanAction_; }

    //@}

    signals:

    //! emitted when an item is selected in list
    void attachmentSelected( Attachment& );

    protected:

    //! enter event
    virtual void enterEvent( QEvent* );

    //! custom event, used to retrieve file validity check event
    void customEvent( QEvent* );

    private slots:

    //! update configuration
    void _updateConfiguration( void );

    //! update context menu
    void _updateActions( void );

    //! create new attachment
    void _new( void );

    //! display current attachment
    void _open( void );

    //! edit current attachment
    void _edit( void );

    //! delete current attachment
    void _delete( void );

    //! reload attachment time stamps
    void _reload( void );

    //! save current attachment
    void _saveAs( void );

    //! clean
    void _clean( void );

    //!@name selections
    //@{

    //! current item changed
    void _itemSelected( const QModelIndex& );

    //@}

    private:

    //! install actions
    void _installActions( void );

    //! model
    AttachmentModel& _model( void )
    { return model_; }

    //! save attachment
    /*! this requires carefull handling of associate entries and Edition windows */
    void _saveAttachments( const AttachmentModel::List& );

    //! if true, listbox is read only
    bool readOnly_;

    //! default height;
    int defaultHeight_;

    //! context menu
    QMenu* contextMenu_;

    //!@name actions
    //@{

    //! visibility action
    QAction* visibilityAction_;

    //! new attachment
    QAction* newAction_;

    //! view attachment
    QAction* openAction_;

    //! edit attachment
    QAction* editAction_;

    //! update actions
    QAction* reloadAction_;

    //! save as action
    QAction* saveAsAction_;

    //! delete attachment
    QAction* deleteAction_;

    //! clean action
    QAction* cleanAction_;

    //@}

    //! model
    AttachmentModel model_;

    //! list
    TreeView* list_;

    // valid file thread
    ValidFileThread thread_;

};

#endif
