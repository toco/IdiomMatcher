//
// Created by Tobias Conradi on 12.09.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_GRAPH_H
#define IDIOMMATCHER_GRAPH_H

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/directed_graph.hpp>
#include <Model/Pattern.h>

namespace IdiomMatcher {
    struct GraphEdge {
        enum Type {
            Default, Data, Control
        };

        GraphEdge(const Type type = Default) : type(type) { }
        const Type type;
    };

    typedef boost::directed_graph<Instruction_ref, GraphEdge> Graph;
    typedef boost::graph_traits<Graph> GraphTraits;
    typedef GraphTraits::vertex_descriptor GraphVertexDescriptor;
    typedef GraphTraits::edge_descriptor GraphEdgeDescriptor;

    typedef std::set<GraphVertexDescriptor> GraphVertexDescriptorSet;


    void writeGraphToGraphViz(Graph &graph);
    // Adds an edge from u to v in graph g of type.
    // When u and v are the same vertex, an edge is only added if directLoopAllowed is true
    // Returns true if an edge was added, false if no edge was added
    bool addEdgeIfNeeded(const GraphVertexDescriptor &u, const GraphVertexDescriptor &v, Graph &g,
                         const GraphEdge::Type type, const bool directLoopAllowed = false);


    // Add an edge using addEdgeIfNeeded from every edge in vertexesSet to targetVertex.
    // Returns true if any edge was added, false if no edge was added.
    bool addEdgesFromVertexSet_ToVertex(const GraphVertexDescriptorSet &vertexesSet,
                                        const GraphVertexDescriptor &targetVertex,
                                        Graph &graph, const GraphEdge::Type type, const bool directLoopAllowed = false);


    void removeEdgesFromGraph(Graph &g, const GraphEdge::Type type = GraphEdge::Type::Default);

}

#endif //IDIOMMATCHER_GRAPH_H
