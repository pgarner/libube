/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, July 2013
 */

#include <cassert>
#include <cstring>
#include <stdexcept>

#include "var.h"

#ifdef VARBOSE
# include <cstdlib>
# define VDEBUG(a) a
#else
# define VDEBUG(a)
#endif

int sizeOf(var::dataEnum iType)
{
    switch (iType)
    {
    case var::TYPE_VAR: return sizeof(var);
    case var::TYPE_CHAR: return sizeof(char);
    case var::TYPE_INT: return sizeof(int);
    case var::TYPE_LONG: return sizeof(long);
    case var::TYPE_FLOAT: return sizeof(float);
    case var::TYPE_DOUBLE: return sizeof(double);
    default:
        throw std::runtime_error("sizeOf(): Unknown type");
    }
}

varheap::varheap()
{
    mData.vp = 0;
    mSize = 0;
    mRefCount = 0;
}

/**
 * Destructor
 *
 * The dtor should be called when the reference count hits zero.  So
 * the refcount should already be zero here.  The memory will have
 * been freed by the dealloc().
 */
varheap::~varheap()
{
    VDEBUG(std::cout << " Dtor" << std::endl);
    if (mRefCount)
        throw std::runtime_error("~varheap: reference count not zero");
}

varheap::varheap(int iSize, var::dataEnum iType)
{
    VDEBUG(std::cout << " Ctor: " << "[" << iSize << "]" << std::endl);
    assert(iSize >= 0);
    mSize = 0;
    mRefCount = 0;
    resize(iSize, iType);
}

varheap::varheap(int iSize, const char* iData)
{
    VDEBUG(std::cout << " Ctor: " << iData << std::endl);
    assert(iSize >= 0);
    mSize = 0;
    mRefCount = 0;
    resize(iSize+1, var::TYPE_CHAR);
    for (int i=0; i<iSize; i++)
        mData.cp[i] = iData[i];
    mData.cp[iSize] = 0;
}


/**
 * Find the next power of two above a given size.
 */
int allocSize(int iSize)
{
    assert(iSize > 0);
    iSize -= 1;
    int size = 1;
    while (iSize > 0)
    {
        iSize >>= 1;
        size <<= 1;
    }
    return size;
}

int varheap::attach()
{
    assert(mRefCount >= 0);
    return ++mRefCount;
}

int varheap::detach(var::dataEnum iType)
{
    assert(mRefCount > 0);
    int count = --mRefCount;
    if (count == 0)
    {
        dealloc(mData, iType);
        delete this;
    }
    return count;
}

void varheap::resize(int iSize, var::dataEnum iType)
{
    // var should translate resize to 0 to a delete
    assert(iSize > 0);
    assert(mSize >= 0);

    // Unallocated
    if (mSize == 0)
    {
        // Allocate
        mSize = allocSize(iSize);
        alloc(mSize, iType);
    }

    // Allocated
    else
    {
        // Re-alloc.
        int newSize = allocSize(iSize);
        if (mSize < newSize)
        {
            dataType old = mData;
            alloc(newSize, iType);
            int toCopy = std::min(mSize, newSize);
            if (iType == var::TYPE_VAR)
                for (int i=0; i<toCopy; i++)
                    mData.vp[i] = old.vp[i];
            else
                std::memcpy(mData.cp, old.cp, sizeOf(iType)*toCopy);
            dealloc(old, iType);
        }
        mSize = newSize;
    }

    return;
}


void varheap::alloc(int iSize, var::dataEnum iType)
{
    assert(iSize >= 0);
    switch (iType)
    {
    case var::TYPE_VAR:
        mData.vp = new var[iSize];
        break;
    case var::TYPE_CHAR:
        mData.cp = new char[iSize];
        break;
    case var::TYPE_INT:
        mData.ip = new int[iSize];
        break;
    case var::TYPE_LONG:
        mData.lp = new long[iSize];
        break;
    case var::TYPE_FLOAT:
        mData.fp = new float[iSize];
        break;
    case var::TYPE_DOUBLE:
        mData.dp = new double[iSize];
        break;
    default:
        throw std::runtime_error("alloc(): Unknown type");
    }
}

void varheap::dealloc(dataType iData, var::dataEnum iType)
{
    switch (iType)
    {
    case var::TYPE_VAR:
        delete [] iData.vp;
        break;
    case var::TYPE_CHAR:
        delete [] iData.cp;
        break;
    case var::TYPE_INT:
        delete [] iData.ip;
        break;
    case var::TYPE_LONG:
        delete [] iData.lp;
        break;
    case var::TYPE_FLOAT:
        delete [] iData.fp;
        break;
    case var::TYPE_DOUBLE:
        delete [] iData.dp;
        break;
    default:
        throw std::runtime_error("dealloc(): Unknown type");
    }
}

long double varheap::strtold()
{
    // Assume we have a char*
    char* endPtr;
    errno = 0;
    long double ld = std::strtold(mData.cp, &endPtr);
    if (endPtr == mData.cp)
        throw std::runtime_error("strtold(): Cannot convert string");
    if (errno)
        throw std::runtime_error("strtold(): errno set");
    return ld;
}
