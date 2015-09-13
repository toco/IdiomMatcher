//
// Created by Tobias Conradi on 16.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_DEPENDENCEGRAPHMATCHING_H
#define IDIOMMATCHER_DEPENDENCEGRAPHMATCHING_H

#include <Matching/Matcher/ControlFlowGraphMatching.h>

namespace IdiomMatcher {

    class DependenceGraphMatching: public ControlFlowGraphMatching {


    public:
		DependenceGraphMatching(const std::string name = "DependenceGraph") : ControlFlowGraphMatching(name) { };

		bool addEdgesToBasicBlockEnd = false;

	protected:
		virtual GraphVertexDescriptor fillPatternGraph(Graph &patternGraph, const Pattern &pattern) const override;
		virtual void fillInstruction(Graph &instructionGraph, DisassemblerAPI &disassemblerAPI, int maxInstructions) const override;

		void transformToPDGAndRemoveCFGEdges(Graph &graph) const;
    };
}


#endif //IDIOMMATCHER_DEPENDENCEGRAPHMATCHING_H
