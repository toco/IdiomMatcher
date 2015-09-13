//
// Created by Tobias Conradi on 30.06.15.
// Licensed under MIT License, see LICENSE for full text.

#include "IDAUIActions.h"
#include "IDAAdapter.h"
#include <Model/PatternPersistence.h>
#include <loader.hpp>


namespace IDAAdapter {

    ActionHandler::ActionHandler(plugin_t *plugin, const std::string &identifier, const std::string &title, const std::string &tooltip, const std::string & shortcut, const bool addToMenu) :
    IDAAPI::UIHooks(), _actionIdentifier(identifier)
{
        action_desc_t actionDescription =
                ACTION_DESC_LITERAL_OWNER(
                        _actionIdentifier.c_str(), // identifier
                        title.c_str(), // title
                        this, // handler
                        plugin, // plugin
                        shortcut.c_str(), // shortcut
                        tooltip.c_str(), // tooltip
                        0 // icon id
                );
        if (!register_action(actionDescription)) {
			msg("failed to register action: %s\n", _actionIdentifier.c_str());
		} else if (addToMenu) {
			if (!attach_action_to_menu("Edit/Other/", identifier.c_str(),SETMENU_APP)) {
				msg("failed to attach to menu, action: %s\n", _actionIdentifier.c_str());
			}
		}

    }
    ActionHandler::~ActionHandler() {

		if (!detach_action_from_menu("Edit/Other", _actionIdentifier.c_str())) {
			msg("failed to detach from menu, action %s\n", _actionIdentifier.c_str());
		}

		if (!unregister_action(_actionIdentifier.c_str())) {
            msg("failed to unregister action: %s\n", _actionIdentifier.c_str());
        }
    }

    int ActionHandler::activate(action_activation_ctx_t *ctx) {
        return 0;
    }

    action_state_t ActionHandler::update(action_update_ctx_t *ctx) {
        return AST_ENABLE_ALWAYS;
    }

#pragma mark UIHooks

    void IDAAdapter::ActionHandler::finish_populating_tform_popup(TForm *form, TPopupMenu *menu) {
        attach_action_to_popup(form, menu, _actionIdentifier.c_str());
    }


#pragma mark - SimpleCallbackActionHandler
    SimpleCallbackActionHandler::SimpleCallbackActionHandler(plugin_t *plugin, const ActionActivateCallback &callback,
                                                             const std::string &identifier, const std::string &title,
															 const std::string &tooltip, const std::string &shortcut) :
            ActionHandler(plugin, identifier, title, tooltip, shortcut), _activationCallback(callback) {
    }

    int SimpleCallbackActionHandler::activate(action_activation_ctx_t *ctx) {

        if (_activationCallback != nullptr) {
            _activationCallback(this);
        } else {
            info("Failed to call activation callback for %s\n",_actionIdentifier.c_str());
        }
        return 0;
    }

#pragma mark - CreatePatternActionHandler

    CreatePatternActionHandler::CreatePatternActionHandler(plugin_t *plugin, AddedPatternCallback addedPatternCallback) :
            ActionHandler(plugin,"IdiomMatcher:CreatePattern","Create Pattern from selection", "Create new Pattern from selection for IdiomMatcher","ALT-CTRL-A"),
            _addedCallback(addedPatternCallback)
    { }

#pragma mark action_handler_t

    int CreatePatternActionHandler::activate(action_activation_ctx_t *ctx) {
        msg("Create pattern action activated\n");
        auto patternName = askstr(0, NULL, "Name for the pattern");
        if (patternName == NULL) {
            return 0;
        }

        ea_t sEA, eEA;
        read_selection(&sEA, &eEA);
        auto startEA = IdiomMatcher::EA(sEA);
        auto endEA = IdiomMatcher::EA(eEA);
        auto name = std::string(patternName);
        auto pattern = createPatternFromEAToEA(startEA,endEA,name);
        if (_addedCallback != nullptr) {
            _addedCallback(pattern);
        } else {
            info("Failed to create pattern\n");
        }

        return 0;
    }

    IdiomMatcher::Pattern CreatePatternActionHandler::createPatternFromEAToEA(IdiomMatcher::EA &startEA,
                                                                              IdiomMatcher::EA &endEA,
                                                                              std::string &name) {
        IDA ida;
        auto currentEA = startEA;
        ida.setCurrentEAAndDecodeInstruction(currentEA);
        IdiomMatcher::Instructions instructions;
        for (; currentEA < endEA; currentEA = ida.advanceInstruction()) {
            instructions.push_back(std::make_shared<IdiomMatcher::Instruction>(ida.getCurrentInstruction()));
        }
        IdiomMatcher::Pattern pattern(name, instructions,ida.executableArchitecture());
        return pattern;
    }
}
