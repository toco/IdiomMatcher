//
// Created by Tobias Conradi on 26.06.15.
// Licensed under MIT License, see LICENSE for full text.

#include "PatternPersistence.h"
#include "Logging.h"
#include <boost/property_tree/json_parser.hpp>

namespace IdiomMatcher {

    Patterns PatternPersistence::readFromFilePath(const std::string &path) {
        Patterns patterns;


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
			warning("failed to open pattern json file %s\n", path.c_str());
			return patterns;
		}
		FileReadStream is(file, readBuffer, sizeof(readBuffer));
		d.ParseStream(is);
		fclose(file);
		if (d.HasParseError()) {
			warning("failed to parse pattern json %s\n", path.c_str());
			warning("error code %d, offset %d\n",d.GetParseError(),d.GetErrorOffset());
			return patterns;
		}


		auto version = d["version"].GetUint();
        if (version != documentVersion) {
            warning("Failed to read pattern json, wrong document version");
            return patterns;
        }

		auto &patternsValue = d["patterns"];
		for (rapidjson::SizeType i = 0; i < patternsValue.Size(); ++i) {
			auto pattern = patternFromJSON(patternsValue[i]);
			if (pattern) {
				patterns.push_back(pattern);
			}
		}
		return patterns;
    }

	template<typename JSONWriter>
	void writePatternsToWriter(JSONWriter &writer, const Patterns &patterns) {
		writer.StartObject();
		writer.Key("version");
		writer.Uint(PatternPersistence::documentVersion);
		writer.Key("patterns");
		writer.StartArray();
		for (auto &pattern : patterns) {
			SerializePattern(writer,*pattern);
		}
		writer.EndArray();
		writer.EndObject();
	}

	bool PatternPersistence::writeToFilePath(const std::string &path, const Patterns &patterns) {

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

		writePatternsToWriter(writer,patterns);

		eos.Flush();
		os.Flush();
		fclose(fh);
		return true;

	}

	template<typename JSONWriter>
	void SerializeAction(JSONWriter &writer, const Action &action) {
		writer.StartObject();
		writer.Key("script");
		writer.String(action.getScript());
		writer.EndObject();
	}

	Action_Ref actionFromJSON(const JSONValue &value) {
		return std::make_shared<Action>(value["script"].GetString());
	}

	Pattern_ref patternFromJSON(const JSONValue &value) {

		std::string name = value["name"].GetString();

		Instructions instructions;
		if (value.HasMember("instructions")) {
			auto &instructionsValue = value["instructions"];
			for (rapidjson::SizeType i = 0; i < instructionsValue.Size(); ++i) {
				auto instruction = instructionFromJSON(instructionsValue[i]);
				if (instruction) {
					instructions.push_back(instruction);
				}
			}
		}

		Actions actions;
		if (value.HasMember("actions")) {
			auto &instructionsValue = value["actions"];
			for (rapidjson::SizeType i = 0; i < instructionsValue.Size(); ++i) {
				auto action = actionFromJSON(instructionsValue[i]);
				if (action) {
					actions.push_back(action);
				}
			}
		}

		bool anchorTop = false;
		if (value.HasMember("anchorTop")) {
			anchorTop = value["anchorTop"].GetBool();
		}

		std::string architecture;
		if (value.HasMember("architecture")) {
			architecture = value["architecture"].GetString();
		}
		return std::make_shared<Pattern>(name,instructions,architecture,actions,anchorTop);
	}

	template <typename JSONWriter>
	void SerializePattern(JSONWriter &writer, const Pattern &pattern) {
		writer.StartObject();
		writer.Key("name");
		writer.String(pattern.getName());
		if (pattern.getInstructions().size() > 0 || serializeDefaultValues) {
			writer.Key("instructions");
			writer.StartArray();
			for (auto &ins : pattern.getInstructions()) {
				SerializeInstruction(writer, *ins);
			}
			writer.EndArray();
		}
		if (pattern.getActions().size() > 0 || serializeDefaultValues) {
			writer.Key("actions");
			writer.StartArray();
			for (auto &action : pattern.getActions()) {
				SerializeAction(writer, *action);
			}
			writer.EndArray();
		}
		if (pattern.getAnchorTop() || serializeDefaultValues) {
			writer.Key("anchorTop");
			writer.Bool(pattern.getAnchorTop());
		}

		if (!pattern.getArchitecture().empty() || serializeDefaultValues) {
			writer.Key("architecture");
			writer.String(pattern.getArchitecture());
		}

		writer.EndObject();
	}

}