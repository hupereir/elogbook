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

#include "Logbook.h"

#include "Attachment.h"
#include "Debug.h"
#include "FileCheck.h"
#include "LogEntry.h"
#include "XmlOptions.h"
#include "Util.h"
#include "XmlDocument.h"
#include "XmlDef.h"
#include "XmlString.h"
#include "XmlTimeStamp.h"

#include <QFile>
#include <QTextStream>

//________________________________
// public methods

const QString Logbook::NoTitle( QObject::tr( "My electronic logbook" ) );
const QString Logbook::NoAuthor( QObject::tr( "anonymous" ) );
const QString Logbook::NoFile;
const QString Logbook::NoDirectory;

//________________________________
Logbook::Logbook( const File& file ):
    Counter( "Logbook" ),
    directory_( NoDirectory ),
    title_( NoTitle ),
    author_( NoAuthor ),
    modified_( false ),
    readOnly_( false ),
    isBackup_( false ),
    creation_( TimeStamp::now() ),
    sortMethod_( Logbook::SortCreation ),
    sortOrder_( 0 ),
    xmlEntries_( 0 ),
    xmlChildren_( 0 )
{
    Debug::Throw( "Logbook::Logbook. (file)\n" );
    setFile( file );
    read();

}

//_________________________________
Logbook::~Logbook( void )
{
    Debug::Throw( "Logbook::~Logbook.\n" );

    // delete log children
    foreach( Logbook* logbook, children_ ) delete logbook;
    children_.clear();

    // delete associated entries
    Base::KeySet<LogEntry> entries( this );
    foreach( LogEntry* entry, entries ) delete entry;

}

//_________________________________
bool Logbook::read( void )
{

    Debug::Throw( "Logbook::read.\n" );

    if( file().isEmpty() )
    {
        Debug::Throw( "Logbook::read - file is empty.\n" );
        return false;
    }

    // update StateFrame
    emit messageAvailable( QString( tr( "Reading '%1'" ) ).arg( file().localName() ) );

    // check input file
    if( !file().exists() ) {
        Debug::Throw(0) << "Logbook::read - ERROR: cannot access file \"" << file() << "\".\n";
        return false;
    }

    // delete associated entries
    foreach( LogEntry* entry, Logbook::entries() )
    { delete entry; }

    // parse the file
    QFile file( Logbook::file() );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        Debug::Throw(0, "Logbook::read - cannot open file.\n" );
        return false;
    }

    // create document
    XmlDocument document;
    if( !document.setContent( &file, error_ ) ) return false;

    // read first child
    QDomElement docElement = document.documentElement();
    QString tagName( docElement.tagName() );
    if( tagName != Xml::Logbook )
    {
        Debug::Throw(0) << "Logbook::read - invalid tag name: " << tagName << endl;
        return false;
    }

    // read attributes
    QDomNamedNodeMap attributes( docElement.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {

        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        QString name( attribute.name() );
        QString value( attribute.value() );

        if( name == Xml::Title ) setTitle( XmlString( value ).toText() );
        else if( name == Xml::File ) setFile( File( XmlString( value ).toText() ) );
        else if( name == Xml::ParentFile ) setParentFile( XmlString( value ).toText() );
        else if( name == Xml::Directory ) setDirectory( File( XmlString( value ).toText() ) );
        else if( name == Xml::Author ) setAuthor( XmlString( value ).toText() );
        else if( name == Xml::SortMethod ) setSortMethod( (SortMethod) value.toInt() );
        else if( name == Xml::SortOrder ) setSortOrder( value.toInt() );
        else if( name == Xml::ReadOnly ) setReadOnly( value.toInt() );
        else if( name == Xml::BackupMask ) setIsBackup( value.toInt() );
        else if( name == Xml::Entries ) {

            setXmlEntries( value.toInt() );
            emit maximumProgressAvailable( value.toInt() );

        } else if( name == Xml::Children ) setXmlChildren( value.toInt() );
        else Debug::Throw(0) << "Logbook::read - unrecognized logbook attribute: \"" << name << "\"\n";

    }

    // parse children
    static unsigned int progress( 10 );
    unsigned int entryCount( 0 );
    for(QDomNode node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        QDomElement element = node.toElement();
        if( element.isNull() ) continue;

        QString tagName( element.tagName() );

        // children
        if( tagName == Xml::Comments ) setComments( XmlString( element.text() ).toText() );
        else if( tagName == Xml::Creation ) setCreation( XmlTimeStamp( element ) );
        else if( tagName == Xml::Modification ) setModification( XmlTimeStamp( element ) );
        else if( tagName == Xml::Backup ) setBackup( XmlTimeStamp( element ) );
        else if( tagName == Xml::RecentEntries ) _readRecentEntries( element );
        else if( tagName == Xml::BackupMask ) backupFiles_ << Backup( element );
        else if( tagName == Xml::Entry ) {

            LogEntry* entry = new LogEntry( element );
            Key::associate( this, entry );
            entryCount++;
            if( !(entryCount%progress) ) emit progressAvailable( progress );

        } else if( tagName == Xml::Child ) {

            // try retrieve file from attributes
            QString file_attribute( element.attribute( Xml::File ) );
            if( file_attribute.isNull() )
            {
                Debug::Throw(0) << "Logbook::read - no file given for child" << endl;
                continue;
            }

            File file( file_attribute );
            if( !file.isAbsolute() ) file = file.addPath( Logbook::file().path() );
            Logbook* child = new Logbook();
            child->setFile( file );

            // propagate progressAvailable signal.
            connect( child, SIGNAL(progressAvailable(int)), SIGNAL(progressAvailable(int)) );

            QString buffer;
            QTextStream( &buffer ) << "Reading " << child->file().localName();
            emit messageAvailable( QString( tr( "Reading '%1'" ) ).arg( child->file().localName() ) );

            child->read();
            children_ << child;

        } else Debug::Throw(0) << "Logbook::read - unrecognized tagName: " << tagName << endl;

    }

    emit progressAvailable( entryCount%progress );

    // discard modifications
    setModified( false );
    saved_ = Logbook::file().lastModified();
    return true;

}

