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


/**
 * Report var type, but treating TYPE_CDOUBLE as a type rather than an
 * array.
 *
 * This avoids an infinite loop where arrays are always broadcasted,
 * and broadcasting calls the original unary operator again.
 *
 * Really should be in a base class, but that entails defining
 * var::dataEnum before class var
 */
var::dataEnum type(var iVar)
{
    var::dataEnum type = iVar.type();
    if ((type == var::TYPE_ARRAY) &&
        (iVar.heap()->type() == var::TYPE_CDOUBLE))
        type = var::TYPE_CDOUBLE;
    return type;
}


void UnaryFunctor::arrayOp(const varheap*, int, int) const
{
    throw std::runtime_error("UnaryFunctor: not an array operation");
};


void BinaryFunctor::arrayOp(const varheap*, const varheap*, int, int) const
{
    throw std::runtime_error("BinaryFunctor: not an array operation");
};


/**
 * Unary broadcaster
 *
 * Broadcasts the operation against iVar.
 */
void UnaryFunctor::broadcast(var iVar, var oVar) const
{
    int iDim = iVar.dim();

    // Case 1: Operation is dimensionless
    // Call back to the unary operator
    if (mDim == 0)
    {
        // This could be parallel!
        for (int i=0; i<iVar.size(); i++)
            oVar.at(i) = operator ()(iVar.at(i));
        return;
    }

    // Case 2: array operation (mDim is at least 1)
    // Check that the array is broadcastable
    if (mDim > iDim)
        throw std::runtime_error("var::broadcast: op dimension too large");

    // If it didn't throw, then the array is broadcastable
    // In this case, loop over iVar with different offsets
    int s = iVar.stride(iDim-mDim);
    std::cout << "Striding: " << s << std::endl;
    for (int i=0; i<iVar.size(); i+=s)
        arrayOp(iVar.heap(), i, iVar.size());
}


/**
 * Binary broadcaster
 *
 * Broadcasts iVar2 against iVar1; i.e., iVar1 is the lvalue and iVar2
 * is the rvalue.
 */
void BinaryFunctor::broadcast(var iVar1, var iVar2, var oVar) const
{
    int dim1 = iVar1.dim();
    int dim2 = iVar2.dim();

    // Case 1: iVar2 has size 1
    // Call back to the unary operator
    if ((dim2 == 1) && (iVar2.size() == 1))
    {
        // This could be parallel!
        for (int i=0; i<iVar1.size(); i++)
            oVar.at(i) = operator ()(iVar1.at(i), iVar2);
        return;
    }

    // Case 2: iVar2 is also an array
    // Check that the two arrays are broadcastable
    if (dim2 > dim1)
        throw std::runtime_error("var::broadcast: input dimension too large");
    if (iVar1.heap()->type() != iVar2.heap()->type())
        throw std::runtime_error("var::broadcast: types must match (for now)");
    for (int i=0; i>dim2; i++)
    {
        // The dimensions should match
        if (iVar1.shape(dim1-i) != iVar2.shape(dim2-i))
            throw std::runtime_error("var::broadcast: dimension mismatch");
    }

    // If it didn't throw, then the arrays are broadcastable
    // In this case, loop over iVar1 with different offsets
    for (int i=0; i<iVar1.size(); i+=iVar2.size())
        arrayOp(iVar1.heap(), iVar2.heap(), i, iVar2.size());
}


var Tan::operator ()(var iVar) const
{
    var r = iVar.copy(true);
    return operator()(iVar, r);
}


var Tan::operator ()(var iVar, var oVar) const
{
    switch (type(iVar))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case var::TYPE_FLOAT:
        oVar = std::tan(iVar.get<float>());
        break;
    case var::TYPE_DOUBLE:
        oVar = std::tan(iVar.get<double>());
        break;
    case var::TYPE_CFLOAT:
        oVar = std::tan(iVar.get<cfloat>());
        break;
    case var::TYPE_CDOUBLE:
        oVar = std::tan(iVar.get<cdouble>());
        break;
    default:
        throw std::runtime_error("Tan::operator(): Unknown type");
    }

    return oVar;
}


