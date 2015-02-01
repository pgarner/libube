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

// Basically BLAS is cblas.h, but the MKL version mkl.h
#ifdef HAVE_MKL
# include <mkl.h>
#else
# include <cblas.h>
#endif

#include "var.h"


namespace libvar
{
    /*
     * The instantiations of the math and standard functors defined in this
     * module.  They are declared extern in var.h
     */
    Abs abs;
    Norm norm;
    NormC normc;
    Sin sin;
    Cos cos;
    Tan tan;
    Floor floor;
    Sqrt sqrt;
    Log log;
    Exp exp;

    Pow pow;
    Set set;
    Add add;
    Sub sub;
    Mul mul;
    Dot dot;
    Div div;
    ASum asum;
    Sum sum;
    IAMax iamax;
}


using namespace libvar;


/**
 * Report var type, but treating TYPE_CDOUBLE as a type rather than an array.
 *
 * This avoids an infinite loop where arrays are always broadcasted, and
 * broadcasting calls the original unary operator again.
 */
ind type(var iVar)
{
    ind type = iVar.type();
    if (iVar.atype<cdouble>())
        type = TYPE_CDOUBLE;
    return type;
}


/**
 * The default allocator is simply to make a copy of the input variable with
 * the same type and shape.  Only the allocation is done; data is not copied.
 */
var Functor::alloc(var iVar) const
{
    var r;
    r = iVar.copy(true);
    return r;
}


/**
 * The normal operation of operator(iVar) is to allocate space then call the
 * protected method scalar().
 */
var UnaryFunctor::operator ()(const var& iVar) const
{
    var v = alloc(iVar);
    scalar(iVar, v);
    return v;
}


/**
 * The normal operation of operator(iVar, oVar) is to just call the protected
 * method scalar().
 */
var UnaryFunctor::operator ()(const var& iVar, var& oVar) const
{
    scalar(iVar, oVar);
    return oVar;
}


/**
 * The default behaviour of scalar(), hence operator(), is to broadcast.
 */
void UnaryFunctor::scalar(const var& iVar, var& oVar) const
{
    broadcast(iVar, oVar);
}


/**
 * This vector() is called by broadcast() without allocating any memory.
 * Overriding it is the most efficient way to implement vector operations.
 *
 * This default implementation encodes the offsets into views and calls the
 * vector() form without offsets.  This allocates memory, but means that the
 * implementation can be more var-like.
 */
void UnaryFunctor::vector(var iVar, ind iIOffset, var& oVar, ind iOOffset) const
{
    int dimI = iVar.dim();
    int dimO = oVar.dim();
    int cdim = dimI - mDim;
    var iv = iVar.subview(dimI-cdim, iIOffset);
    var ov = oVar.subview(dimO-cdim, iOOffset);
    vector(iv, ov);
}


/**
 * The default vector() throws, meaning that it should have been overridden or
 * implemented by scalar() or vector() with offsets.
 */
void UnaryFunctor::vector(var iVar, var& oVar) const
{
    throw std::runtime_error("UnaryFunctor: not a vector operation");
}


/**
 * Unary broadcaster
 *
 * Broadcasts the operation against iVar.  The unary broadcast is obvious
 * because there's only one semantic: repeat the operation over the given
 * (sub-) dimension.  The functor dimension is indicated at construction time.
 */
void UnaryFunctor::broadcast(var iVar, var& oVar) const
{
    int dimI = iVar.dim();

    // Case 1: Operation is dimensionless
    // Call back to the unary operator
    if (mDim == 0)
    {
        // This could be parallel!
        for (int i=0; i<iVar.size(); i++)
        {
            var ref = oVar.at(i);
            scalar(iVar.at(i), ref);
        }
        return;
    }

    // Case 2: array operation (mDim is at least 1)
    // Check that the array is broadcastable
    if (mDim > dimI)
        throw std::runtime_error("var::broadcast: op dimension too large");

    // If it didn't throw, then the array is broadcastable
    // In this case, loop over iVar (and oVar) with different offsets
    int dimO = oVar.dim();
    int stepI = dimI-mDim > 0 ? iVar.stride(dimI-mDim-1) : iVar.size();
    int stepO = dimO-mDim > 0 ? oVar.stride(dimO-mDim-1) : oVar.size();
    int nOps = iVar.size() / stepI;
    for (int i=0; i<nOps; i++)
        vector(iVar, stepI*i, oVar, stepO*i);
}


