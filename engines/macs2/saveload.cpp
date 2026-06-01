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
		View1 *currentView = (View1 *)findView("View1");
		currentView->_started = true;
		changeScene(sceneIndex, false);
	}

	// --- Script variables: 0x2000 bytes (2048 vars × 2 uint16) ---
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

	// g_wScriptClickFlag [0xf94]: 2 bytes - TODO: unknown, save 0
	uint16 scriptClickFlag = 0;
	s.syncAsUint16LE(scriptClickFlag);

	// g_wScriptClickX [0xf96]: 2 bytes - TODO: unknown, save 0
	uint16 scriptClickX = 0;
	s.syncAsUint16LE(scriptClickX);

	// g_wScriptClickY [0xf98]: 2 bytes - TODO: unknown, save 0
	uint16 scriptClickY = 0;
	s.syncAsUint16LE(scriptClickY);

	// g_wScriptClickResult [0xf9a]: 2 bytes - TODO: unknown, save 0
	uint16 scriptClickResult = 0;
	s.syncAsUint16LE(scriptClickResult);

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

	// g_wWalkTargetObjectIndex [0x1016]: 2 bytes - TODO: not mapped yet, save 0
	uint16 walkTargetObjectIndex = 0;
	s.syncAsUint16LE(walkTargetObjectIndex);

	// g_wPickupInProgress [0x1030]: 2 bytes
	uint16 pickupInProgress = _scriptExecutor->_pickupInProgress ? 1 : 0;
	s.syncAsUint16LE(pickupInProgress);
	if (s.isLoading())
		_scriptExecutor->_pickupInProgress = pickupInProgress != 0;

	// g_wActiveInventoryItemId [0xfd0]: 2 bytes - TODO: not mapped yet, save 0
	uint16 activeInventoryItemId = 0;
	s.syncAsUint16LE(activeInventoryItemId);

	// g_wSavedCursorMode [0xfea]: 2 bytes
	uint16 savedCursorMode = (uint16)_scriptExecutor->_cursorModeBeforeWait;
	s.syncAsUint16LE(savedCursorMode);
	if (s.isLoading())
		_scriptExecutor->_cursorModeBeforeWait = (Script::MouseMode)savedCursorMode;

	// g_wClipRectDirty [0xfec]: 1 byte - TODO: not mapped, save 0
	uint8 clipRectDirty = 0;
	s.syncAsByte(clipRectDirty);

	// g_wWalkTargetObjectIndex (duplicate) [0x1016]: 2 bytes
	s.syncAsUint16LE(walkTargetObjectIndex);

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

	// g_bMovementFinishedFlag [0x1020]: 1 byte - TODO: not mapped, save 0
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
	View1 *view1 = (View1 *)findView("View1");
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
	// 40 entries × 5 bytes each (1 byte active + 2 bytes value + 2 bytes remap)
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
				if (ov.Index == idx && ov.Active) {
					active = 1;
					overrideValue = ov.OverrideValue;
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
				ov.Active = true;
				ov.Index = AREA_OVERRIDE_MIN + i;
				ov.OverrideValue = overrideValue;
				_pathfindingOverrides.push_back(ov);
			}
			_areaOverrides[i] = remap;
		}
	}

	// --- Scene data: hotspot overrides [+0x5BD3]: 32 bytes (16 × uint16) ---
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

	// --- Scene data: timer params [+0x53C3..+0x53CF]: 4 × 4 bytes ---
	// TODO: timer state not fully mapped - save zeros
	for (int i = 0; i < 4; i++) {
		uint32 timerParam = 0;
		s.syncAsUint32LE(timerParam);
	}

	// --- Animation blob offsets (variable length) ---
	// Original: for each special anim (1..count), save 2-byte offset from getAnimBlobOffset
	// We save the count from the scene, then 2 bytes per entry
	uint16 numSpecialAnims = (uint16)_backgroundAnimationsBlobs.size();
	// The original doesn't write the count here - it reads it from scene data at +0x50F5
	// which was already loaded by changeScene. We just need to write the offsets.
	for (uint16 i = 0; i < numSpecialAnims; i++) {
		// TODO: animation blob offset tracking not implemented - save 0
		uint16 animOffset = 0;
		s.syncAsUint16LE(animOffset);
	}

	// --- PCM sound: size (2 bytes) + data (variable) ---
	// TODO: PCM sound buffer not tracked in ScummVM - save size=0
	uint16 pcmSoundSize = 0;
	s.syncAsUint16LE(pcmSoundSize);
	if (s.isLoading() && pcmSoundSize > 0) {
		// Skip PCM data we can't use
		byte dummy;
		for (uint16 i = 0; i < pcmSoundSize; i++)
			s.syncAsByte(dummy);
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

		// Runtime fields [+0x00..+0x0A]: 6 × uint16 (walk state)
		// TODO: walk state not fully mapped - save zeros
		for (int i = 0; i < 6; i++) {
			uint16 walkState = 0;
			s.syncAsUint16LE(walkState);
		}

		// [+0x0C]: 0x20 bytes (32 bytes) - TODO: unknown block, save zeros
		byte unknownBlock0C[32] = {0};
		s.syncBytes(unknownBlock0C, 32);

		// [+0x2C]: 2 bytes - TODO: unknown
		uint16 unknown2C = 0;
		s.syncAsUint16LE(unknown2C);

		// [+0x2E]: 2 bytes - TODO: unknown
		uint16 unknown2E = 0;
		s.syncAsUint16LE(unknown2E);

		// [+0x30]: 2 bytes - TODO: unknown
		uint16 unknown30 = 0;
		s.syncAsUint16LE(unknown30);

		// [+0x32]: 1 byte - IsClickable
		uint8 isClickable = obj->IsClickable ? 1 : 0;
		s.syncAsByte(isClickable);
		if (s.isLoading())
			obj->IsClickable = isClickable != 0;

		// [+0x33]: 1 byte - IsVisible
		uint8 isVisible = obj->IsVisible ? 1 : 0;
		s.syncAsByte(isVisible);
		if (s.isLoading())
			obj->IsVisible = isVisible != 0;

		// RuntimeValue217 [+0x20D]: 2 bytes
		s.syncAsUint16LE(obj->RuntimeValue217);
		// [+0x20F]: 2 bytes
		s.syncAsUint16LE(obj->RuntimeValue219);
		// [+0x211]: 2 bytes - TODO: unknown, save 0
		uint16 runtime211 = 0;
		s.syncAsUint16LE(runtime211);
		// [+0x213]: 2 bytes - TODO: unknown, save 0
		uint16 runtime213 = 0;
		s.syncAsUint16LE(runtime213);

		// RuntimeSlotValues [+0x21D..+0x22B]: 8 × uint16
		for (int i = 0; i < 8; i++) {
			uint16 slotVal = (i < 0x15) ? obj->RuntimeSlotValues[i] : 0;
			s.syncAsUint16LE(slotVal);
			if (s.isLoading() && i < 0x15)
				obj->RuntimeSlotValues[i] = slotVal;
		}

		// [+0x215..+0x21B]: 4 × uint16 - more slot values
		for (int i = 8; i < 12; i++) {
			uint16 slotVal = (i < 0x15) ? obj->RuntimeSlotValues[i] : 0;
			s.syncAsUint16LE(slotVal);
			if (s.isLoading() && i < 0x15)
				obj->RuntimeSlotValues[i] = slotVal;
		}

		// overloadAnimTriggerDirection [+0x22D]: 2 bytes
		s.syncAsUint16LE(obj->overloadAnimTriggerDirection);

		// RuntimeFlag22F [+0x22F]: 1 byte
		uint8 flag22F = obj->RuntimeFlag22F ? 1 : 0;
		s.syncAsByte(flag22F);
		if (s.isLoading())
			obj->RuntimeFlag22F = flag22F != 0;

		// useOverloadAnimation [+0x230]: 1 byte
		uint8 useOverload = obj->useOverloadAnimation ? 1 : 0;
		s.syncAsByte(useOverload);
		if (s.isLoading())
			obj->useOverloadAnimation = useOverload != 0;

		// [+0x184]: 1 byte - TODO: unknown, save 0
		uint8 unknown184 = 0;
		s.syncAsByte(unknown184);
		// [+0x185]: 1 byte - TODO: unknown, save 0
		uint8 unknown185 = 0;
		s.syncAsByte(unknown185);
		// [+0x186]: 1 byte - TODO: unknown, save 0
		uint8 unknown186 = 0;
		s.syncAsByte(unknown186);

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
		for (int blobIdx = 0; blobIdx < 0x15; blobIdx++) {
			// Blob active flag [+0x33 of blob entry]: 2 bytes (written as hasData)
			uint16 blobActive = 0;
			if (s.isSaving() && blobIdx < (int)obj->Blobs.size())
				blobActive = obj->Blobs[blobIdx].empty() ? 0 : 1;
			s.syncAsUint16LE(blobActive);
			if (s.isLoading() && blobIdx >= (int)obj->Blobs.size()) {
				obj->Blobs.resize(blobIdx + 1);
				obj->BlobSourceKeys.resize(blobIdx + 1, 0);
				obj->BlobMirrorFlags.resize(blobIdx + 1, false);
				obj->BlobSpeeds.resize(blobIdx + 1, 0);
			}

			if (blobActive) {
				// Blob header: X(2), Y(2), size(2), sourceKey(2), speed(2)
				// then blob data (size bytes)
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
					obj->Blobs[blobIdx].resize(blobSize);
					if (blobIdx < (int)obj->BlobSourceKeys.size())
						obj->BlobSourceKeys[blobIdx] = blobSourceKey;
					if (blobIdx < (int)obj->BlobSpeeds.size())
						obj->BlobSpeeds[blobIdx] = blobSpeed;
				}
				if (blobSize > 0)
					s.syncBytes(obj->Blobs[blobIdx].data(), blobSize);
			} else if (s.isLoading() && blobIdx < (int)obj->Blobs.size()) {
				obj->Blobs[blobIdx].clear();
			}
		}
	}

	// --- Post-load: rebuild view state ---
	if (s.isLoading()) {
		View1 *currentView = (View1 *)findView("View1");
		currentView->_characters.clear();
		for (auto obj : GameObjects::instance()._objects) {
			if (obj->SceneIndex == (uint16)Scenes::instance()._currentSceneIndex) {
				Character *c = new Character();
				c->_gameObject = obj;
				currentView->_characters.push_back(c);
			}
		}
		currentView->setInventorySource(GameObjects::instance().getProtagonistObject());
		currentView->updateCursor();
		currentView->_paletteDirty = true;
	}

	return Common::kNoError;
}

} // End of namespace Macs2
