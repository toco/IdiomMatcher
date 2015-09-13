//
// Created by Tobias Conradi on 07.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include "ControlFlowGraphMatching.h"
#include <algorithm>
#include <boost/graph/vf2_sub_graph_iso.hpp>
#include <Matching/Graph/CFGBuilder.h>

namespace IdiomMatcher {

	void ControlFlowGraphMatching::testForPatternsStartingAtEA(const Patterns &patterns, const EA &startEA, DisassemblerAPI &disassemblerAPI, const FoundMatchFunctionCallback &callback) {

		disassemblerAPI.setCurrentEAAndDecodeInstruction(startEA);
		auto dissMnemonic = disassemblerAPI.getCurrentInstruction().getMnemonic();

		Patterns canditates;

		size_t maxDepth = 0;
		for (auto &pattern: patterns) {
			// early continue if pattern empty
			auto &instructions = pattern->getInstructions();
			if (instructions.empty())
				continue;
			// early continue if first instructions don't match
			if (instructions[0]->getMnemonic() != dissMnemonic) {
				continue;
			}

			canditates.push_back(pattern);
			maxDepth = std::max(maxDepth, instructions.size());
		}

		// early return if no canditate patterns were found
		if (canditates.empty()) return;

		Graph instructionGraph;
		disassemblerAPI.setCurrentEAAndDecodeInstruction(startEA);
		fillInstruction(instructionGraph,disassemblerAPI, maxDepth);

		for (auto &pattern : canditates) {
			auto &pattern_ref = *pattern;
			GraphContainer graphContainer = patternGraphForPattern(pattern);
			GraphVertexDescriptor lastPatternVertexDesc = graphContainer.lastPatternVertexDescriptor;
			Graph &patternGraph = *(graphContainer.graph);

			std::map<std::string, std::string> extractedValues;
			EA matchedEndEA = startEA;

			bool matched = matchGraphs(patternGraph,instructionGraph,lastPatternVertexDesc,&matchedEndEA,extractedValues);
			if (matched && callback) {
				callback(pattern_ref, startEA, matchedEndEA, extractedValues);
			}
		}
	}

    GraphVertexDescriptor ControlFlowGraphMatching::fillPatternGraph(Graph &patternGraph, const Pattern &pattern) const {
		auto instructions = pattern.getInstructions();
		if (instructions.empty())
			return patternGraph.null_vertex();

		std::map<EA, Instruction_ref> eaToInstructionMap;
		for (auto &instruction : instructions) {
			eaToInstructionMap.emplace(instruction->getEA(),instruction);
		}

		const EA &startEA = instructions.front()->getEA();

		auto eaToVertexDescriptorMap = fillCFG(patternGraph, startEA, -1,[&eaToInstructionMap] (const EA &ea) -> Instruction_ref {
			auto it = eaToInstructionMap.find(ea);
			if (it == eaToInstructionMap.end()) {
				return InvalidInstruction_ref;
			} else {
				return it->second;
			}
		});
		const EA &endEA = instructions.back()->getEA();
		auto it = eaToVertexDescriptorMap.find(endEA);
		return it == eaToVertexDescriptorMap.end() ? patternGraph.null_vertex() : it->second;
    }

	void ControlFlowGraphMatching::fillInstruction(Graph &instructionGraph, DisassemblerAPI &disassemblerAPI, int maxInstructions) const {
		EA startEA = disassemblerAPI.getCurrentEA();
		fillCFG(instructionGraph, startEA, maxInstructions,[&disassemblerAPI] (const EA &ea) -> Instruction_ref {
			auto instruction = disassemblerAPI.instructionForEA(ea);
			return std::make_shared<Instruction>(instruction);
		});
	}

	ControlFlowGraphMatching::GraphContainer ControlFlowGraphMatching::patternGraphForPattern(const Pattern_ref &pattern) {
		auto it = patternToGraphMap.find(pattern);
		if (it != patternToGraphMap.end()) {
			return it->second;
		} else {
			GraphContainer container;
			Graph &graph = *(container.graph);
			container.lastPatternVertexDescriptor = fillPatternGraph(graph, *pattern);
			patternToGraphMap.emplace(pattern,container);
			return container;
		}
	}

	// match callback for vf2_subgraph_*
	template <typename Graph1,
			typename Graph2>
	struct lastEACallback {

