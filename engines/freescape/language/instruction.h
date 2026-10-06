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

#ifndef FREESCAPE_INSTRUCTION_H
#define FREESCAPE_INSTRUCTION_H

#include "common/array.h"
#include "common/str.h"
#include "freescape/language/token.h"

namespace Freescape {

enum {
	kConditionalShot = 1 << 0,
	kConditionalTimeout = 1 << 1,
	kConditionalCollided = 1 << 2,
	kConditionalActivated = 1 << 3,
	kConditionalSensed = 1 << 4,
	kConditionalFallen = 1 << 5,
	kConditionalCrushed = 1 << 6,
};

class FCLInstruction;
typedef Common::Array<FCLInstruction> FCLInstructionVector;

FCLInstructionVector *duplicateCondition(const FCLInstructionVector *condition);

class FCLInstruction {
public:
	FCLInstruction();
	FCLInstruction(Token::Type type);
	void setSource(int32 source, Token::Type type = Token::CONSTANT);
	void setAdditional(int32 additional, Token::Type type = Token::CONSTANT);
	void setDestination(int32 destination, Token::Type type = Token::CONSTANT);

	Token::Type getType() const;

	void setBranches(FCLInstructionVector *thenBranch, FCLInstructionVector *elseBranch);

	FCLInstruction duplicate() const;

	// Source/destination: arithmetic uses (variable, value); GOTO uses (area, entrance).
	// Object commands use (object) or (area, object).
	int32 _source;
	int32 _additional;
	int32 _destination;
	// Kit decoders mark omitted operands as UNKNOWN.
	Token::Type _sourceType;
	Token::Type _additionalType;
	Token::Type _destinationType;
	Common::String _text;

	FCLInstructionVector *_thenInstructions;
	FCLInstructionVector *_elseInstructions;

private:
	Token::Type _type;
};

} // End of namespace Freescape

#endif // FREESCAPE_INSTRUCTION_H