//_________________________________
bool Logbook::write( File file )
{

    Debug::Throw( "Logbook::write.\n" );

    // check filename
    if( file.isEmpty() ) file = Logbook::file();
    if( file.isEmpty() ) return false;

    bool completed = true;

    // update stateFrame
    emit messageAvailable( QString( tr( "Writing '%1'" ) ).arg( file.localName() ) );

    // check number of entries and children to save in header
    if( setXmlEntries( entries().size() ) || setXmlChildren( children().size() ) )
    { setModified( true ); }

    emit maximumProgressAvailable( xmlEntries() );

    // write logbook if filename differs from origin or logbook is modified
    if( file != Logbook::file() || modified_ )
    {

        // gets last saved timestamp
        TimeStamp lastSaved( file.lastModified() );

        // make a backup of the file, if necessary
        if( XmlOptions::get().get<bool>( "FILE_BACKUP" ) )
        { file.backup(); }

        // update stateFrame
        QString buffer;
        QTextStream( &buffer ) << "Writing " << file.localName();
        emit messageAvailable( buffer );

        QFile out( file );
        if( !out.open( QIODevice::WriteOnly ) )
        {
            Debug::Throw(0) << "Logbook::write - unable to write to file " << file << endl;
            return false;
        }

        // create document
        XmlDocument document;

        // create main element
        QDomElement top = document.createElement( Xml::Logbook );
        if( !title_.isEmpty() ) top.setAttribute( Xml::Title, title_ );
        if( !directory_.isEmpty() ) top.setAttribute( Xml::Directory, directory_ );
        if( !author_.isEmpty() ) top.setAttribute( Xml::Author, author_ ) ;
        if( !parentFile_.isEmpty() ) top.setAttribute( Xml::ParentFile, parentFile_ );

        top.setAttribute( Xml::SortMethod, QString::number( sortMethod_ ) );
        top.setAttribute( Xml::SortOrder, QString::number( sortOrder_ ) );
        top.setAttribute( Xml::ReadOnly, QString::number( readOnly_ ) );
        top.setAttribute( Xml::BackupMask, QString::number( isBackup_ ) );

        // update number of entries and children
        top.setAttribute( Xml::Entries, QString::number(xmlEntries()) );
        top.setAttribute( Xml::Children, QString::number(xmlChildren()) );

        // append node
        document.appendChild( top );

        // comments
        if( comments().size() )
        {
            QDomElement commentsElement = document.createElement( Xml::Comments );
            QDomText comments_text = document.createTextNode( comments() );
            commentsElement.appendChild( comments_text );
            top.appendChild( commentsElement );
        }

        // write time stamps
        if( creation().isValid() ) top.appendChild( XmlTimeStamp( creation() ).domElement( Xml::Creation, document ) );
        if( modification().isValid() ) top.appendChild( XmlTimeStamp( modification() ).domElement( Xml::Modification, document ) );
        if( backup().isValid() ) top.appendChild( XmlTimeStamp( backup() ).domElement( Xml::Backup, document ) );

        // write recent entries
        if( !recentEntries_.empty() ) top.appendChild( _recentEntriesElement( document ) );

        // write backup files
        foreach( const Backup& backup, backupFiles_ )
        { top.appendChild( backup.domElement( document ) ); }

        // write all entries
        static unsigned int progress( 10 );
        unsigned int entryCount( 0 );
        Base::KeySet<LogEntry> entries( this );
        foreach( LogEntry* entry, entries )
        {

            top.appendChild( entry->domElement( document ) );
            entryCount++;
            if( !(entryCount%progress) ) emit progressAvailable( progress );

        }

        emit progressAvailable( entryCount%progress );

        // dump all logbook childrens
        for( int childCount = 0; childCount < children_.size(); ++childCount )
        {
            File childFilename = _childFilename( file, childCount );
            QDomElement childElement = document.createElement( Xml::Child );
            childElement.setAttribute( Xml::File, childFilename );
            top.appendChild( childElement );
        }

        // finish
        out.write( document.toByteArray() );
        out.close();

        // gets/check new saved timestamp
        TimeStamp savedNew( file.lastModified() );
        if( !( lastSaved < savedNew ) ) completed = false;
        else if( file == Logbook::file() )  modified_ = false;

        // assign new filename
        if( file != Logbook::file() ) setFile( file );

    } else { emit progressAvailable( Base::KeySet<LogEntry>( this ).size() ); }


    // update saved timeStamp
    saved_ = Logbook::file().lastModified();

    // write children
    unsigned int childCount=0;
    foreach( Logbook* logbook, children_ )
    {

        File childFilename( _childFilename( file, childCount ).addPath( file.path() ) );

        // update stateFrame
        emit messageAvailable( QString( tr( "Writing '%1'" ) ).arg( childFilename.localName() ) );

        logbook->setParentFile( file );
        completed &= logbook->write( childFilename );

        ++childCount;

    }

    return completed;

}

