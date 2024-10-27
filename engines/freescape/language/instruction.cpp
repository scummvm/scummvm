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
#include "freescape/sweepAABB.h"

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

Token::Type FCLInstruction::getType() const {
	return _type;
}

void FreescapeEngine::executeEntranceConditions(Entrance *entrance) {
	if (!entrance->_conditionSource.empty()) {
		_firstSound = true;
		_syncSound = false;

		debugC(1, kFreescapeDebugCode, "Executing entrance condition with collision flag: %s", entrance->_conditionSource.c_str());
		executeCode(entrance->_condition, false, true, false, false);
	}
}

bool FreescapeEngine::executeObjectConditions(GeometricObject *obj, bool shot, bool collided, bool activated) {
	bool executed = false;
	assert(obj != nullptr);
	if (!obj->_conditionSource.empty()) {
		_firstSound = true;
		_syncSound = false;
		_objExecutingCodeSize = collided ? obj->getSize() : Math::Vector3d();
		if (collided) {
			clearGameBit(31); // We collided with something that has code
			debugC(1, kFreescapeDebugCode, "Executing with collision flag: %s", obj->_conditionSource.c_str());
		} else if (shot)
			debugC(1, kFreescapeDebugCode, "Executing with shot flag: %s", obj->_conditionSource.c_str());
		else if (activated)
			debugC(1, kFreescapeDebugCode, "Executing with activated flag: %s", obj->_conditionSource.c_str());
		else
			error("Neither shot or collided flag is set!");
		executed = executeCode(obj->_condition, shot, collided, false, activated); // TODO: check this last parameter
	}
	if (activated && !executed)
		if (!_noEffectMessage.empty())
			insertTemporaryMessage(_noEffectMessage, _countdown - 2);

	return executed;
}

void FreescapeEngine::executeLocalGlobalConditions(bool shot, bool collided, bool timer) {
	debugC(1, kFreescapeDebugCode, "Executing room conditions");
	Common::Array<FCLInstructionVector> conditions = _currentArea->_conditions;
	Common::Array<Common::String> conditionSources = _currentArea->_conditionSources;

	for (uint i = 0; i < conditions.size(); i++) {
		debugC(1, kFreescapeDebugCode, "%s", conditionSources[i].c_str());
		executeCode(conditions[i], shot, collided, timer, false);
	}

	_executingGlobalCode = true;
	debugC(1, kFreescapeDebugCode, "Executing global conditions (%d)", _conditions.size());
	for (uint i = 0; i < _conditions.size(); i++) {
		debugC(1, kFreescapeDebugCode, "%s", _conditionSources[i].c_str());
		executeCode(_conditions[i], shot, collided, timer, false);
	}
	_executingGlobalCode = false;
}

bool FreescapeEngine::executeCode(FCLInstructionVector &code, bool shot, bool collided, bool timer, bool activated) {
	int ip = 0;
	bool skip = false;
	int skipDepth = 0;
	int conditionalDepth = 0;
	bool executed = false;
	int codeSize = code.size();

	if (codeSize == 0) {
		assert(isCastle()); // Only seems to happen in Castle Master (magister room)
		debugC(1, kFreescapeDebugCode, "Code is empty!");
		return false;
	}

	while (ip <= codeSize - 1) {
		FCLInstruction &instruction = code[ip];
		debugC(1, kFreescapeDebugCode, "Executing ip: %d with type %d in code with size: %d. Skip flag is: %d", ip, instruction.getType(), codeSize, skip);

		if (instruction.isConditional()) {
			conditionalDepth++;
			debugC(1, kFreescapeDebugCode, "Conditional depth increased to: %d", conditionalDepth);
		} else if (instruction.getType() == Token::ENDIF) {
			conditionalDepth--;
			debugC(1, kFreescapeDebugCode, "Conditional depth decreased to: %d", conditionalDepth);
		}

		if (skip) {
			if (instruction.getType() == Token::ELSE) {
				debugC(1, kFreescapeDebugCode, "Else found, skip depth: %d, conditional depth: %d", skipDepth, conditionalDepth);
				if (skipDepth == conditionalDepth - 1) {
					skip = false;
				}
			} else if (instruction.getType() == Token::ENDIF) {
				debugC(1, kFreescapeDebugCode, "Endif found, skip depth: %d, conditional depth: %d", skipDepth, conditionalDepth);
				if (skipDepth == conditionalDepth) {
					skip = false;
				}
			}
			debugC(1, kFreescapeDebugCode, "Instruction skipped!");
			ip++;
			continue;
		}

		if (instruction.getType() != Token::CONDITIONAL)
			executed = true;

		switch (instruction.getType()) {
		default:
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
			if (executeEndIfNotEqual(instruction)) {
				if (isCastle()) {
					skip = true;
					skipDepth = conditionalDepth - 1;
				} else
					ip = codeSize;
			}
			break;
		case Token::IFGTEQ:
			skip = !checkIfGreaterOrEqual(instruction);
			if (skip)
				skipDepth = conditionalDepth - 1;
			break;

		case Token::IFLTEQ:
			skip = !checkIfLessOrEqual(instruction);
			if (skip)
				skipDepth = conditionalDepth - 1;
			break;


		case Token::ELSE:
			skip = !skip;
			if (skip)
				skipDepth = conditionalDepth - 1;
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
		case Token::EXECUTE:
			executeExecute(instruction, shot, collided, activated);
			ip = codeSize;
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
		case Token::SETFLAGS:
			// TODO
			break;
		case Token::STARTANIM:
			executeStartAnim(instruction);
			break;
		case Token::BITNOTEQ:
			if (executeEndIfBitNotEqual(instruction)) {
				if (isCastle()) {
					skip = true;
					skipDepth = conditionalDepth - 1;
				} else
					ip = codeSize;
			}
			break;
		case Token::INVISQ:
			if (executeEndIfVisibilityIsEqual(instruction)) {
				if (isCastle()) {
					skip = true;
					skipDepth = conditionalDepth - 1;
				} else
					ip = codeSize;
			}
			break;
		}
		ip++;
	}
	return executed;
}

