//
// Created by Tobias Conradi on 13.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_DUMPDISASSEMBLERAPI_H
#define IDIOMMATCHER_DUMPDISASSEMBLERAPI_H
#include <Matching/DisassemblerAPI.h>
#include <Model/DisassemblyPersistence.h>
#include <map>

class DumpDisassemblerAPI : public IdiomMatcher::DisassemblerAPI {
public:
    DumpDisassemblerAPI(const std::string &path);
    DumpDisassemblerAPI(const IdiomMatcher::DisassemblyDocument &document, const std::string &documentPath= "");

    virtual IdiomMatcher::EA minEA() const override;

    virtual IdiomMatcher::EA maxEA() const override;

    virtual IdiomMatcher::EA nextEA(const IdiomMatcher::EA &ea) const override;

	virtual IdiomMatcher::EA minInstructionEA() const;

	virtual IdiomMatcher::EA maxInstructionEA() const;

    virtual IdiomMatcher::Instruction instructionForEA(const IdiomMatcher::EA &instructionEA) const override;

    virtual void setCurrentEAAndDecodeInstruction(const IdiomMatcher::EA ea) override;

    virtual std::string commentForEA(const IdiomMatcher::EA &instructionEA) const override;

    virtual std::string executableName() const override;

    virtual std::string executablePath() const override;

    virtual std::string executableArchitecture() const override;

    virtual std::string getDisassemblerName() const override;

private:

    IdiomMatcher::DisassemblyLine_Ref lineForEA(const IdiomMatcher::EA &ea) const;

    const std::string _path;
    std::map<IdiomMatcher::EA, IdiomMatcher::DisassemblyLine_Ref> _eaToLineMap;
    IdiomMatcher::DisassemblyDocument _document;
};


#endif //IDIOMMATCHER_DUMPDISASSEMBLERAPI_H
