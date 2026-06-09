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
#include "scriptexecutor.h"

#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "macs2/adlib.h"
#include "macs2/detection.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/debugtools.h"
#include <macs2/view1.h>

namespace Macs2 {
namespace Script {

static Common::String joinDebugStrings(const Common::StringArray &strings) {
	Common::String result;
	for (uint i = 0; i < strings.size(); ++i) {
		if (i != 0)
			result += " | ";
		result += strings[i];
	}
	return result;
}

#define ScriptNoEntry debugC(kDebugScript, "Unhandled case in script handling.");
#define STR_HELPER(x) #x

ScriptExecutor::ScriptExecutor() {
	constexpr int numVariables = 1000;
	_variables.resize(numVariables);
	for (int i = 0; i < numVariables; i++) {
		_variables[i].a = 0;
		_variables[i].b = 0;
	}
}

Common::String ScriptExecutor::identifyScriptOpcode(uint8 opcode, uint8 opcode2) {
	if (opcode == 0x5)
		return Common::String::format("(%.2x)", opcode);

	return Common::String::format("(%.2x %.2x)", opcode, opcode2);
}

Common::String ScriptExecutor::identifyHelperOpcode(uint8 opcode, uint16 value) {
	return Common::String::format("(%.2x %.4x)", opcode, value);
}

inline void ScriptExecutor::scriptSkipBlock() {
	_lastOpcodeTriggeredSkip = true;

	_isSkipping = true;
	if (_expectedEndLocation != _stream->pos()) {
		warning("Macs2::ScriptExecutor::scriptSkipBlock resyncing stream from %u to %u",
				(uint32)_expectedEndLocation, (uint32)_stream->pos());
		_expectedEndLocation = _stream->pos();
	}
	int skipDepth = 1;
	while ((skipDepth != 0) && (_stream->pos() < _stream->size())) {
		const byte opcode = readByte();
		if (_stream->pos() >= _stream->size()) {
			break;
		}
		const byte length = readByte();
		if (opcode >= 3) {
			if (opcode <= 6) {
				skipDepth++;
			}
		}
		if ((opcode == 8) && (skipDepth == 1)) {
			skipDepth = 0;
		}
		if (opcode == 7) {
			skipDepth--;
		}

		const int64 remainingBytes = _stream->size() - _stream->pos();
		if (length > remainingBytes) {
			_stream->seek(_stream->size(), SEEK_SET);
			break;
		}

		_stream->seek(length, SEEK_CUR);
	}

	// Fix up the expected location after skipping
	_expectedEndLocation = _stream->pos();
	_isSkipping = false;
}

void ScriptExecutor::scriptSkipAlternate() {
	_isSkipping = true;
	if (_expectedEndLocation != _stream->pos()) {
		warning("Macs2::ScriptExecutor::scriptSkipAlternate resyncing stream from %u to %u",
				(uint32)_expectedEndLocation, (uint32)_stream->pos());
		_expectedEndLocation = _stream->pos();
	}
	int skipDepth = 1;
	while ((skipDepth != 0) && (_stream->pos() < _stream->size())) {
		const byte opcode = readByte();
		if (_stream->pos() >= _stream->size()) {
			break;
		}
		const byte length = readByte();
		if (opcode >= 3) {
			if (opcode <= 6) {
				skipDepth++;
			}
		}
		if (opcode == 7) {
			skipDepth--;
		}

		const int64 remainingBytes = _stream->size() - _stream->pos();
		if (length > remainingBytes) {
			_stream->seek(_stream->size(), SEEK_SET);
			break;
		}

		_stream->seek(length, SEEK_CUR);
	}

	// Fix up the expected location after skipping
	_expectedEndLocation = _stream->pos();
	_isSkipping = false;
}

bool ScriptExecutor::skipToEndOfSkippableSection() {
	// Button 8 skip from handleInput (1008:e8bf):
	// Reads opcode+length pairs, advances stream by length bytes,
	// until opcode 0x1D is found or end of stream.
	while (_stream->pos() < _stream->size() - 1) {
		uint8 opcode = readByte();
		uint8 length = readByte();
		if (opcode == 0x1D) {
			_scriptSkippable = false;
			return true;
		}
		_stream->seek(length, SEEK_CUR);
	}
	return false;
}

void ScriptExecutor::scriptReadValuePair(uint16 &out1, uint16 &out2) {
	// scriptReadValue (1008:9f4d). Reads a typed value from the script stream.
	// Format: byte type + word index.
	// Type 0x00: literal (index is the value)
	// Type 0x01-0xFE: variable lookup (index into script variables)
	// Type 0xFF: special runtime value by index
	out1 = 0;
	out2 = 0;

	byte type = readByte();
	uint16 value = readUint16();

	if (type == 0x00) {
		out1 = value;
		out2 = 0;
		return;
	}

	if (type != 0xFF) {
		// Variable lookup
		if (value < 1 || value > 0x800) {
			warning("scriptReadValuePair: invalid variable index %u", value);
			return;
		}
		const ScriptVariable &var = _variables[value];
		out1 = var.a;
		out2 = var.b;
		return;
	}

	// Type 0xFF: special runtime values
	uint32 special = getSpecialValue(value);
	out1 = special & 0xFFFF;
	out2 = (special >> 16) & 0xFFFF;
}

uint32 ScriptExecutor::scriptReadValue32() {
	uint16 out1;
	uint16 out2;
	scriptReadValuePair(out1, out2);

	return (static_cast<uint32>(out2) << 16) + static_cast<uint32>(out1);
}

uint16 ScriptExecutor::scriptReadValue16() {
	uint16 out1;
	uint16 out2;
	scriptReadValuePair(out1, out2);
	return out1;
}

void ScriptExecutor::scriptSaveVariableHelper(uint32 value) {
	uint8 subOpcode = readByte();
	if (subOpcode == 0x00 || subOpcode == 0xFF) {
		warning("scriptSaveVariable: invalid sub-opcode 0x%02x (error 0x16)", subOpcode);
		return;
	}

	uint16 variableID = readUint16();
	setVariableValue(variableID, value);
}

void ScriptExecutor::scriptChangeAnimation() {
	// bp-2h
	uint32 id = scriptReadValue32();
	id -= 0x1000;
	// bp-4h
	uint16 bp4 = scriptReadValue16();
	if (id <= 0) {
		// mov	word ptr [1028h],8h
		return;
	}
	// l0037_B6F1:
	// TODO: Check if we try to access an invalid index

	// l0037_B715:
	// TODO: Load the data of the animation

	// l0037_B73C:
	// Use the function to extract some value
	// Subtracting an additional 1 since mine are indexed from 0 and not 1 like the game does
	id -= 1;

	BackgroundAnimationBlob &blob = _engine->_backgroundAnimationsBlobs[id];
	BackgroundAnimationBlob::getAnimFrameCount(blob._blob);
	// TODO: We should be doing some checking on the result value

	// TODO: Do some comparison with [bp-4h]
	BackgroundAnimationBlob::advanceAnimFrame(blob._blob, true, bp4 + 0x64);
}

uint16 ScriptExecutor::getAreaAtPoint(uint16 x, uint16 y) {
	// getAreaAtPoint (1008:101d). Reads the pathfinding map pixel and applies
	// the area override table at sceneData + value*5 + 0x4EA8.
	if (x >= 320 || y >= 200 || _engine->_pathfindingMap.w == 0) {
		return 0;
	}
	uint16 result = _engine->_pathfindingMap.getPixel(x, y);
	if (result > 199 && result < 0xFA) {
		uint16 overrideValue = _engine->getPathfindingOverride2(result);
		if (overrideValue > 199) {
			result = overrideValue;
		}
	}
	return result;
}

bool ScriptExecutor::loadIndexedResource(Common::Array<uint8> &outData, uint8 resourceIndex, uint16 /*objectTableOffset*/) {
	if (resourceIndex == 0) {
		warning("Ignoring resource load for zero resource index");
		return false;
	}

	const int64 oldPos = g_engine->_fileStream->pos();
	uint32 address = 0;

	if (_executingScriptObjectID == 0) {
		if (resourceIndex > _engine->_sceneResourceOffsets.size()) {
			warning("Ignoring resource load for missing scene resource %u", resourceIndex);
			return false;
		}
		address = _engine->_sceneResourceOffsets[resourceIndex - 1];
	} else {
		GameObject *object = GameObjects::getObjectByIndex(_executingScriptObjectID);
		if (object == nullptr || object->_dataOffset == 0) {
			warning("Ignoring resource load for missing object %u resource %u", _executingScriptObjectID, resourceIndex);
			return false;
		}
		// Binary reads from runtime+0x18D table (loaded during loadSceneObjects).
		// Table is 32 dword file offsets, indexed by (resourceIndex - 1).
		if (resourceIndex - 1 >= 32) {
			warning("Ignoring resource load for out-of-range index %u on object %u", resourceIndex, _executingScriptObjectID);
			return false;
		}
		address = object->_resourceOffsets[resourceIndex - 1];
	}

	if (address == 0) {
		warning("Ignoring resource load for empty resource %u", resourceIndex);
		g_engine->_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}

	g_engine->_fileStream->seek(address, SEEK_SET);
	const uint32 size = g_engine->_fileStream->readUint32LE();
	if (size == 0) {
		warning("Ignoring resource load for zero-sized resource %u", resourceIndex);
		g_engine->_fileStream->seek(oldPos, SEEK_SET);
		return false;
	}
	outData.resize(size);
	g_engine->_fileStream->read(outData.data(), size);
	g_engine->_fileStream->seek(oldPos, SEEK_SET);
	return !outData.empty();
}

bool ScriptExecutor::loadSoundResource(Common::Array<uint8> &outData, uint8 resourceIndex) {
	return loadIndexedResource(outData, resourceIndex);
}

bool ScriptExecutor::loadMusicResource(Common::Array<uint8> &outData, uint8 resourceIndex) {
	return loadIndexedResource(outData, resourceIndex);
}

void ScriptExecutor::scriptPrintString(bool alignRight) {
	// TODO: Labels above not handled yet
	// TODO: Lots of details not handled
	// l0037_A94E:

	uint16 x = scriptReadValue16();
	uint16 y = scriptReadValue16();
	// TODO: Several globals writes around this code
	uint16 bp2 = readUint16();
	uint16 bp4 = readUint16();

	// TODO: Implement naive string printing here, refine later

	Common::StringArray strings;
	if (_executingScriptObjectID == 0) {
		strings = g_engine->decodeStrings(Scenes::instance()._currentSceneStrings, bp2, bp4, Scenes::instance()._currentSceneIndex, 0);
	} else {
		Common::MemoryReadStream *s = GameObjects::readGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		strings = g_engine->decodeStrings(s, bp2, bp4, 0, _executingScriptObjectID);
	}

	if (alignRight) {
		x -= g_engine->measureStrings(strings) + 0x12;
	}

	// TODO: Look for good pattern for the view, this feels like it is not intended this way
	View1 *currentView = (View1 *)_engine->findView("View1");
	currentView->setStringBoxAt(strings, Common::Point(x, y));
}

void ScriptExecutor::beginBuffering() {
	_lastOpcodeTriggeredSkip = false;
	_debugBuffer.clear();
}

void ScriptExecutor::endBuffering(bool shouldMark) {
	(void)shouldMark;
	_lastOpcodeTriggeredSkip = false;
	_debugBuffer.clear();
}

void ScriptExecutor::setVariableValue(uint16 index, uint16 a, uint16 b) {
	_variables[index].a = a;
	_variables[index].b = b;
}

void ScriptExecutor::setVariableValue(uint16 index, uint32 value) {
	uint16 a = static_cast<uint16>(value >> 16);    // High 16 bits
	uint16 b = static_cast<uint16>(value & 0xFFFF); // Low 16 bits
	setVariableValue(index, b, a);
}

Common::Point ScriptExecutor::getCharPosition() {
	const GameObject *actor = GameObjects::instance().getObjectByIndex(Scenes::instance()._currentActorIndex);
	if (!actor)
		actor = GameObjects::getProtagonistObject();
	return actor ? actor->_position : Common::Point();
}

bool ScriptExecutor::isRelevantObject(const GameObject *obj) const {
	// Original logic (runScriptExecutor at 1008:e3e7):
	// An object is relevant if it has runtime data allocated (non-null pointer at object+0xa).
	// In ScummVM, this corresponds to having a non-empty Script array (the script data
	// lives in the runtime allocation at offset +0x187). The caller already checks Script.size(),
	// so we just need to confirm the object is initialized (has data offset set).
	return obj->_dataOffset != 0;
}

void ScriptExecutor::step() {
	bool shouldContinue = true;

	while (shouldContinue) {
		switch (_state) {
		case ExecutorState::Idle:
			// TODO: Check if there is a scheduled run
			return;
		case ExecutorState::Executing: {
			// Continue execution

			// Check if the currently executing script is at the end
			if (_stream->pos() == _stream->size()) {
				// Handle the next one potentially
				shouldContinue = loadNextScript();
			} else {
				// Let the current script continue
				ExecutionResult result = executeScript();
				if (result == ExecutionResult::WaitingForCallback) {
					// We need to change our state as well now
					_state = ExecutorState::WaitingForCallback;
					if (!_debugPaused) {
						// Original: save cursor mode, then set to Disabled 0x1A (hourglass)
						if (_mouseMode != MouseMode::Disabled) {
							_cursorModeBeforeWait = _mouseMode;
							_engine->setCursorMode(MouseMode::Disabled);
							View1 *v = (View1 *)_engine->findView("View1");
							if (v)
								v->updateCursor();
						}
					}
					return;
				}
			}
			break;
		}
		case ExecutorState::WaitingForCallback: {
			// TODO: Check if this can even occur i.e. if we even schedule something or if
			// we always call the execute directly
			break;
		}
		}
	}
	// Rewind and reset to the scene script after we are done executing
	_executingObjectIndex = Scenes::instance()._currentSceneIndex;
	setScript(Scenes::instance()._currentSceneScript);
	if (_stream && _stream->size() > 0) {
		_stream->seek(0, SEEK_SET);
	}
	_scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
	_state = ExecutorState::Idle;
	g_engine->_scriptExecutor->_isRepeatRun = false;
	// Original: restore cursor from Disabled when all scripts finish
	if (_mouseMode == MouseMode::Disabled) {
		_engine->setCursorMode(_cursorModeBeforeWait);
		View1 *v = (View1 *)_engine->findView("View1");
		if (v)
			v->updateCursor();
	}
}

bool ScriptExecutor::loadNextScript() {
	// Confirmed from runScriptExecutor (1008:e3e7): after the scene script finishes,
	// iterate executingObjectId from 1 to 0x200, skipping objects with no runtime data.
	// Load each object's script from runtime+0x187/+0x189/+0x18B.

	if (_scriptExecutionState == ScriptExecutionState::ExecutingSceneScript) {
		// If we are finished with executing the scene, we need to go over all relevant objects
		// The code below will increment to 1 to start at the protagonist
		_executingObjectIndex = 0;
		_scriptExecutionState = ScriptExecutionState::ExecutingOtherScripts;
	}

	// We always try to advance to the next object's script until we reach the end
	// of the objects list
	GameObject *candidateObject = nullptr;
	do {
		_executingObjectIndex++;
		candidateObject = GameObjects::getObjectByIndex(_executingObjectIndex);

		// TODO: Check if this is a valid option
		if (candidateObject && isRelevantObject(candidateObject)) {
			if (candidateObject->_script.size() != 0) {
				_stream = candidateObject->getScriptStream();
				_executingScriptObjectID = candidateObject->_index;
				debugC(kDebugScript, "----- Switching execution to script for object: %.4x", candidateObject->_index);
				return true;
			}
		}
	} while (candidateObject != nullptr);

	_executingScriptObjectID = 0;

	// We are done executing all relevant objects
	if (_isSceneInitRun) {
		// We need to start again at the scene object
		_isSceneInitRun = false;
		_repeatRunFlag = true;
		_executingObjectIndex = Scenes::instance()._currentSceneIndex;
		_stream = Scenes::instance()._currentSceneScript;
		if (!_stream || _stream->size() == 0) {
			return false;
		}
		_stream->seek(0, SEEK_SET);
		_scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
		debugC(kDebugScript, "----- Switching execution to script for scene: %.4x", _executingObjectIndex);
		return true;
	}

	if (_repeatRunFlag) {
		// Repeat run pass completed. Clear the flag (matching binary behavior:
		// scriptChangeScene sets g_wRepeatRunFlag=1 before runScriptExecutor,
		// then clears it to 0 immediately after).
		_repeatRunFlag = false;
		return false;
	}

	return false;
}

byte Script::ScriptExecutor::readByte() {
	return _stream->readByte();
}

uint16 Script::ScriptExecutor::readUint16() {
	return _stream->readUint16LE();
}

void Script::ScriptExecutor::scriptSetVar() {
	// This writes to a script variable.
	readByte();
	uint16 variableIndex = readUint16();
	ScriptVariable var;
	scriptReadValuePair(var.a, var.b);
	_variables[variableIndex] = var;
}

void Script::ScriptExecutor::scriptSetVarOr() {
	// Padding/type byte (same as opcode 0x01) - read and discarded
	readByte();
	uint16 variableIndex = readUint16();
	// We skip the left shift and just read the first value directly
	uint16 throwaway;
	uint16 value1;
	scriptReadValuePair(throwaway, value1);
	uint16 value2;
	uint16 value3;
	scriptReadValuePair(value2, value3);
	value2 |= value1;
	value3 |= 0x00;
	setVariableValue(variableIndex, value2, value3);
}

void Script::ScriptExecutor::scriptIfTrue() {
	uint16 res1;
	uint16 res2;
	scriptReadValuePair(res1, res2);
	_expectedEndLocation = _stream->pos();
	if ((res1 | res2) == 0) {
		scriptSkipBlock();
	}
	_expectedEndLocation = _stream->pos();
}

void Script::ScriptExecutor::scriptIfFalse() {
	uint16 result1;
	uint16 result2;
	scriptReadValuePair(result1, result2);
	_expectedEndLocation = _stream->pos();
	if (result1 | result2) {
		scriptSkipBlock();
	}
	_expectedEndLocation = _stream->pos();
}

bool Script::ScriptExecutor::scriptCompare() {
	// Comparison opcode from executeOpcodes (1008:db56).
	// Reads a comparison sub-opcode, two 32-bit values (v1:v2 and v3:v4),
	// and skips the following block if the condition is NOT met.
	// Values are treated as signed 32-bit (v2:v1 = high:low).
	uint8 opcode2 = readByte();
	Common::String opcodeInfo = identifyScriptOpcode(0x5, opcode2);
	debugC(kDebugScript, "- Second block opcode: %.2x %s", opcode2, opcodeInfo.c_str());
	uint16 v1; // low word of first value
	uint16 v2; // high word of first value
	scriptReadValuePair(v1, v2);
	uint16 v3; // low word of second value
	uint16 v4; // high word of second value
	scriptReadValuePair(v3, v4);

	// conditionMet = true means we execute the block (don't skip)
	bool conditionMet = false;
	const int32 val1 = (int32)((uint32)v2 << 16 | (uint32)v1);
	const int32 val2 = (int32)((uint32)v4 << 16 | (uint32)v3);

	if (opcode2 == 0x01) {
		// Equal
		conditionMet = (val1 == val2);
	} else if (opcode2 == 0x02) {
		// Not equal
		conditionMet = (val1 != val2);
	} else if (opcode2 == 0x03) {
		// Less than (signed 32-bit)
		conditionMet = (val1 < val2);
	} else if (opcode2 == 0x04) {
		// Greater than (signed 32-bit)
		conditionMet = (val1 > val2);
	} else if (opcode2 == 0x05) {
		// Less than or equal (signed 32-bit)
		conditionMet = (val1 <= val2);
	} else if (opcode2 == 0x06) {
		// Greater than or equal (signed 32-bit)
		conditionMet = (val1 >= val2);
	} else {
		scriptUnimplementedOpcode("Condition", opcode2);
		return false;
	}

	if (!conditionMet) {
		scriptSkipBlock();
	}

	return true;
}

void Script::ScriptExecutor::scriptIfInteraction() {
	// "Use item on object" comparison from executeOpcodes (1008:db56).
	// Reads sub-opcode (1=match, 2=NOT match), then the interacted pair
	// and two comparison objects. Checks both orderings.
	uint8 subOpcode = readByte();
	uint16 interacted1;
	uint16 interacted2;
	scriptReadValuePair(interacted1, interacted2);
	uint16 object1 = scriptReadValue16();
	uint16 object2 = scriptReadValue16();
	const bool match1 = (interacted1 == object1) && (interacted2 == object2);
	const bool match2 = (interacted1 == object2) && (interacted2 == object1);
	bool matched = match1 || match2;
	if (subOpcode == 0x02) {
		matched = !matched;
	}
	if (!matched) {
		scriptSkipBlock();
	}
}

void Script::ScriptExecutor::scriptEndIf() {
	// Opcode 0x07: no-op (confirmed: no handler in disassembly, falls through to loop).
	// It has no specific case handling code in the original.
}

void Script::ScriptExecutor::scriptElse() {
	// This is some kind of skipping as well.
	scriptSkipAlternate();
}

void Script::ScriptExecutor::scriptNop09() {
	// ExecuteScript does not currently have a dedicated opcode 0x09 dispatch branch.
}

void Script::ScriptExecutor::scriptPrintStringLeft() {
	// l0037_DDE8:
	scriptPrintString();
	// Ends execution (confirmed: jumps to e3bd in disassembly).
	endBuffering(_lastOpcodeTriggeredSkip);
}

bool Script::ScriptExecutor::scriptMoveObject() {
	// scriptMoveObject (1008:aa83). Moves an object to a new scene/position.
	// Handles render list updates for both source and destination scenes.
	uint32 objectID = scriptReadValue32() - 0x400;
	uint16 sceneID = scriptReadValue16();
	int16 x = (int16)scriptReadValue16();
	int16 y = (int16)scriptReadValue16();

	if (objectID < 1 || objectID > 0x200) {
		warning("Opcode 0x0B: invalid object %u", objectID);
		return false;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Opcode 0x0B: missing object %u", objectID);
		return false;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	const uint16 currentScene = Scenes::instance()._currentSceneIndex;
	const uint16 actorIndex = Scenes::instance()._currentActorIndex;

	// Step 1: Remove from render list if object was visible in current scene.
	// Original checks: object in current scene, OR in protagonist's inventory,
	// OR inside a container that is in the current scene.
	if (objectID != actorIndex) {
		bool wasInCurrentScene = (object->_sceneIndex == currentScene);
		if (!wasInCurrentScene && object->_sceneIndex == actorIndex + 0x400) {
			wasInCurrentScene = true; // was in protagonist's inventory
		}
		if (!wasInCurrentScene && object->_sceneIndex > 0x400) {
			GameObject *parent = GameObjects::getObjectByIndex(object->_sceneIndex - 0x400);
			if (parent != nullptr && parent->_sceneIndex == currentScene) {
				wasInCurrentScene = true; // was in a container in current scene
			}
		}
		if (wasInCurrentScene) {
			Character *c = currentView->getCharacterByIndex(objectID);
			if (c != nullptr) {
				int idx = currentView->getCharacterArrayIndex(c);
				if (idx >= 0)
					currentView->_characters.remove_at(idx);
			}
		}
	}

	// Step 2: Update object fields
	object->_sceneIndex = sceneID;
	object->_position = Common::Point(x, y);

	// Step 3: Add to render list if object is now visible in current scene.
	if (objectID != actorIndex) {
		bool isInCurrentScene = (sceneID == currentScene);
		if (!isInCurrentScene && sceneID == actorIndex + 0x400) {
			isInCurrentScene = true; // now in protagonist's inventory
		}
		if (!isInCurrentScene && sceneID > 0x400) {
			GameObject *parent = GameObjects::getObjectByIndex(sceneID - 0x400);
			if (parent != nullptr && parent->_sceneIndex == currentScene) {
				isInCurrentScene = true; // now in a container in current scene
			}
		}
		if (isInCurrentScene && sceneID == currentScene) {
			// Add as character to render list
			Character *c = currentView->getCharacterByIndex(objectID);
			if (c == nullptr) {
				c = new Character();
				c->_gameObject = object;
				currentView->_characters.push_back(c);
			}
			c->setPosition(Common::Point(x, y));
		}
	} else if (sceneID == currentScene) {
		// Actor moved into current scene — add to render list if not already present
		Character *c = currentView->getCharacterByIndex(objectID);
		if (c == nullptr) {
			c = new Character();
			c->_gameObject = object;
			currentView->_characters.push_back(c);
		}
		c->setPosition(Common::Point(x, y));
	} else {
		// Actor moved out of current scene — remove from render list
		Character *c = currentView->getCharacterByIndex(objectID);
		if (c != nullptr) {
			int idx = currentView->getCharacterArrayIndex(c);
			if (idx >= 0)
				currentView->_characters.remove_at(idx);
		}
	}

	// Step 4: Update inventory tracking
	if (sceneID == actorIndex + 0x400) {
		// Moved into protagonist's inventory
		bool alreadyInInventory = false;
		for (auto item : currentView->_inventoryItems) {
			if (item->_index == objectID) {
				alreadyInInventory = true;
				break;
			}
		}
		if (!alreadyInInventory)
			currentView->_inventoryItems.push_back(object);
	} else {
		// Remove from inventory if it was there
		for (uint i = 0; i < currentView->_inventoryItems.size(); i++) {
			if (currentView->_inventoryItems[i]->_index == objectID) {
				currentView->_inventoryItems.remove_at(i);
				break;
			}
		}
	}

	// Step 5: If object has no runtime data and was the UseInventory target,
	// reset cursor to Use (0x15)
	if (object->_blobs.empty()) {
		if (_interactedObjectID == objectID + 0x400 && _mouseMode == MouseMode::UseInventory) {
			_engine->setCursorMode(MouseMode::Use);
			currentView->updateCursor();
		}
		// Original also rewrites the saved (pre-wait) cursor mode: 0x17 -> 0x15.
		if (_interactedObjectID == objectID + 0x400 && _savedPickupMouseMode == MouseMode::UseInventory) {
			_savedPickupMouseMode = MouseMode::Use;
		}
	}

	// Step 6: If moved object is the one whose script is currently executing,
	// terminate its script (original: sets scriptEndPosition=0, scriptPosition=0)
	if ((int)objectID == _executingScriptObjectID) {
		_stream->seek(0, SEEK_END);
	}

	return true;
}

ExecutionResult Script::ScriptExecutor::scriptChangeScene() {
	// Scene change from scriptChangeScene (1008:ad6e).
	// Original behavior:
	//   1. Read sceneID, transitionMode, transitionSpeed
	//   2. Validate: sceneID must be 1..0x200
	//   3. If mode==0 && speed==0 or speed>0x40: error 0x26
	//   4. Save old palette, free scene resources, load new scene
	//   5. Transition:
	//      mode 0: fade from old palette at given speed
	//      mode 1: instant cut (clear screen, set palette)
	//   6. Run init pass (IsSceneInitRun=1) -> runScriptExecutor
	//   7. Draw scene, restore palette
	//   8. Set cursor to Walk (0x16)
	//   9. Set script position to end, executing object to 0x201
	//  10. If script was NOT already executing: run repeat pass
	//      (repeatRunFlag=1) -> runScriptExecutor -> (repeatRunFlag=0)
	//  11. If script WAS executing: error 0x17
	uint32 newSceneID = scriptReadValue32();
	const uint16 transitionMode = scriptReadValue16();
	const uint16 transitionSpeed = scriptReadValue16();
	View1 *currentView = (View1 *)_engine->findView("View1");
	// Binary: mode 0 fades old palette to black BEFORE loading new scene
	if (currentView != nullptr && transitionMode == 0 && transitionSpeed != 0) {
		currentView->startFadeToBlack(transitionSpeed);
	}
	g_engine->changeScene(newSceneID, false);
	// Binary: then fades from black to new palette after loading
	if (currentView != nullptr && transitionMode == 0 && transitionSpeed != 0) {
		currentView->startFadingWithSpeed(transitionSpeed);
	}
	// Binary step 8: set cursor to Walk (0x16) after scene change
	_engine->setCursorMode(Script::MouseMode::Walk);
	// Confirmed: executeOpcodes jumps to end-execution path after scriptChangeScene
	// in the game code
	// Confirmed: scriptChangeScene resets interactedObjectID and interactedInventoryItemId
	// or if there is another mechanism for this
	_interactedObjectID = 0;
	_interactedOtherObjectID = 0;
	_requestCallback = false;
	g_engine->scheduleRun(true);
	_isAwaitingCallback = true;
	// NOTE: EndTimer prevents race conditions from overlapping waits

	endTimer();
	endFrameWait();
	endBuffering(_lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

ExecutionResult Script::ScriptExecutor::scriptShowDialogue() {
	// Show a dialogue option.
	uint32 objectID = scriptReadValue32() - 0x400;
	uint16 x = scriptReadValue16();
	uint16 y = scriptReadValue16();
	uint16 side = scriptReadValue16();
	uint32 offset = readUint16();
	uint32 numLines = readUint16();

	View1 *currentView = (View1 *)_engine->findView("View1");

	Common::Array<Common::String> strings;
	if (_executingScriptObjectID == 0) {
		strings = g_engine->decodeStrings(Scenes::instance()._currentSceneStrings, offset, numLines, Scenes::instance()._currentSceneIndex, 0);
	} else {
		Common::MemoryReadStream *s = GameObjects::readGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		strings = g_engine->decodeStrings(s, offset, numLines, 0, _executingScriptObjectID);
	}

	debugC(kDebugScript,
		   "Opcode 0D dialogue: speaker=%u rawPos=(%u,%u) side=%u textOffset=%u numLines=%u scriptObject=%u text=\"%s\"",
		   objectID, x, y, side, offset, numLines, _executingScriptObjectID, joinDebugStrings(strings).c_str());

	_activeDialogueSpeakerObjectID = objectID;
	currentView->showSpeechAct(objectID, strings, Common::Point(x, y), side);
	_isAwaitingCallback = true;
	// NOTE: EndTimer prevents race conditions from overlapping waits

	endTimer();
	endFrameWait();
	endBuffering(_lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

bool Script::ScriptExecutor::scriptWalkToPosition() {
	// Binary scriptWalkToPosition (1008:b843):
	// Sets up runtime movement state. Does NOT block — walkAlongPath handles
	// actual movement per-frame from the game tick.
	uint32 objectID = scriptReadValue32() - 0x400;
	int16 x = (int16)scriptReadValue16();
	int16 y = (int16)scriptReadValue16();

	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *c = currentView ? currentView->getCharacterByIndex(objectID) : nullptr;
	if (c == nullptr) {
		warning("Ignoring walk-to for missing character %u", objectID);
		return false;
	}

	Common::Point current = c->getPosition();

	// Binary: runtime[0x16] = 0 (pathIndex = 0)
	c->_currentPathIndex = 0;
	c->_path.clear();
	c->_isFollowingPath = false;

	// Binary: runtime[4] = x, runtime[5] = y (finalDest)
	c->_pathFinalDestination = Common::Point(x, y);

	// Binary: isPathWalkable(finalDestY, finalDestX, currentY, currentX)
	// = trace FROM current TOWARD finalDest
	bool directPath = _engine->isPathWalkable(y, x, current.y, current.x);

	if (!directPath && _engine->getWalkabilityAt(y, x) < 200) {
		// Not directly walkable but destination is valid — use pathfinding
		// Binary: calculatePath(finalDestY, finalDestX, currentY, currentX, actorIndex)
		c->calculatePath(Common::Point(x, y));
		c->_isFollowingPath = (c->_path.size() > 0);
	}

	// Set immediate target: either from pathfinding result or direct to finalDest
	if (!c->_isFollowingPath) {
		// Binary else branch: runtime[0x16]=0, runtime[0x17]=0, target=finalDest
		c->_endPosition = c->_pathFinalDestination;
	}
	// If path was found, calculatePath already set _endPosition to first waypoint

	// Binary: compute deltaX, deltaY, reset error and directionCalculated
	c->_stepDeltaX = abs(c->_endPosition.x - current.x);
	c->_stepDeltaY = abs(c->_endPosition.y - current.y);
	c->_stepError = 0;
	c->_stepDirectionSet = false;
	c->_isLerping = true;

	return true;
}

ExecutionResult Script::ScriptExecutor::scriptWaitForWalk() {
	// Wait for walk completion from executeOpcodes (1008:db56).
	// Original: validates object, checks runtime data exists, checks frozen flag,
	// sets g_wWalkTargetObjectIndex, hides cursor, returns to gameTick.
	// gameTick checks walk completion each frame.
	uint32 objectID = scriptReadValue32() - 0x400;
	if (objectID < 1 || objectID > 0x200) {
		warning("Opcode 0x11: invalid object %u", objectID);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	GameObject *walkObject = GameObjects::getObjectByIndex(objectID);
	if (walkObject == nullptr) {
		warning("Opcode 0x11: missing object %u", objectID);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	// Original checks runtime+0x231 (frozen/attached flag) → error 0x1F
	if (walkObject->_hasBoundsAttachment) {
		warning("Opcode 0x11: object %u is frozen (bounds attached)", objectID);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *c = currentView->getCharacterByIndex(objectID);
	if (c == nullptr) {
		// Original: error code 2 (no runtime data). Script execution stops.
		warning("Opcode 0x11: no character for object %u (no runtime data)", objectID);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	c->registerWaitForMovementFinishedEvent();
	_requestCallback = false;
	_isAwaitingCallback = true;
	endTimer();
	endBuffering(_lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

void Script::ScriptExecutor::scriptSkipUntil14() {
	uint16 tag = readUint16();
	_stream->seek(0, SEEK_SET);
	while (_stream->pos() < _stream->size()) {
		uint8 opcode = readByte();
		uint8 length = readByte();
		if (opcode == 0x14) {
			uint16 tag14 = readUint16();
			if (tag14 == tag) {
				return;
			}
		} else {
			_stream->seek(length, SEEK_CUR);
		}
	}
	_expectedEndLocation = _stream->pos();
}

void Script::ScriptExecutor::scriptSkipWord() {
	// If we reach opcode 14 regularly, just discard the payload and continue.
	readUint16();
}

void Script::ScriptExecutor::scriptClearDialogueChoices() {
	// Mark that we are gathering strings for setting up a dialogue choice.
	_dialogueChoices.clear();
}

void Script::ScriptExecutor::scriptAddDialogueChoice() {
	// Add a dialogue choice.
	uint16 index = scriptReadValue16();
	// We don't save the index, instead we make sure that we add them in the right
	// order and use the array to keep track.
	// TODO: Removed this assert, during the dialogue in the beginning of chapter
	// 3 (at the fort) an index of 3 came up when only one item had been there before.
	// Not sure if the way of handling it still works or reflects the game, needs
	// to be tested.
	// assert(index - 1 == DialogueChoices.size());
	uint16 offset = readUint16();
	uint16 numLines = readUint16();
	Common::StringArray lines;
	if (_executingScriptObjectID == 0) {
		lines = _engine->decodeStrings(Scenes::instance()._currentSceneStrings, offset, numLines, Scenes::instance()._currentSceneIndex, 0);
	} else {
		Common::MemoryReadStream *stringsStream = GameObjects::readGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		lines = _engine->decodeStrings(stringsStream, offset, numLines, 0, _executingScriptObjectID);
	}
	debugC(kDebugScript,
		   "Opcode 16 choice text: index=%u textOffset=%u numLines=%u scriptObject=%u text=\"%s\"",
		   index, offset, numLines, _executingScriptObjectID, joinDebugStrings(lines).c_str());
	_dialogueChoices.push_back(lines);
}

ExecutionResult Script::ScriptExecutor::scriptShowDialogueChoice() {
	// Finish the dialogue choice.
	View1 *currentView = (View1 *)_engine->findView("View1");
	uint32 x = scriptReadValue32();
	uint32 y = scriptReadValue32();
	uint16 side = scriptReadValue16();
	const uint16 speakerObjectID = Scenes::instance()._currentActorIndex;
	debugC(kDebugScript,
		   "Opcode 17 choice box: speaker=%u rawPos=(%u,%u) side=%u choiceCount=%u",
		   speakerObjectID, x, y, side, _dialogueChoices.size());
	currentView->showDialogueChoice(speakerObjectID, _dialogueChoices, Common::Point(x, y), side);
	_requestCallback = false;
	// NOTE: EndTimer prevents race conditions from overlapping waits

	endTimer();
	endBuffering(_lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

ExecutionResult Script::ScriptExecutor::scriptDismissPanel() {
	// Set the stream to the end and let the calling code figure out that we are done
	// for this run.
	_stream->seek(_stream->size(), SEEK_SET);
	endBuffering(_lastOpcodeTriggeredSkip);
	return ExecutionResult::ScriptFinished;
}

bool Script::ScriptExecutor::scriptWalkToAndPickup() {
	// Walk to and pick up an object.
	uint32 actorIndex = scriptReadValue32() - 0x400;
	uint32 objectIndex = scriptReadValue32() - 0x400;

	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *actor = currentView->getCharacterByIndex(actorIndex);
	GameObject *targetObject = GameObjects::getObjectByIndex(objectIndex);
	if (_pickupInProgress) {
		endTimer();
		endBuffering(_lastOpcodeTriggeredSkip);
		return true;
	}
	if (actor == nullptr || targetObject == nullptr) {
		warning("Invalid pickup request for actor %u target %u", actorIndex, objectIndex);
		return false;
	}
	if (actorIndex == objectIndex || targetObject->_sceneIndex == actor->_gameObject->_index) {
		warning("Ignoring invalid pickup request for actor %u target %u", actorIndex, objectIndex);
		return false;
	}
	if (targetObject->_sceneIndex != actor->_gameObject->_sceneIndex) {
		warning("Ignoring pickup across scenes for actor %u target %u", actorIndex, objectIndex);
		return false;
	}
	_pickupInProgress = true;
	_pickupActorObjectID = actorIndex;
	_pickupTargetObjectID = objectIndex;
	_savedPickupMouseMode = _mouseMode == MouseMode::UseInventory ? MouseMode::Use : _mouseMode;
	currentView->_activeInventoryItem = nullptr;
	_engine->setCursorMode(_savedPickupMouseMode);
	currentView->updateCursor();
	actor->startPickup(targetObject);
	_requestCallback = false;
	_isAwaitingCallback = true;
	// NOTE: EndTimer prevents race conditions from overlapping waits

	endTimer();
	endBuffering(_lastOpcodeTriggeredSkip);
	return true;
}

bool Script::ScriptExecutor::scriptSetPickupFrames() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 value217 = scriptReadValue16();
	uint16 value219 = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring object runtime setup for invalid object %d", objectID);
		return false;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring object runtime setup for missing object %d", objectID);
		return false;
	}

	object->_pickupFrameStart = value217;
	object->_pickupFrameEnd = value219;
	return true;
}

bool Script::ScriptExecutor::scriptSetupObject() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 slotID = scriptReadValue16();
	uint16 value = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring object slot setup for invalid object %d", objectID);
		return false;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring object slot setup for missing object %d", objectID);
		return false;
	}

	if (slotID < 1 || slotID > 0x15) {
		warning("Ignoring object slot setup for invalid slot %u on object %d", slotID, objectID);
		return false;
	}

	// Binary writes to runtime+slot*0x10+0x30 which is the per-slot animation speed.
	// walkAlongPath reads from the same offset. This is _blobSpeeds in ScummVM.
	if ((uint)(slotID - 1) < object->_blobSpeeds.size()) {
		object->_blobSpeeds[slotID - 1] = value;
	}
	return true;
}

void Script::ScriptExecutor::scriptSetSkippable() {
	// Sets g_wScriptSkippable [102Ah] = 1.
	_scriptSkippable = true;
}

void Script::ScriptExecutor::scriptClearSkippable() {
	// Sets g_wScriptSkippable [102Ah] = 0.
	_scriptSkippable = false;
}

bool Script::ScriptExecutor::scriptPlayAnimation() {
	// scriptPlayAnimation (1008:bd58).
	uint32 objectID = scriptReadValue32() - 0x400;
	uint32 slotID = scriptReadValue16();
	uint32 frameOffset = scriptReadValue16();

	if (objectID < 1 || objectID > 0x200) {
		warning("Opcode 0x1E: invalid object %u", objectID);
		return false;
	}
	GameObject *gameObject = GameObjects::getObjectByIndex(objectID);
	if (gameObject == nullptr) {
		warning("Opcode 0x1E: missing object %u", objectID);
		return false;
	}
	if (slotID < 1 || slotID > 0x15) {
		warning("Opcode 0x1E: invalid slot %u for object %u", slotID, objectID);
		return false;
	}

	if (slotID == 0x15) {
		gameObject->useOverloadAnimation = true;
		BackgroundAnimationBlob::advanceAnimFrame(gameObject->overloadAnimation,
												  true, frameOffset + 0x64);
	} else {
		if (slotID - 1 >= gameObject->_blobs.size() || gameObject->_blobs[slotID - 1].empty()) {
			warning("Opcode 0x1E: no blob data for object %u slot %u", objectID, slotID);
			return false;
		}
		BackgroundAnimationBlob::advanceAnimFrame(gameObject->_blobs[slotID - 1],
												  true, frameOffset + 0x64);
	}

	return true;
}

void Script::ScriptExecutor::scriptTestPathfinding() {
	uint32 objectID = scriptReadValue32() - 0x400;
	uint32 x = scriptReadValue32();
	uint32 y = scriptReadValue32();
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	_pathWalkableResult = false;
	if (object == nullptr) {
		warning("Ignoring pathfinding test for invalid object %u", objectID);
	} else {
		_pathWalkableResult = _engine->isPathWalkable(object->_position.y, object->_position.x, y, x);
	}
}

bool Script::ScriptExecutor::scriptSetYOffset() {
	// scriptSetYOffset (1008:c047). Sets object field +8 (vertical offset)
	// AND mirrors it into runtime field +0x21D (motion target).
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 offset = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring vertical offset set for invalid object %d", objectID);
		return false;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring vertical offset set for missing object %d", objectID);
		return false;
	}

	object->_verticalOffsetScale = offset;
	// Original also writes to runtime +0x21D (motion target vertical offset)
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr) {
		Character *c = currentView->getCharacterByIndex((uint16)objectID);
		if (c != nullptr) {
			c->_motionTargetVerticalOffset = offset;
		}
	}
	return true;
}

bool Script::ScriptExecutor::scriptSetMotion() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 targetVerticalOffset = scriptReadValue16();
	uint16 verticalOffsetDelta = scriptReadValue16();
	uint16 motionDistance = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring motion setup for invalid object %d", objectID);
		return false;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *character = currentView ? currentView->getCharacterByIndex((uint16)objectID) : nullptr;
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr || character == nullptr) {
		warning("Ignoring motion setup for missing character object %d", objectID);
		return false;
	}

	character->_motionStartVerticalOffset = object->_verticalOffsetScale;
	character->_motionTargetVerticalOffset = targetVerticalOffset;
	character->_motionVerticalOffsetDelta = verticalOffsetDelta;
	character->_motionDistanceUnits = motionDistance;
	character->_motionProgress = 0;
	character->_hasMotionVerticalOffset = motionDistance != 0 || targetVerticalOffset != object->_verticalOffsetScale;
	return true;
}

bool Script::ScriptExecutor::scriptSetOrientation() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 animIndex = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring orientation set for invalid object %d", objectID);
		return false;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring orientation set for missing object %d", objectID);
		return false;
	}

	if (animIndex < 9 || animIndex > 0x10) {
		warning("Ignoring out-of-range orientation %u for object %d", animIndex, objectID);
		return false;
	}

	object->_orientation = animIndex;
	return true;
}

bool Script::ScriptExecutor::scriptMoveToPosition() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint32 x = scriptReadValue32();
	uint32 y = scriptReadValue32();
	uint16 targetVerticalOffset = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring move-to-position for invalid object %d", objectID);
		return false;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	Character *c = currentView ? currentView->getCharacterByIndex((uint16)objectID) : nullptr;
	if (object == nullptr || c == nullptr) {
		warning("Ignoring move-to-position for missing character object %d", objectID);
		return false;
	}

	const Common::Point target(x, y);
	if (!_engine->isPathWalkable(object->_position.y, object->_position.x, y, x) && _engine->getWalkabilityAt(target) < 0xC8) {
		warning("Ignoring move-to-position for blocked target (%u,%u) on object %d", x, y, objectID);
		return false;
	}

	c->_isFollowingPath = false;
	c->_motionStartVerticalOffset = object->_verticalOffsetScale;
	c->_motionTargetVerticalOffset = targetVerticalOffset;
	c->_motionVerticalOffsetDelta = ABS<int32>((int32)object->_verticalOffsetScale - (int32)targetVerticalOffset);
	c->_motionDistanceUnits = ABS<int32>((int32)x - object->_position.x) + ABS<int32>((int32)y - object->_position.y);
	c->_motionProgress = 0;
	c->_hasMotionVerticalOffset = true;
	c->startLerpTo(Common::Point(x, y), 2 * 1000);
	_isAwaitingCallback = true;
	return true;
}

void Script::ScriptExecutor::scriptAddValues() {
	// Adds two values read and saves them to a script variable.
	// ;; fn0037_C7E6: 0037:C7E6
	uint32 a = scriptReadValue32();
	uint32 b = scriptReadValue32();

	uint32 result = a + b;
	// Go back to the first value being pointed to.
	// In this case, 9F4D and A334 can use the same data, since the
	// index of the script variable will be in the word at offset 1.
	_stream->seek(-6, SEEK_CUR);
	scriptSaveVariableHelper(result);
	// Skip forward across the second 9F4D read's data.
	_stream->seek(3, SEEK_CUR);
}

void Script::ScriptExecutor::scriptSubValues() {
	// Subtracts two values read and saves them to a script variable.
	// ;; fn0037_C82E: 0037:C82E
	uint32 a = scriptReadValue32();
	uint32 b = scriptReadValue32();

	uint32 result = a - b;
	_stream->seek(-6, SEEK_CUR);
	scriptSaveVariableHelper(result);
	_stream->seek(3, SEEK_CUR);
}

void Script::ScriptExecutor::scriptLoadSpecialAnim() {
	// This one loads a special animation set into the overload slot.
	uint32 id = scriptReadValue32() - 0x400;
	// scriptLoadSpecialAnim (1008:c991): 2nd value is the mirror flag (runtime +0x182).
	// Non-zero -> set mirror flag and horizontally flip the blob.
	// The loaded flag (+0x183) is always set to 1.
	uint16 shouldMirror = scriptReadValue16();
	uint8 animationID = readByte();
	Common::Array<uint8> blob = Scenes::instance().readSpecialAnimBlob(animationID, g_engine->_fileStream);
	GameObject *object = GameObjects::getObjectByIndex(id);
	object->overloadAnimation = blob;
	object->overloadAnimationMirrored = (shouldMirror != 0);
	if (shouldMirror != 0) {
		BackgroundAnimationBlob::mirrorAnimBlob(object->overloadAnimation);
	}
}

bool Script::ScriptExecutor::scriptSetDirection() {
	// scriptSetDirection (1008:c858). Writes to runtime field +0x22D.
	// When the character's orientation matches this value, the renderer
	// uses animation slot 0x15 (overload animation) instead of the normal slot.
	// Value 0x7FFF means "never match" (default from loadSceneObjects).
	uint32 characterID = scriptReadValue32() - 0x400;
	uint16 value = scriptReadValue16();
	if (characterID < 1 || characterID > 0x200) {
		warning("Ignoring set direction for invalid object %u", characterID);
		return false;
	}
	GameObject *object = GameObjects::getObjectByIndex(characterID);
	if (object == nullptr) {
		warning("Ignoring set direction for missing object %u", characterID);
		return false;
	}
	object->overloadAnimTriggerDirection = value;
	return true;
}

void Script::ScriptExecutor::scriptStopAnimation() {
	// scriptStopAnimation (1008:c8e4). Original behavior:
	//   1. Read objectID, validate
	//   2. Set runtime +0x22D = 0x7FFF (remove direction/frame limit)
	//   3. Free overload animation blob if loaded (runtime +0x183 flag)
	//   4. Clear overload flag
	uint32 characterID = scriptReadValue32() - 0x400;
	GameObject *obj = GameObjects::getObjectByIndex(characterID);
	if (obj == nullptr) {
		warning("Ignoring stop animation for missing object %u", characterID);
		return;
	}
	obj->overloadAnimTriggerDirection = 0x7FFF;
	obj->useOverloadAnimation = false;
	obj->overloadAnimation.clear();
}

bool Script::ScriptExecutor::scriptOpenInventory() {
	uint32 objectID = scriptReadValue32();
	objectID -= 0x400;
	View1 *currentView = (View1 *)_engine->findView("View1");
	GameObject *inventorySource = GameObjects::getObjectByIndex(objectID);
	if (inventorySource == nullptr) {
		warning("Invalid inventory source object %u", objectID);
		return false;
	}
	_savedExternalInventoryMouseMode = _mouseMode == MouseMode::UseInventory ? MouseMode::Use : _mouseMode;
	_hasPendingExternalInventoryResume = true;
	_externalInventorySourceObjectID = objectID;
	_secondaryInventoryLocation = _stream->pos();
	// Save script click state (original saves at 0xf94-0xf9a equivalents)
	_savedScriptClickFlag = _scriptClickFlag;
	_savedScriptClickX = _scriptClickX;
	_savedScriptClickY = _scriptClickY;
	_savedScriptClickResult = _scriptClickResult;
	currentView->openInventory(inventorySource);
	return true;
}

void Script::ScriptExecutor::scriptLoadObjectAnim() {
	uint32 objectID = scriptReadValue32() - 0x400;
	uint16 slotID = scriptReadValue16();
	const bool shouldMirror = scriptReadValue16() != 0;
	uint8 arrayIndex = readByte();

	g_engine->loadAnimationFromSceneData(objectID, slotID, arrayIndex, shouldMirror);
}

bool Script::ScriptExecutor::scriptCheckObjectData() {
	const uint16 objectID = scriptReadValue16() - 0x400;
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring object refresh for invalid object %u", objectID);
		return false;
	}
	if (object->_blobs.empty()) {
		warning("Ignoring object refresh for unloaded object %u", objectID);
		return false;
	}
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView == nullptr) {
		return false;
	}

	Character *character = currentView->getCharacterByIndex(objectID);
	const int currentIndex = currentView->getCharacterArrayIndex(character);
	if (object->_sceneIndex != Scenes::instance()._currentSceneIndex) {
		if (currentIndex >= 0) {
			currentView->_characters.remove_at(currentIndex);
		}
		return false;
	}

	if (character == nullptr) {
		character = new Character();
		character->_gameObject = object;
	} else if (currentIndex >= 0) {
		currentView->_characters.remove_at(currentIndex);
	}

	currentView->_characters.push_back(character);
	return true;
}

bool Script::ScriptExecutor::scriptCheckInventory() {
	uint16 objectID = scriptReadValue16() - 0x400;
	uint16 parentID = scriptReadValue16();
	const GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring inventory check for invalid object %u", objectID);
		return false;
	}
	_inventoryCheckResult = object->_sceneIndex == parentID;
	return true;
}

bool Script::ScriptExecutor::scriptSetSnapToTarget() {
	const uint16 objectID = scriptReadValue16() - 0x400;
	const bool enabled = scriptReadValue16() != 0;
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring object runtime flag for invalid object %u", objectID);
		return false;
	}
	object->_snapToTarget = enabled;
	return true;
}

