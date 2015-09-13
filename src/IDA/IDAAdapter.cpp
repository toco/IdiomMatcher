//
// Created by Tobias Conradi on 25.06.15.
// Licensed under MIT License, see LICENSE for full text.


#include "IDAAdapter.h"
#include <Model/DisassemblyPersistence.h>
#include <idp.hpp>
#include <loader.hpp>
#include <frame.hpp>
#include <funcs.hpp>
#include <ints.hpp>
#include <regex>
#include <map>

namespace IDAAdapter {
	using namespace IdiomMatcher;

	EA IDA::minEA() const {
		return EA(inf.minEA);
	}

	EA IDA::maxEA() const {
		return EA(inf.maxEA);
	}

	EA IDA::nextEA(const EA &ea) const {
		auto instruction = instructionForEA(ea);
		auto offset = instruction.getSize() ?: 1;
		ea_t selfCalcEA = ea.getValue()+offset;
		ea_t idaNextEA = nextaddr(ea.getValue());
		ea_t choosenEA = selfCalcEA < idaNextEA ? idaNextEA : selfCalcEA;
		return EA(choosenEA);
	}

	static inline bool getUseChangeForFeatureAt(bool *use,bool *change, uint32 feature, int i) {
		switch (i)
		{
			case 0:
				*use = (feature & CF_USE1) == CF_USE1;
				*change = (feature & CF_CHG1) == CF_CHG1;
				break;
			case 1:
				*use = (feature & CF_USE2) == CF_USE2;
				*change = (feature & CF_CHG2) == CF_CHG2;
				break;
			case 2:
				*use = (feature & CF_USE3) == CF_USE3;
				*change = (feature & CF_CHG3) == CF_CHG3;
				break;
			case 3:
				*use = (feature & CF_USE4) == CF_USE4;
				*change = (feature & CF_CHG4) == CF_CHG4;
				break;
			case 4:
				*use = (feature & CF_USE5) == CF_USE5;
				*change = (feature & CF_CHG5) == CF_CHG5;
				break;
			case 5:
				*use = (feature & CF_USE6) == CF_USE6;
				*change = (feature & CF_CHG6) == CF_CHG6;
				break;
			default:
				return false;
		}
		return true;
	}

	size_t registerByteWidthForDataType(char dtype) {
		switch (dtype) {
			case dt_byte:
				return 1;
			case dt_word:
				return 2;
			case dt_dword:
			case dt_float:
				return 4;
			case dt_double:
				return 8;
			case dt_tbyte:
				//#define dt_tbyte        5       ///< variable size (\ph{tbyte_size})
			case dt_packreal:
				//#define dt_packreal     6       ///< packed real format for mc68040
				//// ...to here the order should not be changed, see mc68000
				return 0; // TODO: determine size?

			case dt_qword:
				return 8;
			case dt_byte16:
				return 16;
			case dt_code:
//#define dt_code         9       ///< ptr to code (not used?)
			case dt_void:
//#define dt_void         10      ///< none
				return 0; // TODO: determine size?
			case dt_fword:
				return 6;

			case dt_byte32:
				return 32;
			case dt_byte64:
				return 64;

// TODO:
//#define dt_bitfild      12      ///< bit field (mc680x0)
//#define dt_string       13      ///< pointer to asciiz string
//#define dt_unicode      14      ///< pointer to unicode string
//#define dt_3byte        15      ///< 3-byte data
//#define dt_ldbl         16      ///< long double (which may be different from tbyte)
			default:
				return 0;
		}
	}

	static inline const std::string canonicalRegisterName(char *registerName, size_t length) {
		const char *regName = get_reg_info2(registerName,NULL);
		std::string canonicalName = "";
		if (regName != NULL) {
			canonicalName = std::string(regName);
		} else if (length != -1) {
			canonicalName = std::string(registerName,length);
		}
		return canonicalName;
	}

