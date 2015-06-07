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

#ifdef VARBOSE
# include <cstdlib>
# define VDEBUG(a) a
#else
# define VDEBUG(a)
#endif


using namespace libube;


/*
 * Allow data access to be templated
 */
template<> char* Heap::data<char>() const { return mData.cp; };
template<> int* Heap::data<int>() const { return mData.ip; };
template<> long* Heap::data<long>() const { return mData.lp; };
template<> float* Heap::data<float>() const { return mData.fp; };
template<> double* Heap::data<double>() const { return mData.dp; };
template<> cfloat* Heap::data<cfloat>() const { return mData.cfp; };
template<> cdouble* Heap::data<cdouble>() const { return mData.cdp; };
template<> var* Heap::data<var>() const { return mData.vp; };
template<> pair* Heap::data<pair>() const { return mData.pp; };


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
    mView = 0;
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
    VDEBUG(std::cout << " Dtor" << std::endl);
    if (mRefCount)
        throw error("~Heap: reference count not zero");
    if (mView)
        mView->detach();
}


/**
 * Copy constructor
 *
 * Like the var version, it is non-recursive; it copies the array, but
 * deeper arrays are not copied.  Just the reference counts are
 * implicitly bumped.  However, if it's a view then the heap of which
 * it's a view is copied.
 */
Heap::Heap(const Heap& iHeap, bool iAllocOnly) : Heap()
{
    mType = iHeap.mType;
    resize(iHeap.mSize);
    mView = iHeap.mView ? new Heap(*iHeap.mView, iAllocOnly) : 0;
    if (mView)
        mView->attach();
    if (mView || !iAllocOnly)
        copy(&iHeap, mSize);
}


Heap::Heap(int iSize, ind iType) : Heap()
{
    VDEBUG(std::cout << " Ctor(type): " << "[" << iSize << "]" << std::endl);
    assert(iSize >= 0);
    mType = (iType == TYPE_ARRAY) ? TYPE_VAR : iType;
    resize(iSize);
}

Heap::Heap(int iSize, const char* iData) : Heap()
{
    VDEBUG(std::cout << " Ctor: " << iData << std::endl);
    assert(iSize >= 0);
    mType = TYPE_CHAR;
    resize(iSize);
    for (int i=0; i<iSize; i++)
        mData.cp[i] = iData[i];
}


Heap::Heap(int iSize, const int* iData)
    : Heap(iSize, TYPE_INT)
{
    VDEBUG(std::cout << " Ctor(int*): " << iData << std::endl);
    for (int i=0; i<iSize; i++)
        mData.ip[i] = iData[i];
}


Heap::Heap(int iSize, const cdouble* iData)
    : Heap(iSize, TYPE_CDOUBLE)
{
    for (int i=0; i<iSize; i++)
        mData.cdp[i] = iData[i];
}


int Heap::offset(int iOffset)
{
    if (!view())
        throw error("Heap::offset(): not a view");
    if (iOffset + size() > mView->size())
        throw error("Heap::offset(): offset too large");
    mData.ip[0] = iOffset;
    return iOffset;
}


int& Heap::shape(int iDim) const
{
    if (!view())
        throw error("Heap::shape(): not a view");
    int index = iDim*2 + 1;
    if ((index < 0) || (index >= mSize))
        throw std::range_error("Heap::shape(): dimension out of bounds");
    return mData.ip[index];
}


int& Heap::stride(int iDim) const
{
    if (!view())
        throw error("Heap::stride(): not a view");
    int index = iDim*2 + 2;
    if ((index < 0) || (index >= mSize))
        throw std::range_error("Heap::stride(): dimension out of bounds");
    return mData.ip[index];
}


int Heap::size() const
{
    if (mView)
        return stride(0) * shape(0);
    return mSize;
};


/**
 * Normally, x = y will copy the var such that both x and y point to
 * the same Heap.  However, if x is a view, we want the data from y
 * to be copied into the thing of which x is a view.  This checks
 * whether that copy is possible, i.e., the types and sizes match.
 */