bool Script::ScriptExecutor::scriptTestObjectAnimFrame() {
	// scriptTestObjectAnimFrame: Tests if an object's animation blob's
	// current frame index (via getAnimBlobOffset/source key) falls within
	// [minFrame, maxFrame]. Result stored in animBlobRangeTestResult for helper FF29.
	uint32 objectID = scriptReadValue32() - 0x400;
	uint16 slotID = scriptReadValue16();
	uint16 minFrame = scriptReadValue16();
	uint16 maxFrame = scriptReadValue16();
	_animBlobRangeTestResult = false;
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring object animation range test for invalid object %u", objectID);
		return false;
	}

	uint16 blobSourceKey = 0;
	bool hasBlob = false;
	if (slotID == 0x15) {
		hasBlob = !object->overloadAnimation.empty();
		blobSourceKey = object->overloadAnimationSourceKey;
	} else if (slotID >= 1 && slotID <= object->_blobs.size()) {
		hasBlob = !object->_blobs[slotID - 1].empty();
		if ((uint)(slotID - 1) < object->_blobSourceKeys.size())
			blobSourceKey = object->_blobSourceKeys[slotID - 1];
	} else {
		warning("Ignoring object animation range test for invalid slot %u on object %u", slotID, objectID);
		return false;
	}

	if (hasBlob) {
		_animBlobRangeTestResult = blobSourceKey >= minFrame && blobSourceKey <= maxFrame;
	}
	return true;
}

