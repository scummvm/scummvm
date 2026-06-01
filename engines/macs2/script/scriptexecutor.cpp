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
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
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

#define ScriptNoEntry debug("Unhandled case in script handling.");
#define STR_HELPER(x) #x

ScriptExecutor::ScriptExecutor() {
	constexpr int numVariables = 1000;
	_variables.resize(numVariables);
	for (int i = 0; i < numVariables; i++) {
		_variables[i].a = 0;
		_variables[i].b = 0;
	}
}

Common::String ScriptExecutor::IdentifyScriptOpcode(uint8 opcode, uint8 opcode2) {
	if (opcode == 0x5)
		return Common::String::format("(%.2x)", opcode);

	return Common::String::format("(%.2x %.2x)", opcode, opcode2);
}

Common::String ScriptExecutor::IdentifyHelperOpcode(uint8 opcode, uint16 value) {
	return Common::String::format("(%.2x %.4x)", opcode, value);
}

inline void ScriptExecutor::scriptSkipBlock() {
	lastOpcodeTriggeredSkip = true;
	if (DebugMan.isDebugChannelEnabled(DebugFlag::DEBUG_SV)) {
		debug("-- Entering A3D2");
	} else {
		debug("-- Skipping using A3D2");
	}

	isSkipping = true;
	if (expectedEndLocation != _stream->pos()) {
		warning("Macs2::ScriptExecutor::scriptSkipBlock resyncing stream from %u to %u",
				(uint32)expectedEndLocation, (uint32)_stream->pos());
		expectedEndLocation = _stream->pos();
	}
	int skipDepth = 1;
	while ((skipDepth != 0) && (_stream->pos() < _stream->size())) {
		const byte opcode = ReadByte();
		if (_stream->pos() >= _stream->size()) {
			debugC(DEBUG_SV, "- A3D2 hit end of stream after opcode %.2x [%d]", opcode, skipDepth);
			break;
		}
		const byte length = ReadByte();
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
			debugC(DEBUG_SV, "- A3D2 clamping truncated block: opcode %.2x length %u remaining %lld [%d]",
				   opcode, length, (long long)remainingBytes, skipDepth);
			_stream->seek(_stream->size(), SEEK_SET);
			break;
		}

		_stream->seek(length, SEEK_CUR);
		debugC(DEBUG_SV, "- A3D2 skipping %u bytes for opcode %.2x [%d]", length, opcode, skipDepth);
	}

	if (skipDepth != 0) {
		debugC(DEBUG_SV, "- A3D2 left skip block early at %lld/%lld [%d]",
			   (long long)_stream->pos(), (long long)_stream->size(), skipDepth);
	}

	// Fix up the expected location after skipping
	expectedEndLocation = _stream->pos();
	if (DebugMan.isDebugChannelEnabled(DebugFlag::DEBUG_SV)) {
		debug("-- Leaving A3D2");
	}
	isSkipping = false;
}

void ScriptExecutor::scriptSkipAlternate() {
	if (DebugMan.isDebugChannelEnabled(DebugFlag::DEBUG_SV)) {
		debug("-- Entering A37A");
	} else {
		debug("-- Skipping using A37A");
	}

	isSkipping = true;
	if (expectedEndLocation != _stream->pos()) {
		warning("Macs2::ScriptExecutor::scriptSkipAlternate resyncing stream from %u to %u",
				(uint32)expectedEndLocation, (uint32)_stream->pos());
		expectedEndLocation = _stream->pos();
	}
	int skipDepth = 1;
	while ((skipDepth != 0) && (_stream->pos() < _stream->size())) {
		const byte opcode = ReadByte();
		if (_stream->pos() >= _stream->size()) {
			debugC(DEBUG_SV, "- A37A hit end of stream after opcode %.2x [%d]", opcode, skipDepth);
			break;
		}
		const byte length = ReadByte();
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
			debugC(DEBUG_SV, "- A37A clamping truncated block: opcode %.2x length %u remaining %lld [%d]",
				   opcode, length, (long long)remainingBytes, skipDepth);
			_stream->seek(_stream->size(), SEEK_SET);
			break;
		}

		_stream->seek(length, SEEK_CUR);
		debugC(DEBUG_SV, "- A37A skipping %u bytes for opcode %.2x [%d]", length, opcode, skipDepth);
	}

	if (skipDepth != 0) {
		debugC(DEBUG_SV, "- A37A left skip block early at %lld/%lld [%d]",
			   (long long)_stream->pos(), (long long)_stream->size(), skipDepth);
	}

	// Fix up the expected location after skipping
	expectedEndLocation = _stream->pos();
	if (DebugMan.isDebugChannelEnabled(DebugFlag::DEBUG_SV)) {
		debug("-- Leaving A37A");
	}
	isSkipping = false;
}

bool ScriptExecutor::skipToEndOfSkippableSection() {
	// Button 8 skip from handleInput (1008:e8bf):
	// Reads opcode+length pairs, advances stream by length bytes,
	// until opcode 0x1D is found or end of stream.
	while (_stream->pos() < _stream->size() - 1) {
		uint8 opcode = ReadByte();
		uint8 length = ReadByte();
		if (opcode == 0x1D) {
			scriptSkippable = false;
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

	byte type = ReadByte();
	uint16 value = ReadWord();

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
		const Common::Point &charPos = GetCharPosition();
		out1 = getAreaAtPoint(charPos.x, charPos.y);
		break;
	}
	case 0x05:
		break; // no-op
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
		out1 = repeatRunFlag ? 1 : 0;
		break;
	case 0x0C:
		out1 = 1;
		break;
	case 0x0D:
		out1 = chosenDialogueOption;
		break;
	case 0x23:
		out1 = pathWalkableResult ? 1 : 0;
		break;
	case 0x24: {
		const GameObject *actor = GameObjects::instance().GetObjectByIndex(Scenes::instance().CurrentActorIndex);
		out1 = actor ? actor->Position.x : 0;
		break;
	}
	case 0x25: {
		const GameObject *actor = GameObjects::instance().GetObjectByIndex(Scenes::instance().CurrentActorIndex);
		out1 = actor ? actor->Position.y : 0;
		break;
	}
	case 0x26:
		out1 = IsSceneInitRun ? 1 : 0;
		break;
	case 0x27: {
		if (isRepeatRun) {
			const Common::Point &charPos = GetCharPosition();
			out1 = getAreaAtPoint(charPos.x, charPos.y);
		}
		break;
	}
	case 0x28:
		out1 = inventoryCheckResult ? 1 : 0;
		break;
	case 0x29:
		out1 = animBlobRangeTestResult ? 1 : 0;
		break;
	case 0x2A: {
		View1 *v = (View1 *)_engine->findView("View1");
		const bool uiOpen = v != nullptr && (v->_isShowingInventory || v->_isShowingStringBox || v->isShowingMainMenu);
		out1 = (inventoryCombineFlag && !uiOpen) ? 1 : 0;
		break;
	}
	case 0x2B: {
		View1 *v = (View1 *)_engine->findView("View1");
		const bool uiOpen = v != nullptr && (v->_isShowingInventory || v->_isShowingStringBox || v->isShowingMainMenu);
		out1 = (inventoryActionFlag && !uiOpen) ? 1 : 0;
		break;
	}
	case 0x2C:
		out1 = (_mouseMode == MouseMode::PanelUse) ? _interactedObjectID : 0;
		break;
	case 0x2D:
		out1 = Scenes::instance().CurrentSceneIndex;
		break;
	case 0x2E:
		out1 = 2;
		break;
	case 0x2F:
		out1 = Scenes::instance().LastSceneIndex;
		break;
	case 0x30:
		out1 = (musicEnabled && soundSystemActive) ? 1 : 0;
		break;
	case 0x31:
		out1 = (soundEnabled && soundSystemActive) ? 1 : 0;
		break;
	default:
		if (value >= 0x0E && value <= 0x22) {
			out1 = value - 0x0D;
		} else {
			warning("scriptReadValuePair: unknown special value 0x%02x", value);
		}
		break;
	}
}

