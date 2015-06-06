/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2014
 */

#ifndef PATH_H
#define PATH_H

#include <lube/var.h>

namespace libube
{
    /** Virtual interface to path module */
    class path : public Module
    {
    public:
        virtual var dir() = 0;
        virtual var rdir() = 0;
        virtual var tree() = 0;
    };

    /** Helper function to create path modules */
    path* create(module& iMod, var iArg=nil) {
        return dynamic_cast<path*>(iMod.create(iArg));
    };
};

#endif // PATH_H