//_________________________________
QHash<LogEntry*,LogEntry*> Logbook::synchronize( const Logbook& logbook )
{
    Debug::Throw( "Logbook::synchronize.\n" );

    // retrieve logbook entries
    Base::KeySet<LogEntry> newEntries( logbook.entries() );
    Base::KeySet<LogEntry> currentEntries( entries() );

    // map of duplicated entries
    QHash< LogEntry*, LogEntry* > duplicates;

    // merge new entries into current entries
    foreach( LogEntry* entry, newEntries )
    {

        // check if there is an entry with matching creation and modification time
        Base::KeySet< LogEntry >::iterator duplicate( std::find_if(
            currentEntries.begin(),
            currentEntries.end(),
            LogEntry::DuplicateFTor( entry ) ) );

        // if duplicate entry found and modified more recently, skip the new entry
        if( duplicate != currentEntries.end() && (*duplicate)->modification() >= entry->modification() ) continue;

        // retrieve logbook where entry is to be added
        Logbook* child( latestChild() );

        // create a new entry
        LogEntry *copy( entry->clone() );

        // associate entry with logbook
        Key::associate( copy, child );

        // set child as modified
        child->setModified( true );

        // safe remove the duplicated entry
        if( duplicate != currentEntries.end() )
        {
            // set logbooks as modified
            // and disassociate with entry
            Base::KeySet<Logbook> logbooks( *duplicate );
            foreach( Logbook* logbook, logbooks )
            {
                logbook->setModified( true );
                Base::Key::disassociate( logbook, *duplicate );
            }

            // insert duplicate pairs in map
            duplicates.insert( *duplicate, entry );

            // reset current entries
            currentEntries = entries();

        }

    }

    return duplicates;

}

