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

void Kit8Engine::resetScripts() {
	stopAllSounds();
	_pendingSound = 0;
	_soundSyncReady = false;
	memset(_kitVariables, 0, sizeof(_kitVariables));
	_changedVariables = 0;
	_currentKey = 255;
	_kitVariables[121] = _kitVariables[125] = 255;
	_kitVariables[127] = 0x9c;
	_script = ScriptState();
	_activeConditions = nullptr;
	_initialScriptPending = true;
	_scriptFrameActive = false;
	_redrawPending = false;
	_shotObject = _hitObject = _activatedObject = 0;
	_fallen = _crushed = _pendingTimer = _timerTriggered = false;
	_crossVisible = true;
	_timerTicks = _timerInterval = _delayUntil = 0;
	_lastTime = g_system->getMillis();
	_nextFrameTime = _lastTime;
	_scriptSurface.fillRect(_fullscreenViewArea, 255);
	memcpy(_attributes, _borderAttributes, sizeof(_attributes));
}

void Kit8Engine::readSystemVariables() {
	float scale = 2 * _currentArea->getScale();
	for (int axis = 0; axis < 3; axis++) {
		uint16 position = int32(round(_position.getValue(axis) * scale));
		_kitVariables[112 + 2 * axis] = position;
		_kitVariables[113 + 2 * axis] = position >> 8;
	}
	_kitVariables[118] = (int(round(_pitch / 5)) % 72 + 72) % 72;
	_kitVariables[119] = (int(round((90 - _yaw) / 5)) % 72 + 72) % 72;
	_kitVariables[120] = (int(round(_roll / 5)) % 72 + 72) % 72;
	_kitVariables[121] = _currentKey;
	_kitVariables[124] = _currentArea->getAreaID();
	_changedVariables = 0;
}

void Kit8Engine::writeSystemVariables() {
	float scale = 2 * _currentArea->getScale();
	for (int axis = 0; axis < 3; axis++) {
		if (_changedVariables & (3 << (2 * axis)))
			_position.setValue(axis, (_kitVariables[112 + 2 * axis] | (_kitVariables[113 + 2 * axis] << 8)) / scale);
	}
	if (_changedVariables & (1 << 6)) {
		_pitch = 5 * (_kitVariables[118] % 72);
		if (_pitch > 180)
			_pitch -= 360;
	}
	if (_changedVariables & (1 << 7))
		_yaw = 90 - 5 * _kitVariables[119];
	if (_changedVariables & (1 << 8))
		_roll = 5 * _kitVariables[120];
	if (_changedVariables & 0x1c0)
		updateCamera();
	if (_changedVariables & 0x3f)
		_lastPosition = _position;
	_changedVariables = 0;
}

void Kit8Engine::updateTimeVariables() {
	uint32 now = g_system->getMillis();
	// The Spectrum beeper disables interrupts until the effect ends.
	if (isSpectrum() && isPlayingSound()) {
		_lastTime = now;
		return;
	}
	uint32 elapsed = (now - _lastTime) / 20;
	_lastTime += 20 * elapsed;
	uint16 counter = (_kitVariables[122] | (_kitVariables[123] << 8)) + elapsed;
	_kitVariables[122] = counter;
	_kitVariables[123] = counter >> 8;
	_timerTicks += elapsed;
	uint32 interval = MAX<uint32>(1, _timerInterval);
	if (_timerTicks >= interval) {
		_pendingTimer = true;
		_timerTicks %= interval;
	}
}

void Kit8Engine::startScript(ScriptState &script, const FCLInstructionVector &code) {
	if (script.stack.size() >= 64)
		error("8-bit 3D Construction Kit procedure stack overflow");
	script.stack.push_back(FCLExecutionFrame(&code));
	script.executing = true;
	script.predicate.operation = Token::UNKNOWN;
}

void Kit8Engine::beginScriptFrame() {
	_scriptFrameActive = true;
	readSystemVariables();
	_timerTriggered = _pendingTimer;
	_pendingTimer = false;
	_scriptSurface.fillRect(_viewArea, 255);
	_activeConditions = &_globalConditions;
	_conditionIndex = 0;
	_globalPhase = true;
	if (_initialScriptPending) {
		_initialScriptPending = false;
		for (const auto &condition : _globalConditions) {
			if (condition.id == 1) {
				startScript(_script, condition.condition);
				break;
			}
		}
	}
}

