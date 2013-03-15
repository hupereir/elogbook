#ifndef Logbook_h
#define Logbook_h

// $Id$

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
* ANY WARRANTY;  without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.   See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA   02111-1307 USA
*
*
*******************************************************************************/

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

/*!
\class Logbook
\brief log file parser based on xml
*/
class Logbook:public QObject, public Counter, public BASE::Key
{

    Q_OBJECT

    public:


    //! default string when no title given
    static const QString LOGBOOK_NO_TITLE;

    //! default string when no author given
    static const QString LOGBOOK_NO_AUTHOR;

    //! default string when no file given
    static const QString LOGBOOK_NO_FILE;

    //! default string when no directory given
    static const QString LOGBOOK_NO_DIRECTORY;

    //! max number of entries in logbook (make child logbook if larger)
    enum { MAX_ENTRIES = 50 };

    //! configuration mask
    enum Mask
    {
        LOGBOOK_TITLE = 1<<0,
        LOGBOOK_COMMENTS = 1<< 1,
        LOGBOOK_AUTHOR = 1<<2,
        LOGBOOK_FILE = 1<<3,
        LOGBOOK_DIRECTORY = 1<<4,
        LOGBOOK_CREATION = 1<<5,
        LOGBOOK_MODIFICATION = 1<<6,
        LOGBOOK_BACKUP = 1<<7,
        LOGBOOK_TABLE = 1<<8,
        LOGBOOK_CONTENT = 1<<9,
        LOGBOOK_HEADER =
            LOGBOOK_TITLE|LOGBOOK_AUTHOR|LOGBOOK_FILE|
            LOGBOOK_CREATION|LOGBOOK_MODIFICATION|LOGBOOK_BACKUP|
            LOGBOOK_DIRECTORY|LOGBOOK_COMMENTS,
        LOGBOOK_ALL = LOGBOOK_HEADER|LOGBOOK_TABLE|LOGBOOK_CONTENT
    };

    //! constructor from file
    Logbook( const File& file = File("") );

    //! destructor
    virtual ~Logbook( void );

    //! read from file
    /*!
    reads all xml based objects in the input file and chlids,
    if any [recursive]
    */
    bool read( void );

    //! writes all xml based objects in given|input file, if any [recursive]
    bool write( File file = File("") );

    //! synchronize logbook with remote
    /*!
    returns a map of duplicated entries.
    The first entry is local and can be safely deleted
    The second entry is the remote replacement
    */
    QHash<LogEntry*,LogEntry*> synchronize( const Logbook& logbook );

    //! retrieve Xml parsing errors [recursive]
    XmlError::List xmlErrors( void ) const;

    //! shortcut for logbook children list
    typedef QList< Logbook* > List;

    //!@name recursive accessors
    //@{

    //! retrieves list of all child logbook [recursive]
    List children( void ) const;

    //! retrieves first not full child.
    Logbook* latestChild( void );

    //! retrieve all associated entries [recursive]
    BASE::KeySet<LogEntry> entries( void ) const;

    //! retrieve all associated attachments [recursive]
    BASE::KeySet<Attachment> attachments( void ) const;

    //! truncate recent entries list
    void truncateRecentEntriesList( int );

    //! returns true if logbook is empty (no recursive entries found)
    bool empty( void ) const
    { return entries().empty(); }

    //@}

    //! remove empty children logbooks from list [recursive]
    void removeEmptyChildren( void );

    //!@name recent entries
    //]{

    //! recent entries
    QList<LogEntry*> recentEntries( void ) const;

    //! recent entries
    void addRecentEntry( const LogEntry* );

    //@}

    //!@name attributes
    //@{

    //! creation TimeStamp
    TimeStamp creation( void ) const
    { return creation_; }

    //! creation TimeStamp
    void setCreation( const TimeStamp& stamp )
    { creation_ = stamp; }

    //! modification TimeStamp
    TimeStamp modification( void ) const
    { return modification_; }

    //! modification TimeStamp
    void setModification( const TimeStamp& stamp );

    //! backup TimeStamp
    TimeStamp backup( void ) const
    { return backup_; }

    //! backup TimeStamp
    void setBackup( const TimeStamp& stamp )
    { backup_ = stamp; }

    //! saved TimeStamp
    TimeStamp saved( void ) const
    { return saved_; }

    //! saved TimeStamp
    void setSaved( const TimeStamp& stamp )
    { saved_ = stamp; }

