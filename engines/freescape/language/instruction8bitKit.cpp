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

#include "math/utils.h"

#include "freescape/games/3dck/8bit.h"

namespace Freescape {

void Kit8Engine::readSystemVariables() {
	float scale = 2 * _currentArea->getScale();
	for (int axis = 0; axis < 3; axis++) {
		uint16 position = int32(round(_position.getValue(axis) * scale));
		_variables[112 + 2 * axis] = position;
		_variables[113 + 2 * axis] = position >> 8;
	}
	_variables[118] = (int(round(_pitch / 5)) % 72 + 72) % 72;
	_variables[119] = (int(round((90 - _yaw) / 5)) % 72 + 72) % 72;
	_variables[120] = (int(round(_roll / 5)) % 72 + 72) % 72;
	_variables[121] = _currentKey;
	_variables[124] = _currentArea->getAreaID();
	_changedVariables = 0;
}

void Kit8Engine::writeSystemVariables() {
	float scale = 2 * _currentArea->getScale();
	for (int axis = 0; axis < 3; axis++) {
		if (_changedVariables & (3 << (2 * axis)))
			_position.setValue(axis, (_variables[112 + 2 * axis] | (_variables[113 + 2 * axis] << 8)) / scale);
	}
	if (_changedVariables & (1 << 6)) {
		_pitch = 5 * (_variables[118] % 72);
		if (_pitch > 180)
			_pitch -= 360;
	}
	if (_changedVariables & (1 << 7))
		_yaw = 90 - 5 * _variables[119];
	if (_changedVariables & (1 << 8))
		_roll = 5 * _variables[120];
	if (_changedVariables & 0x1c0)
		updateCamera();
	if (_changedVariables & 0x3f)
		_lastPosition = _position;
	_changedVariables = 0;
}

void Kit8Engine::updateTimeVariables() {
	uint32 now = g_system->getMillis();
	uint32 elapsed = (now - _lastTime) / 20;
	_lastTime += 20 * elapsed;
	uint16 counter = (_variables[122] | (_variables[123] << 8)) + elapsed;
	_variables[122] = counter;
	_variables[123] = counter >> 8;
	_timerTicks += elapsed;
	uint32 interval = MAX<uint32>(1, _timerInterval);
	if (_timerTicks >= interval) {
		_pendingTimer = true;
		_timerTicks %= interval;
	}
}

void Kit8Engine::startScript(const FCLInstructionVector &code) {
	if (_scriptStack.size() >= 64)
		error("8-bit 3D Construction Kit procedure stack overflow");
	_scriptStack.push_back(ScriptFrame(code));
	_executing = true;
	_booleanOp = Token::UNKNOWN;
}

void Kit8Engine::updateScripts() {
	_fallen |= _hasFallen;
	_crushed |= _playerWasCrushed;
	_hasFallen = _playerWasCrushed = false;
	_avoidRenderingFrames = 0;
	if (_delayUntil && int32(_delayUntil - g_system->getMillis()) > 0)
		return;
	_delayUntil = 0;
	if (!_scriptFrameActive) {
		_scriptFrameActive = true;
		readSystemVariables();
		_timerTriggered = _pendingTimer;
		_pendingTimer = false;
		_scriptSurface.fillRect(_viewArea, 255);
		_conditions = &_globalConditions;
		_conditionIndex = 0;
		_globalPhase = true;
		if (_initialScriptPending) {
			_initialScriptPending = false;
			for (const auto &condition : _globalConditions) {
				if (condition.id == 1) {
					startScript(condition.code);
					break;
				}
			}
		}
	}
	uint budget = 4096;
	while (budget) {
		if (_scriptStack.empty()) {
			if (_conditionIndex == _conditions->size()) {
				if (!_globalPhase)
					break;
				_globalPhase = false;
				_conditions = &_areaData[_currentArea->getAreaID()].conditions;
				_conditionIndex = 0;
				continue;
			}
			const ConditionData &condition = (*_conditions)[_conditionIndex++];
			if (_globalPhase && condition.id == 1)
				continue;
			startScript(condition.code);
		}
		if (!executeCode(budget))
			return;
	}
	writeSystemVariables();
	if (!_scriptStack.empty() || _globalPhase || _conditionIndex < _conditions->size())
		return;
	updateInstruments();
	_scriptFrameActive = false;
	_shotObject = _hitObject = _activatedObject = 0;
}

bool Kit8Engine::executeCode(uint &budget) {
	while (!_scriptStack.empty() && budget) {
		ScriptFrame &frame = _scriptStack.back();
		if (frame.ip == frame.code->size()) {
			_scriptStack.pop_back();
			continue;
		}
		const FCLInstruction &instruction = (*frame.code)[frame.ip++];
		Token::Type op = instruction.getType();
		budget--;
		// Condition statements also execute while a THEN/ELSE branch is skipped.
		switch (op) {
		case Token::ENDOFFILE:
			_scriptStack.pop_back();
			continue;
		case Token::CONDITIONAL:
			executeConditional(instruction);
			continue;
		case Token::VISQ:
		case Token::IFEQ:
		case Token::IFGT:
		case Token::IFLT:
			executeComparison(instruction);
			continue;
		case Token::AND:
		case Token::OR:
			_previousZero = _zero;
			_booleanOp = op;
			continue;
		case Token::THEN:
			_executing = _zero;
			continue;
		case Token::ELSE:
			_executing = !_executing;
			continue;
		case Token::ENDIF:
			_executing = true;
			continue;
		default:
			break;
		}
		if (!_executing)
			continue;
		switch (op) {
		case Token::SETVAR:
		case Token::ADDVAR:
		case Token::ADCV:
		case Token::SUBVAR:
		case Token::SBCV:
		case Token::ANDV:
		case Token::ORV:
		case Token::XORV:
		case Token::TESTV:
		case Token::CMPV:
			executeArithmetic(instruction);
			break;
		case Token::VIS:
		case Token::INVIS:
		case Token::TOGVIS:
		case Token::DESTROY:
			executeObjectStatus(instruction);
			break;
		case Token::GOTO:
			executeGoto(instruction);
			break;
		case Token::MODE:
			executeMode(instruction);
			break;
		case Token::EXECUTE:
			executeCall(instruction);
			break;
		case Token::END:
			_scriptStack.pop_back();
			break;
		case Token::ENDGAME:
			_gameStateControl = kFreescapeGameStateRestart;
			_scriptStack.clear();
			return false;
		case Token::TIMER:
			_timerInterval = instruction._source;
			break;
		case Token::CROSS:
			_crossVisible = instruction._source != 0;
			break;
		case Token::COLOUR:
			executeColour(instruction);
			break;
		case Token::SOUND:
		case Token::SYNCSND:
			executeSound(instruction);
			break;
		case Token::DELAY:
			_delayUntil = g_system->getMillis() + 20 * (instruction._source ? instruction._source : 256);
			return false;
		case Token::TEXTCOL:
			_textColor = instruction._source;
			break;
		case Token::PRINT:
			printMessage(instruction._source, instruction._destination, instruction._additional);
			break;
		case Token::REDRAW:
			writeSystemVariables();
			_scriptSurface.fillRect(_viewArea, 255);
			updateInstruments();
			return false;
		default:
			error("Unsupported 8-bit 3D Construction Kit instruction %u", op);
		}
	}
	return _scriptStack.empty();
}

void Kit8Engine::setPredicate(bool value) {
	if (_booleanOp == Token::AND)
		value = _previousZero && value;
	else if (_booleanOp == Token::OR)
		value = _previousZero || value;
	_zero = value;
	_booleanOp = Token::UNKNOWN;
}

void Kit8Engine::executeArithmetic(const FCLInstruction &instruction) {
	byte index = instruction._destination & 127;
	byte source = instruction._sourceType == Token::VARIABLE ? _variables[instruction._source & 127] : instruction._source;
	int destination = _variables[index];
	int result = destination;
	bool store = true;
	switch (instruction.getType()) {
	case Token::SETVAR: result = source; break;
	case Token::ADDVAR: result += source; break;
	case Token::ADCV: result += source + (_carry ? 1 : 0); break;
	case Token::SUBVAR: result -= source; break;
	case Token::SBCV: result -= source + (_carry ? 1 : 0); break;
	case Token::ANDV: result &= source; break;
	case Token::ORV: result |= source; break;
	case Token::XORV: result ^= source; break;
	case Token::TESTV:
		result &= source;
		store = false;
		break;
	case Token::CMPV:
		result = source - destination;
		store = false;
		break;
	default:
		break;
	}
	if (store) {
		_variables[index] = result;
		if (index >= 112 && index <= 120)
			_changedVariables |= 1 << (index - 112);
	}
	_carry = result < 0 || result > 255;
	if (instruction.getType() == Token::SETVAR) {
		// SETV retains the CPC operand decoder's zero flag.
		setPredicate(instruction._sourceType != Token::VARIABLE);
	} else
		setPredicate(byte(result) == 0);
}

Object *Kit8Engine::scriptObject(uint16 area, uint16 id) {
	if (!_areaMap.contains(area))
		return nullptr;
	Object *object = _areaMap[area]->objectWithID(id);
	if (!object)
		object = _areaMap[area]->entranceWithID(id);
	return object;
}

void Kit8Engine::executeComparison(const FCLInstruction &instruction) {
	if (instruction.getType() == Token::VISQ) {
		uint16 area = instruction._destinationType == Token::UNKNOWN ? _currentArea->getAreaID() : instruction._destination;
		Object *object = scriptObject(area, instruction._source);
		_carry = false;
		setPredicate(!object || (!object->isInvisible() && !object->isDestroyed()));
	} else if (instruction.getType() == Token::IFGT)
		setPredicate(!_zero && _carry);
	else if (instruction.getType() == Token::IFLT)
		setPredicate(!_zero && !_carry);
	else
		setPredicate(_zero);
}

void Kit8Engine::executeConditional(const FCLInstruction &instruction) {
	int area = instruction._additionalType == Token::UNKNOWN ? _currentArea->getAreaID() : instruction._additional;
	int id = instruction._destination;
	int value = 0;
	switch (instruction._source) {
	case kConditionalShot: value = _shotObject; break;
	case kConditionalCollided: value = _hitObject; break;
	case kConditionalActivated: value = _activatedObject; break;
	case kConditionalTimeout:
		_carry = false;
		setPredicate(_timerTriggered);
		return;
	case kConditionalFallen:
		setPredicate(_fallen);
		_fallen = false;
		return;
	case kConditionalCrushed:
		setPredicate(_crushed);
		_crushed = false;
		return;
	case kConditionalSensed: {
		Object *object = scriptObject(area, id);
		_carry = false;
		setPredicate(!object || (object->getType() == kSensorType && static_cast<Sensor *>(object)->isShooting()));
		return;
	}
	default:
		break;
	}
	if (area != _currentArea->getAreaID()) {
		value = _currentArea->getAreaID();
		id = area;
	}
	_carry = value < id;
	setPredicate(value == id);
}

void Kit8Engine::executeObjectStatus(const FCLInstruction &instruction) {
	uint16 area = instruction._destinationType == Token::UNKNOWN ? _currentArea->getAreaID() : instruction._destination;
	Object *object = scriptObject(area, instruction._source);
	if (!object || object->isDestroyed())
		return;
	switch (instruction.getType()) {
	case Token::VIS: object->makeVisible(); break;
	case Token::INVIS: object->makeInvisible(); break;
	case Token::TOGVIS: object->toggleVisibility(); break;
	case Token::DESTROY:
		object->makeInvisible();
		object->destroy();
		break;
	default:
		break;
	}
}

void Kit8Engine::executeGoto(const FCLInstruction &instruction) {
	uint16 area = instruction._destinationType == Token::UNKNOWN ? _currentArea->getAreaID() : instruction._destination;
	writeSystemVariables();
	gotoArea(area, instruction._source);
}

void Kit8Engine::executeMode(const FCLInstruction &instruction) {
	writeSystemVariables();
	setMovementMode(instruction._source);
	readSystemVariables();
}

void Kit8Engine::executeCall(const FCLInstruction &instruction) {
	for (const auto &procedure : _procedures) {
		if (procedure.id == instruction._source) {
			startScript(procedure.code);
			return;
		}
	}
}

void Kit8Engine::executeColour(const FCLInstruction &instruction) {
	_palette[instruction._source & 3] = MIN<int>(26, instruction._destination);
	applyPalette();
}

void Kit8Engine::executeSound(const FCLInstruction &instruction) {
	if (instruction._source && !_soundWarning) {
		warning("8-bit 3D Construction Kit sound effects are not implemented");
		_soundWarning = true;
	}
}

bool Kit8Engine::executeObjectConditions(GeometricObject *object, bool shot, bool collided, bool activated) {
	if (shot)
		_shotObject = object->getObjectID();
	if (collided)
		_hitObject = object->getObjectID();
	if (activated)
		_activatedObject = object->getObjectID();
	return false;
}

} // namespace Freescape
