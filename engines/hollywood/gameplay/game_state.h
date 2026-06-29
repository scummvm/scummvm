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
		kFixedInventoryActionTableEntryCount = kInventoryOwnerSlotStride * kInventoryVerbCount + 1,
		kInventoryItemRelationTableEntryCount = kInventoryOwnerSlotStride * kInventoryOwnerSlotStride
	};

	GameplayState() {
		reset();
	}

	void reset() {
		mainFlowStateId = 0;
		activeAudioChapterIndex = 0;
		currentInventoryOwnerIndex = 0;
		currentAmbientMusicCueId = 0;
		for (uint owner = 0; owner < kInventoryOwnerCount; ++owner) {
			inventoryItemCountByOwner[owner] = 0;
			inventoryFirstVisibleSlotByOwner[owner] = 0;
			for (uint slot = 0; slot < kInventoryOwnerSlotStride; ++slot) {
				inventorySlotItemIdByOwner[owner][slot] = 0;
				inventoryItemSlotByOwnerAndItemId[owner][slot] = 0;
				inventoryItemResourcePageByOwnerAndItemId[owner][slot] = 0;
			}
		}
		clearInventoryActionTables();
		sharedActorSpriteBankLoaded = false;
		sueInventoryResourceTablesLoaded = false;
		sueInventoryInitialized = false;
		sceneActionCallbacksInstalled = false;
		inventoryPanelDirty = false;
		multiToolKnifeState = 0;
		reviewedFrankensteinNote = false;
		frankensteinNoteOverlayMode = 0;
		juniorDialogueBranchState = 0;
		juniorDialogueFollowUpSeen = false;
		seenOfficeEntryConversation = false;
		officeStatueActionProgress = 0;
		officeNotePickupState = 0;
		openedOfficeClosetDoor = false;
		spokenToCloakroomAttendant = false;
		cloakroomRagVisible = 1;
		spokenToBruno = false;
		activatedLabExitMachine = false;
		labMachineSpeed = 4;
		seenGramophoneRoomIntro = false;
		gramophoneRoomDoorState = 0;
		gramophoneCrankState = 0;
		seenHannoverOfficeIntro = false;
		crankOnHannoverDesk = true;
		seenHannoverBedroomIntro = false;
		movedBedroomArmor = false;
		knowsHannoverWaitingForSecretary = false;
		cellPipesActive = true;
		cellPlateRatProgress = 0;
		posterOnCellWall = true;
		cellPlateRemoved = false;
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
		sharedActorSpriteBankLoaded = true;
		sueInventoryResourceTablesLoaded = false;
		sceneActionCallbacksInstalled = true;
		initializeSueItemResourcePages();
		initializeSueInventoryItems(false);
		currentInventoryOwnerIndex = 1;
		activeAudioChapterIndex = 7;
		currentAmbientMusicCueId = 0x0c;
		inventoryPanelDirty = true;
		mainFlowStateId = 0x1b62;
	}

	void initializeSueItemResourcePages() {
		if (kInventoryOwnerCount <= 1)
			return;

		for (uint itemId = 0; itemId < kInventoryOwnerSlotStride; ++itemId)
			inventoryItemResourcePageByOwnerAndItemId[1][itemId] = 0;

		inventoryItemResourcePageByOwnerAndItemId[1][0x01] = 0x6f;
		inventoryItemResourcePageByOwnerAndItemId[1][0x02] = 0x7c;
		inventoryItemResourcePageByOwnerAndItemId[1][0x05] = 0x7a;
		inventoryItemResourcePageByOwnerAndItemId[1][0x06] = 0x78;
		inventoryItemResourcePageByOwnerAndItemId[1][0x07] = 0x06;
		inventoryItemResourcePageByOwnerAndItemId[1][0x08] = 0x76;
		inventoryItemResourcePageByOwnerAndItemId[1][0x09] = 0x77;
		inventoryItemResourcePageByOwnerAndItemId[1][0x0b] = 0x74;
		inventoryItemResourcePageByOwnerAndItemId[1][0x0c] = 0x71;
		inventoryItemResourcePageByOwnerAndItemId[1][0x0d] = 0x72;
		inventoryItemResourcePageByOwnerAndItemId[1][0x0f] = 0x70;
		inventoryItemResourcePageByOwnerAndItemId[1][0x10] = 0x75;
		inventoryItemResourcePageByOwnerAndItemId[1][0x11] = 0x73;
		inventoryItemResourcePageByOwnerAndItemId[1][0x13] = 0x79;
		inventoryItemResourcePageByOwnerAndItemId[1][0x14] = 0x65;
		inventoryItemResourcePageByOwnerAndItemId[1][0x15] = 0x6b;
		inventoryItemResourcePageByOwnerAndItemId[1][0x16] = 0x55;
		inventoryItemResourcePageByOwnerAndItemId[1][0x17] = 0x03;
		inventoryItemResourcePageByOwnerAndItemId[1][0x18] = 0x61;
		inventoryItemResourcePageByOwnerAndItemId[1][0x19] = 0x26;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1a] = 0x68;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1b] = 0x2f;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1c] = 0x3f;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1d] = 0x14;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1e] = 0x29;
		inventoryItemResourcePageByOwnerAndItemId[1][0x1f] = 0x4a;
		inventoryItemResourcePageByOwnerAndItemId[1][0x20] = 0x52;
		inventoryItemResourcePageByOwnerAndItemId[1][0x21] = 0x4b;
		inventoryItemResourcePageByOwnerAndItemId[1][0x22] = 0x7b;
	}

	void initializeSueInventoryItems(bool giveAllItems) {
		if (kInventoryOwnerCount <= 1)
			return;

		const byte owner = 1;
		for (uint slot = 0; slot < kInventoryOwnerSlotStride; ++slot) {
			inventorySlotItemIdByOwner[owner][slot] = 0;
			inventoryItemSlotByOwnerAndItemId[owner][slot] = 0;
		}

		if (giveAllItems) {
			giveInventoryItemsWithResourcePages(owner);
			sueInventoryInitialized = true;
			return;
		}

		inventorySlotItemIdByOwner[owner][1] = 1;
		inventorySlotItemIdByOwner[owner][2] = 7;
		inventorySlotItemIdByOwner[owner][3] = 2;
		inventorySlotItemIdByOwner[owner][4] = 5;
		inventoryItemSlotByOwnerAndItemId[owner][1] = 1;
		inventoryItemSlotByOwnerAndItemId[owner][2] = 3;
		inventoryItemSlotByOwnerAndItemId[owner][5] = 4;
		inventoryItemSlotByOwnerAndItemId[owner][9] = 2;
		inventoryItemCountByOwner[owner] = 4;
		inventoryFirstVisibleSlotByOwner[owner] = firstVisibleInventorySlotForCount(4);
		sueInventoryInitialized = true;
		inventoryPanelDirty = true;
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
		inventoryPanelDirty = true;
	}

	byte giveInventoryItemsWithResourcePages(byte owner) {
		if (owner >= kInventoryOwnerCount)
			return 0;

		for (uint slot = 0; slot < kInventoryOwnerSlotStride; ++slot) {
			inventorySlotItemIdByOwner[owner][slot] = 0;
			inventoryItemSlotByOwnerAndItemId[owner][slot] = 0;
		}

		byte writeSlot = kInventoryFirstSlot;
		for (byte itemId = kInventoryFirstSlot; itemId < kInventoryOwnerSlotStride; ++itemId) {
			if (inventoryItemResourcePageByOwnerAndItemId[owner][itemId] == 0)
				continue;

			inventorySlotItemIdByOwner[owner][writeSlot] = itemId;
			inventoryItemSlotByOwnerAndItemId[owner][itemId] = writeSlot;
			++writeSlot;
		}

		inventoryItemCountByOwner[owner] = (byte)(writeSlot - 1);
		inventoryFirstVisibleSlotByOwner[owner] = kInventoryFirstSlot;
		inventoryPanelDirty = true;
		return inventoryItemCountByOwner[owner];
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
		inventoryPanelDirty = true;
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

	uint16 dialogueInventoryRelationHandler(byte primaryItemId, byte secondaryItemId, byte relationMode) const {
		if (primaryItemId >= kInventoryOwnerSlotStride || secondaryItemId >= kInventoryOwnerSlotStride)
			return 0;

		const uint index = (uint)primaryItemId * kInventoryOwnerSlotStride + secondaryItemId;
		if (index >= kInventoryItemRelationTableEntryCount)
			return 0;

		if (relationMode == 1)
			return dialogueRelationMode1HandlerIdsByItemPair[index];
		if (relationMode == 2)
			return dialogueRelationMode2HandlerIdsByItemPair[index];

		return 0;
	}

	void clearInventoryActionTables() {
		for (uint i = 0; i < kFixedInventoryActionTableEntryCount; ++i)
			fixedInventoryVerbHandlerIdsByItemAndStrip[i] = 0;
		for (uint i = 0; i < kInventoryItemRelationTableEntryCount; ++i) {
			dialogueRelationMode1HandlerIdsByItemPair[i] = 0;
			dialogueRelationMode2HandlerIdsByItemPair[i] = 0;
		}
	}

	uint16 mainFlowStateId;
	byte activeAudioChapterIndex;
	byte currentInventoryOwnerIndex;
	byte currentAmbientMusicCueId;
	byte inventoryItemCountByOwner[kInventoryOwnerCount];
	byte inventoryFirstVisibleSlotByOwner[kInventoryOwnerCount];
	byte inventorySlotItemIdByOwner[kInventoryOwnerCount][kInventoryOwnerSlotStride];
	byte inventoryItemSlotByOwnerAndItemId[kInventoryOwnerCount][kInventoryOwnerSlotStride];
	byte inventoryItemResourcePageByOwnerAndItemId[kInventoryOwnerCount][kInventoryOwnerSlotStride];
	uint16 fixedInventoryVerbHandlerIdsByItemAndStrip[kFixedInventoryActionTableEntryCount];
	uint16 dialogueRelationMode1HandlerIdsByItemPair[kInventoryItemRelationTableEntryCount];
	uint16 dialogueRelationMode2HandlerIdsByItemPair[kInventoryItemRelationTableEntryCount];
	bool sharedActorSpriteBankLoaded;
	bool sueInventoryResourceTablesLoaded;
	bool sueInventoryInitialized;
	bool sceneActionCallbacksInstalled;
	bool inventoryPanelDirty;
	byte multiToolKnifeState;
	bool reviewedFrankensteinNote;
	byte frankensteinNoteOverlayMode;
	byte juniorDialogueBranchState;
	bool juniorDialogueFollowUpSeen;
	bool seenOfficeEntryConversation;
	byte officeStatueActionProgress;
	byte officeNotePickupState;
	bool openedOfficeClosetDoor;
	bool spokenToCloakroomAttendant;
	byte cloakroomRagVisible;
	bool spokenToBruno;
	bool activatedLabExitMachine;
	byte labMachineSpeed;
	bool seenGramophoneRoomIntro;
	byte gramophoneRoomDoorState;
	byte gramophoneCrankState;
	bool seenHannoverOfficeIntro;
	bool crankOnHannoverDesk;
	bool seenHannoverBedroomIntro;
	bool movedBedroomArmor;
	bool knowsHannoverWaitingForSecretary;
	bool cellPipesActive;
	byte cellPlateRatProgress;
	bool posterOnCellWall;
	bool cellPlateRemoved;
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
