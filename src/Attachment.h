#ifndef Attachment_h
#define Attachment_h

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

#include "Counter.h"
#include "File.h"
#include "Functors.h"
#include "Key.h"
#include "TimeStamp.h"

#include <QDomDocument>
#include <QDomElement>

class LogEntry;

/**
\class Attachment
\brief Attached file object
*/

class Attachment: private Base::Counter<Attachment>, public Base::Key
{
    public:

    //* default string when no file given
    static const QString NoFile;

    //* default string when no comments given
    static const QString NoComments;

    //* no size
    static const QString NoSize;

    //* contructor
    explicit Attachment( const QString = QString() );

    //* creator from DomElement
    explicit Attachment( const QDomElement& element );

    //* domElement
    QDomElement domElement( QDomDocument& parent ) const;

    //*@name accessors
    //@{

    //* validity
    bool isValid() const
    { return isValid_; }

    //* url
    bool isUrl() const
    { return isUrl_; }

    //* link state
    enum class LinkState
    {
        Unknown,
        Yes,
        No
    };

    //* link
    LinkState isLink() const
    { return isLink_; }

    //* retrieves local file size
    double size() const
    { return size_; }

    //* retrieves local file size
    const QString& sizeString() const
    { return sizeString_; }

    //* retrieves associated entry
    LogEntry* entry() const;

    //* file creation
    const TimeStamp& creation() const
    { return creation_; }

    //* retrieves file last modification
    const TimeStamp& modification() const
    { return modification_; }

    //* retrieves original file name
    const File& sourceFile() const
    { return sourceFile_; }

    //* retrieves attachment file name
    const File& file() const
    { return file_; }

    //* retrieves attachment short file name
    File shortFile() const;

    //* retrieves attachment comments
    const QString& comments() const
    { return comments_; }

    //@}

    //*@name modifiers
    //@{

    //* validity
    bool setIsValid( bool value )
    {
        if( isValid_ == value ) return false;
        isValid_ = value;
        return true;
    }

    //* url
    bool setIsUrl( bool value )
    {
        if( isUrl_ == value ) return false;
        isUrl_ = value;
        return true;
    }

    //* link
    bool setIsLink( const LinkState& value )
    {
        if( isLink_ == value ) return false;
        isLink_ = value;
        return true;
    }

    //* read file size
    void updateSize();

    //* update time stamps
    bool updateTimeStamps();

    //* appends string to attachment comments
    bool setComments( const QString& buf )
    {
        if( comments_ == buf ) return false;
        comments_ = buf;
        return true;
    }

    //* command enum to tell who original file should be transformed into attached file
    enum class Command
    {
        Copy,
        Link,
        ForceCopy,
        ForceLink,
        CopyVersion,
        LinkVersion,
        Nothing
    };

    //* error codes output enum for ProcessCopy
    enum class ErrorCode
    {
        Success,
        SourceNotFound,
        DestNotFound,
        SourceIsDir,
        DestExist
    };

    /** \fn ErrorCode copy( const Attachment::Command& command, const QString& destdir )
    \brief ErrorCode convert original file into attached file. Returns true in case of success
    \param command tells how the original file is to be converted into attached file. Is one of the following:
    Attachment::COPY use command cp, if the attached file is not present
    Attachment::LINK use ln -s, if the attached file is not present
    Attachment::OVERWRITE use command cp, overwrite attached file if present
    Attachment::COPY_VERSION, modify filename to resolve copy, then use cp
    Attachment::LINK_VERSION, modify filename to resolve copy, then use ln -s
    Attachment::DO_NOTHING, just stores the attached file name, but do nothing
    \param destdir destination directory
    */
    ErrorCode copy( const Attachment::Command& command, const QString& destdir );

    //@}

    //* used to check attachment filenames
    using SameFileFTor = Base::Functor::Unary<Attachment, const File&, &Attachment::file>;

    private:

    //* set original attachment file name
    void _setSourceFile( const File& file )
    { sourceFile_ = file; }

    //* set attachment file name
    void _setFile( const File& file );

    //* file creation
    bool _setCreation( TimeStamp stamp )
    {
        if( creation_ == stamp ) return false;
        creation_ = stamp;
        return true;
    }

    //* modification
    bool _setModification( TimeStamp stamp )
    {
        if( modification_ == stamp ) return false;
        modification_ = stamp;
        return true;
    }

    //* attached file name
    File sourceFile_ = File( NoFile );

    //* attached file name
    File file_ = File( NoFile );

    //* comments
    QString comments_ = NoComments;

    //* file size (0 if not valid | URL )
    double size_ = 0;

    //* corresponding size_string
    QString sizeString_ = NoSize;

    //* creation
    TimeStamp creation_;

    //* file last modification timestamp
    TimeStamp modification_;

    //* is url
    bool isUrl_ = false;

    //* is link
    LinkState isLink_ = LinkState::Unknown;

    //* true for URL or if file exist
    bool isValid_ = false;

    /**\fn bool operator < (const Attachment& attachment ) const
    \brief less than operator, based on Attachment Short name lexicographic order
    \param attachment the attachment to which this is to be compared
    */
    friend bool operator < (const Attachment&, const Attachment&);

    /**\fn bool operator == (const Attachment& attachment ) const
    \brief equal to operator, based on Attachment Full name
    \param attachment the attachment to which this is to be compared
    */
    friend bool operator == (const Attachment& first, const Attachment& second)
    { return first.file_ == second.file_; }

};

#endif
