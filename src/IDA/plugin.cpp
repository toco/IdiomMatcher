//
// Created by Tobias Conradi on 25.06.15.
// Licensed under MIT License, see LICENSE for full text.


#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <expr.hpp>
#include <pro.h>
#include <Matching/Matcher/NaiveMatching.h>
#include <Model/PatternPersistence.h>
#include <Model/Logging.h>
#include <IDA/IDAUIActions.h>
#include <boost/property_tree/json_parser.hpp>
#include <IDA/StringSwitchSearch.h>
#include <IDA/AddMissingSwitchPatterns.h>
#include <Matching/Matcher/ControlFlowGraphMatching.h>
#include <IDA/IDAAdapter.h>
#include <Model/DisassemblyPersistence.h>
#include <Matching/Matcher/DependenceGraphMatching.h>
#include <Matching/MatchPersistence.h>
#include <chrono>

int IDAP_init(void);
void IDAP_term(void);
void IDAP_run(int arg);

// There isn't much use for these yet, but I set them anyway.
char IDAP_comment[]     = "This is a plugin to match specified idioms in the disassebled code.";
char IDAP_help[]        = "Idiom Matcher";
// The name of the plug-in displayed in the Edit->Plugins menu. It can // be overridden in the user's plugins.cfg file.
char IDAP_name[] = "IdiomMatcher";
// The hot-key the user can use to run your plug-in.
char IDAP_hotkey[]      = "ALT-CTRL-M";
// The all-important exported PLUGIN object
plugin_t PLUGIN =
		{
				IDP_INTERFACE_VERSION,
				0,
				IDAP_init,
				IDAP_term,
				IDAP_run,
				IDAP_comment,
				IDAP_help,
				IDAP_name,
// IDA version plug-in is written for
// Flags (see below)
// Initialisation function
// Clean-up function
// Main plug-in body
// Comment – unused
// As above – unused
// Plug-in name shown in
// Edit->Plugins menu
// Hot key to run the plug-in
				IDAP_hotkey
		};

std::vector<IDAAdapter::ActionHandler *> actionHandler;

IdiomMatcher::Patterns allPatterns;

void addedPatternCallback(const IdiomMatcher::Pattern& newPattern);
void loadPatternsCallback(IDAAdapter::SimpleCallbackActionHandler*);
void savePatternsCallback(IDAAdapter::SimpleCallbackActionHandler*);
void dumpCallback(IDAAdapter::SimpleCallbackActionHandler*);

static error_t idaapi idc_func_dump(idc_value_t *argv,idc_value_t *r);
static const char idc_func_dump_args[] = { 0 };
static const char idc_func_dump_name[] = "dumpToJSON";

static std::string jsonPathForFileName(std::string jsonName);

namespace pt = boost::property_tree;

bool patternsUnsafed = false;

int IDAP_init(void)
{
	// setup logging functions we want to use
	IdiomMatcher::msg = &msg;
	IdiomMatcher::warning = &warning;
	IdiomMatcher::info = &info;

	IdiomMatcher::serializeDefaultValues = false;

	// add custom actions

	actionHandler.push_back(new IDAAdapter::CreatePatternActionHandler(&PLUGIN, addedPatternCallback));
	actionHandler.push_back(new IDAAdapter::SimpleCallbackActionHandler(&PLUGIN,
																	  loadPatternsCallback,
																	  "IdiomMatcher:LoadPatterns",
																	  "Load Patterns",
																	  "Load Patterns from file for IdiomMatcher",
																		"ALT-CTRL-L"
	));
	actionHandler.push_back( new IDAAdapter::SimpleCallbackActionHandler(&PLUGIN,
																	  savePatternsCallback,
																	  "IdiomMatcher:SavePatterns",
																	  "Save Patterns",
																	  "Save Patterns to file for IdiomMatcher",
																		 "ALT-CTRL-S"
	));
	actionHandler.push_back(new SwitchStringSearch(&PLUGIN));

	actionHandler.push_back(new AddMissingSwitchPatterns(&PLUGIN,&allPatterns));

	actionHandler.push_back(new IDAAdapter::SimpleCallbackActionHandler(&PLUGIN,
																		dumpCallback,
																		"IdiomMatcher:DumpDisassembly",
																		"Dump disassembly to file",
																		"Dump disassembly as JSON file",
																		"ALT-CTRL-D"
	));


	// add custom IDC function dumpJSON()
	set_idc_func_ex(idc_func_dump_name, idc_func_dump, idc_func_dump_args,0);

	return PLUGIN_KEEP;
}
void IDAP_term(void)
{
	if (patternsUnsafed)
		savePatternsCallback(nullptr);

	// remove IDC functions
	set_idc_func_ex(idc_func_dump_name, NULL, NULL, 0);

	// remove custom action handlers
	for (IDAAdapter::ActionHandler *handler : actionHandler) {
		delete handler;
	}

	return;
}