void Script::ScriptExecutor::scriptPrintStringRight() {
	// Opcode 0x30: scriptPrintString with flag=1 (vs opcode 0x0A with flag=0)
	// which changes behaviour in the function.
	scriptPrintString(true);
	endBuffering(_lastOpcodeTriggeredSkip);
}

void Script::ScriptExecutor::scriptSetVolume() {
	// scriptSetVolume (1008:ce0b): clamp the value to 0..100 (signed), as the original does.
	int16 volume = (int16)scriptReadValue16();
	if (volume < 0)
		volume = 0;
	if (volume > 100)
		volume = 100;
	g_engine->getAdlib()->setVolume(g_engine->scaledMusicVolume((uint16)volume));
}

bool Script::ScriptExecutor::scriptSetObjectClickable() {
	uint16 objectID = scriptReadValue16() - 0x0400;
	const uint16 clickable = scriptReadValue16();
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring clickable toggle for invalid object %u", objectID);
		return false;
	}
	object->_isClickable = clickable != 0;
	return true;
}

bool Script::ScriptExecutor::scriptSetObjectVisible() {
	uint16 objectID = scriptReadValue16() - 0x0400;
	const uint16 visible = scriptReadValue16();
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring visibility toggle for invalid object %u", objectID);
		return false;
	}
	object->_isVisible = visible != 0;
	return true;
}