void FreescapeEngine::executeRedraw(FCLInstruction &instruction) {
	debugC(1, kFreescapeDebugCode, "Redrawing screen");
	waitInLoop((100 / 15) + 1);
	if (_syncSound) {
		waitForSounds();
	}
}

void FreescapeEngine::executeExecute(FCLInstruction &instruction, bool shot, bool collided, bool activated) {
	uint16 objId = instruction._source;
	debugC(1, kFreescapeDebugCode, "Executing instructions from object %d", objId);
	Object *obj = _currentArea->objectWithID(objId);
	if (!obj) {
		obj = _areaMap[255]->objectWithID(objId);
		if (!obj) {
			obj = _areaMap[255]->entranceWithID(objId);
			assert(obj);
			executeEntranceConditions((Entrance *)obj);
			return;
		}
	}
	executeObjectConditions((GeometricObject *)obj, shot, collided, activated);
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
	waitInLoop(((20 * delay) / 15) + 1);
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
		uint8 r = 0;
		uint8 g = 0;
		uint8 b = 0;
		uint32 color = 0;

		if (src & (1 << 7)) {
			uint16 v = 0;
			color = 0;
			// Extract the color to replace from the src/dst values
			v = (src & 0x77) << 8;
			v = v | (dst & 0x70);
			v = v >> 4;

			// Convert the color to RGB
			r = (v & 0xf00) >> 8;
			r = r << 4 | r;
			r = r & 0xff;

			g = (v & 0xf0) >> 4;
			g = g << 4 | g;
			g = g & 0xff;

			b = v & 0xf;
			b = b << 4 | b;
			b = b & 0xff;

			color = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
			_currentArea->remapColor(dst & 0x0f, color); // src & 0x77, dst & 0x0f
		} else if ((src & 0xf0) >> 4 == 1) {
			_gfx->readFromPalette(src & 0x0f, r, g, b);
			color = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
			for (int i = 1; i < 16; i++)
				_currentArea->remapColor(i, color);
		} else if ((src & 0x0f) == 1) {
			_gfx->readFromPalette(dst & 0x0f, r, g, b);
			color = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
			for (int i = 1; i < 16; i++)
				_currentArea->remapColor(i, color);
		}
	} else {
		debugC(1, kFreescapeDebugCode, "Switching palette from position %d to %d", src, dst);
		if (src == 0 && dst == 1)
			_currentArea->remapColor(_currentArea->_usualBackgroundColor, _renderMode == Common::kRenderCGA ? 1 : _currentArea->_underFireBackgroundColor);
		else if (src == 0 && dst == 0)
			_currentArea->unremapColor(_currentArea->_usualBackgroundColor);
		else if (src == 15 && dst == 15) // Found in Total Eclipse (DOS)
			_currentArea->unremapColor(_currentArea->_usualBackgroundColor);
		else
			_currentArea->remapColor(src, dst);
	}
	_gfx->setColorRemaps(&_currentArea->_colorRemaps);
	executeRedraw(instruction);
}