void ScriptExecutor::scriptReadValue_Placeholder() {
	scriptReadValue32();
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

void ScriptExecutor::scriptSaveVariable(uint32 value) {
	uint8 subOpcode = ReadByte();
	if (subOpcode == 0x00 || subOpcode == 0xFF) {
		warning("scriptSaveVariable: invalid sub-opcode 0x%02x (error 0x16)", subOpcode);
		return;
	}

	uint16 variableID = ReadWord();
	SetVariableValue(variableID, value);
}

void ScriptExecutor::scriptLoadSpecialAnimImpl() {

	uint16 objectID1;
	uint16 objectID2;
	scriptReadValuePair(objectID1, objectID2);
	// [bp-2h]
	// uint16 offset1 = objectID1 - 0x400;
	// Object IDs are always < 0x800, so subtracting 0x400 from low word is safe
	objectID1 -= 0x400;

	// {[bp-0Dh]
	uint16 out1;
	uint16 out2;
	scriptReadValuePair(out1, out2);
}

void ScriptExecutor::scriptStopAnimationImpl() {
	// scriptStopAnimation (1008:c8e4). Original behavior:
	//   1. Read objectID, validate
	//   2. Set runtime +0x22D = 0x7FFF (remove direction/frame limit)
	//   3. Free overload animation blob if loaded (runtime +0x183 flag)
	//   4. Clear overload flag
	uint32 characterID = scriptReadValue32() - 0x400;
	GameObject *obj = GameObjects::GetObjectByIndex(characterID);
	if (obj == nullptr) {
		warning("Ignoring stop animation for missing object %u", characterID);
		return;
	}
	obj->useOverloadAnimation = false;
	obj->overloadAnimation.clear();
}

void ScriptExecutor::scriptOpcode0x0E() {
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
	BackgroundAnimationBlob::getAnimFrameCount(blob.Blob);
	// TODO: We should be doing some checking on the result value

	// TODO: Do some comparison with [bp-4h]
	BackgroundAnimationBlob::advanceAnimFrame(blob.Blob, true, bp4 + 0x64);
}

uint16 ScriptExecutor::getAreaAtPoint(uint16 x, uint16 y) {
	// getAreaAtPoint (1008:101d). Reads the pathfinding map pixel and applies
	// the area override table at sceneData + value*5 + 0x4EA8.
	if (x >= 320 || y >= 200) {
		return 0;
	}
	uint16 result = _engine->_pathfindingMap.getPixel(x, y);
	if (result > 199 && result < 0xFA) {
		uint16 overrideValue = _engine->GetPathfindingOverride2(result);
		if (overrideValue > 199) {
			result = overrideValue;
		}
	}
	return result;
}

bool ScriptExecutor::IsPathWalkable(const Common::Point &from, const Common::Point &to) {
	// Exact reimplementation of isPathWalkable (1008:1196).
	// Traces from 'to' toward 'from'. Checks walkability only on major-axis steps.
	int16 x1 = from.x, y1 = from.y, x2 = to.x, y2 = to.y;
	uint16 error = 0;
	int16 curX = x2;
	int16 curY = y2;
	uint16 absDx = (uint16)abs((int)(x2 - x1));
	uint16 absDy = (uint16)abs((int)(y2 - y1));

	if (curX == x1 && curY == y1)
		return true;

	do {
		bool steppedX;
		if (error >= absDx) {
			if (y1 < y2)
				curY--;
			if (y1 > y2)
				curY++;
			error -= absDx;
			steppedX = false;
		} else {
			if (x1 < x2)
				curX--;
			if (x1 > x2)
				curX++;
			error += absDy;
			steppedX = true;
		}
		if (absDx > absDy && steppedX) {
			if (getAreaAtPoint(curX, curY) >= 0xC8)
				return false;
		}
		if (absDx <= absDy && !steppedX) {
			if (getAreaAtPoint(curX, curY) >= 0xC8)
				return false;
		}
	} while (curX != x1 || curY != y1);
	return true;
}

bool ScriptExecutor::loadIndexedResource(Common::Array<uint8> &outData, uint8 resourceIndex, uint16 objectTableOffset) {
	if (resourceIndex == 0) {
		warning("Ignoring resource load for zero resource index");
		return false;
	}

	const int64 oldPos = g_engine->_fileStream->pos();
	uint32 address = 0;

	if (_executingScriptObjectID == 0) {
		if (resourceIndex > _engine->array520D.size()) {
			warning("Ignoring resource load for missing scene resource %u", resourceIndex);
			return false;
		}
		address = _engine->array520D[resourceIndex - 1];
	} else {
		GameObject *object = GameObjects::GetObjectByIndex(_executingScriptObjectID);
		if (object == nullptr || object->DataOffset == 0) {
			warning("Ignoring resource load for missing object %u resource %u", _executingScriptObjectID, resourceIndex);
			return false;
		}
		g_engine->_fileStream->seek(object->DataOffset + objectTableOffset + (resourceIndex - 1) * 4, SEEK_SET);
		address = g_engine->_fileStream->readUint32LE();
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

void ScriptExecutor::ScriptPrintString(bool alignRight) {
	// TODO: Labels above not handled yet
	// TODO: Lots of details not handled
	// l0037_A94E:

	uint16 x = scriptReadValue16();
	uint16 y = scriptReadValue16();
	// TODO: Several globals writes around this code
	uint16 bp2 = ReadWord();
	uint16 bp4 = ReadWord();

	// TODO: Implement naive string printing here, refine later

	Common::StringArray strings;
	if (_executingScriptObjectID == 0) {
		strings = g_engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, bp2, bp4);
	} else {
		Common::MemoryReadStream *s = GameObjects::ReadGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		strings = g_engine->DecodeStrings(s, bp2, bp4);
	}

	if (alignRight) {
		x -= g_engine->MeasureStrings(strings) + 0x12;
	}

	// TODO: Look for good pattern for the view, this feels like it is not intended this way
	View1 *currentView = (View1 *)_engine->findView("View1");
	currentView->setStringBoxAt(strings, Common::Point(x, y));
}

void ScriptExecutor::BeginBuffering() {
	lastOpcodeTriggeredSkip = false;
	debugBuffer.clear();
}

void ScriptExecutor::EndBuffering(bool shouldMark) {
	(void)shouldMark;
	lastOpcodeTriggeredSkip = false;
	debugBuffer.clear();
}

void ScriptExecutor::SetVariableValue(uint16 index, uint16 a, uint16 b) {
	_variables[index].a = a;
	_variables[index].b = b;
}

void ScriptExecutor::SetVariableValue(uint16 index, uint32 value) {
	uint16 a = static_cast<uint16>(value >> 16);    // High 16 bits
	uint16 b = static_cast<uint16>(value & 0xFFFF); // Low 16 bits
	SetVariableValue(index, b, a);
}

Common::Point ScriptExecutor::GetCharPosition() {
	const GameObject *actor = GameObjects::instance().GetObjectByIndex(Scenes::instance().CurrentActorIndex);
	if (!actor)
		actor = GameObjects::GetProtagonistObject();
	return actor ? actor->Position : Common::Point();
}

void ScriptExecutor::DumpWholeScript() {
	// TODO: Probably should not hard code this, with this in place, the
	// variable for saving the old position is superfluous
	SetCurrentSceneScriptAt(0);
	_streamDumpPosition = 0;
	expectedEndLocation = _stream->pos();

	// Disable buffering
	lastOpcodeTriggeredSkip = false;

	// Keep track of the depth of the skipping
	uint16 skipValue = 0;

	// The loop comprises the first labels in the file
	// l0037_DB73:
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
		if (_stream->pos() != expectedEndLocation) {
			warning("Macs2::ScriptExecutor::DumpWholeScript resyncing stream from %u to %u",
					(uint32)expectedEndLocation, (uint32)_stream->pos());
			expectedEndLocation = _stream->pos();
		}

		// Read an opcode and length
		byte opcode1 = ReadByte(); // [bp - 1h]
		// TODO: For the sake of easier reading the logs, jumping out if we
		// read a 0 opcode.
		if (opcode1 == 0x00) {
			continue;
		}
		Common::String opcodeInfo;
		if (opcode1 != 0x5) {
			opcodeInfo = IdentifyScriptOpcode(opcode1, 0);
		}
		debug("[%u] - First block opcode: %.2x %s", skipValue, opcode1, opcodeInfo.c_str());
		byte length = ReadByte(); // [bp-2h]
		expectedEndLocation += length + 2;

		if (opcode1 == 0x04) {
			uint16 result1;
			uint16 result2;
			scriptReadValuePair(result1, result2);
		} else if (opcode1 == 0x5) {
			// l0037_DC66:
			// [bp-3h]
			uint8 opcode2 = ReadByte();
			opcodeInfo = IdentifyScriptOpcode(opcode1, opcode2);
			debug("[%u] - Second block opcode: %.2x %s", skipValue, opcode2, opcodeInfo.c_str());
			// [bp-7h]
			uint16 v1;
			// [bp-5h]
			uint16 v2;
			scriptReadValuePair(v1, v2);
			// [bp-0Bh]
			uint16 v3;
			// [bp-9h]
			uint16 v4;
			scriptReadValuePair(v3, v4);
		}

		if (opcode1 >= 3) {
			if (opcode1 <= 6) {
				skipValue++;
			}
		}
		if (opcode1 == 8) {
			if (skipValue == 1) {
				skipValue--;
			}
		}
		if (opcode1 == 7) {
			skipValue--;
		}

		if (opcode1 == 0x0d) {
			// Show a dialogue option
			uint32 objectID = scriptReadValue32() - 0x400;
			debug("Object ID of speaker: %.4x.\n", objectID);
			scriptReadValue16(); // x
			scriptReadValue16(); // y
			scriptReadValue16(); // side
			uint32 offset = ReadWord();
			uint32 numLines = ReadWord();

			// TODO: We are assuming that we are dumping the scene script, if not,
			// we would have to check for the executing object as well
			Common::Array<Common::String> strings;
			strings = g_engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, offset, numLines);

			for (Common::String &currentLine : strings) {
				debug("String: %s", currentLine.c_str());
			}
		}
		_stream->seek(expectedEndLocation);
		EndBuffering(false);
	}
	_stream->seek(_streamDumpPosition, SEEK_SET);
}

bool ScriptExecutor::IsRelevantObject(const GameObject *obj) {
	// Original logic (runScriptExecutor at 1008:e3e7):
	// An object is relevant if it has runtime data allocated (non-null pointer at object+0xa).
	// In ScummVM, this corresponds to having a non-empty Script array (the script data
	// lives in the runtime allocation at offset +0x187). The caller already checks Script.size(),
	// so we just need to confirm the object is initialized (has data offset set).
	return obj->DataOffset != 0;
}

void ScriptExecutor::Step() {
	bool shouldContinue = true;

	while (shouldContinue) {
		switch (_state) {
		case ExecutorState::Idle: {
			// TODO: Check if there is a scheduled run
			return;
		};
			break;
		case ExecutorState::Executing: {
			// Continue execution

			// Check if the currently executing script is at the end
			if (_stream->pos() == _stream->size()) {
				// Handle the next one potentially
				shouldContinue = LoadNextScript();
			} else {
				// Let the current script continue
				ExecutionResult result = ExecuteScript();
				if (result == ExecutionResult::WaitingForCallback) {
					// We need to change our state as well now
					_state = ExecutorState::WaitingForCallback;
					// Original: save cursor mode, then set to Disabled 0x1A (hourglass)
					if (_mouseMode != MouseMode::Disabled) {
						_cursorModeBeforeWait = _mouseMode;
						_engine->SetCursorMode(MouseMode::Disabled);
						View1 *v = (View1 *)_engine->findView("View1");
						if (v)
							v->UpdateCursor();
					}
					return;
				}
			}
		};
			break;
		case ExecutorState::WaitingForCallback: {
			// TODO: Check if this can even occur i.e. if we even schedule something or if
			// we always call the execute directly

		};
			break;
		}
	}
	// Rewind and reset to the scene script after we are done executing
	executingObjectIndex = Scenes::instance().CurrentSceneIndex;
	SetScript(Scenes::instance().CurrentSceneScript);
	if (_stream && _stream->size() > 0) {
		_stream->seek(0, SEEK_SET);
	}
	scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
	_state = ExecutorState::Idle;
	g_engine->_scriptExecutor->isRepeatRun = false;
	// Original: restore cursor from Disabled when all scripts finish
	if (_mouseMode == MouseMode::Disabled) {
		_engine->SetCursorMode(_cursorModeBeforeWait);
		View1 *v = (View1 *)_engine->findView("View1");
		if (v)
			v->UpdateCursor();
	}
}

