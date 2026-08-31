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

#include "common/util.h"
#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/view1.h"

namespace Macs2 {

Common::Error Macs2Engine::syncGame(Common::Serializer &s) {
	const byte SAVE_MAGIC[12] = {'A', 'H', 'F', 'F', 'M', 'S', 'G', 'M', '0', '1', '0', '0'};
	View1 *view1 = (View1 *)findView("View1");
	if (view1 == nullptr)
		return Common::kUnknownError;

	bool pendingScriptRestore = false;
	bool pendingScriptIsExecuting = false;
	uint16 pendingExecutingObjectId = 0;
	uint16 pendingScriptPosition = 0;
	uint16 pendingScriptEndPosition = 0;
	// --- Header: 12-byte magic ---
	if (s.isSaving()) {
		byte magic[12];
		memcpy(magic, SAVE_MAGIC, sizeof(SAVE_MAGIC));
		s.syncBytes(magic, 12);
	} else {
		byte magic[12];
		s.syncBytes(magic, 12);
		if (memcmp(magic, SAVE_MAGIC, sizeof(SAVE_MAGIC)) != 0)
			return Common::kReadingFailed;
	}

	byte slotName[21] = {0};
	if (s.isSaving()) {
		const char defName[] = {'S', 'C', 'U', 'M', 'M', 'V', 'M'};
		slotName[0] = 20;
		memcpy(slotName + 1, defName, sizeof(defName));
	}
	s.syncBytes(slotName, 21);

	uint16 actorIndex = (uint16)Scenes::instance()._currentActorIndex;
	uint16 sceneIndex = (uint16)Scenes::instance()._currentSceneIndex;
	s.syncAsUint16LE(actorIndex);
	s.syncAsUint16LE(sceneIndex);

	if (s.isLoading()) {
		Scenes::instance()._currentActorIndex = actorIndex;
		Scenes::instance()._currentSceneIndex = sceneIndex;
		view1->_started = true;
		changeScene(sceneIndex, false);

		// Clear any Characters created by changeScene. The per-object loop below
		// recreates them on demand, AFTER reading each object's saved scene index,
		// and populates their runtime walk/draw/dirty state. (changeScene uses the
		// RESOURCE.MCS scene indices, which can differ from the saved ones, so we
		// must not rely on its Character set here.)
		for (auto c : view1->_characters)
			delete c;
		view1->_characters.clear();
		view1->flushPendingCharacterDeletes();
	}

	// --- Script variables: exactly 0x2000 bytes (0x800 vars x {uint16 a, b}) ---
	// Must always be 0x800 entries to match the binary's fixed-size block
	for (uint i = 0; i < 0x800; i++) {
		uint16 a = (i < _scriptExecutor->_variables.size()) ? _scriptExecutor->_variables[i].a : 0;
		uint16 b = (i < _scriptExecutor->_variables.size()) ? _scriptExecutor->_variables[i].b : 0;
		s.syncAsUint16LE(a);
		s.syncAsUint16LE(b);
		if (s.isLoading() && i < _scriptExecutor->_variables.size()) {
			_scriptExecutor->_variables[i].a = a;
			_scriptExecutor->_variables[i].b = b;
		}
	}

	uint16 soundSystemActive = _scriptExecutor->_soundSystemActive ? 1 : 0;
	s.syncAsUint16LE(soundSystemActive);
	if (s.isLoading())
		_scriptExecutor->_soundSystemActive = soundSystemActive != 0;

	// --- Script execution state ---
	uint8 scriptIsExecuting = 0;
	if (s.isSaving()) {
		_scriptExecutor->prepareScriptStateForSave();
		scriptIsExecuting = _scriptExecutor->isScriptMidExecution() ? 1 : 0;
	}
	s.syncAsByte(scriptIsExecuting);

	uint16 scriptPosition = (uint16)_scriptExecutor->getScriptPosition();
	s.syncAsUint16LE(scriptPosition);

	uint16 scriptEndPosition = (uint16)_scriptExecutor->getScriptEndPosition();
	s.syncAsUint16LE(scriptEndPosition);

	uint16 executingObjectId = _scriptExecutor->getExecutingObjectId();
	s.syncAsUint16LE(executingObjectId);

	if (s.isLoading()) {
		// Object script bytes are restored later in the per-object loop; defer
		// reattaching the script stream until after runtime+0x187 is loaded.
		pendingScriptRestore = true;
		pendingScriptIsExecuting = scriptIsExecuting != 0;
		pendingExecutingObjectId = executingObjectId;
		pendingScriptPosition = scriptPosition;
		pendingScriptEndPosition = scriptEndPosition;
	}

	s.syncAsUint16LE(_scriptExecutor->_scriptClickFlag);
	s.syncAsUint16LE(_scriptExecutor->_scriptClickX);
	s.syncAsUint16LE(_scriptExecutor->_scriptClickY);
	s.syncAsUint16LE(_scriptExecutor->_scriptClickResult);

	uint8 repeatRunFlag = _scriptExecutor->getRepeatRunFlag() ? 1 : 0;
	s.syncAsByte(repeatRunFlag);
	if (s.isLoading())
		_scriptExecutor->setRepeatRunFlag(repeatRunFlag != 0);

	uint16 frameWaitCounter = _scriptExecutor->getFrameWaitCounter();
	s.syncAsUint16LE(frameWaitCounter);
	if (s.isLoading())
		_scriptExecutor->setFrameWaitCounter(frameWaitCounter);

	s.syncAsUint16LE(_scriptExecutor->_walkTargetObjectIndex);

	uint16 pickupInProgress = _scriptExecutor->_pickupInProgress ? 1 : 0;
	s.syncAsUint16LE(pickupInProgress);
	if (s.isLoading())
		_scriptExecutor->_pickupInProgress = pickupInProgress != 0;

	uint16 activeInventoryItemId = 0;
	if (s.isSaving() && view1->_activeInventoryItem)
		activeInventoryItemId = view1->_activeInventoryItem->_index + 0x400;
	s.syncAsUint16LE(activeInventoryItemId);
	if (s.isLoading()) {
		if (activeInventoryItemId >= 0x401 && activeInventoryItemId <= 0x600) {
			uint16 idx = activeInventoryItemId - 0x400;
			if (idx <= GameObjects::instance()._objects.size()) {
				GameObject *obj = GameObjects::instance()._objects[idx - 1];
				view1->_activeInventoryItem = obj;
			}
		} else {
			view1->_activeInventoryItem = nullptr;
		}
	}

	uint16 savedCursorMode = (uint16)view1->_savedCursorMode;
	s.syncAsUint16LE(savedCursorMode);
	if (s.isLoading())
		view1->_savedCursorMode = (Script::MouseMode)savedCursorMode;

	uint8 clipRectDirty = _clipRectDirty ? 1 : 0;
	s.syncAsByte(clipRectDirty);
	if (s.isLoading())
		_clipRectDirty = clipRectDirty != 0;

	s.syncAsUint16LE(_scriptExecutor->_walkTargetObjectIndex);

	uint16 mouseMode = (uint16)_scriptExecutor->_cursorMode;
	s.syncAsUint16LE(mouseMode);
	if (s.isLoading()) {
		// Mode 0 means "no cursor" / disabled in the original - the binary never
		// calls setCursorMode(0) explicitly; it's a pre-init default. After load
		// the original redraws and accepts input in Walk mode, so map 0 to Walk.
		if (mouseMode == 0)
			mouseMode = (uint16)Script::MouseMode::Walk;
		setCursorMode((Script::MouseMode)mouseMode);
	}

	// 0x101a: 2 bytes - reserved/unused
	uint16 reserved101a = 0;
	s.syncAsUint16LE(reserved101a);

	// 0x101c: 2 bytes - reserved/unused
	uint16 reserved101c = 0;
	s.syncAsUint16LE(reserved101c);

	// 0x101e: 2 bytes - reserved/unused
	uint16 reserved101e = 0;
	s.syncAsUint16LE(reserved101e);

	// Per-frame latch: cleared by drawAllCharacters, set by walkAlongPath when a
	// character reaches destination, checked at end of frame.
	uint8 movementFinishedFlag = _movementFinishedFlag ? 1 : 0;
	s.syncAsByte(movementFinishedFlag);
	if (s.isLoading())
		_movementFinishedFlag = (movementFinishedFlag != 0);

	s.syncAsUint16LE(_scriptExecutor->_interactedObjectID);
	s.syncAsUint16LE(_scriptExecutor->_interactedInventoryItemId);

	uint8 scriptSkippable = _scriptExecutor->_scriptSkippable ? 1 : 0;
	s.syncAsByte(scriptSkippable);
	if (s.isLoading())
		_scriptExecutor->_scriptSkippable = scriptSkippable != 0;

	s.syncAsUint16LE(_scriptExecutor->_pickupActorObjectID);
	s.syncAsUint16LE(_scriptExecutor->_pickupTargetObjectID);

	uint16 isRepeatRun16 = _scriptExecutor->_isRepeatRun ? 1 : 0;
	s.syncAsUint16LE(isRepeatRun16);
	if (s.isLoading())
		_scriptExecutor->_isRepeatRun = isRepeatRun16 != 0;

	uint8 inventoryCheckResult = _scriptExecutor->_inventoryCheckResult ? 1 : 0;
	s.syncAsByte(inventoryCheckResult);
	if (s.isLoading())
		_scriptExecutor->_inventoryCheckResult = inventoryCheckResult != 0;

	uint8 animBlobRangeTestResult = _scriptExecutor->_animBlobRangeTestResult ? 1 : 0;
	s.syncAsByte(animBlobRangeTestResult);
	if (s.isLoading())
		_scriptExecutor->_animBlobRangeTestResult = animBlobRangeTestResult != 0;

	uint8 inventoryActionFlag = _scriptExecutor->_inventoryActionFlag ? 1 : 0;
	s.syncAsByte(inventoryActionFlag);
	if (s.isLoading())
		_scriptExecutor->_inventoryActionFlag = inventoryActionFlag != 0;

	uint8 inventoryCombineFlag = _scriptExecutor->_inventoryCombineFlag ? 1 : 0;
	s.syncAsByte(inventoryCombineFlag);
	if (s.isLoading())
		_scriptExecutor->_inventoryCombineFlag = inventoryCombineFlag != 0;

	uint16 inventoryObjectCount = (uint16)view1->_inventoryItems.size();
	s.syncAsUint16LE(inventoryObjectCount);

	// Inventory object list indices in the actor's inventory
	byte inventoryObjectList[512] = {0};
	if (s.isSaving()) {
		for (uint16 i = 0; i < inventoryObjectCount && i < ARRAYSIZE(inventoryObjectList); i++)
			inventoryObjectList[i] = (byte)view1->_inventoryItems[i]->_index;
	}
	s.syncBytes(inventoryObjectList, ARRAYSIZE(inventoryObjectList));
	if (s.isLoading()) {
		view1->_inventoryItems.clear();
		for (uint16 i = 0; i < inventoryObjectCount && i < ARRAYSIZE(inventoryObjectList); i++) {
			uint16 idx = inventoryObjectList[i];
			if (idx > 0 && idx <= GameObjects::instance()._objects.size() && GameObjects::instance()._objects[idx - 1] != nullptr)
				view1->_inventoryItems.push_back(GameObjects::instance()._objects[idx - 1]);
		}
	}

	// --- Scene data: pathfinding overrides [+0x528D]: 200 bytes ---
	// 40 entries x 5 bytes each (1 byte active + 2 bytes value + 2 bytes remap)
	// indexed by pathfinding value 0xC8..0xEF
	if (s.isLoading())
		_pathfindingOverrides.clear();
	for (int i = 0; i < ARRAYSIZE(_areaOverrides); i++) {
		uint8 active = 0;
		uint16 overrideValue = 0;
		uint16 remap = 0;
		if (s.isSaving()) {
			uint16 idx = AREA_OVERRIDE_MIN + i;
			for (const auto &ov : _pathfindingOverrides) {
				if (ov._index == idx && ov._active) {
					active = 1;
					overrideValue = ov._overrideValue;
					break;
				}
			}
			remap = _areaOverrides[i];
		}
		s.syncAsByte(active);
		s.syncAsUint16LE(overrideValue);
		s.syncAsUint16LE(remap);
		if (s.isLoading()) {
			if (active) {
				PathfindingAreaOverride ov;
				ov._active = true;
				ov._index = AREA_OVERRIDE_MIN + i;
				ov._overrideValue = overrideValue;
				_pathfindingOverrides.push_back(ov);
			}
			_areaOverrides[i] = remap;
		}
	}

	// --- Scene data: hotspot overrides
	if (s.isLoading()) {
		_hotspotOverrides.clear();
		_hotspotOverrides.resize(17, 0xFFFF);
	}
	for (int i = 0; i < 16; i++) {
		uint16 val = 0xFFFF;
		if (s.isSaving() && (i + 1) < (int)_hotspotOverrides.size())
			val = _hotspotOverrides[i + 1];
		s.syncAsUint16LE(val);
		if (s.isLoading()) {
			_hotspotOverrides[i + 1] = val;
		}
	}
	if (s.isLoading() && _hotspotOverrides.size() < 33) {
		_hotspotOverrides.resize(33, 0xFFFF);
	}

	for (int i = 0; i < ARRAYSIZE(_sceneTimerParams); i++) {
		s.syncAsUint32LE(_sceneTimerParams[i]);
	}

	// --- Animation blob sequence positions (one uint16 per background anim) ---
	//
	// Binary save (1008:6859): for each bg anim (1..count) writes
	//   getAnimBlobSequencePos(blob) = blob[+2] = the blob header's current sequence
	//   position word.
	// Binary load (1008:747e): reads the value V, then calls
	//   advanceAnimFrame(save=1, mode=V+100, blob), i.e. jumps the blob to
	//   sequence position V (mode 100+N). This both restores the saved position
	//   AND re-parses the sequence so the blob header is fully consistent -
	//   exactly what scriptChangeAnimation does.
	//
	// The count is iStack_199 = sceneData+0x50F5, which equals
	// _backgroundAnimationsBlobs.size() after changeScene() above.
	uint16 numSpecialAnims = (uint16)_backgroundAnimationsBlobs.size();
	for (uint16 i = 0; i < numSpecialAnims; i++) {
		BackgroundAnimationBlob &blob = _backgroundAnimationsBlobs[i];
		uint16 seqPos = 0;
		if (s.isSaving() && blob._blob.size() >= 4)
			seqPos = READ_LE_UINT16(&blob._blob[2]);
		s.syncAsUint16LE(seqPos);
		if (s.isLoading()) {
			// advanceAnimFrame(1, V + 100, blob): jump to sequence position V.
			// This re-parses the sequence so repeat/delay/loop header fields are
			// consistent (matches the binary loadGameFromFile behavior).
			if (!blob._blob.empty()) {
				BackgroundAnimationBlob::advanceAnimFrame(blob._blob, true, seqPos + 0x64);
				// The jump's parse loop may settle the stored sequence position
				// (blob[+2]) on a different value than V (e.g. when V lands on a
				// command byte). Force it back to exactly V so the field round-trips
				// losslessly and byte-matches what the original wrote (the original
				// stores its live running position, not a re-derived one).
				if (blob._blob.size() >= 4)
					WRITE_LE_UINT16(&blob._blob[2], seqPos);
			}
		}
	}
	updateAllBackgroundAnimationDepthMaps();

	// --- PCM sound: size (2 bytes) + data (variable) ---
	uint16 pcmSoundSize = (uint16)_currentSoundData.size();
	s.syncAsUint16LE(pcmSoundSize);
	if (pcmSoundSize > 0) {
		if (s.isLoading())
			_currentSoundData.resize(pcmSoundSize);
		s.syncBytes(_currentSoundData.data(), pcmSoundSize);
	} else if (s.isLoading()) {
		_currentSoundData.clear();
	}

	// --- Active music slot (2 bytes) ---
	s.syncAsUint16LE(_scriptExecutor->_activeMusicSlot);

	// --- Music slot buffers (slots 1-2): size (2 bytes) + data each ---
	for (int slot = 0; slot < 2; slot++) {
		uint16 musicSize = 0;
		if (s.isSaving())
			musicSize = (uint16)_scriptExecutor->_musicSlots[slot].size();
		s.syncAsUint16LE(musicSize);
		if (musicSize > 0) {
			if (s.isLoading())
				_scriptExecutor->_musicSlots[slot].resize(musicSize);
			s.syncBytes(_scriptExecutor->_musicSlots[slot].data(), musicSize);
		} else if (s.isLoading()) {
			_scriptExecutor->_musicSlots[slot].clear();
		}
	}

	// --- All 512 objects (1..0x200) ---
	// Binary saveGameToFile/loadGameFromFile iterate the 1..0x200 object pointer
	// table at DS:0x77C and process an entry ONLY when its far pointer is
	// non-null (the object exists / was loaded from RESOURCE.MCS). Null slots are
	// SKIPPED entirely - no bytes are written or read for them. The non-null set
	// is identical at save and load time because changeScene() (called above on
	// load) rebuilds the same object table, so this remains symmetric.
	//
	// Earlier this padded 10 zero bytes per null slot, which corrupted the
	// stream relative to the original game (the original never emits those
	// bytes), making the file unreadable by the DOS executable.
	uint16 numObjects = (uint16)GameObjects::instance()._objects.size();
	for (uint16 objIdx = 0; objIdx < 512; objIdx++) {
		if (objIdx >= numObjects)
			continue;
		GameObject *obj = GameObjects::instance()._objects[objIdx];
		if (obj == nullptr)
			continue;
		// Base fields: pos.x(2), pos.y(2), scene(2), orientation(2), verticalOffsetScale(2)
		uint16 posX = (uint16)obj->_position.x;
		uint16 posY = (uint16)obj->_position.y;
		s.syncAsUint16LE(posX);
		s.syncAsUint16LE(posY);
		if (s.isLoading()) {
			obj->_position.x = (int16)posX;
			obj->_position.y = (int16)posY;
		}
		s.syncAsUint16LE(obj->_sceneIndex);
		s.syncAsUint16LE(obj->_orientation);
		s.syncAsUint16LE(obj->_verticalOffsetScale);

		// Extended data: only for objects in current scene or current actor
		bool inCurrentScene = obj->_sceneIndex == sceneIndex;
		bool isActorScene = (obj->_sceneIndex == actorIndex + 0x400);
		bool isActor = (objIdx + 1) == actorIndex;
		bool hasExtendedData = inCurrentScene || isActorScene || isActor;

		if (!hasExtendedData)
			continue;

		// --- Runtime state (0x23A bytes structure in original) ---
		// HasBoundsAttachment [+0x231]: 2 bytes written from memory.
		// OVERLAP: binary writes 2 bytes from +0x231 then 2 bytes from +0x232.
		// The high byte of the first write (mem[0x232]) gets overwritten on load by
		// the low byte of the second write. Only the LOW BYTE is hasBoundsAttachment;
		// the high byte is garbage (first byte of boundsAttachmentObjectID, discarded).
		// On save: pack low = hasBoundsAttachment, high = objectID low byte (mimic overlap).
		// On load: only the low byte matters for the boolean.
		uint16 hasBoundsWord = 0;
		if (s.isSaving())
			hasBoundsWord = (obj->_hasBoundsAttachment ? 1 : 0) | ((obj->_boundsAttachmentObjectID & 0xFF) << 8);
		s.syncAsUint16LE(hasBoundsWord);
		if (s.isLoading())
			obj->_hasBoundsAttachment = (hasBoundsWord & 0xFF) != 0;

		s.syncAsUint16LE(obj->_boundsAttachmentObjectID);
		s.syncAsUint16LE(obj->_boundsAttachmentValue1);
		s.syncAsUint16LE(obj->_boundsAttachmentValue2);
		s.syncAsUint16LE(obj->_boundsAttachmentValue3);

		// Runtime walk state [+0x00..+0x0A]: targetX, targetY, deltaX, deltaY, finalX, finalY
		// Find the Character for this object (exists after changeScene on load too)
		Character *chr = nullptr;
		for (uint ci = 0; ci < view1->_characters.size(); ci++) {
			if (view1->_characters[ci] && view1->_characters[ci]->_gameObject == obj) {
				chr = view1->_characters[ci];
				break;
			}
		}
		// On load, create the Character on demand if none exists yet. We are
		// inside the `hasExtendedData` branch (object is in-scene / actor /
		// inventory per its just-read saved scene), which is exactly when the
		// binary allocates the object's runtime struct. Creating it here lets
		// the runtime walk/draw/dirty fields below round-trip losslessly.
		if (chr == nullptr && s.isLoading()) {
			chr = new Character();
			chr->_gameObject = obj;
			view1->_characters.push_back(chr);
		}
		uint16 targetX = chr ? (uint16)chr->_targetPosition.x : 0;
		uint16 targetY = chr ? (uint16)chr->_targetPosition.y : 0;
		uint16 deltaX = chr ? (uint16)chr->_stepDeltaX : 0;
		uint16 deltaY = chr ? (uint16)chr->_stepDeltaY : 0;
		uint16 finalX = chr ? (uint16)chr->_pathFinalDestination.x : 0;
		uint16 finalY = chr ? (uint16)chr->_pathFinalDestination.y : 0;
		s.syncAsUint16LE(targetX);
		s.syncAsUint16LE(targetY);
		s.syncAsUint16LE(deltaX);
		s.syncAsUint16LE(deltaY);
		s.syncAsUint16LE(finalX);
		s.syncAsUint16LE(finalY);
		if (s.isLoading()) {
			chr->_targetPosition.x = (int16)targetX;
			chr->_targetPosition.y = (int16)targetY;
			chr->_stepDeltaX = (int16)deltaX;
			chr->_stepDeltaY = (int16)deltaY;
			chr->_pathFinalDestination.x = (int16)finalX;
			chr->_pathFinalDestination.y = (int16)finalY;
		}

		byte pathBlock[32] = {0};
		if (s.isSaving() && chr)
			memcpy(pathBlock, chr->_pathBlockRaw, 32);
		s.syncBytes(pathBlock, 32);

		uint16 pathIndex = chr ? (uint16)chr->_currentPathIndex : 0;
		s.syncAsUint16LE(pathIndex);

		uint16 pathLength = chr ? (uint16)chr->_path.size() : 0;
		s.syncAsUint16LE(pathLength);

		if (s.isLoading()) {
			memcpy(chr->_pathBlockRaw, pathBlock, 32);
			chr->_path.clear();
			for (uint16 pi = 0; pi < pathLength && pi < 32; pi++)
				chr->_path.push_back(pathBlock[pi]);
			chr->_currentPathIndex = (int)pathIndex;
		}

		uint16 stepAccum = chr ? (uint16)chr->_stepError : 0;
		s.syncAsUint16LE(stepAccum);
		if (s.isLoading())
			chr->_stepError = (int16)stepAccum;

		uint8 walkStepFlag = 0;
		s.syncAsByte(walkStepFlag);

		uint8 directionSet = chr ? (chr->_stepDirectionSet ? 1 : 0) : 0;
		s.syncAsByte(directionSet);
		if (s.isLoading())
			chr->_stepDirectionSet = directionSet != 0;

		uint16 clipLeft = (uint16)obj->_dirtyLeft;
		uint16 clipTop = (uint16)obj->_dirtyTop;
		uint16 clipRight = (uint16)obj->_dirtyRight;
		uint16 clipBottom = (uint16)obj->_dirtyBottom;
		s.syncAsUint16LE(clipLeft);
		s.syncAsUint16LE(clipTop);
		s.syncAsUint16LE(clipRight);
		s.syncAsUint16LE(clipBottom);
		if (s.isLoading()) {
			obj->_dirtyLeft = (int16)clipLeft;
			obj->_dirtyTop = (int16)clipTop;
			obj->_dirtyRight = (int16)clipRight;
			obj->_dirtyBottom = (int16)clipBottom;
		}

		uint16 motionTarget = chr ? chr->_motionTargetVerticalOffset : 0;
		s.syncAsUint16LE(motionTarget);
		if (s.isLoading())
			chr->_motionTargetVerticalOffset = motionTarget;
		uint16 motionAccum = chr ? chr->_motionProgress : 0;
		s.syncAsUint16LE(motionAccum);
		if (s.isLoading())
			chr->_motionProgress = motionAccum;
		uint16 motionThreshold = chr ? chr->_motionDistanceUnits : 0;
		s.syncAsUint16LE(motionThreshold);
		if (s.isLoading())
			chr->_motionDistanceUnits = motionThreshold;
		uint16 motionStep = chr ? chr->_motionVerticalOffsetDelta : 0;
		s.syncAsUint16LE(motionStep);
		if (s.isLoading())
			chr->_motionVerticalOffsetDelta = motionStep;
		uint16 lastDrawX = (uint16)obj->_lastDrawX;
		uint16 lastDrawY = (uint16)obj->_lastDrawY;
		uint16 lastDrawW = obj->_lastDrawWidth;
		uint16 lastDrawH = obj->_lastDrawHeight;
		s.syncAsUint16LE(lastDrawX);
		s.syncAsUint16LE(lastDrawY);
		s.syncAsUint16LE(lastDrawW);
		s.syncAsUint16LE(lastDrawH);
		if (s.isLoading()) {
			obj->_lastDrawX = (int16)lastDrawX;
			obj->_lastDrawY = (int16)lastDrawY;
			obj->_lastDrawWidth = lastDrawW;
			obj->_lastDrawHeight = lastDrawH;
		}

		uint16 pickupFrameCounter = chr ? chr->_pickupFrameCounter : 0;
		s.syncAsUint16LE(pickupFrameCounter);
		if (s.isLoading())
			chr->_pickupFrameCounter = pickupFrameCounter;
		s.syncAsUint16LE(obj->_pickupFrameStart);
		s.syncAsUint16LE(obj->_pickupFrameEnd);
		uint16 prevOrientation = chr ? chr->_previousOrientation : 0;
		s.syncAsUint16LE(prevOrientation);
		if (s.isLoading())
			chr->_previousOrientation = (uint8)prevOrientation;

		s.syncAsUint16LE(obj->_overloadAnimTriggerDirection);

		uint8 snapToTarget = obj->_snapToTarget ? 1 : 0;
		s.syncAsByte(snapToTarget);
		if (s.isLoading())
			obj->_snapToTarget = snapToTarget != 0;

		uint8 useOverload = obj->_useOverloadAnimation ? 1 : 0;
		s.syncAsByte(useOverload);
		if (s.isLoading())
			obj->_useOverloadAnimation = useOverload != 0;

		uint8 hasInventoryIcon = (obj->_blobs.size() > 0x13 && !obj->_blobs[0x13].empty()) ? 1 : 0;
		s.syncAsByte(hasInventoryIcon);

		uint8 hasShading = obj->_hasShading ? 1 : 0;
		s.syncAsByte(hasShading);
		if (s.isLoading())
			obj->_hasShading = hasShading != 0;

		uint8 hasScaling = obj->_hasScaling ? 1 : 0;
		s.syncAsByte(hasScaling);
		if (s.isLoading())
			obj->_hasScaling = hasScaling != 0;

		uint16 scriptSize = (uint16)obj->_script.size();
		s.syncAsUint16LE(scriptSize);

		// Script resource table [+0x18D]: 0x80 bytes (128 bytes = 32 dword offsets)
		// Stored in GameObject::_resourceOffsets, loaded from file during readResourceFile.
		const uint maxObjRes = maxObjectResources();
		byte scriptResourceTable[128] = {0};
		if (s.isSaving()) {
			for (uint r = 0; r < maxObjRes; r++) {
				WRITE_LE_UINT32(&scriptResourceTable[r * 4], obj->_resourceOffsets[r]);
			}
		}
		s.syncBytes(scriptResourceTable, 128);
		if (s.isLoading()) {
			for (uint r = 0; r < maxObjRes; r++) {
				obj->_resourceOffsets[r] = READ_LE_UINT32(&scriptResourceTable[r * 4]);
			}
		}

		if (s.isLoading())
			obj->_script.resize(scriptSize);
		if (scriptSize > 0)
			s.syncBytes(obj->_script.data(), scriptSize);

		const int animSlotCount = (int)maxAnimSlots();
		const int overloadBlobIdx = (int)overloadAnimSlot() - 1;
		for (int blobIdx = 0; blobIdx < animSlotCount; blobIdx++) {
			// Overload slot is stored separately from normal orientation blobs.
			bool isOverloadSlot = (blobIdx == overloadBlobIdx);

			uint16 blobActive = 0;
			if (s.isSaving()) {
				if (isOverloadSlot) {
					if (obj->_blobs.size() > 20 && !obj->_blobs[20].empty())
						blobActive = 1;
					else
						blobActive = obj->_overloadAnimation.empty() ? 0 : 1;
				} else if (blobIdx < (int)obj->_blobs.size())
					blobActive = obj->_blobs[blobIdx].empty() ? 0 : 1;
			}
			s.syncAsUint16LE(blobActive);

			if (!blobActive && s.isLoading()) {
				// Slot not active in the save file - clear any data that
				// changeScene() pre-loaded from RESOURCE.MCS so it doesn't
				// get re-saved (which would desync the file stream vs original).
				if (isOverloadSlot) {
					obj->_overloadAnimation.clear();
					if (obj->_blobs.size() > 20)
						obj->_blobs[20].clear();
				} else if (blobIdx < (int)obj->_blobs.size()) {
					obj->_blobs[blobIdx].clear();
				}
			}

			if (blobActive) {
				// frame cursor/offset X (not tracked in ScummVM, save 0)
				uint16 field00 = 0;
				s.syncAsUint16LE(field00);
				// frame cursor/offset Y (not tracked in ScummVM, save 0)
				uint16 field02 = 0;
				s.syncAsUint16LE(field02);
				uint16 blobSourceKey = 0;
				if (s.isSaving()) {
					if (isOverloadSlot) {
						if (obj->_blobs.size() > 20 && !obj->_blobs[20].empty())
							blobSourceKey = (obj->_blobSourceKeys.size() > 20) ? obj->_blobSourceKeys[20] : 0;
						else
							blobSourceKey = obj->_overloadAnimationSourceKey;
					} else if (blobIdx < (int)obj->_blobSourceKeys.size())
						blobSourceKey = obj->_blobSourceKeys[blobIdx];
				}
				s.syncAsUint16LE(blobSourceKey);
				uint16 blobSpeed = 0;
				if (s.isSaving() && !isOverloadSlot && blobIdx < (int)obj->_blobWalkSpeeds.size())
					blobSpeed = obj->_blobWalkSpeeds[blobIdx];
				s.syncAsUint16LE(blobSpeed);
				uint16 blobSize = 0;
				if (s.isSaving()) {
					if (isOverloadSlot) {
						if (obj->_blobs.size() > 20 && !obj->_blobs[20].empty())
							blobSize = (uint16)obj->_blobs[20].size();
						else
							blobSize = (uint16)obj->_overloadAnimation.size();
					} else if (blobIdx < (int)obj->_blobs.size())
						blobSize = (uint16)obj->_blobs[blobIdx].size();
				}
				s.syncAsUint16LE(blobSize);
				// Pixel data (blobSize bytes)
				if (s.isSaving()) {
					if (blobSize > 0) {
						if (isOverloadSlot) {
							if (obj->_blobs.size() > 20 && !obj->_blobs[20].empty())
								s.syncBytes(obj->_blobs[20].data(), blobSize);
							else
								s.syncBytes(obj->_overloadAnimation.data(), blobSize);
						} else
							s.syncBytes(obj->_blobs[blobIdx].data(), blobSize);
					}
				} else {
					// Loading: allocate and read pixel data
					if (blobSize > 0) {
						if (isOverloadSlot) {
							if (blobIdx >= (int)obj->_blobs.size())
								obj->_blobs.resize(blobIdx + 1);
							obj->_blobs[blobIdx].resize(blobSize);
							s.syncBytes(obj->_blobs[blobIdx].data(), blobSize);
							obj->_overloadAnimation = obj->_blobs[blobIdx];
						} else {
							if (blobIdx >= (int)obj->_blobs.size())
								obj->_blobs.resize(blobIdx + 1);
							obj->_blobs[blobIdx].resize(blobSize);
							s.syncBytes(obj->_blobs[blobIdx].data(), blobSize);
						}
					}
					if (isOverloadSlot) {
						obj->_overloadAnimationSourceKey = blobSourceKey;
					} else {
						if (blobIdx >= (int)obj->_blobSourceKeys.size())
							obj->_blobSourceKeys.resize(blobIdx + 1);
						obj->_blobSourceKeys[blobIdx] = blobSourceKey;
						if (blobIdx >= (int)obj->_blobWalkSpeeds.size())
							obj->_blobWalkSpeeds.resize(blobIdx + 1);
						obj->_blobWalkSpeeds[blobIdx] = blobSpeed;
					}
				}
			}
		}
	}

	if (s.isLoading()) {
		if (pendingScriptRestore) {
			_scriptExecutor->restoreScriptExecutionAfterLoad(pendingScriptIsExecuting,
															pendingExecutingObjectId,
															pendingScriptPosition,
															pendingScriptEndPosition);
		}

		// NOTE: Characters were already created right after changeScene (above) so
		// the per-object loop could populate their runtime walk/draw/dirty state.
		// Do NOT recreate them here - that would discard the loaded fields.
		view1->rebuildCharacterLookupTable();
		view1->refreshProtagonistInventoryAfterLoad(actorIndex);
		view1->_uiPanelState = View1::kUiPanelNone;
		view1->ensureActionBar();

		// Restore UseInventory cursor image after load.
		// The cursor slot is only populated when clicking an inventory item in the panel;
		// after loading a save with mouseMode==UseInventory, the slot is empty.
		if (_scriptExecutor->_cursorMode == Script::MouseMode::UseInventory && view1->_activeInventoryItem != nullptr) {
			if (AnimFrame *icon = view1->getInventoryIcon(view1->_activeInventoryItem)) {
				int cursorSlot = (int)Script::MouseMode::UseInventory - 1;
				_imageResources[cursorSlot] = *icon;
			}
		}

		view1->updateCursor();
		view1->_paletteDirty = true;

		if (view1->isHelpButtonDisabled()) {
			applyScenePaletteEffect();
			view1->restoreUiPaletteEntries();
		} else {
			view1->startFadingWithSpeed(8);
		}
		view1->presentFrame();
	}

	return Common::kNoError;
}

} // End of namespace Macs2
