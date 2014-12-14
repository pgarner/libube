/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2014
 */

#include "var.h"
#include "path.h"


using namespace boost::filesystem;
using namespace libvar;

libvar::path::path(var iPath)
{
    mPath = iPath ? iPath.str() : initial_path();
}

var libvar::path::dir()
{
    if (!exists(mPath))
        throw std::runtime_error("dir: path doesn't exist");

    var dir;
    directory_iterator end;
    for (directory_iterator it(mPath); it != end; it++)
        dir[it->path().c_str()] = nil;
    
    return dir;
}

var libvar::path::rdir()
{
    if (!exists(mPath))
        throw std::runtime_error("dir: path doesn't exist");

    var dir;
    recursive_directory_iterator end;
    for (recursive_directory_iterator it(mPath); it != end; it++)
        dir[it->path().c_str()] = nil;
    
    return dir;
}

var libvar::path::tree()
{
    return tree(mPath);
}

var libvar::path::tree(boost::filesystem::path iPath)
{
    if (!exists(iPath))
        throw std::runtime_error("tree: path doesn't exist");

    var dir;
    directory_iterator end;
    for (directory_iterator it(iPath); it != end; it++)
        if (is_directory(*it))
            dir[it->path().filename().c_str()] = tree(*it);
        else
            dir[it->path().filename().c_str()] = nil;

    return dir;
}

