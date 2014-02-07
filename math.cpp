/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, July 2013
 */

#include <cassert>
#include <cmath>
#include <cstring>
#include <stdexcept>

#include "var.h"
#include "varheap.h"


/*
 * In some sense it would be best to use cblas.  Netlib defines it,
 * MKL defines it, but OpenBLAS and the like don't necessarily include
 * it.  Sadly, there seems to be no standard header for the fortran
 * versions.  MKL has mkl.h, openblas has f77blas.h.  However, given
 * that the interface is rather standard, we can just reproduce the
 * ones we use here.  These happen to be mods of OpenBLAS's f77blas.h
 */
typedef int blasint;
extern "C" {
    // Actually FORTRAN calling convention
    void   scopy_ (blasint *, float  *, blasint *, float  *, blasint *);
    void   dcopy_ (blasint *, double *, blasint *, double *, blasint *);
    float  sasum_ (blasint *, float  *, blasint *);
    double dasum_ (blasint *, double *, blasint *);
    void   saxpy_ (blasint *, float  *, float  *, blasint *,
                                        float  *, blasint *);
    void   daxpy_ (blasint *, double *, double *, blasint *,
                                        double *, blasint *);
}

#define MATH(func) var var::func() const \
{ \
    var r; \
    switch(mType) \
    { \
    case TYPE_FLOAT: \
        r = std::func(mData.f); \
        break; \
    case TYPE_DOUBLE: \
        r = std::func(mData.d); \
        break; \
    default: \
        throw std::runtime_error("Unknown type"); \
    } \
    return r; \
}

MATH(abs)
MATH(floor)

/**
 * Broadcaster
 *
 * Broadcasts iVar against *this; i.e., *this is the lvalue and iVar
 * is the rvalue.  It should only be called from an operation, and
 * hence *this should be (a reference to) an array.
 */
void var::broadcast(
    var iVar,
    var& (var::*iUnaryOp)(var),
    void (varheap::*iArrayOp)(const varheap*, int)
)
{
    int mDim = dim();
    int iDim = iVar.dim();

    // Case 1: iVar has size 1
    if ((iDim == 1) && (iVar.size() == 1))
    {
        // This could be parallel!
        for (int i=0; i<size(); i++)
            (at(i).*iUnaryOp)(iVar);
        return;
    }

    // Case 2: iVar is also an array
    if (!iArrayOp)
        throw std::runtime_error("var::broadcast: not an array operation");
    if (iDim > mDim)
        throw std::runtime_error("var::broadcast: input dimension too large");
    for (int i=iDim-1; i>=0; i--)
    {
        // The dimensions should match
        if (shape(i) != iVar.shape(i))
            throw std::runtime_error("var::broadcast: dimension mismatch");
    }

    // If it didn't throw, then the arrays are broadcastable
    if (iDim == mDim)
        // Same dimensions
        (heap()->*iArrayOp)(iVar.heap(), iVar.size());
}


/**
 * Copy the data of an array
 *
 * Copy does not respect views.  Rather, it can be used to copy the
 * view data itself (the array of ints).  The copy constructor calls
 * copy() twice: once for the view and once for the data.  BLAS types
 * are accelerated with BLAS Xcopy(); the other built-in types use
 * memcpy().  I don't know whether Xcopy() or memcpy() is faster.
 */
void varheap::copy(const varheap* iHeap, int iSize)
{
    static int one = 1;
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
        scopy_(&iSize, iHeap->mData.fp, &one, mData.fp, &one);
        break;
    case var::TYPE_DOUBLE:
        dcopy_(&iSize, iHeap->mData.dp, &one, mData.dp, &one);
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


void varheap::add(const varheap* iHeap, int iSize)
{
    static int one = 1;
    switch(type())
    {
    case var::TYPE_FLOAT:
    {
        static float alpha = 1.0f;
        float* x = &iHeap->ref<float>(0);
        float* y = &ref<float>(0);
        saxpy_(&iSize, &alpha, x, &one, y, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = 1.0;
        double* x = &iHeap->ref<double>(0);
        double* y = &ref<double>(0);
        daxpy_(&iSize, &alpha, x, &one, y, &one);
        break;
    }
    default:
        throw std::runtime_error("varheap::add: Unknown type");
    }
}


void varheap::sub(const varheap* iHeap, int iSize)
{
    static int one = 1;
    switch(type())
    {
    case var::TYPE_FLOAT:
    {
        static float alpha = -1.0f;
        float* x = &iHeap->ref<float>(0);
        float* y = &ref<float>(0);
        saxpy_(&iSize, &alpha, x, &one, y, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = -1.0;
        double* x = &iHeap->ref<double>(0);
        double* y = &ref<double>(0);
        daxpy_(&iSize, &alpha, x, &one, y, &one);
        break;
    }
    default:
        throw std::runtime_error("varheap::add: Unknown type");
    }
}


var var::sin() const
{
    // C++98 and later overload sin()
    var r;

    switch(mType)
    {
    case TYPE_FLOAT:
        r = std::sin(mData.f);
        break;
    case TYPE_DOUBLE:
        r = std::sin(mData.d);
        break;
    default:
        throw std::runtime_error("Unknown type");
    }

    return r;
}

var var::cos() const
{
    // C++98 and later overload cos()
    var r;

    switch(mType)
    {
    case TYPE_FLOAT:
        r = std::cos(mData.f);
        break;
    case TYPE_DOUBLE:
        r = std::cos(mData.d);
        break;
    default:
        throw std::runtime_error("Unknown type");
    }

    return r;
}

var var::sqrt() const
{
    var r;

    switch(mType)
    {
    case TYPE_FLOAT:
        r = std::sqrt(mData.f);
        break;
    case TYPE_DOUBLE:
        r = std::sqrt(mData.d);
        break;
    default:
        throw std::runtime_error("Unknown type");
    }

    return r;
}


var var::pow(var iPower) const
{
    var r;

    switch(mType)
    {
    case TYPE_ARRAY:
        r = *this;
        if (mData.hp)
            mData.hp->pow(iPower);
        else
            throw std::runtime_error("var::pow(): undefined");
        break;
    case TYPE_FLOAT:
        r = std::pow(mData.f, iPower.cast<float>());
        break;
    case TYPE_DOUBLE:
        r = std::pow(mData.d, iPower.cast<double>());
        break;
    default:
        throw std::runtime_error("Unknown type");
    }

    return r;
}


void varheap::pow(var iPower)
{
    for (int i=0; i<mSize; i++)
        switch (mType)
        {
        case var::TYPE_FLOAT:
            mData.fp[i] = std::pow(mData.fp[i], iPower.cast<float>());
            break;
        case var::TYPE_DOUBLE:
            mData.dp[i] = std::pow(mData.dp[i], iPower.cast<double>());
            break;
        case var::TYPE_VAR:
            mData.vp[i] = mData.vp[i].pow(iPower);
            break;
        default:
            throw std::runtime_error("Unknown type");
        }
}


var var::asum() const
{
    var asum;
    if (type() == TYPE_ARRAY)
        asum = heap()->asum();
    else
        asum = this->abs();
    return asum;
}


var varheap::asum()
{
    var sum;
    int inc = 1;
    switch (mType)
    {
    case var::TYPE_FLOAT:
        sum = sasum_(&mSize, mData.fp, &inc);
        break;
    case var::TYPE_DOUBLE:
        sum = dasum_(&mSize, mData.dp, &inc);
        break;
    default:
        sum = 0L;
        for (int i=0; i<mSize; i++)
            sum += at(i);
    }

    return sum;
}
