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
#include "varheap.h"

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
    case var::TYPE_CHAR: return sizeof(char);
    case var::TYPE_INT: return sizeof(int);
    case var::TYPE_LONG: return sizeof(long);
    case var::TYPE_FLOAT: return sizeof(float);
    case var::TYPE_DOUBLE: return sizeof(double);
    case var::TYPE_VAR: return sizeof(var);
    case var::TYPE_PAIR: return sizeof(pair);
    default:
        throw std::runtime_error("sizeOf(): Unknown type");
    }
}

/**
 * Default constructor; should all be zero.
 */
varheap::varheap()
{
    mData.vp = 0;
    mSize = 0;
    mCapacity = 0;
    mRefCount = 0;
    mType = var::TYPE_VAR;
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
    VDEBUG(std::cout << " Ctor(type): " << "[" << iSize << "]" << std::endl);
    assert(iSize >= 0);
    mSize = 0;
    mCapacity = 0;
    mRefCount = 0;
    mType = (iType == var::TYPE_ARRAY) ? var::TYPE_VAR : iType;
    resize(iSize);
}

varheap::varheap(int iSize, const char* iData)
{
    VDEBUG(std::cout << " Ctor: " << iData << std::endl);
    assert(iSize >= 0);
    mSize = 0;
    mCapacity = 0;
    mRefCount = 0;
    mType = var::TYPE_CHAR;
    resize(iSize);
    for (int i=0; i<iSize; i++)
        mData.cp[i] = iData[i];
    mData.cp[iSize] = 0;
}


varheap::varheap(int iSize, const int* iData)
    : varheap(iSize, var::TYPE_INT)
{
    VDEBUG(std::cout << " Ctor(int*): " << iData << std::endl);
    for (int i=0; i<iSize; i++)
        mData.ip[i] = iData[i];
}


/**
 * Find the next power of two above a given size.
 */
int allocSize(int iSize)
{
    assert(iSize >= 0);
    if (iSize == 0)
        // Always alloc something
        return 1;
    iSize -= 1;
    int size = 1;
    while (iSize > 0)
    {
        iSize >>= 1;
        size <<= 1;
    }
    return size;
}

int varheap::size()
{
    return mSize;
}

int varheap::attach()
{
    assert(mRefCount >= 0);
    return ++mRefCount;
}

int varheap::detach()
{
    assert(mRefCount > 0);
    int count = --mRefCount;
    if (count == 0)
    {
        dealloc(mData);
        delete this;
    }
    return count;
}

void varheap::resize(int iSize)
{
    // It is possible to resize to zero; the capacity stays the same
    assert(mCapacity >= 0);
    mSize = iSize;

    // strings have an extra '\0'
    if (mType == var::TYPE_CHAR)
        iSize += 1;

    // Unallocated
    if (mCapacity == 0)
    {
        // Allocate
        mCapacity = allocSize(iSize);
        alloc(mCapacity);
    }

    // Allocated
    else
    {
        // Re-alloc.
        int newSize = allocSize(iSize);
        if (mCapacity < newSize)
        {
            dataType old = mData;
            alloc(newSize);
            int toCopy = std::min(mCapacity, newSize);
            if (mType == var::TYPE_VAR)
                for (int i=0; i<toCopy; i++)
                    mData.vp[i] = old.vp[i];
            else if (mType == var::TYPE_PAIR)
                for (int i=0; i<toCopy; i++)
                    mData.pp[i] = old.pp[i];
            else
                std::memcpy(mData.cp, old.cp, sizeOf(mType)*toCopy);
            dealloc(old);
        }
        mCapacity = newSize;
    }

    // Put in the null terminator
    if (mType == var::TYPE_CHAR)
        mData.cp[mSize] = 0;
        
    return;
}


void varheap::alloc(int iSize)
{
    assert(iSize >= 0);
    switch (mType)
    {
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
    case var::TYPE_VAR:
        mData.vp = new var[iSize];
        break;
    case var::TYPE_PAIR:
        mData.pp = new pair[iSize];
        break;
    default:
        throw std::runtime_error("alloc(): Unknown type");
    }
}

