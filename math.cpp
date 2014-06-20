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


namespace libvar
{
    /*
     * The instantiations of the math and standard functors defined in this
     * module.  They are declared static in the var class definition.
     */
    Abs abs;
    Sin sin;
    Cos cos;
    Tan tan;
    Floor floor;
    Sqrt sqrt;
    Log log;

    Pow pow;
    Set set;
    Add add;
    Sub sub;
    Mul mul;
    Div div;
    ASum asum;
}


using namespace libvar;


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
    void   ssbmv_ (
        char *, int *, int *, float  *, float  *, int *,
        float  *, int *, float  *, float  *, int *
    );
    void   dsbmv_ (
        char *, int *, int *, double *, double *, int *,
        double *, int *, double *, double *, int *
    );
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
ind type(var iVar)
{
    ind type = iVar.type();
    if ((type == var::TYPE_ARRAY) &&
        (iVar.heap()->type() == var::TYPE_CDOUBLE))
        type = var::TYPE_CDOUBLE;
    return type;
}


void UnaryFunctor::array(var iVar, var* oVar, int iOffset) const
{
    throw std::runtime_error("UnaryFunctor: not an array operation");
}


void BinaryFunctor::array(var iVar1, var iVar2, var* oVar, int iOffset) const
{
    throw std::runtime_error("BinaryFunctor: not an array operation");
}


/**
 * Unary broadcaster
 *
 * Broadcasts the operation against iVar.
 */
void UnaryFunctor::broadcast(var iVar, var* oVar) const
{
    int iDim = iVar.dim();

    // Case 1: Operation is dimensionless
    // Call back to the unary operator
    if (mDim == 0)
    {
        // This could be parallel!
        for (int i=0; i<iVar.size(); i++)
        {
            var ref = oVar->at(i);
            operator ()(iVar.at(i), &ref);
        }
        return;
    }

    // Case 2: array operation (mDim is at least 1)
    // Check that the array is broadcastable
    if (mDim > iDim)
        throw std::runtime_error("var::broadcast: op dimension too large");

    // If it didn't throw, then the array is broadcastable
    // In this case, loop over iVar with different indexes
    int s = iVar.size() / iVar.stride(iDim-mDim-1);
    for (int i=0; i<s; i++)
        array(iVar, oVar, i);
}


/**
 * Binary broadcaster
 *
 * Broadcasts iVar2 against iVar1; i.e., iVar1 is the lvalue and iVar2
 * is the rvalue.
 */
