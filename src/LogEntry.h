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
#include "Functors.h"
#include "IntegralType.h"
#include "Key.h"
#include "Keyword.h"
#include "TextFormatBlock.h"
#include "TimeStamp.h"
#include "XmlOptions.h"

#include <QDomElement>
#include <QDomDocument>

//* log file entry manipulation object
class LogEntry:private Base::Counter<LogEntry>, public Base::Key
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

    using Mask = Base::underlying_type_t<MaskFlag>;

    //* empty creator
    explicit LogEntry();

    //* constructor from DOM
    explicit LogEntry( const QDomElement& );

    //* destructor
    ~LogEntry() override;

    //* delete assignment operator
    /** copy constructor is private, because it is used for deep copy */
    LogEntry& operator = ( const LogEntry& ) = delete;

    //* mime type (for drag and drop)
    static const QString MimeType;

    //*@name accessors
    //@{

    //* DomElement
    QDomElement domElement( QDomDocument& ) const;

    //* return a new entry copy from this
    /* deep copy of the associated attachments is performed */
    LogEntry* copy() const;

    //* creation TimeStamp
    const TimeStamp& creation() const
    { return creation_; }

    //* modification TimeStamp
    const TimeStamp& modification() const
    { return modification_; }

    //* LogEntry title
    QString title() const
    { return title_; }

    //* true if has keywords
    bool hasKeywords() const
    { return !keywords_.empty(); }

    //* Log entry keyword
    const Keyword::Set& keywords() const
    { return keywords_; }

    //* Log entry last author
    QString author() const
    { return author_; }

    //* LogEntry color
    Base::Color color() const
    { return color_; }

    //* entry text format
    TextFormat::Block::List formats() const
    { return formats_; }

    //* LogEntry text
    QString text() const
    { return text_; }


    //* returns true if entry title matches buffer
    bool matchTitle( const QString &) const;

    //* returns true if entry keyword matches buffer
    bool matchKeyword( const QString &) const;

    //* returns true if entry text matches buffer
    bool matchText( const QString &) const;

    //* returns true if entry text matches buffer
    bool matchColor( const QString &) const;

    //* returns true if any entry attachment file name matches buffer
    bool matchAttachment( QString ) const;

    //* returns true if entry is visible (i.e. selected by the find bar and keyword list)
    bool isSelected() const
    { return findSelected_ && keywordSelected_; }

    //* returns true if entry is selected by the find bar
    bool isFindSelected() const
    { return findSelected_; }

    //* returns true if entry is selected by the keyword list
    bool isKeywordSelected() const
    { return keywordSelected_; }

    //@}

    //*@name modifiers
    //@{

    //* creation TimeStamp
    void setCreation( const TimeStamp &stamp )
    { creation_ = stamp; }

    //* set modification_ to _now_
    void setModified();

    //* modification TimeStamp
    void setModification( const TimeStamp &stamp )
    { modification_ = stamp; }

    //* Log entry title
    void setTitle( const QString &title )
    { title_ = title; }

    //* clear keywords
    void clearKeywords();

    //* add a keyword to the list
    void addKeyword( const Keyword &);

    //* replace a keyword in the list
    void replaceKeyword( const Keyword&, const Keyword &);

    //* remove keyword
    void removeKeyword( const Keyword &);

    //* Log entry author
    void setAuthor( const QString &author )
    { author_ = author; }

    //* LogEntry color
    void setColor(  const QColor& color )
    { color_ = Base::Color(color); }

    //* add TextFormatBlock
    void addFormat( TextFormat::Block );

    //* entry text format
    void setFormats( const TextFormat::Block::List& formats )
    { formats_ = formats; }

    //* LogEntry text
    void setText( const QString &text )
    { text_ = text; }

    //* set if entry is said visible by the find bar
    void setFindSelected( bool value )
    { findSelected_ = value; }

    //* set if entry is said vidible by the keyword list
    void setKeywordSelected( bool value )
    { keywordSelected_ = value; }

    //@}

    //* use to get last modified entry
    using LastModifiedFTor = Base::Functor::BinaryMore<LogEntry, const TimeStamp&, &LogEntry::modification>;

    //* use to get first created entry
    using FirstCreatedFTor = Base::Functor::BinaryLess<LogEntry, const TimeStamp&, &LogEntry::creation>;

    //* use to check if entries have same creation time
    using SameCreationFTor = Base::Functor::Unary<LogEntry, const TimeStamp&, &LogEntry::creation>;

    //* use to check if entries have same creation and modification time
    using DuplicateFTor = Base::Functor::Unary<LogEntry, const TimeStamp&, &LogEntry::creation>;

    /**
    used to check if LogEntry keyword matches a given keyword.
    A match is found when the LogEntry keyword starts with the reference keyword
    It is used to check for keywords which have no associated entries
    */
    class MatchKeywordFTor
    {
        public:

        //* constructor
        explicit MatchKeywordFTor( const Keyword &keyword ):
            keyword_( keyword )
        {}

        //* predicate
        bool operator() (const LogEntry* entry ) const
        {
            for( const auto& keyword:entry->keywords_ )
            { if( keyword.inherits( keyword_ ) ) return true; }

            return false;
        }

        private:

        //* comparison keyword
        Keyword keyword_;

    };


    private:

    //* copy constructor
    LogEntry( const LogEntry& ) = default;

    //* LogEntry color
    void setColor( QString );

    //* case sensitivity
    Qt::CaseSensitivity _caseSensitive() const;

    //* log entry creation time
    TimeStamp creation_;

    //* log entry last modification time
    TimeStamp modification_;

    //* log entry title
    QString title_;

    //* log entry keywords list
    Keyword::Set keywords_;

    //* last user name who had access to the entry
    QString author_;

    //* LogEntry text
    QString text_;

    //* LogEntry color
    Base::Color color_;

    //* set to true if entry is said visible by the selection bar
    bool findSelected_ = true;

    //* set to true if entry is said visible by the keyword selection
    bool keywordSelected_ = false;

    //* list of text formats
    TextFormat::Block::List formats_;

};

#endif
