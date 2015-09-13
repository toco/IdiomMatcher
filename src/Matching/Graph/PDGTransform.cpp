//
// Created by Tobias Conradi on 13.09.15.
// Licensed under MIT License, see LICENSE for full text.

#include "PDGTransform.h"

namespace IdiomMatcher {

    typedef std::map <std::string, GraphVertexDescriptorSet> OperandToVertexDescriptorSetMap;

    struct DependenceItem {
        const GraphVertexDescriptor vertexDescriptor;
        const OperandToVertexDescriptorSetMap writerVertexDescriptorMap;
        const GraphVertexDescriptorSet basicBlockVertexDescriptors; // used to keep other vertexes still in the basic blocks
        const GraphVertexDescriptorSet basicBlockStartVertexDescriptors;

        DependenceItem(const GraphVertexDescriptor &vertexDescriptor,
                       const GraphVertexDescriptorSet &fallback = GraphVertexDescriptorSet(),
                       const OperandToVertexDescriptorSetMap &writeMap = OperandToVertexDescriptorSetMap(),
                       const GraphVertexDescriptorSet &otherVertexDescriptors = GraphVertexDescriptorSet())
                : vertexDescriptor(vertexDescriptor), writerVertexDescriptorMap(writeMap),
                  basicBlockVertexDescriptors(otherVertexDescriptors), basicBlockStartVertexDescriptors(fallback) { }


        static inline void updateWriteVertexSetForOperandName(const GraphVertexDescriptor &vertexDescriptor,
                                                              OperandToVertexDescriptorSetMap &newWriteVertexMap,
                                                              const Operand_Ref &op, const std::string &name) {
            if (op->getModified()) {
                // replace the previous write vertexes in the map
                auto writeVertexSetIt = newWriteVertexMap.find(name);
                if (writeVertexSetIt != newWriteVertexMap.end()) {
                    auto &writeSet = writeVertexSetIt->second;
                    writeSet.clear();
                    writeSet.insert(vertexDescriptor);
                } else {
                    newWriteVertexMap[name] = {vertexDescriptor};
                }
            }
        }

        static inline bool addReadEdgesForOperandName(const GraphVertexDescriptor &vertexDescriptor,
                                                      const OperandToVertexDescriptorSetMap &oldWriteVertexMap,
                                                      const Operand_Ref &op, const std::string &name, Graph &graph) {
            bool addedReadEdge = false;
            if (op->getUsed()) {
                // operand is used, add edge from previous write vertexes to current vertex
                auto writeVertexIt = oldWriteVertexMap.find(name);
                if (writeVertexIt != oldWriteVertexMap.end()) {
                    auto &set = writeVertexIt->second;
                    addedReadEdge = addEdgesFromVertexSet_ToVertex(set, vertexDescriptor, graph, GraphEdge::Type::Data);
                }
            }
            return addedReadEdge;
        }

        DependenceItem calculateOutputItem_addEdges(Graph &graph, const bool addEdges,
                                                    const bool addEdgesToBasicBlockEnd = false) {

            OperandToVertexDescriptorSetMap newWriteVertexMap(writerVertexDescriptorMap);

            auto &vertex = graph[vertexDescriptor];

            // add edges for writes and reads
            for (auto &op : vertex->getOperands()) {
                auto registers = op->getRegisters();
                for (auto &name : registers) {
                    updateWriteVertexSetForOperandName(vertexDescriptor, newWriteVertexMap, op, name);
                    if (addEdges)
                        addReadEdgesForOperandName(vertexDescriptor, writerVertexDescriptorMap, op, name, graph);
                }

                // if the operand has no registers use the text instead
                if (registers.empty()) {
                    auto name = op->getText();
                    updateWriteVertexSetForOperandName(vertexDescriptor, newWriteVertexMap, op, name);
                    if (addEdges)
                        addReadEdgesForOperandName(vertexDescriptor, writerVertexDescriptorMap, op, name, graph);
                }
            }

            if (addEdges) {
                // add control dependence edges from basic block start
                for (auto &fallbackDesc : basicBlockStartVertexDescriptors) {
                    addEdgeIfNeeded(fallbackDesc, vertexDescriptor, graph, GraphEdge::Type::Control);
                }
            }
            GraphVertexDescriptorSet newBasicBlockVertexDescriptors(basicBlockVertexDescriptors);
            GraphVertexDescriptorSet newBasicBlockStartVertexDescriptors(basicBlockStartVertexDescriptors);

            auto xrefs = vertex->getXrefs();
            auto newBasicblockStart =
                    1 < std::count_if(xrefs.begin(), xrefs.end(), [](XRef_Ref ref) { return !ref->isData(); });

            if (newBasicblockStart) {
                // if current vertex is a jump we add data edges from all basic block vertexes to the current one
                // there might be implicit data dependencies via flag registers mostly used for jumps
                if (addEdges && addEdgesToBasicBlockEnd) {
                    addEdgesFromVertexSet_ToVertex(newBasicBlockVertexDescriptors, vertexDescriptor, graph,
                                                   GraphEdge::Type::Data);
                    newBasicBlockVertexDescriptors.clear();
                }
                // set the current vertex as a basic block start vertex
                newBasicBlockStartVertexDescriptors.clear();
                newBasicBlockStartVertexDescriptors.insert(vertexDescriptor);
            } else if (addEdgesToBasicBlockEnd) {
                // add the vertex to set which allows nodes to stay whithin basic blocks
                newBasicBlockVertexDescriptors.insert(vertexDescriptor);
            }

            return DependenceItem(vertexDescriptor, newBasicBlockStartVertexDescriptors, newWriteVertexMap,
                                  newBasicBlockVertexDescriptors);
        }

