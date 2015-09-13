//
// Created by Tobias Conradi on 13.09.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_PDGTRANSFORM_H
#define IDIOMMATCHER_PDGTRANSFORM_H

#include <Matching/Graph/Graph.h>

namespace IdiomMatcher {
    void transformGraphToProgramDependenceGraph(Graph &graph, const bool addEdgesToBasicBlockEnd);
}

#endif //IDIOMMATCHER_PDGTRANSFORM_H
