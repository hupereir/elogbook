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
#include "XmlDef.h"
#include "XmlTimeStamp.h"

#include <QDomDocument>
#include <QFile>
#include <QTextStream>

#include <new>

namespace
{

    namespace Local
    {

        //________________________________________________________
        QByteArray safeUncompress( const QByteArray& content )
        {
            try
            {

                return qUncompress( content );

            } catch( std::bad_alloc& exception ) {

                Debug::Throw() << "safeUncompress - caught bad_alloc exception: " << exception.what() << endl;
                return QByteArray();

            }

        }

        //______________________________________________________________________
        File childFileName( File file, int childCount )
        {

            const auto head( file.localName().truncatedName() );
            QString foot( file.extension() );
            if( !foot.isEmpty() ) foot = QString(".") + foot;

            File out( QString( "%1_include_%2%3" )
                .arg( head )
                .arg( childCount )
                .arg( foot ) );

            Debug::Throw( ) << "Local::childFileName - \"" << out << "\".\n";
            return out;

        }

    }

}

//________________________________
// public methods

const QString Logbook::NoTitle( QObject::tr( "My electronic logbook" ) );
const QString Logbook::NoAuthor( QObject::tr( "anonymous" ) );
const QString Logbook::NoFile;
const QString Logbook::NoDirectory;

//________________________________
Logbook::Logbook( File file ):
    Counter( "Logbook" ),
    directory_( NoDirectory ),
    title_( NoTitle ),
    author_( NoAuthor ),
    creation_( TimeStamp::now() )
{
    Debug::Throw( "Logbook::Logbook. (file)\n" );
    setFile( file );
    read();
}

//_________________________________
Logbook::~Logbook()
{
    Debug::Throw( "Logbook::~Logbook.\n" );

    // delete log children
    children_.clear();

    // delete associated entries
    Base::KeySet<LogEntry> entries( this );
    for( const auto& entry:entries ) delete entry;

}

//_________________________________
void Logbook::setUseCompression( bool value )
{
    useCompression_ = value;
    for( const auto& logbook:children_ )
    { logbook->setUseCompression( value ); }
}

