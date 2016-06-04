/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, May 2016
 */

#ifndef CURL_H
#define CURL_H

#include <lube/module.h>

namespace libube
{
    /** Virtual interface to curl module */
    class curl : public Module
    {
    public:
        virtual var transfer(var iURL) = 0;
    };

    /** Module specialised to generate curls */
    class curlmodule : public module
    {
    public:
        curlmodule() : module("curl") {}
        curl& create(var iArg=nil) {
            return dynamic_cast<curl&>(module::create(iArg));
        }
    };
};

#endif // CURL_H