// The plugin can be passed an integer argument from the plugins.cfg
// file. This can be useful when you want the one plug-in to do
// something different depending on the hot-key pressed or menu
// item selected.

void IDAP_run(int arg)
{

	if (arg == 1) {
		return;
	}

    ea_t start;
    ea_t end;
    if (read_selection(&start, &end)) {
        msg("selection start: %zu, selection end: %zu\n",(size_t)start,(size_t)end);
    } else {
        start = get_screen_ea();
        msg("selected line: %zu\n",(size_t)start);
    }

	IDAAdapter::IDA idaAPI;
	IdiomMatcher::Matches matches;

	// callback handler called when a matcher found a match
	IdiomMatcher::Matching::FoundMatchFunctionCallback callback =  [&matches] (const IdiomMatcher::Pattern &pattern, const IdiomMatcher::EA &startEA, const IdiomMatcher::EA &endEA, const std::map<std::string,std::string>& extractedValues) -> bool {

		auto match = std::make_shared<IdiomMatcher::Match>(startEA,endEA,pattern.getName());
		matches.push_back(match);

#ifdef DEBUG
		char comment[MAXSTR];
		memset(comment,0,sizeof(comment));
		get_extra_cmt(endEA.getValue(), 0, comment, sizeof(comment));
		msg("matched pattern %s for from: %jX to: %jX cmt: %s\n",pattern.getName().c_str(), startEA.getValue(),endEA.getValue(),comment);

		for (auto &value : extractedValues) {
			msg("extracted %s : %s\n",value.first.c_str(), value.second.c_str());
		}
#endif

		for (auto action : pattern.getActions()) {
			auto script = action->getScript();
			if (!script.empty()) {
				const int argc = 3;
				idc_value_t arguments[argc];
				arguments[0] = idc_value_t(pattern.getName().c_str());
				arguments[1] = idc_value_t(startEA.getValue());
				arguments[2] = idc_value_t(endEA.getValue());
				char error[MAXSTR];

				for (auto &&value : extractedValues) {
					auto currentPos = 0;
					auto length = value.first.length();
					auto foundPos = script.find(value.first,currentPos);
					for (;foundPos != std::string::npos; foundPos = script.find(value.first,currentPos)) {
						currentPos = foundPos;
						script.replace(currentPos, length, value.second);
					}
				}

				auto actionStr = script.c_str();
				if (!ExecuteLine(actionStr, "matchedPatternAtEA\0", NULL, argc, arguments, NULL, error,
								 sizeof(error))) {
					error[MAXSTR - 1] = '\0';
					msg("failed to execute action with error: %s\n", error);
				}
			}
		}
		return true;
	};

	IdiomMatcher::Matching *matcher;
	int selectedButton = askbuttons_c("Naive","Control Flow Graph","Dependence Graph",ASKBTN_BTN1,"Select matching alorithm.");
	switch (selectedButton) {
		case ASKBTN_BTN1:
			matcher = new IdiomMatcher::NaiveMatching();
			break;
		case ASKBTN_BTN2:
			matcher = new IdiomMatcher::ControlFlowGraphMatching();
			break;
		case ASKBTN_BTN3:
			matcher = new IdiomMatcher::DependenceGraphMatching();
			break;
		default:
			return;
	}

	msg("Start matching with %s algorithm.\n",matcher->getName().c_str());

	auto t1 = std::chrono::high_resolution_clock::now();
	clock_t clockS = clock();
	matcher->searchForPatterns(allPatterns, idaAPI,callback);
	clock_t clockE = clock();
	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = t2 - t1;

	double cpuTime = (clockE-clockS)/(CLOCKS_PER_SEC*1.0);
	double realtime = diff.count();
	IdiomMatcher::msg("Matching finished in %f s CPU time, %f s real time.\n",cpuTime,realtime);

	// save matches to file
	IdiomMatcher::MatchPersistence persistence(idaAPI.executableName(),matcher->getName(),idaAPI.executableArchitecture(),realtime,cpuTime,matches);
	auto path = persistence.matchPathForExecutablePath(idaAPI.executablePath());
	if (persistence.saveToFilePath(path))
		IdiomMatcher::msg("Saved matches to %s\n",path.c_str());
	else
		IdiomMatcher::msg("Failed to save matches to %s\n",path.c_str());

	delete matcher;
}

