//
// Created by Tobias Conradi on 12.09.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_CFGBUILDER_H
#define IDIOMMATCHER_CFGBUILDER_H

#include <Matching/Graph/Graph.h>

namespace IdiomMatcher {


    struct CFGToDoItem {
        CFGToDoItem(const IdiomMatcher::EA &ea, const GraphVertexDescriptor &previousVertex, const uint ttl) : ea(ea),previousVertex(previousVertex),ttl(ttl) {};
        const EA ea;
        const GraphVertexDescriptor previousVertex;
        const uint ttl;
    };

    typedef std::function<Instruction_ref(const EA& ea)> InstructionForEACallback;

    // Constructs a control flow graph starting at startEA with a maximum graph depth of depth.
    std::map <EA, GraphVertexDescriptor> fillCFG(Graph &graph, const EA &startEA, const int depth,
                                                 const InstructionForEACallback instructionForEA);

}

#endif //IDIOMMATCHER_CFGBUILDER_H
