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
	// V255 survives ENDGAME, as in the original runner.
	memset(_kitVariables, 0, 255 * sizeof(_kitVariables[0]));
	_changedVariables = 0;
	_kitVariables[20] = 7;
	_scriptTicks = _timerTicks = 0;
	_lastScriptTick = _ticks;
	_scriptFrameActive = _scriptDelayed = false;
	_initialScriptPending = _initialCondition != 0;
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
	script.loops.clear();
	script.predicate = script.previousPredicate = true;
	script.booleanOp = Token::UNKNOWN;
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
		} else if (!animator) {
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

void KitEngine::setScriptPredicate(ScriptState &script, bool value) {
	if (script.booleanOp == Token::AND)
		value = script.previousPredicate && value;
	else if (script.booleanOp == Token::OR)
		value = script.previousPredicate || value;
	script.predicate = value;
	script.booleanOp = Token::UNKNOWN;
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
		const FCLInstructionVector &code = *script.code;
		uint32 ip = script.ip++;
		const FCLInstruction &instruction = code[ip];
		Token::Type op = instruction.getType();
		int32 source = getVariableOrConstant(instruction._source, instruction._sourceType);
		int32 destination = getVariableOrConstant(instruction._destination, instruction._destinationType);
		int32 additional = getVariableOrConstant(instruction._additional, instruction._additionalType);
		bool hasDestination = instruction._destinationType != Token::UNKNOWN;
		switch (op) {
		case Token::NOP: case Token::ENDIF:
			break;
		case Token::IF:
			script.predicate = true;
			script.booleanOp = Token::UNKNOWN;
			break;
		case Token::AND: case Token::OR:
			script.previousPredicate = script.predicate;
			script.booleanOp = op;
			break;
		case Token::THEN: case Token::ELSE:
			if (op == Token::ELSE || !script.predicate) {
				int depth = 0;
				while (script.ip < code.size()) {
					Token::Type next = code[script.ip++].getType();
					if (next == Token::IF)
						depth++;
					else if (next == Token::ENDIF) {
						if (!depth)
							break;
						depth--;
					} else if (next == Token::ELSE && !depth && op == Token::THEN) {
						break;
					}
				}
			}
			break;
		case Token::CONDITIONAL:
			// RUNVGA retains these flags until the object's execution yields or ends.
			setScriptPredicate(script, checkConditional(instruction,
				script.events & 16, script.events & 32, _timerTriggered, script.events & 8));
			break;
		case Token::SETVAR: case Token::ADDVAR: case Token::SUBVAR: case Token::ANDV: case Token::ORV: case Token::NOTV: {
			uint32 value = destination;
			switch (op) {
			case Token::ADDVAR: value += uint32(source); break;
			case Token::SUBVAR: value = uint32(source) - value; break;
			case Token::ANDV: value &= uint32(source); break;
			case Token::ORV: value |= uint32(source); break;
			case Token::NOTV: value = ~uint32(source); break;
			default: break;
			}
			if (instruction._sourceType == Token::VARIABLE)
				setScriptVariable(instruction._source, value);
			setScriptPredicate(script, value != 0);
			break;
		}
		case Token::VAREQ: case Token::VARGT: case Token::VARLT:
			setScriptPredicate(script, op == Token::VAREQ ? source == destination : op == Token::VARGT ? source > destination : source < destination);
			break;
		case Token::INVIS: case Token::VIS: case Token::TOGVIS: case Token::DESTROY:
		case Token::INVISQ: case Token::VISQ: case Token::DESTROYEDQ: {
			uint16 area = hasDestination && source ? source : _currentArea->getAreaID();
			uint16 object = hasDestination ? destination : source;
			if (op == Token::INVIS || op == Token::VIS || op == Token::TOGVIS || op == Token::DESTROY)
				setObjectStatus(area, object, op);
			else {
				Common::Array<uint16> objects;
				collectObjects(area, object, objects);
				bool result = false;
				for (uint16 member : objects) {
					byte flags = scriptObject(area, member)->flags;
					result = op == Token::DESTROYEDQ ? (flags & 1) != 0 : op == Token::INVISQ ? (flags & 2) != 0 : !(flags & 2);
				}
				setScriptPredicate(script, result);
			}
			break;
		}
		case Token::GETXPOS: case Token::GETYPOS: case Token::GETZPOS: {
			ObjectData *object = scriptObject(additional, destination);
			if (object)
				setScriptVariable(instruction._source & 0xff, int32(object->origin.getValue(op - Token::GETXPOS)));
			break;
		}
		case Token::EXECUTE: {
			ObjectData *object = scriptObject(0, source);
			if (!object || object->type == kGroupType)
				return kFCLFinished;
			// EXECUTE replaces the code, retaining the original object's event flags.
			script.code = &object->condition;
			script.ip = 0;
			script.loops.clear();
			break;
		}
		case Token::GOTO: {
			uint16 area = instruction._sourceType != Token::UNKNOWN ? source : _currentArea->getAreaID();
			if (!_areaMap.contains(area) || !_areaMap[area]->entranceWithID(destination & 0x7fff)) {
				warning("Invalid 3D Construction Kit GOTO (%d, %u)", destination, area);
				return kFCLFinished;
			}
			writeSystemVariables();
			uint16 previous = _currentArea->getAreaID();
			gotoArea(area, destination & 0x7fff);
			if (previous != area)
				return kFCLFinished;
			break;
		}
		case Token::MODE:
			setScriptVariable(6, CLIP<int32>(source, 1, 3) - 1);
			break;
		case Token::ENDGAME:
			_gameStateControl = kFreescapeGameStateRestart;
			return kFCLFinished;
		case Token::END:
			return animator ? kFCLYielded : kFCLFinished;
		case Token::WAIT:
			return kFCLYielded;
		case Token::DELAY:
			if (uint16(source)) {
				_delayUntil = _scriptTicks + uint16(source);
				_scriptDelayed = true;
				return kFCLPaused;
			}
			break;
		case Token::REDRAW:
			_scriptSurface.fillRect(_viewArea, 0);
			return animator ? kFCLYielded : kFCLPaused;
		case Token::LOOP: {
			int depth = 0;
			uint32 end = script.ip;
			while (end < code.size()) {
				Token::Type next = code[end].getType();
				if (next == Token::LOOP)
					depth++;
				else if (next == Token::AGAIN) {
					if (!depth)
						break;
					depth--;
				}
				end++;
			}
			FCLLoop &loop = script.loops[end];
			loop.start = script.ip;
			loop.remaining = uint16(source);
			break;
		}
		case Token::AGAIN: {
			auto loop = script.loops.find(ip);
			if (loop == script.loops.end())
				return kFCLFinished;
			if (--loop->_value.remaining != 0) {
				script.ip = loop->_value.start;
			} else {
				script.loops.erase(loop);
			}
			break;
		}
		case Token::STARTANIM: case Token::STOPANIM: case Token::TRIGANIM: {
			ObjectData *object = scriptObject(hasDestination ? source : 0, uint16(hasDestination ? destination : source) | 0x4000);
			if (object && object->type == 16) {
				if (op == Token::STARTANIM)
					object->flags &= ~2;
				else if (op == Token::STOPANIM)
					object->flags |= 2;
				else
					object->flags |= 1;
			}
			if (animator && op == Token::STOPANIM)
				return kFCLYielded;
			break;
		}
		case Token::START:
			if (animator)
				script.restart = script.ip;
			break;
		case Token::RESTART:
			if (animator)
				script.ip = script.restart;
			break;
		case Token::INCLUDE: case Token::REMOVE:
			if (animator) {
				Common::Array<uint16> objects;
				collectObjects(script.area, source, objects);
				for (uint16 member : objects) {
					ObjectData &object = *scriptObject(script.area, member);
					if (!(object.flags & 0x80))
						continue;
					if (op == Token::INCLUDE && !object.animator) {
						object.animator = script.object->id;
						script.object->animatedObjects.push_back(member);
					} else if (op == Token::REMOVE && object.animator == script.object->id) {
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
			}
			break;
		case Token::WAITTRIG:
			if (animator) {
				if (!(script.object->flags & 1)) {
					script.ip = ip;
					return kFCLYielded;
				}
				script.object->flags &= ~1;
			}
			break;
		case Token::MOVE: case Token::MOVETO:
			if (animator) {
				setScriptPredicate(script, moveAnimation(script,
					Math::Vector3d(int16(source), int16(destination), int16(additional)), op == Token::MOVETO));
				return kFCLYielded;
			}
			break;
		case Token::PRINT:
			printMessage(source, instruction._text);
			break;
		case Token::UPDATEI:
			updateIndicators();
			break;
		case Token::SOUND: case Token::SYNCSND:
			if (!_soundWarning) {
				warning("3D Construction Kit sound playback is not implemented");
				_soundWarning = true;
			}
			break;
		default:
			error("Unhandled 16-bit FCL instruction %d at ip: %u", op, ip);
		}
	}
	return kFCLFinished;
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

void KitEngine::updatePlayerMovement(float deltaTime) {
	if (_scriptFrameActive)
		return;
	float height = _position.y();
	FreescapeEngine::updatePlayerMovement(deltaTime);
	if (_hasFallen) {
		_kitVariables[10] += MAX<int>(0, height - _position.y() - _maxFallingDistance);
		_hasFallen = false;
		_avoidRenderingFrames = 0;
	}
}

} // namespace Freescape
