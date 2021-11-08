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

#include "LogEntryHtmlHelper.h"
#include "Attachment.h"
#include "Color.h"
#include "ColorMenu.h"
#include "HtmlHeaderNode.h"
#include "HtmlTextNode.h"
#include "LogEntry.h"
#include "Logbook.h"
#include "TextFormat.h"
#include "TextPosition.h"


//__________________________________________________________________________________
void LogEntryHtmlHelper::print( QIODevice* device )
{
    Debug::Throw( QStringLiteral("LogEntryHtmlHelper::print.\n") );

    // check entry
    Q_CHECK_PTR( entry_ );

    // do nothing if mask is empty
    if( !mask_ ) return;

    // dump header/style
    QDomDocument document( QStringLiteral("html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"DTD/xhtml1-strict.dtd\"") );

    // html
    QDomElement html = document.appendChild( document.createElement( QStringLiteral("html") ) ).toElement();
    html.setAttribute( QStringLiteral("xmlns"), QStringLiteral("http://www.w3.org/1999/xhtml") );

    // head
    HtmlHeaderNode( html, document );

    // body
    QDomElement body = html.appendChild( document.createElement( QStringLiteral("body") ) ).toElement();
    appendEntry( document, body );

    device->write( qPrintable( document.toString() ) );

}

//__________________________________________________________________________________
void LogEntryHtmlHelper::appendEntry( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( QStringLiteral("LogEntryHtmlHelper::appendEntry.\n") );

    // check entry
    Q_CHECK_PTR( entry_ );

    // do nothing if mask is empty
    if( !mask_ ) return;

    // print everything
    _appendHeader( document, parent );
    _appendBody( document, parent );
    _appendAttachments( document, parent );
}

//__________________________________________________________________________________
void LogEntryHtmlHelper::_appendHeader( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( QStringLiteral("LogEntryHtmlHelper::_appendHeader.\n") );

    // check mask
    if( !( mask_ & LogEntry::HeaderMask ) ) return;

    // surrounding table
    QDomElement table = parent.appendChild( document.createElement( QStringLiteral("table") ) ).toElement();
    table.setAttribute( QStringLiteral("class"), QStringLiteral("header_outer_table") );

    QColor color( Base::Color( entry_->color() ) );
    if( color.isValid() )
    {
        QString buffer;
        QTextStream( &buffer ) << "border: 1px solid" << color.name();
        table.setAttribute( QStringLiteral("style"), buffer );
    }

    QDomElement column = table.
        appendChild( document.createElement( QStringLiteral("tr") ) ).
        appendChild( document.createElement( QStringLiteral("td") ) ).
        toElement();
    column.setAttribute( QStringLiteral("class"), QStringLiteral("header_column") );
    table = column.
        appendChild( document.createElement( QStringLiteral("table") ) ).
        toElement();
    table.setAttribute( QStringLiteral("class"), QStringLiteral("header_inner_table") );
    QDomElement row;

    // keyword
    if( (mask_&LogEntry::KeywordMask) && entry_->hasKeywords() )
    {

        const auto& keywords( entry_->keywords() );
        if( keywords.size() > 1 )
        {
            bool first( true );
            for( const auto& keyword:entry_->keywords() )
            {
                row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
                if( first )
                {

                    row.appendChild( document.createElement( QStringLiteral("td") ) )
                        .appendChild( document.createTextNode( tr( "Keywords:" ) ) );
                    first = false;

                } else row.appendChild( document.createElement( QStringLiteral("td") ) );

                row.appendChild( document.createElement( QStringLiteral("td") ) ).
                    appendChild( document.createTextNode( keyword.get().toUtf8() ) );
            }
        } else {

            const auto& keyword( *entry_->keywords().begin() );
            row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
            row.appendChild( document.createElement( QStringLiteral("td") ) )
                .appendChild( document.createTextNode( tr( "Keyword:" ) ) );
            row.appendChild( document.createElement( QStringLiteral("td") ) ).
                appendChild( document.createTextNode( keyword.get().toUtf8() ) );

        }

    }

    // title
    if( !entry_->title().isEmpty() && (mask_&LogEntry::TitleMask) )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Title:" ) ) );

        QDomElement ref = row.
            appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createElement( QStringLiteral("a") ) ).
            toElement();
        ref.setAttribute( QStringLiteral("name"), QString::number( entry_->creation().unixTime() ) );
        ref.appendChild( document.createTextNode( entry_->title().toUtf8() ) );
    }

    // author
    if( !entry_->author().isEmpty() && (mask_&LogEntry::AuthorMask) )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Author:" ) ) );
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( entry_->author().toUtf8() ) );

    }

    // creation
    if( entry_->creation().isValid() && (mask_&LogEntry::CreationMask) )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Created:" ) ) );
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( entry_->creation().toString().toUtf8() ) );

    }

    // modification
    if( entry_->modification().isValid() && (mask_&LogEntry::ModificationMask) )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Modified:" ) ) );
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( entry_->modification().toString().toUtf8() ) );

    }

}

