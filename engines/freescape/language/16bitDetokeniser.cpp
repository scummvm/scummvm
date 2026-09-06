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

#include "freescape/language/16bitDetokeniser.h"

namespace Freescape {

static const struct {
	byte opcode;
	Token::Type type;
	byte minArgs, maxArgs;
	const char *name;
} opcodeTable[] = {
	{0x00, Token::NOP,          0, 0, "NOP"},
	{0x01, Token::CONDITIONAL,  0, 0, "ACTIVATED?"},
	{0x02, Token::CONDITIONAL,  0, 0, "COLLIDED?"},
	{0x03, Token::CONDITIONAL,  0, 0, "SHOT?"},
	{0x04, Token::CONDITIONAL,  0, 0, "TIMER?"},
	{0x10, Token::SETVAR,       2, 2, "SETVAR"},
	{0x11, Token::ADDVAR,       2, 2, "ADDVAR"},
	{0x12, Token::SUBVAR,       2, 2, "SUBVAR"},
	{0x13, Token::ANDV,         2, 2, "ANDV"},
	{0x14, Token::ORV,          2, 2, "ORV"},
	{0x15, Token::NOTV,         1, 1, "NOTV"},
	{0x16, Token::VAREQ,        2, 2, "VAR=?"},
	{0x17, Token::VARGT,        2, 2, "VAR>?"},
	{0x18, Token::VARLT,        2, 2, "VAR<?"},
	{0x2f, Token::DESTROYEDQ,   1, 2, "DESTROYED?"},
	{0x30, Token::INVIS,        1, 2, "INVIS"},
	{0x31, Token::VIS,          1, 2, "VIS"},
	{0x32, Token::TOGVIS,       1, 2, "TOGVIS"},
	{0x33, Token::DESTROY,      1, 2, "DESTROY"},
	{0x34, Token::INVISQ,       1, 2, "INVIS?"},
	{0x35, Token::VISQ,         1, 2, "VIS?"},
	{0x36, Token::MOVE,         3, 3, "MOVE"},
	{0x37, Token::GETXPOS,      2, 3, "GETXPOS"},
	{0x38, Token::GETYPOS,      2, 3, "GETYPOS"},
	{0x39, Token::GETZPOS,      2, 3, "GETZPOS"},
	{0x3a, Token::MOVETO,       3, 3, "MOVETO"},
	{0x40, Token::IF,           0, 0, "IF"},
	{0x41, Token::THEN,         0, 0, "THEN"},
	{0x42, Token::ELSE,         0, 0, "ELSE"},
	{0x43, Token::ENDIF,        0, 0, "ENDIF"},
	{0x44, Token::AND,          0, 0, "AND"},
	{0x45, Token::OR,           0, 0, "OR"},
	{0x50, Token::STARTANIM,    1, 2, "STARTANIM"},
	{0x51, Token::STOPANIM,     1, 2, "STOPANIM"},
	{0x52, Token::START,        0, 0, "START"},
	{0x53, Token::RESTART,      0, 0, "RESTART"},
	{0x54, Token::INCLUDE,      1, 1, "INCLUDE"},
	{0x55, Token::WAITTRIG,     0, 0, "WAITTRIG"},
	{0x56, Token::TRIGANIM,     1, 2, "TRIGANIM"},
	{0x57, Token::REMOVE,       1, 1, "REMOVE"},
	{0x60, Token::LOOP,         1, 1, "LOOP"},
	{0x61, Token::AGAIN,        2, 2, "AGAIN"},
	{0x70, Token::SOUND,        1, 1, "SOUND"},
	{0x71, Token::SYNCSND,      1, 1, "SYNCSND"},
	{0x80, Token::WAIT,         0, 0, "WAIT"},
	{0x81, Token::DELAY,        1, 1, "DELAY"},
	{0x82, Token::UPDATEI,      1, 1, "UPDATEI"},
	{0x83, Token::PRINT,        2, 255, "PRINT"},
	{0x84, Token::REDRAW,       0, 0, "REDRAW"},
	{0x85, Token::MODE,         1, 1, "MODE"},
	{0x86, Token::ENDGAME,      0, 0, "ENDGAME"},
	{0x87, Token::EXECUTE,      1, 1, "EXECUTE"},
	{0x90, Token::GOTO,         1, 2, "GOTO"},
	{0xff, Token::END,          0, 0, "END"}
};

Common::String detokenise16bitCondition(const Common::Array<byte> &tokenisedCondition, FCLInstructionVector &instructions) {
	Common::String detokenisedStream;
	int loops = 0;
	for (uint32 bytePointer = 0; bytePointer < tokenisedCondition.size();) {
		if (tokenisedCondition.size() - bytePointer < 2)
			error("Truncated 16-bit FCL instruction at %u", bytePointer);
		byte count = tokenisedCondition[bytePointer], opcode = tokenisedCondition[bytePointer + 1];
		if (tokenisedCondition.size() - bytePointer < 2U + 2U * count)
			error("Truncated 16-bit FCL instruction at %u", bytePointer);
		uint index = 0;
		while (index < ARRAYSIZE(opcodeTable) && opcodeTable[index].opcode != opcode)
			index++;
		if (index == ARRAYSIZE(opcodeTable))
			error("Unknown 16-bit FCL opcode %02x at %u", opcode, bytePointer);
		const auto &entry = opcodeTable[index];
		if (count < entry.minArgs || count > entry.maxArgs)
			error("Invalid argument count for 16-bit FCL opcode %02x at %u", opcode, bytePointer);

		FCLInstruction instruction(entry.type);
		int32 operands[3] = {};
		Token::Type types[3] = {Token::UNKNOWN, Token::UNKNOWN, Token::UNKNOWN};
		uint32 argumentPointer = bytePointer + 2;
		uint argumentCount = count;
		detokenisedStream += entry.name;
		if (entry.type == Token::PRINT) {
			uint16 length = READ_BE_UINT16(&tokenisedCondition[argumentPointer]);
			if (count != 2 + (length + 1) / 2)
				error("Invalid 16-bit FCL PRINT instruction at %u", bytePointer);
			instruction._text = Common::String(reinterpret_cast<const char *>(&tokenisedCondition[argumentPointer + 2]), length);
			detokenisedStream += Common::String::format(" (\"%s\", ", instruction._text.c_str());
			argumentPointer += 2 + ((length + 1) & ~1);
			argumentCount = 1;
		} else if (entry.type == Token::AGAIN) {
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

		// Match the operand order used by the 8-bit detokeniser.
		switch (entry.type) {
		case Token::SETVAR: case Token::ADDVAR: case Token::SUBVAR: case Token::ANDV: case Token::ORV:
		case Token::GOTO:
			SWAP(operands[0], operands[1]);
			SWAP(types[0], types[1]);
			break;
		case Token::INVIS: case Token::VIS: case Token::TOGVIS: case Token::DESTROY:
		case Token::INVISQ: case Token::VISQ: case Token::DESTROYEDQ:
		case Token::STARTANIM: case Token::STOPANIM: case Token::TRIGANIM:
			if (count == 2) {
				SWAP(operands[0], operands[1]);
				SWAP(types[0], types[1]);
			}
			break;
		case Token::CONDITIONAL:
			operands[0] = opcode == 1 ? kConditionalActivated : opcode == 2 ? kConditionalCollided :
				opcode == 3 ? kConditionalShot : kConditionalTimeout;
			types[0] = Token::CONSTANT;
			break;
		case Token::LOOP:
			loops++;
			break;
		case Token::AGAIN:
			if (--loops < 0)
				error("16-bit FCL AGAIN without LOOP");
			break;
		default:
			break;
		}
		instruction.setSource(operands[0], types[0]);
		instruction.setDestination(operands[1], types[1]);
		instruction.setAdditional(operands[2], types[2]);
		instructions.push_back(instruction);
		bytePointer += 2 + 2 * count;
	}
	if (loops)
		error("Unterminated 16-bit FCL LOOP");
	return detokenisedStream;
}

} // namespace Freescape
