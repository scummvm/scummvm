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

#ifndef HOLLYWOOD_GAMEPLAY_GAME_STATE_H
#define HOLLYWOOD_GAMEPLAY_GAME_STATE_H

#include "common/scummsys.h"

namespace Hollywood {

struct GameplayState {
	enum {
		kInventoryOwnerCount = 2,
		kInventoryOwnerSlotStride = 0x79,
		kInventoryVerbCount = 8,
		kInventoryFirstSlot = 1,
		kInventoryLastSlot = kInventoryOwnerSlotStride - 1,
		kInventoryVisibleSlotCount = 16,
		kFixedInventoryActionTableEntryCount = kInventoryOwnerSlotStride * kInventoryVerbCount + 1
	};

	GameplayState() {
		reset();
	}

	void reset() {
		mainFlowStateId = 0;
		activeChapterAudioArchiveIndex = 0;
		currentInventoryOwnerIndex = 0;
		currentRandomAmbientMusicTrackId = 0;
		for (uint owner = 0; owner < kInventoryOwnerCount; ++owner) {
			inventoryItemCountByOwner[owner] = 0;
			inventoryFirstVisibleSlotByOwner[owner] = 0;
			for (uint slot = 0; slot < kInventoryOwnerSlotStride; ++slot) {
				inventorySlotItemIdByOwner[owner][slot] = 0;
				inventoryItemSlotByOwnerAndItemId[owner][slot] = 0;
			}
		}
		for (uint i = 0; i < kFixedInventoryActionTableEntryCount; ++i)
			fixedInventoryVerbHandlerIdsByItemAndStrip[i] = 0;
		actorSpriteBankSet00Loaded = false;
		inventoryOwner1ResourceTablesLoaded = false;
		inventoryOwner1ItemsInitialized = false;
		sceneActionCallbackTableInstalled = false;
		inventoryPanelRedrawn = false;
		g01Item0BSequenceCompleted = false;
		g04EntryConversationPlayed = false;
		g04MajorActionProgress = 0;
		g04PatchState = 0;
		g04ExitActionDone = false;
		musicEnabled = true;
		soundEffectsEnabled = true;
		optionsTestAudioEnabled = false;
		musicVolumeLevel = 200;
		soundEffectsVolumeLevel = 200;
		voiceVolumeLevel = 200;
		speechTextSpeedLevel = 200;
		actorSpeechTextMode = 1;
	}

	void initializeForState7000() {
		actorSpriteBankSet00Loaded = true;
		inventoryOwner1ResourceTablesLoaded = true;
		inventoryOwner1ItemsInitialized = true;
		sceneActionCallbackTableInstalled = true;
		currentInventoryOwnerIndex = 1;
		activeChapterAudioArchiveIndex = 7;
		currentRandomAmbientMusicTrackId = 0x0c;
		inventoryPanelRedrawn = true;
		mainFlowStateId = 0x1b62;
	}

	bool hasInventoryItem(byte owner, byte itemId) const {
		if (owner >= kInventoryOwnerCount || itemId >= kInventoryOwnerSlotStride)
			return false;

		return inventoryItemSlotByOwnerAndItemId[owner][itemId] != 0;
	}

	void addInventoryItem(byte owner, byte itemId) {
		if (owner >= kInventoryOwnerCount || itemId >= kInventoryOwnerSlotStride ||
				hasInventoryItem(owner, itemId) ||
				inventoryItemCountByOwner[owner] >= kInventoryLastSlot)
			return;

		const byte slot = (byte)(inventoryItemCountByOwner[owner] + 1);
		inventoryItemCountByOwner[owner] = slot;
		inventorySlotItemIdByOwner[owner][slot] = itemId;
		inventoryItemSlotByOwnerAndItemId[owner][itemId] = slot;
		inventoryFirstVisibleSlotByOwner[owner] = firstVisibleInventorySlotForCount(slot);
		inventoryPanelRedrawn = true;
	}

