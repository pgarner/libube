/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2014
 */

#include "var.h"
#include "varpath.h"
#include "boost/filesystem/operations.hpp"


using namespace boost::filesystem;
using namespace libvar;


namespace libvar
{
    class varpath : public VarPath
    {
    public:
        varpath();
        var dir();
        var rdir();
        var tree();
    private:
        boost::filesystem::path mPath;
        var tree(boost::filesystem::path iPath);
    };
};


void libvar::factory(Module** oModule)
{
    *oModule = new varpath;
}


varpath::varpath()
{
    mPath = initial_path();
}

var varpath::dir()
{
    if (!exists(mPath))
        throw std::runtime_error("dir: path doesn't exist");

    var dir;
    directory_iterator end;
    for (directory_iterator it(mPath); it != end; it++)
        dir[it->path().c_str()] = nil;
    
    return dir;
}

var varpath::rdir()
{
    if (!exists(mPath))
        throw std::runtime_error("dir: path doesn't exist");

    var dir;
    recursive_directory_iterator end;
    for (recursive_directory_iterator it(mPath); it != end; it++)
        dir[it->path().c_str()] = nil;
    
    return dir;
}

var varpath::tree()
{
    return tree(mPath);
}

var varpath::tree(boost::filesystem::path iPath)
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