//_________________________________
XmlError::List Logbook::xmlErrors( void ) const
{
    Debug::Throw( "Logbook::xmlErrors.\n" );
    XmlError::List out;
    if( error_ ) out.append( error_ );
    foreach( Logbook* logbook, children_ )
    { out <<  logbook->xmlErrors(); }

    return out;
}

//_________________________________
Logbook::List Logbook::children( void ) const
{
    List out;
    foreach( Logbook* logbook, children_ )
    {
        out << logbook;
        List children( logbook->children() );
        out << children;
    }

    return out;
}

//_________________________________
Logbook* Logbook::latestChild( void )
{

    Debug::Throw( "Logbook::latestChild.\n" );

    // get older parent
    Logbook* dest = 0;

    // check parent number of entries
    if( Base::KeySet<LogEntry>(this).size() < MaxEntries ) dest = this;

    // check if one existsing child is not complete
    foreach( Logbook* logbook, children_ )
    {
        if( logbook && Base::KeySet<LogEntry>(logbook).size() < MaxEntries )
        {
            dest = logbook;
            break;
        }
    }

    // add a new child if nothing found
    if( !dest )
    {

        dest = new Logbook();
        dest->setTitle( title() );
        dest->setDirectory( directory() );
        dest->setAuthor( author() );
        dest->setFile( _childFilename( file(), children_.size() ).addPath( file().path() ) );
        dest->setModified( true );

        children_ << dest;
        setModified( true );

        // associate to existing FileCheck if any
        Base::KeySet<FileCheck> fileChecks( this );
        if( !fileChecks.empty() )
        {
            Q_ASSERT( fileChecks.size() == 1 );
            (*fileChecks.begin())->registerLogbook( dest );
        }

    }

    return dest;
}

//_________________________________
Base::KeySet<LogEntry> Logbook::entries( void ) const
{

    Base::KeySet<LogEntry> out( this );
    foreach( Logbook* logbook, children_ ) out.merge( logbook->entries() );
    return out;

}

//_________________________________
Base::KeySet<Attachment> Logbook::attachments( void ) const
{

    Base::KeySet<Attachment> out;

    // loop over associated entries, add entries associated attachments
    Base::KeySet<LogEntry> entries( this );
    foreach( LogEntry* entry, entries ) out.merge( Base::KeySet<Attachment>(entry) );

    // loop over children, add associated attachments
    foreach( Logbook* logbook, children_ ) out.merge( logbook->attachments() );

    return out;

}

//___________________________________
void Logbook::truncateRecentEntriesList( int maxCount )
{

    Debug::Throw( "Logbook::truncateRecentEntriesList.\n" );
    while( recentEntries_.size() > maxCount )
    { recentEntries_.removeFirst(); }

}

//_________________________________
void Logbook::removeEmptyChildren( void )
{
    Debug::Throw( "Logbook::removeEmptyChildren.\n" );

    // loop over children
    List tmp;
    foreach( Logbook* logbook, children_ )
    {
        logbook->removeEmptyChildren();
        if( logbook->empty() )
        {

            // remove file
            if( !logbook->file().isEmpty() ) logbook->file().remove();

            delete logbook;

        } else tmp << logbook;
    }

    children_ = tmp;
    return;
}

