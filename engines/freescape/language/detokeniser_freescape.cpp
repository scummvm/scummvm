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
// which was implemented based on John Elliott's reverse engineering of Driller (2001)
// https://web.archive.org/web/20200116141513/http://www.seasip.demon.co.uk/ZX/Driller/

#include "freescape/freescape.h"
#include "freescape/language/detokeniser.h"
#include "freescape/language/variables.h"

namespace Freescape {

static const FCLOpcode kFreescapeOpcodes[] = {
	{0x00, Token::NOP, "NOP", 0, 0, 0},
	{0x01, Token::ADDVAR, "ADDVAR", 3, 3, 0},
	{0x02, Token::ADDVAR, "ADDVAR", 1, 1, 0},
	{0x03, Token::TOGVIS, "TOGVIS", 1, 1, 0},
	{0x04, Token::VIS, "VIS", 1, 1, 0},
	{0x05, Token::INVIS, "INVIS", 1, 1, 0},
	{0x06, Token::TOGVIS, "TOGVIS", 2, 2, 0},
	{0x07, Token::VIS, "VIS", 2, 2, 0},
	{0x08, Token::INVIS, "INVIS", 2, 2, 0},
	{0x09, Token::ADDVAR, "ADDVAR", 1, 1, 0},
	{0x0a, Token::SUBVAR, "SUBVAR", 1, 1, 0},
	{0x0b, Token::VARNOTEQ, "IF VAR!=?", 2, 2, 0},
	{0x0c, Token::SETBIT, "SETBIT", 1, 1, 0},
	{0x0d, Token::CLEARBIT, "CLRBIT", 1, 1, 0},
	{0x0e, Token::BITNOTEQ, "IF BIT!=?", 2, 2, 0},
	{0x0f, Token::SOUND, "SOUND", 1, 1, 0},
	{0x10, Token::DESTROY, "DESTROY", 1, 1, 0},
	{0x11, Token::DESTROY, "DESTROY", 2, 2, 0},
	{0x12, Token::GOTO, "GOTO", 2, 2, 0},
	{0x13, Token::ADDVAR, "ADDVAR", 1, 1, 0},
	{0x14, Token::SETVAR, "SETVAR", 2, 2, 0},
	{0x15, Token::SWAPJET, "SWAPJET", 0, 1, 0},
	{0x16, Token::UNKNOWN, "UNKNOWN", 0, 0, 0},
	{0x17, Token::UNKNOWN, "UNKNOWN", 0, 0, 0},
	{0x18, Token::UNKNOWN, "UNKNOWN", 1, 1, 0},
	{0x19, Token::SPFX, "SPFX", 1, 1, 0},
	{0x1a, Token::REDRAW, "REDRAW", 0, 0, 0},
	{0x1b, Token::DELAY, "DELAY", 1, 1, 0},
	{0x1c, Token::SOUND, "SYNCSND", 1, 1, 0},
	{0x1d, Token::TOGGLEBIT, "TOGGLEBIT", 1, 1, 0},
	{0x1e, Token::INVISQ, "IF INVIS?", 1, 1, 0},
	{0x1f, Token::INVISQ, "IF VIS?", 1, 1, 0},
	{0x20, Token::INVISQ, "IF RINVIS?", 2, 2, 0},
	{0x21, Token::INVISQ, "IF RVIS?", 2, 2, 0},
	{0x22, Token::PRINT, "PRINT", 1, 1, 0},
	{0x23, Token::SCREEN, "SCREEN", 1, 1, 0},
	{0x24, Token::SETFLAGS, "SETFLAGS", 1, 1, 0},
	{0x25, Token::STARTANIM, "STARTANIM", 1, 1, 0},
	{0x26, Token::UNKNOWN, "UNKNOWN", 0, 0, 0},
	{0x27, Token::UNKNOWN, "UNKNOWN", 0, 0, 0},
	{0x28, Token::UNKNOWN, "UNKNOWN", 0, 0, 0},
	{0x29, Token::LOOP, "LOOP", 1, 1, 0},
	{0x2a, Token::AGAIN, "AGAIN", 0, 0, 0},
	{0x2b, Token::UNKNOWN, "UNKNOWN", 0, 0, 0},
	{0x2c, Token::ELSE, "ELSE", 0, 0, 0},
	{0x2d, Token::ENDIF, "ENDIF", 0, 0, 0},
	{0x2e, Token::IFGTEQ, "IFGTE", 2, 2, 0},
	{0x2f, Token::IFLTEQ, "IFLTE", 2, 2, 0},
	{0x30, Token::EXECUTE, "EXECUTE", 1, 1, 0}
};

static const struct {
	byte flag;
	const char *name;
} kFreescapeEvents[] = {
	{kConditionalCollided, "COLLIDED?"},
	{kConditionalTimeout, "TIMER?"},
	{kConditionalShot, "SHOT?"},
	{kConditionalActivated, "ACTIVATED?"}
};

static FCLInstruction decodeFreescapeInstruction(const FCLOpcode &entry, const uint16 *operands, bool isAmigaAtari) {
	FCLInstruction instruction(entry.type);
	if (entry.minArgs > 0)
		instruction.setSource(operands[0]);
	if (entry.minArgs > 1)
		instruction.setDestination(operands[1]);

	switch (entry.type) {
	case Token::ADDVAR:
		if (entry.opcode == 0x01) {
			instruction.setSource(k8bitVariableScore);
			instruction.setDestination(operands[0] | (operands[1] << 8) | (operands[2] << 16));
		} else if (entry.opcode == 0x02 || entry.opcode == 0x13) {
			instruction.setSource(entry.opcode == 0x02 ? k8bitVariableEnergy : k8bitVariableShield);
			instruction.setDestination(int8(operands[0]));
		} else {
			instruction.setDestination(1);
		}
		break;
	case Token::SUBVAR:
		instruction.setDestination(1);
		break;
	case Token::TOGVIS:
	case Token::VIS:
	case Token::INVIS:
		if (entry.minArgs == 1)
			instruction.setDestination(0);
		break;
	case Token::INVISQ:
		if (entry.minArgs == 2)
			instruction.setAdditional(operands[1]);
		instruction.setDestination(entry.opcode == 0x1e || entry.opcode == 0x20);
		break;
	case Token::SOUND:
		instruction.setAdditional(entry.opcode == 0x1c);
		break;
	case Token::SPFX:
		instruction.setSource(operands[0] >> (isAmigaAtari ? 8 : 4));
		instruction.setDestination(operands[0] & (isAmigaAtari ? 0xff : 0xf));
		break;
	default:
		break;
	}
	return instruction;
}

static Common::String formatFreescapeInstruction(const FCLOpcode &entry, const FCLInstruction &instruction) {
	int32 operands[2] = {instruction._source, instruction._destination};
	Token::Type types[2] = {Token::CONSTANT, Token::CONSTANT};
	byte count = entry.minArgs;
	switch (entry.type) {
	case Token::ADDVAR:
	case Token::SUBVAR:
		operands[0] = instruction._destination;
		operands[1] = instruction._source;
		count = 2;
		types[1] = Token::VARIABLE;
		break;
	case Token::SETVAR:
	case Token::VARNOTEQ:
	case Token::IFGTEQ:
	case Token::IFLTEQ:
		types[0] = Token::VARIABLE;
		break;
	case Token::INVISQ:
		operands[1] = instruction._additional;
		break;
	case Token::SPFX:
		count = 2;
		break;
	default:
		break;
	}

	Common::String source = entry.name;
	if (count)
		source += " (";
	for (uint i = 0; i < count; i++) {
		if (i)
			source += ", ";
		source += Common::String::format(types[i] == Token::VARIABLE ? "v%d" : "%d", operands[i]);
	}
	if (count)
		source += ")";
	if (entry.type == Token::VARNOTEQ || entry.type == Token::BITNOTEQ || entry.type == Token::INVISQ)
		source += " THEN END ENDIF";
	return source + '\n';
}

Common::String detokeniseFreescapeCondition(const Common::Array<uint16> &tokenisedCondition, FCLInstructionVector &instructions, bool isAmigaAtari) {
	if (tokenisedCondition.empty())
		error("No tokenised content");

	Common::String detokenisedStream = Common::String::format("CONDITION FLAG: %x\n", tokenisedCondition[0]);
	FCLInstructionVector *conditionalInstructions = nullptr;
	byte conditional = 0;
	for (uint bytePointer = 0; bytePointer < tokenisedCondition.size();) {
		uint16 raw = tokenisedCondition[bytePointer++];
		const auto &event = kFreescapeEvents[(raw >> 6) & 3];
		// Consecutive instructions with the same event flag share a branch.
		if (event.flag != conditional) {
			if (conditionalInstructions) {
				detokenisedStream += "ENDIF\n";
				assert(!conditionalInstructions->empty());
			}
			conditional = event.flag;
			conditionalInstructions = new FCLInstructionVector();
			FCLInstruction branch(Token::CONDITIONAL);
			branch.setSource(conditional);
			branch.setBranches(conditionalInstructions, nullptr);
			instructions.push_back(branch);
			detokenisedStream += Common::String::format("IF %s THEN\n", event.name);
		}

		byte opcode = raw & 0x3f;
		const FCLOpcode *entry = findFCLOpcode(kFreescapeOpcodes, opcode);
		if (entry && entry->minArgs > tokenisedCondition.size() - bytePointer)
			break;
		if (!entry || entry->type == Token::UNKNOWN) {
			debugC(1, kFreescapeDebugParser, "%s", detokenisedStream.c_str());
			error("Unknown Freescape opcode %02x at %u", opcode, bytePointer - 1);
		}

		FCLInstruction instruction = decodeFreescapeInstruction(*entry, tokenisedCondition.data() + bytePointer, isAmigaAtari);
		conditionalInstructions->push_back(instruction);
		detokenisedStream += formatFreescapeInstruction(*entry, instruction);
		// SWAPJET has unused padding outside the Amiga/Atari stream.
		bytePointer += isAmigaAtari ? entry->minArgs : entry->maxArgs;
	}
	return detokenisedStream;
}

} // namespace Freescape
