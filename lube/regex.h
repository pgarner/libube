/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2016
 */

#ifndef REGEX_H
#define REGEX_H

#include <lube/var.h>

namespace libube
{
    /**
     * String regex functor
     *
     * A unary functor for handling regular expressions
     */
    class RegExFunctor : public StringFunctor
    {
    public:
        RegExFunctor(var iRE);
        virtual ~RegExFunctor();
    protected:
        void* mRE;
    };

#   define BASIC_REGEX_FUNCTOR_DECL(f)                      \
    class f : public RegExFunctor                           \
    {                                                       \
    public:                                                 \
        f(var iRE) : RegExFunctor(iRE) {};                  \
        void string(const var& iVar, var& oVar) const;      \
    };

    // RegEx functors
    BASIC_REGEX_FUNCTOR_DECL(Search)
    BASIC_REGEX_FUNCTOR_DECL(Match)
    class Replace : public RegExFunctor
    {
    public:
        Replace(var iRE, var iStr);
        void string(const var& iVar, var& oVar) const;
    private:
        var mStr;
    };
}

#endif // REGEX_H
