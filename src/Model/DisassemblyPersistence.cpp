//
// Created by Tobias Conradi on 12.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include "DisassemblyPersistence.h"
#include "Logging.h"

#include <boost/property_tree/json_parser.hpp>
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/prettywriter.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/encodedstream.h>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>


namespace IdiomMatcher {

#pragma mark - JSON Writing

    // write document head, callee must call writer.EndObject() later
	template<typename JSONWriter>
    static inline void serializeDocumentHead(JSONWriter &writer, const std::string &binName, const std::string &archName, const std::string &disName, const EA &minEA, const EA &maxEA) {
        writer.StartObject();
        writer.String("version");
        writer.Uint(DisassemblyDocument::documentVersion);
        writer.String("binaryName");
        writer.String(binName);
        writer.String("architecture");
        writer.String(archName);
        writer.String("disassembler");
        writer.String(disName);
        writer.String("minEA");
        writer.Uint(minEA.getValue());
        writer.String("maxEA");
        writer.Uint(maxEA.getValue());
    }

	template<typename JSONWriter>
    void SerializeDisassemblyDocument(JSONWriter &writer, const DisassemblyDocument &doc) {
        serializeDocumentHead(writer,doc.getBinaryName(),doc.getArchitectureName(),doc.getDissassembler(),doc.getMinEA(),doc.getMaxEA());

		writer.String("disassembly");
		writer.StartArray();

		for (auto line : doc.getDisassemblyLines()) {
            SerializeDisassemblyLine(writer,*line);
        }

		writer.EndArray();
        writer.EndObject();
    }

	template<typename JSONWriter>
    void dumpDisassemblyToWithWriter(JSONWriter &writer, DisassemblerAPI &api) {
        serializeDocumentHead(writer,api.executableName(),api.executableArchitecture(),api.getDisassemblerName(),api.minEA(),api.maxEA());

		writer.String("disassembly");
        writer.StartArray();

		EA startEA = api.minEA();
		EA endEA = api.maxEA();

		api.setCurrentEAAndDecodeInstruction(startEA);

        for (EA ea = api.minEA(); ea <= endEA; ea = api.advanceInstruction()) {
            auto instruction = api.getCurrentInstruction();
            if (instruction.getSize() == 0) continue; // ignore empty instructions
            auto line = DisassemblyLine(ea,std::make_shared<Instruction>(instruction),api.getCurrentComment());
            SerializeDisassemblyLine(writer,line);
        }

        writer.EndArray();
        writer.EndObject();
    }

    bool dumpDisassemblyToFilePath(const std::string &path, DisassemblerAPI &api) {

        using namespace rapidjson;

        char writeBuffer[65536];
        auto fh = fopen(path.c_str(),"w");
        if (fh == NULL)
            return false;

        FileWriteStream os(fh, writeBuffer, sizeof(writeBuffer));

        typedef EncodedOutputStream<ASCII<>,FileWriteStream> JSONOutputStream;
		typedef rapidjson::Writer<JSONOutputStream, rapidjson::ASCII<>, rapidjson::ASCII<> > JSONWriter;
		JSONOutputStream eos(os, true);
		JSONWriter writer(eos);

        dumpDisassemblyToWithWriter(writer,api);

        eos.Flush();
        os.Flush();
        fclose(fh);
        return true;
    }

	template<typename JSONWriter>
    void SerializeOperand(JSONWriter &writer, const Operand &op) {
        writer.StartObject();
        writer.Key("text");
        writer.String(op.getText());

        if (!op.getRegisters().empty() || serializeDefaultValues) {
            writer.Key("regs");
            writer.StartArray();
            for (auto &reg : op.getRegisters()) {
                writer.String(reg);
            }
            writer.EndArray();
        }

        if (!op.getExtractAs().empty() || serializeDefaultValues) {
            writer.String("extractAs");
            writer.String(op.getExtractAs());
        }

        if (!op.getRegex().empty() || serializeDefaultValues) {
            writer.String("regex");
            writer.String(op.getRegex());
        }
        if (op.getNameIsTemplate() || serializeDefaultValues) {
            writer.String("nameIsTemplate");
            writer.Bool(op.getNameIsTemplate());
        }
        if (!op.getUsed() || serializeDefaultValues) {
            writer.String("used");
            writer.Bool(op.getUsed());
        }
        if (op.getModified() || serializeDefaultValues) {
            writer.String("modified");
            writer.Bool(op.getModified());
        }
		if (op.getAddress()>0 || serializeDefaultValues) {
			writer.String("address");
			writer.Uint(op.getAddress());
		}
        writer.EndObject();
    }

