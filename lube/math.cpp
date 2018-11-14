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
#include <stdexcept>
#include <algorithm>

#include "lube/c++blas.h"
#include "lube/c++lapack.h"
#include "lube/var.h"
#include "lube/math.h"

namespace libube
{
    /*
     * The instantiations of the math and standard functors defined in this
     * module.  They are declared extern in math.h
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
    Swap swap;
    Add add;
    Sub sub;
    Mul mul;
    Dot dot;
    Div div;
    ASum asum;
    Sum sum;
    IAMax iamax;

    // BLAS-like
    Concatenate concatenate;

    // LAPACK
    Roots roots;
    Poly poly;

    // Std C++
    Sort sort;
}


using namespace libube;


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
        throw error("Set::vector: Unknown type");
    }
}


/** Don't allocate anything */
var Swap::alloc(var iVar1, var iVar2) const
{
    return iVar1;
};


void Swap::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    assert(type(iVar1) == TYPE_ARRAY);
    int size = iVar1.size();
    switch(iVar1.atype())
    {
    case TYPE_FLOAT:
        blas::swap(
            size,
            iVar1.ptr<float>(iOffset1),
            iVar2.ptr<float>(iOffset2)
        );
        break;
    case TYPE_DOUBLE:
        blas::swap(
            size,
            iVar1.ptr<double>(iOffset1),
            iVar2.ptr<double>(iOffset2)
        );
        break;
    case TYPE_CFLOAT:
        blas::swap(
            size,
            iVar1.ptr<cfloat>(iOffset1),
            iVar2.ptr<cfloat>(iOffset2)
        );
        break;
    case TYPE_CDOUBLE:
        blas::swap(
            size,
            iVar1.ptr<cdouble>(iOffset1),
            iVar2.ptr<cdouble>(iOffset2)
        );
        break;
    default:
        throw error("Swap::vector: Unknown type");
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


var Dot::alloc(var iVar1, var iVar2) const
{
    var r;
    if (iVar2.dim() == 1)
        // It's a dot product, possibly broadcast
        r = scalarAlloc(iVar1);
    else if (iVar2.dim() == 2)
    {
        // It's a matrix multiplication
        // Allocate an output array
        if (iVar1.dim() < 2)
            throw error("Dot::alloc: var1 dimension < 2");
        var s = iVar1.shape();
        s[s.size()-1] = iVar2.shape(1);
        r = view(s, iVar1.at(0));
    }
    else
        throw error("Dot::alloc: var2 dimension > 2");
    return r;
}


void Dot::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    assert(type(iVar1) == TYPE_ARRAY);
    if (iVar1.is(oVar))
        throw error("Dot::vector: Cannot operate in place");

    if (iVar2.dim() == 1)
    {
        // It's a one dimensional thing, so do it as a dot product with
        // blas::dot()
        int size = iVar2.size();
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
    else if (iVar2.dim() == 2)
    {
        // It's a two dimensional thing, so do it as a matrix multiplication
        // with blas::gemm()
        if (iVar1.shape(1) != iVar2.shape(0))
        {
            std::cout << iVar1.shape() << iVar2.shape() << std::endl;
            throw error("Dot::vector: Shapes not compatible");
        }
        switch(iVar1.atype())
        {
        case TYPE_FLOAT:
            blas::gemm(
                iVar1.shape(0), iVar2.shape(1), iVar1.shape(1),
                1.0f,
                iVar1.ptr<float>(iOffset1),
                iVar2.ptr<float>(iOffset2),
                0.0f,
                oVar.ptr<float>(iOffsetO)
            );
            break;
        case TYPE_DOUBLE:
            blas::gemm(
                iVar1.shape(0), iVar2.shape(1), iVar1.shape(1),
                1.0,
                iVar1.ptr<double>(iOffset1),
                iVar2.ptr<double>(iOffset2),
                0.0,
                oVar.ptr<double>(iOffsetO)
            );
            break;
        default:
            throw error("Dot::vector: Unknown type");
        }
    }
    else
        throw error("Dot::vector: Dimension > 2");
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
        throw error("ASum::vector: Unknown type");
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
        throw error("Sum::vector: Unknown type");
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
        r = view(s, cdouble(0.0, 0.0));
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


var Concatenate::alloc(var iVar) const
{
    // Basically check that the shapes match
    int dim = iVar[0].dim();
    int sum = iVar[0].shape(-1);
    for (int v=1; v<iVar.size(); v++)
    {
        for (int d=0; d<dim-1; d++)
            if (iVar[v].shape(d) != iVar[0].shape(d))
                throw error("Concatenate: dimensions differ");
        // ...and sum the low dimensions
        sum += iVar[v].shape(-1);
    }

    // Allocate a new shape with the summed low dimension
    var s = iVar[0].shape();
    s[dim-1] = sum;
    var r = view(s, iVar[0].at(0));
    return r;
}


void Concatenate::vector(var iVar, var& oVar) const
{
    int sum = 0;
    switch(oVar.atype())
    {
    case TYPE_FLOAT:
        for (int i=0; i<iVar.size(); i++)
        {
            assert(!iVar[i].is(oVar));
            int s = iVar[i].shape(-1);
            blas::copy(
                s, iVar[i].ptr<float>(), oVar.ptr<float>(sum)
            );
            sum += s;
        }
        break;
    case TYPE_DOUBLE:
        for (int i=0; i<iVar.size(); i++)
        {
            assert(!iVar[i].is(oVar));
            int s = iVar[i].shape(-1);
            blas::copy(
                s, iVar[i].ptr<double>(), oVar.ptr<double>(sum)
            );
            sum += s;
        }
        break;
    case TYPE_CFLOAT:
        for (int i=0; i<iVar.size(); i++)
        {
            assert(!iVar[i].is(oVar));
            int s = iVar[i].shape(-1);
            blas::copy(
                s, iVar[i].ptr<cfloat>(), oVar.ptr<cfloat>(sum)
            );
            sum += s;
        }
        break;
    case TYPE_CDOUBLE:
        for (int i=0; i<iVar.size(); i++)
        {
            assert(!iVar[i].is(oVar));
            int s = iVar[i].shape(-1);
            blas::copy(
                s, iVar[i].ptr<cdouble>(), oVar.ptr<cdouble>(sum)
            );
            sum += s;
        }
        break;
    default:
        throw error("Concatenate::vector: Unknown type");
    }
}
