/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2015
 */

#include "lube.h"
#include "lube/graph.h"

int main()
{
    lube::module gm("graph");
    lube::graph* g = lube::create(gm);

    ind v1 = g->addVertex();
    ind v2 = g->addVertex();
    ind v3 = g->addVertex();
    g->addEdge(v1, v2);
    g->addEdge(v1, v2);
    g->addEdge(v2, v3);
    g->addEdge(v3, v1);
    g->addEdge(v1, v3);

    std::cout << "Vertices are: "
              << v1 << " "
              << v2 << " "
              << v3 << std::endl;
    g->writeGraphViz("test.dot");

    return 0;
}