void addedPatternCallback(const IdiomMatcher::Pattern& newPattern) {
	allPatterns.push_back(std::make_shared<IdiomMatcher::Pattern>(newPattern));
	info("Did add new pattern named: %s",newPattern.getName().c_str());
	patternsUnsafed = true;
}

void loadPatternsCallback(IDAAdapter::SimpleCallbackActionHandler* _) {


	if (patternsUnsafed && askyn_c(ASKBTN_NO,"Patterns aren't safed. Discard unsafed Patterns?") == ASKBTN_NO)
		return;

	auto jsonPath = jsonPathForFileName("IdiomMatcher.json");
	char *path = askfile2_c(false, jsonPath.c_str(), "IdiomMatcher JSON files|*.json", "Pattern file to load:");
	if (path != NULL) {
		allPatterns = IdiomMatcher::PatternPersistence::readFromFilePath(std::string(path));
		patternsUnsafed = false;
	}
}

void savePatternsCallback(IDAAdapter::SimpleCallbackActionHandler* _) {
	auto jsonPath = jsonPathForFileName("IdiomMatcher.json");
	if (ASKBTN_YES != askyn_c(ASKBTN_YES,"Save pattern file at %s?",jsonPath.c_str()))
		return;

	if (IdiomMatcher::PatternPersistence::writeToFilePath(jsonPath,allPatterns))
		patternsUnsafed = false;
}

void dumpCallback(IDAAdapter::SimpleCallbackActionHandler *) {
	IDAAdapter::IDA ida;
	auto jsonPath = ida.executablePath();
	jsonPath.append("_dump.json");

	if (ASKBTN_YES != askyn_c(ASKBTN_YES,"Save assembly dump to file at %s?",jsonPath.c_str()))
		return;

	IdiomMatcher::dumpDisassemblyToFilePath(jsonPath, ida);
}

static std::string jsonPathForFileName(std::string jsonName) {
	auto jsonpath = std::string(database_idb);
	size_t lastSlash = jsonpath.find_last_of("/");
	if (lastSlash != std::string::npos) {
		++lastSlash;
		jsonpath.erase(lastSlash);
	}
	jsonpath.append(jsonName);
	return jsonpath;
}


static error_t idaapi idc_func_dump(idc_value_t *argv,idc_value_t *r) {

	IDAAdapter::IDA ida;
	auto jsonPath = ida.executablePath();
	jsonPath.append("_dump.json");

	IdiomMatcher::dumpDisassemblyToFilePath(jsonPath, ida);

	return eOk;
}
