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


/*
 * Template specialisations (before any get used)
 *
 * These can be used to get the actual storage.  C++ doesn't allow
 * overloading on return type, so the return type must be specified.
 */
template<> char* varheap::ptr<char>(int iIndex) const {
    return mView ? mView->ptr<char>(iIndex + mData.ip[0]) : mData.cp + iIndex;
}
template<> int* varheap::ptr<int>(int iIndex) const {
    return mView ? mView->ptr<int>(iIndex + mData.ip[0]) : mData.ip + iIndex;
}
template<> long* varheap::ptr<long>(int iIndex) const {
    return mView ? mView->ptr<long>(iIndex + mData.ip[0]) : mData.lp + iIndex;
}
template<> float* varheap::ptr<float>(int iIndex) const {
    return mView ? mView->ptr<float>(iIndex + mData.ip[0]) : mData.fp + iIndex;
}
template<> double* varheap::ptr<double>(int iIndex) const {
    return mView ? mView->ptr<double>(iIndex + mData.ip[0]) : mData.dp + iIndex;
}
template<> cfloat* varheap::ptr<cfloat>(int iIndex) const {
    return mView
        ? mView->ptr<cfloat>(iIndex + mData.ip[0]) : mData.cfp + iIndex;
}
template<> cdouble* varheap::ptr<cdouble>(int iIndex) const {
    return mView
        ? mView->ptr<cdouble>(iIndex + mData.ip[0]) : mData.cdp + iIndex;
}
template<> var* varheap::ptr<var>(int iIndex) const {
    return mView ? mView->ptr<var>(iIndex + mData.ip[0]) : mData.vp + iIndex;
}
template<> pair* varheap::ptr<pair>(int iIndex) const {
    return mView ? mView->ptr<pair>(iIndex + mData.ip[0]) : mData.pp + iIndex;
}


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
    mView = 0;
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
varheap::varheap(const varheap& iHeap, bool iAllocOnly) : varheap()
{
    mType = iHeap.mType;
    resize(iHeap.mSize);
    mView = iHeap.mView ? new varheap(*iHeap.mView, iAllocOnly) : 0;
    if (mView)
        mView->attach();
    if (mView || !iAllocOnly)
        copy(&iHeap, mSize);
}


varheap::varheap(int iSize, var::dataEnum iType) : varheap()
{
    VDEBUG(std::cout << " Ctor(type): " << "[" << iSize << "]" << std::endl);
    assert(iSize >= 0);
    mType = (iType == var::TYPE_ARRAY) ? var::TYPE_VAR : iType;
    resize(iSize);
}