/**
 * The normal operation of operator(iVar1, iVar2) is to allocate space then
 * call the protected method scalar().
 */
var ArithmeticFunctor::operator ()(
    const var& iVar1, const var& iVar2
) const
{
    var v = alloc(iVar1);
    scalar(iVar1, iVar2, v);
    return v;
}


/**
 * The normal operation of operator(iVar1, iVar2, oVar) is to just call the
 * protected method scalar().
 */
var ArithmeticFunctor::operator ()(
    const var& iVar1, const var& iVar2, var& oVar
) const
{
    scalar(iVar1, iVar2, oVar);
    return oVar;
}


/**
 * The default behaviour of scalar(), hence operator(), is to broadcast.
 */
void ArithmeticFunctor::scalar(
    const var& iVar1, const var& iVar2, var& oVar
) const
{
    broadcast(iVar1, iVar2, oVar);
}


/**
 * This vector() is called by broadcast() without allocating any memory.
 * Overriding it is the most efficient way to implement vector operations.
 *
 * This default implementation encodes the offsets into views and calls the
 * vector() form without offsets.  This allocates memory, but means that the
 * implementation can be more var-like.
 */
void ArithmeticFunctor::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    int dim1 = iVar1.dim();
    int dim2 = iVar2.dim();
    int dimO =  oVar.dim();
    int cdim = dim1 - mDim;
    var iv1 = iVar1.subview(dim1-cdim, iOffset1);
    var iv2 = iVar2.subview(dim2-cdim, iOffset2);
    var ov  =  oVar.subview(dimO-cdim, iOffsetO);
    vector(iv1, iv2, ov);
}


/**
 * The default vector() throws, meaning that it should have been overridden or
 * implemented by scalar() or vector() with offsets.
 */
void ArithmeticFunctor::vector(var iVar1, var iVar2, var& oVar) const
{
    throw std::runtime_error("ArithmeticFunctor: not a vector operation");
}


/**
 * Arithmetic broadcaster
 *
 * Broadcasts iVar2 against iVar1; i.e., iVar1 is the lvalue and iVar2 is the
 * rvalue.  The dimension of the operation is the dimension of iVar2.
 */
void ArithmeticFunctor::broadcast(var iVar1, var iVar2, var& oVar) const
{
    int dim1 = iVar1.dim();
    int dim2 = iVar2.dim();

    // Case 1: iVar2 has size 1
    // Call back to the unary operator
    if ((dim2 == 1) && (iVar2.size() == 1))
    {
        // This could be parallel!
        for (int i=0; i<iVar1.size(); i++)
        {
            var tmp = oVar.at(i);
            scalar(iVar1.at(i), iVar2, tmp);
        }
        return;
    }

    // Case 2: iVar2 is also an array
    // Check that the two arrays are broadcastable
    if (dim2 > dim1)
        throw std::runtime_error("var::broadcast: input dimension too large");
    if (iVar1.atype() != iVar2.atype())
        throw std::runtime_error("var::broadcast: types must match (for now)");
    for (int i=0; i>dim2; i++)
    {
        // The dimensions should match
        if (iVar1.shape(dim1-i) != iVar2.shape(dim2-i))
            throw std::runtime_error("var::broadcast: dimension mismatch");
    }

    // If it didn't throw, then the arrays are broadcastable
    // In this case, loop over iVar1 with different offsets
    int dimO = oVar.dim();
    int step1 = dim1-dim2 > 0 ? iVar1.stride(dim1-dim2-1) : iVar1.size();
    int stepO = dimO-dim2 > 0 ? oVar.stride(dimO-dim2-1) : oVar.size();
    int nOps = iVar1.size() / step1;
    for (int i=0; i<nOps; i++)
        vector(iVar1, step1*i, iVar2, 0, oVar, stepO*i);
}


/**
 * Binary broadcaster
 *
 * Broadcasts iVar2 against iVar1; the general idea is that both vars are the
 * same size.  The dimension of the operation should be given by an mDim set at
 * construction time.
 */
