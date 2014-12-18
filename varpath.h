/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2014
 */

#ifndef VARPATH_H
#define VARPATH_H

#include <var.h>
#include "boost/filesystem/operations.hpp"

namespace libvar
{
    class VarPath : public Module
    {
    public:
        virtual var dir() { return nil; };
        virtual var rdir() { return nil; };
        virtual var tree() { return nil; };
    };
};

#endif // VARPATH_H