bool ScriptExecutor::LoadNextScript() {
	// Confirmed from runScriptExecutor (1008:e3e7): after the scene script finishes,
	// iterate executingObjectId from 1 to 0x200, skipping objects with no runtime data.
	// Load each object's script from runtime+0x187/+0x189/+0x18B.

	if (scriptExecutionState == ScriptExecutionState::ExecutingSceneScript) {
		// If we are finished with executing the scene, we need to go over all relevant objects
		// The code below will increment to 1 to start at the protagonist
		executingObjectIndex = 0;
		scriptExecutionState = ScriptExecutionState::ExecutingOtherScripts;
	}

	// We always try to advance to the next object's script until we reach the end
	// of the objects list
	GameObject *candidateObject = nullptr;
	do {
		executingObjectIndex++;
		candidateObject = GameObjects::GetObjectByIndex(executingObjectIndex);

		// TODO: Check if this is a valid option
		if (candidateObject && IsRelevantObject(candidateObject)) {
			if (candidateObject->Script.size() != 0) {
				_stream = candidateObject->GetScriptStream();
				_executingScriptObjectID = candidateObject->Index;
				debug("----- Switching execution to script for object: %.4x", candidateObject->Index);
				return true;
			}
		}
	} while (candidateObject != nullptr);

	_executingScriptObjectID = 0;

	// We are done executing all relevant objects
	if (IsSceneInitRun) {
		// We need to start again at the scene object
		IsSceneInitRun = false;
		repeatRunFlag = true;
		executingObjectIndex = Scenes::instance().CurrentSceneIndex;
		_stream = Scenes::instance().CurrentSceneScript;
		if (!_stream || _stream->size() == 0) {
			return false;
		}
		_stream->seek(0, SEEK_SET);
		scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
		debug("----- Switching execution to script for scene: %.4x", executingObjectIndex);
		return true;
	}

	if (repeatRunFlag) {
		// We are done
		repeatRunFlag = false;
		return false;
	}

	return false;
};

byte Script::ScriptExecutor::ReadByte() {
	const int64 pos = _stream->pos();
	const byte result = _stream->readByte();
	// if (isSkipping) {
	//  TODO: This had the output channel active, to consider if I want to handle this separately
	// debugC(DEBUG_SV,"Script read (byte): %.2x at location %.4x", result, pos);
	//} else {
	debug("Script read (byte): %.2x at location %.4x", result, (uint32)pos);
	//}
	return result;
}

uint16 Script::ScriptExecutor::ReadWord() {
	const int64 pos = _stream->pos();
	const uint16 result = _stream->readUint16LE();
	debug("Script read (word): %.4x at location %.4x", result, (uint32)pos);
	return result;
}

void Script::ScriptExecutor::scriptOpcode0x01() {
	// This writes to a script variable.
	ReadByte();
	uint16 variableIndex = ReadWord();
	ScriptVariable var;
	scriptReadValuePair(var.a, var.b);
	_variables[variableIndex] = var;
}

void Script::ScriptExecutor::scriptOpcode0x02() {
	// Padding/type byte (same as opcode 0x01) - read and discarded
	ReadByte();
	uint16 variableIndex = ReadWord();
	// We skip the left shift and just read the first value directly
	uint16 throwaway;
	uint16 value1;
	scriptReadValuePair(throwaway, value1);
	uint16 value2;
	uint16 value3;
	scriptReadValuePair(value2, value3);
	value2 |= value1;
	value3 |= 0x00;
	SetVariableValue(variableIndex, value2, value3);
}

void Script::ScriptExecutor::scriptOpcode0x03() {
	uint16 res1;
	uint16 res2;
	scriptReadValuePair(res1, res2);
	expectedEndLocation = _stream->pos();
	if (res1 | res2) {
		scriptSkipBlock();
	}
	expectedEndLocation = _stream->pos();
}

void Script::ScriptExecutor::scriptOpcode0x04() {
	uint16 result1;
	uint16 result2;
	scriptReadValuePair(result1, result2);
	expectedEndLocation = _stream->pos();
	// If any bit is set in the result, we skip, otherwise we fall through and continue the loop
	if ((result1 | result2) == 0) {
		scriptSkipBlock();
	}
	expectedEndLocation = _stream->pos();
}

bool Script::ScriptExecutor::scriptOpcode0x05() {
	// Comparison opcode from executeOpcodes (1008:db56).
	// Reads a comparison sub-opcode, two 32-bit values (v1:v2 and v3:v4),
	// and skips the following block if the condition is NOT met.
	// Values are treated as signed 32-bit (v2:v1 = high:low).
	uint8 opcode2 = ReadByte();
	Common::String opcodeInfo = IdentifyScriptOpcode(0x5, opcode2);
	debug("- Second block opcode: %.2x %s", opcode2, opcodeInfo.c_str());
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
		ScriptUnimplementedOpcode_Main(opcode2);
		return false;
	}

	if (!conditionMet) {
		scriptSkipBlock();
	}

	return true;
}