void BinaryFunctor::broadcast(var iVar1, var iVar2, var& oVar) const
{
    // Check that the two arrays are broadcastable
    if (iVar1.atype() != iVar2.atype())
        throw std::runtime_error("var::broadcast: types must match (for now)");

    // Find the common dimension.
    int dim1 = iVar1.dim();
    int cdim = dim1 - mDim;
    if (cdim < 0)
        throw std::runtime_error("var::broadcast: input dimension too small");

    // Assume that the common dimension is to be broadcast over.
    int step1 = iVar1.stride(cdim);
    int step2 = iVar2.stride(cdim);
    int stepO =  oVar.stride(cdim);
    int nOps = iVar1.size() / step1;
    for (int i=0; i<nOps; i++)
        vector(iVar1, step1*i, iVar2, step2*i, oVar, stepO*i);
}


#define CMATH_UNARY_FUNCTOR(F,f)                                        \
    void F::scalar(const var& iVar, var& oVar) const                    \
    {                                                                   \
        switch (type(iVar))                                             \
        {                                                               \
        case TYPE_ARRAY:                                                \
            broadcast(iVar, oVar);                                      \
            break;                                                      \
        case TYPE_FLOAT:                                                \
            oVar = std::f(iVar.get<float>());                           \
            break;                                                      \
        case TYPE_DOUBLE:                                               \
            oVar = std::f(iVar.get<double>());                          \
            break;                                                      \
        default:                                                        \
            throw std::runtime_error("#F##::scalar: Unknown type");     \
        }                                                               \
    }

#define COMPLEX_UNARY_FUNCTOR(F,f)                                      \
    void F::scalar(const var& iVar, var& oVar) const                    \
    {                                                                   \
        switch (type(iVar))                                             \
        {                                                               \
        case TYPE_ARRAY:                                                \
            broadcast(iVar, oVar);                                      \
            break;                                                      \
        case TYPE_FLOAT:                                                \
            oVar = std::f(iVar.get<float>());                           \
            break;                                                      \
        case TYPE_DOUBLE:                                               \
            oVar = std::f(iVar.get<double>());                          \
            break;                                                      \
        case TYPE_CFLOAT:                                               \
            oVar = std::f(iVar.get<cfloat>());                          \
            break;                                                      \
        case TYPE_CDOUBLE:                                              \
            oVar = std::f(iVar.get<cdouble>());                         \
            break;                                                      \
        default:                                                        \
            throw std::runtime_error("#F##::scalar(): Unknown type");   \
        }                                                               \
    }


CMATH_UNARY_FUNCTOR(Floor,floor)
COMPLEX_UNARY_FUNCTOR(Sin,sin)
COMPLEX_UNARY_FUNCTOR(Cos,cos)
COMPLEX_UNARY_FUNCTOR(Tan,tan)
COMPLEX_UNARY_FUNCTOR(Sqrt,sqrt)
COMPLEX_UNARY_FUNCTOR(Log,log)
COMPLEX_UNARY_FUNCTOR(Exp,exp)
COMPLEX_UNARY_FUNCTOR(NormC,norm)


void Pow::scalar(const var& iVar1, const var& iVar2, var& oVar) const
{
    switch(type(iVar1))
    {
    case TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case TYPE_FLOAT:
        oVar = std::pow(iVar1.get<float>(), iVar2.cast<float>());
        break;
    case TYPE_DOUBLE:
        oVar = std::pow(iVar1.get<double>(), iVar2.cast<double>());
        break;
    case TYPE_CFLOAT:
        oVar = std::pow(iVar1.get<cfloat>(), iVar2.cast<cfloat>());
        break;
    case TYPE_CDOUBLE:
        oVar = std::pow(iVar1.get<cdouble>(), iVar2.cast<cdouble>());
        break;
    default:
        throw std::runtime_error("Pow::scalar: Unknown type");
    }
}


var Abs::alloc(var iVar) const
{
    // This probably ought to be a helper function.  The idea is to copy iVar,
    // but change the type to the real version.
    var r;
    var s = iVar.shape();
    switch (iVar.atype())
    {
    case TYPE_FLOAT:
    case TYPE_CFLOAT:
        r = view(s, 0.0f);
        break;
    case TYPE_DOUBLE:
    case TYPE_CDOUBLE:
        r = view(s, 0.0);
        break;
    }
    return r;
}


