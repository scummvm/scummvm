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

#include "macs2/scriptexecutor.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "macs2/debugtools.h"
#include "macs2/detection.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/music.h"
#include "macs2/view1.h"

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
	// Binary: script variable block is 0x2000 bytes = 2048 entries of {uint16 a,
	// uint16 b}. scriptReadValue (1008:9f4d) accepts indices 1..0x800 and reads
	// at _g_pScriptVariables + value*4 - 4, so there are exactly 0x800 (2048)
	// variables. The save file (saveGameToFile 1008:6859) writes this full
	// 0x2000-byte block. Using fewer entries corrupts the save layout and risks
	// out-of-bounds variable access.
	constexpr int numVariables = 0x800;
	_variables.resize(numVariables);
	for (int i = 0; i < numVariables; i++) {
		_variables[i].a = 0;
		_variables[i].b = 0;
	}
}

ScriptExecutor::~ScriptExecutor() {
	if (_stream && _stream != Scenes::instance()._currentSceneScript) {
		delete _stream;
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
	// Binary handleInput (1008:e8bf): skip past end without opcode 0x1D -> error 0x11.
	setScriptError(0x11);
	_scriptSkippable = false;
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
			setScriptError(0x1A);
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
		setScriptError(0x16);
		return;
	}

	uint16 variableID = readUint16();
	debugC(kDebugScript, "SCRIPT::saveVariable(subOpcode=0x%02x, variableID=%u, value=%u)", subOpcode, variableID, value);
	setVariableValue(variableID, value);
}

void ScriptExecutor::scriptChangeAnimation() {
	// scriptChangeAnimation (1008:b6be). Changes a background animation's
	// current frame by calling advanceAnimFrame with a target position.
	// Binary calls scriptReadValue() twice (16-bit reads); literals like 4097
	// (0x1001) fit in one typed word.
	const uint16 backgroundAnimationIndex = scriptReadValue16() - 0x1000;
	const uint16 targetFrameIndex = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::changeAnimation(bgAnim=%u, targetFrame=%u)", backgroundAnimationIndex, targetFrameIndex);
	if (backgroundAnimationIndex < 1) {
		setScriptError(8);
		return;
	}
	if (backgroundAnimationIndex > _engine->_backgroundAnimationsBlobs.size()) {
		setScriptError(8);
		return;
	}
	// Binary is 1-indexed, C++ array is 0-indexed
	BackgroundAnimationBlob &blob = _engine->_backgroundAnimationsBlobs[backgroundAnimationIndex - 1];
	if (blob._blob.empty()) {
		setScriptError(8);
		return;
	}
	const uint16 sequenceLength = BackgroundAnimationBlob::getAnimFrameCount(blob._blob);
	if (targetFrameIndex > sequenceLength) {
		setScriptError(9);
		return;
	}
	BackgroundAnimationBlob::advanceAnimFrame(blob._blob, true, targetFrameIndex + 0x64);
	// Match save/load restore: keep the requested sequence position in blob[+2]
	// even when the jump parser settles on a command byte instead of a frame byte.
	if (blob._blob.size() >= 4)
		WRITE_LE_UINT16(&blob._blob[2], targetFrameIndex);
	// Blob state is updated immediately but the script executor often runs several
	// more opcodes before the next game tick. Push the new frame to the screen now
	// so door/state changes are visible before any wait opcode yields.
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr)
		currentView->presentFrame();
}