bool Script::ScriptExecutor::scriptSetHotspotOverride() {
	// Sets an entry in the [5BD1] list for hotspot lookup.
	const uint16 v1 = scriptReadValue16() - 0x800;
	const uint16 v2 = scriptReadValue16() - 0x800;

	if (v1 < 0x1 || v1 > 0x10 || v2 < 0x1 || v2 > 0x10) {
		warning("Ignoring hotspot override %.4x -> %.4x outside valid range", v1 + 0x800, v2 + 0x800);
		return false;
	}
	if (v1 == v2) {
		g_engine->_hotspotOverrides[v1] = 0xFFFF;
	} else {
		g_engine->_hotspotOverrides[v1] = v2;
	}
	return true;
}

bool Script::ScriptExecutor::scriptSetObjectBounds() {
	uint16 objectID = scriptReadValue16() - 0x0400;
	uint16 otherObjectID = scriptReadValue16() - 0x0400;
	const uint16 value1 = scriptReadValue16();
	const uint16 value2 = scriptReadValue16();
	const uint16 value3 = scriptReadValue16();
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	GameObject *otherObject = GameObjects::getObjectByIndex(otherObjectID);
	if (object == nullptr || otherObject == nullptr) {
		warning("Ignoring bounds attachment for invalid objects %u -> %u", objectID, otherObjectID);
		return false;
	}

	if (objectID == otherObjectID) {
		object->_hasBoundsAttachment = false;
		object->_boundsAttachmentObjectID = 0;
		object->_boundsAttachmentValue1 = 0;
		object->_boundsAttachmentValue2 = 0;
		object->_boundsAttachmentValue3 = 0;
	} else {
		object->_hasBoundsAttachment = true;
		object->_boundsAttachmentObjectID = otherObjectID;
		object->_boundsAttachmentValue1 = value1;
		object->_boundsAttachmentValue2 = value2;
		object->_boundsAttachmentValue3 = value3;
	}
	return true;
}