void BinaryFunctor::broadcast(var iVar1, var iVar2, var* oVar) const
{
    int dim1 = iVar1.dim();
    int dim2 = iVar2.dim();

    // Case 1: iVar2 has size 1
    // Call back to the unary operator
    if ((dim2 == 1) && (iVar2.size() == 1))
    {
        // This could be parallel!
        for (int i=0; i<iVar1.size(); i++)
            oVar->at(i) = operator ()(iVar1.at(i), iVar2);
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
        array(iVar1, iVar2, oVar, i);
}


#define CMATH_UNARY_FUNCTOR(F,f)                                        \
    var F::operator ()(const var& iVar, var* oVar) const                \
    {                                                                   \
        var r;                                                          \
        if (!oVar)                                                      \
        {                                                               \
            r = iVar.copy(true);                                        \
            oVar = &r;                                                  \
        }                                                               \
                                                                        \
        switch (type(iVar))                                             \
        {                                                               \
        case var::TYPE_ARRAY:                                           \
            broadcast(iVar, oVar);                                      \
            break;                                                      \
        case var::TYPE_FLOAT:                                           \
            *oVar = std::f(iVar.get<float>());                          \
            break;                                                      \
        case var::TYPE_DOUBLE:                                          \
            *oVar = std::f(iVar.get<double>());                         \
            break;                                                      \
        default:                                                        \
            throw std::runtime_error("#F##::operator(): Unknown type"); \
        }                                                               \
                                                                        \
        return *oVar;                                                   \
    }

#define COMPLEX_UNARY_FUNCTOR(F,f)                                      \
    var F::operator ()(const var& iVar, var* oVar) const                \
    {                                                                   \
        var r;                                                          \
        if (!oVar)                                                      \
        {                                                               \
            r = iVar.copy(true);                                        \
            oVar = &r;                                                  \
        }                                                               \
                                                                        \
        switch (type(iVar))                                             \
        {                                                               \
        case var::TYPE_ARRAY:                                           \
            broadcast(iVar, oVar);                                      \
            break;                                                      \
        case var::TYPE_FLOAT:                                           \
            *oVar = std::f(iVar.get<float>());                          \
            break;                                                      \
        case var::TYPE_DOUBLE:                                          \
            *oVar = std::f(iVar.get<double>());                         \
            break;                                                      \
        case var::TYPE_CFLOAT:                                          \
            *oVar = std::f(iVar.get<cfloat>());                         \
            break;                                                      \
        case var::TYPE_CDOUBLE:                                         \
            *oVar = std::f(iVar.get<cdouble>());                        \
            break;                                                      \
        default:                                                        \
            throw std::runtime_error("#F##::operator(): Unknown type"); \
        }                                                               \
                                                                        \
        return *oVar;                                                   \
    }


CMATH_UNARY_FUNCTOR(Floor,floor)
COMPLEX_UNARY_FUNCTOR(Sin,sin)
COMPLEX_UNARY_FUNCTOR(Cos,cos)
COMPLEX_UNARY_FUNCTOR(Tan,tan)
COMPLEX_UNARY_FUNCTOR(Sqrt,sqrt)
COMPLEX_UNARY_FUNCTOR(Log,log)


var Pow::operator ()(const var& iVar1, const var& iVar2, var* oVar) const
{
    var r;
    if (!oVar)
    {
        r = iVar1.copy(true);
        oVar = &r;
    }

    switch(type(iVar1))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case var::TYPE_FLOAT:
        *oVar = std::pow(iVar1.get<float>(), iVar2.cast<float>());
        break;
    case var::TYPE_DOUBLE:
        *oVar = std::pow(iVar1.get<double>(), iVar2.cast<double>());
        break;
    case var::TYPE_CFLOAT:
        *oVar = std::pow(iVar1.get<cfloat>(), iVar2.cast<cfloat>());
        break;
    case var::TYPE_CDOUBLE:
        *oVar = std::pow(iVar1.get<cdouble>(), iVar2.cast<cdouble>());
        break;
    default:
        throw std::runtime_error("Pow::operator(): Unknown type");
    }

    return *oVar;
}


var Abs::operator ()(const var& iVar, var* oVar) const
{
    // Not a COMPLEX_UNARY_FUNCTOR as it takes real or complex but always
    // returns real.
    var r;
    if (!oVar)
    {
        // This probably ought to be a helper function.  The idea is to copy
        // iVar, but change the type to the real version.
        var s = iVar.shape();
        switch (iVar.atype())
        {
        case var::TYPE_FLOAT:
        case var::TYPE_CFLOAT:
            r = view(s, 0.0f);
            break;
        case var::TYPE_DOUBLE:
        case var::TYPE_CDOUBLE:
            r = view(s, 0.0);
            break;
        }
        oVar = &r;
    }

    switch (type(iVar))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case var::TYPE_FLOAT:
        *oVar = std::abs(iVar.get<float>());
        break;
    case var::TYPE_DOUBLE:
        *oVar = std::abs(iVar.get<double>());
        break;
    case var::TYPE_CFLOAT:
        *oVar = std::abs(iVar.get<cfloat>());
        break;
    case var::TYPE_CDOUBLE:
        *oVar = std::abs(iVar.get<cdouble>());
        break;
    default:
        throw std::runtime_error("Abs::operator(): Unknown type");
    }

    return *oVar;
}


