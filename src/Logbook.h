#ifndef Logbook_h
#define Logbook_h

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

#include "Backup.h"
#include "Counter.h"
#include "Debug.h"
#include "File.h"
#include "Key.h"
#include "TimeStamp.h"
#include "XmlError.h"

#include <QDomElement>
#include <QDomDocument>

#include <QHash>
#include <QList>
#include <QObject>

class LogEntry;
class Attachment;
class XmlTextFormatInfo;

/**
\class Logbook
\brief log file parser based on xml
*/
class Logbook:public QObject, private Base::Counter<Logbook>, public Base::Key
{

    Q_OBJECT

    public:


    //* default string when no title given
    static const QString NoTitle;

    //* default string when no author given
    static const QString NoAuthor;

    //* default string when no file given
    static const QString NoFile;

    //* default string when no directory given
    static const QString NoDirectory;

    //* max number of entries in logbook (make child logbook if larger)
    enum { MaxEntries = 50 };

    //* configuration mask
    enum MaskFlag
    {
        TitleMask = 1<<0,
        CommentsMask = 1<< 1,
        AuthorMasks = 1<<2,
        FileMask = 1<<3,
        DirectoryMask = 1<<4,
        CreationMask = 1<<5,
        ModificationMask = 1<<6,
        BackupMask = 1<<7,
        TableOfContentMask = 1<<8,
        ContentMask = 1<<9,
        HeaderMask =
            TitleMask|AuthorMasks|FileMask|
            CreationMask|ModificationMask|BackupMask|
            DirectoryMask|CommentsMask,
        All = HeaderMask|TableOfContentMask|ContentMask
    };

    Q_DECLARE_FLAGS( Mask, MaskFlag )

    //* constructor from file
    explicit Logbook( File = File() );

    //* destructor
    ~Logbook() override;

    //*@name accessors
    //@{

    //* retrieve Xml parsing errors [recursive]
    XmlError::List xmlErrors() const;

    //* shortcut for logbook children list
    using List = QList< Logbook* >;

    //* retrieves list of all child logbook [recursive]
    List children() const;

    //* retrieves first not full child.
    Logbook* latestChild();

    //* retrieve all associated entries [recursive]
    Base::KeySet<LogEntry> entries() const;

    //* recent entries
    QList<LogEntry*> recentEntries() const;

    //* retrieve all associated attachments [recursive]
    Base::KeySet<Attachment> attachments() const;

    //* returns true if logbook is empty (no recursive entries found)
    bool empty() const
    { return entries().empty(); }

    //* logbook filename
    File file() const
    { return file_; }

    //* parent logbook filename
    File parentFile() const
    { return parentFile_; }

    //* logbook title
    QString title() const
    { return title_; }

    //* logbook last author
    QString author() const
    { return author_; }

    //* retrieves attachment comments
    QString comments() const
    { return comments_; }

    //* logbook directory
    File directory() const
    { return directory_; }

    //* checks if logbook directory is set, exists and is a directory
    bool checkDirectory() const
    { return File( directory_ ).isDirectory(); }

    //* tells if logbook or children has been modified since last call [recursive]
    bool modified() const;

    //* read only
    bool isReadOnly() const
    { return readOnly_; }

    //* backup
    bool isBackup() const
    { return isBackup_; }

    //* creation TimeStamp
    TimeStamp creation() const
    { return creation_; }

    //* modification TimeStamp
    TimeStamp modification() const
    { return modification_; }

    //* backup TimeStamp
    TimeStamp backup() const
    { return backup_; }

    //* saved TimeStamp
    TimeStamp saved() const
    { return saved_; }

    /** \brief
    number of entries in logbook as read from xml
    it is not supposed to be synchronized with current list of entries
    */
    int xmlEntries() const
    { return xmlEntries_; }

    /** \brief
    number of children in logbook as read from xml
    it is not supposed to be synchronized with current list of children
    */
    int xmlChildren() const
    { return xmlChildren_; }

    //* true if last backup is too old
    bool needsBackup() const;

    //* generate tagged backup filename
    File backupFilename() const;

    //* sort method enumeration
    enum SortMethod
    {
        SortCreation,
        SortModification,
        SortTitle,
        SortKeyword,
        SortAuthor,
        SortColor

    };

    //* retrieves current sort method associated to oldest parent
    SortMethod sortMethod()
    { return sortMethod_; }

    //* sort order
    int sortOrder() const
    { return sortOrder_; }

    //* backup files
    const Backup::List& backupFiles() const
    { return backupFiles_; }

    //@}

    //*@name modifiers
    //@{

    //* compression [recursive]
    void setUseCompression( bool value );

    //* read from file
    /**
    reads all xml based objects in the input file and chlids,
    if any [recursive]
    */
    bool read();

    //* writes all xml based objects in given|input file, if any [recursive]
    bool write( File = File() );

    //* synchronize logbook with remote
    /**
    returns a map of duplicated entries.
    The first entry is local and can be safely deleted
    The second entry is the remote replacement
    */
    QHash<LogEntry*,LogEntry*> synchronize( const Logbook& logbook );

    //* truncate recent entries list
    void truncateRecentEntriesList( int );

    //* remove empty children logbooks from list [recursive]
    void removeEmptyChildren();

    //* recent entries
    void addRecentEntry( const LogEntry* );

    //* creation TimeStamp
    void setCreation( const TimeStamp& stamp )
    { creation_ = stamp; }

    //* modification TimeStamp
    void setModification( const TimeStamp& stamp );

