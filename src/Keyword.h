#ifndef Keyword_h
#define Keyword_h

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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "Counter.h"

#include <QDomElement>
#include <QDomDocument>
#include <QList>
#include <QSet>
#include <QString>
#include <QTextStream>

//! log entry keyword
class Keyword: public Counter
{

    public:

    //! used when LogEntry keyword is not defined
    static const QString MimeType;

    //! constructor
    Keyword( const QString& = QString() );

    //! constructor from dom
    Keyword( const QDomElement& );

    //! destructor
    virtual ~Keyword( void )
    {}

    //! keyword set
    typedef QSet<Keyword> Set;
    typedef QList<Keyword> List;

    //! dom element
    QDomElement domElement( QDomDocument& ) const;

    //! equal to operator
    bool operator == (const Keyword& other ) const
    { return get() == other.get(); }

    //! equal to operator
    bool operator != (const Keyword& other ) const
    { return !( *this == other ); }

    //! less than operator
    bool operator < (const Keyword& other ) const
    { return get() < other.get(); }

    //!@name accessors
    //@{

    //! full keyword
    const QString& get( void ) const
    { return value_; }

    //! current keyword
    QString current( void ) const;

    //! parent keyword
    Keyword parent( void ) const;

    /*! true if this keyword is direct child of this one */
    bool isChild( const Keyword& keyword ) const;

    //! true if this keyword in descendance of argument
    bool inherits( const Keyword& keyword ) const;

    //@}

    //!@name modifiers
    //@{

    //! clear
    void clear( void )
    { value_ = _format( QString() ); }

    //! set full keyword
    void set( const QString& value )
    { value_ = _format( value ); }

    //! append
    Keyword& append( const QString& value );

    //@}

    private:

    //! format keyword
    QString _format( const QString& ) const;

    //! full value
    QString value_;

    //! streamer
    friend QTextStream& operator << (QTextStream& out, const Keyword& keyword )
    {
        out << keyword.get();
        return out;
    }
};

//! hash
inline unsigned int qHash( const Keyword& keyword )
{ return qHash( keyword.get() ); }

#endif
