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
	const byte SAVE_MAGIC[12] = {'A','H','F','F','M','S','G','M','0','1','0','0'};
	View1 *view1 = (View1 *)findView("View1");

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
	uint8 repeatRunFlag = _scriptExecutor->getRepeatRunFlag() ? 1 : 0;
	s.syncAsByte(repeatRunFlag);
	if (s.isLoading())
		_scriptExecutor->setRepeatRunFlag(repeatRunFlag != 0);

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
			if (idx <= GameObjects::instance()._objects.size()) {
				GameObject *obj = GameObjects::instance()._objects[idx - 1];
				view1->_activeInventoryItem = obj;
			}
		} else {
			view1->_activeInventoryItem = nullptr;
		}
	}

	// g_wSavedCursorMode [0xfea]: 2 bytes
	uint16 savedCursorMode = view1 ? (uint16)view1->_cursorModeBeforeMenu : (uint16)Script::MouseMode::Walk;
	s.syncAsUint16LE(savedCursorMode);
	if (s.isLoading() && view1)
		view1->_cursorModeBeforeMenu = (Script::MouseMode)savedCursorMode;

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
	// character reaches destination, checked at end of frame.
	uint8 movementFinishedFlag = _movementFinishedFlag ? 1 : 0;
	s.syncAsByte(movementFinishedFlag);
	if (s.isLoading())
		_movementFinishedFlag = (movementFinishedFlag != 0);

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
			if (idx > 0 && idx <= GameObjects::instance()._objects.size() && GameObjects::instance()._objects[idx - 1] != nullptr)
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
	// Binary table is at scene+i*2+0x5BD1, accessed with 1-based index (1..16).
	// C++ array needs 17 entries (index 0 unused, indices 1-16 used).
	if (s.isLoading()) {
		_hotspotOverrides.clear();
		_hotspotOverrides.resize(0x11, 0xFFFF);
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
		if (obj == nullptr) {
			uint16 zero16 = 0;
			for (int i = 0; i < 5; i++)
				s.syncAsUint16LE(zero16);
			continue;
		}
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
		// HasBoundsAttachment [+0x231]: 2 bytes (word, but bool)
		uint16 hasBounds = obj->_hasBoundsAttachment ? 1 : 0;
		s.syncAsUint16LE(hasBounds);
		if (s.isLoading())
			obj->_hasBoundsAttachment = hasBounds != 0;

		// BoundsAttachmentObjectID [+0x232]: 2 bytes
		s.syncAsUint16LE(obj->_boundsAttachmentObjectID);
		// BoundsAttachmentValue1 [+0x234]: 2 bytes
		s.syncAsUint16LE(obj->_boundsAttachmentValue1);
		// BoundsAttachmentValue2 [+0x236]: 2 bytes
		s.syncAsUint16LE(obj->_boundsAttachmentValue2);
		// BoundsAttachmentValue3 [+0x238]: 2 bytes
		s.syncAsUint16LE(obj->_boundsAttachmentValue3);

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
		if (s.isSaving() && chr) {
			for (uint pi = 0; pi < chr->_path.size() && pi < 32; pi++)
				pathBlock[pi] = (byte)chr->_path[pi];
		}
		s.syncBytes(pathBlock, 32);

		// [+0x2C]: 2 bytes - path current index
		uint16 pathIndex = chr ? (uint16)chr->_currentPathIndex : 0;
		s.syncAsUint16LE(pathIndex);

		// [+0x2E]: 2 bytes - path length
		uint16 pathLength = chr ? (uint16)chr->_path.size() : 0;
		s.syncAsUint16LE(pathLength);

		if (s.isLoading() && chr) {
			chr->_path.clear();
			for (uint16 pi = 0; pi < pathLength && pi < 32; pi++)
				chr->_path.push_back(pathBlock[pi]);
			chr->_currentPathIndex = (int)pathIndex;
			chr->_isFollowingPath = pathLength > 0;
		}

		// [+0x30]: 2 bytes - step accumulator (Bresenham error)
		uint16 stepAccum = chr ? (uint16)chr->_stepError : 0;
		s.syncAsUint16LE(stepAccum);
		if (s.isLoading() && chr)
			chr->_stepError = (int16)stepAccum;

		// [+0x32]: 1 byte - walk step flag (isLerping)
		uint8 walkStepFlag = chr ? (chr->_isLerping ? 1 : 0) : 0;
		s.syncAsByte(walkStepFlag);
		if (s.isLoading() && chr)
			chr->_isLerping = walkStepFlag != 0;

		// [+0x33]: 1 byte - direction set flag (_stepDirectionSet)
		uint8 directionSet = chr ? (chr->_stepDirectionSet ? 1 : 0) : 0;
		s.syncAsByte(directionSet);
		if (s.isLoading() && chr)
			chr->_stepDirectionSet = directionSet != 0;

		// [+0x20D]: 2 bytes - clip rect left
		uint16 runtime20D = 0;
		s.syncAsUint16LE(runtime20D);
		// [+0x20F]: 2 bytes - clip rect top
		uint16 runtime20F = 0;
		s.syncAsUint16LE(runtime20F);
		// [+0x211]: 2 bytes - clip rect right
		uint16 runtime211 = 0;
		s.syncAsUint16LE(runtime211);
		// [+0x213]: 2 bytes - clip rect bottom
		uint16 runtime213 = 0;
		s.syncAsUint16LE(runtime213);

		// [+0x21D..+0x22B]: 8 x uint16 - motion vertical offset state + sprite draw bounds
		// +0x21D: motion target vertical offset
		// +0x21F: motion accumulator (unused in ScummVM currently)
		// +0x221: motion threshold (unused in ScummVM currently)
		// +0x223: motion step delta (unused in ScummVM currently)
		// +0x225..+0x22B: transient sprite draw bounds (recalculated each frame)
		uint16 motionTarget = chr ? chr->_motionTargetVerticalOffset : 0;
		s.syncAsUint16LE(motionTarget);
		if (s.isLoading() && chr)
			chr->_motionTargetVerticalOffset = motionTarget;
		uint16 motionDelta = chr ? chr->_motionVerticalOffsetDelta : 0;
		s.syncAsUint16LE(motionDelta);
		if (s.isLoading() && chr)
			chr->_motionVerticalOffsetDelta = motionDelta;
		uint16 motionDist = chr ? chr->_motionDistanceUnits : 0;
		s.syncAsUint16LE(motionDist);
		if (s.isLoading() && chr)
			chr->_motionDistanceUnits = motionDist;
		uint16 motionProgress = chr ? chr->_motionProgress : 0;
		s.syncAsUint16LE(motionProgress);
		if (s.isLoading() && chr)
			chr->_motionProgress = motionProgress;
		// +0x225..+0x22B: transient sprite bounds (recalculated, save for format compat)
		for (int i = 0; i < 4; i++) {
			uint16 boundsVal = 0;
			s.syncAsUint16LE(boundsVal);
		}

		// [+0x215]: 2 bytes - pickup frame counter
		uint16 pickupFrameCounter = chr ? chr->_pickupFrameCounter : 0;
		s.syncAsUint16LE(pickupFrameCounter);
		if (s.isLoading() && chr)
			chr->_pickupFrameCounter = pickupFrameCounter;
		// [+0x217]: 2 bytes - _pickupFrameStart
		s.syncAsUint16LE(obj->_pickupFrameStart);
		// [+0x219]: 2 bytes - _pickupFrameEnd
		s.syncAsUint16LE(obj->_pickupFrameEnd);
		// [+0x21B]: 2 bytes - previous orientation (saved before pickup)
		uint16 prevOrientation = chr ? chr->_previousOrientation : 0;
		s.syncAsUint16LE(prevOrientation);
		if (s.isLoading() && chr)
			chr->_previousOrientation = (uint8)prevOrientation;

		// overloadAnimTriggerDirection [+0x22D]: 2 bytes
		s.syncAsUint16LE(obj->overloadAnimTriggerDirection);

		// _snapToTarget [+0x22F]: 1 byte
		uint8 snapToTarget = obj->_snapToTarget ? 1 : 0;
		s.syncAsByte(snapToTarget);
		if (s.isLoading())
			obj->_snapToTarget = snapToTarget != 0;

		// useOverloadAnimation [+0x230]: 1 byte
		uint8 useOverload = obj->useOverloadAnimation ? 1 : 0;
		s.syncAsByte(useOverload);
		if (s.isLoading())
			obj->useOverloadAnimation = useOverload != 0;

		// [+0x184]: 1 byte - hasInventoryIcon (loaded from re_runtimeSlotValuessource file)
		// Indicates presence of inventory icon blob (slot 0x14 = index 0x13)
		uint8 hasInventoryIcon = (obj->_blobs.size() > 0x13 && !obj->_blobs[0x13].empty()) ? 1 : 0;
		s.syncAsByte(hasInventoryIcon);

		// [+0x185]: 1 byte - HasShading (per-object rendering flag from file)
		uint8 hasShading = obj->_hasShading ? 1 : 0;
		s.syncAsByte(hasShading);
		if (s.isLoading())
			obj->_hasShading = hasShading != 0;

		// [+0x186]: 1 byte - HasScaling (per-object rendering flag from file)
		uint8 hasScaling = obj->_hasScaling ? 1 : 0;
		s.syncAsByte(hasScaling);
		if (s.isLoading())
			obj->_hasScaling = hasScaling != 0;

		// IsClickable and IsVisible (set by script opcodes 0x32/0x33)
		uint8 isClickable = obj->_isClickable ? 1 : 0;
		s.syncAsByte(isClickable);
		if (s.isLoading())
			obj->_isClickable = isClickable != 0;

		uint8 isVisible = obj->_isVisible ? 1 : 0;
		s.syncAsByte(isVisible);
		if (s.isLoading())
			obj->_isVisible = isVisible != 0;

		// Script size [+0x18B]: 2 bytes
		uint16 scriptSize = (uint16)obj->_script.size();
		s.syncAsUint16LE(scriptSize);

		// Script resource table [+0x18D]: 0x80 bytes (128 bytes = 32 dword offsets)
		// Stored in GameObject::_resourceOffsets, loaded from file during readResourceFile.
		byte scriptResourceTable[128] = {0};
		if (s.isSaving()) {
			for (int r = 0; r < 32; r++) {
				WRITE_LE_UINT32(&scriptResourceTable[r * 4], obj->_resourceOffsets[r]);
			}
		}
		s.syncBytes(scriptResourceTable, 128);
		if (s.isLoading()) {
			for (int r = 0; r < 32; r++) {
				obj->_resourceOffsets[r] = READ_LE_UINT32(&scriptResourceTable[r * 4]);
			}
		}

		// Script data: scriptSize bytes from [+0x187] pointer
		if (s.isLoading())
			obj->_script.resize(scriptSize);
		if (scriptSize > 0)
			s.syncBytes(obj->_script.data(), scriptSize);

		// --- Animation blobs (21 slots, 1-based: 1..0x15) ---
		// Animation blob save format (binary-compatible with original 1008:6859/747e):
		// Per slot (1..0x15): 2-byte active flag, then if active:
		//   +0x00: 2 bytes (frame cursor X), +0x02: 2 bytes (frame cursor Y),
		//   +0x0C: 2 bytes (source key), +0x0E: 2 bytes (speed),
		//   +0x04: 2 bytes (data size), then data_size bytes of pixel data.
		for (int blobIdx = 0; blobIdx < 0x15; blobIdx++) {
			// Active flag (2 bytes, but only low byte matters)
			uint16 blobActive = 0;
			if (s.isSaving() && blobIdx < (int)obj->_blobs.size())
				blobActive = obj->_blobs[blobIdx].empty() ? 0 : 1;
			s.syncAsUint16LE(blobActive);

			if (blobActive) {
				// entry+0x00: frame cursor/offset X (not tracked in ScummVM, save 0)
				uint16 field00 = 0;
				s.syncAsUint16LE(field00);
				// entry+0x02: frame cursor/offset Y (not tracked in ScummVM, save 0)
				uint16 field02 = 0;
				s.syncAsUint16LE(field02);
				// entry+0x0C: source resource key
				uint16 blobSourceKey = 0;
				if (s.isSaving() && blobIdx < (int)obj->_blobSourceKeys.size())
					blobSourceKey = obj->_blobSourceKeys[blobIdx];
				s.syncAsUint16LE(blobSourceKey);
				// entry+0x0E: speed/timing
				uint16 blobSpeed = 0;
				if (s.isSaving() && blobIdx < (int)obj->_blobSpeeds.size())
					blobSpeed = obj->_blobSpeeds[blobIdx];
				s.syncAsUint16LE(blobSpeed);
				// entry+0x04: data size
				uint16 blobSize = 0;
				if (s.isSaving() && blobIdx < (int)obj->_blobs.size())
					blobSize = (uint16)obj->_blobs[blobIdx].size();
				s.syncAsUint16LE(blobSize);
				// Pixel data (blobSize bytes)
				if (s.isSaving()) {
					if (blobIdx < (int)obj->_blobs.size() && blobSize > 0)
						s.syncBytes(obj->_blobs[blobIdx].data(), blobSize);
				} else {
					// Loading: allocate and read pixel data
					if (blobSize > 0) {
						if (blobIdx >= (int)obj->_blobs.size())
							obj->_blobs.resize(blobIdx + 1);
						obj->_blobs[blobIdx].resize(blobSize);
						s.syncBytes(obj->_blobs[blobIdx].data(), blobSize);
					}
					if (blobIdx >= (int)obj->_blobSourceKeys.size())
						obj->_blobSourceKeys.resize(blobIdx + 1);
					obj->_blobSourceKeys[blobIdx] = blobSourceKey;
					if (blobIdx >= (int)obj->_blobSpeeds.size())
						obj->_blobSpeeds.resize(blobIdx + 1);
					obj->_blobSpeeds[blobIdx] = blobSpeed;
				}
			}
		}
	}

	// --- Post-load: rebuild view state ---
	if (s.isLoading()) {
		view1->_characters.clear();
		for (auto obj : GameObjects::instance()._objects) {
			if (obj == nullptr)
				continue;
			if (obj->_sceneIndex == (uint16)Scenes::instance()._currentSceneIndex) {
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
