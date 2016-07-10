/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, July 2016
 */

#include "var.h"

using namespace libube;

/**
 * Basic constructor
 *
 * Initialises a bufferless connection to a var of type char.  Bufferless means
 * that you can't put things back into the input stream; putback() and unget()
 * will not work, and will fail silently unless 'badbit' is checked explicitly.
 */
varbuf::varbuf(class var iVar)
{
    // Type char; ensure it's on the heap
    if (iVar)
        mVar = iVar.array();
    else
    {
        mVar = "";
        mVar.array();
    }

    // Set the pointers meaning the buffer has zero size.  This will cause the
    // iostream machinery to call overflow() on every write and underflow() or
    // uflow() on every read
    setp(0, 0);
    setg(0, 0, 0);
}


varbuf::int_type varbuf::overflow(int_type iInt)
{
    // Called for every write
    if (iInt != traits_type::eof())
        mVar.push(iInt);
    return iInt;
}

varbuf::int_type varbuf::uflow()
{
    // If there were an intermediate buffer, there would be no need to
    // implement uflow() as the default implementation calls underflow().  It
    // works here as it maps to a specific var method.
    if (mVar.size() > 0)
        return mVar.shift().get<char>();
    return traits_type::eof();
}

varbuf::int_type varbuf::underflow()
{
    // Called for reads, if not uflow().
    if (mVar.size() > 0)
        return mVar[0].get<char>();
    return traits_type::eof();
}

varstream::varstream(class var iVar)
    : std::iostream(&mVarBuf), mVarBuf(iVar)
{
    // Tell the base class to use the buffer in the derived class.  Actually,
    // just passing the buffer to the constructor as above ought to work; I
    // either didn't understand it or had problems before.
    //rdbuf(&mVarBuf);
}