bool Heap::copyable(Heap* iHeap)
{
    if (!view())
        return false;
    if (!iHeap)
        return false;
    if (type() != iHeap->type())
        return false;
    if (dim() != iHeap->dim())
        return false;
    if (dim() == 1)
        return (size() == iHeap->size());
    for (int i=0; i<dim(); i++)
        if (shape(i) != iHeap->shape(i))
            return false;
    return true;
}


/**
 * Find the next power of two above a given size.
 */
int allocSize(int iSize)
{
    assert(iSize >= 0);
    if (iSize < 3)
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
    if (mView)
        return formatView(iStream, iIndent);

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
            iStream << "[" << at(i, true) << "] = ";
            at(i).format(iStream, iIndent+2);
            iStream << ";";
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
        iStream << "{\n";
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
        iStream << "}";
        break;
    case TYPE_CDOUBLE:
        // Don't call at(); it will just create more arrays & loop
        if (size() == 1)
            iStream << *ptr<cdouble>(0);
        else
        {
            iStream << "{\n";
            for (int i=0; i<mSize; i++)
            {
                for (int j=0; j<iIndent+2; j++)
                    iStream << " ";
                iStream << *ptr<cdouble>(i);
                if (i < mSize-1)
                    iStream << ",";
                iStream << "\n";
            }
            for (int j=0; j<iIndent; j++)
                iStream << " ";
            iStream << "}";
        }
        break;
    default:
        iStream << "{";
        for (int i=0; i<mSize; i++)
        {
            if (i != 0)
                iStream << ", ";
            iStream << at(i);
        }
        iStream << "}";
    }
}


void Heap::formatView(std::ostream& iStream, int iIndent)
{
    assert(mData.vp); // Any of the pointers
    assert(mType == TYPE_INT);

    // Output shape if it's more than a matrix
    int nDim = dim();
    if (nDim > 2)
    {
        for (int i=0; i<nDim; i++)
        {
            iStream << shape(i);
            if (i != nDim-1)
                iStream << "x";
        }
        iStream << " tensor:" << std::endl;
    }

    // If it's less than 2D, just print it as with format
    if (nDim < 2)
    {
        iStream << "{";
        for (int i=0; i<size(); i++)
        {
            iStream << mView->at(i + offset());
            if (i != size()-1 )
                iStream << ", ";
        }
        iStream << "}";
        return;
    }

    // Calculate how many matrices we have
    int nMats = 1;
    for (int i=0; i<nDim-2; i++)
        nMats *= shape(i);

    // Format as a sequence of matrices
    int nRows = shape(nDim-2);
    int nCols = shape(nDim-1);
    for (int k=0; k<nMats; k++)
    {
        iStream << "{\n";
        for (int j=0; j<nRows; j++)
        {
            for (int k=0; k<iIndent+2; k++)
                iStream << " ";
            for (int i=0; i<nCols; i++)
            {
                iStream << mView->at(k*nRows*nCols + j*nCols + i + offset());
                if ( (j != nRows-1) || (i != nCols-1) )
                    iStream << ",";
                if (i != nCols-1)
                    iStream << " ";
            }
            iStream << "\n";
        }
        for (int j=0; j<iIndent; j++)
            iStream << " ";
        iStream << "}";
        if (k != nMats-1)
            iStream << std::endl;
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


bool Heap::neq(Heap* iHeap)
{
    for (int i=0; i<mSize; i++)
        if (at(i) != iHeap->at(i))
            return true;
    return false;
}


bool Heap::lt(Heap* iHeap)
{
    if ( (mType == TYPE_CHAR) && (iHeap->mType == TYPE_CHAR) )
        return (std::strcmp(ptr<char>(), iHeap->ptr<char>()) < 0);
    for (int i=0; i<std::min(size(), iHeap->size()); i++)
        if (at(i) < iHeap->at(i))
            return true;
    return false;
}


void Heap::setView(Heap* iHeap)
{
    if (mView)
        mView->detach();
    mView = iHeap->mView ? iHeap->mView : iHeap;
    mView->attach();
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
