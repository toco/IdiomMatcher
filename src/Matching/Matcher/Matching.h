//
// Created by Tobias Conradi on 07.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_MATCHING_H
#define IDIOMMATCHER_MATCHING_H



#include <regex>
#include <map>
#include <Model/Pattern.h>
#include <Matching/DisassemblerAPI.h>

namespace IdiomMatcher {


    class Matching {
	public:
		typedef std::map<std::string, std::string> StringStringMap;
		typedef StringStringMap ExtractedValuesMap;
		typedef StringStringMap PatternNameMap;
		typedef std::function<bool(const Pattern&, const EA& start, const EA& end, const ExtractedValuesMap&)> FoundMatchFunctionCallback;

        Matching(const std::string &matcherName = "", const bool concurrencyAllowed = true) : _name(matcherName), _concurrencyAllowed(concurrencyAllowed) { };
        virtual ~Matching() {};

		virtual void searchForPatterns(const Patterns &patterns, DisassemblerAPI &disassemblerAPI, const FoundMatchFunctionCallback &callback, const EA &startEA, const EA &endEA);

		virtual void searchForPatterns(const Patterns &patterns, DisassemblerAPI &disassemblerAPI, const FoundMatchFunctionCallback &callback);

		virtual void testForPatternsStartingAtEA(const Patterns &patterns, const EA &startEA, DisassemblerAPI &disassemblerAPI, const FoundMatchFunctionCallback &callback) = 0;

        virtual bool testInstructionsMatch(const Instruction &patternInstr, const Instruction &dissInstruction, ExtractedValuesMap *extractedValuesMap = nullptr, PatternNameMap *patternNameMap = nullptr) const;

        virtual std::string getName() const { return _name; };

		virtual bool getConcurrencyAllowed() const { return _concurrencyAllowed; };
    private:
        const std::string _name;
		const bool _concurrencyAllowed;
	};

}


#endif //IDIOMMATCHER_MATCHING_H