void Script::ScriptExecutor::scriptDismissAllPanels() {
	// scriptDismissPanel (1008:d6dd). Restores background if a UI panel
	// was pending, clears panel state, redraws scene, clears timer flag.
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr) {
		if (currentView->_isShowingTextBox || currentView->_uiPanelState == View1::kUiPanelDialogue) {
			currentView->_continueScriptAfterUI = false;
			currentView->clearStringBox(false);
		}

		if (currentView->_uiPanelState == View1::kUiPanelInventory) {
			_hasPendingExternalInventoryResume = false;
			_externalInventorySourceObjectID = 0;
			currentView->closeInventory();
		}

		if (currentView->_uiPanelState == View1::kUiPanelActionBar) {
			currentView->_uiPanelState = View1::kUiPanelNone;
			_engine->setCursorMode(currentView->_cursorModeBeforeMenu);
			currentView->updateCursor();
			currentView->redraw();
		}
	}
}

void Script::ScriptExecutor::scriptResetToSceneScript() {
	// scriptResetToSceneScript (1008:ad3e). Resets script execution
	// context back to the current scene script at position 0.
	_executingScriptObjectID = 0;
	_executingObjectIndex = Scenes::instance()._currentSceneIndex;
	_scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
	_activeDialogueSpeakerObjectID = 0;
	setCurrentSceneScriptAt(0);
}

