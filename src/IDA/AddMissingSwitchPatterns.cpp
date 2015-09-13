//
// Created by Tobias Conradi on 06.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include <IDA/IDAAdapter.h>
#include <ida.hpp>
#include <search.hpp>
#include "AddMissingSwitchPatterns.h"
#include <Matching/Matcher/NaiveMatching.h>


AddMissingSwitchPatterns::AddMissingSwitchPatterns(plugin_t *plugin, IdiomMatcher::Patterns *patterns) :
        IDAAdapter::SimpleCallbackActionHandler(plugin,
                                                AddMissingSwitchPatterns::addMissingSwitchPatterns,
                                                "IdiomMatcher::AddMissingSwitchPatterns",
                                                "Add missing patterns.",
                                                "Add for each string search switch occurrence that is not found by the defined patterns a new pattern."),
        _patterns(patterns)
{ }

void AddMissingSwitchPatterns::addMissingSwitchPatterns(IDAAdapter::SimpleCallbackActionHandler *action) {
    auto &addAction = dynamic_cast<AddMissingSwitchPatterns &> (*action);

    IDAAdapter::IDA ida;
    ida.executableName();

    msg("start auto-adding patterns\n");
    ea_t currentEA = 0;
    ea_t startEA, endEA = 0;
    auto matcher = IdiomMatcher::NaiveMatching();

    while (currentEA!=BADADDR) {
        startEA = currentEA = find_text(currentEA, 0, 0, "switch [0-9]* cases", SEARCH_DOWN | SEARCH_REGEX);
        endEA = currentEA = find_text(currentEA, 0, 0, "switch jump", SEARCH_DOWN);
        if (endEA == BADADDR)
            break;

        bool matched = false;
        for (auto &pattern : *(addAction._patterns)) {
            IdiomMatcher::EA matchedEnd(0);
            std::map<std::string,std::string> extracted;
            bool match = matcher.testForPatternStartingAtEA(*pattern.get(),IdiomMatcher::EA(startEA),&matchedEnd,ida,extracted);
            if (match && matchedEnd.getValue()) {
                matched = true;
                break;
            }
        }
        if (matched)
            continue;

        auto name = std::string(ida.executableName());
        name.append("_");
        name.append(std::to_string(++addAction.counter));

        auto start = IdiomMatcher::EA(startEA);
        auto end = IdiomMatcher::EA(endEA);
        msg("auto add new pattern %X to %X named %s\n",startEA,endEA,name.c_str());
        auto pattern = IDAAdapter::CreatePatternActionHandler::createPatternFromEAToEA(start, end, name);
        addAction._patterns->push_back(std::make_shared<IdiomMatcher::Pattern>(pattern));
    }

    msg("finished auto-adding patterns\n");

}