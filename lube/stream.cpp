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
 * Initialises a bufferless connection to a var of type char.
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
    mInd = 0;
}


int varbuf::overflow(int iInt)
{
    // Called for every write
    //
    // This appends to the end of the current string whereas stringstream
    // starts at the beginning.  This one feels more intuitive to me.
    if (iInt != traits_type::eof())
        mVar.push(iInt);
    return iInt;
}

int varbuf::uflow()
{
    // Sometimes, e.g., if there is an intermediate buffer, there is no need to
    // implement uflow() as there is a default implementation that calls
    // underflow().  It's used here as the default doesn't know it has to
    // increment mInd.
    //
    // Following stringstream, this doesn't destroy the string
    if (mInd < mVar.size())
        return mVar[mInd++].get<char>();
    return traits_type::eof();
}

int varbuf::underflow()
{
    // Called for reads that do not consume data if not uflow().
    if (mInd < mVar.size())
        return mVar[mInd].get<char>();
    return traits_type::eof();
}

int varbuf::pbackfail(int iInt)
{
    // istreams are supposed to allow putting back one character.  Putting back
    // will always fail since it's bufferless, in which case this is called.
    if (mInd > 0)
        return mVar[--mInd].get<char>();
    return traits_type::eof();
}

std::streampos varbuf::seekpos(
    std::streampos iPos, std::ios_base::openmode iMode
)
{
    // Ignore mode for the moment
    if (iPos < mVar.size())
        mInd = iPos;
    return iPos;
}

varstream::varstream(class var iVar)
    : std::iostream(&mVarBuf), mVarBuf(iVar)
{
    // Tell the base class to use the buffer in the derived class.  Actually,
    // just passing the buffer to the constructor as above ought to work; I
    // either didn't understand it or had problems before.
    //rdbuf(&mVarBuf);
}
