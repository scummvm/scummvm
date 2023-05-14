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

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

FCLInstructionVector *duplicateCondition(FCLInstructionVector *condition) {
	if (!condition)
		return nullptr;

	FCLInstructionVector *copy = new FCLInstructionVector();
	for (uint i = 0; i < condition->size(); i++) {
		copy->push_back((*condition)[i].duplicate());
	}
	return copy;
}

FCLInstruction FCLInstruction::duplicate() {
	FCLInstruction copy(_type);
	copy.setSource(_source);
	copy.setDestination(_destination);
	copy.setAdditional(_additional);

	copy._thenInstructions = duplicateCondition(_thenInstructions);
	copy._elseInstructions = duplicateCondition(_elseInstructions);

	return copy;
}

FCLInstruction::FCLInstruction(Token::Type type_) {
	_source = 0;
	_destination = 0;
	_additional = 0;
	_type = type_;
	_thenInstructions = nullptr;
	_elseInstructions = nullptr;
}

FCLInstruction::FCLInstruction() {
	_source = 0;
	_destination = 0;
	_additional = 0;
	_type = Token::UNKNOWN;
	_thenInstructions = nullptr;
	_elseInstructions = nullptr;
}

void FCLInstruction::setSource(int32 source_) {
	_source = source_;
}

void FCLInstruction::setAdditional(int32 additional_) {
	_additional = additional_;
}

void FCLInstruction::setDestination(int32 destination_) {
	_destination = destination_;
}

void FCLInstruction::setBranches(FCLInstructionVector *thenBranch, FCLInstructionVector *elseBranch) {
	_thenInstructions = thenBranch;
	_elseInstructions = elseBranch;
}

Token::Type FCLInstruction::getType() {
	return _type;
}

void FreescapeEngine::executeObjectConditions(GeometricObject *obj, bool shot, bool collided, bool activated) {
	assert(obj != nullptr);
	if (!obj->_conditionSource.empty()) {
		_firstSound = true;
		_objExecutingCodeSize = collided ? obj->getSize() : Math::Vector3d();
		if (collided)
			debugC(1, kFreescapeDebugCode, "Executing with collision flag: %s", obj->_conditionSource.c_str());
		else if (shot)
			debugC(1, kFreescapeDebugCode, "Executing with shot flag: %s", obj->_conditionSource.c_str());
		else if (activated)
			debugC(1, kFreescapeDebugCode, "Executing with activated flag: %s", obj->_conditionSource.c_str());
		else
			error("Neither shot or collided flag is set!");
		executeCode(obj->_condition, shot, collided, false, activated); // TODO: check this last parameter
	}
}

void FreescapeEngine::executeLocalGlobalConditions(bool shot, bool collided, bool timer) {
	if (isCastle())
		return;
	debugC(1, kFreescapeDebugCode, "Executing room conditions");
	Common::Array<FCLInstructionVector> conditions = _currentArea->_conditions;
	Common::Array<Common::String> conditionSources = _currentArea->_conditionSources;

	for (uint i = 0; i < conditions.size(); i++) {
		debugC(1, kFreescapeDebugCode, "%s", conditionSources[i].c_str());
		executeCode(conditions[i], shot, collided, timer, false);
	}

	debugC(1, kFreescapeDebugCode, "Executing global conditions (%d)", _conditions.size());
	for (uint i = 0; i < _conditions.size(); i++) {
		debugC(1, kFreescapeDebugCode, "%s", _conditionSources[i].c_str());
		executeCode(_conditions[i], shot, collided, timer, false);
	}
}