	void removeInventoryItem(byte owner, byte itemId) {
		if (owner >= kInventoryOwnerCount || itemId >= kInventoryOwnerSlotStride)
			return;

		const byte slot = inventoryItemSlotByOwnerAndItemId[owner][itemId];
		if (slot == 0)
			return;

		inventoryItemSlotByOwnerAndItemId[owner][itemId] = 0;
		inventorySlotItemIdByOwner[owner][slot] = 0;
		compactInventory(owner);
		inventoryPanelRedrawn = true;
	}

	void compactInventory(byte owner) {
		if (owner >= kInventoryOwnerCount)
			return;

		byte writeSlot = kInventoryFirstSlot;
		for (byte readSlot = kInventoryFirstSlot; readSlot <= inventoryItemCountByOwner[owner]; ++readSlot) {
			const byte itemId = inventorySlotItemIdByOwner[owner][readSlot];
			if (itemId == 0)
				continue;

			inventorySlotItemIdByOwner[owner][writeSlot] = itemId;
			inventoryItemSlotByOwnerAndItemId[owner][itemId] = writeSlot;
			++writeSlot;
		}

		for (byte slot = writeSlot; slot <= inventoryItemCountByOwner[owner]; ++slot)
			inventorySlotItemIdByOwner[owner][slot] = 0;
		inventoryItemCountByOwner[owner] = (byte)(writeSlot - 1);
		inventoryFirstVisibleSlotByOwner[owner] =
			firstVisibleInventorySlotForCount(inventoryItemCountByOwner[owner]);
	}

	byte firstVisibleInventorySlotForCount(byte itemCount) const {
		if (itemCount <= kInventoryVisibleSlotCount)
			return kInventoryFirstSlot;

		return (byte)(((itemCount - 9) & ~7) + 1);
	}

	uint16 fixedInventoryVerbHandler(byte owner, byte itemId, byte stripIndex) const {
		if (owner != 1 || itemId >= kInventoryOwnerSlotStride || stripIndex == 0 ||
				stripIndex > kInventoryVerbCount)
			return 0;

		const uint index = (uint)itemId * kInventoryVerbCount + stripIndex;
		if (index >= kFixedInventoryActionTableEntryCount)
			return 0;

		return fixedInventoryVerbHandlerIdsByItemAndStrip[index];
	}

	uint16 mainFlowStateId;
	byte activeChapterAudioArchiveIndex;
	byte currentInventoryOwnerIndex;
	byte currentRandomAmbientMusicTrackId;
	byte inventoryItemCountByOwner[kInventoryOwnerCount];
	byte inventoryFirstVisibleSlotByOwner[kInventoryOwnerCount];
	byte inventorySlotItemIdByOwner[kInventoryOwnerCount][kInventoryOwnerSlotStride];
	byte inventoryItemSlotByOwnerAndItemId[kInventoryOwnerCount][kInventoryOwnerSlotStride];
	uint16 fixedInventoryVerbHandlerIdsByItemAndStrip[kFixedInventoryActionTableEntryCount];
	bool actorSpriteBankSet00Loaded;
	bool inventoryOwner1ResourceTablesLoaded;
	bool inventoryOwner1ItemsInitialized;
	bool sceneActionCallbackTableInstalled;
	bool inventoryPanelRedrawn;
	bool g01Item0BSequenceCompleted;
	bool g04EntryConversationPlayed;
	byte g04MajorActionProgress;
	byte g04PatchState;
	bool g04ExitActionDone;
	bool musicEnabled;
	bool soundEffectsEnabled;
	bool optionsTestAudioEnabled;
	byte musicVolumeLevel;
	byte soundEffectsVolumeLevel;
	byte voiceVolumeLevel;
	byte speechTextSpeedLevel;
	byte actorSpeechTextMode;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_GAME_STATE_H
