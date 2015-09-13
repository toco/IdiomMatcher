//
// Created by Tobias Conradi on 12.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_DISASSEMBLYPERSISTENCE_H
#define IDIOMMATCHER_DISASSEMBLYPERSISTENCE_H

#include <Model/Pattern.h>
#include <Matching/DisassemblerAPI.h>

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/encodedstream.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>

namespace IdiomMatcher {

    class DisassemblyLine {
        EA _ea;
        Instruction_ref _instruction;
        std::string _comment;
        // TODO: data?
    public:
        DisassemblyLine(const EA &ea, const Instruction_ref &instruction, const std::string &comment) : _ea(ea), _instruction(instruction), _comment(comment) { }

        const EA getEA() const { return  _ea; }
        const Instruction_ref getInstruction() const { return _instruction; }
        const std::string getComment() const { return _comment; }
    };
    typedef std::shared_ptr<DisassemblyLine> DisassemblyLine_Ref;
    typedef std::vector<DisassemblyLine_Ref> DisassemblyLines;

    class DisassemblyDocument {
    public:


        DisassemblyDocument(const std::string &binaryName, const std::string &architectureName,
                            const std::string &dissassembler, const EA &minEA, const EA &maxEA,
                            const DisassemblyLines &disassemblyLines) :
                _binaryName(binaryName),
                _architectureName(architectureName),
                _dissassembler(dissassembler), _minEA(minEA),
                _maxEA(maxEA),
                _disassemblyLines(disassemblyLines) { }
        DisassemblyDocument() {}

        static DisassemblyDocument readFromFilePath(const std::string &filePath);
        bool writeToFilePath(const std::string &path);

        const DisassemblyLines &getDisassemblyLines() const { return _disassemblyLines; }
        const std::string &getBinaryName() const { return _binaryName; }
        const std::string &getArchitectureName() const { return _architectureName; }
        const std::string &getDissassembler() const { return _dissassembler; }

        const EA &getMinEA() const { return _minEA; }
        const EA &getMaxEA() const { return _maxEA; }

        static const int documentVersion = 2;
    private:

        std::string _binaryName;
        std::string _architectureName;
        std::string _dissassembler;

        EA _minEA = EA(0);
        EA _maxEA = EA(0);

        DisassemblyLines _disassemblyLines;

    };
    const DisassemblyDocument invalidDocument = DisassemblyDocument();

    bool dumpDisassemblyToFilePath(const std::string &path, DisassemblerAPI &api);
    DisassemblyDocument readDocumentFromFilePath(const std::string &path);


	template<typename JSONWriter>
	void dumpDisassemblyToWithWriter(JSONWriter &writer, DisassemblerAPI &api);
	template <typename JSONWriter>
	void SerializeDisassemblyDocument(JSONWriter &writer, const DisassemblyDocument &doc);

	template <typename JSONWriter>
	void SerializeDisassemblyLine(JSONWriter &writer, const DisassemblyLine &line);
	template <typename JSONWriter>
	void SerializeInstruction(JSONWriter &writer, const Instruction &ins);
	template <typename JSONWriter>
	void SerializeXRef(JSONWriter &writer, const XRef &xref);
	template <typename JSONWriter>
	void SerializeOperand(JSONWriter &writer, const Operand &op);

	typedef rapidjson::GenericValue<rapidjson::ASCII<>, rapidjson::MemoryPoolAllocator<> > JSONValue;
	Operand_Ref operandFromJSON(const JSONValue &value);
	XRef_Ref xrefFromJSON(const JSONValue &value);
	Instruction_ref instructionFromJSON(const JSONValue &value);
	DisassemblyLine_Ref disassemblyLineFromJSON(const JSONValue &value);
	DisassemblyDocument documentFromJSON(const JSONValue &value);
}

#endif //IDIOMMATCHER_DISASSEMBLYPERSISTENCE_H
