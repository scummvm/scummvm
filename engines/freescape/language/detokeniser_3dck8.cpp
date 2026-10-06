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

#include "common/textconsole.h"

#include "freescape/language/detokeniser.h"

namespace Freescape {

static const FCLOpcode kKitOpcodes[] = {
	{0x00, Token::SETVAR, "SETV", 2, 2, 0},
	{0x01, Token::ADDVAR, "ADDV", 2, 2, 0},
	{0x02, Token::ADCV, "ADCV", 2, 2, 0},
	{0x03, Token::ANDV, "ANDV", 2, 2, 0},
	{0x04, Token::SUBVAR, "SUBV", 2, 2, 0},
	{0x05, Token::SBCV, "SBCV", 2, 2, 0},
	{0x06, Token::ORV, "ORV", 2, 2, 0},
	{0x07, Token::XORV, "XORV", 2, 2, 0},
	{0x08, Token::TESTV, "TESTV", 2, 2, 0},
	{0x09, Token::CMPV, "CMPV", 2, 2, 0},
	{0x0a, Token::VIS, "VIS", 1, 2, 0},
	{0x0b, Token::INVIS, "INVIS", 1, 2, 0},
	{0x0c, Token::TOGVIS, "TOGVIS", 1, 2, 0},
	{0x0d, Token::DESTROY, "DESTROY", 1, 2, 0},
	{0x0e, Token::GOTO, "GOTO", 1, 2, 0},
	{0x0f, Token::MODE, "MODE", 1, 1, 0},
	{0x20, Token::CONDITIONAL, "IFSHOT", 1, 2, kConditionalShot},
	{0x21, Token::CONDITIONAL, "IFHIT", 1, 2, kConditionalCollided},
	{0x22, Token::CONDITIONAL, "IFACTIVE", 1, 2, kConditionalActivated},
	{0x23, Token::CONDITIONAL, "IFSENSED", 1, 2, kConditionalSensed},
	{0x24, Token::VISQ, "IFVIS", 1, 2, 0},
	{0x25, Token::CONDITIONAL, "IFFALL", 0, 0, kConditionalFallen},
	{0x26, Token::CONDITIONAL, "IFCRUSH", 0, 0, kConditionalCrushed},
	{0x27, Token::CONDITIONAL, "IFTIMER", 0, 0, kConditionalTimeout},
	{0x28, Token::IFEQ, "IFEQ", 0, 0, 0},
	{0x29, Token::IFGT, "IFGT", 0, 0, 0},
	{0x2a, Token::IFLT, "IFLT", 0, 0, 0},
	{0x2b, Token::AND, "AND", 0, 0, 0},
	{0x2c, Token::OR, "OR", 0, 0, 0},
	{0x2d, Token::ELSE, "ELSE", 0, 0, 0},
	{0x2e, Token::THEN, "THEN", 0, 0, 0},
	{0x2f, Token::ENDIF, "ENDIF", 0, 0, 0},
	{0x30, Token::TIMER, "TIMER", 1, 1, 0},
	{0x31, Token::CROSS, "CROSS", 1, 1, 0},
	{0x32, Token::ENDGAME, "ENDGAME", 0, 0, 0},
	{0x33, Token::COLOUR, "COLOUR", 2, 2, 0},
	{0x34, Token::SOUND, "SOUND", 1, 1, 0},
	{0x35, Token::SYNCSND, "SYNCSND", 1, 1, 0},
	{0x36, Token::DELAY, "DELAY", 1, 1, 0},
	{0x37, Token::TEXTCOL, "TEXTCOL", 1, 1, 0},
	{0x38, Token::PRINT, "PRINT", 3, 3, 0},
	{0x39, Token::REDRAW, "REDRAW", 0, 0, 0},
	{0x3a, Token::EXECUTE, "CALL", 1, 1, 0},
	{0x3f, Token::END, "END", 0, 0, 0}
};

Common::String detokeniseKit8Condition(const Common::Array<byte> &tokenisedCondition, FCLInstructionVector &instructions) {
	Common::String source;
	for (uint pos = 0; pos < tokenisedCondition.size();) {
		byte raw = tokenisedCondition[pos++];
		if (raw == 0xff) {
			instructions.push_back(FCLInstruction(Token::ENDOFFILE));
			return source;
		}
		byte opcode = raw & 0x3f;
		bool variableSource = opcode >= 0x10 && opcode <= 0x19;
		if (variableSource)
			opcode &= ~0x10;
		const FCLOpcode *entry = findFCLOpcode(kKitOpcodes, opcode);
		if (!entry)
			error("Unsupported 8-bit 3D Construction Kit opcode %02x", opcode);
		uint count = raw >> 6;
		if (count < entry->minArgs || count > entry->maxArgs || count > tokenisedCondition.size() - pos)
			error("Invalid 8-bit 3D Construction Kit %s operands", entry->name);
		FCLInstruction instruction(entry->type);
		if (count > 0)
			instruction.setSource(tokenisedCondition[pos], variableSource ? Token::VARIABLE : Token::CONSTANT);
		if (count > 1)
			instruction.setDestination(tokenisedCondition[pos + 1], opcode <= 9 ? Token::VARIABLE : Token::CONSTANT);
		if (count > 2)
			instruction.setAdditional(tokenisedCondition[pos + 2]);
		if (entry->event) {
			if (count > 1)
				instruction.setAdditional(tokenisedCondition[pos + 1]);
			if (count > 0)
				instruction.setDestination(tokenisedCondition[pos]);
			instruction.setSource(entry->event);
		}
		normaliseKitOperands(instruction);
		instructions.push_back(instruction);
		source += entry->name;
		for (uint arg = 0; arg < count; arg++)
			source += Common::String::format(" %u", tokenisedCondition[pos + arg]);
		source += '\n';
		pos += count;
	}
	error("Unterminated 8-bit 3D Construction Kit condition");
}

} // namespace Freescape
