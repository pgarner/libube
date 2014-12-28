/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2014
 */

#include <lv.h>
#include <varpath.h>

int main()
{
    lv::module m("path");
    lv::path* p = lv::create(m, "cmake");

    std::cout << "Dir:" << std::endl;
    var dir = p->dir();
    std::cout << dir << std::endl;

    std::cout << "Recursive Dir:" << std::endl;
    var rdir = p->rdir();
    std::cout << rdir << std::endl;

    std::cout << "Tree:" << std::endl;
    var tree = p->tree();
    std::cout << tree << std::endl;

    return 0;
}