void Script::ScriptExecutor::scriptOpcode0x06() {
	// "Use item on object" comparison from executeOpcodes (1008:db56).
	// Reads sub-opcode (1=match, 2=NOT match), then the interacted pair
	// and two comparison objects. Checks both orderings.
	uint8 subOpcode = ReadByte();
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

void Script::ScriptExecutor::scriptOpcode0x07() {
	// Opcode 0x07: no-op (confirmed: no handler in disassembly, falls through to loop).
	// It has no specific case handling code in the original.
}

void Script::ScriptExecutor::scriptOpcode0x08() {
	// This is some kind of skipping as well.
	scriptSkipAlternate();
}

void Script::ScriptExecutor::scriptOpcode0x09() {
	// ExecuteScript does not currently have a dedicated opcode 0x09 dispatch branch.
}

void Script::ScriptExecutor::scriptOpcode0x0A() {
	// l0037_DDE8:
	ScriptPrintString();
	// Ends execution (confirmed: jumps to e3bd in disassembly).
	EndBuffering(lastOpcodeTriggeredSkip);
}

bool Script::ScriptExecutor::scriptOpcode0x0B() {
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
	GameObject *object = GameObjects::GetObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Opcode 0x0B: missing object %u", objectID);
		return false;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	const uint16 currentScene = Scenes::instance().CurrentSceneIndex;
	const uint16 actorIndex = Scenes::instance().CurrentActorIndex;

	// Step 1: Remove from render list if object was visible in current scene.
	// Original checks: object in current scene, OR in protagonist's inventory,
	// OR inside a container that is in the current scene.
	if (objectID != actorIndex) {
		bool wasInCurrentScene = (object->SceneIndex == currentScene);
		if (!wasInCurrentScene && object->SceneIndex == actorIndex + 0x400) {
			wasInCurrentScene = true; // was in protagonist's inventory
		}
		if (!wasInCurrentScene && object->SceneIndex > 0x400) {
			GameObject *parent = GameObjects::GetObjectByIndex(object->SceneIndex - 0x400);
			if (parent != nullptr && parent->SceneIndex == currentScene) {
				wasInCurrentScene = true; // was in a container in current scene
			}
		}
		if (wasInCurrentScene) {
			Character *c = currentView->GetCharacterByIndex(objectID);
			if (c != nullptr) {
				int idx = currentView->GetCharacterArrayIndex(c);
				if (idx >= 0)
					currentView->characters.remove_at(idx);
			}
		}
	}

	// Step 2: Update object fields
	object->SceneIndex = sceneID;
	object->Position = Common::Point(x, y);

	// Step 3: Add to render list if object is now visible in current scene.
	if (objectID != actorIndex) {
		bool isInCurrentScene = (sceneID == currentScene);
		if (!isInCurrentScene && sceneID == actorIndex + 0x400) {
			isInCurrentScene = true; // now in protagonist's inventory
		}
		if (!isInCurrentScene && sceneID > 0x400) {
			GameObject *parent = GameObjects::GetObjectByIndex(sceneID - 0x400);
			if (parent != nullptr && parent->SceneIndex == currentScene) {
				isInCurrentScene = true; // now in a container in current scene
			}
		}
		if (isInCurrentScene && sceneID == currentScene) {
			// Add as character to render list
			Character *c = currentView->GetCharacterByIndex(objectID);
			if (c == nullptr) {
				c = new Character();
				c->GameObject = object;
				currentView->characters.push_back(c);
			}
			c->SetPosition(Common::Point(x, y));
		}
	}

	// Step 4: Update inventory tracking
	if (sceneID == actorIndex + 0x400) {
		// Moved into protagonist's inventory
		bool alreadyInInventory = false;
		for (auto item : currentView->inventoryItems) {
			if (item->Index == objectID) {
				alreadyInInventory = true;
				break;
			}
		}
		if (!alreadyInInventory)
			currentView->inventoryItems.push_back(object);
	} else {
		// Remove from inventory if it was there
		for (uint i = 0; i < currentView->inventoryItems.size(); i++) {
			if (currentView->inventoryItems[i]->Index == objectID) {
				currentView->inventoryItems.remove_at(i);
				break;
			}
		}
	}

	// Step 5: If object has no runtime data and was the UseInventory target,
	// reset cursor to Use (0x15)
	if (object->Blobs.empty()) {
		if (_interactedObjectID == objectID + 0x400 && _mouseMode == MouseMode::UseInventory) {
			_engine->SetCursorMode(MouseMode::Use);
			currentView->UpdateCursor();
		}
		// Original also rewrites the saved (pre-wait) cursor mode: 0x17 -> 0x15.
		if (_interactedObjectID == objectID + 0x400 && savedPickupMouseMode == MouseMode::UseInventory) {
			savedPickupMouseMode = MouseMode::Use;
		}
	}

	// Step 6: If moved object is the one whose script is currently executing,
	// terminate its script (original: sets scriptEndPosition=0, scriptPosition=0)
	if ((int)objectID == _executingScriptObjectID) {
		_stream->seek(0, SEEK_END);
	}

	return true;
}

ExecutionResult Script::ScriptExecutor::scriptOpcode0x0C() {
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
	g_engine->changeScene(newSceneID, false);
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr && transitionMode == 0 && transitionSpeed != 0) {
		currentView->startFadingWithSpeed(transitionSpeed);
	}
	// Confirmed: executeOpcodes jumps to end-execution path after scriptChangeScene
	// in the game code
	// Confirmed: scriptChangeScene resets interactedObjectID and interactedInventoryItemId
	// or if there is another mechanism for this
	_interactedObjectID = 0;
	_interactedOtherObjectID = 0;
	requestCallback = false;
	g_engine->ScheduleRun(true);
	isAwaitingCallback = true;
	// NOTE: EndTimer prevents race conditions from overlapping waits

	EndTimer();
	EndFrameWait();
	EndBuffering(lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

ExecutionResult Script::ScriptExecutor::scriptOpcode0x0D() {
	// Show a dialogue option.
	uint32 objectID = scriptReadValue32() - 0x400;
	uint16 x = scriptReadValue16();
	uint16 y = scriptReadValue16();
	uint16 side = scriptReadValue16();
	uint32 offset = ReadWord();
	uint32 numLines = ReadWord();

	View1 *currentView = (View1 *)_engine->findView("View1");

	Common::Array<Common::String> strings;
	if (_executingScriptObjectID == 0) {
		strings = g_engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, offset, numLines);
	} else {
		Common::MemoryReadStream *s = GameObjects::ReadGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		strings = g_engine->DecodeStrings(s, offset, numLines);
	}

	debugC(kDebugScript,
		   "Opcode 0D dialogue: speaker=%u rawPos=(%u,%u) side=%u textOffset=%u numLines=%u scriptObject=%u text=\"%s\"",
		   objectID, x, y, side, offset, numLines, _executingScriptObjectID, joinDebugStrings(strings).c_str());

	activeDialogueSpeakerObjectID = objectID;
	currentView->ShowSpeechAct(objectID, strings, Common::Point(x, y), side);
	isAwaitingCallback = true;
	// NOTE: EndTimer prevents race conditions from overlapping waits

	EndTimer();
	EndFrameWait();
	EndBuffering(lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

bool Script::ScriptExecutor::scriptOpcode0x10() {
	// scriptWalkToPosition (1008:b843). Uses pathfinding like the original:
	// checks direct walkability first, falls back to A* pathfinding.
	uint32 objectID = scriptReadValue32() - 0x400;
	int16 x = (int16)scriptReadValue16();
	int16 y = (int16)scriptReadValue16();

	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *c = currentView ? currentView->GetCharacterByIndex(objectID) : nullptr;
	if (c == nullptr) {
		warning("Ignoring walk-to for missing character %u", objectID);
		return false;
	}

	Common::Point target(x, y);
	Common::Point current = c->GetPosition();

	// Check if direct path is walkable (like isPathWalkable in the original)
	if (c->isPathWalkable(current, target)) {
		// Direct path is clear - just lerp straight there
		c->StartLerpTo(target, 1000);
	} else if (c->IsWalkable(target)) {
		// Target is walkable but no direct path - use A* pathfinding
		c->Path.clear();
		c->PathFinalDestination = target;
		if (c->calculatePath(target)) {
			c->CurrentPathIndex = -1;
			c->IsFollowingPath = c->walkAlongPath();
		} else {
			// Pathfinding failed - walk directly as fallback
			c->StartLerpTo(target, 1000);
		}
	} else {
		// Target is not walkable - set position directly (no movement)
		// Original: piVar11[0x16]=0, piVar11[0x17]=0, target=current
	}

	return true;
}

ExecutionResult Script::ScriptExecutor::scriptOpcode0x11() {
	// Wait for walk completion from executeOpcodes (1008:db56).
	// Original: validates object, checks runtime data exists, checks frozen flag,
	// sets g_wWalkTargetObjectIndex, hides cursor, returns to gameTick.
	// gameTick checks walk completion each frame.
	uint32 objectID = scriptReadValue32() - 0x400;
	if (objectID < 1 || objectID > 0x200) {
		warning("Opcode 0x11: invalid object %u", objectID);
		EndBuffering(lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	GameObject *walkObject = GameObjects::GetObjectByIndex(objectID);
	if (walkObject == nullptr) {
		warning("Opcode 0x11: missing object %u", objectID);
		EndBuffering(lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	// Original checks runtime+0x231 (frozen/attached flag) → error 0x1F
	if (walkObject->HasBoundsAttachment) {
		warning("Opcode 0x11: object %u is frozen (bounds attached)", objectID);
		EndBuffering(lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *c = currentView->GetCharacterByIndex(objectID);
	if (c == nullptr) {
		// Original: error code 2 (no runtime data). Script execution stops.
		warning("Opcode 0x11: no character for object %u (no runtime data)", objectID);
		EndBuffering(lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	c->RegisterWaitForMovementFinishedEvent();
	requestCallback = false;
	isAwaitingCallback = true;
	EndTimer();
	EndBuffering(lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

void Script::ScriptExecutor::scriptOpcode0x13() {
	uint16 tag = ReadWord();
	_stream->seek(0, SEEK_SET);
	while (_stream->pos() < _stream->size()) {
		uint8 opcode = ReadByte();
		uint8 length = ReadByte();
		if (opcode == 0x14) {
			uint16 tag14 = ReadWord();
			if (tag14 == tag) {
				return;
			}
		} else {
			_stream->seek(length, SEEK_CUR);
		}
	}
	expectedEndLocation = _stream->pos();
}

void Script::ScriptExecutor::scriptOpcode0x14() {
	// If we reach opcode 14 regularly, just discard the payload and continue.
	ReadWord();
}

void Script::ScriptExecutor::scriptOpcode0x15() {
	// Mark that we are gathering strings for setting up a dialogue choice.
	DialogueChoices.clear();
}

void Script::ScriptExecutor::scriptOpcode0x16() {
	// Add a dialogue choice.
	uint16 index = scriptReadValue16();
	// We don't save the index, instead we make sure that we add them in the right
	// order and use the array to keep track.
	// TODO: Removed this assert, during the dialogue in the beginning of chapter
	// 3 (at the fort) an index of 3 came up when only one item had been there before.
	// Not sure if the way of handling it still works or reflects the game, needs
	// to be tested.
	// assert(index - 1 == DialogueChoices.size());
	uint16 offset = ReadWord();
	uint16 numLines = ReadWord();
	Common::StringArray lines;
	if (_executingScriptObjectID == 0) {
		lines = _engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, offset, numLines);
	} else {
		Common::MemoryReadStream *stringsStream = GameObjects::ReadGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		lines = _engine->DecodeStrings(stringsStream, offset, numLines);
	}
	debugC(kDebugScript,
		   "Opcode 16 choice text: index=%u textOffset=%u numLines=%u scriptObject=%u text=\"%s\"",
		   index, offset, numLines, _executingScriptObjectID, joinDebugStrings(lines).c_str());
	DialogueChoices.push_back(lines);
}

ExecutionResult Script::ScriptExecutor::scriptOpcode0x17() {
	// Finish the dialogue choice.
	View1 *currentView = (View1 *)_engine->findView("View1");
	uint32 x = scriptReadValue32();
	uint32 y = scriptReadValue32();
	uint16 side = scriptReadValue16();
	const uint16 speakerObjectID = activeDialogueSpeakerObjectID != 0 ? activeDialogueSpeakerObjectID : _executingScriptObjectID;
	debugC(kDebugScript,
		   "Opcode 17 choice box: speaker=%u rawPos=(%u,%u) side=%u choiceCount=%u",
		   speakerObjectID, x, y, side, DialogueChoices.size());
	currentView->ShowDialogueChoice(speakerObjectID, DialogueChoices, Common::Point(x, y), side);
	requestCallback = false;
	// NOTE: EndTimer prevents race conditions from overlapping waits

	EndTimer();
	EndBuffering(lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

ExecutionResult Script::ScriptExecutor::scriptOpcode0x18() {
	// Set the stream to the end and let the calling code figure out that we are done
	// for this run.
	_stream->seek(_stream->size(), SEEK_SET);
	EndBuffering(lastOpcodeTriggeredSkip);
	return ExecutionResult::ScriptFinished;
}

bool Script::ScriptExecutor::scriptOpcode0x19() {
	// Walk to and pick up an object.
	uint32 actorIndex = scriptReadValue32() - 0x400;
	uint32 objectIndex = scriptReadValue32() - 0x400;

	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *actor = currentView->GetCharacterByIndex(actorIndex);
	GameObject *targetObject = GameObjects::GetObjectByIndex(objectIndex);
	if (pickupInProgress) {
		EndTimer();
		EndBuffering(lastOpcodeTriggeredSkip);
		return true;
	}
	if (actor == nullptr || targetObject == nullptr) {
		warning("Invalid pickup request for actor %u target %u", actorIndex, objectIndex);
		return false;
	}
	if (actorIndex == objectIndex || targetObject->SceneIndex == actor->GameObject->Index) {
		warning("Ignoring invalid pickup request for actor %u target %u", actorIndex, objectIndex);
		return false;
	}
	if (targetObject->SceneIndex != actor->GameObject->SceneIndex) {
		warning("Ignoring pickup across scenes for actor %u target %u", actorIndex, objectIndex);
		return false;
	}
	pickupInProgress = true;
	pickupActorObjectID = actorIndex;
	pickupTargetObjectID = objectIndex;
	savedPickupMouseMode = _mouseMode == MouseMode::UseInventory ? MouseMode::Use : _mouseMode;
	currentView->activeInventoryItem = nullptr;
	_engine->SetCursorMode(savedPickupMouseMode);
	currentView->UpdateCursor();
	actor->StartPickup(targetObject);
	requestCallback = false;
	isAwaitingCallback = true;
	// NOTE: EndTimer prevents race conditions from overlapping waits

	EndTimer();
	EndBuffering(lastOpcodeTriggeredSkip);
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x1A() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 value217 = scriptReadValue16();
	uint16 value219 = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring object runtime setup for invalid object %d", objectID);
		return false;
	}

	GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring object runtime setup for missing object %d", objectID);
		return false;
	}

	object->RuntimeValue217 = value217;
	object->RuntimeValue219 = value219;
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x1B() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 slotID = scriptReadValue16();
	uint16 value = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring object slot setup for invalid object %d", objectID);
		return false;
	}

	GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring object slot setup for missing object %d", objectID);
		return false;
	}

	if (slotID < 1 || slotID > ARRAYSIZE(object->RuntimeSlotValues)) {
		warning("Ignoring object slot setup for invalid slot %u on object %d", slotID, objectID);
		return false;
	}

	object->RuntimeSlotValues[slotID - 1] = value;
	return true;
}

void Script::ScriptExecutor::scriptOpcode0x1C() {
	// Sets g_wScriptSkippable [102Ah] = 1.
	scriptSkippable = true;
}

void Script::ScriptExecutor::scriptOpcode0x1D() {
	// Sets g_wScriptSkippable [102Ah] = 0.
	scriptSkippable = false;
}

bool Script::ScriptExecutor::scriptOpcode0x1E() {
	// scriptPlayAnimation (1008:bd58).
	uint32 objectID = scriptReadValue32() - 0x400;
	uint32 slotID = scriptReadValue16();
	uint32 frameOffset = scriptReadValue16();

	if (objectID < 1 || objectID > 0x200) {
		warning("Opcode 0x1E: invalid object %u", objectID);
		return false;
	}
	GameObject *gameObject = GameObjects::GetObjectByIndex(objectID);
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
		if (slotID - 1 >= gameObject->Blobs.size() || gameObject->Blobs[slotID - 1].empty()) {
			warning("Opcode 0x1E: no blob data for object %u slot %u", objectID, slotID);
			return false;
		}
		BackgroundAnimationBlob::advanceAnimFrame(gameObject->Blobs[slotID - 1],
												  true, frameOffset + 0x64);
	}

	return true;
}

void Script::ScriptExecutor::scriptOpcode0x1F() {
	uint32 objectID = scriptReadValue32() - 0x400;
	uint32 x = scriptReadValue32();
	uint32 y = scriptReadValue32();
	GameObject *object = GameObjects::GetObjectByIndex(objectID);
	pathWalkableResult = false;
	if (object == nullptr) {
		warning("Ignoring pathfinding test for invalid object %u", objectID);
	} else {
		pathWalkableResult = IsPathWalkable(object->Position, Common::Point(x, y));
	}
}

bool Script::ScriptExecutor::scriptOpcode0x20() {
	// scriptSetYOffset (1008:c047). Sets object field +8 (vertical offset)
	// AND mirrors it into runtime field +0x21D (motion target).
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 offset = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring vertical offset set for invalid object %d", objectID);
		return false;
	}

	GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring vertical offset set for missing object %d", objectID);
		return false;
	}

	object->Unknown = offset;
	// Original also writes to runtime +0x21D (motion target vertical offset)
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr) {
		Character *c = currentView->GetCharacterByIndex((uint16)objectID);
		if (c != nullptr) {
			c->motionTargetVerticalOffset = offset;
		}
	}
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x21() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 targetVerticalOffset = scriptReadValue16();
	uint16 verticalOffsetDelta = scriptReadValue16();
	uint16 motionDistance = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring motion setup for invalid object %d", objectID);
		return false;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *character = currentView ? currentView->GetCharacterByIndex((uint16)objectID) : nullptr;
	GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
	if (object == nullptr || character == nullptr) {
		warning("Ignoring motion setup for missing character object %d", objectID);
		return false;
	}

	character->motionStartVerticalOffset = object->Unknown;
	character->motionTargetVerticalOffset = targetVerticalOffset;
	character->motionVerticalOffsetDelta = verticalOffsetDelta;
	character->motionDistanceUnits = motionDistance;
	character->motionProgress = 0;
	character->hasMotionVerticalOffset = motionDistance != 0 || targetVerticalOffset != object->Unknown;
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x22() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint16 animIndex = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring orientation set for invalid object %d", objectID);
		return false;
	}

	GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		warning("Ignoring orientation set for missing object %d", objectID);
		return false;
	}

	if (animIndex < 9 || animIndex > 0x10) {
		warning("Ignoring out-of-range orientation %u for object %d", animIndex, objectID);
		return false;
	}

	object->Orientation = animIndex;
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x23() {
	int32 objectID = (int32)scriptReadValue32() - 0x400;
	uint32 x = scriptReadValue32();
	uint32 y = scriptReadValue32();
	uint16 targetVerticalOffset = scriptReadValue16();
	if (objectID < 1 || objectID > 0x200) {
		warning("Ignoring move-to-position for invalid object %d", objectID);
		return false;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
	Character *c = currentView ? currentView->GetCharacterByIndex((uint16)objectID) : nullptr;
	if (object == nullptr || c == nullptr) {
		warning("Ignoring move-to-position for missing character object %d", objectID);
		return false;
	}

	const Common::Point target(x, y);
	if (!IsPathWalkable(object->Position, target) && _engine->getWalkabilityAt(target) < 0xC8) {
		warning("Ignoring move-to-position for blocked target (%u,%u) on object %d", x, y, objectID);
		return false;
	}

	c->IsFollowingPath = false;
	c->motionStartVerticalOffset = object->Unknown;
	c->motionTargetVerticalOffset = targetVerticalOffset;
	c->motionVerticalOffsetDelta = ABS<int32>((int32)object->Unknown - (int32)targetVerticalOffset);
	c->motionDistanceUnits = ABS<int32>((int32)x - object->Position.x) + ABS<int32>((int32)y - object->Position.y);
	c->motionProgress = 0;
	c->hasMotionVerticalOffset = true;
	c->StartLerpTo(Common::Point(x, y), 2 * 1000);
	isAwaitingCallback = true;
	return true;
}

void Script::ScriptExecutor::scriptOpcode0x24() {
	// Adds two values read and saves them to a script variable.
	// ;; fn0037_C7E6: 0037:C7E6
	uint32 a = scriptReadValue32();
	uint32 b = scriptReadValue32();

	uint32 result = a + b;
	// Go back to the first value being pointed to.
	// In this case, 9F4D and A334 can use the same data, since the
	// index of the script variable will be in the word at offset 1.
	_stream->seek(-6, SEEK_CUR);
	scriptSaveVariable(result);
	// Skip forward across the second 9F4D read's data.
	_stream->seek(3, SEEK_CUR);
}

void Script::ScriptExecutor::scriptOpcode0x25() {
	// Subtracts two values read and saves them to a script variable.
	// ;; fn0037_C82E: 0037:C82E
	uint32 a = scriptReadValue32();
	uint32 b = scriptReadValue32();

	uint32 result = a - b;
	_stream->seek(-6, SEEK_CUR);
	scriptSaveVariable(result);
	_stream->seek(3, SEEK_CUR);
}

void Script::ScriptExecutor::scriptOpcode0x26() {
	// This one loads a special animation set.
	uint32 id = scriptReadValue32() - 0x400;
	// scriptLoadSpecialAnim (1008:c991): 2nd value is the decode/enable flag (runtime +0x182).
	// Non-zero -> enable the overload animation (and the original decodes the blob now).
	uint16 decodeFlag = scriptReadValue16();
	uint8 animationID = ReadByte();
	Common::Array<uint8> blob = Scenes::instance().ReadSpecialAnimBlob(animationID, g_engine->_fileStream);
	GameObject *object = GameObjects::GetObjectByIndex(id);
	object->overloadAnimation = blob;
	object->overloadAnimationMirrored = false;
	object->useOverloadAnimation = (decodeFlag != 0);
}

bool Script::ScriptExecutor::scriptOpcode0x27() {
	// scriptSetDirection (1008:c858). Writes to runtime field +0x22D.
	// When the character's orientation matches this value, the renderer
	// uses animation slot 0x15 (overload animation) instead of the normal slot.
	// Value 0x7FFF means "never match" (default from loadSceneObjects).
	uint16 characterID = scriptReadValue16() - 0x400;
	uint16 value = scriptReadValue16();
	if (characterID < 1 || characterID > 0x200) {
		warning("Ignoring set direction for invalid object %u", characterID);
		return false;
	}
	GameObject *object = GameObjects::GetObjectByIndex(characterID);
	if (object == nullptr) {
		warning("Ignoring set direction for missing object %u", characterID);
		return false;
	}
	object->overloadAnimTriggerDirection = value;
	return true;
}

void Script::ScriptExecutor::scriptOpcode0x28() {
	// scriptStopAnimation (1008:c8e4) - clears overload animation for an object.
	scriptStopAnimationImpl();
}

bool Script::ScriptExecutor::scriptOpcode0x29() {
	uint32 objectID = scriptReadValue32();
	objectID -= 0x400;
	View1 *currentView = (View1 *)_engine->findView("View1");
	GameObject *inventorySource = GameObjects::GetObjectByIndex(objectID);
	if (inventorySource == nullptr) {
		warning("Invalid inventory source object %u", objectID);
		return false;
	}
	savedExternalInventoryMouseMode = _mouseMode == MouseMode::UseInventory ? MouseMode::Use : _mouseMode;
	hasPendingExternalInventoryResume = true;
	externalInventorySourceObjectID = objectID;
	secondaryInventoryLocation = _stream->pos();
	currentView->OpenInventory(inventorySource);
	return true;
}

void Script::ScriptExecutor::scriptOpcode0x2A() {
	uint32 objectID = scriptReadValue32() - 0x400;
	uint16 slotID = scriptReadValue16();
	const bool decodeBlob = scriptReadValue16() != 0;
	uint8 arrayIndex = ReadByte();

	g_engine->loadAnimationFromSceneData(objectID, slotID, arrayIndex, decodeBlob);
}

bool Script::ScriptExecutor::scriptOpcode0x2B() {
	const uint16 objectID = scriptReadValue16() - 0x400;
	GameObject *object = GameObjects::GetObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring object refresh for invalid object %u", objectID);
		return false;
	}
	if (object->Blobs.empty()) {
		warning("Ignoring object refresh for unloaded object %u", objectID);
		return false;
	}
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView == nullptr) {
		return false;
	}

	Character *character = currentView->GetCharacterByIndex(objectID);
	const int currentIndex = currentView->GetCharacterArrayIndex(character);
	if (object->SceneIndex != Scenes::instance().CurrentSceneIndex) {
		if (currentIndex >= 0) {
			currentView->characters.remove_at(currentIndex);
		}
		return false;
	}

	if (character == nullptr) {
		character = new Character();
		character->GameObject = object;
	} else if (currentIndex >= 0) {
		currentView->characters.remove_at(currentIndex);
	}

	currentView->characters.push_back(character);
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x2C() {
	uint16 objectID = scriptReadValue16() - 0x400;
	uint16 parentID = scriptReadValue16();
	const GameObject *object = GameObjects::GetObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring inventory check for invalid object %u", objectID);
		return false;
	}
	inventoryCheckResult = object->SceneIndex == parentID;
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x2D() {
	const uint16 objectID = scriptReadValue16() - 0x400;
	const bool enabled = scriptReadValue16() != 0;
	GameObject *object = GameObjects::GetObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring object runtime flag for invalid object %u", objectID);
		return false;
	}
	object->RuntimeFlag22F = enabled;
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x2F() {
	// scriptTestObjectAnimFrame: Tests if an object's animation blob's
	// current frame index (via getAnimBlobOffset/source key) falls within
	// [minFrame, maxFrame]. Result stored in animBlobRangeTestResult for helper FF29.
	uint32 objectID = scriptReadValue32() - 0x400;
	uint16 slotID = scriptReadValue16();
	uint16 minFrame = scriptReadValue16();
	uint16 maxFrame = scriptReadValue16();
	animBlobRangeTestResult = false;
	GameObject *object = GameObjects::GetObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring object animation range test for invalid object %u", objectID);
		return false;
	}

	uint16 blobSourceKey = 0;
	bool hasBlob = false;
	if (slotID == 0x15) {
		hasBlob = !object->overloadAnimation.empty();
		blobSourceKey = object->overloadAnimationSourceKey;
	} else if (slotID >= 1 && slotID <= object->Blobs.size()) {
		hasBlob = !object->Blobs[slotID - 1].empty();
		if ((uint)(slotID - 1) < object->BlobSourceKeys.size())
			blobSourceKey = object->BlobSourceKeys[slotID - 1];
	} else {
		warning("Ignoring object animation range test for invalid slot %u on object %u", slotID, objectID);
		return false;
	}

	if (hasBlob) {
		animBlobRangeTestResult = blobSourceKey >= minFrame && blobSourceKey <= maxFrame;
	}
	return true;
}

void Script::ScriptExecutor::scriptOpcode0x30() {
	// Opcode 0x30: scriptPrintString with flag=1 (vs opcode 0x0A with flag=0)
	// which changes behaviour in the function.
	ScriptPrintString(true);
	EndBuffering(lastOpcodeTriggeredSkip);
}

void Script::ScriptExecutor::scriptOpcode0x31() {
	// scriptSetVolume (1008:ce0b): clamp the value to 0..100 (signed), as the original does.
	int16 volume = (int16)scriptReadValue16();
	if (volume < 0)
		volume = 0;
	if (volume > 100)
		volume = 100;
	g_engine->getAdlib()->SetVolume((uint16)volume);
}

bool Script::ScriptExecutor::scriptOpcode0x32() {
	uint16 objectID = scriptReadValue16() - 0x0400;
	const uint16 clickable = scriptReadValue16();
	GameObject *object = GameObjects::GetObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring clickable toggle for invalid object %u", objectID);
		return false;
	}
	object->IsClickable = clickable != 0;
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x33() {
	uint16 objectID = scriptReadValue16() - 0x0400;
	const uint16 visible = scriptReadValue16();
	GameObject *object = GameObjects::GetObjectByIndex(objectID);
	if (object == nullptr) {
		warning("Ignoring visibility toggle for invalid object %u", objectID);
		return false;
	}
	object->IsVisible = visible != 0;
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x34() {
	// Sets an entry in the [5BD1] list for hotspot lookup.
	const uint16 v1 = scriptReadValue16() - 0x800;
	const uint16 v2 = scriptReadValue16() - 0x800;

	if (v1 < 0x1 || v1 > 0x10 || v2 < 0x1 || v2 > 0x10) {
		warning("Ignoring hotspot override %.4x -> %.4x outside valid range", v1 + 0x800, v2 + 0x800);
		return false;
	}
	if (v1 == v2) {
		g_engine->HotspotOverrides[v1] = 0xFFFF;
	} else {
		g_engine->HotspotOverrides[v1] = v2;
	}
	return true;
}

bool Script::ScriptExecutor::scriptOpcode0x35() {
	uint16 objectID = scriptReadValue16() - 0x0400;
	uint16 otherObjectID = scriptReadValue16() - 0x0400;
	const uint16 value1 = scriptReadValue16();
	const uint16 value2 = scriptReadValue16();
	const uint16 value3 = scriptReadValue16();
	GameObject *object = GameObjects::GetObjectByIndex(objectID);
	GameObject *otherObject = GameObjects::GetObjectByIndex(otherObjectID);
	if (object == nullptr || otherObject == nullptr) {
		warning("Ignoring bounds attachment for invalid objects %u -> %u", objectID, otherObjectID);
		return false;
	}

	if (objectID == otherObjectID) {
		object->HasBoundsAttachment = false;
		object->BoundsAttachmentObjectID = 0;
		object->BoundsAttachmentValue1 = 0;
		object->BoundsAttachmentValue2 = 0;
		object->BoundsAttachmentValue3 = 0;
	} else {
		object->HasBoundsAttachment = true;
		object->BoundsAttachmentObjectID = otherObjectID;
		object->BoundsAttachmentValue1 = value1;
		object->BoundsAttachmentValue2 = value2;
		object->BoundsAttachmentValue3 = value3;
	}
	return true;
}

void Script::ScriptExecutor::scriptOpcode0x36() {
	// scriptDismissPanel (1008:d6dd). Restores background if a UI panel
	// was pending, clears panel state, redraws scene, clears timer flag.
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr) {
		if (currentView->_isShowingStringBox || currentView->_isShowingDialogueChoice) {
			currentView->_continueScriptAfterUI = false;
			currentView->clearStringBox(false);
		}

		if (currentView->_isShowingInventory) {
			hasPendingExternalInventoryResume = false;
			externalInventorySourceObjectID = 0;
			currentView->CloseInventory();
		}

		if (currentView->isShowingMainMenu) {
			currentView->isShowingMainMenu = false;
			currentView->redraw();
		}
	}
}

void Script::ScriptExecutor::scriptOpcode0x37() {
	// scriptResetToSceneScript (1008:ad3e). Resets script execution
	// context back to the current scene script at position 0.
	_executingScriptObjectID = 0;
	executingObjectIndex = Scenes::instance().CurrentSceneIndex;
	scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
	activeDialogueSpeakerObjectID = 0;
	SetCurrentSceneScriptAt(0);
}

void Script::ScriptExecutor::scriptOpcode0x38() {
	// scriptLoadOverlayFont (1008:d749). Loads a font resource for
	// overlay text into the overlay font buffer.
	uint8 resourceIndex = ReadByte();
	overlayTextStageActive = true;
	if (!_engine->loadOverlayFont(resourceIndex, _executingScriptObjectID)) {
		warning("Opcode 0x38: failed to load overlay font resource %u", resourceIndex);
	}
}

Script::ScriptExecutor::OpcodeControlFlow Script::ScriptExecutor::scriptOpcode0x3A() {
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView == nullptr) {
		warning("Ignoring overlay text entry without an active View1");
		scriptReadValue16();
		scriptReadValue16();
		scriptReadValue16();
		ReadWord();
		ReadWord();
		return OpcodeControlFlow::Continue;
	}

	const uint16 x = scriptReadValue16();
	const uint16 y = scriptReadValue16();
	const uint8 alignment = scriptReadValue16();
	const uint16 stringOffset = ReadWord();
	const uint16 entryType = ReadWord();
	if (!overlayTextStageActive) {
		warning("Opcode 0x3A: overlay text entry at %u,%u without active stage (error 0x21)", x, y);
		EndBuffering(lastOpcodeTriggeredSkip);
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
		strings = _engine->DecodeStrings(Scenes::instance().CurrentSceneStrings, stringOffset, 1);
	} else {
		Common::MemoryReadStream *stringsStream = GameObjects::ReadGameObjectStrings(_executingScriptObjectID, g_engine->_fileStream);
		strings = _engine->DecodeStrings(stringsStream, stringOffset, 1);
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

void Script::ScriptExecutor::scriptOpcode0x3B() {
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr) {
		currentView->clearOverlayTextEntries();
	}
}

void Script::ScriptExecutor::scriptOpcode0x0F() {
	// The original interpreter stores a frame countdown that is decremented
	// once per game tick, rather than using a wall-clock timer.
	uint16 duration = scriptReadValue16();
	requestCallback = false;
	StartFrameWait(duration);
	isAwaitingCallback = true;
	EndBuffering(lastOpcodeTriggeredSkip);
}

void Script::ScriptExecutor::scriptOpcode0x12() {
	// scriptSetPathfinding (1008:c6d7). Sets/clears a pathfinding override.
	// Index must be in range 200..0xEF (walkability values).
	// Writes to scene data at index*5 + 0x4EA5 (enable byte) and +0x4EA6 (value).
	uint16 areaID = scriptReadValue16();
	uint16 active = scriptReadValue16();
	uint16 overrideValue = scriptReadValue16();
	if (active) {
		g_engine->SetPathfindingOverride(areaID, overrideValue);
	} else {
		g_engine->RemovePathfindingOverride(areaID);
	}
}

void Script::ScriptExecutor::scriptOpcode0x2E() {
	// scriptTestSceneAnimFrame: Tests if a scene's special animation blob's
	// current frame index (via getAnimBlobOffset/source key) falls within
	// [minFrame, maxFrame]. Result stored in animBlobRangeTestResult for helper FF29.
	// Index is 0x1000-based in the bytecode (matches opcode 0x0E scriptChangeAnimation,
	// binary scriptTestSceneAnimFrame at 1008:... subtracts 0x1000).
	uint32 sceneAnimIndex = scriptReadValue32() - 0x1000;
	uint32 minFrame = scriptReadValue32();
	uint32 maxFrame = scriptReadValue32();
	animBlobRangeTestResult = false;
	if (sceneAnimIndex == 0 || sceneAnimIndex > Scenes::instance().CurrentSceneSpecialAnimOffsets.size()) {
		warning("Ignoring scene animation range test for invalid index %u", sceneAnimIndex);
	} else {
		const uint16 blobSourceKey = static_cast<uint16>(Scenes::instance().CurrentSceneSpecialAnimOffsets[sceneAnimIndex - 1] >> 16);
		animBlobRangeTestResult = blobSourceKey >= minFrame && blobSourceKey <= maxFrame;
	}
}

void Script::ScriptExecutor::scriptOpcode0x39() {
	if (overlayTextStageActive) {
		overlayTextStageActive = false;
	}
}

void Script::ScriptExecutor::scriptOpcode0x3D() {
	const uint16 fadeSpeed = scriptReadValue16();
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr && fadeSpeed != 0) {
		currentView->startFading(fadeSpeed);
	}
}

void Script::ScriptExecutor::scriptOpcode0x3F() {
	if (soundEnabled)
		_engine->stopCurrentSound();
	_engine->clearCurrentSoundData();
}

ExecutionResult Script::ScriptExecutor::ExecuteScript() {
	debug("----- Scripting function entered - scene: %.2x 1014: %.2x 1012: %.2x", Scenes::instance().CurrentSceneIndex, IsSceneInitRun, repeatRunFlag);
	isRunningScript = true;
	// Confirmed: no interrupt mechanism exists. Wait states (frameWait, walkTarget,
	// pcmSound, musicControl, adlibReady) are resolved by gameTick externally.
	isAwaitingCallback = false;

	requestCallback = false;

	// We use this to keep track of cases where we did not read all information as we should have
	expectedEndLocation = _stream->pos();
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
		if (_stream->pos() != expectedEndLocation) {
			warning("Macs2::ScriptExecutor::ExecuteScript resyncing stream from %u to %u",
					(uint32)expectedEndLocation, (uint32)_stream->pos());
			expectedEndLocation = _stream->pos();
		}

		// Read an opcode and length
		byte opcode1 = ReadByte(); // [bp - 1h]
		// TODO: For the sake of easier reading the logs, jumping out if we
		// read a 0 opcode.
		if (opcode1 == 0x00) {
			// Account for the missing byte from reading opcode 0
			expectedEndLocation++;
			continue;
		}
		Common::String opcodeInfo;
		if (opcode1 != 0x5) {
			opcodeInfo = IdentifyScriptOpcode(opcode1, 0);
		}
		debug("- First block opcode: %.2x %s", opcode1, opcodeInfo.c_str());
		byte length = ReadByte(); // [bp-2h]
		expectedEndLocation += length + 2;

		// TODO: convert this into a function lookup table and extract all opcode handling into separate functions, this is just for easier reading
		if (opcode1 == 0x01) {
			scriptOpcode0x01();
		} else if (opcode1 == 0x02) {
			scriptOpcode0x02();
		} else if (opcode1 == 0x03) {
			scriptOpcode0x03();
		} else if (opcode1 == 0x04) {
			scriptOpcode0x04();
		} else if (opcode1 == 0x5) {
			if (!scriptOpcode0x05()) {
				EndBuffering(lastOpcodeTriggeredSkip);
				break;
			}
		} else if (opcode1 == 0x06) {
			scriptOpcode0x06();
		} else if (opcode1 == 0x07) {
			scriptOpcode0x07();
		} else if (opcode1 == 0x08) {
			scriptOpcode0x08();
		} else if (opcode1 == 0x10) {
			if (!scriptOpcode0x10()) {
				continue;
			}
		} else if (opcode1 == 0x11) {
			return scriptOpcode0x11();
		} else if (opcode1 == 0x13) {
			scriptOpcode0x13();
		} else if (opcode1 == 0x14) {
			scriptOpcode0x14();
		} else if (opcode1 == 0x0a) {
			scriptOpcode0x0A();
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x15) {
			scriptOpcode0x15();
		} else if (opcode1 == 0x16) {
			scriptOpcode0x16();
		} else if (opcode1 == 0x17) {
			return scriptOpcode0x17();
		} else if (opcode1 == 0x18) {
			return scriptOpcode0x18();
		} else if (opcode1 == 0x19) {
			if (scriptOpcode0x19()) {
				return ExecutionResult::WaitingForCallback;
			}
			continue;
		} else if (opcode1 == 0x1a) {
			if (!scriptOpcode0x1A()) {
				continue;
			}
		} else if (opcode1 == 0x1b) {
			if (!scriptOpcode0x1B()) {
				continue;
			}
		} else if (opcode1 == 0x1c) {
			scriptOpcode0x1C();
		} else if (opcode1 == 0x1d) {
			scriptOpcode0x1D();
		} else if (opcode1 == 0x1e) {
			if (!scriptOpcode0x1E()) {
				continue;
			}
		} else if (opcode1 == 0x1f) {
			scriptOpcode0x1F();
		} else if (opcode1 == 0x20) {
			if (!scriptOpcode0x20()) {
				continue;
			}
		} else if (opcode1 == 0x21) {
			if (!scriptOpcode0x21()) {
				continue;
			}
		} else if (opcode1 == 0x22) {
			if (!scriptOpcode0x22()) {
				continue;
			}
		} else if (opcode1 == 0x23) {
			if (!scriptOpcode0x23()) {
				continue;
			}
		} else if (opcode1 == 0x24) {
			scriptOpcode0x24();
		} else if (opcode1 == 0x25) {
			scriptOpcode0x25();
		} else if (opcode1 == 0x26) {
			scriptOpcode0x26();
		} else if (opcode1 == 0x27) {
			if (!scriptOpcode0x27()) {
				continue;
			}
		} else if (opcode1 == 0x28) {
			scriptOpcode0x28();
		} else if (opcode1 == 0x29) {
			if (scriptOpcode0x29()) {
				return ExecutionResult::WaitingForCallback;
			}
			continue;
		} else if (opcode1 == 0x0b) {
			if (!scriptOpcode0x0B()) {
				continue;
			}
		} else if (opcode1 == 0x0c) {
			return scriptOpcode0x0C();
		} else if (opcode1 == 0x0d) {
			return scriptOpcode0x0D();
		} else if (opcode1 == 0x0E) {
			scriptOpcode0x0E();
		} else if (opcode1 == 0x0F) {
			scriptOpcode0x0F();
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x12) {
			scriptOpcode0x12();
		} else if (opcode1 == 0x14) {
			// Opcode 0x14: consume/skip a word value (confirmed: just calls scriptReadWord in disassembly)
			ReadWord();
		} else if (opcode1 == 0x2A) {
			scriptOpcode0x2A();
		} else if (opcode1 == 0x2B) {
			if (!scriptOpcode0x2B()) {
				continue;
			}
		} else if (opcode1 == 0x2C) {
			if (!scriptOpcode0x2C()) {
				continue;
			}
		} else if (opcode1 == 0x2D) {
			if (!scriptOpcode0x2D()) {
				continue;
			}
		} else if (opcode1 == 0x2E) {
			scriptOpcode0x2E();
		} else if (opcode1 == 0x2F) {
			if (!scriptOpcode0x2F()) {
				continue;
			}
		} else if (opcode1 == 0x30) {
			scriptOpcode0x30();
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x31) {
			scriptOpcode0x31();
		} else if (opcode1 == 0x32) {
			if (!scriptOpcode0x32()) {
				continue;
			}
		} else if (opcode1 == 0x33) {
			if (!scriptOpcode0x33()) {
				continue;
			}
		} else if (opcode1 == 0x34) {
			if (!scriptOpcode0x34()) {
				continue;
			}
		} else if (opcode1 == 0x35) {
			if (!scriptOpcode0x35()) {
				continue;
			}
		} else if (opcode1 == 0x36) {
			scriptOpcode0x36();
		} else if (opcode1 == 0x37) {
			scriptOpcode0x37();
		} else if (opcode1 == 0x38) {
			scriptOpcode0x38();
		} else if (opcode1 == 0x39) {
			// scriptEndOverlayText (1008:d80f). Clears the overlay text stage.
			scriptOpcode0x39();
		} else if (opcode1 == 0x3A) {
			const OpcodeControlFlow controlFlow = scriptOpcode0x3A();
			if (controlFlow == OpcodeControlFlow::Continue) {
				continue;
			}
			if (controlFlow == OpcodeControlFlow::ScriptFinished) {
				return ExecutionResult::ScriptFinished;
			}
		} else if (opcode1 == 0x3B) {
			scriptOpcode0x3B();
		} else if (opcode1 == 0x3C) {
			const uint16 fadeSpeed = scriptReadValue16();
			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr && fadeSpeed != 0) {
				currentView->startFadeToBlack(fadeSpeed);
			}
		} else if (opcode1 == 0x3D) {
			scriptOpcode0x3D();
		} else if (opcode1 == 0x3E) {
			const uint8 resourceIndex = ReadByte();
			Common::Array<uint8> soundData;
			if (!loadSoundResource(soundData, resourceIndex))
				continue;

			if (_engine->hasCurrentSound() && soundEnabled)
				_engine->stopCurrentSound();
			_engine->setCurrentSoundData(soundData);
		} else if (opcode1 == 0x3F) {
			scriptOpcode0x3F();
		} else if (opcode1 == 0x40) {
			if (soundEnabled) {
				if (!_engine->hasCurrentSound()) {
					warning("Ignoring sound playback without loaded sound data");
					continue;
				}
				_engine->playCurrentSound();
			}
		} else if (opcode1 == 0x41) {
			if (soundEnabled && soundSystemActive) {
				waitForSoundPlayback = true;
				EndTimer();
				EndBuffering(lastOpcodeTriggeredSkip);
				return ExecutionResult::WaitingForCallback;
			}
		} else if (opcode1 == 0x42) {
			if (soundEnabled)
				_engine->stopCurrentSound();
		} else if (opcode1 == 0x43) {
			const uint16 slotID = scriptReadValue16();
			const uint8 resourceIndex = ReadByte();
			if (slotID < 1 || slotID > 2) {
				warning("Ignoring music load for invalid slot %u", slotID);
				continue;
			}

			Common::Array<uint8> slotData;
			if (loadMusicResource(slotData, resourceIndex))
				musicSlots[slotID - 1] = slotData;
		} else if (opcode1 == 0x44) {
			const uint16 slotID = scriptReadValue16();
			const uint16 startMuted = scriptReadValue16();
			const uint16 fadeParam = scriptReadValue16();
			if (slotID < 1 || slotID > 2) {
				warning("Ignoring music start for invalid slot %u", slotID);
				continue;
			}

			if (!musicEnabled || !soundSystemActive) {
				activeMusicSlot = slotID;
				continue;
			}

			if (activeMusicSlot != 0) {
				_engine->getAdlib()->StopMusic();
				activeMusicSlot = 0;
			}

			if (musicSlots[slotID - 1].empty()) {
				warning("Ignoring music start for empty slot %u", slotID);
				continue;
			}

			_engine->getAdlib()->PlaySongData(musicSlots[slotID - 1]);
			if (startMuted == 0) {
				musicControlMode = 1;
				musicControlParam = fadeParam;
				musicControlVolume = 0x3F;
				_engine->getAdlib()->SetVolume(musicControlVolume);
			} else {
				musicControlMode = 0;
				musicControlParam = 0;
				musicControlVolume = 0;
				_engine->getAdlib()->SetVolume(0);
			}

			activeMusicSlot = slotID;
		} else if (opcode1 == 0x45) {
			const uint16 slotID = scriptReadValue16();
			const uint16 stopImmediately = scriptReadValue16();
			const uint16 fadeParam = scriptReadValue16();
			if (slotID < 1 || slotID > 2) {
				warning("Ignoring music stop for invalid slot %u", slotID);
				continue;
			}

			if (!musicEnabled || !soundSystemActive) {
				activeMusicSlot = 0;
				continue;
			}

			if (activeMusicSlot == slotID) {
				if (stopImmediately == 0) {
					musicControlMode = 2;
					musicControlParam = fadeParam;
				} else {
					_engine->getAdlib()->StopMusic();
					musicControlMode = 0;
					musicControlParam = 0;
					activeMusicSlot = 0;
				}
			}
		} else if (opcode1 == 0x47) {
			if (soundSystemActive && musicEnabled) {
				waitForMusicControl = true;
				EndTimer();
				EndBuffering(lastOpcodeTriggeredSkip);
				return ExecutionResult::WaitingForCallback;
			}
		} else if (opcode1 == 0x46) {
			const uint16 slotID = scriptReadValue16();
			if (slotID < 1 || slotID > 2) {
				warning("Ignoring music free for invalid slot %u", slotID);
				continue;
			}

			if (activeMusicSlot == slotID) {
				if (musicEnabled && soundSystemActive)
					_engine->getAdlib()->StopMusic();
				activeMusicSlot = 0;
			}
			musicSlots[slotID - 1].clear();
		} else if (opcode1 == 0x48) {
			// Retrieve object x and use A334 to save it to a script variable
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object X query for invalid object %d", objectID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object X query for missing object %d", objectID);
				continue;
			}
			scriptSaveVariable(object->Position.x);
		} else if (opcode1 == 0x49) {
			// Retrieve object y and use A334 to save it to a script variable
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object Y query for invalid object %d", objectID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object Y query for missing object %d", objectID);
				continue;
			}
			scriptSaveVariable(object->Position.y);
		} else if (opcode1 == 0x4A) {
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object field query for invalid object %d", objectID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object field query for missing object %d", objectID);
				continue;
			}
			scriptSaveVariable(object->Unknown);
		} else if (opcode1 == 0x4B) {
			// Retrieve object orientation and use A334 to save it to a script variable
			int32 objectID = (int32)scriptReadValue32() - 0x400;
			if (objectID < 1 || objectID > 0x200) {
				warning("Ignoring object orientation query for invalid object %d", objectID);
				continue;
			}
			GameObject *object = GameObjects::GetObjectByIndex((uint16)objectID);
			if (object == nullptr) {
				warning("Ignoring object orientation query for missing object %d", objectID);
				continue;
			}
			scriptSaveVariable(object->Orientation);
		} else if (opcode1 == 0x4C) {
			for (GameObject *object : GameObjects::instance().Objects) {
				if (object != nullptr && object->SceneIndex == Scenes::instance().CurrentActorIndex + 0x400) {
					object->SceneIndex = 0;
				}
			}

			View1 *currentView = (View1 *)_engine->findView("View1");
			if (currentView != nullptr && currentView->inventorySource != nullptr) {
				currentView->SetInventorySource(currentView->inventorySource);
				if (currentView->activeInventoryItem != nullptr &&
					currentView->activeInventoryItem->SceneIndex != currentView->inventorySource->Index) {
					currentView->activeInventoryItem = nullptr;
				}
			}
		} else if (opcode1 == 0x4D) {
			// scriptSetPathfindingRemap (1008:dafb). Writes to scene+value*5+0x4EA8.
			const uint16 sourceValue = scriptReadValue16();
			const uint16 targetValue = scriptReadValue16();
			if (sourceValue < AREA_OVERRIDE_MIN || sourceValue > AREA_OVERRIDE_MAX ||
				targetValue < AREA_OVERRIDE_MIN || targetValue > AREA_OVERRIDE_MAX) {
				warning("Ignoring area remap %.4x -> %.4x outside valid range", sourceValue, targetValue);
				continue;
			}
			g_engine->_areaOverrides[sourceValue - AREA_OVERRIDE_MIN] = targetValue;
		} else if (opcode1 == 0x4E) {
			if (soundSystemActive && musicEnabled) {
				waitForAdlibReady = true;
				EndTimer();
				EndBuffering(lastOpcodeTriggeredSkip);
				return ExecutionResult::WaitingForCallback;
			}
		} else {
			ScriptUnimplementedOpcode_Main(opcode1);
			EndBuffering(lastOpcodeTriggeredSkip);
			break;
		}
		EndBuffering(lastOpcodeTriggeredSkip);
	}
	isRunningScript = false;
	debug("----- Scripting function left");
	return ExecutionResult::ScriptFinished;
}

