//
// Created by Tobias Conradi on 29.06.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_LOGGING_H
#define IDIOMMATCHER_LOGGING_H

#include <functional>

namespace IdiomMatcher {
    typedef int (*LoggingFunction)(const char *format, ...);
    typedef void (*AlertFunction)(const char *format, ...);

    extern AlertFunction warning;
    extern AlertFunction info;
    extern LoggingFunction msg;
}

#endif //IDIOMMATCHER_LOGGING_H