var Set::operator ()(const var& iVar1, const var& iVar2, var* oVar) const
{
    var r;
    if (!oVar)
    {
        r = iVar1.copy(true);
        oVar = &r;
    }

    switch(type(*oVar))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case var::TYPE_CHAR:
        *oVar->ptr<char>() = iVar2.cast<char>();
        break;
    case var::TYPE_INT:
        *oVar->ptr<int>() = iVar2.cast<int>();
        break;
    case var::TYPE_LONG:
        *oVar->ptr<long>() = iVar2.cast<long>();
        break;
    case var::TYPE_FLOAT:
        *oVar->ptr<float>() = iVar2.cast<float>();
        break;
    case var::TYPE_DOUBLE:
        *oVar->ptr<double>() = iVar2.cast<double>();
        break;
    case var::TYPE_CFLOAT:
        *oVar->ptr<cfloat>() = iVar2.cast<cfloat>();
        break;
    case var::TYPE_CDOUBLE:
        *oVar->ptr<cdouble>() = iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Set::operator(): Unknown type");
    }

    return *oVar;
}


void Set::array(var iVar1, var iVar2, var* oVar, int iOffset) const
{
    assert(type(iVar1) == var::TYPE_ARRAY);
    static int one = 1;
    int size = iVar2.size();
    switch(iVar1.heap()->type())
    {
    case var::TYPE_FLOAT:
    {
        float* x = iVar2.ptr<float>();
        float* y = iVar1.ptr<float>()+iOffset;
        scopy_(&size, x, &one, y, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        double* x = iVar2.ptr<double>();
        double* y = iVar1.ptr<double>()+iOffset;
        dcopy_(&size, x, &one, y, &one);
        break;
    }
    default:
        throw std::runtime_error("Set::array: Unknown type");
    }
}


var Add::operator ()(const var& iVar1, const var& iVar2, var* oVar) const
{
    var r;
    if (!oVar)
    {
        r = iVar1.copy(true);
        oVar = &r;
    }

    switch(type(iVar1))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case var::TYPE_CHAR:
        *oVar->ptr<char>() = iVar1.get<char>() + iVar2.cast<char>();
        break;
    case var::TYPE_INT:
        *oVar->ptr<int>() = iVar1.get<int>() + iVar2.cast<int>();
        break;
    case var::TYPE_LONG:
        *oVar->ptr<long>() = iVar1.get<long>() + iVar2.cast<long>();
        break;
    case var::TYPE_FLOAT:
        *oVar->ptr<float>() = iVar1.get<float>() + iVar2.cast<float>();
        break;
    case var::TYPE_DOUBLE:
        *oVar->ptr<double>() = iVar1.get<double>() + iVar2.cast<double>();
        break;
    case var::TYPE_CFLOAT:
        *oVar->ptr<cfloat>() = iVar1.get<cfloat>() + iVar2.cast<cfloat>();
        break;
    case var::TYPE_CDOUBLE:
        *oVar->ptr<cdouble>() = iVar1.get<cdouble>() + iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Add::operator(): Unknown type");
    }

    return *oVar;
}


