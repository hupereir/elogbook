#ifndef AttachmentFrame_h
#define AttachmentFrame_h

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

#include "Attachment.h"
#include "AttachmentModel.h"
#include "Debug.h"
#include "Key.h"
#include "ThreadDeleter.h"
#include "ValidFileThread.h"

#include <QWidget>
#include <QMenu>

class TreeView;

/**
\class  AttachmentFrame
\brief  handles attachment list
*/
class AttachmentFrame: public QWidget, public Base::Key
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit AttachmentFrame( QWidget*, bool );

    //* default size
    void setDefaultHeight( int );

    //* default height
    int defaultHeight() const
    { return defaultHeight_; }

    //* size
    QSize sizeHint() const override;

    //* list
    TreeView& list() const
    { return *treeView_; }

    //* clear
    void clear()
    {
        // clear model and associations
        _model().clear();
        Base::Key::clearAssociations<Attachment>();
    }

    //* add attachment to the list
    void add( Attachment& attachment )
    {
        AttachmentModel::List attachments;
        attachments << &attachment;
        add( attachments );
    }

    //* add attachments to list
    void add( const AttachmentModel::List& attachments );

    //* update attachment in the list
    void update( Attachment& attachment );

    //* select attachment in the list
    void select( Attachment& attachment );

    //* remove attachment from list
    void remove( Attachment& attachment )
    { _model().remove( &attachment ); }

    //* change read only status
    void setReadOnly( bool value )
    {
        readOnly_ = value;
        _updateActions();
    }

    //* read only state
    bool readOnly() const
    { return readOnly_; }

    //* context menu
    QMenu& contextMenu() const
    { return *contextMenu_; }

    //*@name actions
    //@{

    //* visibility action
    QAction& visibilityAction() const
    { return *visibilityAction_; }

    //* new attachment action
    QAction& newAction() const
    { return *newAction_; }

    //* view attachment action
    QAction& openAction() const
    { return *openAction_; }

    //* edit attachment action
    QAction& editAction() const
    { return *editAction_; }

    //* update attachment action
    QAction& reloadAction() const
    { return *reloadAction_; }

    //* update attachment action
    QAction& saveAsAction() const
    { return *saveAsAction_; }

    //* delete attachment action
    QAction& deleteAction() const
    { return *deleteAction_; }

    //* clean action
    QAction& cleanAction() const
    { return *cleanAction_; }

    //@}

    Q_SIGNALS:

    //* emitted when an item is selected in list
    void attachmentSelected( Attachment& );

    protected:

    //* enter event
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void enterEvent( QEvent* ) override;
    #else
    void enterEvent( QEnterEvent* ) override;
    #endif

    private:

    //* process records from thread
    void _processRecords( const FileRecord::List&, bool );

    //* update configuration
    void _updateConfiguration();

    //* update context menu
    void _updateActions();

    //* create new attachment
    void _new();

    //* display current attachment
    void _open();

    //* edit current attachment
    void _edit();

    //* delete current attachment
    void _delete();

    //* reload attachment time stamps
    void _reload();

    //* save current attachment
    void _saveAs();

    //* clean
    void _clean();

    //* current item changed
    void _itemSelected( const QModelIndex& );

    //* install actions
    void _installActions();

    //* model
    AttachmentModel& _model()
    { return model_; }

    //* save attachment
    /** this requires carefull handling of associate entries and Edition windows */
    void _saveAttachments( const AttachmentModel::List& );

    //* if true, listbox is read only
    bool readOnly_ = true;

    //* default height;
    int defaultHeight_ = -1;

    //* context menu
    QMenu* contextMenu_ = nullptr;

    //*@name actions
    //@{

    //* visibility action
    QAction* visibilityAction_ = nullptr;

    //* new attachment
    QAction* newAction_ = nullptr;

    //* view attachment
    QAction* openAction_ = nullptr;

    //* edit attachment
    QAction* editAction_ = nullptr;

    //* update actions
    QAction* reloadAction_ = nullptr;

    //* save as action
    QAction* saveAsAction_ = nullptr;

    //* delete attachment
    QAction* deleteAction_ = nullptr;

    //* clean action
    QAction* cleanAction_ = nullptr;

    //@}

    //* model
    AttachmentModel model_;

    //* list
    TreeView* treeView_ = nullptr;

    //* thread to check file validity
    std::unique_ptr<ValidFileThread, Base::ThreadDeleter> thread_;

};

#endif
