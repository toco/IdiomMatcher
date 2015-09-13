
set (SOURCES
		DisassemblerAPI.cpp
		DisassemblerAPI.h

		Matcher/NaiveMatching.cpp
		Matcher/NaiveMatching.h
		Matcher/DependenceGraphMatching.cpp
		Matcher/DependenceGraphMatching.h
		Matcher/Matching.cpp
		Matcher/Matching.h
		Matcher/ControlFlowGraphMatching.cpp
		Matcher/ControlFlowGraphMatching.h

		MatchPersistence.cpp
		MatchPersistence.h

		Graph/Graph.cpp
		Graph/Graph.h

		Graph/CFGBuilder.cpp
		Graph/CFGBuilder.h
		Graph/PDGTransform.cpp
		Graph/PDGTransform.h
)

add_library(Matching STATIC ${SOURCES})
