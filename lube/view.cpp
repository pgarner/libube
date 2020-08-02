/*
 * Copyright 2018 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, November 2018
 */

#include <cassert>

#include "lube/var.h"
#include "lube/heap.h"

using namespace libube;


/**
 * Copy constructor
 *
 * Like the var version, it is non-recursive; it copies the array, but
 * deeper arrays are not copied.  Just the reference counts are
 * implicitly bumped.  However, if it's a view then the heap of which
 * it's a view is copied.
 */
View::View(const IHeap& iHeap, bool iAllocOnly) : Heap()
{
    assert(iHeap.view());
    mType = TYPE_INT;
    resize(iHeap.dim()*2+1);
    mHeap = new Heap(*iHeap.view(), iAllocOnly);
    mHeap->attach();
    copy(reinterpret_cast<const Heap*>(&iHeap), mSize);
}

/* This is a constructor, not a copy constructor */
View::View(IHeap* iHeap) : Heap()
{
    // If we're given a view, bypass it to get to the underlying heap
    mHeap = iHeap->view() ? iHeap->view() : reinterpret_cast<Heap*>(iHeap);
    mHeap->attach();

    // Initialise the offset with the old one if it was a view
    mType = TYPE_INT;
    resize(1);
    mData.ip[0] = iHeap->view() ? iHeap->offset() : 0;
}


/* Common parts of the view() initialiser methods */
void View::setStrides(int iDim)
{
    if (iDim < 1)
        throw error("View::setStrides(): view must have dim > 0");

    // The second of each pair is the stride
    int p = 1;
    for (int i=iDim-1; i>=0; i--)
    {
        mData.ip[i*2+2] = p;
        p *= mData.ip[i*2+1];
    }

    // the p that drops out should be the overall size
    if (p+mData.ip[0] > mHeap->size())
        throw error("View::setStrides(): Array too small for view");
}


View::View(IHeap* iHeap, const std::initializer_list<int> iList, int iOffset)
    : View(iHeap)
{
    int dim = iList.size();
    resize(dim*2+1);

    // The first entry is the offset
    mData.ip[0] += iOffset;;

    // First entry of each subsequent pair is the dimension
    int i = 0;
    for (const int* it=begin(iList); it!=end(iList); ++it)
    {
        mData.ip[i*2+1] = *it;
        mData.ip[i*2+2] = 0;
        i++;
    }

    // The second of each pair is the stride
    setStrides(dim);
}


View::View(IHeap* iHeap, var iShape, int iOffset) : View(iHeap)
{
    int dim = iShape.size();
    resize(dim*2+1);

    // The first entry is the offset
    mData.ip[0] += iOffset;;

    // First entry of each subsequent pair is the dimension
    for (int i=0; i<dim; i++)
    {
        mData.ip[i*2+1] = iShape[i].get<int>();
        mData.ip[i*2+2] = 0;
    }

    // The second of each pair is the stride
    setStrides(dim);
}


View::~View()
{
    mHeap->detach();
    mHeap = 0;
}


int View::offset(int iOffset)
{
    if (iOffset + size() > mHeap->size())
        throw error("View::offset(): offset too large");
    mData.ip[0] = iOffset;
    return iOffset;
}


int& View::shape(int iDim) const
{
    int index = iDim*2 + 1;
    if ((index < 0) || (index >= mSize))
        throw std::range_error("View::shape(): dimension out of bounds");
    return mData.ip[index];
}


int& View::stride(int iDim) const
{
    int index = iDim*2 + 2;
    if ((index < 0) || (index >= mSize))
        throw std::range_error("View::stride(): dimension out of bounds");
    return mData.ip[index];
}


/**
 * Normally, x = y will copy the var such that both x and y point to
 * the same Heap.  However, if x is a view, we want the data from y
 * to be copied into the thing of which x is a view.  This checks
 * whether that copy is possible, i.e., the types and sizes match.
 */
bool View::copyable(IHeap* iHeap)
{
    if (!iHeap)
        return false;
    if (type() != iHeap->type())
        return false;
    if (dim() != iHeap->dim())
        return false;
    if (dim() == 1)
        return (size() == iHeap->size());
    for (int i=0; i<dim(); i++)
        if (shape(i) != iHeap->shape(i))
            return false;
    return true;
}