void FreescapeEngine::executeCode(FCLInstructionVector &code, bool shot, bool collided, bool timer, bool activated) {
	assert(!(shot && collided));
	int ip = 0;
	bool skip = false;
	int codeSize = code.size();
	assert(codeSize > 0);
	while (ip <= codeSize - 1) {
		FCLInstruction &instruction = code[ip];
		debugC(1, kFreescapeDebugCode, "Executing ip: %d with type %d in code with size: %d", ip, instruction.getType(), codeSize);

		if (skip && instruction.getType() != Token::ELSE && instruction.getType() != Token::ENDIF) {
			debugC(1, kFreescapeDebugCode, "Instruction skipped!");
			ip++;
			continue;
		}

		switch (instruction.getType()) {
		default:
			//if (!isCastle())
			error("Instruction %x at ip: %d not implemented!", instruction.getType(), ip);
			break;
		case Token::NOP:
			debugC(1, kFreescapeDebugCode, "Executing NOP at ip: %d", ip);
			break;

		case Token::CONDITIONAL:
			if (checkConditional(instruction, shot, collided, timer, activated))
				executeCode(*instruction._thenInstructions, shot, collided, timer, activated);
			// else branch is always empty
			assert(instruction._elseInstructions == nullptr);
			break;

		case Token::VARNOTEQ:
			if (executeEndIfNotEqual(instruction))
				ip = codeSize;
			break;
		case Token::IFGTEQ:
			skip = !checkIfGreaterOrEqual(instruction);
			break;

		case Token::ELSE:
			skip = !skip;
			break;

		case Token::ENDIF:
			skip = false;
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
		case Token::SETVAR:
			executeSetVariable(instruction);
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
		case Token::TOGGLEBIT:
			executeToggleBit(instruction);
			break;
		case Token::PRINT:
			executePrint(instruction);
			break;
		case Token::SPFX:
			executeSPFX(instruction);
			break;
		case Token::SCREEN:
			// TODO
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
	_gfx->flipBuffer();
	g_system->updateScreen();
	g_system->delayMillis(10);
	waitForSounds();
}

void FreescapeEngine::executeSound(FCLInstruction &instruction) {
	if (_firstSound)
		stopAllSounds();
	_firstSound = false;
	uint16 index = instruction._source;
	bool sync = instruction._additional;
	debugC(1, kFreescapeDebugCode, "Playing sound %d", index);
	playSound(index, sync);
}

void FreescapeEngine::executeDelay(FCLInstruction &instruction) {
	uint16 delay = instruction._source;
	debugC(1, kFreescapeDebugCode, "Delaying %d * 1/50 seconds", delay);
	g_system->delayMillis(20 * delay);
}

void FreescapeEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source - 1;
	debugC(1, kFreescapeDebugCode, "Printing message %d: \"%s\"", index, _messagesList[index].c_str());
	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_messagesList[index]);
}

void FreescapeEngine::executeSPFX(FCLInstruction &instruction) {
	uint16 src = instruction._source;
	uint16 dst = instruction._destination;
	if (isAmiga() || isAtariST()) {
		int color;
		if (src == 0 && dst >= 2 && dst <= 5) {
			_currentArea->remapColor(dst, 1);
			return;
		}

		if (src == 0) {
			color = dst;
		} else {

			switch (src) {
			case 1:
				color = 15;
			break;
			case 2:
				color = 14;
			break;
			default:
				color = 0;
			}
		}

		debugC(1, kFreescapeDebugCode, "Switching complete palette to color %d", dst);
		for (int i = 1; i < 16; i++)
			_currentArea->remapColor(i, color);
	} else {
		debugC(1, kFreescapeDebugCode, "Switching palette from position %d to %d", src, dst);
		if (src == 0 && dst == 1)
			_currentArea->remapColor(_currentArea->_usualBackgroundColor, _renderMode == Common::kRenderCGA ? 1 : _currentArea->_underFireBackgroundColor);
		else if (src == 0 && dst == 0)
			_currentArea->unremapColor(_currentArea->_usualBackgroundColor);
		else
			_currentArea->remapColor(src, dst);
	}
	executeRedraw(instruction);
}


bool FreescapeEngine::executeEndIfVisibilityIsEqual(FCLInstruction &instruction) {
	uint16 source = instruction._source;
	uint16 additional = instruction._additional;
	uint16 value = instruction._destination;

	Object *obj = nullptr;
	if (additional == 0) {
		obj = _currentArea->objectWithID(source);
		assert(obj);
		debugC(1, kFreescapeDebugCode, "End condition if visibility of obj with id %d is %d!", source, value);
	} else {
		debugC(1, kFreescapeDebugCode, "End condition if visibility of obj with id %d in area %d is %d!", additional, source, value);
		if (_areaMap.contains(source)) {
			obj = _areaMap[source]->objectWithID(additional);
			assert(obj);
		} else {
			assert(isDOS() && isDemo()); // Should only happen in the DOS demo
			return (value == false);
		}
	}

	return (obj->isInvisible() == (value != 0));
}

