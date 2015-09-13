//
// Created by Tobias Conradi on 16.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include "DependenceGraphMatching.h"
#include <Matching/Graph/PDGTransform.h>
namespace IdiomMatcher {

#define DEBUG_PRINTING 0

	void DependenceGraphMatching::transformToPDGAndRemoveCFGEdges(Graph &graph) const {

#if DEBUG_PRINTING
		std::cout << "CFG" << std::endl;
		writeGraphToGraphViz(instructionGraph);
#endif
		transformGraphToProgramDependenceGraph(graph, addEdgesToBasicBlockEnd);
#if DEBUG_PRINTING
		std::cout << "PDG" << std::endl;
		writeGraphToGraphViz(instructionGraph);
#endif
		removeEdgesFromGraph(graph);
	}

	void DependenceGraphMatching::fillInstruction(Graph &instructionGraph, DisassemblerAPI &disassemblerAPI, int maxInstructions) const {
		auto depth = maxInstructions*2+2;
		ControlFlowGraphMatching::fillInstruction(instructionGraph,disassemblerAPI, depth);
		transformToPDGAndRemoveCFGEdges(instructionGraph);
	}

	GraphVertexDescriptor DependenceGraphMatching::fillPatternGraph(Graph &patternGraph, const Pattern &pattern) const {
		GraphVertexDescriptor lastPatternVertexDesc =  ControlFlowGraphMatching::fillPatternGraph(patternGraph,pattern);
		transformToPDGAndRemoveCFGEdges(patternGraph);
		return lastPatternVertexDesc;
	}

}