var Pow::operator ()(var iVar1, var iVar2) const
{
    var r = iVar1.copy(true);
    return operator()(iVar1, iVar2, r);
}


var Pow::operator ()(var iVar1, var iVar2, var oVar) const
{
    switch(type(iVar1))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case var::TYPE_FLOAT:
        oVar = std::pow(iVar1.get<float>(), iVar2.cast<float>());
        break;
    case var::TYPE_DOUBLE:
        oVar = std::pow(iVar1.get<double>(), iVar2.cast<double>());
        break;
    case var::TYPE_CFLOAT:
        oVar = std::pow(iVar1.get<cfloat>(), iVar2.cast<cfloat>());
        break;
    case var::TYPE_CDOUBLE:
        oVar = std::pow(iVar1.get<cdouble>(), iVar2.cast<cdouble>());
        break;
    default:
        throw std::runtime_error("Pow::operator(): Unknown type");
    }

    return oVar;
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
    if ((iDim == 1) && (iVar.size() == 1) && (type() != TYPE_CDOUBLE))
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
    if (heap()->type() != iVar.heap()->type())
        throw std::runtime_error("var::broadcast: types must match (for now)");
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
        float* x = iHeap->ptr<float>();
        float* y = ptr<float>(iOffset);
        scopy_(&iSize, x, &one, y, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        double* x = iHeap->ptr<double>();
        double* y = ptr<double>(iOffset);
        dcopy_(&iSize, x, &one, y, &one);
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
        float* x = iHeap->ptr<float>();
        float* y = ptr<float>(iOffset);
        saxpy_(&iSize, &alpha, x, &one, y, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = 1.0;
        double* x = iHeap->ptr<double>();
        double* y = ptr<double>(iOffset);
        daxpy_(&iSize, &alpha, x, &one, y, &one);
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
        float* x = iHeap->ptr<float>();
        float* y = ptr<float>(iOffset);
        saxpy_(&iSize, &alpha, x, &one, y, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = -1.0;
        double* x = iHeap->ptr<double>();
        double* y = ptr<double>(iOffset);
        daxpy_(&iSize, &alpha, x, &one, y, &one);
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
        float* A = iHeap->ptr<float>();
        float* x = ptr<float>(iOffset);
        //ssbmv_(&uplo, &iSize, &zero,
        //       &alpha, A, &one, x+iOffset, &one, &beta, x+iOffset, &one);
        stbmv_(&uplo, &trans, &diag, &iSize, &zero, A, &one, x, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        //static double alpha = 1.0;
        //static double beta = 0.0;
        double* A = iHeap->ptr<double>();
        double* x = ptr<double>(iOffset);
        //dsbmv_(&uplo, &iSize, &zero,
        //       &alpha, A, &one, x+iOffset, &one, &beta, x+iOffset, &one);
        dtbmv_(&uplo, &trans, &diag, &iSize, &zero, A, &one, x, &one);
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
        float* x = ptr<float>(iOffset);
        sscal_(&iSize, &alpha, x, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        double alpha = iVar.cast<double>();
        double* x = ptr<double>(iOffset);
        dscal_(&iSize, &alpha, x, &one);
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
        float* a = iHeapB->ptr<float>();
        float* b = iHeapA->ptr<float>(iOffsetA);
        float* c = ptr<float>(iOffset);
        sgemm_(&trans, &trans, m, n, k, &alpha, a, k, b, m, &alpha, c, m);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = 1.0;
        double* a = iHeapB->ptr<double>();
        double* b = iHeapA->ptr<double>(iOffsetA);
        double* c = ptr<double>(iOffset);
        dgemm_(&trans, &trans, m, n, k, &alpha, a, k, b, m, &alpha, c, m);
        break;
    }
    default:
        throw std::runtime_error("varheap::mul: Unknown type");
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
