/*
 * Copyright 2016 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, October 2016
 */

#ifndef LUBE_QWT_H
#define LUBE_QWT_H

#include <lube/module.h>

namespace libube
{
    /** Virtual interface to qwt module */
    class qwt : public Module
    {
    public:
        virtual void exec() = 0;
        virtual void plot() = 0;
        virtual void curve(var iX, var iY, var iTitle="") = 0;
        virtual void axes(var iXLabel, var iYLabel) = 0;
    };

    /** Module specialised to generate qwt */
    class qwtmodule : public module
    {
    public:
        qwtmodule() : module("qwt") {}
        qwt& create(var iArg=nil) {
            return dynamic_cast<qwt&>(module::create(iArg));
        }
    };
};

#endif // LUBE_QWT_H