void Script::ScriptExecutor::scriptLoadOverlayFont() {
	// scriptLoadOverlayFont (1008:d749). Loads a font resource for
	// overlay text into the overlay font buffer.
	uint8 resourceIndex = readByte();
	_overlayTextStageActive = true;
	if (!_engine->loadOverlayFont(resourceIndex, _executingScriptObjectID)) {
		warning("Opcode 0x38: failed to load overlay font resource %u", resourceIndex);
	}
}

Script::ScriptExecutor::OpcodeControlFlow Script::ScriptExecutor::scriptAddOverlayTextEntry() {
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView == nullptr) {
		warning("Ignoring overlay text entry without an active View1");
		scriptReadValue16();
		scriptReadValue16();
		scriptReadValue16();
		readUint16();
		readUint16();
		return OpcodeControlFlow::Continue;
	}

	const uint16 x = scriptReadValue16();
	const uint16 y = scriptReadValue16();
	const uint8 alignment = scriptReadValue16();
	const uint16 stringOffset = readUint16();
	const uint16 entryType = readUint16();
	if (!_overlayTextStageActive) {
		warning("Opcode 0x3A: overlay text entry at %u,%u without active stage (error 0x21)", x, y);
		endBuffering(_lastOpcodeTriggeredSkip);
		return OpcodeControlFlow::ScriptFinished;
	}
	if (currentView->_overlayTextEntries.size() >= 10) {
		warning("Ignoring overlay text entry because the overlay list is full");
		return OpcodeControlFlow::Continue;
	}
	if (entryType != 1) {
		warning("Ignoring overlay text entry with unsupported entry type %u", entryType);
		return OpcodeControlFlow::Continue;
	}

	Common::StringArray strings;
	if (_executingScriptObjectID == 0) {
		strings = _engine->decodeStrings(Scenes::instance()._currentSceneStrings, stringOffset, 1, Scenes::instance()._currentSceneIndex, 0);
	} else {
		Common::MemoryReadStream *stringsStream = GameObjects::readGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		strings = _engine->decodeStrings(stringsStream, stringOffset, 1, 0, _executingScriptObjectID);
	}
	if (strings.empty()) {
		warning("Ignoring empty overlay text entry at offset %u", stringOffset);
		return OpcodeControlFlow::Continue;
	}
	debugC(kDebugScript,
		   "Opcode 3A overlay text: rawPos=(%u,%u) align=%u textOffset=%u entryType=%u scriptObject=%u text=\"%s\"",
		   x, y, alignment, stringOffset, entryType, _executingScriptObjectID, strings[0].c_str());

	View1::OverlayTextEntry entry;
	entry.position = Common::Point(x, y);
	entry.alignment = alignment;
	entry.text = strings[0];
	if (entry.text.size() > 0x28) {
		entry.text = entry.text.substr(0, 0x28);
	}
	currentView->addOverlayTextEntry(entry);
	return OpcodeControlFlow::Fallthrough;
}

void Script::ScriptExecutor::scriptClearOverlayText() {
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr) {
		currentView->clearOverlayTextEntries();
	}
}

void Script::ScriptExecutor::scriptFadeToBlack() {
	const uint16 fadeSpeed = scriptReadValue16();
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr && fadeSpeed != 0) {
		currentView->startFadeToBlack(fadeSpeed);
	}
}

void Script::ScriptExecutor::scriptFrameWait() {
	// The original interpreter stores a frame countdown that is decremented
	// once per game tick, rather than using a wall-clock timer.
	uint16 duration = scriptReadValue16();
	_requestCallback = false;
	startFrameWait(duration);
	_isAwaitingCallback = true;
	endBuffering(_lastOpcodeTriggeredSkip);
}

void Script::ScriptExecutor::scriptSetPathfinding() {
	// scriptSetPathfinding (1008:c6d7). Sets/clears a pathfinding override.
	// Index must be in range 200..0xEF (walkability values).
	// Writes to scene data at index*5 + 0x4EA5 (enable byte) and +0x4EA6 (value).
	uint16 areaID = scriptReadValue16();
	uint16 active = scriptReadValue16();
	uint16 overrideValue = scriptReadValue16();
	if (active) {
		g_engine->setPathfindingOverride(areaID, overrideValue);
	} else {
		g_engine->removePathfindingOverride(areaID);
	}
}

void Script::ScriptExecutor::scriptTestSceneAnimFrame() {
	// scriptTestSceneAnimFrame: Tests if a scene's special animation blob's
	// current frame index (via getAnimBlobOffset/source key) falls within
	// [minFrame, maxFrame]. Result stored in animBlobRangeTestResult for helper FF29.
	// Index is 0x1000-based in the bytecode (matches opcode 0x0E scriptChangeAnimation,
	// binary scriptTestSceneAnimFrame at 1008:... subtracts 0x1000).
	uint32 sceneAnimIndex = scriptReadValue32() - 0x1000;
	uint32 minFrame = scriptReadValue32();
	uint32 maxFrame = scriptReadValue32();
	_animBlobRangeTestResult = false;
	if (sceneAnimIndex == 0 || sceneAnimIndex > Scenes::instance()._currentSceneSpecialAnimOffsets.size()) {
		warning("Ignoring scene animation range test for invalid index %u", sceneAnimIndex);
	} else {
		const uint16 blobSourceKey = static_cast<uint16>(Scenes::instance()._currentSceneSpecialAnimOffsets[sceneAnimIndex - 1] >> 16);
		_animBlobRangeTestResult = blobSourceKey >= minFrame && blobSourceKey <= maxFrame;
	}
}

void Script::ScriptExecutor::scriptEndOverlayText() {
	if (_overlayTextStageActive) {
		_overlayTextStageActive = false;
	}
}

void Script::ScriptExecutor::scriptFadeFromBlack() {
	const uint16 fadeSpeed = scriptReadValue16();
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr && fadeSpeed != 0) {
		currentView->startFading(fadeSpeed);
	}
}

bool Script::ScriptExecutor::scriptLoadPcmSound() {
	const uint8 resourceIndex = readByte();
	Common::Array<uint8> soundData;
	if (!loadSoundResource(soundData, resourceIndex)) {
		return false;
	}

	if (_engine->hasCurrentSound() && _soundEnabled) {
		_engine->stopCurrentSound();
	}
	_engine->setCurrentSoundData(soundData);
	return true;
}

bool Script::ScriptExecutor::scriptPlayPcmSound() {
	if (_soundEnabled) {
		if (!_engine->hasCurrentSound()) {
			warning("Ignoring sound playback without loaded sound data");
			return false;
		}
		_engine->playCurrentSound();
	}
	return true;
}

bool Script::ScriptExecutor::scriptWaitForSound() {
	if (_soundEnabled && _soundSystemActive) {
		_waitForSoundPlayback = true;
		endTimer();
		endBuffering(_lastOpcodeTriggeredSkip);
		return true;
	}
	return false;
}

void Script::ScriptExecutor::scriptStopPcmSound() {
	if (_soundEnabled) {
		_engine->stopCurrentSound();
	}
}

bool Script::ScriptExecutor::scriptLoadMusicSlot() {
	const uint16 slotID = scriptReadValue16();
	const uint8 resourceIndex = readByte();
	if (slotID < 1 || slotID > 2) {
		warning("Ignoring music load for invalid slot %u", slotID);
		return false;
	}

	Common::Array<uint8> slotData;
	if (loadMusicResource(slotData, resourceIndex)) {
		_musicSlots[slotID - 1] = slotData;
	}
	return true;
}

bool Script::ScriptExecutor::scriptPlayMusicSlot() {
	const uint16 slotID = scriptReadValue16();
	const uint16 startMuted = scriptReadValue16();
	const uint16 fadeParam = scriptReadValue16();
	if (slotID < 1 || slotID > 2) {
		warning("Ignoring music start for invalid slot %u", slotID);
		return false;
	}

	if (!_musicEnabled || !_soundSystemActive) {
		_activeMusicSlot = slotID;
		return false;
	}

	if (_activeMusicSlot != 0) {
		_engine->getAdlib()->stopMusic();
		_activeMusicSlot = 0;
	}

	if (_musicSlots[slotID - 1].empty()) {
		warning("Ignoring music start for empty slot %u", slotID);
		return false;
	}

	_engine->getAdlib()->playSongData(_musicSlots[slotID - 1]);
	if (startMuted == 0) {
		_musicControlMode = 1;
		_musicControlParam = fadeParam;
		_musicControlVolume = 0x3F;
		_engine->getAdlib()->setVolume(_engine->scaledMusicVolume(_musicControlVolume));
	} else {
		_musicControlMode = 0;
		_musicControlParam = 0;
		_musicControlVolume = 0;
		_engine->getAdlib()->setVolume(_engine->scaledMusicVolume(0));
	}

	_activeMusicSlot = slotID;
	return true;
}

bool Script::ScriptExecutor::scriptStopMusicSlot() {
	const uint16 slotID = scriptReadValue16();
	const uint16 stopImmediately = scriptReadValue16();
	const uint16 fadeParam = scriptReadValue16();
	if (slotID < 1 || slotID > 2) {
		warning("Ignoring music stop for invalid slot %u", slotID);
		return false;
	}

	if (!_musicEnabled || !_soundSystemActive) {
		_activeMusicSlot = 0;
		return false;
	}

	if (_activeMusicSlot == slotID) {
		if (stopImmediately == 0) {
			// Binary: sets mode=2 (fade-out), step=fadeParam, volume=0
			_musicControlMode = 2;
			_musicControlParam = fadeParam;
			_musicControlVolume = 0;
		} else {
			// Binary: adlibStopMusic(), activeSlot=0 (no mode/param clear)
			_engine->getAdlib()->stopMusic();
			_activeMusicSlot = 0;
		}
	}
	return true;
}