//_________________________________
bool Logbook::read()
{

    Debug::Throw( "Logbook::read.\n" );

    if( file_.isEmpty() )
    {
        Debug::Throw( "Logbook::read - file is empty.\n" );
        return false;
    }

    // update StateFrame
    emit messageAvailable( tr( "Reading '%1'" ).arg( file_.localName() ) );

    // check input file
    if( !file_.exists() ) {
        Debug::Throw(0) << "Logbook::read - ERROR: cannot access file \"" << file_ << "\".\n";
        return false;
    }

    // delete associated entries
    for( const auto& entry:this->entries() )
    { delete entry; }

    // parse the file
    QFile file( file_ );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
        Debug::Throw(0, "Logbook::read - cannot open file.\n" );
        return false;
    }

    // read everything from file
    // try read compressed and try uncompress
    auto content( file.readAll() );
    auto uncompressed( Local::safeUncompress( content ) );

    // try read raw if failed
    if( uncompressed.isEmpty() ) uncompressed = content;

    // create document
    QDomDocument document;
    if( !document.setContent( uncompressed, error_ ) ) return false;

    // read first child
    auto docElement = document.documentElement();
    const auto tagName( docElement.tagName() );
    if( tagName != Xml::Logbook )
    {
        Debug::Throw(0) << "Logbook::read - invalid tag name: " << tagName << endl;
        return false;
    }

    // read attributes
    const auto attributes( docElement.attributes() );
    for( int i=0; i<attributes.count(); i++ )
    {

        const auto attribute( attributes.item( i ).toAttr() );
        if( attribute.isNull() ) continue;
        QString name( attribute.name() );
        QString value( attribute.value() );

        if( name == Xml::Title ) setTitle( value );
        else if( name == Xml::File ) setFile( File( value ) );
        else if( name == Xml::ParentFile ) setParentFile( File( value ) );
        else if( name == Xml::Directory ) setDirectory( File( value ) );
        else if( name == Xml::Author ) setAuthor( value );
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
    int entryCount( 0 );
    for( auto&& node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        const auto element = node.toElement();
        if( element.isNull() ) continue;

        const auto tagName( element.tagName() );

        // children
        if( tagName == Xml::Comments ) setComments( element.text() );
        else if( tagName == Xml::Creation ) setCreation( XmlTimeStamp( element ) );
        else if( tagName == Xml::Modification ) setModification( XmlTimeStamp( element ) );
        else if( tagName == Xml::Backup ) setBackup( XmlTimeStamp( element ) );
        else if( tagName == Xml::RecentEntries ) _readRecentEntries( element );
        else if( tagName == Xml::BackupMask ) backupFiles_.append( Backup( element ) );
        else if( tagName == Xml::Entry ) {

            // create entry. Make sure it has a non zero keyword
            LogEntry* entry = new LogEntry( element );

            // make sure there is at least one valid keyword
            auto keywords( entry->keywords() );
            for( const auto& keyword:keywords )
            { if( keyword.isRoot() ) entry->removeKeyword( keyword ); }
            if( entry->keywords().empty() ) entry->addKeyword( Keyword::Default );

            Base::Key::associate( this, entry );
            entryCount++;
            emit progressAvailable( 1 );

        } else if( tagName == Xml::Child ) {

            // try retrieve file from attributes
            auto fileAttribute( element.attribute( Xml::File ) );
            if( fileAttribute.isEmpty() )
            {
                Debug::Throw(0) << "Logbook::read - no file given for child" << endl;
                continue;
            }

            File file( fileAttribute );
            if( !file.isAbsolute() ) file.addPath( Logbook::file_.path() );
            LogbookPtr child( new Logbook );
            child->setFile( file );
            child->setUseCompression( useCompression_ );

            // propagate progressAvailable signal.
            connect( child.get(), SIGNAL(progressAvailable(int)), SIGNAL(progressAvailable(int)) );
            connect( child.get(), SIGNAL(messageAvailable(QString)), SIGNAL(messageAvailable(QString)) );
            child->read();
            children_.append( child );

        } else Debug::Throw(0) << "Logbook::read - unrecognized tagName: " << tagName << endl;

    }

    // discard modifications
    setModified( false );
    saved_ = Logbook::file_.lastModified();
    return true;

}

