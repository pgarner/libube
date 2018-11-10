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

#include "lube/var.h"
#include "lube/heap.h"


using namespace libube;

int sizeOf(ind iType)
{
    switch (iType)
    {
    case TYPE_CHAR: return sizeof(char);
    case TYPE_INT: return sizeof(int);
    case TYPE_LONG: return sizeof(long);
    case TYPE_FLOAT: return sizeof(float);
    case TYPE_DOUBLE: return sizeof(double);
    case TYPE_CFLOAT: return sizeof(cfloat);
    case TYPE_CDOUBLE: return sizeof(cdouble);
    case TYPE_VAR: return sizeof(var);
    case TYPE_PAIR: return sizeof(pair);
    default:
        throw error("sizeOf(): Unknown type");
    }
}

/**
 * Default constructor; should all be zero.
 */
Heap::Heap()
{
    mData.vp = 0;
    mSize = 0;
    mCapacity = 0;
    mRefCount = 0;
    mType = TYPE_VAR;
}


/**
 * Destructor
 *
 * The dtor should be called when the reference count hits zero.  So
 * the refcount should already be zero here.  The memory will have
 * been freed by the dealloc().
 */
Heap::~Heap()
{
    assert(!mRefCount);
}


/**
 * Copy constructor
 *
 * Like the var version, it is non-recursive; it copies the array, but
 * deeper arrays are not copied.  Just the reference counts are
 * implicitly bumped.
 */
Heap::Heap(const IHeap& iHeap, bool iAllocOnly) : Heap()
{
    mType = iHeap.type();
    resize(iHeap.size());
    if (!iAllocOnly)
        copy(reinterpret_cast<const Heap*>(&iHeap), mSize);
}


Heap::Heap(int iSize, ind iType) : Heap()
{
    assert(iSize >= 0);
    mType = (iType == TYPE_ARRAY) ? TYPE_VAR : iType;
    resize(iSize);
}

Heap::Heap(int iSize, const char* iData) : Heap()
{
    mType = TYPE_CHAR;
    append(iSize, iData);
}


Heap::Heap(int iSize, const int* iData) : Heap()
{
    mType = TYPE_INT;
    append(iSize, iData);
}


Heap::Heap(int iSize, const cdouble* iData) : Heap()
{
    mType = TYPE_CDOUBLE;
    append(iSize, iData);
}


var* Heap::deref(ind iIndex)
{
    switch (mType)
    {
    case TYPE_VAR:
        return &mData.vp[iIndex];
    case TYPE_PAIR:
        return &mData.pp[iIndex].val;
    }
    return 0;
}


bool Heap::defined(ind iIndex)
{
    var* v = deref(iIndex);
    return v ? v->defined() : true;
}


int Heap::derefSize(ind iIndex)
{
    var* v = deref(iIndex);
    return v ? v->size() : 1;
}


/**
 * var::type() dereferences but does not indirect to the heap, so it can return
 * TYPE_ARRAY but will never return TYPE_VAR.
 */
ind Heap::derefType(ind iIndex)
{
    var* v = deref(iIndex);
    return v ? v->type() : mType;
}

/**
 * var::atype() dereferences and indirects to the heap if it exists, so it will
 * never return TYPE_ARRAY, but can return TYPE_VAR.
 */
ind Heap::derefAType(ind iIndex)
{
    var* v = deref(iIndex);
    return v ? v->atype() : mType;
}


IHeap* Heap::derefHeap(ind iIndex)
{
    var* v = deref(iIndex);
    return v ? v->heap() : 0;
}


#define APPEND(T, P)                                \
    void Heap::append(int iSize, const T* iData)    \
    {                                               \
        int beg = mSize;                            \
        resize(mSize + iSize);                      \
        for (int i=0; i<iSize; i++)                 \
            mData.P[beg+i] = iData[i];              \
    }

APPEND(char, cp)
APPEND(int, ip)
APPEND(cdouble, cdp)


/**
 * Find the next power of two above a given size.
 */
