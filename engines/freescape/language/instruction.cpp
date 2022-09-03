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

// Based on Phantasma code by Thomas Harte (2013)

#include "freescape/language/8bitDetokeniser.h"
#include "freescape/freescape.h"

namespace Freescape {

FCLInstruction::FCLInstruction(Token::Type _type) {
	source = 0;
	destination = 0;
	additional = 0;
	// TODO: learn modern constructor syntax
	type = _type;
	thenInstructions = nullptr;
	elseInstructions = nullptr;
}

FCLInstruction::FCLInstruction() {
	source = 0;
	destination = 0;
	additional = 0;
	type = Token::UNKNOWN;
	thenInstructions = nullptr;
	elseInstructions = nullptr;
}

void FCLInstruction::setSource(int32 _source) {
	source = _source;
}

void FCLInstruction::setAdditional(int32 _additional) {
	additional = _additional;
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

void FreescapeEngine::executeConditions(GeometricObject *obj, bool shot, bool collided) {
	if (obj->conditionSource != nullptr) {
		debugC(1, kFreescapeDebugCode, "Executing with collision flag: %s", obj->conditionSource->c_str());
		executeCode(obj->condition, shot, collided);
	}

	if (!isDriller())
		return;

	debugC(1, kFreescapeDebugCode, "Executing room conditions");
	for (int i = 0; i < int(_currentArea->conditions.size()); i++) {
		debugC(1, kFreescapeDebugCode, "%s", _currentArea->conditionSources[i]->c_str());
		executeCode(_currentArea->conditions[i], shot, collided);
	}

	debugC(1, kFreescapeDebugCode, "Executing global conditions (%d)", _conditions.size());
	for (int i = 0; i < int(_conditions.size()); i++) {
		debugC(1, kFreescapeDebugCode, "%s", _conditionSources[i]->c_str());
		executeCode(_conditions[i], shot, collided);
	}
}

void FreescapeEngine::executeCode(FCLInstructionVector &code, bool shot, bool collided) {
	assert(!(shot && collided));
	int ip = 0;
	int codeSize = code.size();
	while (ip <= codeSize - 1) {
		FCLInstruction &instruction = code[ip];
		debugC(1, kFreescapeDebugCode, "Executing ip: %d in code with size: %d", ip, codeSize);
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
			case Token::SWAPJET:
			executeSwapJet(instruction);
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
			case Token::SOUND:
			executeSound(instruction);
			break;
			case Token::SETBIT:
			executeSetBit(instruction);
			break;
			case Token::CLEARBIT:
			executeClearBit(instruction);
			break;
			case Token::PRINT:
			executePrint(instruction);
			break;
			case Token::BITNOTEQ:
			if (executeEndIfBitNotEqual(instruction))
				ip = codeSize;
			break;
			case Token::INVISQ:
			if (executeEndIfVisibilityIsEqual(instruction))
				ip = codeSize;
			break;
		}
		ip++;
	}
	return;
}


void FreescapeEngine::executeRedraw(FCLInstruction &instruction) {
	debugC(1, kFreescapeDebugCode, "Redrawing screen");
	drawFrame();
}

void FreescapeEngine::executeSound(FCLInstruction &instruction) {
	uint16 index = instruction.source;
	debugC(1, kFreescapeDebugCode, "Playing sound %d", index);
	playSound(index);
}

void FreescapeEngine::executeDelay(FCLInstruction &instruction) {
	uint16 delay = instruction.source;
	debugC(1, kFreescapeDebugCode, "Delaying %d * 1/50 seconds", delay);
	g_system->delayMillis(20 * delay);
}

void FreescapeEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction.source - 1;
	debugC(1, kFreescapeDebugCode, "Printing message %d", index);
	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_messagesList[index]);
}

bool FreescapeEngine::executeEndIfVisibilityIsEqual(FCLInstruction &instruction) {
	uint16 source = instruction.source;
	uint16 additional = instruction.additional;
	uint16 value = instruction.destination;

	Object *obj = nullptr;
	if (additional == 0) {
		obj = _currentArea->objectWithID(source);
		assert(obj);
		debugC(1, kFreescapeDebugCode, "End condition if visibility of obj with id %d is %d!", source, value);
	} else {
		assert(_areaMap.contains(source));
		obj = _areaMap[source]->objectWithID(additional);
		assert(obj);
		debugC(1, kFreescapeDebugCode, "End condition if visibility of obj with id %d in area %d is %d!", additional, source, value);
	}

	return (obj->isInvisible() == value);
}

bool FreescapeEngine::executeEndIfNotEqual(FCLInstruction &instruction) {
	uint16 variable = instruction.source;
	uint16 value = instruction.destination;
	debugC(1, kFreescapeDebugCode, "End condition if variable %d is not equal to %d!", variable, value);
	return (_gameStateVars[variable] != value);
}

void FreescapeEngine::executeIncrementVariable(FCLInstruction &instruction) {
	int32 variable = instruction.source;
	int32 increment = instruction.destination;
	_gameStateVars[variable] = _gameStateVars[variable] + increment;
	switch (variable) {
	case k8bitVariableScore:
		debugC(1, kFreescapeDebugCode, "Score incremented by %d up to %d", increment, _gameStateVars[variable]);
	break;
	case k8bitVariableEnergy:
		if (_gameStateVars[variable] > k8bitMaxEnergy)
			_gameStateVars[variable] = k8bitMaxEnergy;
		else if (_gameStateVars[variable] < 0)
			_gameStateVars[variable] = 0;
		debugC(1, kFreescapeDebugCode, "Energy incremented by %d up to %d", increment, _gameStateVars[variable]);
	break;
	case k8bitVariableShield:
		if (_gameStateVars[variable] > k8bitMaxShield)
			_gameStateVars[variable] = k8bitMaxShield;
		else if (_gameStateVars[variable] < 0)
			_gameStateVars[variable] = 0;

		debugC(1, kFreescapeDebugCode, "Shield incremented by %d up to %d", increment, _gameStateVars[variable]);
	break;
	default:
		debugC(1, kFreescapeDebugCode, "Variable %d by %d incremented up to %d!", variable, increment, _gameStateVars[variable]);
	break;
	}
}

