//
// Created by Tobias Conradi on 26.06.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_PATTERN_H
#define IDIOMMATCHER_PATTERN_H

#include <vector>
#include <string>
#include <memory>
#include <sys/types.h>

namespace IdiomMatcher {

    // to slim down json files, the default aren't serialized by default
    extern bool serializeDefaultValues; // default = false

    class EA {
	public:
	typedef uintmax_t EAValue_t;
	EA(const EAValue_t value) : _value(value){};

        EAValue_t getValue() const { return _value; };

        std::string description() const;

        bool operator==(const EA &other) const;
		bool operator<(const EA &other) const;
        bool operator<=(const EA &other) const;

    private:
        EAValue_t _value;
    };

    const EA InvalidEA(UINTMAX_MAX);

    class Operand {
    public:
        Operand(const std::string &text, const std::vector<std::string> &registers = std::vector<std::string>(), const std::string &extractAs = "", const std::string &regex = "", const bool nameIsTemplate = false, const bool used = true, const bool modified = false,const uintmax_t address = 0)
                : _text(text), _registers(registers), _extractAs(extractAs), _regex(regex), _nameIsTemplate(nameIsTemplate), _used(used), _modified(modified), _address(address) { }
        Operand(const std::string &text, const std::vector<std::string> &registers, const bool used, const bool modified, const uintmax_t address)
                : _text(text), _registers(registers), _extractAs(""), _regex(""), _nameIsTemplate(false), _used(used), _modified(modified), _address(address) { }
        
        std::string getText() const { return _text; };
        std::string getExtractAs() const { return _extractAs; };
        std::string getRegex() const { return _regex; }
		std::vector<std::string> getRegisters() const { return _registers; }

        const bool getNameIsTemplate() const { return _nameIsTemplate; }
        const bool getUsed() const { return _used; }
        const bool getModified() const { return _modified; }
	const uintmax_t getAddress() const { return _address; }

        const bool isWildcard() const { return _text.empty(); }
        std::string description() const;

    private:
        const std::string _text;
	const std::vector<std::string> _registers;
	const std::string _extractAs;
        const std::string _regex;
	const bool _nameIsTemplate;
        const bool _used;
        const bool _modified;
	const uintmax_t  _address;
    };
    typedef std::shared_ptr<Operand> Operand_Ref;
    typedef std::vector<Operand_Ref> Operands;

	class XRef {
		EA _target;
		bool _isData; // code or data reference?
		bool _isUnordinaryFlow; // is unordinary flow ( not to next instruction by linear execution) only valid if isData == false
	public:
		XRef(const EA &target, const bool isData = false, const bool isUnordinaryFlow = false) : _target(target), _isData(isData), _isUnordinaryFlow(isUnordinaryFlow) { }

		const EA getTarget() const { return _target; }
		const bool isData() const { return _isData; }
		const bool isUnordinaryFlow() const { return _isUnordinaryFlow; }
	};
	typedef std::shared_ptr<XRef> XRef_Ref;
	typedef std::vector<XRef_Ref> XRefs;

    class Instruction {
    public:
        Instruction(const std::string &mnemonic, const Operands &operands, const XRefs &xrefs, const uint16_t size = 0, const EA &ea = InvalidEA, const bool isRegex = false) : _mnemonic(mnemonic), _operands(operands), _xrefs(xrefs), _size(size), _ea(ea), _isRegex(isRegex) {};

        const Operands &getOperands() const { return _operands; }
		const XRefs &getXrefs() const { return _xrefs; }
        const std::string &getMnemonic() const { return _mnemonic; }
        const uint16_t getSize() const { return _size; }
		const EA &getEA() const { return  _ea; }
		const bool getIsRegex() const { return _isRegex; }
		std::string description() const;

    private:
        std::string _mnemonic;
        Operands _operands;
		XRefs _xrefs;
        uint16_t _size = 0;
		EA _ea;
		bool _isRegex;
    };
    typedef std::shared_ptr<Instruction> Instruction_ref;
    typedef std::vector<Instruction_ref > Instructions;

    const Instruction InvalidInstruction("invalid",Operands(),XRefs(),-1);
	const Instruction_ref InvalidInstruction_ref = std::make_shared<Instruction>(InvalidInstruction);

    class Action {
    public:
        Action(const std::string &script) : _script(script) {};

        std::string description() const;

        const std::string getScript() const { return _script; };

    private:
        const std::string _script;
    };
    typedef std::shared_ptr<Action> Action_Ref;
    typedef std::vector<Action_Ref> Actions;

    class Pattern {
    public:
		Pattern(const std::string &name, const Instructions &instructions, const std::string &architecture = "", const Actions &actions = Actions(), const bool anchorTop = false)
                : _name(name), _instructions(instructions), _actions(actions), _anchorTop(anchorTop), _architecture(architecture) { };

        Pattern(const Pattern &p) : _name(p._name), _instructions(p._instructions), _actions(p._actions), _anchorTop(p._anchorTop),_architecture(p._architecture) { };

		const std::string &getName() const { return _name; };
		const Instructions &getInstructions() const { return _instructions; };
		const Actions &getActions() const { return _actions; };
        const bool getAnchorTop() const { return _anchorTop; };
		const std::string &getArchitecture() const { return _architecture; };

    private:
        const std::string _name;
		const Instructions _instructions;
		const Actions _actions;
        const bool _anchorTop;
		const std::string _architecture;
    };
    typedef std::shared_ptr<Pattern> Pattern_ref;
    typedef std::vector<Pattern_ref > Patterns;


}
#endif //IDIOMMATCHER_PATTERN_H
