//
// Created by Tobias Conradi on 26.06.15.
// Licensed under MIT License, see LICENSE for full text.

#include "Pattern.h"

namespace IdiomMatcher {

    // to slim down json files, the default aren't stored by default
    bool serializeDefaultValues = false;


	bool EA::operator==(const EA &other) const {
		return _value == other._value;
	}

	bool EA::operator<(const EA &other) const {
		return _value < other._value;
	}
    bool EA::operator<=(const EA &other) const {
        return _value <= other._value;
    }

	std::string EA::description() const{
		return std::string("EA %d", _value);
	}


    std::string Operand::description() const {
        return std::string(getText().c_str());
    }

    std::string Instruction::description() const {
        std::string description;
        description.append(_mnemonic);

		bool comma = false;
        for (auto op : _operands) {
			if (comma) description.append(",");
			comma = true;
			description.append(" ");
            description.append(op->description().c_str());
        }
        return description;
    }


    std::string Action::description() const {
        auto description = std::string("Action script:");
        description.append(_script);
        return description;
    }

}