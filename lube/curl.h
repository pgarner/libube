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

#include <lube/var.h>

namespace libube
{
    /** Virtual interface to curl module */
    class curl : public Module
    {
    public:
        virtual var transfer(var iURL) = 0;
    };

    /** Helper function to create curl modules */
    curl* create(module& iMod, var iArg=nil) {
        return dynamic_cast<curl*>(iMod.create(iArg));
    };
};

#endif // CURL_H
