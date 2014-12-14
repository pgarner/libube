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

#include <var.h>
#include "boost/filesystem/operations.hpp"

namespace libvar
{
    class path
    {
    public:
        path(var iPath);
        var dir();
        var rdir();
        var tree();
    private:
        boost::filesystem::path mPath;
        var tree(boost::filesystem::path iPath);
    };
};

#endif // PATH_H
