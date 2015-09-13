//
// Created by Tobias Conradi on 11.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_IDIOMMATCHERSTANDALONE_H
#define IDIOMMATCHER_IDIOMMATCHERSTANDALONE_H

#include <string>
#include <deque>

#include "DumpDisassemblerAPI.h"
#include <Matching/Matcher/Matching.h>

int main(int argc, char* argv[]);

class IdiomMatcherStandalone {
public:

    std::string disassemblyFilePath;
    uint startMatch = 0;
	uint endMatch = 0;
    IdiomMatcher::Patterns patterns;

	std::deque<std::string> matcherQueue;
	std::vector<std::string> patternFilePaths;

	bool shouldDumpSwitches = false;


    bool readPatterns();
	DumpDisassemblerAPI readDisassembly();
	void matchAll(DumpDisassemblerAPI &api);
	void match(DumpDisassemblerAPI &api, IdiomMatcher::Matching* matcher);

	void dumpSwitches(DumpDisassemblerAPI &api);
	
};

#endif //IDIOMMATCHER_IDIOMMATCHERSTANDALONE_H
