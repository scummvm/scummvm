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

#include "common/algorithm.h"
#include "math/utils.h"

#include "freescape/games/3dck/3dck.h"

namespace Freescape {

void KitEngine::resetScripts() {
	stopAllSounds();
	_pendingSound = -1;
	// V255 survives ENDGAME, as in the original runner.
	memset(_kitVariables, 0, 255 * sizeof(_kitVariables[0]));
	_changedVariables = 0;
	_kitVariables[20] = 7;
	_scriptTicks = _timerTicks = 0;
	_lastScriptTick = _ticks;
	_scriptFrameActive = _scriptDelayed = false;
	_initialScriptPending = _initialCondition != 0;
	_pendingInteractions = _shootCooldown = _activateCooldown = 0;
	_scriptQueue.clear();
	_suspendedScripts.clear();
	_scriptSurface.fillRect(_fullscreenViewArea, 0);
	for (auto &condition : _globalConditions) {
		condition.script = ScriptState();
		condition.script.source = &condition.condition;
	}
	for (auto &entry : _areaData) {
		AreaData &area = entry._value;
		for (auto &condition : area.conditions) {
			condition.script = ScriptState();
			condition.script.area = entry._key;
			condition.script.source = &condition.condition;
		}
		for (auto &item : area.objects) {
			ObjectData &object = item._value;
			object.script = ScriptState();
			object.script.object = &object;
			object.script.area = entry._key;
			object.script.source = &object.condition;
			object.animatedObjects.clear();
			object.animator = 0;
			object.flags &= 0x84;
			if ((object.flags & 4) || object.type == 16)
				object.flags |= 2;
			object.origin = object.initialOrigin;
			Object *geometry = _areaMap[entry._key]->objectWithID(object.id);
			if (geometry && geometry->isGeometric()) {
				float scale = entry._key == 255 ? 1 : _areaMap[entry._key]->getScale();
				static_cast<GeometricObject *>(geometry)->offsetOrigin(object.origin / scale);
			}
		}
		_areaMap[entry._key]->getSortedObjects().clear();
	}
}

void KitEngine::startScript(ScriptState &script) {
	script.code = script.source;
	script.ip = script.restart = 0;
	script.resumeTick = 0;
	script.loops.clear();
	script.predicate = FCLPredicateState(true);
	script.events = script.object ? script.object->flags & 0x38 : 0;
	if (script.object)
		script.object->flags &= ~0x38;
	script.running = true;
}

void KitEngine::updateTimeVariables() {
	uint32 elapsed = _ticks - _lastScriptTick;
	_lastScriptTick = _ticks;
	_scriptTicks += elapsed;
	_timerTicks += elapsed;
	_kitVariables[19] += elapsed;
}

void KitEngine::readSystemVariables() {
	float scale = _currentArea->getScale();
	for (int axis = 0; axis < 3; axis++)
		_kitVariables[axis] = int32(_position.getValue(axis) * scale);
	_kitVariables[3] = (int(_pitch) % 360 + 360) % 360;
	_kitVariables[4] = (int(90 - _yaw) % 360 + 360) % 360;
	_kitVariables[5] = (int(_roll) % 360 + 360) % 360;
	_kitVariables[7] = int32(_playerHeight * scale);
	_kitVariables[8] = _currentArea->getAreaID();
}

void KitEngine::writeSystemVariables() {
	float scale = _currentArea->getScale();
	for (int axis = 0; axis < 3; axis++) {
		if (_changedVariables & (1 << axis)) {
			_position.setValue(axis, int16(_kitVariables[axis]) / scale);
			_lastPosition = _position;
		}
	}
	if (_changedVariables & (1 << 3))
		_pitch = int16(_kitVariables[3]) / 5 * 5;
	if (_changedVariables & (1 << 4))
		_yaw = 90 - int16(_kitVariables[4]) / 5 * 5;
	if (_changedVariables & (1 << 5))
		_roll = int16(_kitVariables[5]) / 5 * 5;
	if ((_changedVariables & (1 << 7)) && int16(_kitVariables[7]) > 0)
		_playerHeight = int16(_kitVariables[7]) / scale;
	if (_changedVariables & (1 << 6))
		_flyMode = _kitVariables[6] != 0 || _noClipMode;
	if (_changedVariables & 0x38)
		updateCamera();
	_changedVariables = 0;
}

void KitEngine::setScriptVariable(byte index, uint32 value) {
	_kitVariables[index] = value;
	if (index < 8)
		_changedVariables |= 1 << index;
}

void KitEngine::beginScriptFrame() {
	readSystemVariables();
	_timerTriggered = _timerTicks >= _timerInterval;
	if (_timerTriggered)
		_timerTicks = 0;
	_scriptQueue.clear();
	_scriptQueueIndex = 0;
	_scriptFrameActive = true;
	if (_initialScriptPending) {
		_scriptQueue.push_back(ScriptEntry(&_globalConditions[_initialCondition - 1].script, false));
		_initialScriptPending = false;
	}
	for (auto *script : _suspendedScripts) {
		if (script->object) {
			script->events = script->object->flags & 0x38;
			script->object->flags &= ~0x38;
		}
		_scriptQueue.push_back(ScriptEntry(script, true));
	}
	_suspendedScripts.clear();

	AreaData &area = _areaData[_currentArea->getAreaID()];
	for (uint16 id : area.objectOrder) {
		ObjectData &object = area.objects[id];
		if (object.type != 16 && !object.condition.empty())
			_scriptQueue.push_back(ScriptEntry(&object.script, false));
	}
	for (auto &condition : area.conditions)
		_scriptQueue.push_back(ScriptEntry(&condition.script, false));
	for (uint i = 0; i < _globalConditions.size(); i++) {
		if (i + 1 != _initialCondition)
			_scriptQueue.push_back(ScriptEntry(&_globalConditions[i].script, false));
	}
	for (uint16 id : area.objectOrder) {
		ObjectData &object = area.objects[id];
		if (object.type == 16)
			_scriptQueue.push_back(ScriptEntry(&object.script, false));
	}
}

void KitEngine::updateScripts() {
	if (_scriptDelayed) {
		if (int32(_scriptTicks - _delayUntil) < 0)
			return;
		_scriptDelayed = false;
	}
	if (!_scriptFrameActive) {
		if (_playerWasCrushed) {
			_kitVariables[12]++;
			_playerWasCrushed = false;
			_avoidRenderingFrames = 0;
		}
		beginScriptFrame();
	}

	uint budget = 4096;
	uint16 areaID = _currentArea->getAreaID();
	while (_scriptQueueIndex < _scriptQueue.size()) {
		ScriptEntry &entry = _scriptQueue[_scriptQueueIndex];
		ScriptState &script = *entry.script;
		ObjectData *object = script.object;
		bool animator = object && object->type == 16;
		if (script.area && script.area != areaID) {
			if (!animator)
				script.running = false;
			_scriptQueueIndex++;
			continue;
		}
		if (!entry.resume) {
			if ((animator && (object->flags & 2)) || (!animator && script.running) ||
					(object && !animator && !(object->flags & 0x38))) {
				_scriptQueueIndex++;
				continue;
			}
			if (animator && script.running && int32(_scriptTicks - script.resumeTick) < 0) {
				_scriptQueueIndex++;
				continue;
			}
			if (!script.running)
				startScript(script);
			entry.resume = true;
		}
		FCLExecutionResult result = executeCode(script, budget);
		if (result == kFCLPaused) {
			writeSystemVariables();
			return;
		}
		if (result == kFCLFinished) {
			script.running = false;
			if (animator)
				object->flags |= 2;
		} else if (animator) {
			script.resumeTick = _scriptTicks + kFCLRedrawTicks;
		} else {
			_suspendedScripts.push_back(&script);
		}
		_scriptQueueIndex++;
		if (_gameStateControl == kFreescapeGameStateRestart)
			break;
		if (areaID != _currentArea->getAreaID()) {
			for (uint i = _scriptQueueIndex; i < _scriptQueue.size(); i++) {
				ScriptEntry &pending = _scriptQueue[i];
				if (!pending.resume || !pending.script->running)
					continue;
				if (!pending.script->area)
					_suspendedScripts.push_back(pending.script);
				else if (!pending.script->object || pending.script->object->type != 16)
					pending.script->running = false;
			}
			break;
		}
	}
	writeSystemVariables();
	_scriptFrameActive = false;
}

int32 KitEngine::getVariableOrConstant(int32 operand, Token::Type type) const {
	return type == Token::VARIABLE ? int32(_kitVariables[operand]) : operand;
}

KitEngine::ObjectData *KitEngine::scriptObject(uint16 area, uint16 id) {
	if (!area)
		area = _currentArea->getAreaID();
	auto found = _areaData.find(area);
	if (found == _areaData.end())
		return nullptr;
	auto object = found->_value.objects.find(id);
	return object == found->_value.objects.end() ? nullptr : &object->_value;
}

void KitEngine::collectObjects(uint16 area, uint16 id, Common::Array<uint16> &objects) {
	Common::Array<uint16> pending, visited;
	pending.push_back(id);
	while (!pending.empty()) {
		id = pending.back();
		pending.pop_back();
		if (Common::find(visited.begin(), visited.end(), id) != visited.end())
			continue;
		visited.push_back(id);
		ObjectData *object = scriptObject(area, id);
		if (!object)
			continue;
		if (object->type == kGroupType) {
			for (int i = int(object->members.size()) - 1; i >= 0; i--)
				pending.push_back(object->members[i]);
		} else {
			objects.push_back(id);
		}
	}
}

void KitEngine::setObjectStatus(uint16 area, uint16 id, Token::Type operation) {
	Common::Array<uint16> objects;
	collectObjects(area, id, objects);
	for (uint16 member : objects) {
		ObjectData &object = *scriptObject(area, member);
		switch (operation) {
		case Token::INVIS: object.flags |= 2; break;
		case Token::VIS: object.flags &= ~2; break;
		case Token::TOGVIS: object.flags ^= 2; break;
		case Token::DESTROY: object.flags |= 3; break;
		default: break;
		}
		Object *geometry = _areaMap[area]->objectWithID(member);
		if (!geometry)
			continue;
		if (object.flags & 1)
			geometry->destroy();
		if (object.flags & 2)
			geometry->makeInvisible();
		else
			geometry->makeVisible();
		_areaMap[area]->getSortedObjects().clear();
	}
}

FCLExecutionResult KitEngine::executeCode(ScriptState &script, uint &budget) {
	bool animator = script.object && script.object->type == 16;
	while (script.ip < script.code->size()) {
		if (!budget)
			return kFCLPaused;
		budget--;
		uint32 ip = script.ip++;
		const FCLInstruction &instruction = (*script.code)[ip];
		switch (instruction.getType()) {
		case Token::NOP:
		case Token::ENDIF:
			break;
		case Token::IF:
			script.predicate.value = true;
			script.predicate.operation = Token::UNKNOWN;
			break;
		case Token::AND:
		case Token::OR:
			script.predicate.combine(instruction.getType());
			break;
		case Token::THEN:
		case Token::ELSE:
			executeIfThenElse(instruction, script);
			break;
		case Token::CONDITIONAL:
			executeConditional(instruction, script);
			break;
		case Token::SETVAR:
			executeSetVariable(instruction, script);
			break;
		case Token::ADDVAR:
			executeIncrementVariable(instruction, script);
			break;
		case Token::SUBVAR:
			executeDecrementVariable(instruction, script);
			break;
		case Token::ANDV:
			executeAndVariable(instruction, script);
			break;
		case Token::ORV:
			executeOrVariable(instruction, script);
			break;
		case Token::NOTV:
			executeNotVariable(instruction, script);
			break;
		case Token::VAREQ:
		case Token::VARGT:
		case Token::VARLT:
			executeVariableComparison(instruction, script);
			break;
		case Token::INVIS:
		case Token::VIS:
		case Token::TOGVIS:
		case Token::DESTROY:
			executeObjectStatus(instruction);
			break;
		case Token::INVISQ:
		case Token::VISQ:
		case Token::DESTROYEDQ:
			script.predicate.set(checkObjectStatus(instruction));
			break;
		case Token::GETXPOS:
		case Token::GETYPOS:
		case Token::GETZPOS:
			executeGetPosition(instruction);
			break;
		case Token::EXECUTE:
			if (!executeCall(instruction, script))
				return kFCLFinished;
			break;
		case Token::GOTO:
			if (!executeGoto(instruction))
				return kFCLFinished;
			break;
		case Token::MODE:
			executeMode(instruction);
			break;
		case Token::ENDGAME:
			_gameStateControl = kFreescapeGameStateRestart;
			return kFCLFinished;
		case Token::END:
			return animator ? kFCLYielded : kFCLFinished;
		case Token::WAIT:
			return kFCLYielded;
		case Token::DELAY:
			if (executeDelay(instruction))
				return kFCLPaused;
			break;
		case Token::REDRAW:
			_scriptSurface.fillRect(_viewArea, 0);
			playPendingSound();
			return animator ? kFCLYielded : kFCLPaused;
		case Token::LOOP:
			executeLoop(instruction, script);
			break;
		case Token::AGAIN:
			if (!executeAgain(script, ip))
				return kFCLFinished;
			break;
		case Token::STARTANIM:
			executeStartAnim(instruction);
			break;
		case Token::STOPANIM:
			executeStopAnim(instruction);
			if (animator)
				return kFCLYielded;
			break;
		case Token::TRIGANIM:
			executeTriggerAnim(instruction);
			break;
		case Token::START:
			if (animator)
				script.restart = script.ip;
			break;
		case Token::RESTART:
			if (animator)
				script.ip = script.restart;
			break;
		case Token::INCLUDE:
			if (animator)
				executeInclude(instruction, script);
			break;
		case Token::REMOVE:
			if (animator)
				executeRemove(instruction, script);
			break;
		case Token::WAITTRIG:
			if (animator && executeWaitTrigger(script, ip))
				return kFCLYielded;
			break;
		case Token::MOVE:
		case Token::MOVETO:
			if (animator) {
				executeMove(instruction, script);
				return kFCLYielded;
			}
			break;
		case Token::PRINT:
			printMessage(getVariableOrConstant(instruction._source, instruction._sourceType), instruction._text);
			break;
		case Token::UPDATEI:
			updateIndicators();
			break;
		case Token::SOUND:
		case Token::SYNCSND:
			executeSound(instruction);
			break;
		default:
			error("Unhandled 16-bit FCL instruction %d at ip: %u", instruction.getType(), ip);
		}
	}
	return kFCLFinished;
}

void KitEngine::executeIfThenElse(const FCLInstruction &instruction, ScriptState &script) {
	if (instruction.getType() == Token::THEN && script.predicate.value)
		return;
	const FCLInstructionVector &code = *script.code;
	int depth = 0;
	while (script.ip < code.size()) {
		Token::Type next = code[script.ip++].getType();
		if (next == Token::IF) {
			depth++;
		} else if (next == Token::ENDIF) {
			if (!depth)
				break;
			depth--;
		} else if (next == Token::ELSE && !depth && instruction.getType() == Token::THEN) {
			break;
		}
	}
}

void KitEngine::executeConditional(const FCLInstruction &instruction, ScriptState &script) {
	// RUNVGA retains these flags until the object's execution yields or ends.
	script.predicate.set(checkConditional(instruction,
		script.events & 16, script.events & 32, _timerTriggered, script.events & 8));
}

void KitEngine::setVariableResult(const FCLInstruction &instruction, ScriptState &script, uint32 value) {
	if (instruction._sourceType == Token::VARIABLE)
		setScriptVariable(instruction._source, value);
	script.predicate.set(value != 0);
}

void KitEngine::executeSetVariable(const FCLInstruction &instruction, ScriptState &script) {
	uint32 value = getVariableOrConstant(instruction._destination, instruction._destinationType);
	setVariableResult(instruction, script, value);
}

void KitEngine::executeIncrementVariable(const FCLInstruction &instruction, ScriptState &script) {
	uint32 source = getVariableOrConstant(instruction._source, instruction._sourceType);
	uint32 destination = getVariableOrConstant(instruction._destination, instruction._destinationType);
	setVariableResult(instruction, script, source + destination);
}

void KitEngine::executeDecrementVariable(const FCLInstruction &instruction, ScriptState &script) {
	uint32 source = getVariableOrConstant(instruction._source, instruction._sourceType);
	uint32 destination = getVariableOrConstant(instruction._destination, instruction._destinationType);
	setVariableResult(instruction, script, source - destination);
}

void KitEngine::executeAndVariable(const FCLInstruction &instruction, ScriptState &script) {
	uint32 source = getVariableOrConstant(instruction._source, instruction._sourceType);
	uint32 destination = getVariableOrConstant(instruction._destination, instruction._destinationType);
	setVariableResult(instruction, script, source & destination);
}

void KitEngine::executeOrVariable(const FCLInstruction &instruction, ScriptState &script) {
	uint32 source = getVariableOrConstant(instruction._source, instruction._sourceType);
	uint32 destination = getVariableOrConstant(instruction._destination, instruction._destinationType);
	setVariableResult(instruction, script, source | destination);
}

void KitEngine::executeNotVariable(const FCLInstruction &instruction, ScriptState &script) {
	uint32 value = getVariableOrConstant(instruction._source, instruction._sourceType);
	setVariableResult(instruction, script, ~value);
}

void KitEngine::executeVariableComparison(const FCLInstruction &instruction, ScriptState &script) {
	int32 source = getVariableOrConstant(instruction._source, instruction._sourceType);
	int32 destination = getVariableOrConstant(instruction._destination, instruction._destinationType);
	switch (instruction.getType()) {
	case Token::VAREQ:
		script.predicate.set(source == destination);
		break;
	case Token::VARGT:
		script.predicate.set(source > destination);
		break;
	case Token::VARLT:
		script.predicate.set(source < destination);
		break;
	default:
		break;
	}
}

void KitEngine::getObjectReference(const FCLInstruction &instruction, uint16 &area, uint16 &id) const {
	int32 source = getVariableOrConstant(instruction._source, instruction._sourceType);
	area = _currentArea->getAreaID();
	id = source;
	if (instruction._destinationType != Token::UNKNOWN) {
		if (source)
			area = source;
		id = getVariableOrConstant(instruction._destination, instruction._destinationType);
	}
}

void KitEngine::executeObjectStatus(const FCLInstruction &instruction) {
	uint16 area, id;
	getObjectReference(instruction, area, id);
	setObjectStatus(area, id, instruction.getType());
}

bool KitEngine::checkObjectStatus(const FCLInstruction &instruction) {
	uint16 area, id;
	getObjectReference(instruction, area, id);
	Common::Array<uint16> objects;
	collectObjects(area, id, objects);
	byte mask = instruction.getType() == Token::DESTROYEDQ ? 1 : 2;
	bool expected = instruction.getType() != Token::VISQ;
	bool result = false;
	for (uint16 member : objects)
		result = ((scriptObject(area, member)->flags & mask) != 0) == expected;
	return result;
}

void KitEngine::executeGetPosition(const FCLInstruction &instruction) {
	uint16 area = getVariableOrConstant(instruction._additional, instruction._additionalType);
	uint16 id = getVariableOrConstant(instruction._destination, instruction._destinationType);
	ObjectData *object = scriptObject(area, id);
	if (object) {
		int axis = instruction.getType() - Token::GETXPOS;
		setScriptVariable(instruction._source & 0xff, int32(object->origin.getValue(axis)));
	}
}

bool KitEngine::executeCall(const FCLInstruction &instruction, ScriptState &script) {
	uint16 id = getVariableOrConstant(instruction._source, instruction._sourceType);
	ObjectData *object = scriptObject(0, id);
	if (!object || object->type == kGroupType)
		return false;
	// EXECUTE replaces the code, retaining the original object's event flags.
	script.code = &object->condition;
	script.ip = 0;
	script.loops.clear();
	return true;
}

bool KitEngine::executeGoto(const FCLInstruction &instruction) {
	uint16 area = _currentArea->getAreaID();
	if (instruction._sourceType != Token::UNKNOWN)
		area = getVariableOrConstant(instruction._source, instruction._sourceType);
	int32 entrance = getVariableOrConstant(instruction._destination, instruction._destinationType);
	if (!_areaMap.contains(area)) {
		warning("Invalid 3D Construction Kit GOTO (%d, %u)", entrance, area);
		return false;
	}
	writeSystemVariables();
	uint16 previous = _currentArea->getAreaID();
	gotoArea(area, entrance & 0x7fff);
	return previous == area;
}

void KitEngine::executeMode(const FCLInstruction &instruction) {
	int32 mode = getVariableOrConstant(instruction._source, instruction._sourceType);
	setScriptVariable(6, CLIP<int32>(mode, 1, 3) - 1);
}

bool KitEngine::executeDelay(const FCLInstruction &instruction) {
	uint16 delay = getVariableOrConstant(instruction._source, instruction._sourceType);
	if (!delay)
		return false;
	_delayUntil = _scriptTicks + delay;
	_scriptDelayed = true;
	return true;
}

void KitEngine::executeLoop(const FCLInstruction &instruction, ScriptState &script) {
	const FCLInstructionVector &code = *script.code;
	int depth = 0;
	uint32 end = script.ip;
	while (end < code.size()) {
		Token::Type next = code[end].getType();
		if (next == Token::LOOP) {
			depth++;
		} else if (next == Token::AGAIN) {
			if (!depth)
				break;
			depth--;
		}
		end++;
	}
	FCLKit16Loop &loop = script.loops[end];
	loop.start = script.ip;
	loop.remaining = getVariableOrConstant(instruction._source, instruction._sourceType);
}

bool KitEngine::executeAgain(ScriptState &script, uint32 ip) {
	auto loop = script.loops.find(ip);
	if (loop == script.loops.end())
		return false;
	if (--loop->_value.remaining != 0)
		script.ip = loop->_value.start;
	else
		script.loops.erase(loop);
	return true;
}

KitEngine::ObjectData *KitEngine::scriptAnimator(const FCLInstruction &instruction) {
	uint16 area, id;
	getObjectReference(instruction, area, id);
	ObjectData *object = scriptObject(area, id | 0x4000);
	return object && object->type == 16 ? object : nullptr;
}

void KitEngine::executeStartAnim(const FCLInstruction &instruction) {
	ObjectData *object = scriptAnimator(instruction);
	if (object)
		object->flags &= ~2;
}

void KitEngine::executeStopAnim(const FCLInstruction &instruction) {
	ObjectData *object = scriptAnimator(instruction);
	if (object)
		object->flags |= 2;
}

void KitEngine::executeTriggerAnim(const FCLInstruction &instruction) {
	ObjectData *object = scriptAnimator(instruction);
	if (object)
		object->flags |= 1;
}

void KitEngine::executeInclude(const FCLInstruction &instruction, ScriptState &script) {
	uint16 id = getVariableOrConstant(instruction._source, instruction._sourceType);
	Common::Array<uint16> objects;
	collectObjects(script.area, id, objects);
	for (uint16 member : objects) {
		ObjectData &object = *scriptObject(script.area, member);
		if (!(object.flags & 0x80) || object.animator)
			continue;
		object.animator = script.object->id;
		script.object->animatedObjects.push_back(member);
	}
}

void KitEngine::executeRemove(const FCLInstruction &instruction, ScriptState &script) {
	uint16 id = getVariableOrConstant(instruction._source, instruction._sourceType);
	Common::Array<uint16> objects;
	collectObjects(script.area, id, objects);
	for (uint16 member : objects) {
		ObjectData &object = *scriptObject(script.area, member);
		if (!(object.flags & 0x80) || object.animator != script.object->id)
			continue;
		auto &members = script.object->animatedObjects;
		for (uint i = 0; i < members.size(); i++) {
			if (members[i] == member) {
				members.remove_at(i);
				break;
			}
		}
		object.animator = 0;
	}
}

bool KitEngine::executeWaitTrigger(ScriptState &script, uint32 ip) {
	if (!(script.object->flags & 1)) {
		script.ip = ip;
		return true;
	}
	script.object->flags &= ~1;
	return false;
}

void KitEngine::executeMove(const FCLInstruction &instruction, ScriptState &script) {
	int16 x = getVariableOrConstant(instruction._source, instruction._sourceType);
	int16 y = getVariableOrConstant(instruction._destination, instruction._destinationType);
	int16 z = getVariableOrConstant(instruction._additional, instruction._additionalType);
	bool absolute = instruction.getType() == Token::MOVETO;
	script.predicate.set(moveAnimation(script, Math::Vector3d(x, y, z), absolute));
}

void KitEngine::executeSound(const FCLInstruction &instruction) {
	uint16 index = getVariableOrConstant(instruction._source, instruction._sourceType);
	if (instruction.getType() == Token::SYNCSND)
		_pendingSound = index == 0xffff ? -1 : index;
	else if (_sound)
		_sound->playSound(index, Sound::kTypeNormal);
}

bool KitEngine::executeObjectConditions(GeometricObject *obj, bool shot, bool collided, bool activated) {
	ObjectData *object = scriptObject(0, obj->getObjectID());
	if (object)
		object->flags |= (shot ? 16 : 0) | (collided ? 32 : 0) | (activated ? 8 : 0);
	return false;
}

bool KitEngine::moveAnimation(ScriptState &script, Math::Vector3d movement, bool absolute) {
	const auto &members = script.object->animatedObjects;
	if (members.empty())
		return true;
	Math::Vector3d minimum(8191, 8191, 8191), maximum;
	for (uint16 id : members) {
		ObjectData &object = *scriptObject(script.area, id);
		for (int axis = 0; axis < 3; axis++) {
			minimum.setValue(axis, MIN(minimum.getValue(axis), object.origin.getValue(axis)));
			maximum.setValue(axis, MAX(maximum.getValue(axis), object.origin.getValue(axis) + object.size.getValue(axis)));
		}
	}
	if (absolute)
		movement -= minimum;
	bool unobstructed = true;
	for (int axis = 0; axis < 3; axis++) {
		float delta = movement.getValue(axis);
		float clipped = CLIP(delta, -minimum.getValue(axis), 8191 - maximum.getValue(axis));
		if (delta != clipped)
			unobstructed = false;
		movement.setValue(axis, clipped);
	}
	// The runner collides the combined header bounds of all included objects.
	AreaData &area = _areaData[script.area];
	for (uint16 id : area.objectOrder) {
		ObjectData &object = area.objects[id];
		if ((id & 0xc000) || (object.flags & 3) || object.type == kGroupType ||
				object.animator == script.object->id)
			continue;
		bool overlaps = true;
		for (int axis = 0; axis < 3; axis++) {
			float delta = movement.getValue(axis);
			if (minimum.getValue(axis) + delta >= object.origin.getValue(axis) + object.size.getValue(axis) ||
					maximum.getValue(axis) + delta <= object.origin.getValue(axis))
				overlaps = false;
		}
		if (!overlaps)
			continue;
		object.flags |= 32;
		unobstructed = false;
		for (int axis = 0; axis < 3; axis++) {
			float delta = movement.getValue(axis);
			if (delta >= 0) {
				float gap = object.origin.getValue(axis) - maximum.getValue(axis);
				if (gap >= 0 && gap < delta)
					movement.setValue(axis, gap);
			} else {
				float gap = minimum.getValue(axis) - object.origin.getValue(axis) - object.size.getValue(axis);
				if (gap >= 0 && gap < -delta)
					movement.setValue(axis, -gap);
			}
		}
	}
	float scale = script.area == 255 ? 1 : _areaMap[script.area]->getScale();
	for (uint16 id : members) {
		ObjectData &object = *scriptObject(script.area, id);
		object.origin += movement;
		Object *geometry = _areaMap[script.area]->objectWithID(id);
		if (geometry && geometry->isGeometric())
			static_cast<GeometricObject *>(geometry)->offsetOrigin(object.origin / scale);
	}
	_areaMap[script.area]->getSortedObjects().clear();
	return unobstructed;
}

} // namespace Freescape
