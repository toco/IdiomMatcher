//
// Created by Tobias Conradi on 11.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include <stdio.h>
#include <cstdarg>
#include <thread>
#include <future>
#include <regex>
#include <sysexits.h>

#include "IdiomMatcherStandalone.h"
#include <Model/PatternPersistence.h>
#include <Model/Logging.h>
#include <Matching/Matcher/NaiveMatching.h>
#include <Matching/Matcher/ControlFlowGraphMatching.h>
#include <Matching/Matcher/DependenceGraphMatching.h>
#include <Matching/MatchPersistence.h>

bool IdiomMatcherStandalone::readPatterns() {
	IdiomMatcher::Patterns allPatterns;
	for (auto &path : patternFilePaths) {
		auto filePatterns = IdiomMatcher::PatternPersistence::readFromFilePath(path);
		if (filePatterns.empty()) {
			printf("no patterns loaded from %s\n", path.c_str());
		} else {
			allPatterns.insert(allPatterns.end(),filePatterns.begin(),filePatterns.end());
		}
	}
	if (allPatterns.empty()) {
		printf("failed to load any patterns");
		return false;
	}

	patterns = allPatterns;
	return true;
}

DumpDisassemblerAPI IdiomMatcherStandalone::readDisassembly() {
	IdiomMatcher::msg("Read diassembly file: %s\n",disassemblyFilePath.c_str());
	clock_t start = clock();
	DumpDisassemblerAPI api(disassemblyFilePath);
	clock_t end = clock();
	IdiomMatcher::msg("finnished reading diassembly file in %us\n",(end-start)/CLOCKS_PER_SEC);
	return api;
}

void IdiomMatcherStandalone::matchAll(DumpDisassemblerAPI &api) {

	if (matcherQueue.size() == 0) {
		matcherQueue.push_back("Naive");
	}
	for (auto name : matcherQueue) {
		IdiomMatcher::Matching *matcher;
		if (name == "SimpleGraph") {
			matcher = new IdiomMatcher::ControlFlowGraphMatching();
		} else if (name == "DependenceGraph") {
			matcher = new IdiomMatcher::DependenceGraphMatching();
		} else {
			matcher = new IdiomMatcher::NaiveMatching();
		}

		match(api, matcher);
		delete matcher;
	}
}

void IdiomMatcherStandalone::match(DumpDisassemblerAPI &api, IdiomMatcher::Matching* matcher) {

    IdiomMatcher::Matches matches;

	std::mutex callbackMutex;
    IdiomMatcher::Matching::FoundMatchFunctionCallback callback =  [&matches, &api, &callbackMutex] (const IdiomMatcher::Pattern &pattern, const IdiomMatcher::EA &startEA, const IdiomMatcher::EA &endEA, const std::map<std::string,std::string>& extractedValues) -> bool {
		callbackMutex.lock();
        auto comment = api.commentForEA(endEA);
        IdiomMatcher::msg("matched pattern %s for from: %jX to: %jX cmt: %s\n",pattern.getName().c_str(), startEA.getValue(),endEA.getValue(),comment.c_str());

        auto match = std::make_shared<IdiomMatcher::Match>(startEA,endEA,pattern.getName());
        matches.push_back(match);

        for (auto &value : extractedValues) {
            IdiomMatcher::msg("extracted %s : %s\n",value.first.c_str(), value.second.c_str());
        }
		callbackMutex.unlock();
        return true;
    };

	IdiomMatcher::Patterns patternsToTest;
	const std::string &architecture = api.executableArchitecture();
	std::copy_if(patterns.begin(), patterns.end(), std::back_inserter(patternsToTest),
				 [&architecture](const IdiomMatcher::Pattern_ref &pattern) {
					 return pattern->getArchitecture() == architecture;
				 });


	if (patternsToTest.empty()) {
		IdiomMatcher::msg("No patterns for %s found.\n",api.executableArchitecture().c_str());
		exit(EX_DATAERR);
	}

    IdiomMatcher::msg("Start matching with %s algorithm.\n",matcher->getName().c_str());
    clock_t start = clock();
	auto t1 = std::chrono::high_resolution_clock::now();

	// concurrency calculations
	auto concurrencyCount = std::thread::hardware_concurrency();
	if (matcher->getConcurrencyAllowed()) {
		concurrencyCount = 1<concurrencyCount ? concurrencyCount-1 : 1;
	} else {
		concurrencyCount = 1;
	}
	IdiomMatcher::EA startEA = startMatch != 0 ? IdiomMatcher::EA(startMatch) : api.minInstructionEA();
	IdiomMatcher::EA endEA = endMatch != 0 ? IdiomMatcher::EA(endMatch) : api.maxInstructionEA();
	uint offset = ceil((endEA.getValue()-startEA.getValue())*1.0/(concurrencyCount*1.0));
	offset = std::max(offset,(uint)1000);
	IdiomMatcher::EA chunkEndEA = IdiomMatcher::EA(std::min(startEA.getValue()+offset,endEA.getValue()));
	std::vector<std::shared_future<void> > futures;
	for (;startEA < endEA; chunkEndEA = IdiomMatcher::EA(std::min(chunkEndEA.getValue()+offset,endEA.getValue()))) {
		auto fut = std::async([&, startEA, chunkEndEA](){
			DumpDisassemblerAPI myAPI = api;
			
			matcher->searchForPatterns(patternsToTest, myAPI,callback,startEA,chunkEndEA);
		});
		futures.push_back(fut.share());
		startEA = chunkEndEA;
	}
	for (auto &fut : futures) {
		fut.wait();
	}

	auto t2 = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> diff = t2 - t1;

    clock_t end = clock();

    double cpuTime = (end-start)/(CLOCKS_PER_SEC*1.0);
    double realtime = diff.count();
    IdiomMatcher::msg("Matching finished in %f s CPU time, %f s real time.\n",cpuTime,realtime);
    IdiomMatcher::MatchPersistence persistence(api.executableName(),matcher->getName(),api.executableArchitecture(),realtime,cpuTime,matches);
	auto path = persistence.matchPathForExecutablePath(api.executablePath());
    if (persistence.saveToFilePath(path))
		IdiomMatcher::msg("Saved matches to %s\n",path.c_str());
	else
		IdiomMatcher::msg("Failed to save matches to %s\n",path.c_str());

}

void IdiomMatcherStandalone::dumpSwitches(DumpDisassemblerAPI &api) {
	using namespace IdiomMatcher;
	Matches matches;
	EA currentEA = api.minEA();
	EA maxEA = api.maxEA();
	api.setCurrentEAAndDecodeInstruction(currentEA);

	EA switchStartEA = InvalidEA;
	for (; currentEA < maxEA; currentEA = api.advanceInstruction()) {
		auto comment = api.getCurrentComment();
		if (std::regex_match(comment,std::regex("switch [0-9]* cases.*"))) {
			switchStartEA = currentEA;
		} else if (comment == "switch jump") {
			matches.push_back(std::make_shared<Match>(switchStartEA,currentEA,"switch"));
			switchStartEA = InvalidEA;
		}
	}
	MatchPersistence matchPersistence(api.executableName(), "comments", api.executableArchitecture(), 0, 0, matches);
	auto path = matchPersistence.matchPathForExecutablePath(api.executablePath());
	if (matchPersistence.saveToFilePath(path)) {
		msg("Saved switch matches from comments to %s\n",path.c_str());
	} else {
		msg("Failed to save switch matches from comments to %s\n",path.c_str());
	}
}

