//
// Created by Tobias Conradi on 12.09.15.
// Licensed under MIT License, see LICENSE for full text.

#include "CFGBuilder.h"

namespace IdiomMatcher {

    std::map <EA, GraphVertexDescriptor> fillCFG(Graph &graph, const EA &startEA, const int depth,
                                                               const InstructionForEACallback instructionForEA) {
        std::deque <CFGToDoItem> todos;
        todos.push_back(CFGToDoItem(startEA, GraphTraits::null_vertex(), depth));

        std::map <EA, GraphVertexDescriptor> eaToVertexDescriptor;

        while (todos.size() > 0) {
            auto todoItem = todos.front();
            todos.pop_front();
            GraphVertexDescriptor currentVertex;
            auto eaVertIt = eaToVertexDescriptor.find(todoItem.ea);
            if (eaVertIt != eaToVertexDescriptor.end()) {
                currentVertex = eaVertIt->second;
            } else {
                auto instruction = instructionForEA(todoItem.ea);
                if (instruction->getMnemonic() == InvalidInstruction.getMnemonic()) {
                    continue;
                }
                currentVertex = graph.add_vertex(instruction);
                eaToVertexDescriptor[todoItem.ea] = currentVertex;

                if (todoItem.ttl > 0 || todoItem.ttl == -1) {
                    for (auto ref : instruction->getXrefs()) {
                        if (ref->isData()) continue;
                        EA targetEA = ref->getTarget();
                        int newTTL = todoItem.ttl > 0 ? todoItem.ttl - 1 : todoItem.ttl;
                        auto todo = CFGToDoItem(targetEA, currentVertex, newTTL);
                        todos.push_back(todo);
                    }
                }
            }
            if (todoItem.previousVertex != GraphTraits::null_vertex()) {
                graph.add_edge(todoItem.previousVertex, currentVertex, GraphEdge());
            }
        }
        return eaToVertexDescriptor;
    }
}