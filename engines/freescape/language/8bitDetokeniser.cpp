//
//  8bitDetokeniser.cpp
//  Phantasma
//
//  Created by Thomas Harte on 15/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

/*
	This has been implemented based on John Elliott's 2001
	reverse engineering of Driller; see http://www.seasip.demon.co.uk/ZX/Driller/
*/
#include "common/debug.h"
#include "8bitDetokeniser.h"
#include "token.h"

namespace Freescape {

Common::String *detokenise8bitCondition(Common::Array<uint8> &tokenisedCondition, FCLInstructionVector &instructions) {
	Common::String detokenisedStream;
	Common::Array<uint8>::size_type bytePointer = 0;
	Common::Array<uint8>::size_type sizeOfTokenisedContent = tokenisedCondition.size();

	// on the 8bit platforms, all instructions have a conditional flag;
	// we'll want to convert them into runs of "if shot? then" and "if collided? then",
	// and we'll want to start that from the top
	uint8 conditionalIsShot = 0x1;
	FCLInstructionVector *conditionalInstructions = new FCLInstructionVector();
	FCLInstruction currentInstruction;

	// this lookup table tells us how many argument bytes to read per opcode
	uint8 argumentsRequiredByOpcode[35] =
		{
			0, 3, 1, 1, 1, 1, 2, 2,
			2, 1, 1, 2, 1, 1, 2, 1,
			1, 2, 2, 1, 2, 0, 0, 0,
			1, 1, 0, 1, 1, 1, 1, 1, 2, 2, 1};

	while (bytePointer < sizeOfTokenisedContent) {
		// get the conditional type of the next operation
		uint8 newConditionalIsShot = tokenisedCondition[bytePointer] & 0x80;

		// if the conditional type has changed then end the old conditional,
		// if we were in one, and begin a new one
		if (newConditionalIsShot != conditionalIsShot) {
			FCLInstruction branch;
			if (conditionalIsShot)
				branch = FCLInstruction(Token::SHOTQ);
			else
				branch = FCLInstruction(Token::COLLIDEDQ);

			branch.setBranches(conditionalInstructions, nullptr);
			instructions.push_back(branch);

			conditionalIsShot = newConditionalIsShot;
			if (bytePointer)
				detokenisedStream += "ENDIF\n";

			if (conditionalIsShot)
				detokenisedStream += "IF SHOT? THEN\n";
			else
				detokenisedStream += "IF COLLIDED? THEN\n";

			// Allocate the next vector of instructions
			conditionalInstructions = new FCLInstructionVector();
		}

		// get the actual operation
		uint16 opcode = tokenisedCondition[bytePointer] & 0x7f;
		bytePointer++;

		// figure out how many argument bytes we're going to need,
		// check we have enough bytes left to read
		if (opcode > 34) {
			debug("ERROR: failed to read opcode: %x", opcode);
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
			if (opcode != 0x18)
				error("Unknown FCL instruction: 0x%x", (int)opcode);
			break;

		case 0:
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
			detokenisedStream += Common::String::format("(%d, v%d)", (int)tokenisedCondition[bytePointer], k8bitVariableEnergy);
			currentInstruction = FCLInstruction(Token::ADDVAR);
			currentInstruction.setSource(k8bitVariableEnergy);
			currentInstruction.setDestination(tokenisedCondition[bytePointer]);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;
		case 19: // add one-byte value to shield
			detokenisedStream += "ADDVAR ";
			detokenisedStream += Common::String::format("(%d, v%d)", (int)tokenisedCondition[bytePointer], k8bitVariableShield);
			currentInstruction = FCLInstruction(Token::ADDVAR);
			currentInstruction.setSource(k8bitVariableShield);
			currentInstruction.setDestination(tokenisedCondition[bytePointer]);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;

		case 6:
		case 3:
			detokenisedStream += "TOGVIS (";
			currentInstruction = FCLInstruction(Token::TOGVIS);
			currentInstruction.source = 0;
			currentInstruction.destination = 0;
			break; // these all come in unary and binary versions,
		case 7:
		case 4:
			detokenisedStream += "VIS (";
			currentInstruction = FCLInstruction(Token::VIS);
			currentInstruction.source = 0;
			currentInstruction.destination = 0;
			break; // hence each getting two case statement entries
		case 8:
		case 5:
			detokenisedStream += "INVIS (";
			currentInstruction = FCLInstruction(Token::INVIS);
			currentInstruction.source = 0;
			currentInstruction.destination = 0;
			break;

		case 9:
			detokenisedStream += "ADDVAR (1, v";
			currentInstruction = FCLInstruction(Token::ADDVAR);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(1);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			break;
		case 10:
			detokenisedStream += "SUBVAR (1, v";
			currentInstruction = FCLInstruction(Token::SUBVAR);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(1);
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			break;
			break;

		case 11: // end condition if a variable doesn't have a particular value
			detokenisedStream += "IF VAR!=? ";
			detokenisedStream += Common::String::format("(v%d, %d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			detokenisedStream += "THEN END ENDIF";
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
			detokenisedStream += "THEN END ENDIF";
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
			detokenisedStream += "THEN END ENDIF";
			currentInstruction = FCLInstruction(Token::INVISQ);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(true); // visible
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;
		case 31: // end condition if an object is visible
			detokenisedStream += "IF VIS? ";
			detokenisedStream += Common::String::format("(%d)", (int)tokenisedCondition[bytePointer]);
			detokenisedStream += "THEN END ENDIF";
			currentInstruction = FCLInstruction(Token::INVISQ);
			currentInstruction.setSource(tokenisedCondition[bytePointer]);
			currentInstruction.setDestination(false); // visible
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
			bytePointer++;
			numberOfArguments = 0;
			break;

		case 32: // end condition if an object is visible in another area
			detokenisedStream += "IF RVIS? ";
			detokenisedStream += Common::String::format("(%d), (%d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			detokenisedStream += "THEN END ENDIF";
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 33: // end condition if an object is invisible in another area
			detokenisedStream += "IF RINVIS? ";
			detokenisedStream += Common::String::format("(%d), (%d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			detokenisedStream += "THEN END ENDIF";
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 34: // show a message on screen
			detokenisedStream += "MESSAGE (";
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
			break;
		case 29:
			detokenisedStream += "TOGGLEBIT (";
			currentInstruction = FCLInstruction(Token::TOGGLEBIT);
			break;

		case 25:
			// this should toggle border colour or the room palette
			detokenisedStream += "SPFX (";
		break;

		case 20:
			detokenisedStream += "SETVAR ";
			detokenisedStream += Common::String::format("(%d, v%d)", (int)tokenisedCondition[bytePointer], (int)tokenisedCondition[bytePointer + 1]);
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 23:
			detokenisedStream += "UNKNOWN(23)(..) ";
			bytePointer += 2;
			numberOfArguments = 0;
			break;

		case 22:
			detokenisedStream += "UNKNOWN(22)(..) ";
			bytePointer += 2;
			numberOfArguments = 0;
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
			conditionalInstructions->push_back(currentInstruction);
			currentInstruction = FCLInstruction(Token::UNKNOWN);
		}

		// throw in a newline
		detokenisedStream += "\n";
	}

	//if (!conditionalInstructions)
	//	conditionalInstructions = new FCLInstructionVector();

	//conditionalInstructions->push_back(currentInstruction);

	FCLInstruction branch;
	if (conditionalIsShot)
		branch = FCLInstruction(Token::SHOTQ);
	else
		branch = FCLInstruction(Token::COLLIDEDQ);

	branch.setBranches(conditionalInstructions, nullptr);
	instructions.push_back(branch);

	return (new Common::String(detokenisedStream));
}

} // End of namespace Freescape