bool FreescapeEngine::checkConditional(FCLInstruction &instruction, bool shot, bool collided, bool timer, bool activated) {
	uint16 conditional = instruction._source;
	bool result = false;

	if (conditional & kConditionalShot)
		result |= shot;
	if (conditional & kConditionalTimeout)
		result |= timer;
	if (conditional & kConditionalCollided)
		result |= collided;
	if (conditional & kConditionalActivated)
		result |= activated;

	debugC(1, kFreescapeDebugCode, "Check if conditional %x is true: %d!", conditional, result);
	return result;
}

bool FreescapeEngine::checkIfGreaterOrEqual(FCLInstruction &instruction) {
	uint16 variable = instruction._source;
	uint16 value = instruction._destination;
	debugC(1, kFreescapeDebugCode, "Check if variable %d is greater than equal to %d!", variable, value);
	return (_gameStateVars[variable] >= value);
}


bool FreescapeEngine::executeEndIfNotEqual(FCLInstruction &instruction) {
	uint16 variable = instruction._source;
	uint16 value = instruction._destination;
	debugC(1, kFreescapeDebugCode, "End condition if variable %d is not equal to %d!", variable, value);
	return (_gameStateVars[variable] != value);
}

void FreescapeEngine::executeIncrementVariable(FCLInstruction &instruction) {
	int32 variable = instruction._source;
	int32 increment = instruction._destination;
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

		if (increment < 0)
			flashScreen(_renderMode == Common::kRenderCGA ? 1 :_currentArea->_underFireBackgroundColor);

		debugC(1, kFreescapeDebugCode, "Shield incremented by %d up to %d", increment, _gameStateVars[variable]);
		break;
	default:
		debugC(1, kFreescapeDebugCode, "Variable %d by %d incremented up to %d!", variable, increment, _gameStateVars[variable]);
		break;
	}
}

void FreescapeEngine::executeDecrementVariable(FCLInstruction &instruction) {
	uint16 variable = instruction._source;
	uint16 decrement = instruction._destination;
	_gameStateVars[variable] = _gameStateVars[variable] - decrement;
	if (variable == k8bitVariableEnergy) {
		debugC(1, kFreescapeDebugCode, "Energy decrement by %d up to %d", decrement, _gameStateVars[variable]);
	} else
		debugC(1, kFreescapeDebugCode, "Variable %d by %d incremented up to %d!", variable, decrement, _gameStateVars[variable]);
}

void FreescapeEngine::executeSetVariable(FCLInstruction &instruction) {
	uint16 variable = instruction._source;
	uint16 value = instruction._destination;
	_gameStateVars[variable] = value;
	if (variable == k8bitVariableEnergy)
		debugC(1, kFreescapeDebugCode, "Energy set to %d", value);
	else
		debugC(1, kFreescapeDebugCode, "Variable %d by set to %d!", variable, value);
}

void FreescapeEngine::executeDestroy(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction._destination > 0) {
		objectID = instruction._destination;
		areaID = instruction._source;
	} else {
		objectID = instruction._source;
	}

	debugC(1, kFreescapeDebugCode, "Destroying obj %d in area %d!", objectID, areaID);
	assert(_areaMap.contains(areaID));
	Object *obj = _areaMap[areaID]->objectWithID(objectID);
	assert(obj); // We know that an object should be there
	if (obj->isDestroyed())
		debugC(1, kFreescapeDebugCode, "WARNING: Destroying obj %d in area %d already destroyed!", objectID, areaID);

	obj->destroy();
}

void FreescapeEngine::executeMakeInvisible(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction._destination > 0) {
		objectID = instruction._destination;
		areaID = instruction._source;
	} else {
		objectID = instruction._source;
	}

	debugC(1, kFreescapeDebugCode, "Making obj %d invisible in area %d!", objectID, areaID);
	if (_areaMap.contains(areaID)) {
		Object *obj = _areaMap[areaID]->objectWithID(objectID);
		assert(obj); // We assume the object was there
		obj->makeInvisible();
	} else {
		assert(isDOS() && isDemo()); // Should only happen in the DOS demo
	}

}

void FreescapeEngine::executeMakeVisible(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction._destination > 0) {
		objectID = instruction._destination;
		areaID = instruction._source;
	} else {
		objectID = instruction._source;
	}

	debugC(1, kFreescapeDebugCode, "Making obj %d visible in area %d!", objectID, areaID);
	if (_areaMap.contains(areaID)) {
		Object *obj = _areaMap[areaID]->objectWithID(objectID);
		assert(obj); // We assume an object should be there
		obj->makeVisible();
	} else {
		assert(isDOS() && isDemo()); // Should only happen in the DOS demo
	}
}