void Abs::scalar(const var& iVar, var& oVar) const
{
    // Not a COMPLEX_UNARY_FUNCTOR as it takes real or complex but always
    // returns real.
    switch (type(iVar))
    {
    case TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case TYPE_FLOAT:
        oVar = std::abs(iVar.get<float>());
        break;
    case TYPE_DOUBLE:
        oVar = std::abs(iVar.get<double>());
        break;
    case TYPE_CFLOAT:
        oVar = std::abs(iVar.get<cfloat>());
        break;
    case TYPE_CDOUBLE:
        oVar = std::abs(iVar.get<cdouble>());
        break;
    default:
        throw std::runtime_error("Abs::operator(): Unknown type");
    }
}


var Norm::alloc(var iVar) const
{
    // This probably ought to be a helper function.  The idea is to copy iVar,
    // but change the type to the real version.
    var r;
    var s = iVar.shape();
    switch (iVar.atype())
    {
    case TYPE_FLOAT:
    case TYPE_CFLOAT:
        r = view(s, 0.0f);
        break;
    case TYPE_DOUBLE:
    case TYPE_CDOUBLE:
        r = view(s, 0.0);
        break;
    }
    return r;
}


void Norm::scalar(const var& iVar, var& oVar) const
{
    // Not a COMPLEX_UNARY_FUNCTOR as it takes real or complex but always
    // returns real.
    switch (type(iVar))
    {
    case TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case TYPE_FLOAT:
        oVar = std::norm(iVar.get<float>());
        break;
    case TYPE_DOUBLE:
        oVar = std::norm(iVar.get<double>());
        break;
    case TYPE_CFLOAT:
        oVar = std::norm(iVar.get<cfloat>());
        break;
    case TYPE_CDOUBLE:
        oVar = std::norm(iVar.get<cdouble>());
        break;
    default:
        throw std::runtime_error("Abs::operator(): Unknown type");
    }
}


void Set::scalar(const var& iVar1, const var& iVar2, var& oVar) const
{
    switch(type(oVar))
    {
    case TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case TYPE_CHAR:
        *oVar.ptr<char>() = iVar2.cast<char>();
        break;
    case TYPE_INT:
        *oVar.ptr<int>() = iVar2.cast<int>();
        break;
    case TYPE_LONG:
        *oVar.ptr<long>() = iVar2.cast<long>();
        break;
    case TYPE_FLOAT:
        *oVar.ptr<float>() = iVar2.cast<float>();
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>() = iVar2.cast<double>();
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<cfloat>() = iVar2.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<cdouble>() = iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Set::scalar: Unknown type");
    }
}


void Set::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    assert(type(iVar1) == TYPE_ARRAY);
    int size = iVar2.size();
    switch(iVar1.atype())
    {
    case TYPE_FLOAT:
    {
        float* x = iVar2.ptr<float>(iOffset2);
        float* y = iVar1.ptr<float>(iOffset1);
        cblas_scopy(size, x, 1, y, 1);
        break;
    }
    case TYPE_DOUBLE:
    {
        double* x = iVar2.ptr<double>(iOffset2);
        double* y = iVar1.ptr<double>(iOffset1);
        cblas_dcopy(size, x, 1, y, 1);
        break;
    }
    default:
        throw std::runtime_error("Set::array: Unknown type");
    }
}


void Add::scalar(const var& iVar1, const var& iVar2, var& oVar) const
{
    switch(type(iVar1))
    {
    case TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case TYPE_CHAR:
        *oVar.ptr<char>() = iVar1.get<char>() + iVar2.cast<char>();
        break;
    case TYPE_INT:
        *oVar.ptr<int>() = iVar1.get<int>() + iVar2.cast<int>();
        break;
    case TYPE_LONG:
        *oVar.ptr<long>() = iVar1.get<long>() + iVar2.cast<long>();
        break;
    case TYPE_FLOAT:
        *oVar.ptr<float>() = iVar1.get<float>() + iVar2.cast<float>();
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>() = iVar1.get<double>() + iVar2.cast<double>();
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<cfloat>() = iVar1.get<cfloat>() + iVar2.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<cdouble>() = iVar1.get<cdouble>() + iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Add::scalar: Unknown type");
    }
}