//_________________________________
QList<LogEntry*> Logbook::recentEntries( void ) const
{

    QList<LogEntry*> out;
    if( recentEntries_.empty() ) return out;

    Base::KeySet<LogEntry> entries( Logbook::entries() );
    foreach( const TimeStamp& timeStamp, recentEntries_ )
    {
        Base::KeySet<LogEntry>::const_iterator entryIter( std::find_if( entries.begin(), entries.end(), LogEntry::SameCreationFTor( timeStamp ) ) );
        if( entryIter != entries.end() ) out << *entryIter;
    }

    return out;

}

//_________________________________
void Logbook::addRecentEntry( const LogEntry* entry )
{

    Debug::Throw( "Logbook::addRecentEntry.\n" );
    TimeStamp timeStamp( entry->creation() );

    // first remove time stamp from list if it exists
    recentEntries_.erase( std::remove( recentEntries_.begin(), recentEntries_.end(), timeStamp ), recentEntries_.end() );

    // add again at the end of the list
    recentEntries_ << timeStamp;

    // mark logbook as modified
    setModified( true );

}

//_________________________________
void Logbook::setFile( const File& file, bool recursive )
{
    Debug::Throw( "Logbook::setFile.\n" );

    // update file and last saved timestamp
    file_ = file;
    saved_ = File( file_ ).lastModified();

    // update children files
    if( recursive )
    {

        // write children
        unsigned int childCount=0;
        foreach( Logbook* logbook, children_ )
        {
            File childFilename( _childFilename( file, childCount ).addPath( file.path() ) );
            logbook->setParentFile( file );
            logbook->setFile( childFilename, true );
            ++childCount;
        }

    }

}

//_________________________________
bool Logbook::needsBackup( void ) const
{
    Debug::Throw( "Logbook::needsBackup.\n" );
    if( !backup().isValid() ) return true;
    return( int(TimeStamp::now())-int(backup()) > (24*3600)*XmlOptions::get().get<double>( "BACKUP_ITV" ) );
}

//_________________________________
QString Logbook::backupFilename( void ) const
{
    Debug::Throw( "Logbook::MakeBackupFilename.\n" );
    QString head( File( file_ ).truncatedName() );
    QString foot( File( file_ ).extension() );
    if( !foot.isEmpty() ) foot = QString(".") + foot;
    QString tag( TimeStamp::now().toString( TimeStamp::DateTag ) );

    QString out;
    QTextStream( &out ) << head << "_backup_" << tag << foot;

    // check if file exists, add index
    for( int index = 1; File( out ).exists(); ++index )
    {
        out.clear();
        QTextStream( &out ) << head << "_backup_" << tag << "_" << index << foot;
    }

    return out;
}

//_________________________________
bool Logbook::setReadOnly( bool value )
{

    Debug::Throw( "Logbook::setReadOnly.\n" );
    bool changed( false );
    if( readOnly_ != value )
    {
        // update value
        readOnly_ = value;
        changed = true;

        // emit signal
        emit readOnlyChanged( readOnly_ );

    }

    // also change permission on children
    foreach( Logbook* logbook, children_ )
    { changed |= logbook->setReadOnly( value ); }

    return changed;

}

//_________________________________
bool Logbook::setIsBackup( bool value )
{

    Debug::Throw( "Logbook::setIsBackup.\n" );
    bool changed( false );
    if( isBackup_ != value )
    {
        // update value
        isBackup_ = value;
        changed = true;

    }

    // also change permission on children
    foreach( Logbook* logbook, children_ )
    { changed |= logbook->setIsBackup( value ); }

    return changed;

}

//_________________________________
void Logbook::setModified( bool value )
{
    Debug::Throw( "Logbook::setModified.\n");
    modified_ = value;
    if( value ) setModification( TimeStamp::now() );
}

//_________________________________
void Logbook::setModifiedRecursive( bool value )
{
    Debug::Throw( "Logbook::SetModifiedRecursive.\n" );
    modified_ = value;
    if( value ) setModification( TimeStamp::now() );
    foreach( Logbook* logbook, children_ )
    { logbook->setModifiedRecursive( value ); }

}

