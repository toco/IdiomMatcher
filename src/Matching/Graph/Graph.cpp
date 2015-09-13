//
// Created by Tobias Conradi on 12.09.15.
// Licensed under MIT License, see LICENSE for full text.

#include "Graph.h"

#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/graphviz.hpp>

namespace IdiomMatcher {


    struct vertex_writer {
    public:
        vertex_writer(const Graph &graph) : _graph(&graph) { }

        void operator()(std::ostream &out, const Graph::vertex_descriptor &v) const {
            out << "[label=\"" << (*_graph)[v]->description() << "\"]";
        }

    private:
        const Graph *_graph;
    };

    struct edge_writer {
    public:
        edge_writer(const Graph &graph) : _graph(&graph) { }

        void operator()(std::ostream &out, const Graph::edge_descriptor &e) const {
            switch ((*_graph)[e].type) {
                case GraphEdge::Type::Default:
                    break;
                case GraphEdge::Type::Data:
                    out << "[style=dotted]";
                    break;
                case GraphEdge::Type::Control:
                    out << "[style=dashed]";
                    break;
            }
        }

    private:
        const Graph *_graph;
    };

    void writeGraphToGraphViz(Graph &graph) {
        write_graphviz(std::cout, graph, vertex_writer(graph), edge_writer(graph));
    }


    bool addEdgeIfNeeded(const GraphVertexDescriptor &u, const GraphVertexDescriptor &v, Graph &g,
                         const GraphEdge::Type type, const bool directLoopAllowed) {
        if (directLoopAllowed && u==v) return false;

        // check edge already exists
        Graph::in_edge_iterator ei, edge_end;
        for (boost::tie(ei,edge_end) = in_edges(v, g); ei != edge_end; ++ei) {
            auto edge = *ei;
            if (source(edge,g) == u && g[edge].type == type) {
                return false;
            }
        }
        g.add_edge(u, v,GraphEdge(type));
        return true;
    }

    bool addEdgesFromVertexSet_ToVertex(const std::set<GraphVertexDescriptor> &vertexesSet,const  GraphVertexDescriptor &targetVertex, Graph &graph, const GraphEdge::Type type, const bool directLoopAllowed) {
        bool added = false;
        for (auto &sourceVertex : vertexesSet) {
            added = added | addEdgeIfNeeded(sourceVertex, targetVertex, graph, type, directLoopAllowed);
        }
        return added;
    }


    void removeEdgesFromGraph(Graph &g, const GraphEdge::Type type) {
        typename Graph::edge_iterator ei, ei_end;
        std::deque<Graph::edge_descriptor> edgesToRemove;
        for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
            if (g[*ei].type == type) {
                edgesToRemove.push_back(*ei);
            }
        }
        for (auto &e : edgesToRemove) {
            g.remove_edge(e);
        }
    }

}