    //* backup TimeStamp
    void setBackup( const TimeStamp& stamp )
    { backup_ = stamp; }

    //* saved TimeStamp
    void setSaved( const TimeStamp& stamp )
    { saved_ = stamp; }

    //* logbook filename
    void setFile( File, bool recursive = false );

    //* parent logbook filename
    void setParentFile( const File& file )
    { parentFile_ = file; }

    //* logbook title. Returns true if changed.
    bool setTitle( QString title )
    {
        if( title_ == title ) return false;
        title_ = title;
        return true;
    }

    //* logbook author. Returns true if changed.
    bool setAuthor( QString author )
    {
        if( author_ == author ) return false;
        author_ = author;
        return true;
    }

    //* appends string to attachment comments. Returns true if changed.
    bool setComments( QString comments )
    {
        if( comments == comments_ ) return false;
        comments_ = comments ;
        return true;
    }

    //* logbook directory. Returns true if changed.
    bool setDirectory( File directory )
    {
        if( directory_ == directory ) return false;
        directory_ = directory;
        return true;
    }

    /** \brief
    number of entries in logbook as read from xml
    it is not supposed to be synchronized with current list of entries
    Returns true if changed.
    */
    bool setXmlEntries( int value )
    {
        if( xmlEntries_ == value ) return false;
        xmlEntries_ = value ;
        return true;
    }

    /** \brief
    number of children in logbook as read from xml
    it is not supposed to be synchronized with current list of children.
    Returns true if changed.
    */
    bool setXmlChildren( int value )
    {
        if( xmlChildren_ == value ) return false;
        xmlChildren_ = value;
        return true;
    }

    //* set logbook as readonly [recursive]
    //* returns true if changed
    bool setReadOnly( bool );


    //* set logbook as backup [recursive]
    //* returns true if changed
    bool setIsBackup( bool );

    //* sets logbook modified value
    void setModified( bool );

    //* sets logbook and children modified value [recursive]
    void setModifiedRecursive( bool );

    /**
    changes sort method associated to oldest parent
    returns true if changed
    */
    bool setSortMethod( SortMethod );

    //* sort order
    bool setSortOrder( int order );

    //* add backup
    void addBackup( File );

    //* set backup
    void setBackupFiles( const Backup::List& );

    //@}

    //* Sort entries depending on the sort method
    class EntryLessFTor
    {

        public:

        //* constructor
        explicit EntryLessFTor( Logbook::SortMethod sortMethod, int order = 0 ):
            sortMethod_( sortMethod ),
            order_(order)
        {}

        //* sort operator
        bool operator()( LogEntry* first, LogEntry* second ) const;

        private:

        //* sort method (defined a constructor)
        Logbook::SortMethod sortMethod_;

        //* order
        int order_;

    };

    //* used to retrieve logbook associated to given file
    class SameFileFTor
    {

        public:

        //* constructor
        explicit SameFileFTor( File file ):
            file_( file )
        {}

        //* predicate
        bool operator() (const Logbook* logbook) const
        { return logbook && logbook->file() == file_; }

        private:

        //* predicted file
        File file_;

    };

    Q_SIGNALS:

    //* message emission for logbook status during reading/writting
    void messageAvailable( QString message );

    //* emit maximum progress
    /** argument is the maximum number of entries to read */
    void maximumProgressAvailable( int );

    //* emit progress when reading, saving
    /** argument is the number of entries read since last signal */
    void progressAvailable( int );

    //* read-only changed
    void readOnlyChanged( bool );

    protected:

    //* read recent entries
    void _readRecentEntries( const QDomElement& );

    //* recent entries dom element
    QDomElement _recentEntriesElement( QDomDocument& ) const;

    //* generate tagged backup filename
    File _childFilename( File, int ) const;

    private:

    //* list of pointers to logbook children
    List children_;

    //* file from which the logbook entries are read
    File file_;

    //* file of parent logbook, if any
    File parentFile_;

    //* directory where the attached files are read/saved
    File directory_;

    //* title of the log book
    QString title_;

    //* last user name who had access to the logbook
    QString author_;

    //* comments
    QString comments_;

    //* true if at least one logbook entry have been modified/added/deleted until last save
    bool modified_ = false;

    //* true if read only
    bool readOnly_ = false;

    //* true if this logbook is a backup
    bool isBackup_ = false;

    //* true if logbook uses compression
    bool useCompression_ = false;

    //* logbook creation time
    TimeStamp creation_;

    //* logbook last modification time
    TimeStamp modification_;

    //* logbook last backup time
    TimeStamp backup_;

    //* logbook last save time
    TimeStamp saved_;

    //* method used for LogEntry sort
    SortMethod sortMethod_ = Logbook::SortCreation;

    //* backup list
    Backup::List backupFiles_;

    //* list of recent entries
    /** creation time stamp of recent entries are stored */
    using TimeStampList = QList<TimeStamp>;

    //* list of recent entries
    /** creation time stamp of recent entries are stored */
    TimeStampList recentEntries_;

    //* sort order
    int sortOrder_ = 0;

    //* number of entries in logbook as read from xml
    /**  \brief
    number of entries in logbook as read from xml
    it is not supposed to be synchronized with current list of entries
    */
    int xmlEntries_ = 0;

    //* number of children in logbook as read from xml
    /**
    number of children in logbook as read from xml
    it is not supposed to be synchronized with current list of children
    */
    int xmlChildren_ = 0;

    //* error when parsing xml file
    XmlError error_;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( Logbook::Mask )

#endif
