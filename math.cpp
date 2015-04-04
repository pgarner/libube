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
#include <algorithm>

#include "c++blas.h"
#include "c++lapack.h"

#include "var.h"


namespace libvar
{
    /*
     * The instantiations of the math and standard functors defined in this
     * module.  They are declared extern in var.h
     */

    // std math
    Norm norm;
    Sin sin;
    Cos cos;
    Tan tan;
    ATan atan;
    Floor floor;
    Sqrt sqrt;
    Log log;
    Exp exp;
    Real real;
    Imag imag;
    Abs abs;
    Arg arg;
    Pow pow;

    // BLAS
    Set set;
    Add add;
    Sub sub;
    Mul mul;
    Dot dot;
    Div div;
    ASum asum;
    Sum sum;
    IAMax iamax;

    // LAPACK
    Roots roots;
    Poly poly;

    // Std C++
    Sort sort;
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
 * Allocator where the output is the same shape but always real
 */
var RealUnaryFunctor::alloc(var iVar) const
{
    // The idea is to copy iVar, but change the type to the real version.
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


/**
 * Allocator where the output retains the type but is scalar
 */
static var scalarAlloc(var iVar)
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
 * The allocator for a N-ary functor
 */
var NaryFunctor::alloc(var iVar) const
{
    var r;
    if (iVar)
        r = iVar[0].copy(true);
    else
        throw error("NaryFunctor::alloc: must override alloc()");
    return r;
}


/**
 * The normal operation of operator() is to allocate space then call the
 * protected method scalar().  Notice that it can't allocate space without
 * other information, so the default alloc() will throw.
 */
var NaryFunctor::operator ()() const
{
    var v = alloc(nil);
    scalar(nil, v);
    return v;
}


/**
 * The normal operation of operator(iVar) is to allocate space then call the
 * protected method scalar().
 */
var NaryFunctor::operator ()(const var& iVar) const
{
    var v = alloc(iVar);
    scalar(iVar, v);
    return v;
}


/**
 * The normal operation of operator(iVar, oVar) is to just call the protected
 * method scalar().
 */
var NaryFunctor::operator ()(const var& iVar, var& oVar) const
{
    scalar(iVar, oVar);
    return oVar;
}


/**
 * The default behaviour of scalar(), hence operator(), is to broadcast.
 */
void NaryFunctor::scalar(const var& iVar, var& oVar) const
{
    broadcast(iVar, oVar);
}


/**
 * The default vector() throws, meaning that it should have been overridden or
 * implemented by scalar() or vector() with offsets.
 */
void NaryFunctor::vector(var iVar, var& oVar) const
{
    throw error("UnaryFunctor: not a vector operation");
}


/**
 * Nary broadcaster
 *
 * Broadcasts the operation over the common dimension of each var in the array
 * iVar.
 */
void NaryFunctor::broadcast(var iVar, var& oVar) const
{
    // Check that the arrays are broadcastable
    for (int i=1; i<iVar.size(); i++)
        if (iVar[0].atype() != iVar[i].atype())
            throw error(
                "var::broadcast: types must match (for now)"
            );

    // Find the common dimension.
    int dimO = oVar.dim();
    int cdim = dimO - mDim;
    if (cdim < 0)
        throw error("var::broadcast: input dimension too small");

    // Assume that the common dimension is to be broadcast over.
    int stepO = cdim > 0 ?  oVar.stride(cdim-1) : 0;
    int nOps = cdim > 0 ? oVar.size() / stepO : 1;
    for (int i=0; i<nOps; i++)
    {
        var iv;
        for (int j=0; j<iVar.size(); j++)
        {
            int dim = iVar[j].dim();
            int step = cdim > 0 ? iVar[j].stride(cdim-1) : 0;
            // Don't take a view of a single value
            iv.push(
                dim == cdim ? iVar[j][i] : iVar[j].subview(dim-cdim, step*i)
            );
        }
        var ov = (dimO == cdim) ? oVar[i] : oVar.subview(dimO-cdim, stepO*i);
        vector(iv, ov);
    }
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
    throw error("UnaryFunctor: not a vector operation");
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
    {
        vstream s;
        s << "UnaryFunctor::broadcast: dimension too large ";
        s << mDim << " > " << dimI;
        throw error(s);
    }

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
    var iv1 =
        (dim1 == cdim) ? iVar1[iOffset1] : iVar1.subview(dim1-cdim, iOffset1);
    var iv2 =
        (dim2 == cdim) ? iVar2[iOffset2] : iVar2.subview(dim2-cdim, iOffset2);
    var ov =
        (dimO == cdim) ?  oVar[iOffsetO] :  oVar.subview(dimO-cdim, iOffsetO);
    vector(iv1, iv2, ov);
}


/**
 * The default vector() throws, meaning that it should have been overridden or
 * implemented by scalar() or vector() with offsets.
 */
void ArithmeticFunctor::vector(var iVar1, var iVar2, var& oVar) const
{
    throw error("ArithmeticFunctor: not a vector operation");
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
        throw error("var::broadcast: input dimension too large");
    if (iVar1.atype() != iVar2.atype())
        throw error("var::broadcast: types must match (for now)");
    for (int i=1; i<dim2; i++)
    {
        // The dimensions should match
        if (iVar1.shape(dim1-i) != iVar2.shape(dim2-i))
            throw error("var::broadcast: dimension mismatch");
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
        throw error("var::broadcast: types must match (for now)");

    // Find the common dimension.
    int dim1 = iVar1.dim();
    int cdim = dim1 - mDim;
    if (cdim < 0)
        throw error("var::broadcast: input dimension too small");

    // Assume that the common dimension is to be broadcast over.
    int step1 = cdim > 0 ? iVar1.stride(cdim-1) : 0;
    int step2 = cdim > 0 ? iVar2.stride(cdim-1) : 0;
    int stepO = cdim > 0 ?  oVar.stride(cdim-1) : 0;
    int nOps = cdim > 0 ? iVar1.size() / step1 : 1;
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
            throw error("#F##::scalar: Unknown type");                  \
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
            throw error("#F##::scalar(): Unknown type");                \
        }                                                               \
    }


CMATH_UNARY_FUNCTOR(Floor,floor)
COMPLEX_UNARY_FUNCTOR(Sin,sin)
COMPLEX_UNARY_FUNCTOR(Cos,cos)
COMPLEX_UNARY_FUNCTOR(Tan,tan)
COMPLEX_UNARY_FUNCTOR(ATan,atan)
COMPLEX_UNARY_FUNCTOR(Sqrt,sqrt)
COMPLEX_UNARY_FUNCTOR(Log,log)
COMPLEX_UNARY_FUNCTOR(Exp,exp)
COMPLEX_UNARY_FUNCTOR(Real,real)
COMPLEX_UNARY_FUNCTOR(Imag,imag)
COMPLEX_UNARY_FUNCTOR(Abs,abs)
COMPLEX_UNARY_FUNCTOR(Arg,arg)
COMPLEX_UNARY_FUNCTOR(Norm,norm)


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
        throw error("Pow::scalar: Unknown type");
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
        throw error("Set::scalar: Unknown type");
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
        blas::copy(
            size,
            iVar2.ptr<float>(iOffset2),
            iVar1.ptr<float>(iOffset1)
        );
        break;
    case TYPE_DOUBLE:
        blas::copy(
            size,
            iVar2.ptr<double>(iOffset2),
            iVar1.ptr<double>(iOffset1)
        );
        break;
    default:
        throw error("Set::array: Unknown type");
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
        throw error("Add::scalar: Unknown type");
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
        blas::axpy(
            size, 1.0f,
            iVar2.ptr<float>(iOffset2),
            iVar1.ptr<float>(iOffset1)
        );
        break;
    case TYPE_DOUBLE:
        blas::axpy(
            size, 1.0,
            iVar2.ptr<double>(iOffset2),
            iVar1.ptr<double>(iOffset1)
        );
        break;
    case TYPE_CFLOAT:
        blas::axpy(
            size, cfloat(1.0f,0.0f),
            iVar2.ptr<cfloat>(iOffset2),
            iVar1.ptr<cfloat>(iOffset1)
        );
        break;
    case TYPE_CDOUBLE:
        blas::axpy(
            size, cdouble(1.0,0.0),
            iVar2.ptr<cdouble>(iOffset2),
            iVar1.ptr<cdouble>(iOffset1)
        );
        break;
    default:
        throw error("Add::vector: Unknown type");
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
        throw error("Sub::scalar: Unknown type");
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
        blas::axpy(
            size, -1.0f,
            iVar2.ptr<float>(iOffset2),
            iVar1.ptr<float>(iOffset1)
        );
        break;
    case TYPE_DOUBLE:
        blas::axpy(
            size, -1.0,
            iVar2.ptr<double>(iOffset2),
            iVar1.ptr<double>(iOffset1)
        );
        break;
    default:
        throw error("Sub::vector: Unknown type");
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
    if (oVar.is(iVar1))
    {
        // Scale in place
        switch(iVar1.atype())
        {
        case TYPE_FLOAT:
            blas::scal(
                size,
                iVar2.cast<float>(),
                iVar1.ptr<float>(iOffset)
            );
            break;
        case TYPE_DOUBLE:
            blas::scal(
                size,
                iVar2.cast<double>(),
                iVar1.ptr<double>(iOffset)
            );
            break;
        default:
            throw error("Mul::scal: Unknown type");
        }
    }
    else
    {
        // Copy data before scaling
        switch(iVar1.atype())
        {
        case TYPE_FLOAT:
        {
            float alpha = iVar2.cast<float>();
            float* x = iVar1.ptr<float>(iOffset);
            float* y =  oVar.ptr<float>(iOffset);
            blas::copy(size, x, y);
            blas::scal(size, alpha, y);
            break;
        }
        case TYPE_DOUBLE:
        {
            double alpha = iVar2.cast<double>();
            double* x = iVar1.ptr<double>(iOffset);
            double* y =  oVar.ptr<double>(iOffset);
            blas::copy(size, x, y);
            blas::scal(size, alpha, y);
            break;
        }
        default:
            throw error("Mul::scal: Unknown type");
        }
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
        throw error("Mul::scalar: Unknown type");
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
        // tbmv() (triangular band) overwrites the current location.
        switch(iVar1.atype())
        {
        case TYPE_FLOAT:
            blas::tbmv(
                size, 0,
                iVar2.ptr<float>(iOffset2), iVar1.ptr<float>(iOffset1)
            );
            break;
        case TYPE_DOUBLE:
            blas::tbmv(
                size, 0,
                iVar2.ptr<double>(iOffset2), iVar1.ptr<double>(iOffset1)
            );
            break;
        default:
            throw error("Mul::vector: Unknown type");
        }
    }
    else
    {
        // sbmv() (symmetric band) puts the result in a new location.
        switch(iVar1.atype())
        {
        case TYPE_FLOAT:
            blas::sbmv(size, 0,
                       1.0f, iVar2.ptr<float>(iOffset2),
                             iVar1.ptr<float>(iOffset1),
                       0.0f, oVar.ptr<float>(iOffsetO));
            break;
        case TYPE_DOUBLE:
            blas::sbmv(size, 0,
                       1.0, iVar2.ptr<double>(iOffset2),
                            iVar1.ptr<double>(iOffset1),
                       0.0, oVar.ptr<double>(iOffsetO));
            break;
        default:
            throw error("Mul::vector: Unknown type");
        }
    }
}


var Dot::alloc(var iVar) const
{
    var r = scalarAlloc(iVar);
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
        throw error("Dot::vector: Cannot operate in place");

    switch(iVar1.atype())
    {
    case TYPE_FLOAT:
        *oVar.ptr<float>(iOffsetO) =
            blas::dot(
                size,
                iVar1.ptr<float>(iOffset1),
                iVar2.ptr<float>(iOffset2)
            );
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>(iOffsetO) =
            blas::dot(
                size,
                iVar1.ptr<double>(iOffset1),
                iVar2.ptr<double>(iOffset2)
            );
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<cfloat>(iOffsetO) =
            blas::dot(
                size,
                iVar1.ptr<cfloat>(iOffset1),
                iVar2.ptr<cfloat>(iOffset2)
            );
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<cdouble>(iOffsetO) =
            blas::dot(
                size,
                iVar1.ptr<cdouble>(iOffset1),
                iVar2.ptr<cdouble>(iOffset2)
            );
        break;
    default:
        throw error("Dot::vector: Unknown type");
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
        throw error("Div::scalar: Unknown type");
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
        throw error("ASum::operator(): Unknown type");
    }
}


void ASum::vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const
{
    assert(type(iVar) == TYPE_ARRAY);
    int size = iVar.shape(-1);
    switch(iVar.atype())
    {
    case TYPE_FLOAT:
        *oVar.ptr<float>(iOffsetO) =
            blas::asum(size, iVar.ptr<float>(iOffsetI));
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<double>(iOffsetO) =
            blas::asum(size, iVar.ptr<double>(iOffsetI));
        break;
    default:
        throw error("ASum::array: Unknown type");
    }
}


var Sum::alloc(var iVar) const
{
    var r = scalarAlloc(iVar);
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
        throw error("Sum::operator(): Unknown type");
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
    int size = iVar.shape(-1);
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
        throw error("Sum::array: Unknown type");
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
        throw error("IAMax::vector: Cannot operate in place");

    int size = iVar.shape(-1);
    switch(iVar.atype())
    {
    case TYPE_FLOAT:
        *oVar.ptr<long>(iOffsetO) =
            blas::iamax(size, iVar.ptr<float>(iOffsetI));
        break;
    case TYPE_DOUBLE:
        *oVar.ptr<long>(iOffsetO) =
            blas::iamax(size, iVar.ptr<double>(iOffsetI));
        break;
    case TYPE_CFLOAT:
        *oVar.ptr<long>(iOffsetO) =
            blas::iamax(size, iVar.ptr<cfloat>(iOffsetI));
        break;
    case TYPE_CDOUBLE:
        *oVar.ptr<long>(iOffsetO) =
            blas::iamax(size, iVar.ptr<cdouble>(iOffsetI));
        break;
    default:
        throw error("IAMax::vector: Unknown type");
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
        throw error("varheap::mul: Unknown type");
    }
}
#endif

/**
 * Allocator where the output is the same shape but always complex
 */
var Roots::alloc(var iVar) const
{
    // The idea is to copy iVar, but change the type to the complex version.
    var r;
    var s = iVar.shape();
    s[s.size()-1] -= 1;
    switch (iVar.atype())
    {
    case TYPE_FLOAT:
    case TYPE_CFLOAT:
        r = view(s, cfloat(0.0f, 0.0f));
        break;
    case TYPE_DOUBLE:
    case TYPE_CDOUBLE:
        r = view(s, cfloat(0.0, 0.0));
        break;
    }
    return r;
}

void Roots::vector(var iVar, var& oVar) const
{
    // Form the companion matrix of the polynomial
    int size = oVar.size();
    var a = var(size*size, iVar.at(0)).view({size, size});
    for (int r=1; r<size; r++)
        a(r, size-1) = 0.0;
    for (int r=0; r<size-1; r++)
        for (int c=0; c<size-1; c++)
            a(r+1,c) = (r == c) ? 1.0 : 0.0;
    for (int c=0; c<size; c++)
        a(0,c) = iVar(c+1)/iVar(0) * -1;

    // Find the eigenvalues of the matrix
    var re, im;
    switch (iVar.atype())
    {
    case TYPE_FLOAT:
        re = var(size, 1.0f);
        im = var(size, 1.0f);
        lapack::geev(
            size,
            a.ptr<float>(),
            re.ptr<float>(), im.ptr<float>()
        );
        break;
    case TYPE_DOUBLE:
        re = var(size, 1.0);
        im = var(size, 1.0);
        lapack::geev(
            size,
            a.ptr<double>(),
            re.ptr<double>(), im.ptr<double>()
        );
        break;
#if 0
    case TYPE_CFLOAT:
        re = var(size, 1.0f);
        im = var(size, 1.0f);
        lapack::geev(
            size,
            a.ptr<cfloat>(),
            re.ptr<cfloat>(), im.ptr<cfloat>(),
            (cfloat*)0, (cfloat*)0
        );
        break;
    case TYPE_CDOUBLE:
        re = var(size, 1.0);
        im = var(size, 1.0);
        lapack::geev(
            size,
            a.ptr<cdouble>(),
            re.ptr<cdouble>(), im.ptr<cdouble>(),
            (cdouble*)0, (cdouble*)0
        );
        break;
#endif
    default:
        throw error("Unknown type");
    }

    // Convert (re,im) into complex
    switch (iVar.atype())
    {
    case TYPE_FLOAT:
    case TYPE_CFLOAT:
        for (int i=0; i<size; i++)
            oVar(i) = cfloat(re[i].get<float>(), im[i].get<float>());
        break;
    case TYPE_DOUBLE:
    case TYPE_CDOUBLE:
        for (int i=0; i<size; i++)
            oVar(i) = cdouble(re[i].get<double>(), im[i].get<double>());
        break;
    }
}

var Poly::alloc(var iVar) const
{
    var r;
    var s = iVar.shape();
    s[s.size()-1] += 1;
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
        r = view(s, cfloat(0.0, 0.0));
        break;
    }
    return r;
}


