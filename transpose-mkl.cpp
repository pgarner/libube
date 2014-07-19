/*
 * Copyright 2014 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, July 2014
 */

#include <cassert>
#include <mkl.h>
#include "var.h"

namespace libvar
{
    Transpose transpose;
}

using namespace libvar;

var Transpose::operator ()(const var& iVar, var* oVar) const
{
    var r;
    if (!oVar)
    {
        // Allocate an output array
        var s = iVar.shape();
        int dim = s.size();
        if (dim < 2)
            throw std::runtime_error("Transpose::operator(): dimension < 2");
        var tmp = s[dim-1];
        s[dim-1] = s[dim-2];
        s[dim-2] = tmp;
        r = view(s, iVar.at(0));
        oVar = &r;
    }

    // Transpose always broadcasts to array()
    broadcast(iVar, oVar);
    return *oVar;
}


void Transpose::array(var iVar, var* oVar, int iIndex) const
{
    assert(oVar);
    int iVarOffset = iVar.stride(iVar.dim()-mDim-1) * iIndex;
    int oVarOffset = oVar->stride(oVar->dim()-mDim-1) * iIndex;
    int dim = iVar.dim();
    ind rows = iVar.shape(dim-2);
    ind cols = iVar.shape(dim-1);
    if (iVar.is(*oVar))
        // In place transpose
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
            mkl_simatcopy (
                'r', 't', rows, cols, 1.0f,
                oVar->ptr<float>(oVarOffset), cols, rows
            );
            break;
        default:
            throw std::runtime_error("Transpose::array(): unknown type");            
        }
    else
        // Transpose to new location
        switch (iVar.atype())
        {
        case TYPE_FLOAT:
            mkl_somatcopy (
                'r', 't', rows, cols, 1.0f,
                iVar.ptr<float>(iVarOffset), cols,
                oVar->ptr<float>(oVarOffset), rows
            );
            break;
        default:
            throw std::runtime_error("Transpose::array(): unknown type");            
        }
}