void ScriptExecutor::Run(bool firstRun) {
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
		repeatRunFlag = false;
		IsSceneInitRun = firstRun;
	}
	_state = ExecutorState::Executing;
	Step();
}

void ScriptExecutor::SetScript(Common::MemoryReadStream *stream) {
	_stream = stream;
}

void ScriptExecutor::SetCurrentSceneScriptAt(uint32 offset) {
	SetScript(Scenes::instance().CurrentSceneScript);
	_stream->seek(offset, SEEK_SET);
}

void ScriptExecutor::tick() {
	if (musicControlMode != 0 && activeMusicSlot != 0) {
		const uint16 step = MAX<uint16>(musicControlParam, 1);
		if (musicControlMode == 1) {
			musicControlVolume = (musicControlVolume > step) ? musicControlVolume - step : 0;
			_engine->getAdlib()->SetVolume(musicControlVolume);
			if (musicControlVolume == 0) {
				musicControlMode = 0;
			}
		} else {
			const uint16 nextVolume = MIN<uint16>(musicControlVolume + step, 0x3F);
			musicControlVolume = nextVolume;
			if (musicControlVolume < 0x3F) {
				_engine->getAdlib()->SetVolume(musicControlVolume);
			} else {
				musicControlMode = 0;
				activeMusicSlot = 0;
				_engine->getAdlib()->StopMusic();
			}
		}
	}

	if (waitForSoundPlayback) {
		if (!_engine->isCurrentSoundPlaying()) {
			waitForSoundPlayback = false;
			Run();
		} else {
			debug("Waiting for sound playback to finish (handle active)");
		}
		return;
	}

	if (waitForMusicControl) {
		if (musicControlMode == 0) {
			waitForMusicControl = false;
			Run();
		}
		return;
	}

	if (waitForAdlibReady) {
		if (_engine->getAdlib()->isPlaybackReady()) {
			waitForAdlibReady = false;
			Run();
		}
		return;
	}

	if (isFrameWaitActive) {
		if (frameWaitTicksRemaining > 0) {
			--frameWaitTicksRemaining;
		}
		if (frameWaitTicksRemaining == 0) {
			isFrameWaitActive = false;
			Run();
		}
	}

	if (isTimerActive) {
		if (g_engine->currentMillis > timerEndMillis) {
			isTimerActive = false;
			// TODO: Think about if this is the right way of executing it, or maybe we rather need
			// to use Execute
			Run();
		}
	}
}