void Add::array(var iVar1, var iVar2, var* oVar, int iOffset) const
{
    assert(type(iVar1) == var::TYPE_ARRAY);
    assert(iVar1.is(*oVar));
    static int one = 1;
    int size = iVar2.size();
    switch(iVar1.heap()->type())
    {
    case var::TYPE_FLOAT:
    {
        static float alpha = 1.0f;
        float* x = iVar2.ptr<float>();
        float* y = iVar1.ptr<float>()+iOffset;
        saxpy_(&size, &alpha, x, &one, y, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = 1.0;
        double* x = iVar2.ptr<double>();
        double* y = iVar1.ptr<double>()+iOffset;
        daxpy_(&size, &alpha, x, &one, y, &one);
        break;
    }
    default:
        throw std::runtime_error("Add::array: Unknown type");
    }
}


var Sub::operator ()(const var& iVar1, const var& iVar2, var* oVar) const
{
    var r;
    if (!oVar)
    {
        r = iVar1.copy(true);
        oVar = &r;
    }

    switch(type(iVar1))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case var::TYPE_CHAR:
        *oVar->ptr<char>() = iVar1.get<char>() - iVar2.cast<char>();
        break;
    case var::TYPE_INT:
        *oVar->ptr<int>() = iVar1.get<int>() - iVar2.cast<int>();
        break;
    case var::TYPE_LONG:
        *oVar->ptr<long>() = iVar1.get<long>() - iVar2.cast<long>();
        break;
    case var::TYPE_FLOAT:
        *oVar->ptr<float>() = iVar1.get<float>() - iVar2.cast<float>();
        break;
    case var::TYPE_DOUBLE:
        *oVar->ptr<double>() = iVar1.get<double>() - iVar2.cast<double>();
        break;
    case var::TYPE_CFLOAT:
        *oVar->ptr<cfloat>() = iVar1.get<cfloat>() - iVar2.cast<cfloat>();
        break;
    case var::TYPE_CDOUBLE:
        *oVar->ptr<cdouble>() = iVar1.get<cdouble>() - iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Sub::operator(): Unknown type");
    }

    return *oVar;
}


void Sub::array(var iVar1, var iVar2, var* oVar, int iOffset) const
{
    assert(type(iVar1) == var::TYPE_ARRAY);
    assert(iVar1.is(*oVar));
    static int one = 1;
    int size = iVar2.size();
    switch(iVar1.heap()->type())
    {
    case var::TYPE_FLOAT:
    {
        static float alpha = -1.0f;
        float* x = iVar2.ptr<float>();
        float* y = iVar1.ptr<float>()+iOffset;
        saxpy_(&size, &alpha, x, &one, y, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = -1.0;
        double* x = iVar2.ptr<double>();
        double* y = iVar1.ptr<double>()+iOffset;
        daxpy_(&size, &alpha, x, &one, y, &one);
        break;
    }
    default:
        throw std::runtime_error("Sub::array: Unknown type");
    }
}


/**
 * Overload of broadcast() for multiplication.  This catches the case where
 * just scaling is being done, meaning a different BLAS call is necessary.
 */
void Mul::broadcast(var iVar1, var iVar2, var* oVar) const
{
    // If iVar2 has size 1, call scale rather than let the base class broadcast
    // it over the unary operator.
    if ((iVar2.dim() == 1) && (iVar2.size() == 1))
    {
        scale(iVar1, iVar2, oVar, 0);
        return;
    }
    else
        BinaryFunctor::broadcast(iVar1, iVar2, oVar);
}


void Mul::scale(var iVar1, var iVar2, var* oVar, int iOffset) const
{
    assert(type(iVar1) == var::TYPE_ARRAY);
    static int one = 1;
    int size = iVar1.size();
    switch(iVar1.heap()->type())
    {
    case var::TYPE_FLOAT:
    {
        float alpha = iVar2.cast<float>();
        float* x = iVar1.ptr<float>()+iOffset;
        sscal_(&size, &alpha, x, &one);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        double alpha = iVar2.cast<double>();
        double* x = iVar1.ptr<double>()+iOffset;
        dscal_(&size, &alpha, x, &one);
        break;
    }
    default:
        throw std::runtime_error("Mul::scal: Unknown type");
    }
}


var Mul::operator ()(const var& iVar1, const var& iVar2, var* oVar) const
{
    var r;
    if (!oVar)
    {
        r = iVar1.copy(true);
        oVar = &r;
    }

    switch(type(iVar1))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case var::TYPE_CHAR:
        *oVar->ptr<char>() = iVar1.get<char>() * iVar2.cast<char>();
        break;
    case var::TYPE_INT:
        *oVar->ptr<int>() = iVar1.get<int>() * iVar2.cast<int>();
        break;
    case var::TYPE_LONG:
        *oVar->ptr<long>() = iVar1.get<long>() * iVar2.cast<long>();
        break;
    case var::TYPE_FLOAT:
        *oVar->ptr<float>() = iVar1.get<float>() * iVar2.cast<float>();
        break;
    case var::TYPE_DOUBLE:
        *oVar->ptr<double>() = iVar1.get<double>() * iVar2.cast<double>();
        break;
    case var::TYPE_CFLOAT:
        *oVar->ptr<cfloat>() = iVar1.get<cfloat>() * iVar2.cast<cfloat>();
        break;
    case var::TYPE_CDOUBLE:
        *oVar->ptr<cdouble>() = iVar1.get<cdouble>() * iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Mul::operator(): Unknown type");
    }

    return *oVar;
}