	static inline std::vector<std::string> registersFromOperandText(const std::string &opText, size_t regWidth, const ea_t ea) {
		std::vector<std::string> registers;
		char buf[MAXSTR];
		size_t length;
		std::map<std::string::size_type,std::string> posToReg;
		for (int regIndex = 0;(length = get_reg_name(regIndex,regWidth, buf, sizeof(buf))) != -1; ++regIndex) {
			std::string registerName(buf,length);
			auto pos = opText.find(registerName);

			if (pos==std::string::npos) {
				get_reg_info2(buf, nullptr);
				regvar_t* regvar = find_regvar(get_func(ea),ea,buf);
				if (regvar) {
					pos = opText.find(std::string(regvar->user));
				}
			}

			if (pos!=std::string::npos) {
				auto posToRegIt = posToReg.find(pos);
				auto canonicalRegName = canonicalRegisterName(buf, length);
				bool emplace = true;
				if (posToRegIt != posToReg.end()) {
					if (posToRegIt->second.length() < canonicalRegName.length()) {
						posToReg.erase(posToRegIt);
					} else {
						emplace = false;
					}
				}
				if (emplace) {
					posToReg.emplace(pos,canonicalRegName);
				}
			}
		}
		for (auto it : posToReg) {
			registers.push_back(it.second);
		}
		return registers;
	}

	static inline Operand_Ref operandForOPStruct_ea_feature_index(const op_t &op, const ea_t ea, const uint32 feature, const uint operandIndex) {
		std::string opText = ({
			char buf[MAXSTR];
			ua_outop2(ea, buf, sizeof(buf), operandIndex);
			size_t opTextLength = tag_remove(buf, buf, sizeof(buf)); // remove color tags
			std::string(buf,opTextLength);
		});

		std::vector<std::string> registers;
		size_t regWidth = registerByteWidthForDataType(op.dtyp);
		if (op.type == o_reg) {
			char buf[MAXSTR];
			size_t length = get_reg_name(op.reg,regWidth, buf, sizeof(buf));
			if (length != -1) {
				registers.push_back(canonicalRegisterName(buf, length));
			}
		} else if (op.type == o_mem || op.type == o_phrase || op.type == o_displ || op.type >= o_idpspec0) {
			auto registerWidth = regWidth;
			auto dtyp = op.dtyp;
			bool found = opText.empty();
			while (!(found || registerWidth == 0)) {
				auto extractedRegs = registersFromOperandText(opText, registerWidth,ea);
				registers.insert(registers.end(), extractedRegs.begin(), extractedRegs.end());
				found = !extractedRegs.empty();
				dtyp++;
				registerWidth = registerByteWidthForDataType(dtyp);
			}
		}

		uint address = op.addr;
		bool used, modified;
		getUseChangeForFeatureAt(&used, &modified, feature, operandIndex);
		auto operand = std::make_shared<Operand>(opText,registers,used,modified,address);
		return operand;
	}


	Instruction IDA::instructionForEA(const EA &instructionEA) const {
		ea_t ea = (ea_t)instructionEA.getValue();
		decode_insn(ea);
		uint32 feature = cmd.get_canon_feature();

		std::vector<std::shared_ptr<Operand> > operands;

		for (int i = 0; cmd.Operands[i].type != o_void; i++) {
			op_t op;
			memcpy(&op,&cmd.Operands[i],sizeof(op_t));
			auto operand = operandForOPStruct_ea_feature_index(op,ea,feature,i);
			operands.push_back(operand);
		}

		const auto mnenonic = std::string(cmd.get_canon_mnem());
		const auto instructionSize = cmd.size;


		XRefs refs;
		xrefblk_t xb;

		for ( bool ok=xb.first_from(ea, XREF_ALL); ok; ok=xb.next_from() )
		{
			refs.push_back(std::make_shared<XRef>(EA(xb.to),!xb.iscode,xb.type != fl_F));
		}

		auto instruction = Instruction(mnenonic, operands, refs, instructionSize, instructionEA);
		return instruction;
	}

	std::string IDA::executableName() const {
		char buf[MAXSTR];
		size_t size = get_root_filename(buf,sizeof(buf));
		return std::string(buf,size);
	}

	std::string IDA::executablePath() const {
		char buf[MAXSTR];
		size_t size = get_input_file_path(buf,sizeof(buf));
		return std::string(buf,size);
	}

	std::string IDA::executableArchitecture() const {
		char buf[MAXSTR];
		size_t length = get_file_type_name(buf,sizeof(buf));
		auto arch = std::string(buf,length);
		arch.append("; CPU-ID: ");
		arch.append(std::to_string(ph.id));
		return arch;
	}

	std::string IDA::commentForEA(const IdiomMatcher::EA &instructionEA) const {
		char buf[MAXSTR];
		memset(buf, 0, sizeof(buf));
		size_t length;
		std::string comment = "";

		length = get_cmt(instructionEA.getValue(),false,buf,sizeof(buf));
		if (length != -1) {
			comment.append(buf,length);
		}

		return comment;
	}

	std::string IDA::getDisassemblerName() const {
		return "IDA Pro";
	}

};
