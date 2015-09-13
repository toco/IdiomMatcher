//
// Created by Tobias Conradi on 02.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_DISASSEMBLERAPI_H
#define IDIOMMATCHER_DISASSEMBLERAPI_H

#include <Model/Pattern.h>

namespace IdiomMatcher {


    class DisassemblerAPI {
	protected:
		EA _currentEA;
		Instruction _currentInstruction;
		std::string _currentComment;
		DisassemblerAPI(const EA &ea, const Instruction &instruction) : _currentEA(ea), _currentInstruction(instruction) {};
	public:
		virtual EA minEA() const = 0;

		virtual EA maxEA() const = 0;

		virtual EA nextEA(const EA &ea) const = 0;

		virtual Instruction instructionForEA(const IdiomMatcher::EA &instructionEA) const = 0;

		virtual std::string commentForEA(const IdiomMatcher::EA &instructionEA) const = 0;

		virtual EA getCurrentEA() const { return _currentEA; }

		virtual Instruction getCurrentInstruction() const { return _currentInstruction; }

		virtual EA advanceInstruction();

		virtual void setCurrentEAAndDecodeInstruction(const IdiomMatcher::EA ea);

		virtual std::string getCurrentComment() const { return _currentComment; }

		virtual std::string executableName() const = 0;

		virtual std::string executablePath() const = 0;

		virtual std::string executableArchitecture() const = 0;

		virtual std::string getDisassemblerName() const = 0;

		virtual ~DisassemblerAPI() {};
	};

}


#endif //IDIOMMATCHER_DISASSEMBLERAPI_H
