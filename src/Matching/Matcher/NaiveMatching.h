//
// Created by Tobias Conradi on 26.06.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_NAIVEMATCHING_H
#define IDIOMMATCHER_NAIVEMATCHING_H

#include <vector>
#include <map>
#include <functional>

#include <Model/Pattern.h>
#include <Matching/Matcher/Matching.h>

namespace IdiomMatcher {


    class NaiveMatching : public Matching {

	public:
		NaiveMatching() : Matching ("Naive") { }
		virtual void testForPatternsStartingAtEA(const Patterns &patterns, const EA &startEA,
												 DisassemblerAPI &disassemblerAPI,
												 const FoundMatchFunctionCallback &callback);

		virtual bool testForPatternStartingAtEA(const Pattern &pattern, const EA &startEA, EA *matchedEndEA,
												DisassemblerAPI &disassemblerAPI,
												ExtractedValuesMap &extractedValues);
	};

}
#endif //IDIOMMATCHER_BASICMATCHING_H