bool FreescapeEngine::executeEndIfVisibilityIsEqual(FCLInstruction &instruction) {
	uint16 source = instruction._source;
	uint16 additional = instruction._additional;
	uint16 value = instruction._destination;

	Object *obj = nullptr;
	if (additional == 0) {
		obj = _currentArea->objectWithID(source);
		if (!obj && isCastle())
			return false; // The value is not important
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
	assert(instruction._destination <= 128);

	uint16 variable = instruction._source;
	int8 value = instruction._destination;
	debugC(1, kFreescapeDebugCode, "Check if variable %d is greater than equal to %d!", variable, value);
	return ((int8)_gameStateVars[variable] >= value);
}

bool FreescapeEngine::checkIfLessOrEqual(FCLInstruction &instruction) {
	assert(instruction._destination <= 128);

	uint16 variable = instruction._source;
	int8 value = instruction._destination;
	debugC(1, kFreescapeDebugCode, "Check if variable %d is less than equal to %d!", variable, value);
	return ((int8)_gameStateVars[variable] <= value);
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
	if (variable == k8bitVariableScore) {
		debugC(1, kFreescapeDebugCode, "Score incremented by %d up to %d", increment, _gameStateVars[variable]);
	} else if (variable == k8bitVariableEnergy) {
		if (_gameStateVars[variable] > _maxEnergy)
			_gameStateVars[variable] = _maxEnergy;
		else if (_gameStateVars[variable] < 0)
			_gameStateVars[variable] = 0;
		debugC(1, kFreescapeDebugCode, "Energy incremented by %d up to %d", increment, _gameStateVars[variable]);
	} else if (variable == k8bitVariableShield) {
		if (_gameStateVars[variable] > _maxShield)
			_gameStateVars[variable] = _maxShield;
		else if (_gameStateVars[variable] < 0)
			_gameStateVars[variable] = 0;

		if (increment < 0 && !isCastle())
			flashScreen(_renderMode == Common::kRenderCGA ? 1 :_currentArea->_underFireBackgroundColor);

		debugC(1, kFreescapeDebugCode, "Shield incremented by %d up to %d", increment, _gameStateVars[variable]);
	} else {
		debugC(1, kFreescapeDebugCode, "Variable %d by %d incremented up to %d!", variable, increment, _gameStateVars[variable]);
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
		assert(isDriller() && isDOS() && isDemo());
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
		if (!obj && isCastle() && _executingGlobalCode)
			return; // No side effects

		if (!obj) {
			obj = _areaMap[255]->objectWithID(objectID);
			if (!obj) {
				error("obj %d does not exists in area %d nor in the global one!", objectID, areaID);
				return;
			}
			_currentArea->addObjectFromArea(objectID, _areaMap[255]);
			obj = _areaMap[areaID]->objectWithID(objectID);
			assert(obj); // We know that an object should be there
		}

		obj->makeVisible();
		if (!isDriller()) {
			Math::AABB boundingBox = createPlayerAABB(_position, _playerHeight);
			if (obj->_boundingBox.collides(boundingBox)) {
				_playerWasCrushed = true;
				_avoidRenderingFrames = 60 * 3;
				if (isEclipse())
					playSoundFx(2, true);
				_shootingFrames = 0;
			}
		}
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
	if (!obj->isInvisible()) {
		if (!isDriller()) {
			Math::AABB boundingBox = createPlayerAABB(_position, _playerHeight);
			if (obj->_boundingBox.collides(boundingBox)) {
				_playerWasCrushed = true;
				_shootingFrames = 0;
			}
		}
	}
}

void FreescapeEngine::executeGoto(FCLInstruction &instruction) {
	uint16 areaID = instruction._source;
	uint16 entranceID = instruction._destination;
	gotoArea(areaID, entranceID);
	_gotoExecuted = true;
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
	uint16 index = instruction._source;
	uint16 value = instruction._destination;
	assert(index <= 32);
	debugC(1, kFreescapeDebugCode, "End condition if bit %d is not equal to %d!", index, value);
	return (getGameBit(index) != value);
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

void FreescapeEngine::executeStartAnim(FCLInstruction &instruction) {
	uint16 objID = instruction._source;
	debugC(1, kFreescapeDebugCode, "Staring animation of object %d", objID);
	Object *obj = _currentArea->objectWithID(objID);
	assert(obj);
	Group *group = nullptr;
	if (obj->getType() == kGroupType) {
		group = (Group *)obj;
	} else {
		assert(obj->_partOfGroup);
		group = (Group *)obj->_partOfGroup;
	}
	debugC(1, kFreescapeDebugCode, "From group %d", group->getObjectID());
	if (!group->isDestroyed())
		group->start();
}


} // End of namespace Freescape
