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
#include "freescape/language/token.h"

namespace Freescape {

class FCLInstruction;
typedef Common::Array<FCLInstruction> FCLInstructionVector;

class FCLInstruction {
public:
	FCLInstruction();
	FCLInstruction(Token::Type type);
	void setSource(int32 source);
	void setAdditional(int32 additional);
	void setDestination(int32 destination);

	Token::Type getType() const;

	bool isConditional() const {
		Token::Type type = getType();
		return 	type == Token::Type::BITNOTEQ || type == Token::Type::VARNOTEQ || \
				type == Token::Type::IFGTEQ || type == Token::Type::IFLTEQ || \
				type == Token::Type::VAREQ || _type == Token::Type::INVISQ;
	}

	void setBranches(FCLInstructionVector *thenBranch, FCLInstructionVector *elseBranch);

	FCLInstruction duplicate();

	int32 _source;
	int32 _additional;
	int32 _destination;

	FCLInstructionVector *_thenInstructions;
	FCLInstructionVector *_elseInstructions;

private:
	enum Token::Type _type;
};

} // End of namespace Freescape

#endif // FREESCAPE_INSTRUCTION_H