bool Script::ScriptExecutor::scriptWaitForMusic() {
	if (_soundSystemActive && _musicEnabled) {
		_waitForMusicControl = true;
		endTimer();
		endBuffering(_lastOpcodeTriggeredSkip);
		return true;
	}
	return false;
}

bool Script::ScriptExecutor::scriptFreeMusicSlot() {
	const uint16 slotID = scriptReadValue16();
	if (slotID < 1 || slotID > 2) {
		warning("Ignoring music free for invalid slot %u", slotID);
		return false;
	}

	if (_activeMusicSlot == slotID) {
		if (_musicEnabled && _soundSystemActive) {
			_engine->getAdlib()->stopMusic();
		}
		_activeMusicSlot = 0;
	}
	_musicSlots[slotID - 1].clear();
	return true;
}

bool Script::ScriptExecutor::scriptGetObjectX() {
	// Retrieve object x and use A334 to save it to a script variable.
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring object X query for invalid object %d", objectID);
		return false;
	}
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring object X query for missing object %d", objectID);
		return false;
	}
	scriptSaveVariableHelper(object->_position.x);
	return true;
}

bool Script::ScriptExecutor::scriptGetObjectY() {
	// Retrieve object y and use A334 to save it to a script variable.
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring object Y query for invalid object %d", objectID);
		return false;
	}
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring object Y query for missing object %d", objectID);
		return false;
	}
	scriptSaveVariableHelper(object->_position.y);
	return true;
}

bool Script::ScriptExecutor::scriptGetObjectField8() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring object field query for invalid object %d", objectID);
		return false;
	}
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring object field query for missing object %d", objectID);
		return false;
	}
	scriptSaveVariableHelper(object->_verticalOffsetScale);
	return true;
}

bool Script::ScriptExecutor::scriptGetObjectOrientation() {
	// Retrieve object orientation and use A334 to save it to a script variable.
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring object orientation query for invalid object %d", objectID);
		return false;
	}
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring object orientation query for missing object %d", objectID);
		return false;
	}
	scriptSaveVariableHelper(object->_orientation);
	return true;
}

void Script::ScriptExecutor::scriptClearActorInventory() {
	for (GameObject *object : GameObjects::instance()._objects) {
		if (object != nullptr && object->_sceneIndex == Scenes::instance()._currentActorIndex + 0x400) {
			object->_sceneIndex = 0;
		}
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr && currentView->_inventorySource != nullptr) {
		currentView->setInventorySource(currentView->_inventorySource);
		if (currentView->_activeInventoryItem != nullptr &&
			currentView->_activeInventoryItem->_sceneIndex != currentView->_inventorySource->_index + 0x400) {
			currentView->_activeInventoryItem = nullptr;
		}
	}
}

bool Script::ScriptExecutor::scriptSetPathfindingRemap() {
	// scriptSetPathfindingRemap (1008:dafb). Writes to scene+value*5+0x4EA8.
	const uint16 sourceValue = scriptReadValue16();
	const uint16 targetValue = scriptReadValue16();
	if (sourceValue < AREA_OVERRIDE_MIN || sourceValue > AREA_OVERRIDE_MAX ||
		targetValue < AREA_OVERRIDE_MIN || targetValue > AREA_OVERRIDE_MAX) {
		warning("Ignoring area remap %.4x -> %.4x outside valid range", sourceValue, targetValue);
		return false;
	}
	g_engine->_areaOverrides[sourceValue - AREA_OVERRIDE_MIN] = targetValue;
	return true;
}

bool Script::ScriptExecutor::scriptWaitForAdlib() {
	if (_soundSystemActive && _musicEnabled) {
		_waitForAdlibReady = true;
		endTimer();
		endBuffering(_lastOpcodeTriggeredSkip);
		return true;
	}
	return false;
}

void Script::ScriptExecutor::scriptFreePcmSound() {
	if (_soundEnabled)
		_engine->stopCurrentSound();
	_engine->clearCurrentSoundData();
}

ExecutionResult Script::ScriptExecutor::executeScript() {
	debugC(kDebugScript, "----- Scripting function entered - scene: %.2x 1014: %.2x 1012: %.2x", Scenes::instance()._currentSceneIndex, _isSceneInitRun, _repeatRunFlag);
	_isRunningScript = true;
	// Confirmed: no interrupt mechanism exists. Wait states (frameWait, walkTarget,
	// pcmSound, musicControl, adlibReady) are resolved by gameTick externally.
	_isAwaitingCallback = false;

	_requestCallback = false;

	// We use this to keep track of cases where we did not read all information as we should have
	_expectedEndLocation = _stream->pos();
	// The loop comprises the first labels in the file
	for (;;) {
		// TODO: Just for breaking out at the moment when end conditions fail to work
		if (_stream->eos()) {
			break;
		}
		// TODO: Probably only one of these is necessary
		if (_stream->size() == 0 || _stream->pos() >= _stream->size() - 1) {
			break;
		}

		// Make sure we have read all the bytes we should have read
		// TODO: Think about if we should also check this on exiting the function,
		// maybe we miss some cases like this
		if (_stream->pos() != _expectedEndLocation) {
			warning("Macs2::ScriptExecutor::ExecuteScript resyncing stream from %u to %u",
					(uint32)_expectedEndLocation, (uint32)_stream->pos());
			_expectedEndLocation = _stream->pos();
		}

		// Read an opcode and length
#ifdef USE_IMGUI
		if (scriptDebuggerShouldPause()) {
			_debugPaused = true;
			_state = ExecutorState::Executing; // preserve state for resume
			return ExecutionResult::WaitingForCallback;
		}
#endif
		byte opcode1 = readByte(); // [bp - 1h]
		// TODO: For the sake of easier reading the logs, jumping out if we
		// read a 0 opcode.
		if (opcode1 == 0x00) {
			// Account for the missing byte from reading opcode 0
			_expectedEndLocation++;
			continue;
		}
		Common::String opcodeInfo;
		if (opcode1 != 0x5) {
			opcodeInfo = identifyScriptOpcode(opcode1, 0);
		}
		debugC(kDebugScript, "- First block opcode: %.2x %s", opcode1, opcodeInfo.c_str());
		byte length = readByte(); // [bp-2h]
		_expectedEndLocation += length + 2;

		// TODO: convert this into a function lookup table and extract all opcode handling into separate functions, this is just for easier reading
		if (opcode1 == 0x01) {
			scriptSetVar();
		} else if (opcode1 == 0x02) {
			scriptSetVarOr();
		} else if (opcode1 == 0x03) {
			scriptIfFalse();
		} else if (opcode1 == 0x04) {
			scriptIfTrue();
		} else if (opcode1 == 0x5) {
			if (!scriptCompare()) {
				endBuffering(_lastOpcodeTriggeredSkip);
				break;
			}
		} else if (opcode1 == 0x06) {
			scriptIfInteraction();
		} else if (opcode1 == 0x07) {
			scriptEndIf();
		} else if (opcode1 == 0x08) {
			scriptElse();
		} else if (opcode1 == 0x10) {
			if (!scriptWalkToPosition()) {
				continue;
			}
		} else if (opcode1 == 0x11) {
			return scriptWaitForWalk();
		} else if (opcode1 == 0x13) {
			scriptSkipUntil14();
		} else if (opcode1 == 0x14) {
			scriptSkipWord();
		} else if (opcode1 == 0x0a) {
			scriptPrintStringLeft();
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x15) {
			scriptClearDialogueChoices();
		} else if (opcode1 == 0x16) {
			scriptAddDialogueChoice();
		} else if (opcode1 == 0x17) {
			return scriptShowDialogueChoice();
		} else if (opcode1 == 0x18) {
			return scriptDismissPanel();
		} else if (opcode1 == 0x19) {
			if (scriptWalkToAndPickup()) {
				return ExecutionResult::WaitingForCallback;
			}
			continue;
		} else if (opcode1 == 0x1a) {
			if (!scriptSetPickupFrames()) {
				continue;
			}
		} else if (opcode1 == 0x1b) {
			if (!scriptSetupObject()) {
				continue;
			}
		} else if (opcode1 == 0x1c) {
			scriptSetSkippable();
		} else if (opcode1 == 0x1d) {
			scriptClearSkippable();
		} else if (opcode1 == 0x1e) {
			if (!scriptPlayAnimation()) {
				continue;
			}
		} else if (opcode1 == 0x1f) {
			scriptTestPathfinding();
		} else if (opcode1 == 0x20) {
			if (!scriptSetYOffset()) {
				continue;
			}
		} else if (opcode1 == 0x21) {
			if (!scriptSetMotion()) {
				continue;
			}
		} else if (opcode1 == 0x22) {
			if (!scriptSetOrientation()) {
				continue;
			}
		} else if (opcode1 == 0x23) {
			if (!scriptMoveToPosition()) {
				continue;
			}
		} else if (opcode1 == 0x24) {
			scriptAddValues();
		} else if (opcode1 == 0x25) {
			scriptSubValues();
		} else if (opcode1 == 0x26) {
			scriptLoadSpecialAnim();
		} else if (opcode1 == 0x27) {
			if (!scriptSetDirection()) {
				continue;
			}
		} else if (opcode1 == 0x28) {
			scriptStopAnimation();
		} else if (opcode1 == 0x29) {
			if (scriptOpenInventory()) {
				return ExecutionResult::WaitingForCallback;
			}
			continue;
		} else if (opcode1 == 0x0b) {
			if (!scriptMoveObject()) {
				continue;
			}
		} else if (opcode1 == 0x0c) {
			return scriptChangeScene();
		} else if (opcode1 == 0x0d) {
			return scriptShowDialogue();
		} else if (opcode1 == 0x0E) {
			scriptChangeAnimation();
		} else if (opcode1 == 0x0F) {
			scriptFrameWait();
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x12) {
			scriptSetPathfinding();
		} else if (opcode1 == 0x2A) {
			scriptLoadObjectAnim();
		} else if (opcode1 == 0x2B) {
			if (!scriptCheckObjectData()) {
				continue;
			}
		} else if (opcode1 == 0x2C) {
			if (!scriptCheckInventory()) {
				continue;
			}
		} else if (opcode1 == 0x2D) {
			if (!scriptSetSnapToTarget()) {
				continue;
			}
		} else if (opcode1 == 0x2E) {
			scriptTestSceneAnimFrame();
		} else if (opcode1 == 0x2F) {
			if (!scriptTestObjectAnimFrame()) {
				continue;
			}
		} else if (opcode1 == 0x30) {
			scriptPrintStringRight();
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x31) {
			scriptSetVolume();
		} else if (opcode1 == 0x32) {
			if (!scriptSetObjectClickable()) {
				continue;
			}
		} else if (opcode1 == 0x33) {
			if (!scriptSetObjectVisible()) {
				continue;
			}
		} else if (opcode1 == 0x34) {
			if (!scriptSetHotspotOverride()) {
				continue;
			}
		} else if (opcode1 == 0x35) {
			if (!scriptSetObjectBounds()) {
				continue;
			}
		} else if (opcode1 == 0x36) {
			scriptDismissAllPanels();
		} else if (opcode1 == 0x37) {
			scriptResetToSceneScript();
		} else if (opcode1 == 0x38) {
			scriptLoadOverlayFont();
		} else if (opcode1 == 0x39) {
			// scriptEndOverlayText (1008:d80f). Clears the overlay text stage.
			scriptEndOverlayText();
		} else if (opcode1 == 0x3A) {
			const OpcodeControlFlow controlFlow = scriptAddOverlayTextEntry();
			if (controlFlow == OpcodeControlFlow::Continue) {
				continue;
			}
			if (controlFlow == OpcodeControlFlow::ScriptFinished) {
				return ExecutionResult::ScriptFinished;
			}
		} else if (opcode1 == 0x3B) {
			scriptClearOverlayText();
		} else if (opcode1 == 0x3C) {
			scriptFadeToBlack();
		} else if (opcode1 == 0x3D) {
			scriptFadeFromBlack();
		} else if (opcode1 == 0x3E) {
			if (!scriptLoadPcmSound()) {
				continue;
			}
		} else if (opcode1 == 0x3F) {
			scriptFreePcmSound();
		} else if (opcode1 == 0x40) {
			if (!scriptPlayPcmSound()) {
				continue;
			}
		} else if (opcode1 == 0x41) {
			if (scriptWaitForSound()) {
				return ExecutionResult::WaitingForCallback;
			}
		} else if (opcode1 == 0x42) {
			scriptStopPcmSound();
		} else if (opcode1 == 0x43) {
			if (!scriptLoadMusicSlot()) {
				continue;
			}
		} else if (opcode1 == 0x44) {
			if (!scriptPlayMusicSlot()) {
				continue;
			}
		} else if (opcode1 == 0x45) {
			if (!scriptStopMusicSlot()) {
				continue;
			}
		} else if (opcode1 == 0x47) {
			if (scriptWaitForMusic()) {
				return ExecutionResult::WaitingForCallback;
			}
		} else if (opcode1 == 0x46) {
			if (!scriptFreeMusicSlot()) {
				continue;
			}
		} else if (opcode1 == 0x48) {
			if (!scriptGetObjectX()) {
				continue;
			}
		} else if (opcode1 == 0x49) {
			if (!scriptGetObjectY()) {
				continue;
			}
		} else if (opcode1 == 0x4A) {
			if (!scriptGetObjectField8()) {
				continue;
			}
		} else if (opcode1 == 0x4B) {
			if (!scriptGetObjectOrientation()) {
				continue;
			}
		} else if (opcode1 == 0x4C) {
			scriptClearActorInventory();
		} else if (opcode1 == 0x4D) {
			if (!scriptSetPathfindingRemap()) {
				continue;
			}
		} else if (opcode1 == 0x4E) {
			if (scriptWaitForAdlib()) {
				return ExecutionResult::WaitingForCallback;
			}
		} else {
			scriptUnimplementedOpcode("Main", opcode1);
			endBuffering(_lastOpcodeTriggeredSkip);
			break;
		}
		endBuffering(_lastOpcodeTriggeredSkip);
	}
	_isRunningScript = false;
	debugC(kDebugScript, "----- Scripting function left");
	return ExecutionResult::ScriptFinished;
}

