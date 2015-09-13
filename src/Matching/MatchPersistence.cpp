//
// Created by Tobias Conradi on 20.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include "MatchPersistence.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/encodedstream.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

namespace IdiomMatcher {

    typedef rapidjson::GenericValue<rapidjson::ASCII<>, rapidjson::MemoryPoolAllocator<> > Value;

    template<typename Writer>
    void SerializeMatch(Writer &writer, const Match &match) {
        writer.StartObject();
        writer.Key("startEA");
        writer.Uint(match.getStartEA().getValue());
        writer.Key("endEA");
        writer.Uint(match.getEndEA().getValue());
        writer.Key("patternName");
        writer.String(match.getPatternName());
        writer.EndObject();
    }

    Match_Ref MatchFromJSON(const Value &value) {
        auto startEA = EA(value["startEA"].GetUint());
        auto endEA = EA(value["endEA"].GetUint());
        auto patternName = value["patternName"].GetString();
        return std::make_shared<Match>(startEA,endEA,patternName);
    }

    template<typename Writer>
    void SerializeMatchPersistence(Writer &writer, const MatchPersistence &matchPersistence) {
        writer.StartObject();
        writer.Key("executableName");
        writer.String(matchPersistence.getExectuableName());
        writer.Key("matcherName");
        writer.String(matchPersistence.getMatcherName());
		writer.Key("executableArchitecture");
		writer.String(matchPersistence.getExecutableArchitecture());
        writer.Key("realTime");
        writer.Double(matchPersistence.getRealTime());
        writer.Key("cpuTime");
        writer.Double(matchPersistence.getCpuTime());
        writer.Key("matches");
        writer.StartArray();
        for (auto &match : matchPersistence.getMatches()) {
            SerializeMatch(writer,*match);
        }
        writer.EndArray();
        writer.EndObject();
    }

    MatchPersistence MatchPersistenceFromJSON(const Value &value) {
        auto executableName = value["executableName"].GetString();
        auto matcherName = value["matcherName"].GetString();
		auto executableArchitecture = value["executableArchitecture"].GetString();
		auto realTime = value["realTime"].GetDouble();
        auto cpuTime = value["cpuTime"].GetDouble();

        Matches matches;
        auto &matchesValue = value["matches"];
        for (rapidjson::SizeType i = 0; i < matchesValue.Size(); ++i) {
            auto match = MatchFromJSON(matchesValue[i]);
            if (match) {
                matches.push_back(match);
            }
        }
        return MatchPersistence(executableName,matcherName,executableArchitecture,realTime,cpuTime,matches);
    }

    bool MatchPersistence::saveToFilePath(const std::string &path) const {

        using namespace rapidjson;

        char writeBuffer[65536];
        auto fh = fopen(path.c_str(),"w");
        if (fh == NULL)
            return false;

        FileWriteStream os(fh, writeBuffer, sizeof(writeBuffer));
        typedef EncodedOutputStream<ASCII<>,FileWriteStream> OutputStream;
        OutputStream eos(os, true);
        Writer<OutputStream, ASCII<>, ASCII<> > writer(eos);

        SerializeMatchPersistence(writer,*this);

        eos.Flush();
        os.Flush();
        fclose(fh);
        return true;
    }

    std::string MatchPersistence::matchPathForExecutablePath(const std::string &binaryPath) const {
        auto jsonpath = std::string(binaryPath);
        size_t lastPoint = jsonpath.find_last_of(".");
        if (lastPoint != std::string::npos) {
            jsonpath.erase(lastPoint);
        }
        jsonpath.append("_matched_");
        jsonpath.append(_matcherName);
        jsonpath.append(".json");
        return jsonpath;
    }
}