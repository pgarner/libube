/*
 * Copyright 2015 by Philip N. Garner
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2015
 */

#include "lube/graph.h"
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>

namespace libube
{
    typedef boost::adjacency_list<
        boost::vecS,
        boost::vecS,
        boost::bidirectionalS,
        var,
        var
        >
    adjacency_list;
    typedef boost::graph_traits<adjacency_list>::vertex_descriptor vertex;
    typedef boost::graph_traits<adjacency_list>::edge_descriptor edge;

    /** Concrete implementation of graph module */
    class Graph : public graph
    {
    public:
        Graph(var iArg) {};
        void addEdge(ind iVertex1, ind iVertex2);
        ind addVertex();
        void writeGraphViz(var iFileName);
    private:
        adjacency_list mGraph;
    };

    /** Factory function to create a class */
    void factory(Module** oModule, var iArg)
    {
        *oModule = new Graph(iArg);
    }
};

using namespace libube;

void Graph::addEdge(ind iVertex1, ind iVertex2)
{
    // An edge is some non-trivial thing; it does ot cast to an integer type
    std::pair<edge, bool> ret;
    ret = add_edge(iVertex1, iVertex2, mGraph);
}

ind Graph::addVertex()
{
    // A vertex is just an index; at least, it can be cast to an ind.  So we
    // can return an ind
    vertex v = add_vertex(mGraph);
    return v;
}

class LabelWriter
{
public:
    LabelWriter(adjacency_list& iGraph) : mGraph(iGraph) {}
    template <class VertexOrEdge>
    void operator()(std::ostream& iOut, const VertexOrEdge& iV) const
    {
        var v = mGraph[iV];
        if (v.index("NAME"))
            // Should replace with escaped quote rather than nothing
            iOut << "[label=\""
                 << v.at("NAME").replace("\"", "").str()
                 << "\"]";
    }
private:
    adjacency_list& mGraph;
};

void Graph::writeGraphViz(var iFileName)
{
    std::ofstream ofs(iFileName.str(), std::ofstream::out);
    LabelWriter lw(mGraph);
    write_graphviz(ofs, mGraph, lw);
}
