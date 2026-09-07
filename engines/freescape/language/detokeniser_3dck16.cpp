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

#include "common/endian.h"
#include "common/textconsole.h"

#include "freescape/language/detokeniser.h"

namespace Freescape {

static const FCLOpcode kKitOpcodes[] = {
	{0x00, Token::NOP, "NOP", 0, 0, 0},
	{0x01, Token::CONDITIONAL, "ACTIVATED?", 0, 0, kConditionalActivated},
	{0x02, Token::CONDITIONAL, "COLLIDED?", 0, 0, kConditionalCollided},
	{0x03, Token::CONDITIONAL, "SHOT?", 0, 0, kConditionalShot},
	{0x04, Token::CONDITIONAL, "TIMER?", 0, 0, kConditionalTimeout},
	{0x10, Token::SETVAR, "SETVAR", 2, 2, 0},
	{0x11, Token::ADDVAR, "ADDVAR", 2, 2, 0},
	{0x12, Token::SUBVAR, "SUBVAR", 2, 2, 0},
	{0x13, Token::ANDV, "ANDV", 2, 2, 0},
	{0x14, Token::ORV, "ORV", 2, 2, 0},
	{0x15, Token::NOTV, "NOTV", 1, 1, 0},
	{0x16, Token::VAREQ, "VAR=?", 2, 2, 0},
	{0x17, Token::VARGT, "VAR>?", 2, 2, 0},
	{0x18, Token::VARLT, "VAR<?", 2, 2, 0},
	{0x2f, Token::DESTROYEDQ, "DESTROYED?", 1, 2, 0},
	{0x30, Token::INVIS, "INVIS", 1, 2, 0},
	{0x31, Token::VIS, "VIS", 1, 2, 0},
	{0x32, Token::TOGVIS, "TOGVIS", 1, 2, 0},
	{0x33, Token::DESTROY, "DESTROY", 1, 2, 0},
	{0x34, Token::INVISQ, "INVIS?", 1, 2, 0},
	{0x35, Token::VISQ, "VIS?", 1, 2, 0},
	{0x36, Token::MOVE, "MOVE", 3, 3, 0},
	{0x37, Token::GETXPOS, "GETXPOS", 2, 3, 0},
	{0x38, Token::GETYPOS, "GETYPOS", 2, 3, 0},
	{0x39, Token::GETZPOS, "GETZPOS", 2, 3, 0},
	{0x3a, Token::MOVETO, "MOVETO", 3, 3, 0},
	{0x40, Token::IF, "IF", 0, 0, 0},
	{0x41, Token::THEN, "THEN", 0, 0, 0},
	{0x42, Token::ELSE, "ELSE", 0, 0, 0},
	{0x43, Token::ENDIF, "ENDIF", 0, 0, 0},
	{0x44, Token::AND, "AND", 0, 0, 0},
	{0x45, Token::OR, "OR", 0, 0, 0},
	{0x50, Token::STARTANIM, "STARTANIM", 1, 2, 0},
	{0x51, Token::STOPANIM, "STOPANIM", 1, 2, 0},
	{0x52, Token::START, "START", 0, 0, 0},
	{0x53, Token::RESTART, "RESTART", 0, 0, 0},
	{0x54, Token::INCLUDE, "INCLUDE", 1, 1, 0},
	{0x55, Token::WAITTRIG, "WAITTRIG", 0, 0, 0},
	{0x56, Token::TRIGANIM, "TRIGANIM", 1, 2, 0},
	{0x57, Token::REMOVE, "REMOVE", 1, 1, 0},
	{0x60, Token::LOOP, "LOOP", 1, 1, 0},
	{0x61, Token::AGAIN, "AGAIN", 2, 2, 0},
	{0x70, Token::SOUND, "SOUND", 1, 1, 0},
	{0x71, Token::SYNCSND, "SYNCSND", 1, 1, 0},
	{0x80, Token::WAIT, "WAIT", 0, 0, 0},
	{0x81, Token::DELAY, "DELAY", 1, 1, 0},
	{0x82, Token::UPDATEI, "UPDATEI", 1, 1, 0},
	{0x83, Token::PRINT, "PRINT", 2, 255, 0},
	{0x84, Token::REDRAW, "REDRAW", 0, 0, 0},
	{0x85, Token::MODE, "MODE", 1, 1, 0},
	{0x86, Token::ENDGAME, "ENDGAME", 0, 0, 0},
	{0x87, Token::EXECUTE, "EXECUTE", 1, 1, 0},
	{0x90, Token::GOTO, "GOTO", 1, 2, 0},
	{0xff, Token::END, "END", 0, 0, 0}
};

Common::String detokeniseKit16Condition(const Common::Array<byte> &tokenisedCondition, FCLInstructionVector &instructions) {
	Common::String detokenisedStream;
	int loops = 0;
	for (uint32 bytePointer = 0; bytePointer < tokenisedCondition.size();) {
		if (tokenisedCondition.size() - bytePointer < 2)
			error("Truncated 16-bit FCL instruction at %u", bytePointer);
		byte count = tokenisedCondition[bytePointer], opcode = tokenisedCondition[bytePointer + 1];
		if (tokenisedCondition.size() - bytePointer < 2U + 2U * count)
			error("Truncated 16-bit FCL instruction at %u", bytePointer);
		const FCLOpcode *entry = findFCLOpcode(kKitOpcodes, opcode);
		if (!entry)
			error("Unknown 16-bit FCL opcode %02x at %u", opcode, bytePointer);
		if (count < entry->minArgs || count > entry->maxArgs)
			error("Invalid argument count for 16-bit FCL opcode %02x at %u", opcode, bytePointer);

		FCLInstruction instruction(entry->type);
		int32 operands[3] = {};
		Token::Type types[3] = {Token::UNKNOWN, Token::UNKNOWN, Token::UNKNOWN};
		uint32 argumentPointer = bytePointer + 2;
		uint argumentCount = count;
		detokenisedStream += entry->name;
		if (entry->type == Token::PRINT) {
			uint16 length = READ_BE_UINT16(&tokenisedCondition[argumentPointer]);
			if (count != 2 + (length + 1) / 2)
				error("Invalid 16-bit FCL PRINT instruction at %u", bytePointer);
			instruction._text = Common::String(reinterpret_cast<const char *>(&tokenisedCondition[argumentPointer + 2]), length);
			detokenisedStream += Common::String::format(" (\"%s\", ", instruction._text.c_str());
			argumentPointer += 2 + ((length + 1) & ~1);
			argumentCount = 1;
		} else if (entry->type == Token::AGAIN) {
			argumentCount = 0; // The two words hold the runner's loop state.
		} else if (count) {
			detokenisedStream += " (";
		}
		for (uint i = 0; i < argumentCount; i++) {
			uint16 operand = READ_BE_UINT16(&tokenisedCondition[argumentPointer + 2 * i]);
			types[i] = operand & 0x8000 ? Token::VARIABLE : Token::CONSTANT;
			operands[i] = types[i] == Token::VARIABLE ? operand & 0xff : (operand & 0x4000 ? int32(operand) - 0x8000 : operand);
			if (i)
				detokenisedStream += ", ";
			detokenisedStream += Common::String::format(types[i] == Token::VARIABLE ? "v%d" : "%d", operands[i]);
		}
		if (argumentCount)
			detokenisedStream += ")";
		detokenisedStream += "\n";

		switch (entry->type) {
		case Token::CONDITIONAL:
			operands[0] = entry->event;
			types[0] = Token::CONSTANT;
			break;
		case Token::LOOP:
			loops++;
			break;
		case Token::AGAIN:
			if (loops) {
				loops--;
			} else if (!READ_BE_UINT16(&tokenisedCondition[bytePointer + 2])) {
				// An orphaned AGAIN with no jump offset has no effect in RUNVGA.
				instruction = FCLInstruction(Token::NOP);
			} else {
				error("16-bit FCL AGAIN without LOOP");
			}
			break;
		default:
			break;
		}
		instruction.setSource(operands[0], types[0]);
		instruction.setDestination(operands[1], types[1]);
		instruction.setAdditional(operands[2], types[2]);
		normaliseKitOperands(instruction);
		instructions.push_back(instruction);
		bytePointer += 2 + 2 * count;
	}
	if (loops)
		error("Unterminated 16-bit FCL LOOP");
	return detokenisedStream;
}

} // namespace Freescape
