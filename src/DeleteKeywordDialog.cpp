
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
* ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA   02111-1307 USA
*
*
*******************************************************************************/

/*!
\file DeleteKeywordDialog.cpp
\brief Delete attachment popup dialog
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "DeleteKeywordDialog.h"
#include "Debug.h"

#include <QtGui/QButtonGroup>
#include <QtGui/QGroupBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#include <cassert>

//_____________________________________________________
DeleteKeywordDialog::DeleteKeywordDialog( QWidget* parent, const QList<Keyword>& keywords, const bool& has_entries ):
CustomDialog( parent )
{

    Debug::Throw( "DeleteKeywordDialog::DeleteKeywordDialog.\n" );

    assert( !keywords.empty() );

    setWindowTitle( "Delete Keyword - elogbook" );
    QString buffer;
    QTextStream what( &buffer );
    if( keywords.size() == 1 ) what << "Delete keyword " << keywords.front() << " ?";
    else {

        what << "Delete keywords " << endl << "  ";

        unsigned int max_keywords = 10;
        unsigned int index(0);
        foreach( const Keyword& keyword, keywords )
        {
            what << keyword << " ";
            index++;
            if( index >= max_keywords )
            {
                index = 0;
                what << endl << "  ";
            }
        }
        what << "?";

    }

    mainLayout().addWidget( new QLabel( buffer, this ) );

    QGroupBox *box = new QGroupBox( this );
    mainLayout().addWidget( box );
    box->setLayout( new QVBoxLayout() );
    box->layout()->setMargin(5);
    box->layout()->setSpacing(5);


    // radio buttons
    QButtonGroup* group = new QButtonGroup( this );

    box->layout()->addWidget( moveRadioButton_ = new QRadioButton( "Move entries to parent keyword", box ) );
    moveRadioButton_->setToolTip( "Select this button to move entries associated to this keyword to the parent keyword." );
    group->addButton( moveRadioButton_ );

    box->layout()->addWidget( deleteRadioButton_ = new QRadioButton( "Delete entries", box ) );
    deleteRadioButton_->setToolTip( "Select this button to delete entries associated to this keyword." );
    group->addButton( deleteRadioButton_ );

    group->setExclusive( true );
    moveRadioButton_->setChecked( true );

    if( !has_entries ) box->setEnabled( false );

    adjustSize();

}