varheap::varheap(int iSize, const char* iData) : varheap()
{
    VDEBUG(std::cout << " Ctor: " << iData << std::endl);
    assert(iSize >= 0);
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


varheap::varheap(int iSize, const cdouble* iData)
    : varheap(iSize, var::TYPE_CDOUBLE)
{
    for (int i=0; i<iSize; i++)
        mData.cdp[i] = iData[i];
}


int varheap::offset(int iOffset)
{
    if (!view())
        throw std::runtime_error("varheap::offset(): not a view");
    if (iOffset + size() > mView->size())
        throw std::runtime_error("varheap::offset(): offset too large");
    mData.ip[0] = iOffset;
    return iOffset;
}


int varheap::shape(int iDim) const
{
    if (mView)
    {
        int index = iDim*2 + 1;
        if ((index < 0) || (index >= mSize))
            throw std::range_error("varheap::shape(): dimension out of bounds");
        return mData.ip[index];
    }
    if (iDim > 0)
        throw std::range_error("varheap::shape(): not view and dimension > 0");
    return size();
}


int varheap::stride(int iDim) const
{
    if (mView)
    {
        int index = iDim*2 + 2;
        if ((index < 0) || (index >= mSize))
            throw std::range_error("varheap::stride(): dimension out of bounds");
        return mData.ip[index];
    }
    if (iDim > 0)
        throw std::range_error("varheap::stride(): not view and dimension > 0");
    return 1;
}


int varheap::size() const
{
    if (mView)
        return stride(0) * shape(0);
    return mSize;
};


/**
 * Normally, x = y will copy the var such that both x and y point to
 * the same varheap.  However, if x is a view, we want the data from y
 * to be copied into the thing of which x is a view.  This checks
 * whether that copy is possible, i.e., the types and sizes match.
 */
bool varheap::copyable(varheap* iHeap)
{
    if (!view())
        return false;
    if (!iHeap)
        return false;
    if (type() != iHeap->type())
        return false;
    if (dim() != iHeap->dim())
        return false;
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


/**
 * Copy the data of an array
 *
 * Copy does not respect views.  Rather, it can be used to copy the
 * view data itself (the array of ints).  The copy constructor calls
 * copy() twice: once for the view and once for the data.
 */
void varheap::copy(const varheap* iHeap, int iSize)
{
    switch(mType)
    {
    case var::TYPE_CHAR:
        memcpy(mData.cp, iHeap->mData.cp, mSize*sizeof(char));
        break;
    case var::TYPE_INT:
        memcpy(mData.ip, iHeap->mData.ip, mSize*sizeof(int));
        break;
    case var::TYPE_LONG:
        memcpy(mData.lp, iHeap->mData.lp, mSize*sizeof(long));
        break;
    case var::TYPE_FLOAT:
        memcpy(mData.fp, iHeap->mData.fp, mSize*sizeof(float));
        break;
    case var::TYPE_DOUBLE:
        memcpy(mData.dp, iHeap->mData.dp, mSize*sizeof(double));
        break;
    case var::TYPE_VAR:
        for (int i=0; i<mSize; i++)
            mData.vp[i] = iHeap->mData.vp[i];
        break;
    case var::TYPE_PAIR:
        for (int i=0; i<mSize; i++)
            mData.pp[i] = iHeap->mData.pp[i];
        break;
    default:
        throw std::runtime_error("varheap::copy(): Unknown type");
    }
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
    case var::TYPE_CFLOAT:
        mData.cfp = new cfloat[iSize];
        break;
    case var::TYPE_CDOUBLE:
        mData.cdp = new cdouble[iSize];
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
    case var::TYPE_CFLOAT:
        delete [] iData.cfp;
        break;
    case var::TYPE_CDOUBLE:
        delete [] iData.cdp;
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


void varheap::format(std::ostream& iStream, int iIndent)
{
    assert(mData.vp); // Any of the pointers

    if (mView)
        return formatView(iStream);

    switch (type())
    {
    case var::TYPE_CHAR:
        iStream << "\"";
        iStream << mData.cp;
        iStream << "\"";
        break;
    case var::TYPE_PAIR:
        iStream << "{\n";
        for (int i=0; i<mSize; i++)
        {
            for (int j=0; j<iIndent+2; j++)
                iStream << " ";
            iStream << "[" << at(i, true) << "] = ";
            at(i).format(iStream, iIndent+2);
            iStream << ";\n";
        }
        for (int j=0; j<iIndent; j++)
            iStream << " ";
        iStream << "}";
        break;
    case var::TYPE_VAR:
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
    case var::TYPE_CDOUBLE:
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


void varheap::formatView(std::ostream& iStream)
{
    assert(mData.vp); // Any of the pointers
    assert(mType == var::TYPE_INT);

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
        iStream << "{";
        for (int j=0; j<nRows; j++)
        {
            if (j != 0)
                iStream << " ";
            for (int i=0; i<nCols; i++)
            {
                iStream << mView->at(k*nRows*nCols + j*nCols + i + offset());
                if ( (i != nCols-1 ) || (j != nRows-1) )
                    iStream << ", ";
            }
            if (j != nRows-1)
                iStream << std::endl;
        }
        iStream << "}";
        if (k != nMats-1)
            iStream << std::endl;
    }
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
    case var::TYPE_CFLOAT:
        r = mData.cfp[iIndex];
        break;
    case var::TYPE_CDOUBLE:
        r = mData.cdp[iIndex];
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


var& varheap::key(int iIndex)
{
    if (mType != var::TYPE_PAIR)
        throw std::runtime_error("varheap::key(): Not a key:value pair");
    if ( (iIndex < 0) || (iIndex >= mSize) )
        throw std::range_error("varheap::at(): index out of bounds");
    return mData.pp[iIndex].key;
}


bool varheap::neq(varheap* iHeap)
{
    for (int i=0; i<mSize; i++)
        if (at(i) != iHeap->at(i))
            return true;
    return false;
}


bool varheap::lt(varheap* iHeap)
{
    if ( (mType == var::TYPE_CHAR) && (iHeap->mType == var::TYPE_CHAR) )
        return (std::strcmp(ptr<char>(), iHeap->ptr<char>()) < 0);
    for (int i=0; i<std::min(size(), iHeap->size()); i++)
        if (at(i) < iHeap->at(i))
            return true;
    return false;
}


void varheap::setView(varheap* iVarHeap)
{
    if (mView)
        mView->detach();
    mView = iVarHeap->mView ? iVarHeap->mView : iVarHeap;
    mView->attach();
}


int& varheap::viewRef(int iIndex)
{
    if (!mView)
        std::runtime_error("varheap::viewRef(): Not a view");
    return mData.ip[iIndex];
}