void Add::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    assert(type(iVar1) == TYPE_ARRAY);
    assert(iVar1.is(oVar));
    int size = iVar2.size();
    switch(iVar1.atype())
    {
    case TYPE_FLOAT:
    {
        float* x = iVar2.ptr<float>(iOffset2);
        float* y = iVar1.ptr<float>(iOffset1);
        cblas_saxpy(size, 1.0f, x, 1, y, 1);
        break;
    }
    case TYPE_DOUBLE:
    {
        double* x = iVar2.ptr<double>(iOffset2);
        double* y = iVar1.ptr<double>(iOffset1);
        cblas_daxpy(size, 1.0, x, 1, y, 1);
        break;
    }
    default:
        throw std::runtime_error("Add::vector: Unknown type");
    }
}


void Sub::scalar(const var& iVar1, const var& iVar2, var& oVar) const
{
    switch(type(iVar1))
    {
    case TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case TYPE_CHAR:
        *oVar.ptr<char>() = iVar1.get<char>() - iVar2.cast<char>();
        break;
    case TYPE_INT:
        *oVar.ptr<int>() = iVar1.get<int>() - iVar2.cast<int>();
        break;
    case TYPE_LONG:
        *oVar.ptr<long>() = iVar1.get<long>() - iVar2.cast<long>();
        break;
    case TYPE_FLOAT:
        *oVar.ptr<float>() = iVar1.get<float>() - iVar2.cast<float>();
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>() = iVar1.get<double>() - iVar2.cast<double>();
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<cfloat>() = iVar1.get<cfloat>() - iVar2.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<cdouble>() = iVar1.get<cdouble>() - iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Sub::scalar: Unknown type");
    }
}


void Sub::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    assert(type(iVar1) == TYPE_ARRAY);
    assert(iVar1.is(oVar));
    int size = iVar2.size();
    switch(iVar1.atype())
    {
    case TYPE_FLOAT:
    {
        float* x = iVar2.ptr<float>(iOffset2);
        float* y = iVar1.ptr<float>(iOffset1);
        cblas_saxpy(size, -1.0f, x, 1, y, 1);
        break;
    }
    case TYPE_DOUBLE:
    {
        double* x = iVar2.ptr<double>(iOffset2);
        double* y = iVar1.ptr<double>(iOffset1);
        cblas_daxpy(size, -1.0, x, 1, y, 1);
        break;
    }
    default:
        throw std::runtime_error("Sub::vector: Unknown type");
    }
}


/**
 * Overload of broadcast() for multiplication.  This catches the case where
 * just scaling is being done, meaning a different BLAS call is necessary.
 */
void Mul::broadcast(var iVar1, var iVar2, var& oVar) const
{
    // If iVar2 has size 1, call scale rather than let the base class broadcast
    // it over the unary operator.
    if ((iVar2.dim() == 1) && (iVar2.size() == 1))
    {
        scale(iVar1, iVar2, oVar, 0);
        return;
    }
    else
        ArithmeticFunctor::broadcast(iVar1, iVar2, oVar);
}


void Mul::scale(var iVar1, var iVar2, var& oVar, int iOffset) const
{
    assert(type(iVar1) == TYPE_ARRAY);
    int size = iVar1.size();
    switch(iVar1.atype())
    {
    case TYPE_FLOAT:
    {
        float alpha = iVar2.cast<float>();
        float* x = iVar1.ptr<float>(iOffset);
        cblas_sscal(size, alpha, x, 1);
        break;
    }
    case TYPE_DOUBLE:
    {
        double alpha = iVar2.cast<double>();
        double* x = iVar1.ptr<double>(iOffset);
        cblas_dscal(size, alpha, x, 1);
        break;
    }
    default:
        throw std::runtime_error("Mul::scal: Unknown type");
    }
}


