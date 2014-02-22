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
 * versions.  MKL has mkl.h, OpenBLAS has f77blas.h.  However, given
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
MATH(sqrt)
MATH(cos)
MATH(sin)
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
    void (varheap::*iArrayOp)(const varheap*, int, int)
)
{
    int mDim = dim();
    int iDim = iVar.dim();

    // Case 1: iVar has size 1
    // Call back to the unary operator
    if ((iDim == 1) && (iVar.size() == 1))
    {
        // This could be parallel!
        for (int i=0; i<size(); i++)
            (at(i).*iUnaryOp)(iVar);
        return;
    }

    // Case 2: iVar is also an array
    // Check that the two arrays are broadcastable
    if (!iArrayOp)
        throw std::runtime_error("var::broadcast: not an array operation");
    if (iDim > mDim)
        throw std::runtime_error("var::broadcast: input dimension too large");
    for (int i=0; i>iDim; i++)
    {
        // The dimensions should match
        if (shape(mDim-i) != iVar.shape(iDim-i))
            throw std::runtime_error("var::broadcast: dimension mismatch");
    }

    // If it didn't throw, then the arrays are broadcastable
    // In this case, loop over *this with different offsets
    for (int i=0; i<size(); i+=iVar.size())
        (heap()->*iArrayOp)(iVar.heap(), i, iVar.size());
}


void varheap::set(const varheap* iHeap, int iOffset, int iSize)
{
    static int one = 1;
    switch(type())
    {
    case var::TYPE_FLOAT:
    {
        float* x = &iHeap->ref<float>(0);
        float* y = &ref<float>(0);
        scopy_(&iSize, x, &one, y+iOffset, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        double* x = &iHeap->ref<double>(0);
        double* y = &ref<double>(0);
        dcopy_(&iSize, x, &one, y+iOffset, &one);
        break;
    }
    default:
        throw std::runtime_error("varheap::set: Unknown type");
    }
}


void varheap::add(const varheap* iHeap, int iOffset, int iSize)
{
    static int one = 1;
    switch(type())
    {
    case var::TYPE_FLOAT:
    {
        static float alpha = 1.0f;
        float* x = &iHeap->ref<float>(0);
        float* y = &ref<float>(0);
        saxpy_(&iSize, &alpha, x, &one, y+iOffset, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = 1.0;
        double* x = &iHeap->ref<double>(0);
        double* y = &ref<double>(0);
        daxpy_(&iSize, &alpha, x, &one, y+iOffset, &one);
        break;
    }
    default:
        throw std::runtime_error("varheap::add: Unknown type");
    }
}


void varheap::sub(const varheap* iHeap, int iOffset, int iSize)
{
    static int one = 1;
    switch(type())
    {
    case var::TYPE_FLOAT:
    {
        static float alpha = -1.0f;
        float* x = &iHeap->ref<float>(0);
        float* y = &ref<float>(0);
        saxpy_(&iSize, &alpha, x, &one, y+iOffset, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = -1.0;
        double* x = &iHeap->ref<double>(0);
        double* y = &ref<double>(0);
        daxpy_(&iSize, &alpha, x, &one, y+iOffset, &one);
        break;
    }
    default:
        throw std::runtime_error("varheap::sub: Unknown type");
    }
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
