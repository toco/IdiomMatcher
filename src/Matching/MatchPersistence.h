//
// Created by Tobias Conradi on 20.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_MATCHPERSISTENCE_H
#define IDIOMMATCHER_MATCHPERSISTENCE_H

#include <string>
#include <Model/Pattern.h>
namespace IdiomMatcher {

    class Match {
        const EA _startEA;
        const EA _endEA;
        const std::string _patternName;

    public:
        Match(const EA &startEA, const EA &endEA, const std::string &patternName) : _startEA(startEA),
                                                                                       _endEA(endEA),
                                                                                       _patternName(patternName) { }

        const EA &getStartEA() const { return _startEA; }
        const EA &getEndEA() const { return _endEA; }
        const std::string &getPatternName() const { return _patternName; }
    };

    typedef std::shared_ptr<Match> Match_Ref;
    typedef std::vector<Match_Ref> Matches;

    class MatchPersistence {
        const std::string _executableName;
        const std::string _matcherName;
		const std::string _executableArchitecture;
        const double _realTime;
        const double _cpuTime;
        const Matches _matches;

    public:
        const std::string &getExectuableName() const { return _executableName; }
		const std::string &getExecutableArchitecture() const { return _executableArchitecture; }
        const std::string &getMatcherName() const { return _matcherName; }
        double getRealTime() const { return _realTime; }
        double getCpuTime() const { return _cpuTime; }
        const Matches &getMatches() const { return _matches; }

		MatchPersistence(const std::string &executableName, const std::string &matcherName, const std::string &exectuableArchitecture, double realTime, double cpuTime, const Matches &matches)
                : _executableName(executableName), _matcherName(matcherName), _executableArchitecture(exectuableArchitecture), _realTime(realTime), _cpuTime(cpuTime), _matches(matches) { }

        bool saveToFilePath(const std::string &path) const;
        std::string matchPathForExecutablePath(const std::string &path) const;
    };
}

#endif //IDIOMMATCHER_MATCHPERSISTENCE_H
