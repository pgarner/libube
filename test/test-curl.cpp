/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2016
 */

#include "lube/lube.h"
#include "lube/curl.h"

using namespace lube;

int main(int argc, char** argv)
{
    curlmodule cm;
    curl& c = cm.create();
    var d = c.transfer("https://raw.githubusercontent.com/pgarner/libube/master/test/test-curl.cmake");
    std::cout << d.str();
}