void FreescapeEngine::executeDecrementVariable(FCLInstruction &instruction) {
	uint16 variable = instruction.source;
	uint16 decrement = instruction.destination;
	_gameStateVars[variable] = _gameStateVars[variable] - decrement;
	if (variable == k8bitVariableEnergy) {
		debugC(1, kFreescapeDebugCode, "Energy decrement by %d up to %d", decrement, _gameStateVars[variable]);
	} else
		debugC(1, kFreescapeDebugCode, "Variable %d by %d incremented up to %d!", variable, decrement, _gameStateVars[variable]);
}

void FreescapeEngine::executeDestroy(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction.destination > 0) {
		objectID = instruction.destination;
		areaID = instruction.source;
	} else {
		objectID = instruction.source;
	}

	debugC(1, kFreescapeDebugCode, "Destroying obj %d in area %d!", objectID, areaID);
	assert(_areaMap.contains(areaID));
	Object *obj = _areaMap[areaID]->objectWithID(objectID);
	if (obj->isDestroyed())
		debugC(1, kFreescapeDebugCode, "WARNING: Destroying obj %d in area %d already destroyed!", objectID, areaID);

	obj->destroy();
}

void FreescapeEngine::executeMakeInvisible(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction.destination > 0) {
		objectID = instruction.destination;
		areaID = instruction.source;
	} else {
		objectID = instruction.source;
	}

	debugC(1, kFreescapeDebugCode, "Making obj %d invisible in area %d!", objectID, areaID);
	Object *obj = _areaMap[areaID]->objectWithID(objectID);
	obj->makeInvisible();
}

void FreescapeEngine::executeMakeVisible(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction.destination > 0) {
		objectID = instruction.destination;
		areaID = instruction.source;
	} else {
		objectID = instruction.source;
	}

	debugC(1, kFreescapeDebugCode, "Making obj %d visible in area %d!", objectID, areaID);
	Object *obj = _areaMap[areaID]->objectWithID(objectID);
	obj->makeVisible();
}

void FreescapeEngine::executeToggleVisibility(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction.destination > 0) {
		objectID = instruction.destination;
		areaID = instruction.source;
	} else {
		objectID = instruction.source;
	}

	debugC(1, kFreescapeDebugCode, "Toggling obj %d visibility in area %d!", objectID, areaID);
	Object *obj = _areaMap[areaID]->objectWithID(objectID);
	if (obj)
		obj->toggleVisibility();
	else {
		obj = _areaMap[255]->objectWithID(objectID);
		if (!obj)
			error("ERROR!: obj %d does not exists in area %d nor in the global one!", objectID, areaID);

		// If an object is not in the area, it is considered to be invisible
		_currentArea->addObjectFromArea(objectID, _areaMap[255]);
		obj = _areaMap[areaID]->objectWithID(objectID);
		obj->makeVisible();
	}

}

void FreescapeEngine::executeGoto(FCLInstruction &instruction) {
	uint16 areaID = instruction.source;
	uint16 entranceID = instruction.destination;
	gotoArea(areaID, entranceID);
}

void FreescapeEngine::executeSetBit(FCLInstruction &instruction) {
	uint16 index = instruction.source - 1; // Starts in 1
	assert(index < 32);
	_gameStateBits[_currentArea->getAreaID()] |= (1 << index);
	debugC(1, kFreescapeDebugCode, "Setting bit %d", index);
	//debug("v: %d", (_gameStateBits[_currentArea->getAreaID()] & (1 << index)));
}

void FreescapeEngine::executeClearBit(FCLInstruction &instruction) {
	uint16 index = instruction.source - 1; // Starts in 1
	assert(index < 32);
	_gameStateBits[_currentArea->getAreaID()] &= ~(1 << index);
	debugC(1, kFreescapeDebugCode, "Clearing bit %d", index);
}

void FreescapeEngine::executeToggleBit(FCLInstruction &instruction) {
	uint16 index = instruction.source - 1; // Starts in 1
	_gameStateBits[_currentArea->getAreaID()] ^= (1 << index);
	debugC(1, kFreescapeDebugCode, "Toggling bit %d", index);
}

bool FreescapeEngine::executeEndIfBitNotEqual(FCLInstruction &instruction) {
	uint16 index = instruction.source - 1; // Starts in 1
	uint16 value = instruction.destination;
	assert(index < 32);
	debugC(1, kFreescapeDebugCode, "End condition if bit %d is not equal to %d!", index, value);
	return (((_gameStateBits[_currentArea->getAreaID()] >> index) & 1) != value);
}

void FreescapeEngine::executeSwapJet(FCLInstruction &instruction) {
	_flyMode = !_flyMode;
	if (_flyMode)
		debugC(1, kFreescapeDebugCode, "Swaping to ship mode");
	else
		debugC(1, kFreescapeDebugCode, "Swaping to tank mode");
	// TODO: implement the rest of the changes (e.g. border)
}

} // End of namespace Freescape