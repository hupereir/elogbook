#ifndef LogEntry_h
#define LogEntry_h

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

#include "Color.h"
#include "Counter.h"
#include "Key.h"
#include "Keyword.h"
#include "TextFormatBlock.h"
#include "TimeStamp.h"
#include "XmlOptions.h"

#include <QDomElement>
#include <QDomDocument>

//* log file entry manipulation object
class LogEntry:public Counter, public Base::Key
{

    public:

    //* configuration mask
    enum MaskFlag
    {
        KeywordMask = 1<<0,
        TitleMask = 1<<1,
        AuthorMask = 1<<2,
        CreationMask = 1<<3,
        ModificationMask = 1<<4,
        TextMask = 1<<5,
        AttachmentsMask = 1<<6,
        HeaderMask = TitleMask | KeywordMask | CreationMask | ModificationMask | AuthorMask,
        All = HeaderMask | TextMask | AttachmentsMask
    };

    Q_DECLARE_FLAGS( Mask, MaskFlag )

    //* empty creator
    LogEntry( void );

    //* constructor from DOM
    LogEntry( const QDomElement& );

    //* destructor
    ~LogEntry( void );

    //* DomElement
    QDomElement domElement( QDomDocument& ) const;

    //* return a new entry cloned from this
    LogEntry *clone( void ) const;

    //* mime type (for drag and drop)
    static const QString MimeType;

    //*@name attributes
    //@{

    //* set modification_ to _now_
    void modified( void );

    //* creation TimeStamp
    void setCreation( const TimeStamp stamp )
    { creation_ = stamp; }

    //* creation TimeStamp
    TimeStamp creation() const
    { return creation_; }

    //* modification TimeStamp
    void setModification( const TimeStamp stamp )
    { modification_ = stamp; }

    //* modification TimeStamp
    TimeStamp modification() const
    { return modification_; }

    //* Log entry title
    void setTitle( const QString& title )
    { title_ = title; }

    //* LogEntry title
    QString title( void ) const
    { return title_; }

    //* Log entry keyword
    void setKeyword( const Keyword& keyword )
    { keyword_ = keyword; }

    //* Log entry keyword
    const Keyword& keyword( void ) const
    { return keyword_; }

    //* Log entry author
    void setAuthor( const QString& author )
    { author_ = author; }

    //* Log entry last author
    QString author( void ) const
    { return author_; }

    //* LogEntry color
    void setColor(  QColor color )
    { color_ = color; }

    //* LogEntry color
    Base::Color color( void ) const
    { return color_; }

    //* add TextFormatBlock
    void addFormat( const Format::TextFormatBlock& format )
    { formats_.append(format); }

    //* entry text format
    void setFormats( const Format::TextFormatBlock::List& formats )
    { formats_ = formats; }

    //* entry text format
    const Format::TextFormatBlock::List& formats( void ) const
    { return formats_; }

    //* clears LogEntry text
    void setText( const QString& text )
    { text_ = text; }

    //* LogEntry text
    const QString& text( void ) const
    { return text_; }

    //* returns true if entry title matches buffer
    bool matchTitle( const QString& buf ) const;

    //* returns true if entry keyword matches buffer
    bool matchKeyword( const QString& buf ) const;

    //* returns true if entry text matches buffer
    bool matchText(  const QString& buf ) const;

    //* returns true if entry text matches buffer
    bool matchColor(  const QString& buf ) const;

    //* returns true if any entry attachment file name matches buffer
    bool matchAttachment( const QString& buf ) const;

    //* set if entry is said visible by the find bar
    void setFindSelected( bool value )
    { findSelected_ = value; }

    //* set if entry is said vidible by the keyword list
    void setKeywordSelected( bool value )
    { keywordSelected_ = value; }

    //* returns true if entry is visible (i.e. selected by the find bar and keyword list)
    bool isSelected( void ) const
    { return findSelected_ && keywordSelected_; }

    //* returns true if entry is selected by the find bar
    bool isFindSelected( void ) const
    { return findSelected_; }

    //* returns true if entry is selected by the keyword list
    bool isKeywordSelected( void ) const
    { return keywordSelected_; }

    //* use to get last modified entry
    class LastModifiedFTor
    {

        public:

        //* returns true if first entry was modified after the second
        bool operator() ( const LogEntry* first, const LogEntry* second )
        { return second->modification() < first->modification(); }

    };

    //* use to get first created entry
    class FirstCreatedFTor
    {

        public:

        //* returns true if first entry was modified after the second
        bool operator() ( const LogEntry* first, const LogEntry* second )
        { return first->creation() < second->creation(); }

    };

    //* use to check if entries have same creation time
    class SameCreationFTor
    {
        public:

        //* constructor
        SameCreationFTor( const TimeStamp& stamp ):
            stamp_( stamp )
        {}

        //* predicate
        bool operator()( const LogEntry *entry ) const
        { return entry->creation() == stamp_; }

        private:

        //* predicted stamp
        TimeStamp stamp_;

    };

    //* use to check if entries have same creation and modification time
    class DuplicateFTor
    {
        public:

        //* constructor
        DuplicateFTor( LogEntry* entry ):
            entry_( entry )
        {}

        //* predicate
        bool operator()( const LogEntry *entry ) const
        { return entry->creation() == entry_->creation(); }

        private:

        //* predicte entry
        LogEntry *entry_;

    };

    /*!
    used to check if LogEntry keyword matches a given keyword.
    A match is found when the LogEntry keyword starts with the reference keyword
    It is used to check for keywords which have no associated entries
    */
    class MatchKeywordFTor
    {
        public:

        //* constructor
        MatchKeywordFTor( const Keyword& keyword ):
            keyword_( keyword )
        {}

        //* predicate
        bool operator() (const LogEntry* entry ) const
        { return entry->keyword().inherits( keyword_ );}

        private:

        //* comparison keyword
        Keyword keyword_;

    };


    private:

    //* LogEntry color
    void setColor( QString );

    //* initialize fields (default values)
    void _init( void );

    //* case sensitivity
    Qt::CaseSensitivity _caseSensitive( void ) const;

    //* log entry creation time
    TimeStamp creation_;

    //* log entry last modification time
    TimeStamp modification_;

    //* log entry title
    QString title_;

    //* log entry keywords
    Keyword keyword_;

    //* last user name who had access to the entry
    QString author_;

    //* LogEntry text
    QString text_;

    //* LogEntry color
    Base::Color color_;

    //* set to true if entry is said visible by the selection bar
    bool findSelected_;

    //* set to true if entry is said visible by the keyword selection
    bool keywordSelected_;

    //* list of text formats
    Format::TextFormatBlock::List formats_;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( LogEntry::Mask )

#endif