//_________________________________
bool Logbook::write( File file )
{

    Debug::Throw( "Logbook::write.\n" );

    // check filename
    if( file.isEmpty() ) file = Logbook::file_;
    if( file.isEmpty() ) return false;

    bool completed = true;

    // check number of entries and children to save in header
    if( setXmlEntries( entries().size() ) || setXmlChildren( children().size() ) )
    { setModified( true ); }

    emit maximumProgressAvailable( xmlEntries() );

    // write logbook if filename differs from origin or logbook is modified
    if( file != file_ || modified_ )
    {

        // gets last saved timestamp
        TimeStamp lastSaved( file.lastModified() );

        // make a backup of the file, if necessary
        if( XmlOptions::get().get<bool>( "FILE_BACKUP" ) )
        { file.backup(); }

        // update stateFrame
        emit messageAvailable( tr( "Writing '%1'" ).arg( file.localName() ) );

        if( !QFile( file ).open( QIODevice::WriteOnly ) )
        {
            Debug::Throw(0) << "Logbook::write - unable to write to file " << file << endl;
            return false;
        }

        // create document
        QDomDocument document;

        // create main element
        auto top = document.createElement( Xml::Logbook );
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
            auto commentsElement = document.createElement( Xml::Comments );
            auto commentsText = document.createTextNode( comments() );
            commentsElement.appendChild( commentsText );
            top.appendChild( commentsElement );
        }

        // write time stamps
        if( creation().isValid() ) top.appendChild( XmlTimeStamp( creation() ).domElement( Xml::Creation, document ) );
        if( modification().isValid() ) top.appendChild( XmlTimeStamp( modification() ).domElement( Xml::Modification, document ) );
        if( backup().isValid() ) top.appendChild( XmlTimeStamp( backup() ).domElement( Xml::Backup, document ) );

        // write recent entries
        if( !recentEntries_.empty() ) top.appendChild( _recentEntriesElement( document ) );

        // write backup files
        for( const auto& backup:backupFiles_ )
        { top.appendChild( backup.domElement( document ) ); }

        // write all entries
        int entryCount( 0 );
        Base::KeySet<LogEntry> entries( this );
        for( const auto& entry:entries )
        {

            top.appendChild( entry->domElement( document ) );
            entryCount++;
            emit progressAvailable( 1 );

        }

        // dump all logbook childrens
        for( int childCount = 0; childCount < children_.size(); ++childCount )
        {
            auto childFileName = Local::childFileName( file, childCount );
            auto childElement = document.createElement( Xml::Child );
            childElement.setAttribute( Xml::File, childFileName );
            top.appendChild( childElement );
        }

        // finish
        QFile out( file );
        out.open( QIODevice::WriteOnly );
        if( useCompression_ ) out.write( qCompress( document.toByteArray() ) );
        else out.write( document.toByteArray() );
        out.close();

        // gets/check new saved timestamp
        TimeStamp savedNew( file.lastModified() );
        if( !( lastSaved < savedNew ) ) completed = false;
        else if( file == file_ )  modified_ = false;

        // assign new filename
        if( file != file_ ) setFile( file );

    } else { emit progressAvailable( Base::KeySet<LogEntry>( this ).size() ); }

    // update saved timeStamp
    saved_ = file_.lastModified();

    // write children
    int childCount=0;
    for( const auto& logbook:children_ )
    {

        File childFileName( Local::childFileName( file, childCount ).addPath( file.path() ) );

        logbook->setParentFile( file );
        if( !logbook->write( childFileName ) ) completed = false;

        ++childCount;

    }

    return completed;

}

