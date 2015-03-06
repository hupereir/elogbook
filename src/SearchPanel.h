#ifndef SearchPanel_h
#define SearchPanel_h

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

#include "CustomToolBar.h"
#include "IconSize.h"

#include <QCheckBox>
#include <QContextMenuEvent>
#include <QHash>
#include <QPushButton>

class CustomComboBox;
class TransitionWidget;

//! selects entries from keyword/title/text/...
class SearchPanel: public CustomToolBar
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! constructor
    SearchPanel( const QString&, QWidget*, const QString& = QString() );

    //! search mode enumeration
    enum SearchMode
    {
        None = 0,
        Title = 1<<0,
        Keyword = 1<<1,
        Text = 1<<2,
        Attachment = 1<<3,
        Color = 1<<4
    };

    Q_DECLARE_FLAGS( SearchModes, SearchMode )

    //! editor
    CustomComboBox& editor( void ) const
    { return *editor_; }

    Q_SIGNALS:

    //! emitted when the Find button is pressed
    void selectEntries( QString, SearchPanel::SearchModes );

    //! emitted when the Show All button is pressed
    void showAllEntries( void );

    public Q_SLOTS:

    //! show
    virtual void show( void );

    //! hide
    virtual void hide( void );

    //! visibility
    virtual void setVisible( bool );

    protected:

    //! context menu
    virtual void contextMenuEvent( QContextMenuEvent* );

    //! transition widget
    TransitionWidget& _transitionWidget( void ) const
    { return *transitionWidget_; }

    protected Q_SLOTS:

    //! toggle visibility [overloaded]
    virtual void _toggleVisibility( bool );

    //! find button
    virtual void _updateFindButton( const QString& );

    private Q_SLOTS:

    //! configuration
    void _updateConfiguration( void );

    //! toolbar text position
    void _updateToolButtonStyle( Qt::ToolButtonStyle );

    //! toolbar text position
    void _updateToolButtonIconSize( IconSize::Size );

    //! save configuration
    void _saveMask( void );

    //! send SelectEntries request
    void _selectionRequest( void );

    //! enable all entries button
    void _enableAllEntriesButton( void )
    { allEntriesButton_->setEnabled( true ); }

    //! disable all entries button
    void _disableAllEntriesButton( void )
    { allEntriesButton_->setEnabled( false ); }

    private:

    //! checkboxes
    using CheckBoxMap = QHash<SearchMode, QCheckBox* >;

    //! transition widget
    TransitionWidget* transitionWidget_;

    //! find button
    QPushButton* findButton_;

    //! show all entries button
    QPushButton* allEntriesButton_;

    //! checkboxes
    CheckBoxMap checkboxes_;

    //! select LogEntry according to title
    QCheckBox *titleSelection_;

    //! select LogEntry according to key
    QCheckBox *keywordSelection_;

    //! select LogEntry according to text
    QCheckBox *textSelection_;

    //! select LogEntry according to attachment title
    QCheckBox *attachmentSelection_;

    //! select LogEntry according to color
    QCheckBox *colorSelection_;

    //! selection text widget
    CustomComboBox *editor_;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( SearchPanel::SearchModes )

#endif
