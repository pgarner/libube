/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2014
 */

/*
 * Take care here; there is a boost thing called path.  In this sense, there is
 * the possibility that one could be confused as the other.  Of course, it
 * boils down to being careful with namespaces.
 */

#include "lube/path.h"
#include "boost/filesystem/operations.hpp"

namespace fs = boost::filesystem;

namespace libube
{
    /** Concrete implementation of path module */
    class Path : public path
    {
    public:
        Path(var iArg);
        var dir(bool iVal);
        var rdir(bool iVal);
        var tree();
    private:
        fs::path mPath;
        var tree(fs::path iPath);
    };

    /** Factory function to create a class */
    void factory(Module** oModule, var iArg)
    {
        *oModule = new Path(iArg);
    }
};

using namespace libube;

Path::Path(var iArg)
{
    mPath = iArg ? iArg.str() : fs::initial_path();
}

var bits(fs::path iPath)
{
    var val;
    val[2] = iPath.extension().c_str();
    val[1] = iPath.stem().c_str();
    val[0] = iPath.parent_path().c_str();
    return val;
}

var Path::dir(bool iVal)
{
    if (!exists(mPath))
        throw error("dir: path doesn't exist");

    var dir;
    fs::directory_iterator end;
    for (fs::directory_iterator it(mPath); it != end; it++)
        dir[it->path().c_str()] = iVal ? bits(it->path()) : nil;
    return dir;
}

var Path::rdir(bool iVal)
{
    if (!exists(mPath))
        throw error("rdir: path doesn't exist");

    var dir;
    if (!fs::is_directory(mPath))
    {
        dir[mPath.c_str()] = iVal ? bits(mPath) : nil;
        return dir;
    }

    fs::recursive_directory_iterator end;
    for (fs::recursive_directory_iterator it(mPath); it != end; it++)
        dir[it->path().c_str()] = iVal ? bits(it->path()) : nil;
    return dir;
}

var Path::tree()
{
    return tree(mPath);
}

var Path::tree(boost::filesystem::path iPath)
{
    if (!exists(iPath))
        throw error("tree: path doesn't exist");

    var dir;
    fs::directory_iterator end;
    for (fs::directory_iterator it(iPath); it != end; it++)
        if (fs::is_directory(*it))
            dir[it->path().filename().c_str()] = tree(*it);
        else
            dir[it->path().filename().c_str()] = nil;

    return dir;
}

