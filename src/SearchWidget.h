#ifndef SearchWidget_h
#define SearchWidget_h

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.
s*
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

#include <QCheckBox>
#include <QHash>
#include <QPushButton>

class CustomComboBox;

//* selects entries from keyword/title/text/...
class SearchWidget: public QWidget, private Base::Counter<SearchWidget>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit SearchWidget( QWidget* );

    //* search mode enumeration
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

    //*@name accessors
    //@{

    //* editor
    CustomComboBox& editor() const
    { return *editor_; }

    //@}

    //*@name modifiers
    //@{

    //* change text
    void setText( const QString& );

    //@}

    Q_SIGNALS:

    //* emitted when the Find button is pressed
    void selectEntries( QString, SearchWidget::SearchModes );

    //* emitted when the Show All button is pressed
    void showAllEntries();

    public Q_SLOTS:

    //* take action when at least one match is found
    void matchFound();

    //* take action when no match is found
    void noMatchFound();

    protected:

    //* change event
    void changeEvent( QEvent* ) override;

    private Q_SLOTS:

    //* find button
    void _updateFindButton( const QString& );

    //* restore palette
    void _restorePalette();

    //* configuration
    void _updateConfiguration();

    //* save configuration
    void _saveMask();

    //* send SelectEntries request
    void _selectionRequest();

    //* enable all entries button
    void _enableAllEntriesButton()
    { allEntriesButton_->setEnabled( true ); }

    //* disable all entries button
    void _disableAllEntriesButton()
    { allEntriesButton_->setEnabled( false ); }

    private:

    //* create not found palette
    void _updateNotFoundPalette();

    //* checkboxes
    using CheckBoxMap = QHash<SearchMode, QCheckBox* >;

    //* find button
    QPushButton* findButton_ = nullptr;

    //* show all entries button
    QPushButton* allEntriesButton_ = nullptr;

    //* checkboxes
    CheckBoxMap checkboxes_;

    //* selection text widget
    CustomComboBox *editor_ = nullptr;

    //* not found palette
    QPalette notFoundPalette_;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( SearchWidget::SearchModes )

#endif
