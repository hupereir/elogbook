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
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
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

const QString Logbook::LOGBOOK_NO_TITLE( QObject::tr( "My electronic logbook" ) );
const QString Logbook::LOGBOOK_NO_AUTHOR( QObject::tr( "anonymous" ) );
const QString Logbook::LOGBOOK_NO_FILE;
const QString Logbook::LOGBOOK_NO_DIRECTORY;

//________________________________
Logbook::Logbook( const File& file ):
    Counter( "Logbook" ),
    directory_( LOGBOOK_NO_DIRECTORY ),
    title_( LOGBOOK_NO_TITLE ),
    author_( LOGBOOK_NO_AUTHOR ),
    modified_( false ),
    readOnly_( false ),
    isBackup_( false ),
    creation_( TimeStamp::now() ),
    sortMethod_( Logbook::SORT_CREATION ),
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
    BASE::KeySet<LogEntry> entries( this );
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
    if( tagName != XML::LOGBOOK )
    {
        Debug::Throw(0) << "Logbook::read - invalid tag name: " << tagName << endl;
        return false;
    }

    // read attributes
    QDomNamedNodeMap attributes( docElement.attributes() );
    for( unsigned int i=0; i<attributes.length(); i++ )
    {

        QDomAttr attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        QString name( attribute.name() );
        QString value( attribute.value() );

        if( name == XML::TITLE ) setTitle( XmlString( value ).toText() );
        else if( name == XML::FILE ) setFile( File( XmlString( value ).toText() ) );
        else if( name == XML::PARENT_FILE ) setParentFile( XmlString( value ).toText() );
        else if( name == XML::DIRECTORY ) setDirectory( File( XmlString( value ).toText() ) );
        else if( name == XML::AUTHOR ) setAuthor( XmlString( value ).toText() );
        else if( name == XML::SORT_METHOD ) setSortMethod( (SortMethod) value.toInt() );
        else if( name == XML::SORT_ORDER ) setSortOrder( value.toInt() );
        else if( name == XML::READ_ONLY ) setReadOnly( value.toInt() );
        else if( name == XML::LOGBOOK_BACKUP ) setIsBackup( value.toInt() );
        else if( name == XML::ENTRIES ) {

            setXmlEntries( value.toInt() );
            emit maximumProgressAvailable( value.toInt() );

        } else if( name == XML::CHILDREN ) setXmlChildren( value.toInt() );
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
        if( tagName == XML::COMMENTS ) setComments( XmlString( element.text() ).toText() );
        else if( tagName == XML::CREATION ) setCreation( XmlTimeStamp( element ) );
        else if( tagName == XML::MODIFICATION ) setModification( XmlTimeStamp( element ) );
        else if( tagName == XML::BACKUP ) setBackup( XmlTimeStamp( element ) );
        else if( tagName == XML::RECENT_ENTRIES ) _readRecentEntries( element );
        else if( tagName == XML::LOGBOOK_BACKUP ) backupFiles_ << Backup( element );
        else if( tagName == XML::ENTRY ) {

            LogEntry* entry = new LogEntry( element );
            Key::associate( latestChild(), entry );
            entryCount++;
            if( !(entryCount%progress) ) emit progressAvailable( progress );

        } else if( tagName == XML::CHILD ) {

            // try retrieve file from attributes
            QString file_attribute( element.attribute( XML::FILE ) );
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
            connect( child, SIGNAL( progressAvailable( int ) ), SIGNAL( progressAvailable( int ) ) );

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

    Debug::Throw( ) << "Logbook::write - \"" << file << "\".\n";
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
        QDomElement top = document.createElement( XML::LOGBOOK );
        if( !title_.isEmpty() ) top.setAttribute( XML::TITLE, XmlString( title_ ).toXml() );
        if( !directory_.isEmpty() ) top.setAttribute( XML::DIRECTORY, XmlString(directory_ ) );
        if( !author_.isEmpty() ) top.setAttribute( XML::AUTHOR, XmlString( author_ ).toXml() ) ;
        if( !parentFile_.isEmpty() ) top.setAttribute( XML::PARENT_FILE, XmlString( parentFile_ ).toXml() );

        top.setAttribute( XML::SORT_METHOD, QString().setNum( sortMethod_ ) );
        top.setAttribute( XML::SORT_ORDER, QString().setNum( sortOrder_ ) );
        top.setAttribute( XML::READ_ONLY, QString().setNum( readOnly_ ) );
        top.setAttribute( XML::LOGBOOK_BACKUP, QString().setNum( isBackup_ ) );

        // update number of entries and children
        top.setAttribute( XML::ENTRIES, QString().setNum(xmlEntries()) );
        top.setAttribute( XML::CHILDREN, QString().setNum(xmlChildren()) );

        // append node
        document.appendChild( top );

        // comments
        if( comments().size() )
        {
            QDomElement comments_element = document.createElement( XML::COMMENTS );
            QDomText comments_text = document.createTextNode( XmlString( comments() ).toXml() );
            comments_element.appendChild( comments_text );
            top.appendChild( comments_element );
        }

        // write time stamps
        if( creation().isValid() ) top.appendChild( XmlTimeStamp( creation() ).domElement( XML::CREATION, document ) );
        if( modification().isValid() ) top.appendChild( XmlTimeStamp( modification() ).domElement( XML::MODIFICATION, document ) );
        if( backup().isValid() ) top.appendChild( XmlTimeStamp( backup() ).domElement( XML::BACKUP, document ) );

        // write recent entries
        if( !recentEntries_.empty() ) top.appendChild( _recentEntriesElement( document ) );

        // write backup files
        foreach( const Backup& backup, backupFiles_ )
        { top.appendChild( backup.domElement( document ) ); }

        // write all entries
        static unsigned int progress( 10 );
        unsigned int entryCount( 0 );
        BASE::KeySet<LogEntry> entries( this );
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
            QDomElement childElement = document.createElement( XML::CHILD );
            childElement.setAttribute( XML::FILE, XmlString( childFilename ).toXml() );
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

    } else { emit progressAvailable( BASE::KeySet<LogEntry>( this ).size() ); }


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
    BASE::KeySet<LogEntry> newEntries( logbook.entries() );
    BASE::KeySet<LogEntry> currentEntries( entries() );

    // map of duplicated entries
    QHash< LogEntry*, LogEntry* > duplicates;

    // merge new entries into current entries
    foreach( LogEntry* entry, newEntries )
    {

        // check if there is an entry with matching creation and modification time
        BASE::KeySet< LogEntry >::iterator duplicate( std::find_if(
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
            BASE::KeySet<Logbook> logbooks( *duplicate );
            foreach( Logbook* logbook, logbooks )
            {
                logbook->setModified( true );
                BASE::Key::disassociate( logbook, *duplicate );
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
    if( BASE::KeySet<LogEntry>(this).size() < MAX_ENTRIES ) dest = this;

    // check if one existsing child is not complete
    foreach( Logbook* logbook, children_ )
    {
        if( logbook && BASE::KeySet<LogEntry>(logbook).size() < MAX_ENTRIES )
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
        BASE::KeySet<FileCheck> fileChecks( this );
        if( !fileChecks.empty() )
        {
            Q_ASSERT( fileChecks.size() == 1 );
            (*fileChecks.begin())->registerLogbook( dest );
        }

    }

    return dest;
}

//_________________________________
BASE::KeySet<LogEntry> Logbook::entries( void ) const
{

    BASE::KeySet<LogEntry> out( this );
    foreach( Logbook* logbook, children_ ) out.merge( logbook->entries() );
    return out;

}

//_________________________________
BASE::KeySet<Attachment> Logbook::attachments( void ) const
{

    BASE::KeySet<Attachment> out;

    // loop over associated entries, add entries associated attachments
    BASE::KeySet<LogEntry> entries( this );
    foreach( LogEntry* entry, entries ) out.merge( BASE::KeySet<Attachment>(entry) );

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

    BASE::KeySet<LogEntry> entries( Logbook::entries() );
    foreach( const TimeStamp& timeStamp, recentEntries_ )
    {
        BASE::KeySet<LogEntry>::const_iterator entryIter( std::find_if( entries.begin(), entries.end(), LogEntry::SameCreationFTor( timeStamp ) ) );
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

    // adds again at the end of the list
    recentEntries_ << timeStamp;

    // mark logbook as modified
    setModified( true );

}

//_________________________________
void Logbook::setFile( const File& file )
{
    Debug::Throw( "Logbook::setFile.\n" );
    file_ = file;
    saved_ = File( file_ ).lastModified();
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
    QString tag( TimeStamp::now().toString( TimeStamp::DATE_TAG ) );

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

        case Logbook::SORT_COLOR:
        return (first->color() < second->color() );
        break;

        case Logbook::SORT_CREATION:
        return (first->creation() < second->creation());
        break;

        case Logbook::SORT_MODIFICATION:
        return (first->modification() < second->modification());
        break;

        case Logbook::SORT_TITLE:
        return (first->title() < second->title());
        break;

        case Logbook::SORT_KEYWORD:
        return (first->keyword() < second->keyword());

        case Logbook::SORT_AUTHOR:
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
        if( tagName == XML::CREATION ) recentEntries_ << XmlTimeStamp( childElement );

    }

}

//______________________________________________________________________
QDomElement Logbook::_recentEntriesElement( QDomDocument& document ) const
{
    Debug::Throw( "Logbook::_recentEntriesElement.\n" );

    QDomElement out( document.createElement( XML::RECENT_ENTRIES ) );
    foreach( const TimeStamp& timeStamp, recentEntries_ )
    { out.appendChild( XmlTimeStamp( timeStamp ).domElement( XML::CREATION, document ) ); }

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
