/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2014
 */

#ifndef IND_H
#define IND_H

#include <cstdint>

namespace libvar
{
    /**
     * Index type
     *
     * Basically an integer type the same size as a pointer.  However, when
     * evaluated in a boolean context, it returns false if the value is
     * negative and true otherwise.  This is in contrast to a normal C integer
     * type that returns false if the value is 0 and true otherwise.  Hence, it
     * functions as a postive integer.  The default (undefined) value is zero,
     * hence true.
     *
     * In addition, negating via operator~() returns a negative version of the
     * value -(x+1).  The operation is self-inverting; this allows any integer
     * within range to be stored as a "negative" or "false" version of itself.
     *
     * All other defined operators do the same as would be expected for an
     * integer type.
     */
    class ind {
    public:
        ind() { mInt = 0; };
        ind(intptr_t iInt) { mInt = iInt; };
        static bool size() { return sizeof(ind) == sizeof(std::size_t); };
        explicit operator bool() const { return mInt < 0 ? false : true; };
        operator intptr_t() const { return mInt; };
        ind  operator ++(int iInt) { ind t = *this; ++mInt; return t; };
        ind  operator --(int iInt) { ind t = *this; --mInt; return t; };
        ind& operator ++() { ++mInt; return *this; };
        ind& operator --() { --mInt; return *this; };
        ind  operator  ~() { return -(mInt+1); };
    private:
        intptr_t mInt;
    };
}

#endif // IND_H