void varheap::dealloc(dataType iData)
{
    switch (mType)
    {
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
    case var::TYPE_VAR:
        delete [] iData.vp;
        break;
    case var::TYPE_PAIR:
        delete [] iData.pp;
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

var varheap::at(int iIndex, bool iKey) const
{
    if ( (iIndex < 0) || (iIndex >= mSize) )
        throw std::range_error("varheap::at(): index out of bounds");
    
    var r;
    switch (mType)
    {
    case var::TYPE_CHAR:
        r = mData.cp[iIndex];
        break;
    case var::TYPE_INT:
        r = mData.ip[iIndex];
        break;
    case var::TYPE_LONG:
        r = mData.lp[iIndex];
        break;
    case var::TYPE_FLOAT:
        r = mData.fp[iIndex];
        break;
    case var::TYPE_DOUBLE:
        r = mData.dp[iIndex];
        break;
    case var::TYPE_VAR:
        r = mData.vp[iIndex];
        break;
    case var::TYPE_PAIR:
        r = iKey ? mData.pp[iIndex].key : mData.pp[iIndex].val;
        break;
    default:
        throw std::runtime_error("varheap::at(): Unknown type");
    }

    // Done
    return r;
}

void varheap::set(var iVar, int iIndex, bool iKey)
{
    int lo = (iIndex < 0) ? 0 : iIndex;
    int hi = (iIndex < 0) ? mSize : iIndex+1;
    for (int i=lo; i<hi; i++)
        switch (mType)
        {
        case var::TYPE_CHAR:
            mData.cp[i] = iVar.cast<char>();
            break;
        case var::TYPE_INT:
            mData.ip[i] = iVar.cast<int>();
            break;
        case var::TYPE_LONG:
            mData.lp[i] = iVar.cast<long>();
            break;
        case var::TYPE_FLOAT:
            mData.fp[i] = iVar.cast<float>();
            break;
        case var::TYPE_DOUBLE:
            mData.dp[i] = iVar.cast<double>();
            break;
        case var::TYPE_VAR:
            mData.vp[i] = iVar;
            break;
        case var::TYPE_PAIR:
            if (iKey)
                mData.pp[i].key = iVar;
            else
                mData.pp[i].val = iVar;
            break;
        default:
            throw std::runtime_error("varheap::set(): Unknown type");
        }
}

void varheap::add(var iVar, int iIndex)
{
    int lo = (iIndex < 0) ? 0 : iIndex;
    int hi = (iIndex < 0) ? mSize : iIndex+1;
    for (int i=lo; i<hi; i++)
        switch (mType)
        {
        case var::TYPE_CHAR:
            mData.cp[i] += iVar.cast<char>();
            break;
        case var::TYPE_INT:
            mData.ip[i] += iVar.cast<int>();
            break;
        case var::TYPE_LONG:
            mData.lp[i] += iVar.cast<long>();
            break;
        case var::TYPE_FLOAT:
            mData.fp[i] += iVar.cast<float>();
            break;
        case var::TYPE_DOUBLE:
            mData.dp[i] += iVar.cast<double>();
            break;
        case var::TYPE_VAR:
            mData.vp[i] += iVar;
            break;
        default:
            throw std::runtime_error("varheap::add(): Unknown type");
        }
}

void varheap::sub(var iVar, int iIndex)
{
    int lo = (iIndex < 0) ? 0 : iIndex;
    int hi = (iIndex < 0) ? mSize : iIndex+1;
    for (int i=lo; i<hi; i++)
        switch (mType)
        {
        case var::TYPE_CHAR:
            mData.cp[i] -= iVar.cast<char>();
            break;
        case var::TYPE_INT:
            mData.ip[i] -= iVar.cast<int>();
            break;
        case var::TYPE_LONG:
            mData.lp[i] -= iVar.cast<long>();
            break;
        case var::TYPE_FLOAT:
            mData.fp[i] -= iVar.cast<float>();
            break;
        case var::TYPE_DOUBLE:
            mData.dp[i] -= iVar.cast<double>();
            break;
        case var::TYPE_VAR:
            mData.vp[i] -= iVar;
            break;
        default:
            throw std::runtime_error("varheap::add(): Unknown type");
        }
}
