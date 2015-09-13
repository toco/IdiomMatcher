//
// Created by Tobias Conradi on 25.06.15.
// Licensed under MIT License, see LICENSE for full text.


#ifndef IDIOMMATCHER_IDAADAPTER_H
#define IDIOMMATCHER_IDAADAPTER_H

#include <Matching/DisassemblerAPI.h>

namespace IDAAdapter {

	class IDA : public IdiomMatcher::DisassemblerAPI {
	public:
		IDA(const IdiomMatcher::EA &ea = IdiomMatcher::EA(0)) : IdiomMatcher::DisassemblerAPI(ea, instructionForEA(ea)) { };

		virtual IdiomMatcher::EA minEA() const override;

		virtual IdiomMatcher::EA maxEA() const override;

		virtual IdiomMatcher::EA nextEA(const IdiomMatcher::EA &ea) const override;

		virtual IdiomMatcher::Instruction instructionForEA(const IdiomMatcher::EA &instructionEA) const override;

		virtual std::string commentForEA(const IdiomMatcher::EA &instructionEA) const override;

		virtual std::string executableName() const override;

		virtual std::string executablePath() const override;

		virtual std::string executableArchitecture() const override;

		virtual std::string getDisassemblerName() const override;
	};

};

#endif //IDIOMMATCHER_IDAADAPTER_H
