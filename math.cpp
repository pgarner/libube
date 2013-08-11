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
