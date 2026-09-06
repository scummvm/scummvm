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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FREESCAPE_LANGUAGE_DETOKENISER_H
#define FREESCAPE_LANGUAGE_DETOKENISER_H

#include "freescape/language/instruction.h"

namespace Freescape {

// Classic games retain their byte-oriented dialect on DOS, Amiga and Atari ST.
Common::String detokeniseFreescapeCondition(const Common::Array<uint16> &tokenisedCondition, FCLInstructionVector &instructions, bool isAmigaAtari);
Common::String detokeniseKit8Condition(const Common::Array<byte> &tokenisedCondition, FCLInstructionVector &instructions);
Common::String detokeniseKit16Condition(const Common::Array<byte> &tokenisedCondition, FCLInstructionVector &instructions);

void normaliseKitOperands(FCLInstruction &instruction);

struct FCLOpcode {
	byte opcode;
	Token::Type type;
	const char *name;
	byte minArgs, maxArgs;
	byte event;
};

template<uint N>
const FCLOpcode *findFCLOpcode(const FCLOpcode (&opcodes)[N], byte opcode) {
	for (const auto &entry : opcodes) {
		if (entry.opcode == opcode)
			return &entry;
	}
	return nullptr;
}

} // namespace Freescape

#endif