	template<typename JSONWriter>
	void SerializeXRef(JSONWriter &writer, const XRef &xref) {
		writer.StartObject();
		writer.String("target");
		writer.Uint(xref.getTarget().getValue());
		if (xref.isData() || serializeDefaultValues) {
			writer.String("isData");
			writer.Bool(xref.isData());
		}
		if (xref.isUnordinaryFlow() || serializeDefaultValues) {
			writer.Key("isUnordinaryFlow");
			writer.Bool(xref.isUnordinaryFlow());
		}
		writer.EndObject();
	}

	template<typename JSONWriter>
    void SerializeInstruction(JSONWriter &writer, const Instruction &ins) {
        writer.StartObject();
        writer.String("mnem");
        writer.String(ins.getMnemonic());
        if (ins.getSize()>0 || serializeDefaultValues) {
            writer.String("size");
            writer.Uint(ins.getSize());
        }
        if (ins.getOperands().size()>0 || serializeDefaultValues) {
            writer.String("ops");
            writer.StartArray();
            for (auto operand : ins.getOperands()) {
                SerializeOperand(writer,*operand);
            }
            writer.EndArray();
        }
		if (ins.getXrefs().size()>0 || serializeDefaultValues) {
			writer.String("xrefs");
			writer.StartArray();
			for (auto xref : ins.getXrefs()) {
				SerializeXRef(writer,*xref);
			}
			writer.EndArray();
		}
		if (!(ins.getEA() == InvalidEA) || serializeDefaultValues) {
			writer.Key("ea");
			writer.Uint(ins.getEA().getValue());
		}
		if (ins.getIsRegex() || serializeDefaultValues) {
			writer.Key("isRegex");
			writer.Bool(ins.getIsRegex());
		}

        writer.EndObject();
    }

	template<typename JSONWriter>
    void SerializeDisassemblyLine(JSONWriter &writer, const DisassemblyLine &line) {
        writer.StartObject();
        writer.String("ea");
        writer.Uint(line.getEA().getValue());
        writer.String("instruction");
        SerializeInstruction(writer,*line.getInstruction());
        if (!line.getComment().empty() || serializeDefaultValues) {
            writer.String("comment");
            writer.String(line.getComment());
        }
        writer.EndObject();
    }


#pragma mark - JSON reading

    Operand_Ref operandFromJSON(const JSONValue &value) {
        auto name = value["text"].GetString();

        std::vector<std::string> registers;
        if (value.HasMember("regs")) {
            auto &opsValue = value["regs"];
            for (rapidjson::SizeType i = 0; i < opsValue.Size(); ++i) {
                auto reg = opsValue[i].GetString();
                registers.push_back(reg);
            }
        }

        auto extractAs = "";
        if (value.HasMember("extractAs")) {
            extractAs = value["extractAs"].GetString();
        }
        auto regex = "";
        if (value.HasMember("regex")) {
            regex = value["regex"].GetString();
        }
        bool nameIsTemplate = false;
        if (value.HasMember("nameIsTemplate")) {
            nameIsTemplate = value["nameIsTemplate"].GetBool();
        }
        bool used = true;
        if (value.HasMember("used")) {
            used = value["used"].GetBool();
        }
        bool modified = false;
        if (value.HasMember("modified")) {
            modified = value["modified"].GetBool();
        }
		uint address = 0;
		if (value.HasMember("address")) {
			address = value["address"].GetUint();
		}
        return std::make_shared<Operand>(name,registers,extractAs,regex,nameIsTemplate,used,modified,address);
    }

	XRef_Ref xrefFromJSON(const JSONValue &value) {
		auto ea = EA(value["target"].GetUint());
		bool isData = false;
		if (value.HasMember("isData")) {
			isData = value["isData"].GetBool();
		}

		bool isUnordinaryFlow = false;
		if (value.HasMember("isUnordinaryFlow")) {
			isUnordinaryFlow = value["isUnordinaryFlow"].GetBool();
		}

		return std::make_shared<XRef>(ea,isData, isUnordinaryFlow);
	}

