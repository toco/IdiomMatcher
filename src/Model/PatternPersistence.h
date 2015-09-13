//
// Created by Tobias Conradi on 26.06.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_PATTERNPERSISTENCE_H
#define IDIOMMATCHER_PATTERNPERSISTENCE_H

#include <Model/Pattern.h>
#include <Model/DisassemblyPersistence.h>


namespace IdiomMatcher {

    class PatternPersistence {
    public:
        static Patterns readFromFilePath(const std::string &path);
        static bool writeToFilePath(const std::string &path, const Patterns &patterns);

        static const int documentVersion = 2;
    };

    Action_Ref actionFromJSON(const JSONValue &value);

    template <typename JSONWriter>
    void SerializeAction(JSONWriter &writer, const Action &action);

    Pattern_ref patternFromJSON(const JSONValue &value);

    template <typename JSONWriter>
    void SerializePattern(JSONWriter &writer, const Pattern &pattern);


}

#endif //IDIOMMATCHER_PATTERNPERSISTENCE_H