void Mul::scalar(const var& iVar1, const var& iVar2, var& oVar) const
{
    switch(type(iVar1))
    {
    case TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case TYPE_CHAR:
        *oVar.ptr<char>() = iVar1.get<char>() * iVar2.cast<char>();
        break;
    case TYPE_INT:
        *oVar.ptr<int>() = iVar1.get<int>() * iVar2.cast<int>();
        break;
    case TYPE_LONG:
        *oVar.ptr<long>() = iVar1.get<long>() * iVar2.cast<long>();
        break;
    case TYPE_FLOAT:
        *oVar.ptr<float>() = iVar1.get<float>() * iVar2.cast<float>();
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>() = iVar1.get<double>() * iVar2.cast<double>();
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<cfloat>() = iVar1.get<cfloat>() * iVar2.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<cdouble>() = iVar1.get<cdouble>() * iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Mul::scalar: Unknown type");
    }
}


void Mul::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    // Elementwise multiplication is actually multiplication by a diagonal
    // matrix.  In BLAS speak, a diagonal matrix is a band matrix with no
    // superdiagonals.
    assert(type(iVar1) == TYPE_ARRAY);
    int size = iVar2.size();
    if (iVar1.is(oVar))
    {
        // xtbmv() (triangular band) overwrites the current location.
        switch(iVar1.atype())
        {
        case TYPE_FLOAT:
        {
            float* A = iVar2.ptr<float>(iOffset2);
            float* x = iVar1.ptr<float>(iOffset1);
            cblas_stbmv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit,
                        size, 0, A, 1, x, 1);
            break;
        }
        case TYPE_DOUBLE:
        {
            double* A = iVar2.ptr<double>(iOffset2);
            double* x = iVar1.ptr<double>(iOffset1);
            cblas_dtbmv(CblasRowMajor, CblasUpper, CblasNoTrans, CblasNonUnit,
                        size, 0, A, 1, x, 1);
            break;
        }
        default:
            throw std::runtime_error("Mul::vector: Unknown type");
        }
    }
    else
    {
        // xsbmv() (symmetric band) puts the result in a new location.
        switch(iVar1.atype())
        {
        case TYPE_FLOAT:
        {
            float* A = iVar2.ptr<float>(iOffset2);
            float* x = iVar1.ptr<float>(iOffset1);
            float* y = oVar.ptr<float>(iOffsetO);
            cblas_ssbmv(CblasRowMajor, CblasUpper, size, 0,
                        1.0f, A, 1, x, 1, 0.0f, y, 1);
            break;
        }
        case TYPE_DOUBLE:
        {
            double* A = iVar2.ptr<double>(iOffset2);
            double* x = iVar1.ptr<double>(iOffset1);
            double* y = oVar.ptr<double>(iOffsetO);
            cblas_dsbmv(CblasRowMajor, CblasUpper, size, 0,
                        1.0, A, 1, x, 1, 0.0, y, 1);
            break;
        }
        default:
            throw std::runtime_error("Mul::vector: Unknown type");
        }
    }
}


var Dot::alloc(var iVar) const
{
    var r;
    if (iVar.dim() > 1)
    {
        // Allocate an output array
        var s = iVar.shape();
        s[s.size()-1] = 1;
        r = view(s, iVar.at(0));
    }
    else
    {
        r = iVar.at(0);
    }
    return r;
}


void Dot::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    assert(type(iVar1) == TYPE_ARRAY);
    int size = iVar2.size();
    if (iVar1.is(oVar))
        throw std::runtime_error("Dot::vector: Cannot operate in place");

    switch(iVar1.atype())
    {
    case TYPE_FLOAT:
    {
        float* x = iVar1.ptr<float>(iOffset1);
        float* y = iVar2.ptr<float>(iOffset2);
        *oVar.ptr<float>(iOffsetO) = cblas_sdot(size, x, 1, y, 1);
        break;
    }
    case TYPE_DOUBLE:
    {
        double* x = iVar1.ptr<double>(iOffset1);
        double* y = iVar2.ptr<double>(iOffset2);
        *oVar.ptr<double>(iOffsetO) = cblas_ddot(size, x, 1, y, 1);
        break;
    }
    case TYPE_CFLOAT:
    {
        cfloat* x = iVar1.ptr<cfloat>(iOffset1);
        cfloat* y = iVar2.ptr<cfloat>(iOffset2);
        cblas_cdotc_sub(size, x, 1, y, 1, oVar.ptr<cfloat>(iOffsetO));
        break;
    }
    case TYPE_CDOUBLE:
    {
        cdouble* x = iVar1.ptr<cdouble>(iOffset1);
        cdouble* y = iVar2.ptr<cdouble>(iOffset2);
        cblas_zdotc_sub(size, x, 1, y, 1, oVar.ptr<cdouble>(iOffsetO));
        break;
    }
    default:
        throw std::runtime_error("Dot::vector: Unknown type");
    }
}