    //! logbook filename
    const File& file( void ) const
    { return file_; }

    //! logbook filename
    void setFile( const File& );

    //! parent logbook filename
    const File& parentFile( void ) const
    { return parentFile_; }

    //! parent logbook filename
    void setParentFile( const QString& file )
    { parentFile_ = file; }

    //! logbook title
    QString title( void ) const
    { return title_; }

    //! logbook title. Returns true if changed.
    bool setTitle( const QString& title )
    {
        if( title_ == title ) return false;
        title_ = title;
        return true;
    }

    //! logbook last author
    QString author( void ) const
    { return author_; }

    //! logbook author. Returns true if changed.
    bool setAuthor( const QString& author )
    {
        if( author_ == author ) return false;
        author_ = author;
        return true;
    }

    //! retrieves attachment comments
    const QString& comments( void ) const
    { return comments_; }

    //! appends string to attachment comments. Returns true if changed.
    bool setComments( const QString& comments )
    {
        if( comments == comments_ ) return false;
        comments_ = comments ;
        return true;
    }

    //! logbook directory
    const File& directory( void ) const
    { return directory_; }

    //! checks if logbook directory is set, exists and is a directory
    bool checkDirectory( void ) const
    { return File( directory_ ).isDirectory(); }

    //! logbook directory. Returns true if changed.
    bool setDirectory( const File& directory )
    {
        if( directory_ == directory ) return false;
        directory_ = directory;
        return true;
    }

    /*! \brief
    number of entries in logbook as read from xml
    it is not supposed to be synchronized with current list of entries
    */
    const int& xmlEntries( void ) const
    { return xmlEntries_; }

    /*! \brief
    number of entries in logbook as read from xml
    it is not supposed to be synchronized with current list of entries
    Returns true if changed.
    */
    bool setXmlEntries( const int& value )
    {
        if( xmlEntries_ == value ) return false;
        xmlEntries_ = value ;
        return true;
    }

    /*! \brief
    number of children in logbook as read from xml
    it is not supposed to be synchronized with current list of children
    */
    const int& xmlChildren( void ) const
    { return xmlChildren_; }

    /*! \brief
    number of children in logbook as read from xml
    it is not supposed to be synchronized with current list of children.
    Returns true if changed.
    */
    bool setXmlChildren( const int& value )
    {
        if( xmlChildren_ == value ) return false;
        xmlChildren_ = value;
        return true;
    }

    //@}

    //! true if last backup is too old
    bool needsBackup( void ) const;

    //! generate tagged backup filename
    QString backupFilename( void ) const;

    //! sets logbook modified value
    void setModified( const bool& value );

    //! sets logbook and children modified value [recursive]
    void setModifiedRecursive( bool value );

    //! tells if logbook or children has been modified since last call [recursive]
    bool modified( void ) const;

    //!@name sort
    //@{

    //! sort method enumeration
    enum SortMethod
    {
        //! sort LogEntry objects according to creation time
        SORT_CREATION,

        //! sort LogEntry objects according to last modification time
        SORT_MODIFICATION,

        //! sort LogEntry objects according to title
        SORT_TITLE,

        //! sort LogEntry objects according to keyword
        SORT_KEYWORD,

        //! sort LogEntry objects according to author
        SORT_AUTHOR,

        //! colors
        SORT_COLOR

    };

    //! Sort entries depending on the sort method
    class EntryLessFTor
    {

        public:

        //! constructor
        EntryLessFTor( const Logbook::SortMethod& sort_method, const int& order = 0 ):
            sortMethod_( sort_method ),
            order_(order)
        {}

        //! sort operator
        bool operator()( LogEntry* first, LogEntry* second ) const;

        private:

        //! sort method (defined a constructor)
        Logbook::SortMethod sortMethod_;

        //! order
        int order_;

    };

    /*!
    changes sort method associated to oldest parent
    returns true if changed
    */
    bool setSortMethod( const SortMethod& sort_method );

    //! retrieves current sort method associated to oldest parent
    SortMethod sortMethod( void )
    { return sortMethod_; }

    //! sort order
    bool setSortOrder( const int& order );

    //! sort order
    const int& sortOrder( void ) const
    { return sortOrder_; }

    //@}

    //! used to retrieve logbook associated to given file
    class SameFileFTor
    {

        public:

        //! constructor
        SameFileFTor( const QString& file ):
            file_( file )
        {}

