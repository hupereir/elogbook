#ifndef Backup_h
#define Backup_h

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
#include "TimeStamp.h"

#include <QDomDocument>
#include <QDomElement>

//* store backup information
class Backup: private Base::Counter<Backup>
{

    public:

    //* constructor
    Backup( const File& file = File(), const TimeStamp& creation = TimeStamp::now() ):
        Counter( "Backup" ),
        file_( file ),
        creation_( creation ),
        valid_( true )
    {}

    //* constructor from Dom
    Backup( const QDomElement& );

    //* equal to operator
    bool operator == (const Backup& other ) const
    { return creation() == other.creation() && file() == other.file(); }

    //* less than operator
    bool operator < (const Backup& other ) const
    {
        if( creation() != other.creation() ) return creation() < other.creation();
        else return file() < other.file();
    }

    //*@name accessors
    //@{

    //* get dom
    QDomElement domElement( QDomDocument& ) const;

    //* creation
    const TimeStamp& creation( void ) const
    { return creation_; }

    //* file
    const File& file( void ) const
    { return file_; }

    //* validity
    bool isValid( void ) const
    { return valid_; }

    //@}

    //*@name modifiers

    //* time
    void setCreation( const TimeStamp& creation )
    { creation_ = creation; }

    //* file
    void setFile( const File& file )
    { file_ = file; }

    //* check validity
    void checkValidity( void )
    { valid_ = file_.exists(); }

    //@}

    //* list
    class List: public QList<Backup>
    {
        public:

        //* constructor
        List( void )
        {}

        //* constructor
        List( const QList<Backup>& other ):
            QList<Backup>( other )
        {}

        //* validity
        void checkValidity( void )
        {
            for( iterator iter = begin(); iter != end(); ++iter )
            { iter->checkValidity(); }
        }

    };

    //* test validity
    class InvalidFTor
    {

        public:

        //* predicate
        bool operator() ( Backup backup ) const
        { return !backup.isValid(); }

    };

    private:

    //* filename
    File file_;

    //* timestamp
    TimeStamp creation_;

    //* validity
    bool valid_ = false;

};

#endif
