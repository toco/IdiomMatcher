//
// Created by Tobias Conradi on 07.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include "Matching.h"
namespace IdiomMatcher {

	void Matching::searchForPatterns(const Patterns &patterns,
									 DisassemblerAPI &disassemblerAPI,
									 const FoundMatchFunctionCallback &callback,
									 const EA &startEA,
									 const EA &endEA) {

		IdiomMatcher::EA currentEA = startEA;
		IdiomMatcher::EA maxEA = endEA;

		if (patterns.empty()) {
			return;
		}
		while (currentEA < maxEA) {
			testForPatternsStartingAtEA(patterns, currentEA, disassemblerAPI, callback);
			currentEA = disassemblerAPI.nextEA(currentEA);
		}
	}

    void Matching::searchForPatterns(const Patterns &patterns, DisassemblerAPI &disassemblerAPI, const FoundMatchFunctionCallback &callback) {
		searchForPatterns(patterns, disassemblerAPI, callback, disassemblerAPI.minEA(), disassemblerAPI.maxEA());
    }

    bool Matching::testInstructionsMatch(const Instruction &patternInstr, const Instruction &dissInstruction, ExtractedValuesMap *externalExtractedValuesMap, PatternNameMap *patternNameMap) const {

		bool mnenomicMatch = false;
        if (patternInstr.getIsRegex()) {
			mnenomicMatch = std::regex_match(dissInstruction.getMnemonic(), std::regex(patternInstr.getMnemonic()));
		} else {
			mnenomicMatch = patternInstr.getMnemonic() == dissInstruction.getMnemonic();
		}

		if (!mnenomicMatch) {
			return false;
		}

        ExtractedValuesMap extractedValues;
        bool doExtract = (externalExtractedValuesMap != nullptr);
		bool doPatternName = (patternNameMap != nullptr);

        int operandIndex = 0;
        auto &instructionOperands = dissInstruction.getOperands();
        auto dissOperandCount = instructionOperands.size();
        bool matchedOperands = true;
        for (auto &operand : patternInstr.getOperands()) {
            if (dissOperandCount <= operandIndex) {
                matchedOperands = false;
                break;
            }
			auto &disassembledOperand = instructionOperands[operandIndex];

			auto disassembledOperandText = disassembledOperand->getText();
            auto regexString = operand->getRegex();
            if (!regexString.empty()) {
                bool matched = std::regex_match(disassembledOperandText, std::regex(regexString));
                if (!matched) {
                    matchedOperands = false;
                    break;
                }
            }

			if ((operand->getNameIsTemplate() && doPatternName) || !operand->getNameIsTemplate() ) {
				int regIndex = 0;
				auto disassembledRegs = disassembledOperand->getRegisters();
				auto disassembledRegsCount = disassembledRegs.size();
				for (auto regName : operand->getRegisters()) {
					std::string disassembledRegName;
					if (regIndex < disassembledRegsCount) {
						disassembledRegName = disassembledRegs[regIndex];
					} else if (disassembledOperand->getAddress() != 0) {
						disassembledRegName = std::to_string(disassembledOperand->getAddress());
					} else {
						disassembledRegName = disassembledOperandText;
					}

					if (operand->getNameIsTemplate() && doPatternName) {
						auto patternIt = patternNameMap->find(regName);
						if (patternIt != patternNameMap->end()) {
							if (patternIt->second != disassembledRegName) {
								matchedOperands = false;
								break;
							}
						} else {
							patternNameMap->insert(std::make_pair(regName, disassembledRegName));
						}
					} else {
						if (!regName.empty() && regName != disassembledRegName) {
							matchedOperands = false;
							break;
						}
					}

					regIndex++;
				}
				if (!matchedOperands) {
					break;
				}
			}

            if (doExtract) {
                auto extractAs = operand->getExtractAs();
                if (!extractAs.empty() && operandIndex < dissOperandCount) {
                    extractedValues[extractAs] = disassembledOperandText;
                }
            }
            operandIndex++;
        }

        if (matchedOperands && doExtract) {
            externalExtractedValuesMap->insert(extractedValues.cbegin(),extractedValues.cend());
        }

        return matchedOperands;
    }
}