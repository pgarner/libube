/*
 * Copyright 2014 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, December 2014
 */

#include <lv.h>
#include <path.h>

int main()
{
    lv::path p("CMakeFiles");

    var dir = p.dir();    
    std::cout << dir << std::endl;

    var rdir = p.rdir();    
    std::cout << rdir << std::endl;

    var tree = p.tree();
    std::cout << tree << std::endl;

    return 0;
}
