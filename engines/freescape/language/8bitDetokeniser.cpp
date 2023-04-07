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
// http://www.seasip.demon.co.uk/ZX/Driller/

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

uint8 k8bitMaxVariable = 64;
uint8 k8bitMaxShield = 64;
uint8 k8bitMaxEnergy = 64;

Common::String detokenise8bitCondition(Common::Array<uint8> &tokenisedCondition, FCLInstructionVector &instructions) {
	Common::String detokenisedStream;
	Common::Array<uint8>::size_type bytePointer = 0;
	Common::Array<uint8>::size_type sizeOfTokenisedContent = tokenisedCondition.size();

	// on the 8bit platforms, all instructions have a conditional flag;
	// we'll want to convert them into runs of "if shot? then", "if collided? then" or "if timer? then",
	// and we'll want to start that from the top
	FCLInstructionVector *conditionalInstructions = new FCLInstructionVector();
	FCLInstruction currentInstruction = FCLInstruction(Token::UNKNOWN);

	// this lookup table tells us how many argument bytes to read per opcode
	uint8 argumentsRequiredByOpcode[49] =
		{0, 3, 1, 1, 1, 1, 2, 2,
		 2, 1, 1, 2, 1, 1, 2, 1,
		 1, 2, 2, 1, 2, 0, 0, 0,
		 1, 1, 0, 1, 1, 1, 1, 1,
		 2, 2, 1, 1, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 2, 2,
		 1};

	if (sizeOfTokenisedContent > 0)
		detokenisedStream += Common::String::format("CONDITION FLAG: %x\n", tokenisedCondition[0]);
	Token::Type newConditional = Token::UNKNOWN;
	Token::Type oldConditional = Token::UNKNOWN;

	while (bytePointer < sizeOfTokenisedContent) {
		// get the conditional type of the next operation
		uint8 conditionalByte = tokenisedCondition[bytePointer];

		if (conditionalByte & 0x80)
			newConditional = Token::SHOTQ;
		else if (conditionalByte & 0x40)
			newConditional = Token::TIMERQ;
		else
			newConditional = Token::COLLIDEDQ;

		// if the conditional type has changed then end the old conditional,
		// if we were in one, and begin a new one
		if (bytePointer == 0 || newConditional != oldConditional) {
			oldConditional = newConditional;
			FCLInstruction branch;
			branch = FCLInstruction(oldConditional);

			branch.setBranches(conditionalInstructions, nullptr);
			instructions.push_back(branch);

			if (bytePointer > 0) {
				detokenisedStream += "ENDIF\n";
				// Allocate the next vector of instructions
				conditionalInstructions = new FCLInstructionVector();
			}

			if (oldConditional == Token::SHOTQ)
				detokenisedStream += "IF SHOT? THEN\n";
			else if (oldConditional == Token::TIMERQ)
				detokenisedStream += "IF TIMER? THEN\n";
			else if (oldConditional == Token::COLLIDEDQ)
				detokenisedStream += "IF COLLIDED? THEN\n";
			else
				error("Invalid conditional: %x", oldConditional);
		}

		// get the actual operation
		uint16 opcode = tokenisedCondition[bytePointer] & 0x3f;
		bytePointer++;

		// figure out how many argument bytes we're going to need,
		// check we have enough bytes left to read
		if (opcode > 48) {
			debugC(1, kFreescapeDebugParser, "%s", detokenisedStream.c_str());
			if (opcode != 0x3f)
				error("ERROR: failed to read opcode: %x", opcode);
			break;
		}

		uint8 numberOfArguments = argumentsRequiredByOpcode[opcode];
		if (bytePointer + numberOfArguments > sizeOfTokenisedContent)
			break;

		// generate the string
		switch (opcode) {
		default:
			detokenisedStream += "<UNKNOWN 8 bit: ";
			detokenisedStream += Common::String::format("%x", (int)opcode);
			detokenisedStream += " > ";
			break;

		case 0:
			detokenisedStream += "NOP ";
			currentInstruction = FCLInstruction(Token::NOP);
			break; // NOP
		case 1:    // add three-byte value to score
		{
			int32 additionValue =
				tokenisedCondition[bytePointer] |
				(tokenisedCondition[bytePointer + 1] << 8) |
				(tokenisedCondition[bytePointer + 2] << 16);
			detokenisedStream += "ADDVAR";
			detokenisedStream += Common::String::format("(%d, v%d)", additionValue, k8bitVariableScore);
			currentInstruction = FCLInstruction(Token::ADDVAR);
			currentInstruction.setSource(k8bitVariableScore);
			currentInstruction.setDestination(additionValue);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer += 3;
			numberOfArguments = 0;
		} break;
		case 2: // add one-byte value to energy
			detokenisedStream += "ADDVAR ";
			detokenisedStream += Common::String::format("(%d, v%d)", (int8)tokenisedCondition[bytePointer], k8bitVariableEnergy);
			currentInstruction = FCLInstruction(Token::ADDVAR);
			currentInstruction.setSource(k8bitVariableEnergy);
			currentInstruction.setDestination((int8)tokenisedCondition[bytePointer]);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;
		case 19: // add one-byte value to shield
			detokenisedStream += "ADDVAR ";
			detokenisedStream += Common::String::format("(%d, v%d)", (int8)tokenisedCondition[bytePointer], k8bitVariableShield);
			currentInstruction = FCLInstruction(Token::ADDVAR);
			currentInstruction.setSource(k8bitVariableShield);
			currentInstruction.setDestination((int8)tokenisedCondition[bytePointer]);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;

		case 6:
		case 3:
			detokenisedStream += "TOGVIS (";
			currentInstruction = FCLInstruction(Token::TOGVIS);
			currentInstruction.setSource(0);
			currentInstruction.setDestination(0);
			break; // these all come in unary and binary versions,
		case 7:
		case 4:
			detokenisedStream += "VIS (";
			currentInstruction = FCLInstruction(Token::VIS);
			currentInstruction.setSource(0);
			currentInstruction.setDestination(0);
			break; // hence each getting two case statement entries
		case 8:
		case 5:
			detokenisedStream += "INVIS (";
			currentInstruction = FCLInstruction(Token::INVIS);
			currentInstruction.setSource(0);
			currentInstruction.setDestination(0);
			break;

		case 9:
			detokenisedStream += "ADDVAR (1, v";
			currentInstruction = FCLInstruction(Token::ADDVAR);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(1);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;
		case 10:
			detokenisedStream += "SUBVAR (1, v";
			currentInstruction = FCLInstruction(Token::SUBVAR);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(1);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;

		case 11: // end condition if a variable doesn't have a particular value
			detokenisedStream += "IF VAR!=? ";
			detokenisedStream += Common::String::format("(v%d, %d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			detokenisedStream += " THEN END ENDIF";
			currentInstruction = FCLInstruction(Token::VARNOTEQ);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(tokenisedCondition[bytePointer + 1]);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer += 2;
			numberOfArguments = 0;
			break;
		case 14: // end condition if a bit doesn't have a particular value
			detokenisedStream += "IF BIT!=? ";
			detokenisedStream += Common::String::format("(%d, %d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			detokenisedStream += " THEN END ENDIF";
			currentInstruction = FCLInstruction(Token::BITNOTEQ);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(tokenisedCondition[bytePointer + 1]);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer += 2;
			numberOfArguments = 0;
			break;
		case 30: // end condition if an object is invisible
			detokenisedStream += "IF INVIS? ";
			detokenisedStream += Common::String::format("(%d)", (int)tokenisedCondition[bytePointer]);
			detokenisedStream += " THEN END ENDIF";
			currentInstruction = FCLInstruction(Token::INVISQ);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(true); // invisible
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;
		case 31: // end condition if an object is visible
			detokenisedStream += "IF VIS? ";
			detokenisedStream += Common::String::format("(%d)", (int)tokenisedCondition[bytePointer]);
			detokenisedStream += " THEN END ENDIF";
			currentInstruction = FCLInstruction(Token::INVISQ);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(false); // visible
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;

		case 32: // end condition if an object is visible in another area
			detokenisedStream += "IF RINVIS? ";
			detokenisedStream += Common::String::format("(%d, %d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			detokenisedStream += " THEN END ENDIF";
			currentInstruction = FCLInstruction(Token::INVISQ);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setAdditional(tokenisedCondition[bytePointer + 1]);
			currentInstruction.setDestination(true); // invisible
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 33: // end condition if an object is invisible in another area
			detokenisedStream += "IF RVIS? ";
			detokenisedStream += Common::String::format("(%d, %d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			detokenisedStream += " THEN END ENDIF";
			currentInstruction = FCLInstruction(Token::INVISQ);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setAdditional(tokenisedCondition[bytePointer + 1]);
			currentInstruction.setDestination(false); // visible
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 34: // show a message on screen
			detokenisedStream += "PRINT (";
			currentInstruction = FCLInstruction(Token::PRINT);
			break;

		case 12:
			detokenisedStream += "SETBIT (";
			currentInstruction = FCLInstruction(Token::SETBIT);
			break;
		case 13:
			detokenisedStream += "CLRBIT (";
			currentInstruction = FCLInstruction(Token::CLEARBIT);
			break;

		case 15:
			detokenisedStream += "SOUND (";
			currentInstruction = FCLInstruction(Token::SOUND);
			currentInstruction.setAdditional(false);
			break;
		case 17:
		case 16:
			detokenisedStream += "DESTROY (";
			currentInstruction = FCLInstruction(Token::DESTROY);
			break;
		case 18:
			detokenisedStream += "GOTO (";
			currentInstruction = FCLInstruction(Token::GOTO);
			break;

		case 21:
			detokenisedStream += "SWAPJET";
			currentInstruction = FCLInstruction(Token::SWAPJET);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;
		case 26:
			detokenisedStream += "REDRAW";
			currentInstruction = FCLInstruction(Token::REDRAW);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			break;
		case 27:
			detokenisedStream += "DELAY (";
			currentInstruction = FCLInstruction(Token::DELAY);
			break;
		case 28:
			detokenisedStream += "SYNCSND (";
			currentInstruction = FCLInstruction(Token::SOUND);
			currentInstruction.setAdditional(true);
			break;
		case 29:
			detokenisedStream += "TOGGLEBIT (";
			currentInstruction = FCLInstruction(Token::TOGGLEBIT);
			break;

		case 25:
			// this should toggle border colour or the room palette
			detokenisedStream += "SPFX (";
			currentInstruction = FCLInstruction(Token::SPFX);
			currentInstruction.setSource(tokenisedCondition[bytePointer] >> 4);
			currentInstruction.setDestination(tokenisedCondition[bytePointer] & 0xf);
			detokenisedStream += Common::String::format("%d, %d)", currentInstruction._source, currentInstruction._destination);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;

		case 20:
			detokenisedStream += "SETVAR ";
			detokenisedStream += Common::String::format("(v%d, %d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 35:
			detokenisedStream += "SCREEN (";
			currentInstruction = FCLInstruction(Token::SCREEN);
			break;

		case 44:
			detokenisedStream += "ELSE ";
			break;

		case 45:
			detokenisedStream += "ENDIF ";
			break;

		case 46:
			detokenisedStream += "IFGTE ";
			detokenisedStream += Common::String::format("(v%d, %d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 47:
			detokenisedStream += "IFLTE ";
			detokenisedStream += Common::String::format("(v%d, %d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 48:
			detokenisedStream += "EXECUTE ";
			currentInstruction = FCLInstruction(Token::EXECUTE);
			break;
		}

		// if there are any regular arguments to add, do so
		if (numberOfArguments) {
			for (uint8 argumentNumber = 0; argumentNumber < numberOfArguments; argumentNumber++) {
				if (argumentNumber == 0)
					currentInstruction.setSource(tokenisedCondition[bytePointer]);
				else if (argumentNumber == 1)
					currentInstruction.setDestination(tokenisedCondition[bytePointer]);
				else
					error("Unexpected number of arguments!");

				detokenisedStream += Common::String::format("%d", (int)tokenisedCondition[bytePointer]);
				bytePointer++;

				if (argumentNumber < numberOfArguments - 1)
					detokenisedStream += ", ";
			}

			detokenisedStream += ")";
			assert(currentInstruction.getType() != Token::UNKNOWN);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
		}

		// throw in a newline
		detokenisedStream += "\n";
	}

	return detokenisedStream;
}

} // End of namespace Freescape