void Mul::array(var iVar1, var iVar2, var* oVar, int iOffset) const
{
    // Elementwise multiplication is actually multiplication by a diagonal
    // matrix.  In BLAS speak, a diagonal matrix is a band matrix with no
    // superdiagonals.
    assert(type(iVar1) == var::TYPE_ARRAY);
    static int zero = 0;
    static int one = 1;
    static char uplo = 'U';
    int size = iVar2.size();
    if (iVar1.is(*oVar))
    {
        // xtbmv() (triangular band) overwrites the current location.
        static char trans = 'T';
        static char diag = 'N';
        switch(iVar1.heap()->type())
        {
        case var::TYPE_FLOAT:
        {
            float* A = iVar2.ptr<float>();
            float* x = iVar1.ptr<float>()+iOffset;
            stbmv_(&uplo, &trans, &diag, &size, &zero, A, &one, x, &one);
            break;
        }
        case var::TYPE_DOUBLE:
        {
            double* A = iVar2.ptr<double>();
            double* x = iVar1.ptr<double>()+iOffset;
            dtbmv_(&uplo, &trans, &diag, &size, &zero, A, &one, x, &one);
            break;
        }
        default:
            throw std::runtime_error("Mul::array: Unknown type");
        }
    }
    else
    {
        // xsbmv() (symmetric band) puts the result in a new location.
        switch(iVar1.heap()->type())
        {
        case var::TYPE_FLOAT:
        {
            static float alpha = 1.0f;
            static float beta = 0.0f;
            float* A = iVar2.ptr<float>();
            float* x = iVar1.ptr<float>()+iOffset;
            float* y = oVar->ptr<float>()+iOffset;
            ssbmv_(&uplo, &size, &zero,
                   &alpha, A, &one, x, &one, &beta, y, &one);
            break;
        }
        case var::TYPE_DOUBLE:
        {
            static double alpha = 1.0;
            static double beta = 0.0;
            double* A = iVar2.ptr<double>();
            double* x = iVar1.ptr<double>()+iOffset;
            double* y = oVar->ptr<double>()+iOffset;
            dsbmv_(&uplo, &size, &zero,
                   &alpha, A, &one, x, &one, &beta, y, &one);
            break;
        }
        default:
            throw std::runtime_error("Mul::array: Unknown type");
        }
    }
}


var Div::operator ()(const var& iVar1, const var& iVar2, var* oVar) const
{
    var r;
    if (!oVar)
    {
        r = iVar1.copy(true);
        oVar = &r;
    }

    switch(type(iVar1))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar1, iVar2, oVar);
        break;
    case var::TYPE_CHAR:
        *oVar->ptr<char>() = iVar1.get<char>() / iVar2.cast<char>();
        break;
    case var::TYPE_INT:
        *oVar->ptr<int>() = iVar1.get<int>() / iVar2.cast<int>();
        break;
    case var::TYPE_LONG:
        *oVar->ptr<long>() = iVar1.get<long>() / iVar2.cast<long>();
        break;
    case var::TYPE_FLOAT:
        *oVar->ptr<float>() = iVar1.get<float>() / iVar2.cast<float>();
        break;
    case var::TYPE_DOUBLE:
        *oVar->ptr<double>() = iVar1.get<double>() / iVar2.cast<double>();
        break;
    case var::TYPE_CFLOAT:
        *oVar->ptr<cfloat>() = iVar1.get<cfloat>() / iVar2.cast<cfloat>();
        break;
    case var::TYPE_CDOUBLE:
        *oVar->ptr<cdouble>() = iVar1.get<cdouble>() / iVar2.cast<cdouble>();
        break;
    default:
        throw std::runtime_error("Div::operator(): Unknown type");
    }

    return *oVar;
}