uint16 ScriptExecutor::getAreaAtPoint(uint16 x, uint16 y) {
	// getAreaAtPoint (1008:101d). Reads the pathfinding map pixel and applies
	// the area override table at sceneData + value*5 + 0x4EA8.
	if (x >= kScreenWidth || y >= kGameHeight || _engine->_pathfindingMap.w == 0) {
		return 0;
	}
	uint16 result = _engine->_pathfindingMap.getPixel(x, y);
	if (result >= AREA_OVERRIDE_MIN && result < 250) {
		uint16 overrideValue = _engine->getPathfindingOverride2(result);
		if (overrideValue >= AREA_OVERRIDE_MIN) {
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

	if (_executingScriptObjectId == 0) {
		if (resourceIndex > _engine->_sceneResourceOffsets.size()) {
			warning("Ignoring resource load for missing scene resource %u", resourceIndex);
			return false;
		}
		address = _engine->_sceneResourceOffsets[resourceIndex - 1];
	} else {
		GameObject *object = GameObjects::getObjectByIndex(_executingScriptObjectId);
		if (object == nullptr || object->_dataOffset == 0) {
			warning("Ignoring resource load for missing object %u resource %u", _executingScriptObjectId, resourceIndex);
			return false;
		}
		// Binary reads from runtime+0x18D table (loaded during loadObjectData).
		// Table is 32 dword file offsets, indexed by (resourceIndex - 1).
		if (resourceIndex - 1 >= 32) {
			warning("Ignoring resource load for out-of-range index %u on object %u", resourceIndex, _executingScriptObjectId);
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
	// scriptPrintString (1008:A94E)
	// Panel restore: if a panel request is pending and background needs restoring,
	// redraw the scene to erase the inventory/dialogue panel before showing text.
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView && currentView->_pendingPanelRequest != View1::kPanelRequestNone &&
		currentView->_uiBackgroundRestorePending) {
		currentView->redraw();
		currentView->_uiBackgroundRestorePending = false;
	}

	const uint16 x = scriptReadValue16();
	const uint16 y = scriptReadValue16();
	const uint16 bp2 = readUint16();
	const uint16 bp4 = readUint16();

	debugC(kDebugScript, "SCRIPT::printString(x=%u, y=%u, strOffset=%u, numLines=%u, alignRight=%d)", x, y, bp2, bp4, alignRight);

	Common::StringArray strings;
	if (_executingScriptObjectId == 0) {
		strings = g_engine->decodeStrings(Scenes::instance()._currentSceneStrings, bp2, bp4, Scenes::instance()._currentSceneIndex, 0);
	} else {
		Common::MemoryReadStream *s = GameObjects::readGameObjectStrings(_executingScriptObjectId, g_engine->_fileStream);
		strings = g_engine->decodeStrings(s, bp2, bp4, 0, _executingScriptObjectId);
		delete s;
	}

	int stringBoxX = x;
	const int stringBoxY = y;
	if (alignRight) {
		const int totalWidth = g_engine->measureStrings(strings) + 0x12;
		stringBoxX -= totalWidth;
	}

	if (currentView) {
		// Binary scriptPrintString (1008:a9fa): renders text, then sets g_wIsShowingTextBox=1
		currentView->_stringBoxPosition = Common::Point(stringBoxX, stringBoxY);
		currentView->_drawnStringBox = strings;
		currentView->_isShowingTextBox = true;
		currentView->currentSpeechActData.speaker = nullptr;
		currentView->_continueScriptAfterUI = true;
		currentView->redraw();
	}

	_waitingForUiClick = true;

	// Binary scriptPrintString (1008:aa59): if cursor was Disabled (0x1A), restore Walk (0x16)
	// so handleInput (1008:f1d4) accepts mouse clicks to dismiss the text box.
	if (_cursorMode == MouseMode::Disabled) {
		_engine->setCursorMode(MouseMode::Walk);
		if (currentView)
			currentView->updateCursor();
	}
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

void ScriptExecutor::setScriptError(uint16 code) {
	if (_scriptErrorCode == 0) {
		_scriptErrorCode = code;
		warning("Script error 0x%02x at object %u opcode 0x%02x pos %u",
				code, _executingScriptObjectId, _lastOpcode, (uint32)_lastOpcodeStreamPos);
	}
}

void ScriptExecutor::enterBlockingWaitCursor() {
	// executeOpcodes (1008:db56): frame/walk/sound waits inline before LAB_1008_e3bd.
	if (_cursorMode != MouseMode::Disabled)
		_cursorModeBeforeWait = _cursorMode;
	_engine->setCursorMode(MouseMode::Disabled);
}

void ScriptExecutor::clearScriptUiWaitState() {
	_waitingForUiClick = false;
	View1 *v = (View1 *)_engine->findView("View1");
	if (v) {
		v->_isShowingTextBox = false;
		v->_isShowingDialoguePanel = false;
		v->_isDialogueChoiceInputActive = false;
	}
}

void ScriptExecutor::recordScriptErrorPosition() {
	if (!hasScriptError() || !_stream)
		return;
	// Binary LAB_1008_e3bd: save position and scene/object context on halt.
	_errorScriptPosition = (uint32)_stream->pos();
	if (_executingScriptObjectId == 0) {
		_errorScriptContext = Scenes::instance()._currentSceneIndex;
	} else {
		_errorScriptContext = (uint16)(_executingScriptObjectId + 0x400);
	}
}

void ScriptExecutor::runSceneScriptPass(bool initRun, bool repeatRun) {
	if (isScriptWaitDeferred()) {
		return;
	}
	_executingScriptObjectId = 0;
	_isSceneInitRun = initRun;
	_repeatRunFlag = repeatRun;
	_scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
	setScript(Scenes::instance()._currentSceneScript);
	if (_stream && _stream->size() > 0)
		_stream->seek(0, SEEK_SET);
	const ExecutorState previousState = _state;
	_state = ExecutorState::Executing;
	step();
	if (_state != ExecutorState::WaitingForCallback)
		_state = previousState;
}

void ScriptExecutor::beginSceneEntryInitPass() {
	_deferredRepeatAfterInit = false;
	_initPassComplete = false;
	runSceneScriptPass(true, false);
	if (isScriptWaitDeferred() && !_initPassComplete) {
		_deferredRepeatAfterInit = true;
		_isSceneInitRun = true;
	} else {
		_isSceneInitRun = false;
	}
}

void ScriptExecutor::finishSceneEntryRepeatPass(bool terminateOuterScript) {
	if (!_initPassComplete || hasScriptError())
		return;
	// Binary scriptChangeScene / loadResourceFile: set script position to end and
	// executingObjectId=0x201 to stop outer object iteration, then repeat pass.
	if (terminateOuterScript) {
		if (_stream)
			_stream->seek(_stream->size(), SEEK_SET);
		_executingScriptObjectId = 0x201;
		_terminateOuterScriptBeforeRepeat = false;
	}
	runSceneScriptPass(false, true);
}

void ScriptExecutor::runSceneEntryScriptPasses() {
	// Binary loadResourceFile (1008:2e8d): init pass then repeat pass.
	beginSceneEntryInitPass();
	finishSceneEntryRepeatPass(false);
}

void ScriptExecutor::setVariableValue(uint16 index, uint16 a, uint16 b) {
	_variables[index].a = a;
	_variables[index].b = b;
}

void ScriptExecutor::setVariableValue(uint16 index, uint32 value) {
	uint16 a = static_cast<uint16>(value >> 16);    // High 16 bits
	uint16 b = static_cast<uint16>(value & 0xFFFF); // Low 16 bits
	debugC(kDebugScript, "SCRIPT::setVariableValue(index=%u, value=%u (a=%u, b=%u))", index, value, a, b);
	setVariableValue(index, b, a);
}

Common::Point ScriptExecutor::getCharPosition() {
	const GameObject *actor = GameObjects::instance().getObjectByIndex(Scenes::instance()._currentActorIndex);
	if (!actor)
		actor = GameObjects::getProtagonistObject();
	return actor ? actor->_position : Common::Point();
}

void ScriptExecutor::debugLogActorWalkState(const char *context) {
	if (!debugChannelSet(-1, kDebugScript))
		return;

	const uint16 actorIndex = Scenes::instance()._currentActorIndex;
	const GameObject *actor = GameObjects::getObjectByIndex(actorIndex);
	if (actor == nullptr) {
		debugC(kDebugScript, "%s: no actor object", context);
		return;
	}

	const int16 x = actor->_position.x;
	const int16 y = actor->_position.y;
	const uint16 area = getAreaAtPoint((uint16)x, (uint16)y);
	const uint16 walk = _engine->getWalkabilityAt(y, x);
	View1 *view = (View1 *)_engine->findView("View1");
	Character *character = view ? view->getCharacterByIndex(actorIndex) : nullptr;

	debugC(kDebugScript,
		   "%s: actor=(%d,%d) area=%u walk=%u/0x%04x int16=%d walkable=%s var[122]=%u "
		   "cursor=0x%02x executorState=%u walkWaitObj=%u frameWait=%u soundWait=%d musicWait=%d "
		   "pendingVMotion=%s vOff=%u motionTgt=%u motionProg=%u/%u repeatRun=%d",
		   context, x, y, area, walk, walk, (int16)walk, Macs2Engine::isWalkabilityWalkable(walk) ? "yes" : "NO",
		   getVariableValue(122), (uint)_cursorMode, (uint)_state, _walkTargetObjectIndex,
		   _frameWaitTicksRemaining, _waitForPcmSound ? 1 : 0, _waitForMusicControl ? 1 : 0,
		   (character && character->hasPendingVerticalMotion()) ? "yes" : "no",
		   actor->_verticalOffsetScale,
		   character ? character->_motionTargetVerticalOffset : 0u,
		   character ? character->_motionProgress : 0u,
		   character ? character->_motionDistanceUnits : 0u,
		   _repeatRunFlag ? 1 : 0);
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
			if (_stream->pos() >= _stream->size()) {
				// Binary (runScriptExecutor 1008:e3e7): if script finishes while
				// g_wScriptSkippable is still set, treat as error 0x11 and abort.
				if (_scriptSkippable) {
					setScriptError(0x11);
					_scriptSkippable = false;
					shouldContinue = false;
					break;
				}
				// Handle the next one potentially
				shouldContinue = loadNextScript();
			} else {
				// Let the current script continue
				ExecutionResult result = executeOpcodes();
				if (hasScriptError()) {
					recordScriptErrorPosition();
					shouldContinue = false;
					break;
				}
				if (result == ExecutionResult::WaitingForCallback) {
					// We need to change our state as well now
					_state = ExecutorState::WaitingForCallback;
					if (!_debugPaused && !_waitingForUiClick) {
						// Binary sets hourglass inline in executeOpcodes for blocking
						// waits. UI waits (0x0A/0x0D/0x17) set _waitingForUiClick instead.
						enterBlockingWaitCursor();
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
	if (_cursorMode == MouseMode::Disabled) {
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
	// of the objects list. Original iterates 1..0x200, skipping null/irrelevant objects.
	GameObject *candidateObject = nullptr;
	do {
		_executingObjectIndex++;
		if (_executingObjectIndex > 0x200)
			break;
		candidateObject = GameObjects::getObjectByIndex(_executingObjectIndex);

		if (candidateObject && isRelevantObject(candidateObject)) {
			if (candidateObject->_script.size() != 0) {
				if (_stream && _stream != Scenes::instance()._currentSceneScript) {
					delete _stream;
				}
				_stream = candidateObject->getScriptStream();
				_executingScriptObjectId = candidateObject->_index;
				debugC(kDebugScript, "----- Switching execution to script for object: %.4x", candidateObject->_index);
				return true;
			}
		}
	} while (_executingObjectIndex <= 0x200);

	_executingScriptObjectId = 0;

	// We are done executing all relevant objects
	if (_isSceneInitRun) {
		// Binary (1008:e3e7): init pass ends after scene script + object scripts.
		// Repeat is NOT started here unless init was deferred (paused on wait) and
		// scriptChangeScene skipped the explicit second runScriptExecutor call.
		_isSceneInitRun = false;
		_initPassComplete = true;
		if (_deferredRepeatAfterInit) {
			_deferredRepeatAfterInit = false;
			if (_terminateOuterScriptBeforeRepeat) {
				if (_stream)
					_stream->seek(_stream->size(), SEEK_SET);
				_executingScriptObjectId = 0x201;
				_terminateOuterScriptBeforeRepeat = false;
			}
			_repeatRunFlag = true;
			_executingObjectIndex = Scenes::instance()._currentSceneIndex;
			if (_stream && _stream != Scenes::instance()._currentSceneScript) {
				delete _stream;
			}
			_stream = Scenes::instance()._currentSceneScript;
			if (!_stream || _stream->size() == 0) {
				return false;
			}
			_stream->seek(0, SEEK_SET);
			_scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
			debugC(kDebugScript, "----- Deferred repeat pass for scene: %.4x", _executingObjectIndex);
			return true;
		}
	}

	if (_repeatRunFlag) {
		// Repeat run pass completed. Clear the flag (matching binary behavior:
		// scriptChangeScene sets g_wRepeatRunFlag=1 before runScriptExecutor,
		// then clears it to 0 immediately after).
		_repeatRunFlag = false;
		if (_stream && _stream != Scenes::instance()._currentSceneScript) {
			delete _stream;
			_stream = nullptr;
		}
		return false;
	}

	if (_stream && _stream != Scenes::instance()._currentSceneScript) {
		delete _stream;
		_stream = nullptr;
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
	(void)readByte();
	uint16 variableIndex = readUint16();
	ScriptVariable var;
	scriptReadValuePair(var.a, var.b);
	debugC(kDebugScript, "SCRIPT::setVar(variableIndex=%u, value1=%u, value2=%u)", variableIndex, var.a, var.b);
	_variables[variableIndex] = var;
}

void Script::ScriptExecutor::scriptSetVarOr() {
	// Padding/type byte (same as opcode 0x01) - read and discarded
	(void)readByte();
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
	debugC(kDebugScript, "SCRIPT::setVarOr(variableIndex=%u, value1=%u, value2=%u, value3=%u)", variableIndex, value1, value2, value3);
	setVariableValue(variableIndex, value2, value3);
}

void Script::ScriptExecutor::scriptIfTrue() {
	uint16 res1;
	uint16 res2;
	scriptReadValuePair(res1, res2);
	debugC(kDebugScript, "SCRIPT::ifTrue(result1=%u, result2=%u)", res1, res2);
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
	debugC(kDebugScript, "SCRIPT::ifFalse(result1=%u, result2=%u)", result1, result2);
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
	const uint8 opcode2 = readByte();
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

	debugC(kDebugScript, "SCRIPT::compare(val1=%d, val2=%d)", val1, val2);

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
		// Binary (1008:db56): unknown cmpOp leaves conditionMet false -> skip block.
		debugC(kDebugScript, "SCRIPT::compare: unknown sub-opcode 0x%02x, skipping block", opcode2);
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
	const uint8 subOpcode = readByte();
	uint16 interacted1;
	uint16 interacted2;
	scriptReadValuePair(interacted1, interacted2);
	const uint16 object1 = scriptReadValue16();
	const uint16 object2 = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::ifInteraction(subOpcode=0x%02x, interacted1=%u, interacted2=%u, object1=%u, object2=%u)",
		   subOpcode, interacted1, interacted2, object1, object2);
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
	debugC(kDebugScript, "SCRIPT::endIf()");
}

void Script::ScriptExecutor::scriptElse() {
	// This is some kind of skipping as well.
	scriptSkipAlternate();
	debugC(kDebugScript, "SCRIPT::else()");
}

void Script::ScriptExecutor::scriptNop09() {
	// ExecuteScript does not currently have a dedicated opcode 0x09 dispatch branch.
	debugC(kDebugScript, "SCRIPT::nop09()");
}

void Script::ScriptExecutor::scriptPrintStringLeft() {
	// l0037_DDE8:
	debugC(kDebugScript, "SCRIPT::printStringLeft()");
	scriptPrintString();
	// Ends execution (confirmed: jumps to e3bd in disassembly).
	endBuffering(_lastOpcodeTriggeredSkip);
}

void Script::ScriptExecutor::scriptMoveObject() {
	// scriptMoveObject (1008:aa83). Moves an object to a new scene/position.
	// Handles render list updates for both source and destination scenes.
	const uint32 objectID = scriptReadValue32() - 0x400;
	const uint16 sceneID = scriptReadValue16();
	const int16 x = (int16)scriptReadValue16();
	const int16 y = (int16)scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::moveObject(objectID=%u, sceneID=%u, x=%d, y=%d)", objectID, sceneID, x, y);

	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	clearScriptError();

	// Binary (1008:aa83): parent container must exist when sceneIndex > 0x400.
	if (object->_sceneIndex > 0x400) {
		GameObject *oldParent = GameObjects::getObjectByIndex(object->_sceneIndex - 0x400);
		if (oldParent == nullptr) {
			setScriptError(2);
			return;
		}
	}
	if (sceneID > 0x400) {
		GameObject *newParent = GameObjects::getObjectByIndex(sceneID - 0x400);
		if (newParent == nullptr) {
			setScriptError(0x19);
			return;
		}
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
				saveWalkRuntime(c, object);
				int idx = currentView->getCharacterArrayIndex(c);
				if (idx >= 0)
					currentView->_characters.remove_at(idx);
				delete c;
			}
		}
	}

	// Step 2: Update object fields
	object->_sceneIndex = sceneID;
	object->_position = Common::Point(x, y);

	// Binary (1008:aa83): reload runtime from file when object enters the active scene
	// (direct placement, protagonist inventory, or container visible in current scene).
	const bool destInScene = (sceneID == currentScene);
	const bool destInInventory = (sceneID == actorIndex + 0x400);
	bool destInContainerInScene = false;
	if (sceneID > 0x400) {
		GameObject *destParent = GameObjects::getObjectByIndex(sceneID - 0x400);
		destInContainerInScene = destParent != nullptr && destParent->_sceneIndex == currentScene;
	}
	if (destInScene || destInInventory || destInContainerInScene) {
		_engine->loadObjectData(object);
	}

	currentView->rebuildCharacterLookupTable();

	// Step 3: Add to render list if object is now visible in current scene.
	const Common::Point destPos(x, y);
	auto placeCharacterInScene = [&](Character *c) {
		if (c == nullptr)
			return;
		// Binary (1008:aa83): runtime[+0x00,+0x02,+0x08,+0x0a] = object x/y after move.
		c->setPosition(destPos);
		resetCharacterWalkPath(c);
		clearStoredWalkRuntime(object);
		object->resetDrawBounds();
	};

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
			placeCharacterInScene(c);
		}
	} else if (sceneID == currentScene) {
		// Actor moved into current scene - add to render list if not already present
		Character *c = currentView->getCharacterByIndex(objectID);
		if (c == nullptr) {
			c = new Character();
			c->_gameObject = object;
			currentView->_characters.push_back(c);
		}
		placeCharacterInScene(c);
	} else {
		// Actor moved out of current scene - remove from render list
		Character *c = currentView->getCharacterByIndex(objectID);
		if (c != nullptr) {
			saveWalkRuntime(c, object);
			int idx = currentView->getCharacterArrayIndex(c);
			if (idx >= 0)
				currentView->_characters.remove_at(idx);
			delete c;
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

	// Check from sortObjectsByDepth (1008:0da6): if the moved object is the active
	// inventory item cursor, clear it and reset cursor mode from UseInventory to Use.
	if (currentView->_activeInventoryItem != nullptr &&
		currentView->_activeInventoryItem->_index == objectID) {
		currentView->_activeInventoryItem = nullptr;
		if (currentView->_savedCursorMode == Script::MouseMode::UseInventory) {
			currentView->_savedCursorMode = Script::MouseMode::Use;
		}
		if (_cursorModeBeforeWait == MouseMode::UseInventory) {
			_cursorModeBeforeWait = MouseMode::Use;
		}
		if (_cursorMode == MouseMode::UseInventory) {
			_engine->setCursorMode(MouseMode::Use);
			currentView->updateCursor();
		}
	}

	// Step 5: If object has no runtime data (original: puVar5[5]==0 && puVar5[6]==0)
	// and was the UseInventory target, reset cursor to Use (0x15).
	// Also clear _activeInventoryItem if it was this object.
	if (object->_dataOffset == 0) {
		if (_interactedObjectID == objectID + 0x400 && _cursorMode == MouseMode::UseInventory) {
			_engine->setCursorMode(MouseMode::Use);
			currentView->_activeInventoryItem = nullptr;
			currentView->updateCursor();
		}
		// Original also rewrites the saved (pre-wait) cursor mode: 0x17 -> 0x15.
		if (_interactedObjectID == objectID + 0x400 && _cursorModeBeforeWait == MouseMode::UseInventory) {
			_cursorModeBeforeWait = MouseMode::Use;
		}
	}

	// Step 6: If moved object is the one whose script is currently executing,
	// terminate its script (original: sets scriptEndPosition=0, scriptPosition=0)
	if ((int)objectID == _executingScriptObjectId) {
		_stream->seek(0, SEEK_END);
	}

	// Binary (1008:aa83): when runtime data exists, sync target/finalDest to the new
	// object position so a subsequent waitForWalk completes immediately.
	if (object->_dataOffset != 0) {
		GameObject::StoredWalkRuntime &s = object->_storedWalkRuntime;
		s.valid = true;
		s.targetPosition = object->_position;
		s.pathFinalDestination = object->_position;
		s.currentPathIndex = 0;
		s.path.clear();
		s.stepDeltaX = 0;
		s.stepDeltaY = 0;
		s.stepError = 0;
		s.stepDirectionSet = false;
	}

	currentView->rebuildCharacterLookupTable();
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
	// Binary: g_wRepeatRunFlag = 0, g_wIsSceneInitRun = 0 at entry.
	_repeatRunFlag = false;
	_isSceneInitRun = false;
	const uint32 newSceneID = scriptReadValue32();
	const uint16 transitionMode = scriptReadValue16();
	const uint16 transitionSpeed = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::changeScene(newSceneID=%u, transitionMode=%u, transitionSpeed=%u)", newSceneID, transitionMode, transitionSpeed);

	if (newSceneID == 0 || newSceneID > 0x200) {
		setScriptError(3);
		endTimer();
		endFrameWait();
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	if (transitionMode == 0 && (transitionSpeed == 0 || transitionSpeed > 0x40)) {
		setScriptError(0x26);
		endTimer();
		endFrameWait();
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	if (transitionMode > 1) {
		setScriptError(4);
		endTimer();
		endFrameWait();
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}

	// Binary scriptChangeScene (1008:ad6e): beginFrame, hourglass cursor, flipScreen
	// before loading the new scene. Also drop stale text/dialogue flags from the
	// previous scene so blocking waits are not misclassified as UI-click waits.
	clearScriptUiWaitState();
	_engine->setCursorMode(MouseMode::Disabled);

	View1 *currentView = (View1 *)_engine->findView("View1");
	byte savedPalette[768];
	if (currentView != nullptr && transitionMode == 0 && transitionSpeed != 0 &&
		!currentView->isHelpButtonDisabled()) {
		memcpy(savedPalette, g_engine->_palVanilla, sizeof(savedPalette));
	}
	g_engine->changeScene(newSceneID, false);
	// Binary (1008:ad6e): fade old palette to black after scene load (mode 0),
	// then init script, draw, then fade new palette in via fadePaletteFromBlack.
	if (currentView != nullptr && transitionMode == 0 && transitionSpeed != 0 &&
		!currentView->isHelpButtonDisabled()) {
		currentView->fadePaletteToBlack(transitionSpeed, savedPalette);
	} else if (currentView != nullptr && transitionMode == 1 &&
			   !currentView->isHelpButtonDisabled()) {
		currentView->instantSceneCut();
	} else if (currentView != nullptr && currentView->isHelpButtonDisabled()) {
		g_engine->applyScenePaletteEffect();
		currentView->restoreUiPaletteEntries();
	}
	// Binary step 6-7: init script pass (g_wIsSceneInitRun=1), draw scene, then palette restore.
	// Binary (1008:ad6e): two separate runScriptExecutor calls - init then repeat.
	_terminateOuterScriptBeforeRepeat = true;
	beginSceneEntryInitPass();
	if (_deferredRepeatAfterInit) {
		// Init paused on wait - repeat runs when init completes via loadNextScript.
	} else {
		_isSceneInitRun = false;
	}
	if (currentView != nullptr)
		currentView->redraw();

	if (currentView != nullptr && transitionMode == 0 && transitionSpeed != 0 &&
		!currentView->isHelpButtonDisabled()) {
		currentView->startFadingWithSpeed(transitionSpeed);
	}

	// Binary step 8: set cursor to Walk (0x16) after scene change
	_engine->setCursorMode(Script::MouseMode::Walk);
	_interactedObjectID = 0;
	_interactedInventoryItemId = 0;
	// Binary: after init completes synchronously, terminate outer script context and
	// run repeat pass (g_wRepeatRunFlag=1). Error 0x17 only when init paused on wait
	// (g_wScriptIsExecuting != 0); trailing bytes after opcode 0x0C do not block repeat.
	finishSceneEntryRepeatPass(true);
	// NOTE: EndTimer prevents race conditions from overlapping waits

	endTimer();
	if (!isScriptWaitDeferred())
		endFrameWait();
	endBuffering(_lastOpcodeTriggeredSkip);
	// Binary scriptChangeScene (1008:ad6e) returns void after synchronous init/repeat
	// passes. Returning WaitingForCallback here made the outer step() disable the cursor
	// again and strand the executor when the repeat pass had already finished (e.g.
	// re-entering scene 6 from the bar).
	if (!isScriptWaitDeferred() && _state == ExecutorState::WaitingForCallback)
		_state = ExecutorState::Executing;
	return isScriptWaitDeferred() ? ExecutionResult::WaitingForCallback
								  : ExecutionResult::ScriptFinished;
}

ExecutionResult Script::ScriptExecutor::scriptShowDialogue() {
	// Show a dialogue option (1008:b2a8).
	debugC(kDebugScript, "scriptShowDialogue: walkTarget=%d", _walkTargetObjectIndex);
	const uint32 objectID = scriptReadValue32() - 0x400;
	const uint16 x = scriptReadValue16();
	const uint16 y = scriptReadValue16();
	const uint16 side = scriptReadValue16();
	const uint32 offset = readUint16();
	const uint32 numLines = readUint16();
	debugC(kDebugScript, "SCRIPT::showDialogue(objectID=%u, x=%u, y=%u, side=%u, textOffset=%u, numLines=%u)", objectID, x, y, side, offset, numLines);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	GameObject *speaker = GameObjects::getObjectByIndex(objectID);
	if (speaker == nullptr) {
		setScriptError(0x19);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	if (speaker->_dataOffset == 0) {
		setScriptError(2);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	if (side > 1) {
		setScriptError(5);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	// Binary: bSlotLoaded for portrait slots 0x12 and 0x13 (runtime+0x153, +0x163).
	if (speaker->_blobs.size() < 19 || speaker->_blobs[17].empty() || speaker->_blobs[18].empty()) {
		setScriptError(6);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");

	Common::Array<Common::String> strings;
	if (_executingScriptObjectId == 0) {
		strings = g_engine->decodeStrings(Scenes::instance()._currentSceneStrings, offset, numLines, Scenes::instance()._currentSceneIndex, 0);
	} else {
		Common::MemoryReadStream *s = GameObjects::readGameObjectStrings(_executingScriptObjectId, g_engine->_fileStream);
		strings = g_engine->decodeStrings(s, offset, numLines, 0, _executingScriptObjectId);
		delete s;
	}

	debugC(kDebugScript,
		   "Opcode 0D dialogue: speaker=%u rawPos=(%u,%u) side=%u textOffset=%u numLines=%u scriptObject=%u text=\"%s\"",
		   objectID, x, y, side, offset, numLines, _executingScriptObjectId, joinDebugStrings(strings).c_str());

	_dialogueSpeakerObjectID = objectID;
	currentView->showSpeechAct(objectID, strings, Common::Point(x, y), side);

	_waitingForUiClick = true;

	// Binary scriptShowDialogue (1008:b490): if cursor was Disabled (0x1A), restore Walk (0x16).
	if (_cursorMode == MouseMode::Disabled) {
		_engine->setCursorMode(MouseMode::Walk);
		currentView->updateCursor();
	}

	// NOTE: EndTimer prevents race conditions from overlapping waits
	endTimer();
	endFrameWait();
	endBuffering(_lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

void Script::ScriptExecutor::scriptWalkToPosition() {
	// Binary scriptWalkToPosition (1008:b843):
	// Sets up runtime movement state. Does NOT block - walkAlongPath handles
	// actual movement per-frame from the game tick.
	const uint32 objectID = scriptReadValue32() - 0x400;
	const int16 x = (int16)scriptReadValue16();
	const int16 y = (int16)scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::walkToPosition(objectID=%u, x=%d, y=%d)", objectID, x, y);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *c = currentView ? getOrCreateCharacter((uint16)objectID) : nullptr;

	// Binary (1008:b843): inlined walk-runtime setup on heap block at object+0x0A.
	// No shared subroutine in MCSEXEC.EXE. ScummVM maps that block via Character when
	// on-screen, or a transient Character + saveWalkRuntime() when off-screen.
	Character stackCharacter;
	if (c == nullptr) {
		stackCharacter._gameObject = object;
		c = &stackCharacter;
	}

	const Common::Point current = c->getPosition();
	const Common::Point target(x, y);

	c->_currentPathIndex = 0;
	c->_path.clear();
	c->_pathFinalDestination = target;

	bool directPath = _engine->isPathWalkable(y, x, current.y, current.x);
	if (!directPath && Macs2Engine::isWalkabilityWalkable(_engine->getWalkabilityAt(y, x))) {
		c->calculatePath(target);
	}
	if (c->_path.empty()) {
		c->_targetPosition = c->_pathFinalDestination;
	}

	c->_stepDeltaX = (int16)ABS((int32)c->_targetPosition.x - current.x);
	c->_stepDeltaY = (int16)ABS((int32)c->_targetPosition.y - current.y);
	c->_stepError = 0;
	c->_stepDirectionSet = false;

	// Binary loadObjectData seeds runtime+0x21D from the object table vertical offset;
	// scriptWalkToPosition does not change it. Match that so waitForWalk can complete
	// when no scriptSetMotion vertical interpolation is active.
	c->_motionTargetVerticalOffset = object->_verticalOffsetScale;

	saveWalkRuntime(c, object);
}

ExecutionResult Script::ScriptExecutor::scriptWaitForWalk() {
	// Wait for walk completion from executeOpcodes (1008:db56).
	// Original: validates object, checks runtime data exists, checks frozen flag,
	// sets g_wWalkTargetObjectIndex, hides cursor, returns to gameTick.
	// gameTick checks walk completion each frame.
	const uint32 objectID = scriptReadValue32() - 0x400;
	debugC(kDebugScript, "SCRIPT::waitForWalk(objectID=%u)", objectID);
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	GameObject *walkObject = GameObjects::getObjectByIndex(objectID);
	if (walkObject == nullptr) {
		setScriptError(0x19);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	// Binary (1008:db56 opcode 0x11): object+0x0A runtime must exist; no on-screen Character required.
	if (walkObject->_dataOffset == 0) {
		setScriptError(2);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	// Original checks runtime+0x231 (frozen/attached flag) -> error 0x1F
	if (walkObject->_hasBoundsAttachment) {
		setScriptError(0x1F);
		endBuffering(_lastOpcodeTriggeredSkip);
		return ExecutionResult::ScriptFinished;
	}
	_walkTargetObjectIndex = objectID;
	endTimer();
	endBuffering(_lastOpcodeTriggeredSkip);
	debugLogActorWalkState("waitForWalk start");
	enterBlockingWaitCursor();
	// Binary: opcode 0x11 exits executeOpcodes with g_wScriptIsExecuting still true.
	// runScriptExecutor returns immediately (no object iteration, no cursor restore).
	// ScummVM equivalent: return WaitingForCallback so step() pauses execution.
	// step() handles cursor save/set to Disabled automatically.
	return ExecutionResult::WaitingForCallback;
}

void Script::ScriptExecutor::scriptSkipUntil14() {
	// scriptSkipUntil14 @ 1008:a439: read tag, scan from script start for opcode 0x14
	// with matching tag, then continue execution after that label (not at 0x13 block end).
	const uint16 tag = readUint16();
	debugC(kDebugScript, "SCRIPT::skipUntil14(tag=%.4x)", tag);
	_stream->seek(0, SEEK_SET);
	while (_stream->pos() < _stream->size()) {
		uint8 opcode = readByte();
		uint8 length = readByte();
		if (opcode == 0x14) {
			uint16 tag14 = readUint16();
			if (tag14 == tag) {
				_expectedEndLocation = _stream->pos();
				return;
			}
		} else {
			_stream->seek(length, SEEK_CUR);
		}
	}
	setScriptError(0x20);
	_expectedEndLocation = _stream->pos();
}

void Script::ScriptExecutor::scriptSkipWord() {
	// If we reach opcode 14 regularly, just discard the payload and continue.
	const uint16 val = readUint16();
	debugC(kDebugScript, "SCRIPT::skipWord(%u)", val);
}

void Script::ScriptExecutor::scriptClearDialogueChoices() {
	// Mark that we are gathering strings for setting up a dialogue choice.
	_dialogueChoices.clear();
	_chosenDialogueOption = 0;
	_dialogueChoiceScriptIndices.clear();
	debugC(kDebugScript, "SCRIPT::clearDialogueChoices()");
}

void Script::ScriptExecutor::scriptAddDialogueChoice() {
	// Add a dialogue choice.
	const uint16 index = scriptReadValue16();
	const uint16 offset = readUint16();
	const uint16 numLines = readUint16();

	// Binary (1008:c75a): if choice count already 16, set error 0x0E and discard entry.
	if (_dialogueChoices.size() >= 16) {
		setScriptError(0x0E);
		return;
	}

	// Binary stores this index at scene+0x5351+choiceIndex*6 (first field of each 6-byte entry).
	// handleTimerClick (1008:d53b) stores it at scene+0x53B7 as the chosen result.
	_dialogueChoiceScriptIndices.push_back(index);
	// We don't save the index, instead we make sure that we add them in the right
	// order and use the array to keep track.
	// TODO: Removed this assert, during the dialogue in the beginning of chapter
	// 3 (at the fort) an index of 3 came up when only one item had been there before.
	// Not sure if the way of handling it still works or reflects the game, needs
	// to be tested.
	// assert(index - 1 == DialogueChoices.size());
	debugC(kDebugScript, "SCRIPT::addDialogueChoice: index=%u textOffset=%u numLines=%u scriptObject=%u", index, offset, numLines, _executingScriptObjectId);
	Common::StringArray lines;
	if (_executingScriptObjectId == 0) {
		lines = _engine->decodeStrings(Scenes::instance()._currentSceneStrings, offset, numLines, Scenes::instance()._currentSceneIndex, 0);
	} else {
		Common::MemoryReadStream *stringsStream = GameObjects::readGameObjectStrings(_executingScriptObjectId, g_engine->_fileStream);
		lines = _engine->decodeStrings(stringsStream, offset, numLines, 0, _executingScriptObjectId);
		delete stringsStream;
	}
	debugC(kDebugScript,
		   "Opcode 16 choice text: index=%u textOffset=%u numLines=%u scriptObject=%u text=\"%s\"",
		   index, offset, numLines, _executingScriptObjectId, joinDebugStrings(lines).c_str());
	_dialogueChoices.push_back(lines);
}

ExecutionResult Script::ScriptExecutor::scriptShowDialogueChoice() {
	// Finish the dialogue choice.
	const uint32 x = scriptReadValue32();
	const uint32 y = scriptReadValue32();
	const uint16 side = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::showDialogueChoice: walkTarget=%d", _walkTargetObjectIndex);
	const uint16 speakerObjectID = Scenes::instance()._currentActorIndex;
	debugC(kDebugScript,
		   "Opcode 17 choice box: speaker=%u rawPos=(%u,%u) side=%u choiceCount=%u",
		   speakerObjectID, x, y, side, _dialogueChoices.size());
	View1 *currentView = (View1 *)_engine->findView("View1");
	currentView->showDialogueChoice(speakerObjectID, _dialogueChoices, Common::Point(x, y), side);

	_waitingForUiClick = true;

	// Binary scriptShowDialogueChoice (1008:ceb9): if cursor was Disabled (0x1A), restore Walk (0x16).
	if (_cursorMode == MouseMode::Disabled) {
		_engine->setCursorMode(MouseMode::Walk);
		currentView->updateCursor();
	}

	// NOTE: EndTimer prevents race conditions from overlapping waits
	endTimer();
	endBuffering(_lastOpcodeTriggeredSkip);
	return ExecutionResult::WaitingForCallback;
}

ExecutionResult Script::ScriptExecutor::scriptDismissPanel() {
	// Set the stream to the end and let the calling code figure out that we are done
	// for this run.
	debugLogActorWalkState("dismissPanel (opcode 0x18)");
	_stream->seek(_stream->size(), SEEK_SET);
	endBuffering(_lastOpcodeTriggeredSkip);
	return ExecutionResult::ScriptFinished;
}

void Script::ScriptExecutor::scriptWalkToAndPickup() {
	// Walk to and pick up an object (1008:c475).
	// Binary returns immediately if pickup already in progress, without reading params.
	if (_pickupInProgress) {
		return;
	}

	const uint32 actorIndex = scriptReadValue32() - 0x400;
	const uint32 objectIndex = scriptReadValue32() - 0x400;
	debugC(kDebugScript, "SCRIPT::walkToAndPickup(actor=%u, object=%u)", actorIndex, objectIndex);

	clearScriptError();
	if (_cursorMode != MouseMode::Disabled) {
		_cursorModeBeforeWait = _cursorMode;
	}
	_engine->setCursorMode(MouseMode::Disabled);

	if (actorIndex < 1 || actorIndex > 0x200 || objectIndex < 1 || objectIndex > 0x200) {
		setScriptError(2);
		return;
	}

	GameObject *actorObject = GameObjects::getObjectByIndex(actorIndex);
	GameObject *targetObject = GameObjects::getObjectByIndex(objectIndex);
	if (actorObject == nullptr || targetObject == nullptr) {
		setScriptError(0x19);
		return;
	}

	if (targetObject->_dataOffset != 0) {
		_engine->loadObjectData(targetObject);
		if (hasScriptError())
			return;
	}

	if (actorObject->_dataOffset == 0) {
		setScriptError(0x19);
		return;
	}

	if (actorIndex == objectIndex || targetObject->_sceneIndex == actorObject->_index) {
		setScriptError(2);
		return;
	}

	if (actorObject->_hasBoundsAttachment) {
		setScriptError(0x1F);
		return;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	if (actorObject->_sceneIndex == targetObject->_sceneIndex) {
		// Binary (1008:c475): duplicates the inlined walk-runtime block from 1008:b843,
		// then sets g_wPickupInProgress. ScummVM: startPickup() mirrors that inline block.
		Character *actor = currentView ? getOrCreateCharacter((uint16)actorIndex) : nullptr;
		if (actor != nullptr) {
			currentView->_activeInventoryItem = nullptr;
			currentView->updateCursor();
			_pickupInProgress = true;
			_pickupActorObjectID = actorIndex;
			_pickupTargetObjectID = objectIndex;
			actor->startPickup(targetObject);
			saveWalkRuntime(actor, actorObject);
		}
	}

	if (!hasScriptError()) {
		_walkTargetObjectIndex = actorIndex;
	}
}

bool Script::ScriptExecutor::scriptSetPickupFrames() {
	const int32 objectID = (int32)scriptReadValue32() - 0x400;
	const uint16 frameStart = scriptReadValue16();
	const uint16 frameEnd = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setPickupFrames(objectID=%d, frameStart=%u, frameEnd=%u)", objectID, frameStart, frameEnd);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return true;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return true;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return true;
	}

	object->_pickupFrameStart = frameStart;
	object->_pickupFrameEnd = frameEnd;
	return true;
}

void Script::ScriptExecutor::scriptSetupObject() {
	const int32 objectID = (int32)scriptReadValue32() - 0x400;
	const uint16 slotID = scriptReadValue16();
	const uint16 value = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setupObject(objectID=%d, slotID=%u, value=%u)", objectID, slotID, value);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}

	if (slotID < 1 || slotID > 0x15) {
		setScriptError(0x10);
		return;
	}

	// Binary: runtime+slot*0x10+0x33 (bSlotLoaded) must be set.
	if (!object->isAnimSlotLoaded(slotID)) {
		setScriptError(0x10);
		return;
	}

	// Binary writes to runtime+slot*0x10+0x30 which is slot_base+0x0C = wAnimSpeed.
	if ((uint)(slotID - 1) < object->_blobWalkSpeeds.size()) {
		object->_blobWalkSpeeds[slotID - 1] = value;
	}
}

void Script::ScriptExecutor::scriptSetSkippable() {
	// Sets g_wScriptSkippable [102Ah] = 1.
	_scriptSkippable = true;
	debugC(kDebugScript, "SCRIPT::setSkippable()");
}

void Script::ScriptExecutor::scriptClearSkippable() {
	// Sets g_wScriptSkippable [102Ah] = 0.
	_scriptSkippable = false;
	debugC(kDebugScript, "SCRIPT::clearSkippable()");
}

void Script::ScriptExecutor::scriptPlayAnimation() {
	// scriptPlayAnimation (1008:bd58).
	const uint32 objectID = scriptReadValue32() - 0x400;
	const uint32 slotID = scriptReadValue16();
	const int16 frameOffset = (int16)scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::playAnimation(objectID=%d, slotID=%u, frameOffset=%d)", objectID, slotID, frameOffset);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *gameObject = GameObjects::getObjectByIndex(objectID);
	if (gameObject == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (gameObject->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	if (slotID < 1 || slotID > 0x15) {
		setScriptError(0x10);
		return;
	}
	if (!gameObject->isAnimSlotLoaded((uint16)slotID)) {
		setScriptError(0x10);
		return;
	}

	Common::Array<uint8> *blob = gameObject->getAnimSlotBlob((uint16)slotID);
	if (blob == nullptr || blob->empty()) {
		setScriptError(0x10);
		return;
	}

	AnimBlobView view(*blob);
	if (!view.isValid()) {
		setScriptError(8);
		return;
	}
	// Binary getAnimFrameCount (1010:168c) returns sequence length (blob+0x0A+1),
	// not the pixel frame count word at frameDataOffset. advanceAnimFrame uses
	// frameOffset+0x64 as a sequence position index.
	const uint16 seqLength = view.sequenceLength();
	if (frameOffset < 0 || (uint16)frameOffset > seqLength) {
		setScriptError(0x12);
		return;
	}

	BackgroundAnimationBlob::advanceAnimFrame(*blob, true, (uint16)frameOffset + 0x64);
}

void Script::ScriptExecutor::scriptTestPathfinding() {
	const uint32 objectID = scriptReadValue32() - 0x400;
	const uint32 x = scriptReadValue32();
	const uint32 y = scriptReadValue32();
	debugC(kDebugScript, "SCRIPT::testPathfinding(objectID=%d, target=(%u,%u))", objectID, x, y);

	clearScriptError();
	_pathWalkableResult = false;
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	_pathWalkableResult = _engine->isPathWalkable(y, x, object->_position.y, object->_position.x);
}

Character *Script::ScriptExecutor::getOrCreateCharacter(uint16 objectID) {
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr || object->_dataOffset == 0)
		return nullptr;
	if (object->_sceneIndex != (uint16)Scenes::instance()._currentSceneIndex)
		return nullptr;
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView == nullptr)
		return nullptr;
	Character *c = currentView->getCharacterByIndex(objectID);
	if (c == nullptr) {
		c = new Character();
		c->_gameObject = object;
		c->_motionTargetVerticalOffset = object->_verticalOffsetScale;
		currentView->_characters.push_back(c);
		restoreWalkRuntime(c, object);
	}
	return c;
}

void Script::ScriptExecutor::saveWalkRuntime(const Character *c, GameObject *o) {
	if (c == nullptr || o == nullptr)
		return;
	GameObject::StoredWalkRuntime &s = o->_storedWalkRuntime;
	s.valid = true;
	s.targetPosition = c->_targetPosition;
	s.pathFinalDestination = c->_pathFinalDestination;
	s.stepDeltaX = c->_stepDeltaX;
	s.stepDeltaY = c->_stepDeltaY;
	s.stepError = c->_stepError;
	s.stepDirectionSet = c->_stepDirectionSet;
	s.currentPathIndex = c->_currentPathIndex;
	s.path = c->_path;
	s.motionTargetVerticalOffset = c->_motionTargetVerticalOffset;
	s.motionVerticalOffsetDelta = c->_motionVerticalOffsetDelta;
	s.motionDistanceUnits = c->_motionDistanceUnits;
	s.motionProgress = c->_motionProgress;
}

void Script::ScriptExecutor::restoreWalkRuntime(Character *c, const GameObject *o) {
	if (c == nullptr || o == nullptr || !o->_storedWalkRuntime.valid)
		return;
	const GameObject::StoredWalkRuntime &s = o->_storedWalkRuntime;
	c->_targetPosition = s.targetPosition;
	c->_pathFinalDestination = s.pathFinalDestination;
	c->_stepDeltaX = s.stepDeltaX;
	c->_stepDeltaY = s.stepDeltaY;
	c->_stepError = s.stepError;
	c->_stepDirectionSet = s.stepDirectionSet;
	c->_currentPathIndex = s.currentPathIndex;
	c->_path = s.path;
	c->_motionTargetVerticalOffset = s.motionTargetVerticalOffset;
	c->_motionVerticalOffsetDelta = s.motionVerticalOffsetDelta;
	c->_motionDistanceUnits = s.motionDistanceUnits;
	c->_motionProgress = s.motionProgress;
}

void Script::ScriptExecutor::clearStoredWalkRuntime(GameObject *o) {
	if (o == nullptr)
		return;
	o->_storedWalkRuntime = GameObject::StoredWalkRuntime();
}

void Script::ScriptExecutor::seedMoveToPositionState(GameObject *object, Character *c, const Common::Point &target, uint16 targetVerticalOffset) {
	const Common::Point current = c ? c->getPosition() : object->_position;
	GameObject::StoredWalkRuntime &s = object->_storedWalkRuntime;
	s.valid = true;
	s.currentPathIndex = 0;
	s.path.clear();
	s.targetPosition = target;
	s.pathFinalDestination = target;
	s.stepDeltaX = (int16)ABS((int32)target.x - current.x);
	s.stepDeltaY = (int16)ABS((int32)target.y - current.y);
	s.stepError = 0;
	s.stepDirectionSet = false;
	s.motionProgress = 0;
	s.motionTargetVerticalOffset = targetVerticalOffset;
	s.motionVerticalOffsetDelta = (uint16)ABS((int32)object->_verticalOffsetScale - (int32)targetVerticalOffset);
	s.motionDistanceUnits = (uint16)(s.stepDeltaX + s.stepDeltaY);
	if (c != nullptr)
		restoreWalkRuntime(c, object);
}

void Script::ScriptExecutor::seedMotionState(GameObject *object, Character *c, uint16 targetVerticalOffset, uint16 verticalOffsetDelta, uint16 motionDistance) {
	GameObject::StoredWalkRuntime &s = object->_storedWalkRuntime;
	s.valid = true;
	s.motionTargetVerticalOffset = targetVerticalOffset;
	s.motionVerticalOffsetDelta = verticalOffsetDelta;
	s.motionDistanceUnits = motionDistance;
	s.motionProgress = 0;
	if (c != nullptr) {
		c->_motionTargetVerticalOffset = targetVerticalOffset;
		c->_motionVerticalOffsetDelta = verticalOffsetDelta;
		c->_motionDistanceUnits = motionDistance;
		c->_motionProgress = 0;
	}
}

void Script::ScriptExecutor::saveOpenInventoryScriptContext() {
	// scriptOpenInventory (1008:c3e6): g_wSavedScriptPosition / End / ExecutingObjectId.
	_savedOpenInventoryScriptPos = _stream ? _stream->pos() : 0;
	_savedOpenInventoryScriptEndPos = _stream ? _stream->size() : 0;
	_savedOpenInventoryExecutingObjectId = _executingScriptObjectId;
	_secondaryInventoryLocation = _savedOpenInventoryScriptPos;
}

void Script::ScriptExecutor::restoreOpenInventoryScriptContext() {
	// handleInput panel state 3 + button 6 (1008:e8bf): restore saved script context.
	const uint16 savedObjId = _savedOpenInventoryExecutingObjectId;
	if (savedObjId != 0 && savedObjId <= 0x200) {
		GameObject *obj = GameObjects::getObjectByIndex(savedObjId);
		if (obj == nullptr || obj->_script.empty()) {
			return;
		}
		if (_stream && _stream != Scenes::instance()._currentSceneScript) {
			delete _stream;
		}
		_stream = obj->getScriptStream();
		_executingScriptObjectId = savedObjId;
		_executingObjectIndex = savedObjId;
		_scriptExecutionState = ScriptExecutionState::ExecutingOtherScripts;
	} else {
		setScript(Scenes::instance()._currentSceneScript);
		_executingScriptObjectId = 0;
		_executingObjectIndex = Scenes::instance()._currentSceneIndex;
		_scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
	}
	if (_stream) {
		_stream->seek(_savedOpenInventoryScriptPos, SEEK_SET);
	}
	_expectedEndLocation = _savedOpenInventoryScriptEndPos;
	_scriptClickFlag = _savedScriptClickFlag;
	_scriptClickX = _savedScriptClickX;
	_scriptClickY = _savedScriptClickY;
	_scriptClickResult = _savedScriptClickResult;
	_state = ExecutorState::Executing;
	_isRunningScript = true;
}

void Script::ScriptExecutor::scriptSetYOffset() {
	// scriptSetYOffset (1008:c047). Sets object field +8 (vertical offset)
	// AND mirrors it into runtime field +0x21D (motion target).
	const int32 objectID = (int32)scriptReadValue32() - 0x400;
	const uint16 offset = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setYOffset(objectID=%d, offset=%u)", objectID, offset);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}

	object->_verticalOffsetScale = offset;
	if (Character *character = getOrCreateCharacter((uint16)objectID)) {
		character->_motionTargetVerticalOffset = offset;
	}
	object->_storedWalkRuntime.valid = true;
	object->_storedWalkRuntime.motionTargetVerticalOffset = offset;
}

void Script::ScriptExecutor::scriptSetMotion() {
	const int32 objectID = (int32)scriptReadValue32() - 0x400;
	const uint16 targetVerticalOffset = scriptReadValue16();
	const uint16 verticalOffsetDelta = scriptReadValue16();
	const uint16 motionDistance = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setMotion(objectID=%d, targetVerticalOffset=%u, verticalOffsetDelta=%u, motionDistance=%u)",
		   objectID, targetVerticalOffset, verticalOffsetDelta, motionDistance);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}

	Character *character = getOrCreateCharacter((uint16)objectID);
	seedMotionState(object, character, targetVerticalOffset, verticalOffsetDelta, motionDistance);
	debugLogActorWalkState("after setMotion");
}

bool Script::ScriptExecutor::scriptSetOrientation() {
	const int32 objectID = (int32)scriptReadValue32() - 0x400;
	const uint16 animIndex = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setOrientation(objectID=%d, animIndex=%u)", objectID, animIndex);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return true;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return true;
	}
	if (animIndex < 9 || animIndex > 0x10) {
		setScriptError(0x14);
		return true;
	}

	object->_orientation = animIndex;
	return true;
}

void Script::ScriptExecutor::scriptMoveToPosition() {
	// Opcode 0x23 scriptMoveToPosition (1008:bafc): seeds runtime walk state directly;
	// does not pathfind or use time-based lerp.
	const int32 objectID = (int32)scriptReadValue32() - 0x400;
	const int16 x = (int16)scriptReadValue16();
	const int16 y = (int16)scriptReadValue16();
	const uint16 targetVerticalOffset = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::moveToPosition(objectID=%d, target=(%d,%d), targetVerticalOffset=%u)", objectID, x, y, targetVerticalOffset);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}

	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}

	const Common::Point target(x, y);
	if (!_engine->isPathWalkable(object->_position.y, object->_position.x, y, x) &&
		Macs2Engine::isWalkabilityWalkable(_engine->getWalkabilityAt(target))) {
		setScriptError(0x15);
		return;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	Character *c = currentView ? getOrCreateCharacter((uint16)objectID) : nullptr;
	seedMoveToPositionState(object, c, target, targetVerticalOffset);
}

void Script::ScriptExecutor::scriptAddValues() {
	// Adds two values read and saves them to a script variable.
	// ;; fn0037_C7E6: 0037:C7E6
	const uint32 a = scriptReadValue32();
	const uint32 b = scriptReadValue32();
	debugC(kDebugScript, "SCRIPT::addValues(a=%u, b=%u)", a, b);

	const uint32 result = a + b;
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
	const uint32 a = scriptReadValue32();
	const uint32 b = scriptReadValue32();
	debugC(kDebugScript, "SCRIPT::subValues(a=%u, b=%u)", a, b);

	const uint32 result = a - b;
	_stream->seek(-6, SEEK_CUR);
	scriptSaveVariableHelper(result);
	_stream->seek(3, SEEK_CUR);
}

void Script::ScriptExecutor::scriptLoadSpecialAnim() {
	// This one loads a special animation set into the overload slot (1008:c991).
	const uint32 id = scriptReadValue32() - 0x400;
	const uint16 shouldMirror = scriptReadValue16();
	const uint8 animationID = readByte();
	debugC(kDebugScript, "SCRIPT::loadSpecialAnim(objectID=%u, animationID=%u, shouldMirror=%u)", id, animationID, shouldMirror);

	clearScriptError();
	if (id < 1 || id > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(id);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}

	const Common::Array<uint8> &blob = Scenes::instance().readSpecialAnimBlob(animationID, g_engine->_fileStream);
	object->_overloadAnimation = blob;
	object->_overloadAnimationMirrored = (shouldMirror != 0);
	if (shouldMirror != 0) {
		BackgroundAnimationBlob::mirrorAnimBlob(object->_overloadAnimation);
	}
	while (object->_blobs.size() <= 20)
		object->_blobs.push_back(Common::Array<uint8>());
	object->_blobs[20] = object->_overloadAnimation;
}

void Script::ScriptExecutor::scriptSetDirection() {
	// scriptSetDirection (1008:c858). Writes to runtime field +0x22D.
	const uint32 characterID = scriptReadValue32() - 0x400;
	const uint16 value = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setDirection(characterID=%u, value=%u)", characterID, value);

	clearScriptError();
	if (characterID < 1 || characterID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(characterID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	object->_overloadAnimTriggerDirection = value;
}

void Script::ScriptExecutor::scriptStopAnimation() {
	// scriptStopAnimation (1008:c8e4).
	const uint32 characterID = scriptReadValue32() - 0x400;
	debugC(kDebugScript, "SCRIPT::stopAnimation(characterID=%u)", characterID);

	clearScriptError();
	if (characterID < 1 || characterID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *obj = GameObjects::getObjectByIndex(characterID);
	if (obj == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (obj->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	obj->_overloadAnimTriggerDirection = 0x7FFF;
	obj->_useOverloadAnimation = false;
	obj->_overloadAnimation.clear();
	if (obj->_blobs.size() > 20)
		obj->_blobs[20].clear();
}

void Script::ScriptExecutor::scriptOpenInventory() {
	const uint32 objectID = scriptReadValue32() - 0x400;
	debugC(kDebugScript, "SCRIPT::openInventory(objectID=%u)", objectID);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *inventorySource = GameObjects::getObjectByIndex(objectID);
	if (inventorySource == nullptr) {
		setScriptError(0x19);
		return;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	_savedExternalInventoryMouseMode = _cursorMode == MouseMode::UseInventory ? MouseMode::Use : _cursorMode;
	_hasPendingExternalInventoryResume = true;
	_externalInventorySourceObjectID = objectID;
	saveOpenInventoryScriptContext();
	// Save script click state (original saves at 0xf94-0xf9a equivalents)
	_savedScriptClickFlag = _scriptClickFlag;
	_savedScriptClickX = _scriptClickX;
	_savedScriptClickY = _scriptClickY;
	_savedScriptClickResult = _scriptClickResult;
	currentView->openInventory(inventorySource);
}

void Script::ScriptExecutor::scriptLoadObjectAnim() {
	const uint16 objectID = scriptReadValue16() - 0x400;
	const uint16 slotID = scriptReadValue16();
	const bool shouldMirror = scriptReadValue16() != 0;
	const uint8 arrayIndex = readByte();

	debugC(kDebugScript, "SCRIPT::loadObjectAnim(objectID=%u, slotID=%u, arrayIndex=%u, shouldMirror=%u)", objectID, slotID, arrayIndex, shouldMirror);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	if (slotID < 1 || slotID > 0x15) {
		setScriptError(0x13);
		return;
	}

	g_engine->loadAnimationFromSceneData(objectID, slotID, arrayIndex, shouldMirror, _executingScriptObjectId);
}

void Script::ScriptExecutor::scriptCheckObjectData() {
	const uint16 objectID = scriptReadValue16() - 0x400;
	debugC(kDebugScript, "SCRIPT::checkObjectData(objectID=%u)", objectID);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}

	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView == nullptr)
		return;

	const Common::Point posBefore = object->_position;
	const bool snapBefore = object->_snapToTarget;

	Character *character = currentView->getCharacterByIndex(objectID);
	const int currentIndex = currentView->getCharacterArrayIndex(character);
	if (currentIndex >= 0) {
		currentView->_characters.remove_at(currentIndex);
		delete character;
		character = nullptr;
	}

	// Binary: sortObjectsByDepth (1008:0d79) then loadObjectData (1008:08ec).
	_engine->sortObjectsByDepth(objectID);
	if (!_engine->loadObjectData(object))
		return;

	debugC(kDebugPath,
		   "WALK::checkObjectData obj=%u pos before=(%d,%d) after=(%d,%d) snap %d->%d (loadObjectData clears runtime)",
		   objectID, posBefore.x, posBefore.y, object->_position.x, object->_position.y,
		   snapBefore ? 1 : 0, object->_snapToTarget ? 1 : 0);

	if (object->_sceneIndex == Scenes::instance()._currentSceneIndex) {
		character = new Character();
		character->_gameObject = object;
		// sortObjectsByDepth cleared stored walk runtime; a fresh Character otherwise
		// leaves _targetPosition at (0,0) and update() drifts toward the origin.
		resetCharacterWalkPath(character);
		character->_motionTargetVerticalOffset = object->_verticalOffsetScale;
		clearStoredWalkRuntime(object);
		currentView->_characters.push_back(character);
	}
	currentView->rebuildCharacterLookupTable();
}

void Script::ScriptExecutor::scriptCheckInventory() {
	const uint16 objectID = scriptReadValue16() - 0x400;
	const uint16 parentID = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::checkInventory(objectID=%u, parentID=%u)", objectID, parentID);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	const GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	_inventoryCheckResult = object->_sceneIndex == parentID;
}

void Script::ScriptExecutor::scriptSetSnapToTarget() {
	const uint16 objectID = scriptReadValue16() - 0x400;
	const bool enabled = scriptReadValue16() != 0;
	debugC(kDebugScript, "SCRIPT::setSnapToTarget(objectID=%u, enabled=%u)", objectID, enabled);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	object->_snapToTarget = enabled;
}

void Script::ScriptExecutor::scriptTestObjectAnimFrame() {
	// scriptTestObjectAnimFrame (1008:be91).
	uint32 objectID = scriptReadValue32() - 0x400;
	uint16 slotID = scriptReadValue16();
	uint16 minFrame = scriptReadValue16();
	uint16 maxFrame = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::testObjectAnimFrame(objectID=%u, slotID=%u, minFrame=%u, maxFrame=%u)", objectID, slotID, minFrame, maxFrame);

	clearScriptError();
	_animBlobRangeTestResult = false;

	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	if (slotID < 1 || slotID > 0x15) {
		setScriptError(0x10);
		return;
	}

	const Common::Array<uint8> *blob = object->getAnimSlotBlob(slotID);
	if (blob == nullptr || blob->empty()) {
		setScriptError(0x10);
		return;
	}

	AnimBlobView view(*blob);
	if (!view.isValid()) {
		setScriptError(8);
		return;
	}
	const uint16 seqPos = view.sequencePosition();
	_animBlobRangeTestResult = seqPos >= minFrame && seqPos <= maxFrame;
}

void Script::ScriptExecutor::scriptPrintStringRight() {
	// Opcode 0x30: scriptPrintString with flag=1 (vs opcode 0x0A with flag=0)
	// which changes behaviour in the function.
	debugC(kDebugScript, "SCRIPT::printStringRight()");
	scriptPrintString(true);
	endBuffering(_lastOpcodeTriggeredSkip);
}

void Script::ScriptExecutor::scriptSetPaletteDarkness() {
	// Binary (1008:ce0b): clamps value to 0..100, writes to sceneData+0x5205
	// (darkenPercent), then calls applyPaletteInterpolation to update displayed palette.
	int16 darkenPercent = (int16)scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setPaletteDarkness(darkenPercent=%d)", darkenPercent);
	if (darkenPercent < 0)
		darkenPercent = 0;
	if (darkenPercent > 100)
		darkenPercent = 100;
	g_engine->_paletteDarkenPercent = (uint16)darkenPercent;
	g_engine->applyPaletteDarkening();
	View1 *view = (View1 *)g_engine->findView("View1");
	if (view)
		view->_paletteDirty = true;
}

void Script::ScriptExecutor::scriptSetObjectShading() {
	// Opcode 0x32 (1008:b9ba): runtime+0x185 bHasShading (NOT clickable - misnamed in script docs).
	const uint16 objectID = scriptReadValue16() - 0x400;
	const uint16 hasShading = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setObjectShading(objectID=%u, hasShading=%u)", objectID, hasShading);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	object->_hasShading = hasShading != 0;
}

void Script::ScriptExecutor::scriptSetObjectScaling() {
	// Opcode 0x33 (1008:ba5b): runtime+0x186 bHasScaling (NOT visible - misnamed in script docs).
	const uint16 objectID = scriptReadValue16() - 0x400;
	const uint16 hasScaling = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setObjectScaling(objectID=%u, hasScaling=%u)", objectID, hasScaling);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0) {
		setScriptError(2);
		return;
	}
	object->_hasScaling = hasScaling != 0;
}

void Script::ScriptExecutor::scriptSetHotspotOverride() {
	// Opcode 0x34 scriptSetHotspotOverride (1008:ce40): scene+0x5BD1 hotspot remap table.
	const uint16 v1 = scriptReadValue16() - 0x800;
	const uint16 v2 = scriptReadValue16() - 0x800;
	debugC(kDebugScript, "SCRIPT::setHotspotOverride(hotspot=%u, override=%u)", v1, v2);

	clearScriptError();
	if (v1 < 1 || v1 > 0x10 || v2 < 1 || v2 > 0x10) {
		setScriptError(0x1e);
		return;
	}
	if (v1 == v2) {
		g_engine->_hotspotOverrides[v1] = 0xFFFF;
	} else {
		g_engine->_hotspotOverrides[v1] = v2;
	}
}

void Script::ScriptExecutor::scriptSetObjectBounds() {
	// Opcode 0x35 scriptSetObjectBounds (1008:c19f): runtime+0x231..+0x238 attachment.
	const uint16 objectID = scriptReadValue16() - 0x400;
	const uint16 otherObjectID = scriptReadValue16() - 0x400;
	const uint16 value1 = scriptReadValue16();
	const uint16 value2 = scriptReadValue16();
	const uint16 value3 = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setObjectBounds(objectID=%u, otherObjectID=%u, value1=%u, value2=%u, value3=%u)", objectID, otherObjectID, value1, value2, value3);

	clearScriptError();
	if (objectID < 1 || objectID > 0x200 || otherObjectID < 1 || otherObjectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex(objectID);
	GameObject *otherObject = GameObjects::getObjectByIndex(otherObjectID);
	if (object == nullptr || otherObject == nullptr) {
		setScriptError(0x19);
		return;
	}
	if (object->_dataOffset == 0 || otherObject->_dataOffset == 0) {
		setScriptError(2);
		return;
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
}

void Script::ScriptExecutor::scriptDismissAllPanels() {
	// Opcode 0x36 -> scriptDismissPanel (1008:d6dd): restore pending UI background only.
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView == nullptr)
		return;
	if (currentView->_pendingPanelRequest == View1::kPanelRequestNone ||
		!currentView->_uiBackgroundRestorePending) {
		return;
	}
	debugC(kDebugScript, "SCRIPT::dismissPanel()");
	currentView->redraw();
	currentView->_uiBackgroundRestorePending = false;
	currentView->_uiPanelState = View1::kUiPanelNone;
	currentView->_pendingPanelRequest = View1::kPanelRequestNone;
	currentView->_isDialogueChoiceInputActive = false;
	currentView->_activeInventoryItem = nullptr;
	currentView->clearClickedButtonIndex();
}

void Script::ScriptExecutor::scriptResetToSceneScript() {
	// scriptResetToSceneScript (1008:ad3e). Resets script execution
	// context back to the current scene script at position 0.
	debugC(kDebugScript, "SCRIPT::resetToSceneScript()");
	_executingScriptObjectId = 0;
	_executingObjectIndex = Scenes::instance()._currentSceneIndex;
	_scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;
	setCurrentSceneScriptAt(0);
	_expectedEndLocation = 0;
}

void Script::ScriptExecutor::scriptLoadOverlayFont() {
	// scriptLoadOverlayFont (1008:d749). Loads a font resource for
	// overlay text into the overlay font buffer.
	uint8 resourceIndex = readByte();
	debugC(kDebugScript, "SCRIPT::loadOverlayFont(resourceIndex=%u)", resourceIndex);
	_overlayTextStageActive = true;
	if (!_engine->loadOverlayFont(resourceIndex, _executingScriptObjectId)) {
		warning("Opcode 0x38: failed to load overlay font resource %u", resourceIndex);
	}
}

void Script::ScriptExecutor::scriptAddOverlayTextEntry() {
	// Opcode 0x3A scriptAddOverlayTextEntry (1008:d82c).
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView == nullptr)
		return;

	clearScriptError();
	const uint16 x = scriptReadValue16();
	const uint16 y = scriptReadValue16();
	const uint8 alignment = scriptReadValue16();
	const uint16 stringOffset = readUint16();
	const uint16 entryType = readUint16();
	debugC(kDebugScript, "SCRIPT::addOverlayTextEntry(x=%u, y=%u, align=%u, stringOffset=%u, entryType=%u)", x, y, alignment, stringOffset, entryType);

	if (!_overlayTextStageActive) {
		setScriptError(0x21);
		return;
	}
	if (currentView->_overlayTextEntries.size() >= 10) {
		setScriptError(0x22);
		return;
	}
	if (entryType != 1) {
		setScriptError(0x23);
		return;
	}

	Common::StringArray strings;
	if (_executingScriptObjectId == 0) {
		strings = _engine->decodeStrings(Scenes::instance()._currentSceneStrings, stringOffset, 1, Scenes::instance()._currentSceneIndex, 0);
	} else {
		Common::MemoryReadStream *stringsStream = GameObjects::readGameObjectStrings(_executingScriptObjectId, g_engine->_fileStream);
		strings = _engine->decodeStrings(stringsStream, stringOffset, 1, 0, _executingScriptObjectId);
		delete stringsStream;
	}
	if (strings.empty()) {
		warning("Empty overlay text entry at offset %u", stringOffset);
		return;
	}
	if (strings[0].size() >= 0x29) {
		setScriptError(0x24);
		return;
	}
	debugC(kDebugScript,
		   "Opcode 3A overlay text: rawPos=(%u,%u) align=%u textOffset=%u entryType=%u scriptObject=%u text=\"%s\"",
		   x, y, alignment, stringOffset, entryType, _executingScriptObjectId, strings[0].c_str());

	View1::OverlayTextEntry entry;
	entry.position = Common::Point(x, y);
	entry.alignment = alignment;
	entry.text = strings[0];
	currentView->addOverlayTextEntry(entry);
}

void Script::ScriptExecutor::scriptClearOverlayText() {
	debugC(kDebugScript, "SCRIPT::clearOverlayText()");
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr) {
		currentView->clearOverlayTextEntries();
	}
}

void Script::ScriptExecutor::scriptFadeToBlack() {
	const uint16 fadeSpeed = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::fadeToBlack(speed=%u)", fadeSpeed);
	View1 *currentView = (View1 *)_engine->findView("View1");
	// Binary (executeOpcodes 0x3C): skip fade when g_wHelpButtonDisabled is set.
	if (currentView != nullptr && currentView->isHelpButtonDisabled()) {
		return;
	}
	if (currentView != nullptr && fadeSpeed != 0) {
		currentView->startFadeToBlack(fadeSpeed);
	}
}

void Script::ScriptExecutor::scriptFrameWait() {
	// The original interpreter stores a frame countdown that is decremented
	// once per game tick, rather than using a wall-clock timer.
	uint16 duration = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::frameWait(duration=%u)", duration);
	debugLogActorWalkState("frameWait start");
	startFrameWait(duration);
	enterBlockingWaitCursor();
	endBuffering(_lastOpcodeTriggeredSkip);
}

void Script::ScriptExecutor::scriptSetPathfinding() {
	// scriptSetPathfinding (1008:c6d7). Sets/clears a pathfinding override.
	// Index must be in range 200..0xEF (walkability values).
	// Writes to scene data at index*5 + 0x4EA5 (enable byte) and +0x4EA6 (value).
	uint16 areaID = scriptReadValue16();
	uint16 active = scriptReadValue16();
	uint16 overrideValue = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::setPathfinding(areaID=%u, active=%u, overrideValue=%u/0x%04x int16=%d walkable=%s)",
		   areaID, active, overrideValue, overrideValue, (int16)overrideValue,
		   (!active || Macs2Engine::isWalkabilityWalkable(overrideValue)) ? "yes" : "NO");
	if (areaID < 200 || areaID > 0xEF) {
		setScriptError(0x0D);
		return;
	}
	if (active) {
		g_engine->setPathfindingOverride(areaID, overrideValue);
	} else {
		g_engine->removePathfindingOverride(areaID);
	}
	debugLogActorWalkState("after setPathfinding");
}

void Script::ScriptExecutor::scriptTestSceneAnimFrame() {
	// scriptTestSceneAnimFrame (1008:b78d).
	uint32 sceneAnimIndex = scriptReadValue32() - 0x1000;
	uint32 minFrame = scriptReadValue32();
	uint32 maxFrame = scriptReadValue32();
	debugC(kDebugScript, "SCRIPT::testSceneAnimFrame(sceneAnimIndex=%u, minFrame=%u, maxFrame=%u)", sceneAnimIndex, minFrame, maxFrame);

	clearScriptError();
	_animBlobRangeTestResult = false;

	if (sceneAnimIndex == 0 || sceneAnimIndex > _engine->_backgroundAnimationsBlobs.size()) {
		setScriptError(8);
		return;
	}
	const BackgroundAnimationBlob &blob = _engine->_backgroundAnimationsBlobs[sceneAnimIndex - 1];
	if (blob._blob.empty()) {
		setScriptError(8);
		return;
	}
	AnimBlobView view(blob._blob);
	if (!view.isValid()) {
		setScriptError(8);
		return;
	}
	const uint16 seqPos = view.sequencePosition();
	_animBlobRangeTestResult = seqPos >= minFrame && seqPos <= maxFrame;
}

void Script::ScriptExecutor::scriptEndOverlayText() {
	debugC(kDebugScript, "SCRIPT::endOverlayText()");
	if (_overlayTextStageActive) {
		_overlayTextStageActive = false;
	}
}

void Script::ScriptExecutor::scriptFadeFromBlack() {
	const uint16 fadeSpeed = scriptReadValue16();
	debugC(kDebugScript, "SCRIPT::fadeFromBlack(speed = %u)", fadeSpeed);
	View1 *currentView = (View1 *)_engine->findView("View1");
	if (currentView != nullptr && currentView->isHelpButtonDisabled()) {
		return;
	}
	if (currentView != nullptr && fadeSpeed != 0) {
		currentView->startFading(fadeSpeed);
	}
}

void Script::ScriptExecutor::scriptLoadPcmSound() {
	const uint8 resourceIndex = readByte();
	Common::Array<uint8> soundData;
	if (!loadSoundResource(soundData, resourceIndex)) {
		warning("Opcode 0x3E: failed to load PCM sound resource %u", resourceIndex);
		return;
	}

	if (_engine->hasCurrentSound() && _soundEnabled) {
		_engine->stopCurrentSound();
	}
	_engine->setCurrentSoundData(soundData);
}

void Script::ScriptExecutor::scriptPlayPcmSound() {
	// Binary (1000:0c7f): no error path; no-op when Sound Blaster disabled.
	if (_soundEnabled) {
		if (!_engine->hasCurrentSound()) {
			warning("Opcode 0x40: playPcmSound with no loaded sound data");
			return;
		}
		_engine->playCurrentSound();
	}
}

bool Script::ScriptExecutor::scriptWaitForSound() {
	if (_soundEnabled && _soundSystemActive) {
		_waitForPcmSound = true;
		endTimer();
		endBuffering(_lastOpcodeTriggeredSkip);
		debugC(kDebugScript, "SCRIPT::waitForSound start (soundPlaying=%d hasSound=%d)",
			   g_engine->isCurrentSoundPlaying() ? 1 : 0, g_engine->hasCurrentSound() ? 1 : 0);
		debugLogActorWalkState("waitForSound start");
		enterBlockingWaitCursor();
		return true;
	}
	debugC(kDebugScript, "SCRIPT::waitForSound skipped (soundEnabled=%d soundSystemActive=%d)",
		   _soundEnabled ? 1 : 0, _soundSystemActive ? 1 : 0);
	return false;
}

void Script::ScriptExecutor::scriptStopPcmSound() {
	if (_soundEnabled) {
		_engine->stopCurrentSound();
	}
}

void Script::ScriptExecutor::scriptLoadMusicSlot() {
	const uint16 slotID = scriptReadValue16();
	const uint8 resourceIndex = readByte();

	clearScriptError();
	if (slotID < 1 || slotID > 2) {
		setScriptError(0x27);
		return;
	}

	// Binary (1000:0cac): cannot reload slot while it is the active playing slot (error 0x28).
	if (_activeMusicSlot == slotID) {
		setScriptError(0x28);
		return;
	}

	Common::Array<uint8> slotData;
	if (loadMusicResource(slotData, resourceIndex)) {
		_musicSlots[slotID - 1] = Common::move(slotData);
	}
}

void Script::ScriptExecutor::scriptPlayMusicSlot() {
	const uint16 slotID = scriptReadValue16();
	const uint16 startMuted = scriptReadValue16();
	const uint16 fadeParam = scriptReadValue16();

	clearScriptError();
	if (slotID < 1 || slotID > 2) {
		setScriptError(0x27);
		return;
	}

	if (!_musicEnabled || !_soundSystemActive) {
		_activeMusicSlot = slotID;
		return;
	}

	if (_activeMusicSlot != 0) {
		_engine->getAdlib()->stopMusic();
		_activeMusicSlot = 0;
	}

	if (_musicSlots[slotID - 1].empty()) {
		warning("Opcode 0x44: playMusicSlot with empty slot %u", slotID);
		_activeMusicSlot = slotID;
		return;
	}

	_engine->getAdlib()->playSongData(_musicSlots[slotID - 1]);
	if (startMuted == 0) {
		_musicControlMode = 1;
		_musicControlStep = fadeParam;
		_musicControlVolume = 0x3F;
		_engine->getAdlib()->setVolume(_engine->scaledMusicVolume(_musicControlVolume));
	} else {
		_musicControlMode = 0;
		_musicControlStep = 0;
		_musicControlVolume = 0;
		_engine->getAdlib()->setVolume(_engine->scaledMusicVolume(0));
	}

	_activeMusicSlot = slotID;
}

void Script::ScriptExecutor::scriptStopMusicSlot() {
	const uint16 slotID = scriptReadValue16();
	const uint16 stopImmediately = scriptReadValue16();
	const uint16 fadeParam = scriptReadValue16();

	clearScriptError();
	if (slotID < 1 || slotID > 2) {
		setScriptError(0x27);
		return;
	}

	if (!_musicEnabled || !_soundSystemActive) {
		_activeMusicSlot = 0;
		return;
	}

	if (_activeMusicSlot == slotID) {
		if (stopImmediately == 0) {
			_musicControlMode = 2;
			_musicControlStep = fadeParam;
			_musicControlVolume = 0;
		} else {
			_engine->getAdlib()->stopMusic();
			_activeMusicSlot = 0;
		}
	}
}

bool Script::ScriptExecutor::scriptWaitForMusic() {
	if (_soundSystemActive && _musicEnabled) {
		_waitForMusicControl = true;
		endTimer();
		endBuffering(_lastOpcodeTriggeredSkip);
		enterBlockingWaitCursor();
		return true;
	}
	return false;
}

void Script::ScriptExecutor::scriptFreeMusicSlot() {
	const uint16 slotID = scriptReadValue16();

	clearScriptError();
	if (slotID < 1 || slotID > 2) {
		setScriptError(0x27);
		return;
	}

	if (_activeMusicSlot == slotID) {
		if (_musicEnabled && _soundSystemActive) {
			_engine->getAdlib()->stopMusic();
		}
		_activeMusicSlot = 0;
	}
	_musicSlots[slotID - 1].clear();
}

void Script::ScriptExecutor::scriptGetObjectX() {
	// Opcode 0x48 scriptGetObjectX (1008:d917): saves object field +0 (X).
	int32 objectID = (int32)scriptReadValue32() - 0x400;

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	scriptSaveVariableHelper(object->_position.x);
}

void Script::ScriptExecutor::scriptGetObjectY() {
	// Opcode 0x49 scriptGetObjectY (1008:d977): saves object field +2 (Y).
	int32 objectID = (int32)scriptReadValue32() - 0x400;

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	scriptSaveVariableHelper(object->_position.y);
}

void Script::ScriptExecutor::scriptGetObjectField8() {
	// Opcode 0x4A scriptGetObjectField8 (1008:d9d8): saves object field +8.
	int32 objectID = (int32)scriptReadValue32() - 0x400;

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	scriptSaveVariableHelper(object->_verticalOffsetScale);
}

void Script::ScriptExecutor::scriptGetObjectOrientation() {
	// Opcode 0x4B scriptGetObjectOrientation (1008:da3a): saves object field +6.
	int32 objectID = (int32)scriptReadValue32() - 0x400;

	clearScriptError();
	if (objectID < 1 || objectID > 0x200) {
		setScriptError(2);
		return;
	}
	GameObject *object = GameObjects::getObjectByIndex((uint16)objectID);
	if (object == nullptr) {
		setScriptError(0x19);
		return;
	}
	scriptSaveVariableHelper(object->_orientation);
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

void Script::ScriptExecutor::scriptSetPathfindingRemap() {
	// scriptSetPathfindingRemap (1008:dafb). Writes to scene+value*5+0x4EA8.
	const uint16 sourceValue = scriptReadValue16();
	const uint16 targetValue = scriptReadValue16();

	clearScriptError();
	if (sourceValue < AREA_OVERRIDE_MIN || sourceValue > AREA_OVERRIDE_MAX ||
		targetValue < AREA_OVERRIDE_MIN || targetValue > AREA_OVERRIDE_MAX) {
		setScriptError(0x0D);
		return;
	}
	g_engine->_areaOverrides[sourceValue - AREA_OVERRIDE_MIN] = targetValue;
}

bool Script::ScriptExecutor::scriptWaitForAdlib() {
	if (_soundSystemActive && _musicEnabled) {
		_waitForAdlibReady = true;
		endTimer();
		endBuffering(_lastOpcodeTriggeredSkip);
		enterBlockingWaitCursor();
		return true;
	}
	return false;
}

void Script::ScriptExecutor::scriptFreePcmSound() {
	debugC(kDebugScript, "SCRIPT::freePcmSound()");
	if (_soundEnabled)
		_engine->stopCurrentSound();
	_engine->clearCurrentSoundData();
}

ExecutionResult Script::ScriptExecutor::executeOpcodes() {
	debugC(kDebugScript, "----- Scripting function entered - scene: %.2x 1014: %.2x 1012: %.2x", Scenes::instance()._currentSceneIndex, _isSceneInitRun, _repeatRunFlag);
	_isRunningScript = true;
	// Confirmed: no interrupt mechanism exists. Wait states (frameWait, walkTarget,
	// pcmSound, musicControl, adlibReady) are resolved by gameTick externally.

	// We use this to keep track of cases where we did not read all information as we should have
	_expectedEndLocation = _stream->pos();
	// The loop comprises the first labels in the file
	for (;;) {
		if (hasScriptError()) {
			break;
		}
		// TODO: Just for breaking out at the moment when end conditions fail to work
		if (_stream->eos()) {
			break;
		}
		// TODO: Probably only one of these is necessary
		if (_stream->size() == 0 || _stream->pos() >= _stream->size() - 1) {
			break;
		}

		// Make sure we have read all the bytes we should have read
		if (_stream->pos() != _expectedEndLocation) {
			warning("Macs2::ScriptExecutor::ExecuteScript resyncing stream: expected pos %u, actual %u (delta %d) after opcode 0x%02x (object %u, script size %u)",
					(uint32)_expectedEndLocation, (uint32)_stream->pos(),
					(int)((int64)_stream->pos() - (int64)_expectedEndLocation),
					_lastOpcode, _executingScriptObjectId, (uint32)_stream->size());
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
		const byte opcode1 = readByte(); // [bp - 1h]
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
		debugC(kDebugScript, "- First block opcode: %.2x %s at pos %u (len expected end %u)", opcode1, opcodeInfo.c_str(), (uint32)(_stream->pos() - 1), (uint32)_expectedEndLocation);
		const byte length = readByte(); // [bp-2h]
		_lastOpcode = opcode1;
		_lastOpcodeStreamPos = _stream->pos() - 2;
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
		} else if (opcode1 == 0x09) {
			scriptNop09();
		} else if (opcode1 == 0x10) {
			scriptWalkToPosition();
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
			scriptWalkToAndPickup();
			endBuffering(_lastOpcodeTriggeredSkip);
			break;
		} else if (opcode1 == 0x1a) {
			scriptSetPickupFrames();
		} else if (opcode1 == 0x1b) {
			scriptSetupObject();
		} else if (opcode1 == 0x1c) {
			scriptSetSkippable();
		} else if (opcode1 == 0x1d) {
			scriptClearSkippable();
		} else if (opcode1 == 0x1e) {
			scriptPlayAnimation();
		} else if (opcode1 == 0x1f) {
			scriptTestPathfinding();
		} else if (opcode1 == 0x20) {
			scriptSetYOffset();
		} else if (opcode1 == 0x21) {
			scriptSetMotion();
		} else if (opcode1 == 0x22) {
			scriptSetOrientation();
		} else if (opcode1 == 0x23) {
			scriptMoveToPosition();
		} else if (opcode1 == 0x24) {
			scriptAddValues();
		} else if (opcode1 == 0x25) {
			scriptSubValues();
		} else if (opcode1 == 0x26) {
			scriptLoadSpecialAnim();
		} else if (opcode1 == 0x27) {
			scriptSetDirection();
		} else if (opcode1 == 0x28) {
			scriptStopAnimation();
		} else if (opcode1 == 0x29) {
			// Binary (1008:e0f9): always terminates script + sets executingObjectId sentinel.
			scriptOpenInventory();
			_stream->seek(_stream->size(), SEEK_SET);
			_executingObjectIndex = 0x201;
			_executingScriptObjectId = 0x201;
			endBuffering(_lastOpcodeTriggeredSkip);
			break;
		} else if (opcode1 == 0x0b) {
			scriptMoveObject();
			if (hasScriptError()) {
				endBuffering(_lastOpcodeTriggeredSkip);
				break;
			}
			// Binary: after scriptMoveObject(), exits if position >= end.
			// scriptMoveObject can advance the stream to end (e.g. if the moved
			// object is the one whose script is executing).
			if (_stream->pos() >= _stream->size()) {
				endBuffering(_lastOpcodeTriggeredSkip);
				break;
			}
		} else if (opcode1 == 0x0c) {
			// Binary: scriptChangeScene (1008:ad6e) runs init+repeat synchronously and
			// returns void; only pause the outer opcode loop when a wait is still active.
			return scriptChangeScene();
		} else if (opcode1 == 0x0d) {
			const ExecutionResult dialogueResult = scriptShowDialogue();
			if (dialogueResult == ExecutionResult::WaitingForCallback)
				return dialogueResult;
			break;
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
			scriptCheckObjectData();
		} else if (opcode1 == 0x2C) {
			scriptCheckInventory();
		} else if (opcode1 == 0x2D) {
			scriptSetSnapToTarget();
		} else if (opcode1 == 0x2E) {
			scriptTestSceneAnimFrame();
		} else if (opcode1 == 0x2F) {
			scriptTestObjectAnimFrame();
		} else if (opcode1 == 0x30) {
			scriptPrintStringRight();
			return ExecutionResult::WaitingForCallback;
		} else if (opcode1 == 0x31) {
			scriptSetPaletteDarkness();
		} else if (opcode1 == 0x32) {
			scriptSetObjectShading();
		} else if (opcode1 == 0x33) {
			scriptSetObjectScaling();
		} else if (opcode1 == 0x34) {
			scriptSetHotspotOverride();
		} else if (opcode1 == 0x35) {
			scriptSetObjectBounds();
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
			scriptAddOverlayTextEntry();
		} else if (opcode1 == 0x3B) {
			scriptClearOverlayText();
		} else if (opcode1 == 0x3C) {
			scriptFadeToBlack();
		} else if (opcode1 == 0x3D) {
			scriptFadeFromBlack();
		} else if (opcode1 == 0x3E) {
			scriptLoadPcmSound();
		} else if (opcode1 == 0x3F) {
			scriptFreePcmSound();
		} else if (opcode1 == 0x40) {
			scriptPlayPcmSound();
		} else if (opcode1 == 0x41) {
			if (scriptWaitForSound()) {
				return ExecutionResult::WaitingForCallback;
			}
		} else if (opcode1 == 0x42) {
			scriptStopPcmSound();
		} else if (opcode1 == 0x43) {
			scriptLoadMusicSlot();
		} else if (opcode1 == 0x44) {
			scriptPlayMusicSlot();
		} else if (opcode1 == 0x45) {
			scriptStopMusicSlot();
		} else if (opcode1 == 0x47) {
			if (scriptWaitForMusic()) {
				return ExecutionResult::WaitingForCallback;
			}
		} else if (opcode1 == 0x46) {
			scriptFreeMusicSlot();
		} else if (opcode1 == 0x48) {
			scriptGetObjectX();
		} else if (opcode1 == 0x49) {
			scriptGetObjectY();
		} else if (opcode1 == 0x4A) {
			scriptGetObjectField8();
		} else if (opcode1 == 0x4B) {
			scriptGetObjectOrientation();
		} else if (opcode1 == 0x4C) {
			scriptClearActorInventory();
		} else if (opcode1 == 0x4D) {
			scriptSetPathfindingRemap();
		} else if (opcode1 == 0x4E) {
			if (scriptWaitForAdlib()) {
				return ExecutionResult::WaitingForCallback;
			}
		} else {
			setScriptError(7);
			endBuffering(_lastOpcodeTriggeredSkip);
			break;
		}
		endBuffering(_lastOpcodeTriggeredSkip);
	}
	_isRunningScript = false;
	if (hasScriptError())
		recordScriptErrorPosition();
	debugC(kDebugScript, "----- Scripting function left");
	return ExecutionResult::ScriptFinished;
}

void ScriptExecutor::run(bool firstRun) {
	// Binary runScriptExecutor (1008:e50c) entry guard:
	// Returns immediately if ANY wait condition is active.
	if (_frameWaitTicksRemaining != 0 || _walkTargetObjectIndex != 0 ||
		_waitForPcmSound || _waitForMusicControl || _waitForAdlibReady) {
		debugC(kDebugScript, "run() blocked by entry guard: frameWait=%d walkTarget=%d sound=%d music=%d adlib=%d",
			   _frameWaitTicksRemaining, _walkTargetObjectIndex,
			   _waitForPcmSound ? 1 : 0, _waitForMusicControl ? 1 : 0,
			   _waitForAdlibReady ? 1 : 0);
		return;
	}

	const bool resumingAfterCallback = (_state == ExecutorState::WaitingForCallback) && !firstRun;
	if (!resumingAfterCallback) {
		clearScriptError();
		// TODO: Not sure if this is the right place and condition to reset this
		// variable. Context here is that we might have an object that triggers several
		// description strings in a row, and we would disable the executing object
		// if we always reset this object
		// TODO: Watch out for issues caused by this
		_executingScriptObjectId = 0;
		_repeatRunFlag = false;
		_isSceneInitRun = firstRun;
	}
	_state = ExecutorState::Executing;
	step();
}

void ScriptExecutor::setScript(Common::MemoryReadStream *stream) {
	_stream = stream;
}

void ScriptExecutor::releaseObjectStream() {
	if (_stream && _stream != Scenes::instance()._currentSceneScript) {
		delete _stream;
		_stream = nullptr;
	}
}

void ScriptExecutor::setCurrentSceneScriptAt(uint32 offset) {
	setScript(Scenes::instance()._currentSceneScript);
	_stream->seek(offset, SEEK_SET);
}

void ScriptExecutor::tick() {
	// Music fade tick is handled in View1::tick() (matching binary gameTick).
	// Sound/music/adlib wait handling moved to View1::tick() to match binary
	// gameTick cascading if/else structure (drawScene runs before resume).

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
		// Handled by View1::tick() to match binary ordering:
		// drawScene(1) runs BEFORE script resumes when frameWait expires.
		if (_frameWaitTicksRemaining > 0) {
			--_frameWaitTicksRemaining;
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

uint32 ScriptExecutor::getDebugOpcodePosition() const {
	if (_state == ExecutorState::WaitingForCallback)
		return _lastOpcodeStreamPos;
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
		if (_cursorMode == MouseMode::Use) {
			out1 = _interactedObjectID;
		} else if (_cursorMode == MouseMode::UseInventory) {
			out1 = _interactedObjectID | (_interactedInventoryItemId << 16);
			out2 = _interactedInventoryItemId;
		}
		break;
	case 0x02:
		out1 = (_cursorMode == MouseMode::Look) ? _interactedObjectID : 0;
		break;
	case 0x03:
		out1 = (_cursorMode == MouseMode::Talk) ? _interactedObjectID : 0;
		break;
	case 0x04: {
		const Common::Point &charPos = getCharPosition();
		out1 = getAreaAtPoint(charPos.x, charPos.y);
		break;
	}
	case 0x05:
		break;
	case 0x06:
	case 0x0A:
	case 0x0C:
		out1 = 1;
		break;
	case 0x07:
	case 0x08:
	case 0x09:
		out1 = 0;
		break;
	case 0x0B:
		out1 = _repeatRunFlag ? 1 : 0;
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
		out1 = (_cursorMode == MouseMode::PanelUse) ? _interactedObjectID : 0;
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