//_________________________________
void Logbook::setModification( const TimeStamp& stamp )
{
    Debug::Throw( "Logbook::SetModification.\n" );
    modification_ = stamp;
}

//_________________________________
bool Logbook::modified( void ) const
{
    Debug::Throw( "Logbook::modified.\n" );

    if( modified_ ) return true;

    foreach( Logbook* logbook, children_ )
    { if( logbook->modified() ) return true; }

    return false;
}

//______________________________________________________________________
bool Logbook::setSortMethod( Logbook::SortMethod sortMethod )
{
    Debug::Throw( "Logbook::setSortMethod.\n" );
    bool changed = ( this->sortMethod() != sortMethod );
    if( changed ) {
        sortMethod_ = sortMethod;
        setModified( true );
    }
    return changed;
}

//______________________________________________________________________
bool Logbook::setSortOrder( int order )
{
    Debug::Throw( "Logbook::setSortOrder.\n" );
    bool changed = (sortOrder() != order );
    if( changed )
    {
        sortOrder_ = order;
        setModified( true );
    }
    return changed;
}

//______________________________________________________________________
bool Logbook::EntryLessFTor::operator () ( LogEntry* first, LogEntry* second ) const
{

    if( order_ ) std::swap( first, second );

    switch( sortMethod_ )
    {

        case Logbook::SortColor:
        return (first->color() < second->color() );
        break;

        case Logbook::SortCreation:
        return (first->creation() < second->creation());
        break;

        case Logbook::SortModification:
        return (first->modification() < second->modification());
        break;

        case Logbook::SortTitle:
        return (first->title() < second->title());
        break;

        case Logbook::SortKeyword:
        return (first->keyword() < second->keyword());

        case Logbook::SortAuthor:
        return (first->author() < second->author());

        default:
        Debug::Throw(0,"EntryLessFTor - invalid sort method.\n" );
        break;
    }
    return false;
}

//______________________________________________________________________
void Logbook::addBackup( const File& file )
{
    Debug::Throw( "Logbook::addBackup.\n" );
    backupFiles_ << Backup( file );
    backup_ = backupFiles_.back().creation();
    setModified( true );
}

//______________________________________________________________________
void Logbook::setBackupFiles( const Backup::List& backups )
{

    Debug::Throw( "Logbook::setBackupFiles.\n" );
    if( backupFiles_ == backups ) return;
    backupFiles_ = backups;

    if( backupFiles_.empty() ) backup_ = TimeStamp();
    else backup_ = backupFiles_.back().creation();

    setModified( true );

}

//______________________________________________________________________
void Logbook::_readRecentEntries( const QDomElement& element )
{

    Debug::Throw( "Logbook::_readRecentEntries.\n" );
    recentEntries_.clear();

    // loop over children
    for(QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        QDomElement childElement = node.toElement();
        if( childElement.isNull() ) continue;

        // children
        QString tagName( childElement.tagName() );
        if( tagName == Xml::Creation ) recentEntries_ << XmlTimeStamp( childElement );

    }

}

//______________________________________________________________________
QDomElement Logbook::_recentEntriesElement( QDomDocument& document ) const
{
    Debug::Throw( "Logbook::_recentEntriesElement.\n" );

    QDomElement out( document.createElement( Xml::RecentEntries ) );
    foreach( const TimeStamp& timeStamp, recentEntries_ )
    { out.appendChild( XmlTimeStamp( timeStamp ).domElement( Xml::Creation, document ) ); }

    return out;

}

//______________________________________________________________________
File Logbook::_childFilename( const File& file, int childCount ) const
{

    const File head( file.localName().truncatedName() );
    QString foot( file.extension() );
    if( !foot.isEmpty() ) foot = QString(".") + foot;

    const QString out = QString( "%1_include_%2%3" )
        .arg( head )
        .arg( childCount )
        .arg( foot );

    Debug::Throw( ) << "Logbook::_MakeChildFilename - \"" << out << "\".\n";
    return out;

}
