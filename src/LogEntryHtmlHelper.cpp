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
    Debug::Throw( "LogEntryHtmlHelper::print.\n" );

    // check entry
    Q_CHECK_PTR( entry_ );

    // do nothing if mask is empty
    if( !mask_ ) return;

    // dump header/style
    QDomDocument document( "html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"DTD/xhtml1-strict.dtd\"" );

    // html
    QDomElement html = document.appendChild( document.createElement( "html" ) ).toElement();
    html.setAttribute( "xmlns", "http://www.w3.org/1999/xhtml" );

    // head
    HtmlHeaderNode( html, document );

    // body
    QDomElement body = html.appendChild( document.createElement( "body" ) ).toElement();
    appendEntry( document, body );

    device->write( qPrintable( document.toString() ) );

}

//__________________________________________________________________________________
void LogEntryHtmlHelper::appendEntry( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( "LogEntryHtmlHelper::appendEntry.\n" );

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

    Debug::Throw( "LogEntryHtmlHelper::_appendHeader.\n" );

    // check mask
    if( !( mask_ & LogEntry::HeaderMask ) ) return;

    // surrounding table
    QDomElement table = parent.appendChild( document.createElement( "table" ) ).toElement();
    table.setAttribute( "class", "header_outer_table" );

    QColor color( Base::Color( entry_->color() ) );
    if( color.isValid() )
    {
        QString buffer;
        QTextStream( &buffer ) << "border: 1px solid" << color.name();
        table.setAttribute( "style", buffer );
    }

    QDomElement column = table.
        appendChild( document.createElement( "tr" ) ).
        appendChild( document.createElement( "td" ) ).
        toElement();
    column.setAttribute( "class", "header_column" );
    table = column.
        appendChild( document.createElement( "table" ) ).
        toElement();
    table.setAttribute( "class", "header_inner_table" );
    QDomElement row;

    // keyword
    if( mask_&LogEntry::KeywordMask )
    {
        bool first( true );
        for( const auto& keyword:entry_->keywords() )
        {
            row = table.appendChild( document.createElement( "tr" ) ).toElement();
            row.appendChild( document.createElement( "td" ) );

            if( first )
            {

                row.appendChild( document.createElement( "td" ) )
                    .appendChild( document.createTextNode( tr( "Keyword:" ) ) );
                first = false;

            } else row.appendChild( document.createElement( "td" ) );

            row.appendChild( document.createElement( "td" ) ).
                appendChild( document.createTextNode( keyword.get() ) );
        }

    }

    // title
    if( !entry_->title().isEmpty() && (mask_&LogEntry::TitleMask) )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Title:" ) ) );

        QDomElement ref = row.
            appendChild( document.createElement( "td" ) ).
            appendChild( document.createElement( "a" ) ).
            toElement();
        ref.setAttribute( "name", QString::number( entry_->creation() ) );
        ref.appendChild( document.createTextNode( entry_->title() ) );
    }

    // author
    if( !entry_->author().isEmpty() && (mask_&LogEntry::AuthorMask) )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Author:" ) ) );
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( entry_->author() ) );

    }

    // creation
    if( entry_->creation().isValid() && (mask_&LogEntry::CreationMask) )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Created:" ) ) );
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( entry_->creation().toString() ) );

    }

    // modification
    if( entry_->modification().isValid() && (mask_&LogEntry::ModificationMask) )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Modified:" ) ) );
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( entry_->modification().toString() ) );

    }

}

//__________________________________________________________________________________
void LogEntryHtmlHelper::_appendBody( QDomDocument& document, QDomElement& parent )
{
    Debug::Throw( "LogEntryHtmlHelper::_appendBody.\n" );

    // check mask
    if( !(mask_&LogEntry::TextMask ) ) return;

    // paragraph node
    QDomElement par = parent.appendChild( document.createElement("p") ).toElement();

    // get formats
    Format::TextFormatBlock::List formats( entry_->formats() );

    // text
    const QString text( entry_->text() );

    // loop over index position
    QString buffer;
    for( int index = 0; index < text.size(); )
    {
        Debug::Throw() << "index: " << index << " position: " << index << endl;
        Format::TextFormatBlock::List::iterator iter( std::find_if(
            formats.begin(),
            formats.end(),
            Format::TextFormatBlock::ContainsFTor( index ) ) );
        if( iter == formats.end() || iter->isEmpty() )
        {
            QTextStream( &buffer ) << text[index];
            index ++;

        } else {

            Debug::Throw() << *iter << endl;

            // write previous text
            HtmlTextNode( buffer, parent, document );
            buffer.clear();

            // open new element define format
            QDomElement localNode( parent );
            if( iter->format() & Format::Underline ) localNode = localNode.appendChild( document.createElement( "u" ) ).toElement();
            if( iter->format() & Format::Italic ) localNode = localNode.appendChild( document.createElement( "i" ) ).toElement();
            if( iter->format() & Format::Bold ) localNode = localNode.appendChild( document.createElement( "b" ) ).toElement();
            if( iter->format() & Format::Strike ) localNode = localNode.appendChild( document.createElement( "s" ) ).toElement();
            if( iter->color().isValid() )
            {
                localNode = localNode.appendChild( document.createElement( "font" ) ).toElement();
                localNode.setAttribute( "color", iter->color().name() );
            }

            while( index < text.size() && (int)index < iter->end() )
            {
                QTextStream( &buffer ) << text[index];
                index++;
            }

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
    Debug::Throw( "LogEntryHtmlHelper::_appendAttachments.\n" );

    // check mask
    if( !(mask_&LogEntry::AttachmentsMask) ) return;

    // check attachments
    Base::KeySet<Attachment> attachments( entry_ );
    if( attachments.empty() ) return;

    // paragraph node
    QDomElement par = parent.
        appendChild( document.createElement("p") ).
        appendChild( document.createElement( "b" ) ).toElement();
    HtmlTextNode( tr( "Attachments:" ), par, document );

    // table
    QDomElement table = parent.appendChild( document.createElement( "table" ) ).toElement();
    table.setAttribute( "class", "header_inner_table" );

    // header
    QDomElement row = table.appendChild( document.createElement( "tr" ) ).toElement();
    row.appendChild( document.createElement( "td" ) ).
        appendChild( document.createElement( "b" ) ).
        appendChild( document.createTextNode( tr( "Location" ) ) );

    row.appendChild( document.createElement( "td" ) ).
        appendChild( document.createElement( "b" ) ).
        appendChild( document.createTextNode( tr( "Comments" ) ) );

    // attachments
    for( const auto& attachment:attachments )
    {

        // file
        QDomElement row = table.appendChild( document.createElement( "tr" ) ).toElement();
        QDomElement ref = row.
            appendChild( document.createElement( "td" ) ).
            appendChild( document.createElement( "a" ) ).
            toElement();
        if( attachment->isUrl() ) ref.setAttribute( "href", attachment->file() );
        else ref.setAttribute( "href", QString("file:") + attachment->file() );
        ref.appendChild( document.createTextNode( attachment->file() ) );

        // comments
        if( !attachment->comments().isEmpty() )
        {
            QDomElement column = row.appendChild( document.createElement( "td" ) ).toElement();
            HtmlTextNode( attachment->comments(), column, document );
        }

    }

}
