//
// Created by Tobias Conradi on 26.06.15.
// Licensed under MIT License, see LICENSE for full text.


#include "NaiveMatching.h"

namespace IdiomMatcher {


    void NaiveMatching::testForPatternsStartingAtEA(const Patterns &patterns, const EA &startEA, DisassemblerAPI &disassemblerAPI, const FoundMatchFunctionCallback &callback) {
        for (auto pattern : patterns) {
            std::map<std::string, std::string> extractedValues;
            EA matchedEnd = startEA;

            auto &pattern_ref = *pattern.get();
            bool matched = testForPatternStartingAtEA(pattern_ref, startEA, &matchedEnd, disassemblerAPI, extractedValues);
            if (matched && callback) {
                callback(pattern_ref, startEA, matchedEnd, extractedValues);
            }
        }
    }


    bool NaiveMatching::testForPatternStartingAtEA(const IdiomMatcher::Pattern &pattern,
                                                   const IdiomMatcher::EA &startEA, IdiomMatcher::EA *matchedEndEA,
                                                   DisassemblerAPI &disassemblerAPI,
                                                   ExtractedValuesMap &extractedValues) {
        disassemblerAPI.setCurrentEAAndDecodeInstruction(startEA);

        bool matchedPattern = false;
		PatternNameMap nameMap;
        for (auto &patternInstruction : pattern.getInstructions()) {
            auto currentInstruction = disassemblerAPI.getCurrentInstruction();
            bool matchedInstructions = testInstructionsMatch(*patternInstruction,currentInstruction,&extractedValues,&nameMap);
			if (!matchedInstructions) return false;
            *matchedEndEA = disassemblerAPI.getCurrentEA();
            disassemblerAPI.advanceInstruction();
            matchedPattern = true;
        }
        return matchedPattern;
    }
}