void Div::scalar(const var& iVar1, const var& iVar2, var& oVar) const
{
    switch(type(iVar1))
    {
    case TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case TYPE_CHAR:
        *oVar.ptr<char>() = iVar1.get<char>() / iVar2.cast<char>();
        break;
    case TYPE_INT:
        *oVar.ptr<int>() = iVar1.get<int>() / iVar2.cast<int>();
        break;
    case TYPE_LONG:
        *oVar.ptr<long>() = iVar1.get<long>() / iVar2.cast<long>();
        break;
    case TYPE_FLOAT:
        *oVar.ptr<float>() = iVar1.get<float>() / iVar2.cast<float>();
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>() = iVar1.get<double>() / iVar2.cast<double>();
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<cfloat>() = iVar1.get<cfloat>() / iVar2.cast<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<cdouble>() = iVar1.get<cdouble>() / iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Div::scalar: Unknown type");
    }
}


var ASum::alloc(var iVar) const
{
    var r;
    if (iVar.dim() > 1)
    {
        // Allocate an output array
        var s = iVar.shape();
        s[s.size()-1] = 1;
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
        case TYPE_CFLOAT:
            r = view(s, 0.0f);
            break;
        case TYPE_DOUBLE:
        case TYPE_CDOUBLE:
            r = view(s, 0.0);
            break;
        }
    }
    else
    {
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
        case TYPE_CFLOAT:
            r = 0.0f;
            break;
        case TYPE_DOUBLE:
        case TYPE_CDOUBLE:
            r = 0.0;
            break;
        }
    }
    return r;
}


void ASum::scalar(const var& iVar, var& oVar) const
{
    switch(type(iVar))
    {
    case TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case TYPE_FLOAT:
        *oVar.ptr<float>() = std::abs(iVar.get<float>());
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>() = std::abs(iVar.get<double>());
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<cfloat>() = std::abs(iVar.get<cfloat>());
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<cdouble>() = std::abs(iVar.get<cdouble>());
        break;
    default:
        throw std::runtime_error("ASum::operator(): Unknown type");
    }
}


void ASum::vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const
{
    assert(type(iVar) == TYPE_ARRAY);
    int size = iVar.size();
    switch(iVar.atype())
    {
    case TYPE_FLOAT:
        *(oVar.ptr<float>(iOffsetO)) =
            cblas_sasum(size, iVar.ptr<float>(iOffsetI), 1);
        break;
    case TYPE_DOUBLE:
        *(oVar.ptr<double>(iOffsetO)) =
            cblas_dasum(size, iVar.ptr<double>(iOffsetI), 1);
        break;
    default:
        throw std::runtime_error("ASum::array: Unknown type");
    }
}


var Sum::alloc(var iVar) const
{
    var r;
    if (iVar.dim() > 1)
    {
        // Allocate an output array
        var s = iVar.shape();
        s[s.size()-1] = 1;
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
            r = view(s, 0.0f);
            break;
        case TYPE_DOUBLE:
            r = view(s, 0.0);
            break;
        case TYPE_CFLOAT:
            r = view(s, cfloat(0.0f, 0.0f));
            break;
        case TYPE_CDOUBLE:
            r = view(s, cdouble(0.0, 0.0));
            break;
        }
    }
    else
    {
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
            r = 0.0f;
            break;
        case TYPE_DOUBLE:
            r = 0.0;
            break;
        case TYPE_CFLOAT:
            r = cfloat(0.0f, 0.0f);
            break;
        case TYPE_CDOUBLE:
            r = cdouble(0.0, 0.0);
            break;
        }
    }
    return r;
}