void FreescapeEngine::executeToggleVisibility(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction._destination > 0) {
		objectID = instruction._destination;
		areaID = instruction._source;
	} else {
		objectID = instruction._source;
	}

	debugC(1, kFreescapeDebugCode, "Toggling obj %d visibility in area %d!", objectID, areaID);
	Object *obj = _areaMap[areaID]->objectWithID(objectID);
	if (obj)
		obj->toggleVisibility();
	else {
		obj = _areaMap[255]->objectWithID(objectID);
		if (!obj) {
			// This happens in Driller, the ketar hangar
			warning("ERROR!: obj %d does not exists in area %d nor in the global one!", objectID, areaID);
			return;
		}
		// If an object is not in the area, it is considered to be invisible
		_currentArea->addObjectFromArea(objectID, _areaMap[255]);
		obj = _areaMap[areaID]->objectWithID(objectID);
		assert(obj); // We know that an object should be there
		obj->makeVisible();
	}
}

void FreescapeEngine::executeGoto(FCLInstruction &instruction) {
	uint16 areaID = instruction._source;
	uint16 entranceID = instruction._destination;
	gotoArea(areaID, entranceID);
}

void FreescapeEngine::executeSetBit(FCLInstruction &instruction) {
	uint16 index = instruction._source; // Starts at 1
	assert(index > 0 && index <= 32);
	setGameBit(index);
	debugC(1, kFreescapeDebugCode, "Setting bit %d", index);
}

void FreescapeEngine::executeClearBit(FCLInstruction &instruction) {
	uint16 index = instruction._source; // Starts at 1
	assert(index > 0 && index <= 32);
	clearGameBit(index);
	debugC(1, kFreescapeDebugCode, "Clearing bit %d", index);
}

void FreescapeEngine::executeToggleBit(FCLInstruction &instruction) {
	uint16 index = instruction._source; // Starts at 1
	assert(index > 0 && index <= 32);
	toggleGameBit(index);
	debugC(1, kFreescapeDebugCode, "Toggling bit %d", index);
}

bool FreescapeEngine::executeEndIfBitNotEqual(FCLInstruction &instruction) {
	uint16 index = instruction._source - 1; // Starts in 1
	uint16 value = instruction._destination;
	assert(index < 32);
	debugC(1, kFreescapeDebugCode, "End condition if bit %d is not equal to %d!", index, value);
	return (((_gameStateBits[_currentArea->getAreaID()] >> index) & 1) != value);
}

void FreescapeEngine::executeSwapJet(FCLInstruction &instruction) {
	//playSound(15, false);
	_flyMode = !_flyMode;
	uint16 areaID = _currentArea->getAreaID();

	if (_flyMode) {
		debugC(1, kFreescapeDebugCode, "Swaping to ship mode");
		if (areaID == 27) {
			traverseEntrance(26);
			_lastPosition = _position;
		}
		_playerHeight = 2;
		_playerHeightNumber = -1;

		// Save tank energy and shield
		_gameStateVars[k8bitVariableEnergyDrillerTank] = _gameStateVars[k8bitVariableEnergy];
		_gameStateVars[k8bitVariableShieldDrillerTank] = _gameStateVars[k8bitVariableShield];

		// Restore ship energy and shield
		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergyDrillerJet];
		_gameStateVars[k8bitVariableShield] = _gameStateVars[k8bitVariableShieldDrillerJet];
	} else {
		debugC(1, kFreescapeDebugCode, "Swaping to tank mode");
		_playerHeightNumber = 0;
		if (areaID == 27) {
			traverseEntrance(27);
			_lastPosition = _position;
		}

		// Save shield energy and shield
		_gameStateVars[k8bitVariableEnergyDrillerJet] = _gameStateVars[k8bitVariableEnergy];
		_gameStateVars[k8bitVariableShieldDrillerJet] = _gameStateVars[k8bitVariableShield];

		// Restore ship energy and shield
		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergyDrillerTank];
		_gameStateVars[k8bitVariableShield] = _gameStateVars[k8bitVariableShieldDrillerTank];
	}
	// TODO: implement the rest of the changes (e.g. border)
}

} // End of namespace Freescape
