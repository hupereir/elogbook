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

#include "NewAttachmentDialog.h"
#include "Debug.h"
#include "GridLayout.h"
#include "LineEditor.h"
#include "QtUtil.h"

#include <QLayout>
#include <QLabel>

//_____________________________________________________
NewAttachmentDialog::NewAttachmentDialog( QWidget* parent ):
Dialog( parent )
{

    Debug::Throw( QStringLiteral("NewAttachmentDialog::NewAttachmentDialog.\n") );
    setWindowTitle( tr( "New Attachment" ) );

    GridLayout* gridLayout = new GridLayout;
    QtUtil::setMargin(gridLayout, 0);
    gridLayout->setSpacing(5);
    gridLayout->setMaxCount(2);
    gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
    mainLayout().addLayout( gridLayout, 0 );

    // source file
    gridLayout->addWidget( new QLabel( tr( "Source file:" ), this ) );
    gridLayout->addWidget( fileEditor_ = new BrowsedLineEditor( this ) );
    fileEditor_->setFileMode( QFileDialog::ExistingFile );
    fileEditor_->setToolTip( tr( "Attachment source file (or URL) to be stored in logbook" ) );

    gridLayout->addWidget( new QLabel( tr( "Destination directory:" ), this ) );
    gridLayout->addWidget( destinationDirectoryEditor_ = new BrowsedLineEditor( this ) );
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    destinationDirectoryEditor_->setFileMode( QFileDialog::DirectoryOnly );
    #else
    destinationDirectoryEditor_->setFileMode( QFileDialog::Directory );
    #endif
    destinationDirectoryEditor_->setToolTip( tr( "Attachment directory where attached file is stored (either copied or linked)" ) );

    // action
    gridLayout->addWidget( new QLabel( tr( "Action:" ), this ) );
    gridLayout->addWidget( actionComboBox_ = new QComboBox( this ) );
    QString actions[] = { tr( "Copy" ), tr( "Link" ), QString() };
    for( int i=0; !actions[i].isEmpty(); i++ )
    { actionComboBox_->addItem( actions[i] ); }

    actionComboBox_->setCurrentIndex( 0 );
    actionComboBox_->setToolTip( tr( "Action to perform in order to save the attachment" ) );

    // is url
    gridLayout->addWidget(
        urlCheckBox_ = new QCheckBox( tr( "Selected attachment is a web adress" ), this ),
        gridLayout->currentRow(), 1, 1, 1 );

    connect( urlCheckBox_, &QAbstractButton::toggled, this, &NewAttachmentDialog::_urlChanged );

    gridLayout->setColumnStretch( 1, 1 );

    // comments
    mainLayout().addWidget( new QLabel( tr( "Comments:" ), this ), 0 );
    mainLayout().addWidget( commentsEditor_ = new TextEditor( this ), 1 );
    commentsEditor_->setToolTip( tr( "Attachment comments" ) );

    adjustSize();

}

//____________________________________________________
File NewAttachmentDialog::file() const
{
    File out( fileEditor_->editor().text() );
    return isUrl() ? out : out.expand();
}

//____________________________________________________
File NewAttachmentDialog::destinationDirectory() const
{ return File( destinationDirectoryEditor_->editor().text() ).expand(); }

//____________________________________________________
bool NewAttachmentDialog::isUrl() const
{ return urlCheckBox_->isChecked(); }

//____________________________________________________
QString NewAttachmentDialog::comments() const
{
    Debug::Throw( QStringLiteral("NewAttachmentDialog::GetComments.\n") );
    return commentsEditor_->toPlainText();
}

//____________________________________________________
Attachment::Command NewAttachmentDialog::action() const
{ return actionComboBox_->currentText() == tr( "Copy" ) ? Attachment::Command::CopyVersion: Attachment::Command::LinkVersion; }

//____________________________________________________
void NewAttachmentDialog::setFile( const File& file )
{

    Debug::Throw( QStringLiteral("NewAttachmentDialog::SetFile.\n") );
    fileEditor_->setFile( file );

}

//____________________________________________________
void NewAttachmentDialog::setDestinationDirectory( const File& file )
{
    Debug::Throw( QStringLiteral("NewAttachmentDialog::SetDestinationDirectory.\n") );
    destinationDirectoryEditor_->setFile( file );
}

//____________________________________________________
void NewAttachmentDialog::setIsUrl( bool value )
{
    Debug::Throw( QStringLiteral("NewAttachmentDialog::setType.\n") );
    urlCheckBox_->setChecked( value );
}

//____________________________________________________
void NewAttachmentDialog::setAction( Attachment::Command command )
{
    Debug::Throw( QStringLiteral("NewAttachmentDialog::SetAction.\n") );
    QString actionString = ( command == Attachment::Command::CopyVersion ) ? tr( "Copy" ) : tr( "Link" );
    actionComboBox_->setCurrentIndex( actionComboBox_->findText( actionString ) );
}

//____________________________________________________
void NewAttachmentDialog::setComments( const QString& comments )
{
    Debug::Throw( QStringLiteral("NewAttachmentDialog::SetComments.\n") );
    commentsEditor_->setPlainText( comments );
}

//____________________________________________________
void NewAttachmentDialog::_urlChanged( bool value )
{
    bool enabled = !value;
    destinationDirectoryEditor_->setEnabled( enabled );
    actionComboBox_->setEnabled( enabled );
    return;
}
