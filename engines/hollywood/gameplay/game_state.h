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
	GameplayState() {
		reset();
	}

	void reset() {
		mainFlowStateId = 0;
		activeChapterAudioArchiveIndex = 0;
		currentInventoryOwnerIndex = 0;
		currentRandomAmbientMusicTrackId = 0;
		inventoryFirstVisibleSlotByOwner[0] = 0;
		inventoryFirstVisibleSlotByOwner[1] = 0;
		actorSpriteBankSet00Loaded = false;
		inventoryOwner1ResourceTablesLoaded = false;
		inventoryOwner1ItemsInitialized = false;
		sceneActionCallbackTableInstalled = false;
		inventoryPanelRedrawn = false;
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

	uint16 mainFlowStateId;
	byte activeChapterAudioArchiveIndex;
	byte currentInventoryOwnerIndex;
	byte currentRandomAmbientMusicTrackId;
	byte inventoryFirstVisibleSlotByOwner[2];
	bool actorSpriteBankSet00Loaded;
	bool inventoryOwner1ResourceTablesLoaded;
	bool inventoryOwner1ItemsInitialized;
	bool sceneActionCallbackTableInstalled;
	bool inventoryPanelRedrawn;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_GAME_STATE_H