int allocSize(int iSize)
{
    const int minSize = 8;
    assert(iSize >= 0);
    if (iSize < minSize)
        iSize = minSize;
    if (iSize <= 2)
        return iSize;
    iSize -= 1;
    int size = 1;
    while (iSize > 0)
    {
        iSize >>= 1;
        size <<= 1;
    }
    return size;
}

int Heap::attach()
{
    assert(mRefCount >= 0);
    return ++mRefCount;
}

int Heap::detach()
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

void Heap::resize(int iSize)
{
    // It is possible to resize to zero; the capacity stays the same
    assert(mCapacity >= 0);
    mSize = iSize;

    // strings have an extra '\0'
    if (mType == TYPE_CHAR)
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
            if (mType == TYPE_VAR)
                for (int i=0; i<toCopy; i++)
                    mData.vp[i] = old.vp[i];
            else if (mType == TYPE_PAIR)
                for (int i=0; i<toCopy; i++)
                    mData.pp[i] = old.pp[i];
            else
                std::memcpy(mData.cp, old.cp, sizeOf(mType)*toCopy);
            dealloc(old);
            mCapacity = newSize;
        }
    }

    // Put in the null terminator
    if (mType == TYPE_CHAR)
        mData.cp[mSize] = 0;
        
    return;
}


/**
 * Copy the data of an array
 *
 * Copy does not respect views.  Rather, it can be used to copy the
 * view data itself (the array of ints).  The copy constructor calls
 * copy() twice: once for the view and once for the data.
 */
void Heap::copy(const Heap* iHeap, int iSize)
{
    switch(mType)
    {
    case TYPE_CHAR:
        memcpy(mData.cp, iHeap->mData.cp, mSize*sizeof(char));
        break;
    case TYPE_INT:
        memcpy(mData.ip, iHeap->mData.ip, mSize*sizeof(int));
        break;
    case TYPE_LONG:
        memcpy(mData.lp, iHeap->mData.lp, mSize*sizeof(long));
        break;
    case TYPE_FLOAT:
        memcpy(mData.fp, iHeap->mData.fp, mSize*sizeof(float));
        break;
    case TYPE_DOUBLE:
        memcpy(mData.dp, iHeap->mData.dp, mSize*sizeof(double));
        break;
    case TYPE_VAR:
        for (int i=0; i<mSize; i++)
            mData.vp[i] = iHeap->mData.vp[i];
        break;
    case TYPE_PAIR:
        for (int i=0; i<mSize; i++)
            mData.pp[i] = iHeap->mData.pp[i];
        break;
    default:
        throw error("Heap::copy(): Unknown type");
    }
}


void Heap::alloc(int iSize)
{
    assert(iSize >= 0);
    if (iSize == 0)
    {
        mData.cp = 0;
        return;
    }
    switch (mType)
    {
    case TYPE_CHAR:
        mData.cp = new char[iSize];
        break;
    case TYPE_INT:
        mData.ip = new int[iSize];
        break;
    case TYPE_LONG:
        mData.lp = new long[iSize];
        break;
    case TYPE_FLOAT:
        mData.fp = new float[iSize];
        break;
    case TYPE_DOUBLE:
        mData.dp = new double[iSize];
        break;
    case TYPE_CFLOAT:
        mData.cfp = new cfloat[iSize];
        break;
    case TYPE_CDOUBLE:
        mData.cdp = new cdouble[iSize];
        break;
    case TYPE_VAR:
        mData.vp = new var[iSize];
        break;
    case TYPE_PAIR:
        mData.pp = new pair[iSize];
        break;
    default:
        throw error("alloc(): Unknown type");
    }
}

void Heap::dealloc(dataType iData)
{
    if (!iData.cp)
        return;
    switch (mType)
    {
    case TYPE_CHAR:
        delete [] iData.cp;
        break;
    case TYPE_INT:
        delete [] iData.ip;
        break;
    case TYPE_LONG:
        delete [] iData.lp;
        break;
    case TYPE_FLOAT:
        delete [] iData.fp;
        break;
    case TYPE_DOUBLE:
        delete [] iData.dp;
        break;
    case TYPE_CFLOAT:
        delete [] iData.cfp;
        break;
    case TYPE_CDOUBLE:
        delete [] iData.cdp;
        break;
    case TYPE_VAR:
        delete [] iData.vp;
        break;
    case TYPE_PAIR:
        delete [] iData.pp;
        break;
    default:
        throw error("dealloc(): Unknown type");
    }
}

