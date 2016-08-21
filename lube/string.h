/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2016
 */

#ifndef STRING_H
#define STRING_H

#include <lube/func.h>

namespace libube
{
    /**
     * String functor
     *
     * A unary functor for handling strings
     */
    class StringFunctor : public Functor
    {
    public:
        var operator ()(const var& iVar) const;
        var operator ()(const var& iVar, var& oVar) const;
    protected:
        virtual var alloc(var iVar) const;
        virtual void broadcast(var iVar, var& oVar) const;
        virtual void string(const var& iVar, var& oVar) const;
    };


#   define BASIC_STRING_FUNCTOR_DECL(f)                     \
    class f : public StringFunctor                          \
    {                                                       \
    public:                                                 \
        void string(const var& iVar, var& oVar) const;      \
    };

    // String functors
    BASIC_STRING_FUNCTOR_DECL(ToUpper)
    BASIC_STRING_FUNCTOR_DECL(ToLower)
    BASIC_STRING_FUNCTOR_DECL(Strip)

    // Extern definitions
    extern ToUpper toupper;
    extern ToLower tolower;
    extern Strip strip;
}

#endif // STRING_H
