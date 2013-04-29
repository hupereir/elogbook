// $Id$
#ifndef AskForSaveDialog_h
#define AskForSaveDialog_h

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

/*!
\file AskForSaveDialog.h
\brief QDialog used to ask if modifications of a file should be saved
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

#include "BaseDialog.h"
#include "Counter.h"
#include "File.h"

//! QDialog used to ask if modifications of a file should be saved
class AskForSaveDialog: public BaseDialog, public Counter
{

    //! Qt macro
    Q_OBJECT

    public:

    //! return codes
    /*!
    the enumeration is used both to interpret the result and to decide which
    buttons are to be shown in the dialog.
    */
    enum ReturnCode {

        //! file is to be saved
        YES = 1<<0,

        //! file is not to be saved
        NO = 1<<1,

        //! all files are to be saved
        ALL = 1<<2,

        //! action is canceled
        CANCEL = 1<<3,

        //! all buttons
        DEFAULT = YES|NO|CANCEL

    };

    //! constructor
    AskForSaveDialog( QWidget* parent, const QString& message, const unsigned int& buttons = DEFAULT );

    private slots:

    //! save changes
    void _yes( void )
    { done( YES ); }

    //! discard changes
    void _no( void )
    { done( NO ); }

    //! save for all modified entries
    void _all( void )
    { done( ALL ); }

    //! cancel action
    void _cancel( void )
    { done( CANCEL ); }

};

#endif