void Heap::format(std::ostream& iStream, int iIndent)
{
    switch (type())
    {
    case TYPE_CHAR:
        iStream << "\"";
        iStream << mData.cp;
        iStream << "\"";
        break;
    case TYPE_PAIR:
        // Might be empty
        iStream << "{";
        for (int i=0; i<mSize; i++)
        {
            iStream << "\n";
            for (int j=0; j<iIndent+2; j++)
                iStream << " ";
            iStream << at(i, true) << ": ";
            at(i).format(iStream, iIndent+2);
            if (i < mSize-1)
                iStream << ",";
        }
        if (mSize)
        {
            iStream << "\n";
            for (int j=0; j<iIndent; j++)
                iStream << " ";
        }
        iStream << "}";
        break;
    case TYPE_VAR:
        iStream << "[\n";
        for (int i=0; i<mSize; i++)
        {
            for (int j=0; j<iIndent+2; j++)
                iStream << " ";
            at(i).format(iStream, iIndent+2);
            if (i < mSize-1)
                iStream << ",";
            iStream << "\n";
        }
        for (int j=0; j<iIndent; j++)
            iStream << " ";
        iStream << "]";
        break;
    case TYPE_CDOUBLE:
        // Don't call at(); it will just create more arrays & loop
        if (size() == 1)
            iStream << *ptrcdouble(0);
        else
        {
            iStream << "[\n";
            for (int i=0; i<mSize; i++)
            {
                for (int j=0; j<iIndent+2; j++)
                    iStream << " ";
                iStream << *ptrcdouble(i);
                if (i < mSize-1)
                    iStream << ",";
                iStream << "\n";
            }
            for (int j=0; j<iIndent; j++)
                iStream << " ";
            iStream << "]";
        }
        break;
    default:
        iStream << "[";
        for (int i=0; i<mSize; i++)
        {
            if (i != 0)
                iStream << ", ";
            iStream << at(i);
        }
        iStream << "]";
    }
}


var Heap::at(int iIndex, bool iKey) const
{
    if ( (iIndex < 0) || (iIndex >= mSize) )
        throw std::range_error("Heap::at(): index out of bounds");

    var r;
    switch (mType)
    {
    case TYPE_CHAR:
        r = mData.cp[iIndex];
        break;
    case TYPE_INT:
        r = mData.ip[iIndex];
        break;
    case TYPE_LONG:
        r = mData.lp[iIndex];
        break;
    case TYPE_FLOAT:
        r = mData.fp[iIndex];
        break;
    case TYPE_DOUBLE:
        r = mData.dp[iIndex];
        break;
    case TYPE_CFLOAT:
        r = mData.cfp[iIndex];
        break;
    case TYPE_CDOUBLE:
        r = mData.cdp[iIndex];
        break;
    case TYPE_VAR:
        r = mData.vp[iIndex];
        break;
    case TYPE_PAIR:
        r = iKey ? mData.pp[iIndex].key : mData.pp[iIndex].val;
        break;
    default:
        throw error("Heap::at(): Unknown type");
    }

    // Done
    return r;
}


var& Heap::key(int iIndex)
{
    if (mType != TYPE_PAIR)
        throw error("Heap::key(): Not a key:value pair");
    if ( (iIndex < 0) || (iIndex >= mSize) )
        throw std::range_error("Heap::at(): index out of bounds");
    return mData.pp[iIndex].key;
}


bool Heap::neq(IHeap* iHeap)
{
    for (int i=0; i<mSize; i++)
        if (at(i) != iHeap->at(i))
            return true;
    return false;
}


bool Heap::lt(IHeap* iHeap)
{
    if ( (mType == TYPE_CHAR) && (iHeap->type() == TYPE_CHAR) )
        return (std::strcmp(ptrchar(), iHeap->ptrchar()) < 0);
    for (int i=0; i<std::min(size(), iHeap->size()); i++)
        if (at(i) < iHeap->at(i))
            return true;
    return false;
}


