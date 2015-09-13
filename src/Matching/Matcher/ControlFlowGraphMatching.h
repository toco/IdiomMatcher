//
// Created by Tobias Conradi on 07.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_CONTROLFLOWGRAPHMATCHING_H
#define IDIOMMATCHER_CONTROLFLOWGRAPHMATCHING_H

#include <Matching/Matcher/Matching.h>
#include <Matching/Graph/Graph.h>

namespace IdiomMatcher {

    class ControlFlowGraphMatching : public Matching {
    protected:
        virtual bool matchGraphs(const Graph &patternGraph,
                                 const Graph &instructionGraph,
                                 const GraphVertexDescriptor &lastPatternVertexDesc,
                                 EA *matchedEndEA,
                                 Matching::ExtractedValuesMap &extractedValues) const;

        virtual GraphVertexDescriptor fillPatternGraph(Graph &patternGraph, const Pattern &pattern) const;
		virtual void fillInstruction(Graph &instructionGraph, DisassemblerAPI &disassemblerAPI, int maxInstructions) const;


		// storage for already build pattern graphs
		typedef std::shared_ptr<Graph> Graph_ref;
		struct GraphContainer {
			Graph_ref graph = std::make_shared<Graph>();
			GraphVertexDescriptor lastPatternVertexDescriptor = Graph::null_vertex();
		};
		std::map<Pattern_ref,GraphContainer> patternToGraphMap;

		// Always returns a pattern graph for a pattern.
		// Looks up the pattern in patternToGraphMap, if the pattern wasn't found
		// build sthe pattern graph using fillPatternGraph() and stores in map.
		virtual GraphContainer patternGraphForPattern(const Pattern_ref &pattern);

	public:

		ControlFlowGraphMatching(const std::string &name = "ControlFlowGraph") : Matching(name , true) { };

		virtual void testForPatternsStartingAtEA(const Patterns &patterns,
												 const EA &startEA,
												 DisassemblerAPI &disassemblerAPI,
												 const FoundMatchFunctionCallback &callback);
	};
}

#endif //IDIOMMATCHER_SIMPLEGRAPHMATCHING_H