//_________________________________
QHash<LogEntry*,LogEntry*> Logbook::synchronize( const Logbook& logbook )
{
    Debug::Throw( "Logbook::synchronize.\n" );

    // retrieve logbook entries
    auto newEntries( logbook.entries() );
    auto currentEntries( entries() );

    // map of duplicated entries
    QHash< LogEntry*, LogEntry* > duplicates;

    // merge new entries into current entries
    for( const auto& entry:newEntries )
    {

        // check if there is an entry with matching creation and modification time
        auto duplicate( std::find_if( currentEntries.begin(), currentEntries.end(), LogEntry::DuplicateFTor( entry ) ) );

        // if duplicate entry found and modified more recently, skip the new entry
        if( duplicate != currentEntries.end() && (*duplicate)->modification() >= entry->modification() ) continue;

        // retrieve logbook where entry is to be added
        auto child( latestChild() );

        // create a new entry
        auto copy( entry->copy() );

        // associate entry with logbook
        Base::Key::associate( copy, child.get() );

        // set child as modified
        child->setModified( true );

        // safe remove the duplicated entry
        if( duplicate != currentEntries.end() )
        {
            // set logbooks as modified
            // and disassociate with entry
            for( const auto& logbook:Base::KeySet<Logbook>( *duplicate ) )
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
XmlError::List Logbook::xmlErrors() const
{
    Debug::Throw( "Logbook::xmlErrors.\n" );
    XmlError::List out;
    if( error_ ) out.append( error_ );
    for( const auto& logbook:children_ )
    { out.append( logbook->xmlErrors() ); }

    return out;
}

//_________________________________
Logbook::List Logbook::children() const
{
    List out;
    for( const auto& logbook:children_ )
    {
        out.append( logbook );
        List children( logbook->children() );
        out.append( children );
    }

    return out;
}

//_________________________________
Logbook::LogbookPtr Logbook::latestChild()
{

    Debug::Throw( "Logbook::latestChild.\n" );

    // check if one existsing child is not complete
    for( const auto& logbook:children_ )
    {
        if( logbook && Base::KeySet<LogEntry>(logbook.get()).size() < MaxEntries )
        { return logbook; }
    }

    // add a new child if nothing found
    LogbookPtr logbook( new Logbook );
    logbook->setTitle( title() );
    logbook->setDirectory( directory() );
    logbook->setAuthor( author() );
    logbook->setFile( Local::childFileName( file_, children_.size() ).addPath( file_.path() ) );
    logbook->setUseCompression( useCompression_ );
    logbook->setModified( true );
    connect( logbook.get(), SIGNAL(messageAvailable(QString)), SIGNAL(messageAvailable(QString)) );

    children_.append( logbook );
    setModified( true );

    // associate to existing FileCheck if any
    Base::KeySet<FileCheck> fileChecks( this );
    if( !fileChecks.empty() )
    { (*fileChecks.begin())->registerLogbook( logbook.get() ); }

    return logbook;

}

//_________________________________
Base::KeySet<LogEntry> Logbook::entries() const
{

    Base::KeySet<LogEntry> out( this );

    for( const auto& logbook:children_ )
    { out.unite( logbook->entries() ); }

    return out;

}

//_________________________________
Base::KeySet<Attachment> Logbook::attachments() const
{

    Base::KeySet<Attachment> out;

    // loop over associated entries, add entries associated attachments
    for( const auto& entry:Base::KeySet<LogEntry>( this ) )
    { out.unite( Base::KeySet<Attachment>(entry) ); }

    // loop over children, add associated attachments
    for( const auto& logbook:children_ )
    { out.unite( logbook->attachments() ); }

    return out;

}

//___________________________________
void Logbook::truncateRecentEntriesList( int maxCount )
{

    Debug::Throw( "Logbook::truncateRecentEntriesList.\n" );
    if( recentEntries_.size() > maxCount )
    { recentEntries_.erase( recentEntries_.begin(), recentEntries_.begin() + (recentEntries_.size() - maxCount ) ); }

}

//_________________________________
void Logbook::removeEmptyChildren()
{
    Debug::Throw( "Logbook::removeEmptyChildren.\n" );

    // loop over children
    /* should try use algorithm instead */
    List tmp;
    for( const auto& logbook:children_ )
    {
        logbook->removeEmptyChildren();
        if( logbook->empty() )
        {

            // remove file
            if( !logbook->file_.isEmpty() ) logbook->file_.remove();

        } else tmp.append( logbook );
    }

    #if QT_VERSION >= 0x040800
    children_.swap( tmp );
    #else
    children_ = tmp;
    #endif

    return;
}

//_________________________________
QList<LogEntry*> Logbook::recentEntries() const
{

    QList<LogEntry*> out;
    if( recentEntries_.empty() ) return out;

    Base::KeySet<LogEntry> entries( this->entries() );
    for( const auto& timeStamp:recentEntries_ )
    {
        auto entryIter( std::find_if( entries.begin(), entries.end(), LogEntry::SameCreationFTor( timeStamp ) ) );
        if( entryIter != entries.end() ) out.append( *entryIter );
    }

    return out;

}

//_________________________________
void Logbook::addRecentEntry( const LogEntry* entry )
{

    Debug::Throw( "Logbook::addRecentEntry.\n" );
    auto timeStamp( entry->creation() );

    // first remove time stamp from list if it exists
    recentEntries_.erase( std::remove( recentEntries_.begin(), recentEntries_.end(), timeStamp ), recentEntries_.end() );

    // add again at the end of the list
    recentEntries_.append( timeStamp );

    // mark logbook as modified
    setModified( true );

}

//_________________________________
void Logbook::setFile( File file, bool recursive )
{
    Debug::Throw( "Logbook::setFile.\n" );

    // update file and last saved timestamp
    file_ = file;
    saved_ = File( file_ ).lastModified();

    // update children files
    if( recursive )
    {

        // write children
        int childCount=0;
        for( const auto& logbook:children_ )
        {
            File childFileName( Local::childFileName( file, childCount ).addPath( file.path() ) );
            logbook->setParentFile( file );
            logbook->setFile( childFileName, true );
            ++childCount;
        }

    }

}

//_________________________________
bool Logbook::needsBackup() const
{
    Debug::Throw( "Logbook::needsBackup.\n" );
    if( !backup().isValid() ) return true;
    return( TimeStamp::now().unixTime() > backup().unixTime() + (24*3600)*XmlOptions::get().get<double>( "BACKUP_ITV" ) );
}

//_________________________________
File Logbook::backupFileName() const
{
    Debug::Throw( "Logbook::MakeBackupFileName.\n" );
    auto head( File( file_ ).truncatedName() );
    auto foot( File( file_ ).extension() );
    if( !foot.isEmpty() ) foot = File( QString(".") + foot );
    QString tag( TimeStamp::now().toString( TimeStamp::Format::DateTag ) );

    File out( QString( "%1_backup_%2%3" ).arg( head, tag, foot ) );

    // check if file exists, add index
    for( int index = 1; File( out ).exists(); ++index )
    { out = File( QString( "%1_backup_%2_%4%3" ).arg( head, tag, foot ).arg(index) ); }

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
    for( const auto& logbook:children_ )
    { if( logbook->setReadOnly( value ) ) changed = true; }

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
    for( const auto& logbook:children_ )
    { if( logbook->setIsBackup( value ) ) changed = true; }

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
    for( const auto& logbook:children_ )
    { logbook->setModifiedRecursive( value ); }

}

//_________________________________
void Logbook::setModification( const TimeStamp& stamp )
{
    Debug::Throw( "Logbook::SetModification.\n" );
    modification_ = stamp;
}

//_________________________________
bool Logbook::modified() const
{ return modified_ || std::any_of( children_.begin(), children_.end(), ModifiedFTor() ); }

//______________________________________________________________________
bool Logbook::setSortMethod( Logbook::SortMethod sortMethod )
{
    Debug::Throw( "Logbook::setSortMethod.\n" );
    bool changed = ( this->sortMethod() != sortMethod );
    if( changed )
    {
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
        {
            auto firstKeywords( first->keywords() );
            auto secondKeywords( second->keywords() );
            if( firstKeywords.empty() ) return true;
            else if( secondKeywords.empty() ) return false;
            else return *firstKeywords.begin() < *secondKeywords.begin();
        }

        case Logbook::SortAuthor:
        return (first->author() < second->author());

        default:
        Debug::Throw(0,"EntryLessFTor - invalid sort method.\n" );
        break;
    }
    return false;
}

//______________________________________________________________________
void Logbook::addBackup( File file )
{
    Debug::Throw( "Logbook::addBackup.\n" );
    backupFiles_.append( Backup( file ) );
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
    for( auto&& node = element.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        const auto childElement = node.toElement();
        if( childElement.isNull() ) continue;

        // children
        const auto tagName( childElement.tagName() );
        if( tagName == Xml::Creation ) recentEntries_.append( XmlTimeStamp( childElement ) );

    }

}

//______________________________________________________________________
QDomElement Logbook::_recentEntriesElement( QDomDocument& document ) const
{
    Debug::Throw( "Logbook::_recentEntriesElement.\n" );

    auto out( document.createElement( Xml::RecentEntries ) );
    for( const auto& timeStamp:recentEntries_ )
    { out.appendChild( XmlTimeStamp( timeStamp ).domElement( Xml::Creation, document ) ); }

    return out;

}
