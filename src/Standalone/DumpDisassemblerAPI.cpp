//
// Created by Tobias Conradi on 13.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include <string>
#include <memory>
#include <iosfwd>
#include <bitset>
#include "DumpDisassemblerAPI.h"

using namespace IdiomMatcher;

DumpDisassemblerAPI::DumpDisassemblerAPI(const std::string &path) : DumpDisassemblerAPI(readDocumentFromFilePath(path),path) { }

DumpDisassemblerAPI::DumpDisassemblerAPI(const IdiomMatcher::DisassemblyDocument &document, const std::string &documentPath) : DisassemblerAPI(InvalidEA, InvalidInstruction), _path(documentPath), _document(document) {
    for (auto line : _document.getDisassemblyLines()) {
        _eaToLineMap.emplace(line->getEA(),line);
    }
}

EA DumpDisassemblerAPI::minEA() const {
    return _document.getMinEA();
}

EA DumpDisassemblerAPI::maxEA() const {
    return _document.getMaxEA();
}

EA DumpDisassemblerAPI::nextEA(const EA &ea) const {
	// returns the first ea that is bigger then ea
	auto iterator = _eaToLineMap.upper_bound(ea);
	if (iterator != _eaToLineMap.end()) {
		return iterator->first;
	}
	return InvalidEA;
}


IdiomMatcher::EA DumpDisassemblerAPI::minInstructionEA() const {
	if (_eaToLineMap.begin() == _eaToLineMap.end()) {
		return InvalidEA;
	} else {
		auto it = _eaToLineMap.begin();
		return it->first;
	}
}

IdiomMatcher::EA DumpDisassemblerAPI::maxInstructionEA() const {
	if (_eaToLineMap.rbegin() == _eaToLineMap.rend()) {
		return InvalidEA;
	} else {
		auto it = _eaToLineMap.rbegin();
		return it->first;
	}
}

Instruction DumpDisassemblerAPI::instructionForEA(const EA &instructionEA) const {
    auto line = lineForEA(instructionEA);
    return line ? *(line->getInstruction()) : InvalidInstruction;
}

std::string DumpDisassemblerAPI::commentForEA(const EA &instructionEA) const {
    auto line = lineForEA(instructionEA);
    return line ? line->getComment() : "";
}

std::string DumpDisassemblerAPI::executableName() const {
    return _document.getBinaryName();
}

std::string DumpDisassemblerAPI::executablePath() const {
    return _path;
}

std::string DumpDisassemblerAPI::executableArchitecture() const {
    return _document.getArchitectureName();
}

std::string DumpDisassemblerAPI::getDisassemblerName() const {
    return _document.getDissassembler();
}

DisassemblyLine_Ref DumpDisassemblerAPI::lineForEA(const EA &ea) const {
    auto iterator = _eaToLineMap.find(ea);
    DisassemblyLine_Ref lineRef = nullptr;
    if (iterator != _eaToLineMap.end()) {
        return iterator->second;
    }
    return lineRef;
}

void DumpDisassemblerAPI::setCurrentEAAndDecodeInstruction(const IdiomMatcher::EA ea) {
    auto line = lineForEA(ea);
	_currentEA = ea;
    _currentInstruction = line ? *(line->getInstruction()) : InvalidInstruction;
    _currentComment = line ? line->getComment() : "";
}
