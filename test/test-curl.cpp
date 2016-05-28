/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2016
 */

#include <lube.h>
#include <lube/curl.h>

int main(int argc, char** argv)
{
    lube::module cm("curl");
    lube::curl* c = lube::create(cm);
    var d = c->transfer("https://curl.haxx.se/docs/copyright.html");
    std::cout << d.str() << std::endl;
}