var ASum::operator ()(const var& iVar, var* oVar) const
{
    var r;
    if (!oVar)
    {
        if (iVar.dim() > 1)
        {
            // Allocate an output array
            var s = iVar.shape();
            s[s.size()-1] = 1;
            switch (iVar.atype())
            {
            case var::TYPE_FLOAT:
            case var::TYPE_CFLOAT:
                r = view(s, 0.0f);
                break;
            case var::TYPE_DOUBLE:
            case var::TYPE_CDOUBLE:
                r = view(s, 0.0);
                break;
            }
        }
        else
        {
            switch (iVar.atype())
            {
            case var::TYPE_FLOAT:
            case var::TYPE_CFLOAT:
                r = 0.0f;
                break;
            case var::TYPE_DOUBLE:
            case var::TYPE_CDOUBLE:
                r = 0.0;
                break;
            }
        }
        oVar = &r;
    }

    switch(type(iVar))
    {
    case var::TYPE_ARRAY:
        broadcast(iVar, oVar);
        break;
    case var::TYPE_FLOAT:
        *oVar->ptr<float>() = std::abs(iVar.get<float>());
        break;
    case var::TYPE_DOUBLE:
        *oVar->ptr<double>() = std::abs(iVar.get<double>());
        break;
    case var::TYPE_CFLOAT:
        *oVar->ptr<cfloat>() = std::abs(iVar.get<cfloat>());
        break;
    case var::TYPE_CDOUBLE:
        *oVar->ptr<cdouble>() = std::abs(iVar.get<cdouble>());
        break;
    default:
        throw std::runtime_error("ASum::operator(): Unknown type");
    }

    return *oVar;
}


void ASum::array(var iVar, var* oVar, int iIndex) const
{
    assert(type(iVar) == var::TYPE_ARRAY);
    static int inc = 1;
    int size = iVar.size();
    int iVarOffset = iVar.stride(iVar.dim()-mDim-1) * iIndex;
    int oVarOffset = oVar->stride(oVar->dim()-mDim-1) * iIndex;
    switch(iVar.heap()->type())
    {
    case var::TYPE_FLOAT:
        *(oVar->ptr<float>() + oVarOffset) =
            sasum_(&size, iVar.ptr<float>() + iVarOffset, &inc);
        break;
    case var::TYPE_DOUBLE:
        *(oVar->ptr<double>() + oVarOffset) =
            dasum_(&size, iVar.ptr<double>() + iVarOffset, &inc);
        break;
    default:
        throw std::runtime_error("ASum::array: Unknown type");
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
    case var::TYPE_FLOAT:
    {
        static float alpha = 1.0f;
        float* a = iHeapB->ptr<float>();
        float* b = iVarA.ptr<float>()+iOffsetA;
        float* c = ptr<float>(iOffset);
        sgemm_(&trans, &trans, m, n, k, &alpha, a, k, b, m, &alpha, c, m);
        break;
    }
    case var::TYPE_DOUBLE:
    {
        static double alpha = 1.0;
        double* a = iHeapB->ptr<double>();
        double* b = iVarA.ptr<double>()+iOffsetA;
        double* c = ptr<double>(iOffset);
        dgemm_(&trans, &trans, m, n, k, &alpha, a, k, b, m, &alpha, c, m);
        break;
    }
    default:
        throw std::runtime_error("varheap::mul: Unknown type");
    }
}
#endif