/**
 * Shift the array contents down (backwards), returning the lowest indexed
 * element that would have fallen off the bottom (front).
 */
var Heap::shift()
{
    // Move syntax is memmove(dest, src, n)
    //
    // var and pair use placement delete on the overwritten element (to delete
    // it) and placement new on the exposed one (to reset it, rather than
    // actually construct a new one).
    var r;
    switch (mType)
    {
    case TYPE_CHAR:
        r = mData.cp[0];
        memmove(mData.cp, mData.cp+1, (mSize-1)*sizeof(char));
        break;
    case TYPE_INT:
        r = mData.ip[0];
        memmove(mData.ip, mData.ip+1, (mSize-1)*sizeof(int));
        break;
    case TYPE_LONG:
        r = mData.lp[0];
        memmove(mData.lp, mData.lp+1, (mSize-1)*sizeof(long));
        break;
    case TYPE_FLOAT:
        r = mData.fp[0];
        memmove(mData.fp, mData.fp+1, (mSize-1)*sizeof(float));
        break;
    case TYPE_DOUBLE:
        r = mData.dp[0];
        memmove(mData.dp, mData.dp+1, (mSize-1)*sizeof(double));
        break;
    case TYPE_CFLOAT:
        r = mData.cfp[0];
        memmove(mData.cfp, mData.cfp+1, (mSize-1)*sizeof(cfloat));
        break;
    case TYPE_CDOUBLE:
        r = mData.cdp[0];
        memmove(mData.cdp, mData.cdp+1, (mSize-1)*sizeof(cdouble));
        break;
    case TYPE_VAR:
        r = mData.vp[0];
        mData.vp[0].~var();
        memmove(mData.vp, mData.vp+1, (mSize-1)*sizeof(var));
        new (&mData.vp[mSize-1]) var();
        break;
    case TYPE_PAIR:
        r = mData.pp[0].val;
        mData.pp[0].~pair();
        memmove(mData.pp, mData.pp+1, (mSize-1)*sizeof(pair));
        new (&mData.pp[mSize-1]) pair();
        break;
    default:
        throw error("Heap::shift(): Unknown type");
    }
    resize(mSize-1);

    // Done
    return r;
}


/**
 * Shift the array contents up (forwards).
 */
void Heap::unshift(var iVar)
{
    resize(mSize+1);
    switch (mType)
    {
    case TYPE_CHAR:
        memmove(mData.cp+1, mData.cp, (mSize-1)*sizeof(char));
        mData.cp[0] = iVar.get<char>();
        break;
    case TYPE_INT:
        memmove(mData.ip+1, mData.ip, (mSize-1)*sizeof(int));
        mData.ip[0] = iVar.get<int>();
        break;
    case TYPE_LONG:
        memmove(mData.lp+1, mData.lp, (mSize-1)*sizeof(long));
        mData.lp[0] = iVar.get<long>();
        break;
    case TYPE_FLOAT:
        memmove(mData.fp+1, mData.fp, (mSize-1)*sizeof(float));
        mData.fp[0] = iVar.get<float>();
        break;
    case TYPE_DOUBLE:
        memmove(mData.dp+1, mData.dp, (mSize-1)*sizeof(double));
        mData.dp[0] = iVar.get<double>();
        break;
    case TYPE_CFLOAT:
        memmove(mData.cfp+1, mData.cfp, (mSize-1)*sizeof(cfloat));
        mData.cfp[0] = iVar.get<cfloat>();
        break;
    case TYPE_CDOUBLE:
        memmove(mData.cdp+1, mData.cdp, (mSize-1)*sizeof(cdouble));
        mData.cdp[0] = iVar.get<cdouble>();
        break;
    case TYPE_VAR:
        memmove(mData.vp+1, mData.vp, (mSize-1)*sizeof(var));
        new (&mData.vp[0]) var();
        mData.vp[0] = iVar;
        break;
    case TYPE_PAIR:
        throw error("Heap::unshift(): Can't unshift a pair");
    default:
        throw error("Heap::unshift(): Unknown type");
    }
}
