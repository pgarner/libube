/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, July 2014
 */

#include <cassert>
#include <mkl.h>
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
        IHeap* h = iVar.heap();
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
            mkl_simatcopy (
                'r', 't', rows, cols, 1.0f,
                oVar.ptr<float>(iOffsetO), cols, rows
            );
            break;
        case TYPE_DOUBLE:
            mkl_dimatcopy (
                'r', 't', rows, cols, 1.0,
                oVar.ptr<double>(iOffsetO), cols, rows
            );
            break;
        default:
            throw error("Transpose::vector(): unknown type");            
        }
    else
        // Transpose to new location
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
            mkl_somatcopy (
                'r', 't', rows, cols, 1.0f,
                iVar.ptr<float>(iOffsetI), cols,
                oVar.ptr<float>(iOffsetO), rows
            );
            break;
        case TYPE_DOUBLE:
            mkl_domatcopy (
                'r', 't', rows, cols, 1.0,
                iVar.ptr<double>(iOffsetI), cols,
                oVar.ptr<double>(iOffsetO), rows
            );
            break;
        default:
            throw error("Transpose::vector(): unknown type");
        }
}
