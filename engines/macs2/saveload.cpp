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

#include "macs2/gameobjects.h"
#include "macs2/macs2.h"
#include "macs2/view1.h"

namespace Macs2 {

// Binary-compatible save format matching the original DOS game's
// saveGameToFile (1008:6859) and loadGameFromFile (1008:747e).
// Save files are interchangeable between ScummVM and the original game.

Common::Error Macs2Engine::syncGame(Common::Serializer &s) {
	const byte SAVE_MAGIC[12] = {'A', 'H', 'F', 'F', 'M', 'S', 'G', 'M', '0', '1', '0', '0'};
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

	// --- 21-byte slot name (Pascal string: [len][chars], zero-padded to 21) ---
	// The original save/load menu (initSaveLoadPanel 1008:6184) reads these 21
	// bytes and renders them with drawText as a length-prefixed string. A zero
	// length byte makes the slot appear empty in the original, so write a real
	// name when producing an original-format save.
	byte slotName[21] = {0};
	if (s.isSaving()) {
		const char *defName = "SCUMMVM";
		uint8 len = (uint8)strlen(defName);
		if (len > 20)
			len = 20;
		slotName[0] = len;
		memcpy(slotName + 1, defName, len);
	}
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
	// Must always be 0x800 entries to match the binary's fixed-size block; do not
	// key off _variables.size() in case it ever differs.
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

	// --- g_wSoundSystemActive: 2 bytes [0x1f4c] ---
	uint16 soundSystemActive = _scriptExecutor->_soundSystemActive ? 1 : 0;
	s.syncAsUint16LE(soundSystemActive);
	if (s.isLoading())
		_scriptExecutor->_soundSystemActive = soundSystemActive != 0;

	// --- Script execution state ---
	// g_wScriptIsExecuting [0xf88]: 1 byte
	uint8 scriptIsExecuting = _scriptExecutor->isExecuting() ? 1 : 0;
	s.syncAsByte(scriptIsExecuting);
	if (s.isLoading()) {
		if (scriptIsExecuting == 0)
			_scriptExecutor->setIdle();
	}

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
		// Restore script execution state matching binary loadGameFromFile (1008:747e).
		// The original sets g_wScriptDataPtrLow/High based on executingObjectId:
		//   objectId == 0: use scene script (sceneData+0x5207/0x5209)
		//   objectId != 0: use object's runtime script (runtime+0x187/0x189)
		// Then g_wScriptPosition is used by the executor to seek within that script.
		if (scriptIsExecuting) {
			if (executingObjectId == 0) {
				// Scene script
				_scriptExecutor->setCurrentSceneScriptAt(scriptPosition);
			} else {
				// Object script: find the object and set its script stream
				GameObject *execObj = GameObjects::getObjectByIndex(executingObjectId);
				if (execObj && !execObj->_script.empty()) {
					Common::MemoryReadStream *objStream = execObj->getScriptStream();
					_scriptExecutor->setScript(objStream);
					if (objStream && scriptPosition < objStream->size())
						objStream->seek(scriptPosition, SEEK_SET);
				}
			}
			_scriptExecutor->setExecutingObjectId(executingObjectId);
		} else {
			_scriptExecutor->setIdle();
			_scriptExecutor->setExecutingObjectId(executingObjectId);
		}
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
	uint16 savedCursorMode = view1 ? (uint16)view1->_savedCursorMode : (uint16)Script::MouseMode::Walk;
	s.syncAsUint16LE(savedCursorMode);
	if (s.isLoading() && view1)
		view1->_savedCursorMode = (Script::MouseMode)savedCursorMode;

	// g_wClipRectDirty [0xfec]: 1 byte - flags clip region needs full-screen reset
	uint8 clipRectDirty = _clipRectDirty ? 1 : 0;
	s.syncAsByte(clipRectDirty);
	if (s.isLoading())
		_clipRectDirty = clipRectDirty != 0;

	// g_wWalkTargetObjectIndex (duplicate) [0x1016]: 2 bytes
	s.syncAsUint16LE(_scriptExecutor->_walkTargetObjectIndex);

	// PTR_LOOP_1020_1018 [0x1018]: 2 bytes - mouse mode
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
	s.syncAsUint16LE(_scriptExecutor->_interactedInventoryItemId);

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
			seqPos = READ_LE_UINT16(&blob._blob[2]); // header word at +2 = sequence position
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
		if (s.isLoading() && chr) {
			chr->_targetPosition.x = (int16)targetX;
			chr->_targetPosition.y = (int16)targetY;
			chr->_stepDeltaX = (int16)deltaX;
			chr->_stepDeltaY = (int16)deltaY;
			chr->_pathFinalDestination.x = (int16)finalX;
			chr->_pathFinalDestination.y = (int16)finalY;
		}

		// [+0x0C]: 0x20 bytes - raw runtime path block (opaque per-waypoint data
		// in the original). Preserved verbatim for byte-exact DOS save round-trip.
		byte pathBlock[32] = {0};
		if (s.isSaving() && chr)
			memcpy(pathBlock, chr->_pathBlockRaw, 32);
		s.syncBytes(pathBlock, 32);

		// [+0x2C]: 2 bytes - path current index
		uint16 pathIndex = chr ? (uint16)chr->_currentPathIndex : 0;
		s.syncAsUint16LE(pathIndex);

		// [+0x2E]: 2 bytes - path length
		uint16 pathLength = chr ? (uint16)chr->_path.size() : 0;
		s.syncAsUint16LE(pathLength);

		if (s.isLoading() && chr) {
			memcpy(chr->_pathBlockRaw, pathBlock, 32);
			chr->_path.clear();
			for (uint16 pi = 0; pi < pathLength && pi < 32; pi++)
				chr->_path.push_back(pathBlock[pi]);
			chr->_currentPathIndex = (int)pathIndex;
		}

		// [+0x30]: 2 bytes - step accumulator (Bresenham error)
		uint16 stepAccum = chr ? (uint16)chr->_stepError : 0;
		s.syncAsUint16LE(stepAccum);
		if (s.isLoading() && chr)
			chr->_stepError = (int16)stepAccum;

		// [+0x32]: 1 byte - walk step flag (legacy, no binary equivalent)
		uint8 walkStepFlag = 0;
		s.syncAsByte(walkStepFlag);
		// Not used - binary has no _isLerping flag

		// [+0x33]: 1 byte - direction set flag (_stepDirectionSet)
		uint8 directionSet = chr ? (chr->_stepDirectionSet ? 1 : 0) : 0;
		s.syncAsByte(directionSet);
		if (s.isLoading() && chr)
			chr->_stepDirectionSet = directionSet != 0;

		// [+0x20D..+0x213]: 4 x uint16 - per-object dirty rect (inclusive coords)
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

		// [+0x21D..+0x22B]: 8 x uint16 - motion vertical offset state + sprite draw bounds
		// +0x21D: motion target vertical offset (scriptSetMotion param 1)
		// +0x21F: vOffset accumulator (Bresenham error, runtime running total)
		// +0x221: vOffset threshold (scriptSetMotion param 3: motionDistance)
		// +0x223: vOffset step delta (scriptSetMotion param 2: verticalOffsetDelta)
		// +0x225..+0x22B: transient sprite draw bounds (recalculated each frame)
		uint16 motionTarget = chr ? chr->_motionTargetVerticalOffset : 0;
		s.syncAsUint16LE(motionTarget);
		if (s.isLoading() && chr)
			chr->_motionTargetVerticalOffset = motionTarget;
		uint16 motionAccum = chr ? chr->_motionProgress : 0;
		s.syncAsUint16LE(motionAccum);
		if (s.isLoading() && chr)
			chr->_motionProgress = motionAccum;
		uint16 motionThreshold = chr ? chr->_motionDistanceUnits : 0;
		s.syncAsUint16LE(motionThreshold);
		if (s.isLoading() && chr)
			chr->_motionDistanceUnits = motionThreshold;
		uint16 motionStep = chr ? chr->_motionVerticalOffsetDelta : 0;
		s.syncAsUint16LE(motionStep);
		if (s.isLoading() && chr)
			chr->_motionVerticalOffsetDelta = motionStep;
		// +0x225..+0x22B: sprite draw bounds from previous frame
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
		s.syncAsUint16LE(obj->_overloadAnimTriggerDirection);

		// _snapToTarget [+0x22F]: 1 byte
		uint8 snapToTarget = obj->_snapToTarget ? 1 : 0;
		s.syncAsByte(snapToTarget);
		if (s.isLoading())
			obj->_snapToTarget = snapToTarget != 0;

		// useOverloadAnimation [+0x230]: 1 byte
		uint8 useOverload = obj->_useOverloadAnimation ? 1 : 0;
		s.syncAsByte(useOverload);
		if (s.isLoading())
			obj->_useOverloadAnimation = useOverload != 0;

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

		// NOTE: the binary save record has NO isClickable/isVisible fields here.
		// loadGameFromFile/saveGameToFile go directly from +0x186 (hasScaling) to
		// +0x18B (scriptSize). These flags are not persisted by the original (they
		// are re-established by script execution), so we must not serialize them
		// or the object record length diverges from the original file.

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
			// Slot 0x15 (blobIdx 20) is the overload animation, stored separately
			bool isOverloadSlot = (blobIdx == 20);

			// Active flag (2 bytes, but only low byte matters)
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
				// entry+0x00: frame cursor/offset X (not tracked in ScummVM, save 0)
				uint16 field00 = 0;
				s.syncAsUint16LE(field00);
				// entry+0x02: frame cursor/offset Y (not tracked in ScummVM, save 0)
				uint16 field02 = 0;
				s.syncAsUint16LE(field02);
				// entry+0x0C: source resource key
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
				// entry+0x0E: speed/timing
				uint16 blobSpeed = 0;
				if (s.isSaving() && !isOverloadSlot && blobIdx < (int)obj->_blobWalkSpeeds.size())
					blobSpeed = obj->_blobWalkSpeeds[blobIdx];
				s.syncAsUint16LE(blobSpeed);
				// entry+0x04: data size
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

	// --- Post-load: rebuild view state ---
	if (s.isLoading()) {
		// NOTE: Characters were already created right after changeScene (above) so
		// the per-object loop could populate their runtime walk/draw/dirty state.
		// Do NOT recreate them here - that would discard the loaded fields.
		view1->rebuildCharacterLookupTable();
		view1->setInventorySource(GameObjects::instance().getProtagonistObject());

		// Restore UseInventory cursor image after load.
		// The cursor slot is only populated when clicking an inventory item in the panel;
		// after loading a save with mouseMode==UseInventory, the slot is empty.
		if (_scriptExecutor->_cursorMode == Script::MouseMode::UseInventory && view1->_activeInventoryItem != nullptr) {
			AnimFrame *icon = view1->getInventoryIcon(view1->_activeInventoryItem);
			if (icon != nullptr) {
				int cursorSlot = (int)Script::MouseMode::UseInventory - 1;
				_imageResources[cursorSlot] = *icon;
			}
		}

		view1->updateCursor();
		view1->_paletteDirty = true;

		// Binary loadGameFromFile (1008:82a7): help-disabled path uses
		// applyScenePaletteEffect + partial UI palette; else fade from black.
		if (view1->isHelpButtonDisabled()) {
			applyScenePaletteEffect();
			view1->restoreUiPaletteEntries();
		} else {
			view1->startFadingWithSpeed(8);
		}
	}

	return Common::kNoError;
}

} // End of namespace Macs2
