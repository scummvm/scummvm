/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Based on Phantasma code by Thomas Harte (2013),
// available at https://github.com/TomHarte/Phantasma/ (MIT)

#include "freescape/language/instruction.h"

namespace Freescape {

FCLInstructionVector *duplicateCondition(const FCLInstructionVector *condition) {
	if (!condition)
		return nullptr;

	FCLInstructionVector *copy = new FCLInstructionVector();
	for (uint i = 0; i < condition->size(); i++) {
		copy->push_back((*condition)[i].duplicate());
	}
	return copy;
}

FCLInstruction FCLInstruction::duplicate() const {
	FCLInstruction copy(_type);
	copy.setSource(_source, _sourceType);
	copy.setDestination(_destination, _destinationType);
	copy.setAdditional(_additional, _additionalType);
	copy._text = _text;

	copy._thenInstructions = duplicateCondition(_thenInstructions);
	copy._elseInstructions = duplicateCondition(_elseInstructions);

	return copy;
}

FCLInstruction::FCLInstruction(Token::Type type_) {
	_source = 0;
	_destination = 0;
	_additional = 0;
	_sourceType = _destinationType = _additionalType = Token::UNKNOWN;
	_type = type_;
	_thenInstructions = nullptr;
	_elseInstructions = nullptr;
}

FCLInstruction::FCLInstruction() : FCLInstruction(Token::UNKNOWN) {}

void FCLInstruction::setSource(int32 source_, Token::Type type) {
	_source = source_;
	_sourceType = type;
}

void FCLInstruction::setAdditional(int32 additional_, Token::Type type) {
	_additional = additional_;
	_additionalType = type;
}

void FCLInstruction::setDestination(int32 destination_, Token::Type type) {
	_destination = destination_;
	_destinationType = type;
}

void FCLInstruction::setBranches(FCLInstructionVector *thenBranch, FCLInstructionVector *elseBranch) {
	_thenInstructions = thenBranch;
	_elseInstructions = elseBranch;
}

Token::Type FCLInstruction::getType() const {
	return _type;
}

} // End of namespace Freescape