		lastEACallback(const Graph1& graph1, const Graph2& graph2, const GraphVertexDescriptor &vertexDescriptor, Instruction_ref *lastMatchedInstruction, const ControlFlowGraphMatching &graphMatcher, Matching::ExtractedValuesMap &extractedValuesMap, bool *verifiedMatch = nullptr)
				: graph1_(graph1), graph2_(graph2), _lastPatternVertexDescriptor(vertexDescriptor), _lastMatchedInstruction(lastMatchedInstruction), _graphMatcher(graphMatcher), _extractedValuesMap(extractedValuesMap), _matched(verifiedMatch) { }

		template <typename CorrespondenceMap1To2,
				typename CorrespondenceMap2To1>

		bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) const {
			Matching::ExtractedValuesMap extractedValues;
			Matching::PatternNameMap patternNameMap;
			bool matches = false;

			// Test if the mapping is valid when considering template names, extract values
			BGL_FORALL_VERTICES (vert, graph1_, Graph) {
				auto vert2 = boost::get(f,vert);
				auto inst1 = graph1_[vert];
				auto inst2 = graph2_[vert2];
				matches = _graphMatcher.testInstructionsMatch(*inst1,*inst2,&extractedValues,&patternNameMap);
				if (!matches)
					return true; // continue search
			}

			if (_matched)
				*_matched = matches;

			if (_lastPatternVertexDescriptor != Graph::null_vertex()) {
				auto lastInstructionVertexDescriptor = f[_lastPatternVertexDescriptor];
				*_lastMatchedInstruction = graph2_[lastInstructionVertexDescriptor];
			}
			_extractedValuesMap.insert(extractedValues.cbegin(),extractedValues.cend());
			return false; // end search
		}

	private:
		const Graph1& graph1_;
		const Graph2& graph2_;
		GraphVertexDescriptor _lastPatternVertexDescriptor;
		Instruction_ref *_lastMatchedInstruction;
		const ControlFlowGraphMatching &_graphMatcher;
		Matching::ExtractedValuesMap &_extractedValuesMap;
		bool *_matched;
	};


	bool ControlFlowGraphMatching::matchGraphs(const Graph &patternGraph, const Graph &instructionGraph,
                                          const GraphVertexDescriptor &lastPatternVertexDesc,
                                          EA *matchedEndEA,
                                          Matching::ExtractedValuesMap &extractedValues) const {
        Instruction_ref lastMatchedInstruction;
		bool verifiedMatched = false;
        auto callback = lastEACallback<Graph,Graph>(patternGraph,instructionGraph,lastPatternVertexDesc, &lastMatchedInstruction, *this, extractedValues, &verifiedMatched);
        using namespace boost;
        bool matched = vf2_subgraph_mono(patternGraph,
                                        instructionGraph,
                                        callback,
                                        get(vertex_index, patternGraph),
                                        get(vertex_index, instructionGraph),
                                        vertex_order_by_mult(patternGraph),
                                        [this, &patternGraph, &instructionGraph](GraphEdgeDescriptor small_edge, GraphEdgeDescriptor large_edge) {
											auto &patternEdge = patternGraph[small_edge];
											auto &instructionEdge = instructionGraph[large_edge];
											if (patternEdge.type != instructionEdge.type)
												return false;
											auto source1 = patternGraph[source(small_edge,patternGraph)];
											auto source2 = instructionGraph[source(large_edge,instructionGraph)];
                                            if (!testInstructionsMatch(*source1, *source2))
                                                return false;
                                            auto target1 = patternGraph[target(small_edge,patternGraph)];
                                            auto target2 = instructionGraph[target(large_edge,instructionGraph)];
                                            return testInstructionsMatch(*target1, *target2);
                                        },
                                        [this, &patternGraph, &instructionGraph](GraphVertexDescriptor small_vd, GraphVertexDescriptor large_vd) {
                                            auto patternInstr = patternGraph[small_vd];
                                            auto dissInstr = instructionGraph[large_vd];
                                            return testInstructionsMatch(*patternInstr,*dissInstr);
                                        });
		
		matched = matched && verifiedMatched;

        if (matched && lastMatchedInstruction != nullptr) {
            *matchedEndEA = lastMatchedInstruction->getEA();
        }
        return matched;
    }
}