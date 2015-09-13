//
// Created by Tobias Conradi on 05.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_STRINGSWITCHSEARCH_H
#define IDIOMMATCHER_STRINGSWITCHSEARCH_H
#include <IDA/IDAUIActions.h>

struct SwitchStringSearch : IDAAdapter::SimpleCallbackActionHandler{
    SwitchStringSearch(plugin_t *plugin);
};


#endif //IDIOMMATCHER_STRINGSWITCHSEARCH_H