void Sum::scalar(const var& iVar, var& oVar) const
{
    switch(type(iVar))
    {
    case TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case TYPE_FLOAT:
        *oVar.ptr<float>() = iVar.get<float>();
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>() = iVar.get<double>();
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<cfloat>() = iVar.get<cfloat>();
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<cdouble>() = iVar.get<cdouble>();
        break;
    default:
        throw std::runtime_error("Sum::operator(): Unknown type");
    }
}


template<class T>
T sumArray(int iSize, T* iPointer)
{
    T sum = (T)0;
    for (int i=0; i<iSize; i++)
        sum += iPointer[i];
    return sum;
}


void Sum::vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const
{
    assert(type(iVar) == TYPE_ARRAY);
    int size = iVar.size();
    switch(iVar.atype())
    {
    case TYPE_FLOAT:
        *(oVar.ptr<float>(iOffsetO)) =
            sumArray(size, iVar.ptr<float>(iOffsetI));
        break;
    case TYPE_DOUBLE:
        *(oVar.ptr<double>(iOffsetO)) =
            sumArray(size, iVar.ptr<double>(iOffsetI));
        break;
    case TYPE_CFLOAT:
        *(oVar.ptr<cfloat>(iOffsetO)) =
            sumArray(size, iVar.ptr<cfloat>(iOffsetI));
        break;
    case TYPE_CDOUBLE:
        *(oVar.ptr<cdouble>(iOffsetO)) =
            sumArray(size, iVar.ptr<cdouble>(iOffsetI));
        break;
    default:
        throw std::runtime_error("Sum::array: Unknown type");
    }
}


var IAMax::alloc(var iVar) const
{
    var r;
    if (iVar.dim() > 1)
    {
        // Allocate an output array
        var s = iVar.shape();
        s[s.size()-1] = 1;
        r = view(s, 0l);
    }
    else
    {
        r = 0l;
    }
    return r;
}


void IAMax::vector(
    var iVar, ind iOffsetI, var& oVar, ind iOffsetO
) const
{
    assert(type(iVar) == TYPE_ARRAY);
    if (iVar.is(oVar))
        throw std::runtime_error("IAMax::vector: Cannot operate in place");

    int size = iVar.shape(iVar.dim()-1);
    switch(iVar.atype())
    {
    case TYPE_FLOAT:
    {
        float* x = iVar.ptr<float>(iOffsetI);
        *oVar.ptr<long>(iOffsetO) = cblas_isamax(size, x, 1);
        break;
    }
    case TYPE_DOUBLE:
    {
        double* x = iVar.ptr<double>(iOffsetI);
        *oVar.ptr<long>(iOffsetO) = cblas_idamax(size, x, 1);
        break;
    }
    case TYPE_CFLOAT:
    {
        cfloat* x = iVar.ptr<cfloat>(iOffsetI);
        *oVar.ptr<long>(iOffsetO) = cblas_icamax(size, x, 1);
        break;
    }
    case TYPE_CDOUBLE:
    {
        cdouble* x = iVar.ptr<cdouble>(iOffsetI);
        *oVar.ptr<long>(iOffsetO) = cblas_izamax(size, x, 1);
        break;
    }
    default:
        throw std::runtime_error("IAMax::vector: Unknown type");
    }
}


#if 0
void varheap::mul(
    int iM, int iN, int iK, int iOffset,
    var iVarA, int iOffsetA, varheap* iHeapB
)
{
    // Swap B and A as they're transposed in FORTRAN world
    static char trans = 'T';
    int* m = &iN;
    int* n = &iM;
    int* k = &iK;
    switch(type())
    {
    case TYPE_FLOAT:
    {
        static float alpha = 1.0f;
        float* a = iHeapB->ptr<float>();
        float* b = iVarA.ptr<float>(iOffsetA);
        float* c = ptr<float>(iOffset);
        sgemm_(&trans, &trans, m, n, k, &alpha, a, k, b, m, &alpha, c, m);
        break;
    }
    case TYPE_DOUBLE:
    {
        static double alpha = 1.0;
        double* a = iHeapB->ptr<double>();
        double* b = iVarA.ptr<double>(iOffsetA);
        double* c = ptr<double>(iOffset);
        dgemm_(&trans, &trans, m, n, k, &alpha, a, k, b, m, &alpha, c, m);
        break;
    }
    default:
        throw std::runtime_error("varheap::mul: Unknown type");
    }
}
#endif