void ScriptExecutor::run(bool firstRun) {
	// Scene initialization run
	// TODO: Need to better encapsulate this down the road
	// TODO: Not sure which order is really right, need to check in SIS logs
	const bool resumingAfterCallback = (_state == ExecutorState::WaitingForCallback) && !firstRun;
	if (!resumingAfterCallback) {
		// TODO: Not sure if this is the right place and condition to reset this
		// variable. Context here is that we might have an object that triggers several
		// description strings in a row, and we would disable the executing object
		// if we always reset this object
		// TODO: Watch out for issues caused by this
		_executingScriptObjectID = 0;
		_repeatRunFlag = false;
		_isSceneInitRun = firstRun;
	}
	_state = ExecutorState::Executing;
	step();
}

void ScriptExecutor::setScript(Common::MemoryReadStream *stream) {
	_stream = stream;
}

void ScriptExecutor::setCurrentSceneScriptAt(uint32 offset) {
	setScript(Scenes::instance()._currentSceneScript);
	_stream->seek(offset, SEEK_SET);
}

void ScriptExecutor::tick() {
	if (_musicControlMode != 0 && _activeMusicSlot != 0) {
		const uint16 step = MAX<uint16>(_musicControlParam, 1);
		if (_musicControlMode == 1) {
			_musicControlVolume = (_musicControlVolume > step) ? _musicControlVolume - step : 0;
			_engine->getAdlib()->setVolume(_engine->scaledMusicVolume(_musicControlVolume));
			if (_musicControlVolume == 0) {
				_musicControlMode = 0;
			}
		} else {
			const uint16 nextVolume = MIN<uint16>(_musicControlVolume + step, 0x3F);
			_musicControlVolume = nextVolume;
			if (_musicControlVolume < 0x3F) {
				_engine->getAdlib()->setVolume(_engine->scaledMusicVolume(_musicControlVolume));
			} else {
				_musicControlMode = 0;
				_activeMusicSlot = 0;
				_engine->getAdlib()->stopMusic();
			}
		}
	}

	if (_waitForSoundPlayback) {
		if (!_engine->isCurrentSoundPlaying()) {
			_waitForSoundPlayback = false;
			run();
		} else {
			debugC(kDebugScript, "Waiting for sound playback to finish (handle active)");
		}
		return;
	}

	if (_waitForMusicControl) {
		if (_musicControlMode == 0) {
			_waitForMusicControl = false;
			run();
		}
		return;
	}

	if (_waitForAdlibReady) {
		if (_engine->getAdlib()->isPlaybackReady()) {
			_waitForAdlibReady = false;
			run();
		}
		return;
	}

	if (_debugPaused) {
#ifdef USE_IMGUI
		if (!_scriptDebugPaused || _scriptDebugStepRequested) {
			_debugPaused = false;
			run();
		}
#else
		_debugPaused = false;
#endif
		return;
	}

	if (_isFrameWaitActive) {
		if (_frameWaitTicksRemaining > 0) {
			--_frameWaitTicksRemaining;
		}
		if (_frameWaitTicksRemaining == 0) {
			_isFrameWaitActive = false;
			run();
		}
	}

	if (_isTimerActive) {
		if (g_engine->currentMillis > _timerEndMillis) {
			_isTimerActive = false;
			// TODO: Think about if this is the right way of executing it, or maybe we rather need
			// to use Execute
			run();
		}
	}
}

void ScriptExecutor::startTimer(uint32 duration) {
	_isTimerActive = true;
	_timerEndMillis = g_engine->currentMillis + duration;
}

void ScriptExecutor::endTimer() {
	_isTimerActive = false;
}

void ScriptExecutor::startFrameWait(uint16 duration) {
	_isFrameWaitActive = true;
	_frameWaitTicksRemaining = duration;
}

void ScriptExecutor::endFrameWait() {
	_isFrameWaitActive = false;
	_frameWaitTicksRemaining = 0;
}

void ScriptExecutor::rewind() {
	_stream->seek(0);
}

uint32 ScriptExecutor::getScriptPosition() const {
	return _stream ? (uint32)_stream->pos() : 0;
}

uint32 ScriptExecutor::getScriptEndPosition() const {
	return _stream ? (uint32)_stream->size() : 0;
}

uint32 ScriptExecutor::getVariableValue(int index) const {
	if (index < 0 || index >= (int)_variables.size())
		return 0;
	return _variables[index].a | ((uint32)_variables[index].b << 16);
}

uint32 ScriptExecutor::getSpecialValue(uint16 value) {
	uint16 out1 = 0;
	uint16 out2 = 0;
	switch (value) {
	case 0x01:
		if (_mouseMode == MouseMode::Use) {
			out1 = _interactedObjectID;
		} else if (_mouseMode == MouseMode::UseInventory) {
			out1 = _interactedObjectID | (_interactedOtherObjectID << 16);
			out2 = _interactedOtherObjectID;
		}
		break;
	case 0x02:
		out1 = (_mouseMode == MouseMode::Look) ? _interactedObjectID : 0;
		break;
	case 0x03:
		out1 = (_mouseMode == MouseMode::Talk) ? _interactedObjectID : 0;
		break;
	case 0x04: {
		const Common::Point &charPos = getCharPosition();
		out1 = getAreaAtPoint(charPos.x, charPos.y);
		break;
	}
	case 0x05:
		break;
	case 0x06:
		out1 = 1;
		break;
	case 0x07:
	case 0x08:
	case 0x09:
		out1 = 0;
		break;
	case 0x0A:
		out1 = 1;
		break;
	case 0x0B:
		out1 = _repeatRunFlag ? 1 : 0;
		break;
	case 0x0C:
		out1 = 1;
		break;
	case 0x0D:
		out1 = _chosenDialogueOption;
		break;
	case 0x23:
		out1 = _pathWalkableResult ? 1 : 0;
		break;
	case 0x24: {
		const GameObject *actor = GameObjects::instance().getObjectByIndex(Scenes::instance()._currentActorIndex);
		out1 = actor ? actor->_position.x : 0;
		break;
	}
	case 0x25: {
		const GameObject *actor = GameObjects::instance().getObjectByIndex(Scenes::instance()._currentActorIndex);
		out1 = actor ? actor->_position.y : 0;
		break;
	}
	case 0x26:
		out1 = _isSceneInitRun ? 1 : 0;
		break;
	case 0x27: {
		if (_isRepeatRun) {
			const Common::Point &charPos = getCharPosition();
			out1 = getAreaAtPoint(charPos.x, charPos.y);
		}
		break;
	}
	case 0x28:
		out1 = _inventoryCheckResult ? 1 : 0;
		break;
	case 0x29:
		out1 = _animBlobRangeTestResult ? 1 : 0;
		break;
	case 0x2A: {
		View1 *v = (View1 *)_engine->findView("View1");
		// Binary: g_wUiPanelState != 0 (any panel open: action bar, inventory, dialogue, help)
		const bool uiOpen = v != nullptr && (v->_uiPanelState != View1::kUiPanelNone || v->_currentMode == ViewMode::VM_HELP);
		out1 = (_inventoryCombineFlag && !uiOpen) ? 1 : 0;
		break;
	}
	case 0x2B: {
		View1 *v = (View1 *)_engine->findView("View1");
		// Binary: g_wUiPanelState != 0 (any panel open: action bar, inventory, dialogue, help)
		const bool uiOpen = v != nullptr && (v->_uiPanelState != View1::kUiPanelNone || v->_currentMode == ViewMode::VM_HELP);
		out1 = (_inventoryActionFlag && !uiOpen) ? 1 : 0;
		break;
	}
	case 0x2C:
		out1 = (_mouseMode == MouseMode::PanelUse) ? _interactedObjectID : 0;
		break;
	case 0x2D:
		out1 = Scenes::instance()._currentSceneIndex;
		break;
	case 0x2E:
		out1 = 2;
		break;
	case 0x2F:
		out1 = Scenes::instance()._lastSceneIndex;
		break;
	case 0x30:
		out1 = (_musicEnabled && _soundSystemActive) ? 1 : 0;
		break;
	case 0x31:
		out1 = (_soundEnabled && _soundSystemActive) ? 1 : 0;
		break;
	default:
		if (value >= 0x0E && value <= 0x22) {
			out1 = value - 0x0D;
		} else {
			warning("getSpecialValue: unknown special value 0x%02x", value);
		}
		break;
	}
	return out1 | ((uint32)out2 << 16);
}

} // namespace Script

} // namespace Macs2
