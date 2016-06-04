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

#include <lube/module.h>

namespace libube
{
    /** Virtual interface to path module */
    class path : public Module
    {
    public:
        virtual var dir(bool iVal=false) = 0;
        virtual var rdir(bool iVal=false) = 0;
        virtual var tree() = 0;
    };

    /** Module specialised to generate paths */
    class pathmodule : public module
    {
    public:
        pathmodule() : module("path") {}
        path& create(var iArg=nil) {
            return dynamic_cast<path&>(module::create(iArg));
        }
    };
};

#endif // PATH_H