    Instruction_ref instructionFromJSON(const JSONValue &value) {
        auto mnemonic = value["mnem"].GetString();
        uint size = 0;
        if (value.HasMember("size")) {
            size = value["size"].GetUint();
        }
        Operands operands;
        if (value.HasMember("ops")) {
            auto &opsValue = value["ops"];
            for (rapidjson::SizeType i = 0; i < opsValue.Size(); ++i) {
                auto op = operandFromJSON(opsValue[i]);
                if (op) {
                    operands.push_back(op);
                }
            }
        }
		XRefs xrefs;
		if (value.HasMember("xrefs")) {
			auto &xrefsValue = value["xrefs"];
			for (rapidjson::SizeType i = 0; i < xrefsValue.Size(); ++i) {
				auto ref = xrefFromJSON(xrefsValue[i]);
				if (ref) {
					xrefs.push_back(ref);
				}
			}
		}
		EA ea = InvalidEA;
		if (value.HasMember("ea")) {
			auto &eaValue = value["ea"];
			ea = EA(eaValue.GetUint());
		}
		bool isRegex = false;
		if (value.HasMember("isRegex")) {
			auto &isRegexValue = value["isRegex"];
			isRegex = isRegexValue.GetBool();
		}
        return std::make_shared<Instruction>(mnemonic,operands,xrefs,size,ea,isRegex);
    }
    DisassemblyLine_Ref disassemblyLineFromJSON(const JSONValue &value) {
        auto ea = EA(value["ea"].GetUint());
        std::string comment;
        if (value.HasMember("comment")) {
            comment = value["comment"].GetString();
        }
        Instruction_ref instruction = instructionFromJSON(value["instruction"]);

		// fix for old style json files not including the EA in the instruction
		if (instruction->getEA() == InvalidEA && !(ea == InvalidEA)) {
			instruction = std::make_shared<Instruction>(instruction->getMnemonic(),instruction->getOperands(),instruction->getXrefs(),instruction->getSize(),ea);
		}
        return std::make_shared<DisassemblyLine>(ea,instruction,comment);
    }

    DisassemblyDocument documentFromJSON(const JSONValue &value) {
        auto version = value["version"].GetUint();
        if (version != DisassemblyDocument::documentVersion) {
            return invalidDocument;
        }

        auto binaryName = value["binaryName"].GetString();
        auto architecture = value["architecture"].GetString();
        auto disassembler = value["disassembler"].GetString();
        auto minEA = value["minEA"].GetUint();
        auto maxEA = value["maxEA"].GetUint();
        auto &linesValue = value["disassembly"];
        DisassemblyLines lines;
        for (rapidjson::SizeType i = 0; i < linesValue.Size(); ++i) {
            auto line = disassemblyLineFromJSON(linesValue[i]);
            if (line) {
                lines.push_back(line);
            }
        }
        return DisassemblyDocument(binaryName,architecture,disassembler,EA(minEA),EA(maxEA),lines);
    }

    DisassemblyDocument readDocumentFromFilePath(const std::string &path) {
        using namespace rapidjson;

        typedef GenericDocument<ASCII<>, MemoryPoolAllocator<>, MemoryPoolAllocator<>> DocumentType;
        char valueBuffer[4096];
        char parseBuffer[1024];
        MemoryPoolAllocator<> valueAllocator(valueBuffer, sizeof(valueBuffer));
        MemoryPoolAllocator<> parseAllocator(parseBuffer, sizeof(parseBuffer));
        DocumentType d(&valueAllocator, sizeof(parseBuffer), &parseAllocator);
        char readBuffer[65536];
        auto file = fopen(path.c_str(),"r");
        if (file == nullptr) {
            return invalidDocument;
        }
        FileReadStream is(file, readBuffer, sizeof(readBuffer));
		clock_t start = clock();
		d.ParseStream(is);
		clock_t end = clock();
		msg("reading json %lus\n",(end-start)/CLOCKS_PER_SEC);
        fclose(file);
        if (d.HasParseError()) {
            warning("failed to parse document json %s\n", path.c_str());
			warning("error code %d, offset %d\n",d.GetParseError(),d.GetErrorOffset());
            return invalidDocument;
        }
		start = clock();
        auto doc = documentFromJSON(d);
		end = clock();
		msg("building object tree %lus\n",(end-start)/CLOCKS_PER_SEC);
		return doc;
    }
}