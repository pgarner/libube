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
 * ones we use here.
 */
extern "C" {
    // Actually FORTRAN calling convention
    void   scopy_ (int *, float  *, int *, float  *, int *);
    void   dcopy_ (int *, double *, int *, double *, int *);
    float  sasum_ (int *, float  *, int *);
    double dasum_ (int *, double *, int *);
    void   saxpy_ (int *, float  *, float  *, int *, float  *, int *);
    void   daxpy_ (int *, double *, double *, int *, double *, int *);
    void   sscal_ (int *,  float  *, float  *, int *);
    void   dscal_ (int *,  double *, double *, int *);
    void   stbmv_ (
        char *, char *, char *, int *, int *, float  *, int *, float  *, int *
    );
    void   dtbmv_ (
        char *, char *, char *, int *, int *, double *, int *, double *, int *
    );
    //void   ssbmv_ (
    //    char *, int *, int *, float  *, float  *, int *,
    //    float  *, int *, float  *, float  *, int *
    //);
    //void   dsbmv_ (
    //    char *, int *, int *, double *, double *, int *,
    //    double *, int *, double *, double *, int *
    //);
    void   sgemm_ (
        char *, char *, int *, int *, int *, float *,
        float  *, int *, float  *, int *, float  *, float  *, int *
    );
    void   dgemm_ (
        char *, char *, int *, int *, int *, double *,
        double *, int *, double *, int *, double *, double *, int *
    );
}


/**
 * Operations
 *
 * There are three different cases:
 *
 * 1. We want the operation to happen in place, e.g., x += 1
 *
 * 2. We want the operation to allocate new storage, e.g., y = x + 1
 *
 * 3. We want the result in existing storage, e.g., y[0] = x + 1 This
 * would also happen in case 2 if y were a view.
 *
 * Going into existing storage via operator=() can be wasteful if it's
 * a view, i.e., there is a temporary.  The solution seems to be to
 * define all vector operations to take the target storage as an
 * argument.  If it's not defined we have case 1, if it's defined
 * ahead we have case 2, and in general we have case 3.  All are
 * methods, but case 2 can be a (static) function.
 *
 * That said, for some BLAS operations, the natural operation is to
 * overwrite.  In this case, the BLAS wrapper can allocate or not.
 */

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
        // Scaling is a special case
        if (iUnaryOp == (&var::operator *=))
            heap()->scal(size(), 0, iVar);
        else
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


void varheap::mul(const varheap* iHeap, int iOffset, int iSize)
{
    // Elementwise multiplication is actually multiplication by a
    // diagonal matrix.  In BLAS speak, a diagonal matrix is a band
    // matrix with no superdiagonals.  In this sense, we want xsbmv()
    // (symmetric band), but that puts the result in a new location.
    // Rather, xtbmv() (triangular band) overwrites the current
    // location.
    static int zero = 0;
    static int one = 1;
    static char uplo = 'U';
    static char trans = 'T';
    static char diag = 'N';
    switch(type())
    {
    case var::TYPE_FLOAT:
    {
        //static float alpha = 1.0f;
        //static float beta = 0.0f;
        float* A = &iHeap->ref<float>(0);
        float* x = &ref<float>(0);
        //ssbmv_(&uplo, &iSize, &zero,
        //       &alpha, A, &one, x+iOffset, &one, &beta, x+iOffset, &one);
        stbmv_(&uplo, &trans, &diag, &iSize, &zero, A, &one, x+iOffset, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        //static double alpha = 1.0;
        //static double beta = 0.0;
        double* A = &iHeap->ref<double>(0);
        double* x = &ref<double>(0);
        //dsbmv_(&uplo, &iSize, &zero,
        //       &alpha, A, &one, x+iOffset, &one, &beta, x+iOffset, &one);
        dtbmv_(&uplo, &trans, &diag, &iSize, &zero, A, &one, x+iOffset, &one);
        break;
    }
    default:
        throw std::runtime_error("varheap::mul: Unknown type");
    }
}


void varheap::scal(int iSize, int iOffset, var iVar)
{
    static int one = 1;
    switch(type())
    {
    case var::TYPE_FLOAT:
    {
        float alpha = iVar.cast<float>();
        float* x = &ref<float>(0);
        sscal_(&iSize, &alpha, x+iOffset, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        double alpha = iVar.cast<double>();
        double* x = &ref<double>(0);
        dscal_(&iSize, &alpha, x+iOffset, &one);
        break;
    }
    default:
        throw std::runtime_error("varheap::scal: Unknown type");
    }
}


void varheap::mul(
    int iM, int iN, int iK, int iOffset,
    const varheap* iHeapA, int iOffsetA, varheap* iHeapB
)
{
    // Swap B and A as they're transposed in FORTRAN world
    static char trans = 'T';
    int* m = &iN;
    int* n = &iM;
    int* k = &iK;
    switch(type())
    {
    case var::TYPE_FLOAT:
    {
        static float alpha = 1.0f;
        float* a = &iHeapB->ref<float>(0);
        float* b = &iHeapA->ref<float>(0) + iOffsetA;
        float* c = &ref<float>(0) + iOffset;
        sgemm_(&trans, &trans, m, n, k, &alpha, a, k, b, m, &alpha, c, m);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = 1.0;
        double* a = &iHeapB->ref<double>(0);
        double* b = &iHeapA->ref<double>(0) + iOffsetA;
        double* c = &ref<double>(0) + iOffset;
        dgemm_(&trans, &trans, m, n, k, &alpha, a, k, b, m, &alpha, c, m);
        break;
    }
    default:
        throw std::runtime_error("varheap::mul: Unknown type");
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