        void addNewItemsToToDoQueue(Graph &graph, std::deque <GraphVertexDescriptor> &itemsToDo) const {
            boost::graph_traits<Graph>::out_edge_iterator eo, edge_end;
            for (boost::tie(eo, edge_end) = out_edges(vertexDescriptor, graph); eo != edge_end; ++eo) {
                if (graph[*eo].type == GraphEdge::Type::Default) {
                    auto targetDescriptor = target(*eo, graph);
                    itemsToDo.push_back(targetDescriptor);
                }
            }
        }

        const bool operator!=(const DependenceItem &other) const {
            return vertexDescriptor != other.vertexDescriptor ||
                   writerVertexDescriptorMap != other.writerVertexDescriptorMap ||
                   basicBlockVertexDescriptors != other.basicBlockVertexDescriptors ||
                   basicBlockStartVertexDescriptors != other.basicBlockStartVertexDescriptors;
        }

        const bool operator==(const DependenceItem &other) const {
            return vertexDescriptor == other.vertexDescriptor &&
                   writerVertexDescriptorMap == other.writerVertexDescriptorMap &&
                   basicBlockVertexDescriptors == other.basicBlockVertexDescriptors &&
                   basicBlockStartVertexDescriptors == other.basicBlockStartVertexDescriptors;
        }

    };

    void mergeOperandToVertexSetMap_IntoMap(const OperandToVertexDescriptorSetMap &source,
                                            OperandToVertexDescriptorSetMap &target) {
        for (auto &sourcePair : source) {
            auto targetIt = target.find(sourcePair.first);
            if (targetIt != target.cend()) {
                targetIt->second.insert(sourcePair.second.cbegin(), sourcePair.second.cend());
            } else {
                target.insert(sourcePair);
            }
        }
    }

    DependenceItem inputItemForVertexDescriptor_graph_withMap(const GraphVertexDescriptor &currentDescriptor,
                                                              const Graph &graph,
                                                              const std::map <GraphVertexDescriptor, DependenceItem> &map) {
        OperandToVertexDescriptorSetMap writeMap;
        OperandToVertexDescriptorSetMap readMap;
        GraphVertexDescriptorSet basicBlockVertexDescriptors;
        GraphVertexDescriptorSet basicBlockStartVertexDescriptors;

        Graph::in_edge_iterator ei, edge_end;
        for (boost::tie(ei, edge_end) = in_edges(currentDescriptor, graph); ei != edge_end; ++ei) {
            if (graph[*ei].type == GraphEdge::Type::Default) {
                auto sourceDescrip = source(*ei, graph);
                auto itemIt = map.find(sourceDescrip);
                if (itemIt != map.end()) {
                    auto &item = itemIt->second;
                    mergeOperandToVertexSetMap_IntoMap(item.writerVertexDescriptorMap, writeMap);
                    basicBlockVertexDescriptors.insert(item.basicBlockVertexDescriptors.cbegin(),
                                                       item.basicBlockVertexDescriptors.cend());
                    basicBlockStartVertexDescriptors.insert(item.basicBlockStartVertexDescriptors.cbegin(),
                                                            item.basicBlockStartVertexDescriptors.cend());
                }
            }
        }
        return DependenceItem(currentDescriptor, basicBlockStartVertexDescriptors, writeMap,
                              basicBlockVertexDescriptors);
    }


    void transformGraphToProgramDependenceGraph(Graph &graph, const bool addEdgesToBasicBlockEnd) {
        using namespace boost;

        std::map <GraphVertexDescriptor, DependenceItem> vertexDescriptorToCurrentOutItems;
        std::deque <GraphVertexDescriptor> itemsToDo;

        GraphVertexDescriptor startVertexDesc = graph.add_vertex(std::make_shared<Instruction>(InvalidInstruction));
        GraphVertexDescriptor firstVertexDesc = *boost::vertices(graph).first;
        graph.add_edge(startVertexDesc, firstVertexDesc);

        vertexDescriptorToCurrentOutItems.emplace(startVertexDesc, DependenceItem(startVertexDesc,
                                                                                  GraphVertexDescriptorSet{
                                                                                          startVertexDesc}));

        itemsToDo.push_back(firstVertexDesc);

        while (!itemsToDo.empty()) {
            auto currentVertexDesc = itemsToDo.front();
            itemsToDo.pop_front();

            auto inputItem = inputItemForVertexDescriptor_graph_withMap(currentVertexDesc, graph,
                                                                        vertexDescriptorToCurrentOutItems);

            auto newOutputItem = inputItem.calculateOutputItem_addEdges(graph, true, addEdgesToBasicBlockEnd);

            auto oldOutputItemsIt = vertexDescriptorToCurrentOutItems.find(currentVertexDesc);
            bool changedOutput = true;
            if (oldOutputItemsIt != vertexDescriptorToCurrentOutItems.end()) {
                auto &oldOutputItem = oldOutputItemsIt->second;
                if (oldOutputItem != newOutputItem) {
                    vertexDescriptorToCurrentOutItems.erase(oldOutputItemsIt);
                } else {
                    changedOutput = false;
                }
            }
            if (changedOutput) {
                newOutputItem.addNewItemsToToDoQueue(graph, itemsToDo);
                vertexDescriptorToCurrentOutItems.emplace(currentVertexDesc, newOutputItem);
            }
        }
    }
}