void Kit8Engine::updateScripts() {
	if (isSpectrum() && isPlayingSound())
		return;
	_fallen |= _hasFallen;
	_crushed |= _playerWasCrushed;
	_hasFallen = _playerWasCrushed = false;
	_avoidRenderingFrames = 0;
	if ((!_scriptFrameActive || _redrawPending) && !isFrameReady())
		return;
	_redrawPending = false;
	if (_delayUntil && int32(_delayUntil - g_system->getMillis()) > 0)
		return;
	_delayUntil = 0;
	if (!_scriptFrameActive)
		beginScriptFrame();
	uint budget = 4096;
	while (budget) {
		if (_script.stack.empty()) {
			// The CPC runner enables text after its first initialization condition.
			_textOutputEnabled = true;
			if (_conditionIndex == _activeConditions->size()) {
				if (!_globalPhase)
					break;
				_globalPhase = false;
				_activeConditions = &_areaData[_currentArea->getAreaID()].conditions;
				_conditionIndex = 0;
				continue;
			}
			const ConditionData &condition = (*_activeConditions)[_conditionIndex++];
			if (_globalPhase && condition.id == 1)
				continue;
			startScript(_script, condition.condition);
		}
		if (executeCode(_script, budget) != kFCLFinished)
			return;
	}
	writeSystemVariables();
	if (!_script.stack.empty() || _globalPhase || _conditionIndex < _activeConditions->size())
		return;
	updateInstruments();
	_scriptFrameActive = false;
	// Approximate 8-bit rendering time using Freescape's movement cadence.
	_nextFrameTime = _lastTime + kFrameDuration;
	_soundSyncReady = true;
	_shotObject = _hitObject = _activatedObject = 0;
}

FCLExecutionResult Kit8Engine::executeCode(ScriptState &script, uint &budget) {
	while (!script.stack.empty() && budget) {
		FCLExecutionFrame &frame = script.stack.back();
		if (frame.ip == frame.code->size()) {
			script.stack.pop_back();
			continue;
		}
		const FCLInstruction &instruction = (*frame.code)[frame.ip++];
		Token::Type op = instruction.getType();
		budget--;
		// Condition statements also execute while a THEN/ELSE branch is skipped.
		switch (op) {
		case Token::ENDOFFILE:
			script.stack.pop_back();
			continue;
		case Token::CONDITIONAL:
			executeConditional(instruction, script);
			continue;
		case Token::VISQ:
		case Token::IFEQ:
		case Token::IFGT:
		case Token::IFLT:
			executeComparison(instruction, script);
			continue;
		case Token::AND:
		case Token::OR:
			script.predicate.combine(op);
			continue;
		case Token::THEN:
			script.executing = script.predicate.value;
			continue;
		case Token::ELSE:
			script.executing = !script.executing;
			continue;
		case Token::ENDIF:
			script.executing = true;
			continue;
		default:
			break;
		}
		if (!script.executing)
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
			executeArithmetic(instruction, script);
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
			executeCall(instruction, script);
			break;
		case Token::END:
			script.stack.pop_back();
			break;
		case Token::ENDGAME:
			_gameStateControl = kFreescapeGameStateRestart;
			script.stack.clear();
			return kFCLPaused;
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
			if (isSpectrum() && isPlayingSound())
				return kFCLPaused;
			break;
		case Token::DELAY:
			_delayUntil = g_system->getMillis() + 20 * (instruction._source ? instruction._source : 256);
			return kFCLPaused;
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
			_nextFrameTime = _lastTime + kFrameDuration;
			_redrawPending = true;
			_soundSyncReady = true;
			return kFCLPaused;
		default:
			error("Unsupported 8-bit 3D Construction Kit instruction %u", op);
		}
	}
	return script.stack.empty() ? kFCLFinished : kFCLPaused;
}

void Kit8Engine::executeArithmetic(const FCLInstruction &instruction, ScriptState &script) {
	byte index = instruction._source & 127;
	byte operand = instruction._destinationType == Token::VARIABLE ? _kitVariables[instruction._destination & 127] : instruction._destination;
	int value = _kitVariables[index];
	int result = value;
	bool store = true;
	switch (instruction.getType()) {
	case Token::SETVAR: result = operand; break;
	case Token::ADDVAR: result += operand; break;
	case Token::ADCV: result += operand + (script.carry ? 1 : 0); break;
	case Token::SUBVAR: result -= operand; break;
	case Token::SBCV: result -= operand + (script.carry ? 1 : 0); break;
	case Token::ANDV: result &= operand; break;
	case Token::ORV: result |= operand; break;
	case Token::XORV: result ^= operand; break;
	case Token::TESTV:
		result &= operand;
		store = false;
		break;
	case Token::CMPV:
		result = operand - value;
		store = false;
		break;
	default:
		break;
	}
	if (store) {
		_kitVariables[index] = result;
		if (index >= 112 && index <= 120)
			_changedVariables |= 1 << (index - 112);
	}
	script.carry = result < 0 || result > 255;
	if (instruction.getType() == Token::SETVAR) {
		// SETV retains the CPC operand decoder's zero flag.
		script.predicate.set(instruction._destinationType != Token::VARIABLE);
	} else
		script.predicate.set(byte(result) == 0);
}