void ScriptExecutor::StartTimer(uint32 duration) {
	isTimerActive = true;
	timerEndMillis = g_engine->currentMillis + duration;
}

void ScriptExecutor::EndTimer() {
	isTimerActive = false;
}

void ScriptExecutor::StartFrameWait(uint16 duration) {
	isFrameWaitActive = true;
	frameWaitTicksRemaining = duration;
}

void ScriptExecutor::EndFrameWait() {
	isFrameWaitActive = false;
	frameWaitTicksRemaining = 0;
}

void ScriptExecutor::Rewind() {
	_stream->seek(0);
}

uint32 ScriptExecutor::GetScriptPosition() const {
	return _stream ? (uint32)_stream->pos() : 0;
}

uint32 ScriptExecutor::GetScriptEndPosition() const {
	return _stream ? (uint32)_stream->size() : 0;
}

uint32 ScriptExecutor::GetVariableValue(int index) const {
	if (index < 0 || index >= (int)_variables.size())
		return 0;
	return _variables[index].a | ((uint32)_variables[index].b << 16);
}

uint32 ScriptExecutor::GetSpecialValue(uint16 value) {
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
		const Common::Point &charPos = GetCharPosition();
		out1 = getAreaAtPoint(charPos.x, charPos.y);
		break;
	}
	case 0x0B:
		out1 = repeatRunFlag ? 1 : 0;
		break;
	case 0x0D:
		out1 = chosenDialogueOption;
		break;
	case 0x23:
		out1 = pathWalkableResult ? 1 : 0;
		break;
	case 0x24: {
		const GameObject *actor = GameObjects::instance().GetObjectByIndex(Scenes::instance().CurrentActorIndex);
		out1 = actor ? actor->Position.x : 0;
		break;
	}
	case 0x25: {
		const GameObject *actor = GameObjects::instance().GetObjectByIndex(Scenes::instance().CurrentActorIndex);
		out1 = actor ? actor->Position.y : 0;
		break;
	}
	case 0x26:
		out1 = IsSceneInitRun ? 1 : 0;
		break;
	case 0x28:
		out1 = inventoryCheckResult ? 1 : 0;
		break;
	case 0x2A: {
		View1 *v = (View1 *)_engine->findView("View1");
		const bool uiOpen = v != nullptr && (v->_isShowingInventory || v->_isShowingStringBox || v->isShowingMainMenu);
		out1 = (inventoryCombineFlag && !uiOpen) ? 1 : 0;
		break;
	}
	case 0x2B: {
		View1 *v = (View1 *)_engine->findView("View1");
		const bool uiOpen = v != nullptr && (v->_isShowingInventory || v->_isShowingStringBox || v->isShowingMainMenu);
		out1 = (inventoryActionFlag && !uiOpen) ? 1 : 0;
		break;
	}
	case 0x2D:
		out1 = Scenes::instance().CurrentSceneIndex;
		break;
	case 0x2F:
		out1 = Scenes::instance().LastSceneIndex;
		break;
	default:
		break;
	}
	return out1 | ((uint32)out2 << 16);
}

} // namespace Script

} // namespace Macs2
