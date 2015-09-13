//
// Created by Tobias Conradi on 05.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include "StringSwitchSearch.h"
#include <search.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <loader.hpp>

namespace pt = boost::property_tree;

static void performSearch(IDAAdapter::SimpleCallbackActionHandler *);
static void saveTree(pt::ptree &tree);

SwitchStringSearch::SwitchStringSearch(plugin_t *plugin) :
        IDAAdapter::SimpleCallbackActionHandler(plugin,
                                                performSearch,
                                                "IdiomMatcher::SwitchStringSearch",
                                                "Search switch strings",
                                                "Searches switches by string and saves them to a JSON file."
        ) { }


static void performSearch(IDAAdapter::SimpleCallbackActionHandler *handler) {

    msg("start string search\n");
    ea_t currentEA = 0;
    pt::ptree matchPairs;
    while (currentEA!=BADADDR) {
        pt::ptree match;
        match.put("name","switch");
        currentEA = find_text(currentEA, 0, 0, "switch [0-9]* cases", SEARCH_DOWN | SEARCH_REGEX);
        match.put("startEA",currentEA);
        currentEA = find_text(currentEA, 0, 0, "switch jump", SEARCH_DOWN);
        match.put("endEA",currentEA);
        if (currentEA != BADADDR)
            matchPairs.push_back(std::make_pair("", match));
    }
    saveTree(matchPairs);
    msg("finished string search\n");
}

static void saveTree(pt::ptree &tree) {
    auto jsonpath = std::string(database_idb);
    size_t lastPoist = jsonpath.find_last_of(".");
    if (lastPoist != std::string::npos) {
        jsonpath.erase(lastPoist);
    }
    jsonpath.append("_comments.json");

    pt::ptree document;
    document.add_child("matches",tree);
    try {
        pt::write_json(jsonpath, document);
    } catch (std::exception error) {
        warning("failed to write comments json");
        return;;
    }
    msg("saved string search results to %s\n",jsonpath.c_str());
}
