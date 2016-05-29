/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2015
 */

#ifndef GRAPH_H
#define GRAPH_H

#include <lube/var.h>

namespace libube
{
    /** Virtual interface to graph module */
    class graph : public Module
    {
    public:
        virtual void addEdge(ind iVertex1, ind iVertex2) = 0;
        virtual ind addVertex() = 0;
        virtual void writeGraphViz(var iFileName) = 0;
     };

    /** Module specialised to generate graphs */
    class graphmodule : public module
    {
    public:
        graphmodule() : module("graph") {}
        graph* create(var iArg=nil) {
            return dynamic_cast<graph*>(module::create(iArg));
        }
    };
};

#endif // GRAPH_H
