/*
 * Copyright 2013 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, July 2013
 */

#include <cmath>
#include <stdexcept>

#include "var.h"
#include "varheap.h"


/*
 * In some sense it would be best to use cblas.  Netlib defines it,
 * MKL defines it, but OpenBLAS and the like don't.  Sadly, there
 * seems to be no standard header for the fortran versions.  MKL has
 * mkl.h, openblas has f77blas.h.  However, given that the interface
 * is rather standard, we can just reproduce the ones we use here.
 * These happen to be mods of OpenBLAS's f77blas.h
 */
typedef int blasint;
extern "C" {
    // Actually FORTRAN calling convention
    float  sasum_ (blasint *, float  *, blasint *);
    double dasum_ (blasint *, double *, blasint *);
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
    if (reference())
        return derefer().asum();

    var asum;
    if (mType == TYPE_ARRAY)
        asum = mData.hp->asum();
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
