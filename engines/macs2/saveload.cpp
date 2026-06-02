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

#include "macs2/macs2.h"
#include "macs2/gameobjects.h"
#include "macs2/view1.h"
#include "macs2/script/scriptexecutor.h"

namespace Macs2 {

// Binary-compatible save format matching the original DOS game's
// saveGameToFile (1008:6859) and loadGameFromFile (1008:747e).
// Save files are interchangeable between ScummVM and the original game.

Common::Error Macs2Engine::syncGame(Common::Serializer &s) {
	const char *SAVE_MAGIC = "AHFFMSGM0100";
	View1 *view1 = (View1 *)findView("View1");

	// --- Header: 12-byte magic ---
	if (s.isSaving()) {
		byte magic[12];
		memcpy(magic, SAVE_MAGIC, 12);
		s.syncBytes(magic, 12);
	} else {
		char magic[12];
		s.syncBytes((byte *)magic, 12);
		if (memcmp(magic, SAVE_MAGIC, 12) != 0)
			return Common::kReadingFailed;
	}

	// --- 21-byte slot name (Pascal string, we just zero-fill on save) ---
	byte slotName[21] = {0};
	s.syncBytes(slotName, 21);

	// --- Core indices: actor (2 bytes) + scene (2 bytes) ---
	uint16 actorIndex = (uint16)Scenes::instance()._currentActorIndex;
	uint16 sceneIndex = (uint16)Scenes::instance()._currentSceneIndex;
	s.syncAsUint16LE(actorIndex);
	s.syncAsUint16LE(sceneIndex);

	if (s.isLoading()) {
		Scenes::instance()._currentActorIndex = actorIndex;
		Scenes::instance()._currentSceneIndex = sceneIndex;
		view1->_started = true;
		changeScene(sceneIndex, false);
	}

	// --- Script variables: 0x2000 bytes (2048 vars x 2 uint16) ---
	for (uint i = 0; i < _scriptExecutor->_variables.size(); i++) {
		s.syncAsUint16LE(_scriptExecutor->_variables[i].a);
		s.syncAsUint16LE(_scriptExecutor->_variables[i].b);
	}

	// --- g_wSoundSystemActive: 2 bytes [0x1f4c] ---
	uint16 soundSystemActive = _scriptExecutor->_soundSystemActive ? 1 : 0;
	s.syncAsUint16LE(soundSystemActive);
	if (s.isLoading())
		_scriptExecutor->_soundSystemActive = soundSystemActive != 0;

	// --- Script execution state ---
	// g_wScriptIsExecuting [0xf88]: 1 byte
	uint8 scriptIsExecuting = _scriptExecutor->isExecuting() ? 1 : 0;
	s.syncAsByte(scriptIsExecuting);

	// g_wScriptPosition [0xf8a]: 2 bytes
	uint16 scriptPosition = (uint16)_scriptExecutor->getScriptPosition();
	s.syncAsUint16LE(scriptPosition);

	// g_wScriptEndPosition [0xf90]: 2 bytes
	uint16 scriptEndPosition = (uint16)_scriptExecutor->getScriptEndPosition();
	s.syncAsUint16LE(scriptEndPosition);

	// g_wExecutingScriptObjectId [0xf92]: 2 bytes
	uint16 executingObjectId = _scriptExecutor->getExecutingObjectId();
	s.syncAsUint16LE(executingObjectId);

	if (s.isLoading()) {
		// Restore script execution state - set the stream position
		if (scriptIsExecuting && executingObjectId == 0) {
			_scriptExecutor->setCurrentSceneScriptAt(scriptPosition);
		}
		_scriptExecutor->setExecutingObjectId(executingObjectId);
	}

	// g_wScriptClickFlag [0xf94]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_scriptClickFlag);

	// g_wScriptClickX [0xf96]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_scriptClickX);

	// g_wScriptClickY [0xf98]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_scriptClickY);

	// g_wScriptClickResult [0xf9a]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_scriptClickResult);

	// --- Game state globals ---
	// g_wRepeatRunFlag [0x1012]: 1 byte
	uint8 repeatRunFlag = _scriptExecutor->_isRepeatRun ? 1 : 0;
	s.syncAsByte(repeatRunFlag);
	if (s.isLoading())
		_scriptExecutor->_isRepeatRun = repeatRunFlag != 0;

	// g_wFrameWaitCounter [0x100a]: 2 bytes
	uint16 frameWaitCounter = _scriptExecutor->getFrameWaitCounter();
	s.syncAsUint16LE(frameWaitCounter);
	if (s.isLoading())
		_scriptExecutor->setFrameWaitCounter(frameWaitCounter);

	// g_wWalkTargetObjectIndex [0x1016]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_walkTargetObjectIndex);

	// g_wPickupInProgress [0x1030]: 2 bytes
	uint16 pickupInProgress = _scriptExecutor->_pickupInProgress ? 1 : 0;
	s.syncAsUint16LE(pickupInProgress);
	if (s.isLoading())
		_scriptExecutor->_pickupInProgress = pickupInProgress != 0;

	// g_wActiveInventoryItemId [0xfd0]: 2 bytes
	uint16 activeInventoryItemId = 0;
	if (s.isSaving() && view1->_activeInventoryItem)
		activeInventoryItemId = view1->_activeInventoryItem->_index + 0x400;
	s.syncAsUint16LE(activeInventoryItemId);
	if (s.isLoading()) {
		if (activeInventoryItemId >= 0x401 && activeInventoryItemId <= 0x600) {
			uint16 idx = activeInventoryItemId - 0x400;
			if (idx <= GameObjects::instance()._objects.size())
				view1->_activeInventoryItem = GameObjects::instance()._objects[idx - 1];
		} else {
			view1->_activeInventoryItem = nullptr;
		}
	}

	// g_wSavedCursorMode [0xfea]: 2 bytes
	uint16 savedCursorMode = (uint16)_scriptExecutor->_cursorModeBeforeWait;
	s.syncAsUint16LE(savedCursorMode);
	if (s.isLoading())
		_scriptExecutor->_cursorModeBeforeWait = (Script::MouseMode)savedCursorMode;

	// g_wClipRectDirty [0xfec]: 1 byte - flags clip region needs full-screen reset
	uint8 clipRectDirty = _clipRectDirty ? 1 : 0;
	s.syncAsByte(clipRectDirty);
	if (s.isLoading())
		_clipRectDirty = clipRectDirty != 0;

	// g_wWalkTargetObjectIndex (duplicate) [0x1016]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_walkTargetObjectIndex);

	// PTR_LOOP_1020_1018 [0x1018]: 2 bytes - mouse mode
	uint16 mouseMode = (uint16)_scriptExecutor->_mouseMode;
	s.syncAsUint16LE(mouseMode);
	if (s.isLoading())
		_scriptExecutor->_mouseMode = (Script::MouseMode)mouseMode;

	// 0x101a: 2 bytes - reserved/unused
	uint16 reserved101a = 0;
	s.syncAsUint16LE(reserved101a);

	// 0x101c: 2 bytes - reserved/unused
	uint16 reserved101c = 0;
	s.syncAsUint16LE(reserved101c);

	// 0x101e: 2 bytes - reserved/unused
	uint16 reserved101e = 0;
	s.syncAsUint16LE(reserved101e);

	// g_bMovementFinishedFlag [0x1020]: 1 byte
	// Per-frame latch: cleared by drawAllCharacters, set by walkAlongPath when a
	// character reaches destination, checked at end of frame. Always 0 is safe on load.
	uint8 movementFinishedFlag = 0;
	s.syncAsByte(movementFinishedFlag);

	// g_wInteractedObjectId [0x1024]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_interactedObjectID);

	// g_wInteractedInventoryItemId [0x1026]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_interactedOtherObjectID);

	// g_wScriptSkippable [0x102a]: 1 byte
	uint8 scriptSkippable = _scriptExecutor->_scriptSkippable ? 1 : 0;
	s.syncAsByte(scriptSkippable);
	if (s.isLoading())
		_scriptExecutor->_scriptSkippable = scriptSkippable != 0;

	// g_wPickupActorObjectId [0x102c]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_pickupActorObjectID);

	// g_wPickupTargetObjectId [0x102e]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_pickupTargetObjectID);

	// g_wIsRepeatRun [0x1032]: 2 bytes
	uint16 isRepeatRun16 = _scriptExecutor->_isRepeatRun ? 1 : 0;
	s.syncAsUint16LE(isRepeatRun16);
	if (s.isLoading())
		_scriptExecutor->_isRepeatRun = isRepeatRun16 != 0;

	// g_wInventoryCheckResult [0x103c]: 1 byte
	uint8 inventoryCheckResult = _scriptExecutor->_inventoryCheckResult ? 1 : 0;
	s.syncAsByte(inventoryCheckResult);
	if (s.isLoading())
		_scriptExecutor->_inventoryCheckResult = inventoryCheckResult != 0;

	// g_wAnimBlobRangeTestResult [0x103e]: 1 byte
	uint8 animBlobRangeTestResult = _scriptExecutor->_animBlobRangeTestResult ? 1 : 0;
	s.syncAsByte(animBlobRangeTestResult);
	if (s.isLoading())
		_scriptExecutor->_animBlobRangeTestResult = animBlobRangeTestResult != 0;

	// g_wInventoryActionFlag [0x1040]: 1 byte
	uint8 inventoryActionFlag = _scriptExecutor->_inventoryActionFlag ? 1 : 0;
	s.syncAsByte(inventoryActionFlag);
	if (s.isLoading())
		_scriptExecutor->_inventoryActionFlag = inventoryActionFlag != 0;

	// g_wInventoryCombineFlag [0x1042]: 1 byte
	uint8 inventoryCombineFlag = _scriptExecutor->_inventoryCombineFlag ? 1 : 0;
	s.syncAsByte(inventoryCombineFlag);
	if (s.isLoading())
		_scriptExecutor->_inventoryCombineFlag = inventoryCombineFlag != 0;

	// g_wInventoryObjectCount [0x222a]: 2 bytes - number of items in inventory list
	uint16 inventoryObjectCount = (uint16)view1->_inventoryItems.size();
	s.syncAsUint16LE(inventoryObjectCount);

	// --- Inventory object list [0x202a]: 0x200 bytes ---
	// Byte array of object indices in the actor's inventory.
	// Original stores as bytes (object index, 1-based).
	byte inventoryObjectList[512] = {0};
	if (s.isSaving()) {
		for (uint16 i = 0; i < inventoryObjectCount && i < 512; i++)
			inventoryObjectList[i] = (byte)view1->_inventoryItems[i]->_index;
	}
	s.syncBytes(inventoryObjectList, 512);
	if (s.isLoading()) {
		view1->_inventoryItems.clear();
		for (uint16 i = 0; i < inventoryObjectCount && i < 512; i++) {
			uint16 idx = inventoryObjectList[i];
			if (idx > 0 && idx <= GameObjects::instance()._objects.size())
				view1->_inventoryItems.push_back(GameObjects::instance()._objects[idx - 1]);
		}
	}

	// --- Scene data: pathfinding overrides [+0x528D]: 200 bytes ---
	// 40 entries x 5 bytes each (1 byte active + 2 bytes value + 2 bytes remap)
	// indexed by pathfinding value 0xC8..0xEF
	if (s.isLoading())
		_pathfindingOverrides.clear();
	for (int i = 0; i < 40; i++) {
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
			// Remap field at +0x4EA8 = _areaOverrides[i]
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

	// --- Scene data: hotspot overrides [+0x5BD3]: 32 bytes (16 x uint16) ---
	if (s.isLoading())
		_hotspotOverrides.clear();
	for (int i = 0; i < 16; i++) {
		uint16 val = 0xFFFF;
		if (s.isSaving() && i < (int)_hotspotOverrides.size())
			val = _hotspotOverrides[i];
		s.syncAsUint16LE(val);
		if (s.isLoading()) {
			if (i < (int)_hotspotOverrides.size())
				_hotspotOverrides[i] = val;
			else
				_hotspotOverrides.push_back(val);
		}
	}

	for (int i = 0; i < 4; i++) {
		s.syncAsUint32LE(_sceneTimerParams[i]);
	}

	// --- Animation blob offsets (variable length) ---
	// Original: for each special anim (1..count), save 2-byte offset from getAnimBlobOffset
	uint16 numSpecialAnims = (uint16)_backgroundAnimationsBlobs.size();
	for (uint16 i = 0; i < numSpecialAnims; i++) {
		uint16 animOffset = (i < _backgroundAnimationsBlobs.size()) ?
		                    (uint16)_backgroundAnimationsBlobs[i]._frameIndex : 0;
		s.syncAsUint16LE(animOffset);
		if (s.isLoading() && i < _backgroundAnimationsBlobs.size())
			_backgroundAnimationsBlobs[i]._frameIndex = animOffset;
	}

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
	// Original iterates 1..512 inclusive. ScummVM may have fewer objects.
	uint16 numObjects = (uint16)GameObjects::instance()._objects.size();
	for (uint16 objIdx = 0; objIdx < 512; objIdx++) {
		if (objIdx >= numObjects) {
			// Pad with empty object data for binary compatibility
			uint16 zero16 = 0;
			for (int i = 0; i < 5; i++)
				s.syncAsUint16LE(zero16);
			continue;
		}
		GameObject *obj = GameObjects::instance()._objects[objIdx];
		// Original checks if object pointer is non-null (always true for us)
		// Base fields: pos.x(2), pos.y(2), scene(2), orientation(2), unknown(2)
		uint16 posX = (uint16)obj->Position.x;
		uint16 posY = (uint16)obj->Position.y;
		s.syncAsUint16LE(posX);
		s.syncAsUint16LE(posY);
		s.syncAsUint16LE(obj->SceneIndex);
		s.syncAsUint16LE(obj->Orientation);
		s.syncAsUint16LE(obj->Unknown);
		if (s.isLoading()) {
			obj->Position.x = (int16)posX;
			obj->Position.y = (int16)posY;
		}

		// Extended data: only for objects in current scene or current actor
		bool inCurrentScene = (obj->SceneIndex == sceneIndex);
		bool isActorScene = (obj->SceneIndex == actorIndex + 0x400);
		bool isActor = ((objIdx + 1) == actorIndex);
		bool hasExtendedData = inCurrentScene || isActorScene || isActor;

		if (!hasExtendedData)
			continue;

		// --- Runtime state (0x23A bytes structure in original) ---
		// HasBoundsAttachment [+0x231]: 2 bytes (word, but bool)
		uint16 hasBounds = obj->HasBoundsAttachment ? 1 : 0;
		s.syncAsUint16LE(hasBounds);
		if (s.isLoading())
			obj->HasBoundsAttachment = hasBounds != 0;

		// BoundsAttachmentObjectID [+0x232]: 2 bytes
		s.syncAsUint16LE(obj->BoundsAttachmentObjectID);
		// BoundsAttachmentValue1 [+0x234]: 2 bytes
		s.syncAsUint16LE(obj->BoundsAttachmentValue1);
		// BoundsAttachmentValue2 [+0x236]: 2 bytes
		s.syncAsUint16LE(obj->BoundsAttachmentValue2);
		// BoundsAttachmentValue3 [+0x238]: 2 bytes
		s.syncAsUint16LE(obj->BoundsAttachmentValue3);

		// Runtime walk state [+0x00..+0x0A]: targetX, targetY, deltaX, deltaY, finalX, finalY
		// Find the Character for this object (exists after changeScene on load too)
		Character *chr = nullptr;
		if (view1) {
			for (uint ci = 0; ci < view1->_characters.size(); ci++) {
				if (view1->_characters[ci] && view1->_characters[ci]->_gameObject == obj) {
					chr = view1->_characters[ci];
					break;
				}
			}
		}
		uint16 targetX = chr ? (uint16)chr->_endPosition.x : 0;
		uint16 targetY = chr ? (uint16)chr->_endPosition.y : 0;
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
		if (s.isLoading() && chr) {
			chr->_endPosition.x = (int16)targetX;
			chr->_endPosition.y = (int16)targetY;
			chr->_stepDeltaX = (int16)deltaX;
			chr->_stepDeltaY = (int16)deltaY;
			chr->_pathFinalDestination.x = (int16)finalX;
			chr->_pathFinalDestination.y = (int16)finalY;
		}

		// [+0x0C]: 0x20 bytes (path node index list)
		byte pathBlock[32] = {0};
		s.syncBytes(pathBlock, 32);

		// [+0x2C]: 2 bytes - path current index
		uint16 pathIndex = 0;
		s.syncAsUint16LE(pathIndex);

		// [+0x2E]: 2 bytes - path length
		uint16 pathLength = 0;
		s.syncAsUint16LE(pathLength);

		// [+0x30]: 2 bytes - step accumulator
		uint16 stepAccum = 0;
		s.syncAsUint16LE(stepAccum);

		// [+0x32]: 1 byte - walk step flag (Bresenham related)
		uint8 walkStepFlag = 0;
		s.syncAsByte(walkStepFlag);

		// [+0x33]: 1 byte - direction set flag (_stepDirectionSet)
		uint8 directionSet = chr ? (chr->_stepDirectionSet ? 1 : 0) : 0;
		s.syncAsByte(directionSet);
		if (s.isLoading() && chr)
			chr->_stepDirectionSet = directionSet != 0;

		// _pickupFrameStart [+0x217]: 2 bytes
		s.syncAsUint16LE(obj->_pickupFrameStart);
		// _pickupFrameEnd [+0x219]: 2 bytes
		s.syncAsUint16LE(obj->_pickupFrameEnd);
		// [+0x211]: 2 bytes - path index/length/accumulator - walk interrupted, save 0
		uint16 runtime211 = 0;
		s.syncAsUint16LE(runtime211);
		// [+0x213]: 2 bytes - path index/length/accumulator - walk interrupted, save 0
		uint16 runtime213 = 0;
		s.syncAsUint16LE(runtime213);

		// RuntimeSlotValues [+0x21D..+0x22B]: 8 x uint16
		for (int i = 0; i < 8; i++) {
			uint16 slotVal = (i < 0x15) ? obj->RuntimeSlotValues[i] : 0;
			s.syncAsUint16LE(slotVal);
			if (s.isLoading() && i < 0x15)
				obj->RuntimeSlotValues[i] = slotVal;
		}

		// [+0x215..+0x21B]: 4 x uint16 - more slot values
		for (int i = 8; i < 12; i++) {
			uint16 slotVal = (i < 0x15) ? obj->RuntimeSlotValues[i] : 0;
			s.syncAsUint16LE(slotVal);
			if (s.isLoading() && i < 0x15)
				obj->RuntimeSlotValues[i] = slotVal;
		}

		// overloadAnimTriggerDirection [+0x22D]: 2 bytes
		s.syncAsUint16LE(obj->overloadAnimTriggerDirection);

		// _snapToTarget [+0x22F]: 1 byte
		uint8 flag22F = obj->_snapToTarget ? 1 : 0;
		s.syncAsByte(flag22F);
		if (s.isLoading())
			obj->_snapToTarget = flag22F != 0;

		// useOverloadAnimation [+0x230]: 1 byte
		uint8 useOverload = obj->useOverloadAnimation ? 1 : 0;
		s.syncAsByte(useOverload);
		if (s.isLoading())
			obj->useOverloadAnimation = useOverload != 0;

		// [+0x184]: 1 byte - hasInventoryIcon (loaded from resource, checked by inventory UI)
		// Not tracked separately in ScummVM - determined by blob presence
		uint8 hasInventoryIcon = 0;
		s.syncAsByte(hasInventoryIcon);

		// [+0x185]: 1 byte - isClickable (set by opcode 0x32)
		uint8 isClickable = obj->IsClickable ? 1 : 0;
		s.syncAsByte(isClickable);
		if (s.isLoading())
			obj->IsClickable = isClickable != 0;

		// [+0x186]: 1 byte - isVisible (set by opcode 0x33)
		uint8 isVisible = obj->IsVisible ? 1 : 0;
		s.syncAsByte(isVisible);
		if (s.isLoading())
			obj->IsVisible = isVisible != 0;

		// Script size [+0x18B]: 2 bytes
		uint16 scriptSize = (uint16)obj->Script.size();
		s.syncAsUint16LE(scriptSize);

		// Script resource table [+0x18D]: 0x80 bytes (128 bytes)
		// TODO: script resource offset table not tracked separately - save zeros
		byte scriptResourceTable[128] = {0};
		s.syncBytes(scriptResourceTable, 128);

		// Script data: scriptSize bytes from [+0x187] pointer
		if (s.isLoading())
			obj->Script.resize(scriptSize);
		if (scriptSize > 0)
			s.syncBytes(obj->Script.data(), scriptSize);

		// --- Animation blobs (21 slots, 1-based: 1..0x15) ---
		// The original binary's save format does NOT store blob pixel data.
		// Blob pixel data is reloaded from the resource file by loadSceneObjects
		// (called via changeScene during load). We only sync the metadata
		// (active flag, source key, speed) to stay binary-compatible.
		for (int blobIdx = 0; blobIdx < 0x15; blobIdx++) {
			uint16 blobActive = 0;
			if (s.isSaving() && blobIdx < (int)obj->Blobs.size())
				blobActive = obj->Blobs[blobIdx].empty() ? 0 : 1;
			s.syncAsUint16LE(blobActive);

			if (blobActive) {
				uint16 blobX = 0, blobY = 0;
				uint16 blobSize = 0;
				uint16 blobSourceKey = 0;
				uint16 blobSpeed = 0;

				if (s.isSaving() && blobIdx < (int)obj->Blobs.size()) {
					blobSize = (uint16)obj->Blobs[blobIdx].size();
					blobSourceKey = (blobIdx < (int)obj->BlobSourceKeys.size()) ?
					                obj->BlobSourceKeys[blobIdx] : 0;
					blobSpeed = (blobIdx < (int)obj->BlobSpeeds.size()) ?
					            obj->BlobSpeeds[blobIdx] : 0;
				}

				s.syncAsUint16LE(blobX);
				s.syncAsUint16LE(blobY);
				s.syncAsUint16LE(blobSourceKey);
				s.syncAsUint16LE(blobSpeed);
				s.syncAsUint16LE(blobSize);

				if (s.isLoading()) {
					// Don't overwrite blob pixel data from changeScene/readResourceFile.
					// Only restore metadata.
					if (blobIdx < (int)obj->BlobSourceKeys.size())
						obj->BlobSourceKeys[blobIdx] = blobSourceKey;
					if (blobIdx < (int)obj->BlobSpeeds.size())
						obj->BlobSpeeds[blobIdx] = blobSpeed;
				}
				// Skip over blobSize bytes in the stream (binary compat)
				if (s.isLoading() && blobSize > 0) {
					// The save format may or may not contain pixel data.
					// For binary-original saves, blobSize here reflects the runtime
					// allocation but the actual data is NOT in the save stream.
					// We skip nothing — the binary format doesn't write pixel bytes.
				}
			}
		}
	}

	// --- Post-load: rebuild view state ---
	if (s.isLoading()) {
		view1->_characters.clear();
		for (auto obj : GameObjects::instance()._objects) {
			if (obj->SceneIndex == (uint16)Scenes::instance()._currentSceneIndex) {
				Character *c = new Character();
				c->_gameObject = obj;
				view1->_characters.push_back(c);
			}
		}
		view1->setInventorySource(GameObjects::instance().getProtagonistObject());
		view1->updateCursor();
		view1->_paletteDirty = true;

		// Reconstruct walk-wait callback state from _walkTargetObjectIndex.
		// In the original, gameTick polls this each frame to detect walk arrival.
		// In ScummVM, we use WaitingForCallback + _executeScriptOnFinishLerp.
		if (_scriptExecutor->_walkTargetObjectIndex != 0) {
			Character *walkChar = view1->getCharacterByIndex(_scriptExecutor->_walkTargetObjectIndex);
			if (walkChar) {
				walkChar->registerWaitForMovementFinishedEvent();
				_scriptExecutor->_requestCallback = false;
				_scriptExecutor->_isAwaitingCallback = true;
				_scriptExecutor->setWaitingForCallback();
			}
		}
	}

	return Common::kNoError;
}

} // End of namespace Macs2
