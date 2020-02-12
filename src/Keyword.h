#ifndef Keyword_h
#define Keyword_h

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
#include "QOrderedSet.h"

#include <QDomElement>
#include <QDomDocument>
#include <QSet>
#include <QString>
#include <QTextStream>
#include <QVector>

//* log entry keyword
class Keyword final: private Base::Counter<Keyword>
{

    public:

    //* default keyword
    static const Keyword Default;

    //* used when LogEntry keyword is not defined
    static const QString MimeType;

    //* constructor
    explicit Keyword( const QString& = QString() );

    //* constructor from DOM
    explicit Keyword( const QDomElement& );

    //* keyword set
    using Set = QSet<Keyword>;
    using OrderedSet = QOrderedSet<Keyword>;
    using List = QVector<Keyword>;

    //*@name accessors
    //@{

    //* DomElement
    QDomElement domElement( QDomDocument& ) const;

    //* full keyword
    const QString& get() const
    { return value_; }

    //* true if is root
    bool isRoot() const
    { return value_.isEmpty() || value_ == QString( '/' ); }

    //* current keyword
    QString current() const;

    //* parent keyword
    Keyword parent() const;

    /** true if this keyword is direct child of this one */
    bool isChild( const Keyword& keyword ) const;

    //* true if this keyword in descendance of argument
    bool inherits( const Keyword& keyword ) const;

    //@}

    //*@name modifiers
    //@{

    //* clear
    void clear()
    { value_ = _format( QString() ); }

    //* set full keyword
    void set( const QString &value )
    { value_ = _format( value ); }

    //* append
    Keyword& append( const QString &value );

    //@}

    private:

    //* format keyword
    QString _format( QString ) const;

    //* full value
    QString value_;

    //* streamer
    friend QTextStream& operator << (QTextStream& out, const Keyword& keyword )
    {
        out << keyword.get();
        return out;
    }
};

//* equal to operator
inline bool operator == (const Keyword& first, const Keyword& second)
{ return first.get() == second.get(); }

//* less than operator
inline bool operator < (const Keyword& first, const Keyword& second)
{ return first.get() < second.get(); }

//* hash
inline uint qHash( const Keyword& keyword )
{ return qHash( keyword.get() ); }

#endif
