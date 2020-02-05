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

#include "AttachmentWindow.h"

#include "Application.h"
#include "EditionWindow.h"
#include "IconEngine.h"
#include "IconNames.h"
#include "Logbook.h"
#include "LogEntry.h"
#include "TreeView.h"

#include <QShortcut>
#include <QLayout>

//________________________________________
AttachmentWindow::AttachmentWindow( QWidget* parent ):
    CustomDialog( parent, CloseButton )
{

    Debug::Throw( QStringLiteral("AttachmentWindow::AttachmentWindow.\n") );
    setWindowTitle( tr( "Attachments" ) );
    setOptionName( QStringLiteral("ATTACHMENT_WINDOW") );

    layout()->setMargin(0);
    buttonLayout().setMargin(5);

    mainLayout().addWidget( frame_ = new AttachmentFrame( this, true ) );
    connect( frame_, &AttachmentFrame::attachmentSelected, this, &AttachmentWindow::_displayEntry );

    frame_->contextMenu().insertAction( &frame_->newAction(), &frame_->list().findAction() );

    // shortcuts
    connect( new QShortcut( QKeySequence::Quit, this ), &QShortcut::activated, qApp, &QApplication::closeAllWindows );
    connect( new QShortcut( QKeySequence::Close, this ), &QShortcut::activated, this, &AttachmentWindow::close );

    uniconifyAction_ = new QAction( IconEngine::get( IconNames::Attach ), tr( "Attachments" ), this );
    uniconifyAction_->setToolTip( tr( "Raise application main window" ) );
    connect( uniconifyAction_, &QAction::triggered, this, &AttachmentWindow::uniconify );

};

//________________________________________
void AttachmentWindow::show()
{
    Debug::Throw( QStringLiteral("AttachmentWindow::show.\n") );
    centerOnWidget( qApp->activeWindow());
    QWidget::show();
    QWidget::raise();
}

//________________________________________
void AttachmentWindow::uniconify()
{
    CustomDialog::uniconify();
    frame_->list().setFocus();
}

//________________________________________
void AttachmentWindow::_displayEntry( Attachment& attachment )
{

    Debug::Throw( QStringLiteral("AttachmentWindow::_displayEntry.\n"));

    // retrieve associated entry
    LogEntry *entry( attachment.entry() );

    // check if entry is visible
    if( entry && !entry->isSelected() ) entry->setFindSelected( true );
    emit entrySelected( entry );

}
