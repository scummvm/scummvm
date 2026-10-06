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

#include "freescape/language/detokeniser.h"

namespace Freescape {

void normaliseKitOperands(FCLInstruction &instruction) {
	switch (instruction.getType()) {
	case Token::SETVAR:
	case Token::ADDVAR:
	case Token::ADCV:
	case Token::SUBVAR:
	case Token::SBCV:
	case Token::ANDV:
	case Token::ORV:
	case Token::XORV:
	case Token::TESTV:
	case Token::CMPV:
	case Token::GOTO:
		break;
	case Token::INVIS:
	case Token::VIS:
	case Token::TOGVIS:
	case Token::DESTROY:
	case Token::INVISQ:
	case Token::VISQ:
	case Token::DESTROYEDQ:
	case Token::STARTANIM:
	case Token::STOPANIM:
	case Token::TRIGANIM:
		if (instruction._destinationType != Token::UNKNOWN)
			break;
		return;
	default:
		return;
	}
	int32 source = instruction._source;
	Token::Type sourceType = instruction._sourceType;
	instruction.setSource(instruction._destination, instruction._destinationType);
	instruction.setDestination(source, sourceType);
}

} // namespace Freescape
