//
// Created by Tobias Conradi on 06.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_ADDMISSINGSWITCHPATTERNS_H
#define IDIOMMATCHER_ADDMISSINGSWITCHPATTERNS_H


#include <IDA/IDAUIActions.h>

class AddMissingSwitchPatterns : public IDAAdapter::SimpleCallbackActionHandler {
    IdiomMatcher::Patterns *const _patterns;
    static void addMissingSwitchPatterns(IDAAdapter::SimpleCallbackActionHandler *action);
    uint counter = 0;
public:
    AddMissingSwitchPatterns(plugin_t *plugin, IdiomMatcher::Patterns *patterns);
};

#endif //IDIOMMATCHER_ADDMISSINGSWITCHPATTERNS_H
