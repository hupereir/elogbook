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
#include "Operators.h"
#include "TimeStamp.h"

#include <QDomDocument>
#include <QDomElement>

//* store backup information
class Backup final: private Base::Counter<Backup>
{

    public:

    //* constructor
    explicit Backup( const File& file = File(), const TimeStamp& creation = TimeStamp::now() ):
        Counter( "Backup" ),
        file_( file ),
        creation_( creation ),
        valid_( true )
    {}

    //* constructor from Dom
    explicit Backup( const QDomElement& );

    //*@name accessors
    //@{

    //* get dom
    QDomElement domElement( QDomDocument& ) const;

    //* creation
    const TimeStamp& creation() const
    { return creation_; }

    //* file
    const File& file() const
    { return file_; }

    //* validity
    bool isValid() const
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
    void checkValidity()
    { valid_ = file_.exists(); }

    //@}

    //* list
    using List = QList<Backup>;

    //* test validity
    using InvalidFTor = Base::Functor::UnaryFalse<Backup, &Backup::isValid>;

    private:

    //* filename
    File file_;

    //* timestamp
    TimeStamp creation_;

    //* validity
    bool valid_ = false;

};

//* equal to operator
inline bool operator == (const Backup& first, const Backup& second)
{ return first.creation() == second.creation() && first.file() == second.file(); }

//* less than operator
inline bool operator < (const Backup& first, const Backup& second)
{
    if( first.creation() != second.creation() ) return first.creation() < second.creation();
    else return first.file() < second.file();
}

#endif
