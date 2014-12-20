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

#include "varpath.h"
#include "boost/filesystem/operations.hpp"

namespace fs = boost::filesystem;

namespace libvar
{
    /** Concrete implementation of path module */
    class Path : public path
    {
    public:
        Path(var iArg);
        var dir();
        var rdir();
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

using namespace libvar;

Path::Path(var iArg)
{
    mPath = iArg ? iArg.str() : fs::initial_path();
}

var Path::dir()
{
    if (!exists(mPath))
        throw std::runtime_error("dir: path doesn't exist");

    var dir;
    fs::directory_iterator end;
    for (fs::directory_iterator it(mPath); it != end; it++)
        dir[it->path().c_str()] = nil;
    
    return dir;
}

var Path::rdir()
{
    if (!exists(mPath))
        throw std::runtime_error("dir: path doesn't exist");

    var dir;
    fs::recursive_directory_iterator end;
    for (fs::recursive_directory_iterator it(mPath); it != end; it++)
        dir[it->path().c_str()] = nil;
    
    return dir;
}

var Path::tree()
{
    return tree(mPath);
}

var Path::tree(boost::filesystem::path iPath)
{
    if (!exists(iPath))
        throw std::runtime_error("tree: path doesn't exist");

    var dir;
    fs::directory_iterator end;
    for (fs::directory_iterator it(iPath); it != end; it++)
        if (fs::is_directory(*it))
            dir[it->path().filename().c_str()] = tree(*it);
        else
            dir[it->path().filename().c_str()] = nil;

    return dir;
}

