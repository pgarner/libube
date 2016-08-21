/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, August 2016
 */

#include "lube/func.h"
#include "lube/var.h"

using namespace libube;


/**
 * The default allocator is simply to make a copy of the input variable with
 * the same type and shape.  Only the allocation is done; data is not copied.
 */
var UnaryFunctor::alloc(var iVar) const
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
        varstream s;
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


var BinaryFunctor::alloc(var iVar1, var iVar2) const
{
    var r;
    r = iVar1.copy(true);
    return r;
}


/**
 * The normal operation of operator(iVar1, iVar2) is to allocate space then
 * call the protected method scalar().
 */
var BinaryFunctor::operator ()(
    const var& iVar1, const var& iVar2
) const
{
    var v = alloc(iVar1, iVar2);
    scalar(iVar1, iVar2, v);
    return v;
}


/**
 * The normal operation of operator(iVar1, iVar2, oVar) is to just call the
 * protected method scalar().
 */
var BinaryFunctor::operator ()(
    const var& iVar1, const var& iVar2, var& oVar
) const
{
    scalar(iVar1, iVar2, oVar);
    return oVar;
}


/**
 * The default behaviour of scalar(), hence operator(), is to broadcast.
 */
void BinaryFunctor::scalar(
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
void BinaryFunctor::vector(
    var iVar1, ind iOffset1,
    var iVar2, ind iOffset2,
    var& oVar, ind iOffsetO
) const
{
    // This uses mDim.  Should it?  As long as this isn't called, we don't need
    // mDim.  This means that dot and gemm both go through dot.
    // If mDim is not needed then the inheritance of BinaryFunctor and
    // ArithmeticFunctor can be reversed.
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
void BinaryFunctor::vector(var iVar1, var iVar2, var& oVar) const
{
    throw error("BinaryFunctor: not a vector operation");
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
#if 0
    // Does not work for matrix multiplication
    for (int i=1; i<dim2; i++)
    {
        // The dimensions should match
        if (iVar1.shape(dim1-i) != iVar2.shape(dim2-i))
            throw error("var::broadcast: dimension mismatch");
    }
#endif

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