void Kit8Engine::getObjectReference(const FCLInstruction &instruction, uint16 &area, uint16 &id) const {
	area = _currentArea->getAreaID();
	id = instruction._source;
	if (instruction._destinationType != Token::UNKNOWN) {
		area = instruction._source;
		id = instruction._destination;
	}
}

Object *Kit8Engine::scriptObject(uint16 area, uint16 id) {
	if (!_areaMap.contains(area))
		return nullptr;
	Object *object = _areaMap[area]->objectWithID(id);
	if (!object)
		object = _areaMap[area]->entranceWithID(id);
	return object;
}

void Kit8Engine::executeComparison(const FCLInstruction &instruction, ScriptState &script) {
	if (instruction.getType() == Token::VISQ) {
		uint16 area, id;
		getObjectReference(instruction, area, id);
		Object *object = scriptObject(area, id);
		script.carry = false;
		script.predicate.set(!object || (!object->isInvisible() && !object->isDestroyed()));
	} else if (instruction.getType() == Token::IFGT)
		script.predicate.set(!script.predicate.value && script.carry);
	else if (instruction.getType() == Token::IFLT)
		script.predicate.set(!script.predicate.value && !script.carry);
	else
		script.predicate.set(script.predicate.value);
}

void Kit8Engine::executeConditional(const FCLInstruction &instruction, ScriptState &script) {
	int area = instruction._additionalType == Token::UNKNOWN ? _currentArea->getAreaID() : instruction._additional;
	int id = instruction._destination;
	int value = 0;
	switch (instruction._source) {
	case kConditionalShot: value = _shotObject; break;
	case kConditionalCollided: value = _hitObject; break;
	case kConditionalActivated: value = _activatedObject; break;
	case kConditionalTimeout:
		script.carry = false;
		script.predicate.set(_timerTriggered);
		return;
	case kConditionalFallen:
		script.predicate.set(_fallen);
		_fallen = false;
		return;
	case kConditionalCrushed:
		script.predicate.set(_crushed);
		_crushed = false;
		return;
	case kConditionalSensed: {
		Object *object = scriptObject(area, id);
		script.carry = false;
		script.predicate.set(!object || (object->getType() == kSensorType && static_cast<Sensor *>(object)->isShooting()));
		return;
	}
	default:
		break;
	}
	if (area != _currentArea->getAreaID()) {
		value = _currentArea->getAreaID();
		id = area;
	}
	script.carry = value < id;
	script.predicate.set(value == id);
}

void Kit8Engine::executeObjectStatus(const FCLInstruction &instruction) {
	uint16 area, id;
	getObjectReference(instruction, area, id);
	Object *object = scriptObject(area, id);
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
	uint16 area = instruction._sourceType == Token::UNKNOWN ? _currentArea->getAreaID() : instruction._source;
	writeSystemVariables();
	gotoArea(area, instruction._destination);
}

void Kit8Engine::executeMode(const FCLInstruction &instruction) {
	writeSystemVariables();
	setMovementMode(instruction._source);
	readSystemVariables();
}

void Kit8Engine::executeCall(const FCLInstruction &instruction, ScriptState &script) {
	for (const auto &procedure : _procedures) {
		if (procedure.id == instruction._source) {
			startScript(script, procedure.condition);
			return;
		}
	}
}

void Kit8Engine::executeColour(const FCLInstruction &instruction) {
	if (isC64() && instruction._source >= 4)
		return;
	if (isSpectrum() && instruction._source >= 3) {
		// The Spectrum runner routes selectors 3 and above to the hardware border.
		_palette[3] = instruction._destination & 7;
		return;
	}
	uint index = instruction._source & 3;
	_palette[index] = isSpectrum() ? instruction._destination & (index == 2 ? 1 : 7) :
		isC64() ? instruction._destination & 15 : MIN<int>(26, instruction._destination);
	applyPalette();
}

void Kit8Engine::executeSound(const FCLInstruction &instruction) {
	playSound(instruction._source, instruction.getType() == Token::SYNCSND);
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