//__________________________________________________________________________________
void LogEntryHtmlHelper::_appendBody( QDomDocument& document, QDomElement& parent )
{
    Debug::Throw( QStringLiteral("LogEntryHtmlHelper::_appendBody.\n") );

    // check mask
    if( !(mask_&LogEntry::TextMask ) ) return;

    // paragraph node
    QDomElement par = parent.appendChild( document.createElement(QStringLiteral("p")) ).toElement();

    // get formats
    TextFormat::Block::List formats( entry_->formats() );

    // text
    const QString text( entry_->text() );

    // loop over index position
    QString buffer;
    for( int index = 0; index < text.size(); )
    {
        Debug::Throw() << "index: " << index << " position: " << index << Qt::endl;
        auto iter( std::find_if( formats.begin(), formats.end(), TextFormat::Block::ContainsFTor( index ) ) );
        if( iter == formats.end() || iter->isEmpty() )
        {
            QTextStream( &buffer ) << text[index];
            index ++;

        } else {

            Debug::Throw() << *iter << Qt::endl;

            // write previous text
            HtmlTextNode( buffer, parent, document );
            buffer.clear();

            // open new element define format
            QDomElement localNode( parent );
            if( iter->format() & TextFormat::Underline ) localNode = localNode.appendChild( document.createElement( QStringLiteral("u") ) ).toElement();
            if( iter->format() & TextFormat::Italic ) localNode = localNode.appendChild( document.createElement( QStringLiteral("i") ) ).toElement();
            if( iter->format() & TextFormat::Bold ) localNode = localNode.appendChild( document.createElement( QStringLiteral("b") ) ).toElement();
            if( iter->format() & TextFormat::Strike ) localNode = localNode.appendChild( document.createElement( QStringLiteral("s") ) ).toElement();
            if( iter->foreground().isValid() || iter->background().isValid() )
            {
                localNode = localNode.appendChild( document.createElement( QStringLiteral("font") ) ).toElement();
                if(  iter->foreground().isValid() && iter->background().isValid() )
                {
                    localNode.setAttribute( QStringLiteral("style"), QString("color:%1;background-color:%2;").arg(iter->foreground().name(),iter->background().name()) );
                } else if( iter->foreground().isValid() ) {
                    localNode.setAttribute( QStringLiteral("style"), QString("color:%1;").arg(iter->foreground().name()) );
                } else if( iter->background().isValid() ) {
                    localNode.setAttribute( QStringLiteral("style"), QString("background-color:%1;").arg(iter->background().name()) );
                }
            }

            for( ; index < text.size() && (int)index < iter->end(); ++index )
            { QTextStream( &buffer ) << text[index]; }

            // remove format from list
            formats.erase( iter );

            // process text
            HtmlTextNode( buffer, localNode, document );
            buffer.clear();

        }
    }

    HtmlTextNode( buffer, parent, document );

}

//__________________________________________________________________________________
void LogEntryHtmlHelper::_appendAttachments( QDomDocument& document, QDomElement& parent )
{
    Debug::Throw( QStringLiteral("LogEntryHtmlHelper::_appendAttachments.\n") );

    // check mask
    if( !(mask_&LogEntry::AttachmentsMask) ) return;

    // check attachments
    Base::KeySet<Attachment> attachments( entry_ );
    if( attachments.empty() ) return;

    // paragraph node
    auto par = parent.
        appendChild( document.createElement(QStringLiteral("p")) ).
        appendChild( document.createElement( QStringLiteral("b") ) ).toElement();
    HtmlTextNode( tr( "Attachments:" ), par, document );

    // table
    auto table = parent.appendChild( document.createElement( QStringLiteral("table") ) ).toElement();
    table.setAttribute( QStringLiteral("class"), QStringLiteral("header_inner_table") );

    // header
    auto row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
    row.appendChild( document.createElement( QStringLiteral("td") ) ).
        appendChild( document.createElement( QStringLiteral("b") ) ).
        appendChild( document.createTextNode( tr( "Location" ) ) );

    row.appendChild( document.createElement( QStringLiteral("td") ) ).
        appendChild( document.createElement( QStringLiteral("b") ) ).
        appendChild( document.createTextNode( tr( "Comments" ) ) );

    // attachments
    for( const auto& attachment:attachments )
    {

        // file
        auto row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        auto ref = row.
            appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createElement( QStringLiteral("a") ) ).
            toElement();
        if( attachment->isUrl() ) ref.setAttribute( QStringLiteral("href"), attachment->file() );
        else ref.setAttribute( QStringLiteral("href"), QStringLiteral("file:") + attachment->file() );
        ref.appendChild( document.createTextNode( attachment->file().get().toUtf8() ) );

        // comments
        if( !attachment->comments().isEmpty() )
        {
            QDomElement column = row.appendChild( document.createElement( QStringLiteral("td") ) ).toElement();
            HtmlTextNode( attachment->comments(), column, document );
        }

    }

}
