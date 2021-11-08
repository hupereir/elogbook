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

#include "LogbookHtmlHelper.h"
#include "HtmlHeaderNode.h"
#include "HtmlTextNode.h"
#include "LogEntryHtmlHelper.h"

//__________________________________________________________________________________
void LogbookHtmlHelper::print( QIODevice* device )
{
    Debug::Throw( QStringLiteral("LogbookHtmlHelper::print.\n") );

    // check logbook
    Q_CHECK_PTR( logbook_ );

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
    _appendHeader( document, body );
    _appendTable( document, body );
    _appendEntries( document, body );

    // write to document
    device->write( qPrintable( document.toString() ) );

}

//__________________________________________________________________________________
void LogbookHtmlHelper::_appendHeader( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( QStringLiteral("LogbookHtmlHelper::_appendHeader.\n") );

    // check mask
    if( !( mask_ & Logbook::HeaderMask ) ) return;

    // surrounding table
    QDomElement table = parent.appendChild( document.createElement( QStringLiteral("table") ) ).toElement();
    table.setAttribute( QStringLiteral("class"), QStringLiteral("header_outer_table") );

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

    // title
    if( mask_&Logbook::TitleMask )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Title:" ) ) );
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( logbook_->title().toUtf8() ) );
    }

    // comments
    if( mask_&Logbook::CommentsMask && !logbook_->comments().isEmpty() )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Comments:" ) ) );

        QDomElement column = row.appendChild( document.createElement( QStringLiteral("td") ) ).toElement();
        HtmlTextNode( logbook_->comments(), column, document );
    }

    // author
    if( mask_&Logbook::AuthorMasks && !logbook_->author().isEmpty() )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Author:" ) ) );
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( logbook_->author().toUtf8() ) );
    }

    // file
    if( mask_&Logbook::FileMask && !logbook_->file().isEmpty() )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "File:" ) ) );
        QDomElement column = row.appendChild( document.createElement( QStringLiteral("td") ) ).toElement();
        QDomElement ref = column.appendChild( document.createElement( QStringLiteral("a") ) ).toElement();
        ref.setAttribute( QStringLiteral("href"), logbook_->file() );
        ref.appendChild( document.createTextNode( logbook_->file().get().toUtf8() ) );
    }

    // directory
    if( mask_&Logbook::DirectoryMask && !logbook_->directory().isEmpty() )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Directory:" ) ) );
        QDomElement column = row.appendChild( document.createElement( QStringLiteral("td") ) ).toElement();
        QDomElement ref = column.appendChild( document.createElement( QStringLiteral("a") ) ).toElement();
        ref.setAttribute( QStringLiteral("href"), logbook_->directory() );
        ref.appendChild( document.createTextNode( logbook_->directory().get().toUtf8() ) );

        if( !logbook_->checkDirectory() )
        { column.appendChild( document.createTextNode( tr( " (not found)" ) ) ); }

   }

    // creation
    if( logbook_->creation().isValid() && (mask_&Logbook::CreationMask) )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Created:" ) ) );
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( logbook_->creation().toString().toUtf8() ) );
    }

    // modification
    if( logbook_->modification().isValid() && (mask_&Logbook::ModificationMask) )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Modified:" ) ) );
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( logbook_->modification().toString().toUtf8() ) );
    }

    // backup
    if( logbook_->backup().isValid() && (mask_&Logbook::BackupMask) )
    {
        row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( tr( "Backup:" ) ) );
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( logbook_->modification().toString().toUtf8() ) );
    }

    parent.appendChild( document.createElement( QStringLiteral("p") ) );

}


//__________________________________________________________________________________
void LogbookHtmlHelper::_appendTable( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( QStringLiteral("LogbookHtmlHelper::_appendTable.\n") );

    // check entries
    if( entries_.empty() ) return;

    // check mask
    if( !( mask_ & Logbook::TableOfContentMask ) ) return;

    // table
    auto table = parent.appendChild( document.createElement( QStringLiteral("table") ) ).toElement();
    table.setAttribute( QStringLiteral("class"), QStringLiteral("header_inner_table") );

    // header
    auto row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();
    row.appendChild( document.createElement( QStringLiteral("td") ) ).
        appendChild( document.createElement( QStringLiteral("b") ) ).
        appendChild( document.createTextNode( tr( "Title" ) ) );

    row.appendChild( document.createElement( QStringLiteral("td") ) ).
        appendChild( document.createElement( QStringLiteral("b") ) ).
        appendChild( document.createTextNode( tr( "Keyword" ) ) );

    row.appendChild( document.createElement( QStringLiteral("td") ) ).
        appendChild( document.createElement( QStringLiteral("b") ) ).
        appendChild( document.createTextNode( tr( "Created" ) ) );

    row.appendChild( document.createElement( QStringLiteral("td") ) ).
        appendChild( document.createElement( QStringLiteral("b") ) ).
        appendChild( document.createTextNode( tr( "Last Modified" ) ) );

    // loop over entries
    for( const auto& entry:entries_ )
    {
        auto row = table.appendChild( document.createElement( QStringLiteral("tr") ) ).toElement();

        // title
        QDomElement ref = row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createElement( QStringLiteral("a") ) ).toElement();
        ref.setAttribute( QStringLiteral("href"), QStringLiteral( "#" ) + QString::number( entry->creation().unixTime() ) );
        ref.appendChild( document.createTextNode( entry->title().toUtf8() ) );

        // keywords
        ref = row.appendChild( document.createElement( QStringLiteral("td") ) ).toElement();
        if( entry->keywords().contains( currentKeyword_ ) )
        {

            ref.appendChild( document.createTextNode( currentKeyword_.get().toUtf8() ) );

        } else if( entry->hasKeywords() ) {

            int i=0;
            const int keywordCount( entry->keywords().size() );
            for( const auto& keyword:entry->keywords() )
            {
                ref.appendChild( document.createTextNode( keyword.get().toUtf8() ) );
                if( i < keywordCount-1 ) ref.appendChild( document.createElement( QStringLiteral("br") ) );

                // increment counter
                ++i;
            }

        }

        // creation
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( entry->creation().toString().toUtf8() ) );

        // modification
        row.appendChild( document.createElement( QStringLiteral("td") ) ).
            appendChild( document.createTextNode( entry->modification().toString().toUtf8() ) );

    }

    parent.appendChild( document.createElement( QStringLiteral("p") ) );

    return;

}


//__________________________________________________________________________________
void LogbookHtmlHelper::_appendEntries( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( QStringLiteral("LogbookHtmlHelper::_appendEntries.\n") );

    // check entries
    if( entries_.empty() ) return;

    // check entries
    if( !( entryMask_ & LogEntry::All ) ) return;

    // check mask
    if( !( mask_ & Logbook::ContentMask ) ) return;

    LogEntryHtmlHelper helper;
    for( const auto& entry:entries_ )
    {

        helper.setEntry( entry );
        helper.setMask( entryMask_ );
        helper.appendEntry( document, parent );
        parent.appendChild( document.createElement( QStringLiteral("p") ) );

    }

    return;

}
