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
#include "LogEntryHtmlHelper.h"
#include "HtmlHeaderNode.h"
#include "HtmlTextNode.h"

//__________________________________________________________________________________
void LogbookHtmlHelper::print( QIODevice* device )
{
    Debug::Throw( "LogbookHtmlHelper::print.\n" );

    // check logbook
    Q_CHECK_PTR( logbook_ );

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
    _appendHeader( document, body );
    _appendTable( document, body );
    _appendEntries( document, body );

    // write to document
    device->write( qPrintable( document.toString() ) );

}

//__________________________________________________________________________________
void LogbookHtmlHelper::_appendHeader( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( "LogbookHtmlHelper::_appendHeader.\n" );

    // check mask
    if( !( mask_ & Logbook::HeaderMask ) ) return;

    // surrounding table
    QDomElement table = parent.appendChild( document.createElement( "table" ) ).toElement();
    table.setAttribute( "class", "header_outer_table" );

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

    // title
    if( mask_&Logbook::TitleMask )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Title:" ) ) );
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( logbook_->title() ) );
    }

    // comments
    if( mask_&Logbook::CommentsMask && !logbook_->comments().isEmpty() )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Comments:" ) ) );

        QDomElement column = row.appendChild( document.createElement( "td" ) ).toElement();
        HtmlTextNode( logbook_->comments(), column, document );
    }

    // author
    if( mask_&Logbook::AuthorMasks && !logbook_->author().isEmpty() )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Author:" ) ) );
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( logbook_->author() ) );
    }

    // file
    if( mask_&Logbook::FileMask && !logbook_->file().isEmpty() )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "File:" ) ) );
        QDomElement column = row.appendChild( document.createElement( "td" ) ).toElement();
        QDomElement ref = column.appendChild( document.createElement( "a" ) ).toElement();
        ref.setAttribute( "href", logbook_->file() );
        ref.appendChild( document.createTextNode( logbook_->file() ) );
    }

    // directory
    if( mask_&Logbook::DirectoryMask && !logbook_->directory().isEmpty() )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Directory:" ) ) );
        QDomElement column = row.appendChild( document.createElement( "td" ) ).toElement();
        QDomElement ref = column.appendChild( document.createElement( "a" ) ).toElement();
        ref.setAttribute( "href", logbook_->directory() );
        ref.appendChild( document.createTextNode( logbook_->directory() ) );

        if( !logbook_->checkDirectory() )
        { column.appendChild( document.createTextNode( tr( " (not found)" ) ) ); }

   }

    // creation
    if( logbook_->creation().isValid() && (mask_&Logbook::CreationMask) )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Created:" ) ) );
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( logbook_->creation().toString() ) );
    }

    // modification
    if( logbook_->modification().isValid() && (mask_&Logbook::ModificationMask) )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Modified:" ) ) );
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( logbook_->modification().toString() ) );
    }

    // backup
    if( logbook_->backup().isValid() && (mask_&Logbook::BackupMask) )
    {
        row = table.appendChild( document.createElement( "tr" ) ).toElement();
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( tr( "Backup:" ) ) );
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( logbook_->modification().toString() ) );
    }

    parent.appendChild( document.createElement( "p" ) );

}


//__________________________________________________________________________________
void LogbookHtmlHelper::_appendTable( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( "LogbookHtmlHelper::_appendTable.\n" );

    // check entries
    if( entries_.empty() ) return;

    // check mask
    if( !( mask_ & Logbook::TableOfContentMask ) ) return;

    // table
    QDomElement table = parent.appendChild( document.createElement( "table" ) ).toElement();
    table.setAttribute( "class", "header_inner_table" );

    // header
    QDomElement row = table.appendChild( document.createElement( "tr" ) ).toElement();
    row.appendChild( document.createElement( "td" ) ).
        appendChild( document.createElement( "b" ) ).
        appendChild( document.createTextNode( tr( "Keyword" ) ) );

    row.appendChild( document.createElement( "td" ) ).
        appendChild( document.createElement( "b" ) ).
        appendChild( document.createTextNode( tr( "Title" ) ) );

    row.appendChild( document.createElement( "td" ) ).
        appendChild( document.createElement( "b" ) ).
        appendChild( document.createTextNode( tr( "Created" ) ) );

    row.appendChild( document.createElement( "td" ) ).
        appendChild( document.createElement( "b" ) ).
        appendChild( document.createTextNode( tr( "Last Modified" ) ) );

    // loop over entries
    for( const auto& entry:entries_ )
    {
        QDomElement row = table.appendChild( document.createElement( "tr" ) ).toElement();

        // keyword
        QDomElement ref = row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createElement( "a" ) ).toElement();
        ref.setAttribute( "href", QString( "#" ) + QString::number( entry->creation() ) );

        // FIXME: should add either all keywords or current
        if( entry->hasKeywords() )
        { ref.appendChild( document.createTextNode( entry->keywords().begin()->get() ) ); }

        // title
        ref = row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createElement( "a" ) ).toElement();
        ref.setAttribute( "href", QString( "#" ) + QString::number( entry->creation() ) );
        ref.appendChild( document.createTextNode( entry->title() ) );

        // creation
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( entry->creation().toString() ) );

        // modification
        row.appendChild( document.createElement( "td" ) ).
            appendChild( document.createTextNode( entry->modification().toString() ) );

    }

    parent.appendChild( document.createElement( "p" ) );

    return;

}


//__________________________________________________________________________________
void LogbookHtmlHelper::_appendEntries( QDomDocument& document, QDomElement& parent )
{

    Debug::Throw( "LogbookHtmlHelper::_appendEntries.\n" );

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
        parent.appendChild( document.createElement( "p" ) );

    }

    return;

}