template<class T>
void polyt(int iOrder, T* iData, T* oData)
{
    oData[0] = (T)1.0;
    for (int i=0; i<iOrder; i++)
    {
        oData[i+1] = (T)0.0;
        T tmp0 = oData[0];
        for (int j=0; j<=i; j++)
        {
            T tmp1 = oData[j+1];
            oData[j+1] -= iData[i]*tmp0;
            tmp0 = tmp1;
        }
    }
}

void Poly::vector(var iVar, var& oVar) const
{
    int order = iVar.shape(-1);
    switch (iVar.atype())
    {
    case TYPE_FLOAT:
        polyt(order, iVar.ptr<float>(), oVar.ptr<float>());
        break;
    case TYPE_DOUBLE:
        polyt(order, iVar.ptr<double>(), oVar.ptr<double>());
        break;
    case TYPE_CFLOAT:
        polyt(order, iVar.ptr<cfloat>(), oVar.ptr<cfloat>());
        break;
    case TYPE_CDOUBLE:
        polyt(order, iVar.ptr<cdouble>(), oVar.ptr<cdouble>());
        break;
    default:
        throw error("Poly::vector: Unknown type");
    }
}


void Sort::vector(var iVar, var& oVar) const
{
    if (!iVar.is(oVar))
        throw error("Sort::vector: in place only for the moment");
    int size = iVar.shape(-1);
    switch (iVar.atype())
    {
    case TYPE_CHAR:
        std::sort(iVar.ptr<char>(), iVar.ptr<char>()+size);
        break;
    case TYPE_INT:
        std::sort(iVar.ptr<int>(), iVar.ptr<int>()+size);
        break;
    case TYPE_LONG:
        std::sort(iVar.ptr<long>(), iVar.ptr<long>()+size);
        break;
    case TYPE_FLOAT:
        std::sort(iVar.ptr<float>(), iVar.ptr<float>()+size);
        break;
    case TYPE_DOUBLE:
        std::sort(iVar.ptr<double>(), iVar.ptr<double>()+size);
        break;
#if 0
        // Sorting CFLOAT and CDOUBLE is undefined
    case TYPE_CFLOAT:
        std::sort(iVar.ptr<cfloat>(), iVar.ptr<cfloat>()+size);
        break;
    case TYPE_CDOUBLE:
        std::sort(iVar.ptr<cdouble>(), iVar.ptr<cdouble>()+size);
        break;
#endif
    default:
        throw error("Sort::vector: Unknown type");
    }
}
