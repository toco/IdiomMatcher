//
// Created by Tobias Conradi on 02.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include "DisassemblerAPI.h"

namespace IdiomMatcher {


    EA DisassemblerAPI::advanceInstruction() {
        _currentEA = nextEA(_currentEA);
        _currentInstruction = instructionForEA(_currentEA);
		_currentComment = commentForEA(_currentEA);
        return _currentEA;
    }

    void DisassemblerAPI::setCurrentEAAndDecodeInstruction(const IdiomMatcher::EA ea) {
        _currentEA = ea;
        _currentInstruction = instructionForEA(ea);
        _currentComment = commentForEA(ea);
    }

}