//
//  Instruction.cpp
//  Phantasma
//
//  Created by Thomas Harte on 08/12/2013.
//  Copyright (c) 2013 Thomas Harte. All rights reserved.
//

#include "freescape/language/instruction.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/freescape.h"

namespace Freescape {

FCLInstruction::FCLInstruction(Token::Type _type) {
	// TODO: learn modern constructor syntax
	type = _type;
	thenInstructions = nullptr;
	elseInstructions = nullptr;
}

FCLInstruction::FCLInstruction() {
	type = Token::UNKNOWN;
	thenInstructions = nullptr;
	elseInstructions = nullptr;
}

void FCLInstruction::setSource(int32 _source) {
	source = _source;
}

void FCLInstruction::setDestination(int32 _destination) {
	destination = _destination;
}

void FCLInstruction::setBranches(FCLInstructionVector *thenBranch, FCLInstructionVector *elseBranch) {
	thenInstructions = thenBranch;
	elseInstructions = elseBranch;
}

Token::Type FCLInstruction::getType() {
	return type;
}

void FreescapeEngine::executeCode(FCLInstructionVector &code, bool shot, bool collided) {
	assert(!(shot && collided));
	int ip = 0;
	int codeSize = code.size();
	while (ip <= codeSize - 1) {
		FCLInstruction &instruction = code[ip];
		debug("Executing ip: %d in code with size: %d", ip, codeSize);
		switch (instruction.getType()) {
			default:
			break;
			case Token::COLLIDEDQ:
			if (collided)
				executeCode(*instruction.thenInstructions, shot, collided);
			// else branch is always empty
			assert(instruction.elseInstructions == nullptr);
			break;
			case Token::SHOTQ:
			if (shot)
				executeCode(*instruction.thenInstructions, shot, collided);
			// else branch is always empty
			assert(instruction.elseInstructions == nullptr);
			break;
			case Token::VARNOTEQ:
			if (executeEndIfNotEqual(instruction))
				ip = codeSize;
			break;
			case Token::ADDVAR:
			executeIncrementVariable(instruction);
			break;
			case Token::SUBVAR:
			executeDecrementVariable(instruction);
			break;
			case Token::GOTO:
			executeGoto(instruction);
			break;
			case Token::TOGVIS:
			executeToggleVisibility(instruction);
			break;
			case Token::INVIS:
			executeMakeInvisible(instruction);
			break;
			case Token::VIS:
			executeMakeVisible(instruction);
			break;
			case Token::DESTROY:
			executeDestroy(instruction);
			break;
			case Token::REDRAW:
			executeRedraw(instruction);
			break;
			case Token::DELAY:
			executeDelay(instruction);
			break;
			case Token::SETBIT:
			executeSetBit(instruction);
			break;
			case Token::CLEARBIT:
			executeClearBit(instruction);
			break;
			case Token::BITNOTEQ:
			if (executeEndIfBitNotEqual(instruction))
				ip = codeSize;
			break;
		}
		ip++;
	}
	return;
}


void FreescapeEngine::executeRedraw(FCLInstruction &instruction) {
	debug("Redrawing screen");
	drawFrame();
}

void FreescapeEngine::executeDelay(FCLInstruction &instruction) {
	uint16 delay = instruction.source;
	debug("Delaying %d * 1/50 seconds", delay);
	g_system->delayMillis(20 * delay);
}

bool FreescapeEngine::executeEndIfNotEqual(FCLInstruction &instruction) {
	uint16 variable1 = instruction.source;
	uint16 variable2 = instruction.destination;
	debug("End condition if variable %d is not equal to variable %d!", variable1, variable2);
	return (_gameState[variable1] != _gameState[variable2]);
}

void FreescapeEngine::executeIncrementVariable(FCLInstruction &instruction) {
	uint16 variable = instruction.source;
	uint16 increment = instruction.destination;
	_gameState[variable] = _gameState[variable] + increment;
	if (variable == k8bitVariableScore) {
		debug("Score incremented by %d up to %d", increment, _gameState[variable]);
	} else
		debug("Variable %d by %d incremented up to %d!", variable, increment, _gameState[variable]);
}

void FreescapeEngine::executeDecrementVariable(FCLInstruction &instruction) {
	uint16 variable = instruction.source;
	uint16 decrement = instruction.destination;
	_gameState[variable] = _gameState[variable] - decrement;
	if (variable == k8bitVariableEnergy) {
		debug("Energy decrement by %d up to %d", decrement, _gameState[variable]);
	} else
		debug("Variable %d by %d incremented up to %d!", variable, decrement, _gameState[variable]);
}

void FreescapeEngine::executeDestroy(FCLInstruction &instruction) {
	uint16 objectID = instruction.source;
	debug("Destroying obj %d!", objectID);
	Object *obj = _currentArea->objectWithID(objectID);
	assert(!obj->isDestroyed());
	obj->destroy();
}

void FreescapeEngine::executeMakeInvisible(FCLInstruction &instruction) {
	uint16 objectID = instruction.source;
	debug("Making obj %d invisible!", objectID);
	Object *obj = _currentArea->objectWithID(objectID);
	obj->makeInvisible();
}

void FreescapeEngine::executeMakeVisible(FCLInstruction &instruction) {
	uint16 objectID = instruction.source;
	debug("Making obj %d visible!", objectID);
	Object *obj = _currentArea->objectWithID(objectID);
	obj->makeVisible();
}

void FreescapeEngine::executeToggleVisibility(FCLInstruction &instruction) {
	uint16 objectID = instruction.source;
	debug("Toggling obj %d visibility!", objectID);
	Object *obj = _currentArea->objectWithID(objectID);
	obj->toggleVisibility();
}

void FreescapeEngine::executeGoto(FCLInstruction &instruction) {
	uint16 areaID = instruction.source;
	uint16 entranceID = instruction.destination;
	gotoArea(areaID, entranceID);
}

void FreescapeEngine::executeSetBit(FCLInstruction &instruction) {
	uint16 index = instruction.source;
	_areaBits[index] = true;
	debug("Setting bit %d", index);
}

void FreescapeEngine::executeClearBit(FCLInstruction &instruction) {
	uint16 index = instruction.source;
	_areaBits[index] = false;
	debug("Clearing bit %d", index);
}

void FreescapeEngine::executeToggleBit(FCLInstruction &instruction) {
	uint16 index = instruction.source;
	_areaBits[index] = ~_areaBits[index];
	debug("Toggling bit %d", index);
}

bool FreescapeEngine::executeEndIfBitNotEqual(FCLInstruction &instruction) {
	uint16 index = instruction.source;
	uint16 value = instruction.destination;
	debug("End condition if bit %d is not equal to %d!", index, value);
	return (_areaBits[index] != value);
}

} // End of namespace Freescape