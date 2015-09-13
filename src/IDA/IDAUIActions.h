//
// Created by Tobias Conradi on 30.06.15.
// Licensed under MIT License, see LICENSE for full text.


#ifndef IDIOMMATCHER_IDAUIACTIONS_H
#define IDIOMMATCHER_IDAUIACTIONS_H

#include <functional>
#include <Model/Pattern.h>
#include "UIHooks.h"

namespace IDAAdapter {


    struct ActionHandler : public action_handler_t, public IDAAPI::UIHooks {

        ActionHandler(plugin_t *plugin, const std::string &identifier, const std::string &title = "", const std::string &tooltip = "", const std::string &shortcut = "", const bool addToMenu = true );
        virtual ~ActionHandler();

        virtual int activate(action_activation_ctx_t *ctx) override;

        // default: AST_ENABLE_ALWAYS
        virtual action_state_t update(action_update_ctx_t *ctx) override;

        virtual void finish_populating_tform_popup(TForm *form, TPopupMenu *menu) override;

    protected:
        const std::string _actionIdentifier;
    };

    struct SimpleCallbackActionHandler : ActionHandler {
        typedef std::function<void(IDAAdapter::SimpleCallbackActionHandler*)> ActionActivateCallback;

        SimpleCallbackActionHandler(plugin_t *plugin, const ActionActivateCallback &callback, const std::string &identifier, const std::string &title,
                                    const std::string &tooltip = "", const std::string &shortcut = "");
        virtual int activate(action_activation_ctx_t *ctx) override;

    private:
        const ActionActivateCallback _activationCallback;
    };

    struct CreatePatternActionHandler : ActionHandler {
        typedef std::function<void(const IdiomMatcher::Pattern&)> AddedPatternCallback;
        CreatePatternActionHandler(plugin_t *plugin, AddedPatternCallback);

        virtual int activate(action_activation_ctx_t *ctx) override;

        static IdiomMatcher::Pattern createPatternFromEAToEA(IdiomMatcher::EA& startEA, IdiomMatcher::EA &endEA, std::string &name);

    private:
        const AddedPatternCallback _addedCallback;
    };

}
#endif //IDIOMMATCHER_IDAUIACTIONS_H
