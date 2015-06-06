/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2014
 */

#include <cassert>
#include "lube/var.h"
#include "lube/heap.h" // for the view transpose

namespace libube
{
    // The instantiation of the class
    Transpose transpose;
}

using namespace libube;

var Transpose::alloc(var iVar) const
{
    // Allocate an output array
    var r;
    var s = iVar.shape();
    int dim = s.size();
    if (dim < 2)
        throw error("Transpose::alloc: dimension < 2");

    // This is a gotcha: Without the dereference(), s[dim-1] is an lvalue so
    // tmp will use the move semantic.  This means it will remain a reference;
    // this swap is exactly when you don't want it to be one.  The other option
    // is use get<int>(), which may even be quicker.
    var tmp = s[dim-1].dereference();
    s[dim-1] = s[dim-2];
    s[dim-2] = tmp;
    r = view(s, iVar.at(0));
    return r;
}

void Transpose::scalar(const var& iVar, var& oVar) const
{
    // Note that we need to transpose the view too.  If it's allocated, it's
    // transposed at allocation; if it's supplied it's assumed to be correct
    // already.  This leaves the in-place case; this happens after the
    // operation.

    // Transpose always broadcasts to array()
    broadcast(iVar, oVar);

    // Transpose view if necessary
    if (iVar.is(oVar))
    {
        // Swap the trailing dimensions
        int d = iVar.dim();
        varheap* h = iVar.heap();
        int& str1 = h->stride(d-1);
        int& shp1 = h->shape(d-1);
        int& str2 = h->stride(d-2);
        int& shp2 = h->shape(d-2);
        int tmp = shp1;
        shp1 = shp2;
        shp2 = tmp;
        str2 = shp1 * str1;
    }
}

/*
 * Out of place transpose.  Conceptually really simple, but I don't know
 * whether accessing the source or target in order is better.
 */
template <class T>
static void outOfPlace(T* iData, int iRows, int iCols, T* oData)
{
    for (int r=0; r<iRows; r++)
        for (int c=0; c<iCols; c++)
            oData[c*iRows+r] = iData[r*iCols+c];
}

/*
 * In place transpose.  This is actually a really tricky thing to do.  The one
 * here is not far from the right answer as it doesn't allocate extra memory.
 * It may be slow, but hey.  Just copied from
 * http://www.cheshirekow.com/wordpress/?p=384
 */
template <class T>
static void inPlace(T* iData, int iRows, int iCols)
{
    using std::swap; 
    int i, j, k, k_start, k_new;
    int m = iRows;
    int n = iCols;
    int length = iRows*iCols;
    for(k_start=1; k_start < length; k_start++)
    {
        T temp = iData[k_start];
        bool abort = false;
        k_new = k = k_start;
 
        // go through the cycle once and ensure that the starting point is
        // minimal
        do
        {
            if( k_new < k_start )
            {
                abort = true;
                break;
            }
 
            k = k_new;
            i = k % n;
            j = k / n;
            k_new = i*m + j;
        } while(k_new != k_start);
 
 
        // if the current value is not the minimum of the cycle, then don't
        // perform the cycle
        if(abort)
            continue;
 
        // otherwise, perform the cycle
        k_new = k = k_start;
        do
        {
            swap<T>(iData[k_new], temp);
 
            k = k_new;
            i = k % n;
            j = k / n;
            k_new = i*m + j;
        } while(k_new != k_start);
        swap<T>(iData[k_new], temp);
    }
}

void Transpose::vector(var iVar, ind iOffsetI, var& oVar, ind iOffsetO) const
{
    assert(oVar);
    int dim = iVar.dim();
    ind rows = iVar.shape(dim-2);
    ind cols = iVar.shape(dim-1);
    if (iVar.is(oVar))
        // In place transpose
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
            inPlace<float>(oVar.ptr<float>(iOffsetO), rows, cols);
            break;
        case TYPE_DOUBLE:
            inPlace<double>(oVar.ptr<double>(iOffsetO), rows, cols);
            break;
        default:
            throw error("Transpose::vector(): unknown type");            
        }
    else
        // Transpose to new location
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
            outOfPlace<float>(iVar.ptr<float>(iOffsetI), rows, cols,
                              oVar.ptr<float>(iOffsetO));
            break;
        case TYPE_DOUBLE:
            outOfPlace<double>(iVar.ptr<double>(iOffsetI), rows, cols,
                               oVar.ptr<double>(iOffsetO));
            break;
        default:
            throw error("Transpose::vector(): unknown type");
        }
}