        //! predicate
        bool operator() (const Logbook* logbook) const
        { return logbook && logbook->file() == file_; }

        private:

        //! predicted file
        QString file_;

    };

    //! store backup information
    class Backup: public Counter
    {

        public:

        //! constructor
        Backup( const File& file = File(), const TimeStamp& creation = TimeStamp::now() ):
            Counter( "Logbook::Backup" ),
            file_( file ),
            creation_( creation ),
            valid_( true )
        {}

        //! constructor from Dom
        Backup( const QDomElement& );

        //! equal to operator
        bool operator == (const Backup& other ) const
        { return creation() == other.creation() && file() == other.file(); }

        //! less than operator
        bool operator < (const Backup& other ) const
        {
            if( creation() != other.creation() ) return creation() < other.creation();
            else return file() < other.file();
        }

        //!@name accessors
        //@{

        //! get dom
        QDomElement domElement( QDomDocument& ) const;

        //! creation
        const TimeStamp& creation( void ) const
        { return creation_; }

        //! file
        const File& file( void ) const
        { return file_; }

        //! validity
        bool isValid( void ) const
        { return valid_; }

        //@}

        //!@name modifiers

        //! time
        void setCreation( const TimeStamp& creation )
        { creation_ = creation; }

        //! file
        void setFile( const File& file )
        { file_ = file; }

        //! check validity
        void checkValidity( void )
        { valid_ = file_.exists(); }

        //@}

        //! list
        class List: public QList<Backup>
        {
            public:

            //! constructor
            List( void )
            {}

            //! constructor
            List( const QList<Backup>& other ):
                QList<Backup>( other )
                {}

            //! validity
            void checkValidity( void )
            {
                for( iterator iter = begin(); iter != end(); ++iter )
                { iter->checkValidity(); }
            }

        };

        //! test validity
        class InvalidFTor
        {

            public:

            //! predicate
            bool operator() ( Logbook::Backup backup ) const
            { return !backup.isValid(); }

        };

        private:

        //! filename
        File file_;

        //! timestamp
        TimeStamp creation_;

        //! validity
        bool valid_;

    };

    //! add backup
    void addBackup( const File& );

    //! set backup
    void setBackupFiles( const Backup::List& );

    //! backup files
    const Backup::List& backupFiles( void ) const
    { return backupFiles_; }

    signals:

    //! message emission for logbook status during reading/writting
    void messageAvailable( const QString& message );

    //! emit maximum progress
    /*! argument is the maximum number of entries to read */
    void maximumProgressAvailable( int );

    //! emit progress when reading, saving
    /*! argument is the number of entries read since last signal */
    void progressAvailable( int );

    protected:

    //! read recent entries
    void _readRecentEntries( const QDomElement& );

    //! recent entries dom element
    QDomElement _recentEntriesElement( QDomDocument& ) const;

    //! generate tagged backup filename
    File _childFilename( const File& file, const int& child_number ) const;

    private:

    //! list of pointers to logbook children
    List children_;

    //! true if at least one logbook entry have been modified/added/deleted until last save
    bool modified_;

    //! file from which the log book entries are read
    File file_;

    //! file of parent logbook, if any
    File parentFile_;

    //! directory where the attached files are read/saved
    File directory_;

    //! title of the log book
    QString title_;

    //! last user name who had access to the logbook
    QString author_;

    //! comments
    QString comments_;

    //! logbook creation time
    TimeStamp creation_;

    //! logbook last modification time
    TimeStamp modification_;

    //! logbook last backup time
    TimeStamp backup_;

    //! logbook last save time
    TimeStamp saved_;

    //! method used for LogEntry sort
    SortMethod sortMethod_;

    //! backup list
    Backup::List backupFiles_;

    //! list of recent entries
    /*! creation time stamp of recent entries are stored */
    typedef QList<TimeStamp> TimeStampList;

    //! list of recent entries
    /*! creation time stamp of recent entries are stored */
    TimeStampList recentEntries_;

    //! sort order
    int sortOrder_;

    //! number of entries in logbook as read from xml
    /*!  \brief
    number of entries in logbook as read from xml
    it is not supposed to be synchronized with current list of entries
    */
    int xmlEntries_;

    //! number of children in logbook as read from xml
    /*!
    number of children in logbook as read from xml
    it is not supposed to be synchronized with current list of children
    */
    int xmlChildren_;

    //! error when parsing xml file
    XmlError error_;

};

#endif
