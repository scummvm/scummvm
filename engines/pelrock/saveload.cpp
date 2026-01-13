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
#include "common/savefile.h"

#include "pelrock.h"
#include "pelrock/pelrock.h"
#include "pelrock/types.h"

namespace Pelrock {

#define SAVEGAME_CURRENT_VERSION 1

// Helper functions for syncing structs
void syncSticker(Common::Serializer &s, Sticker &sticker) {
	s.syncAsSint32LE(sticker.stickerIndex);
	// if(s.isLoading()) {

	// }
	// s.syncAsSint32LE(sticker.roomNumber);

	// if(s.isLoading()) {
	//     sticker.stickerData = new byte[sticker.w * sticker.h];
	// }
	// s.syncAsUint16LE(sticker.x);
	// s.syncAsUint16LE(sticker.y);
	// s.syncAsByte(sticker.w);
	// s.syncAsByte(sticker.h);
	// // Note: stickerData pointer not serialized - must be reconstructed on load
}

void syncExit(Common::Serializer &s, Exit &exit) {
	s.syncAsByte(exit.index);
	s.syncAsSint16LE(exit.x);
	s.syncAsSint16LE(exit.y);
	s.syncAsByte(exit.w);
	s.syncAsByte(exit.h);
	s.syncAsUint16LE(exit.targetRoom);
	s.syncAsSint16LE(exit.targetX);
	s.syncAsSint16LE(exit.targetY);
	s.syncAsUint16LE(exit.targetDir);
	s.syncAsByte((byte &)exit.dir);
	s.syncAsByte(exit.isEnabled);
}

void syncExitChange(Common::Serializer &s, ExitChange &change) {
	s.syncAsByte(change.roomNumber);
	s.syncAsByte(change.exitIndex);
	syncExit(s, change.exit);
}

void syncWalkBox(Common::Serializer &s, WalkBox &walkbox) {
	s.syncAsSint16LE(walkbox.x);
	s.syncAsSint16LE(walkbox.y);
	s.syncAsSint16LE(walkbox.w);
	s.syncAsSint16LE(walkbox.h);
	s.syncAsByte(walkbox.flags);
}

void syncWalkBoxChange(Common::Serializer &s, WalkBoxChange &change) {
	s.syncAsByte(change.roomNumber);
	s.syncAsByte(change.walkboxIndex);
	syncWalkBox(s, change.walkbox);
}

void syncHotSpot(Common::Serializer &s, HotSpot &hotspot) {
	s.syncAsByte(hotspot.index);
	s.syncAsByte(hotspot.innerIndex);
	s.syncAsSint32LE(hotspot.id);
	s.syncAsSint16LE(hotspot.x);
	s.syncAsSint16LE(hotspot.y);
	s.syncAsSint16LE(hotspot.w);
	s.syncAsSint16LE(hotspot.h);
	s.syncAsByte(hotspot.actionFlags);
	s.syncAsByte(hotspot.extra);
	s.syncAsByte((byte &)hotspot.isEnabled);
	s.syncAsByte((byte &)hotspot.isSprite);
	s.syncAsByte(hotspot.zOrder);
}

void syncHotSpotChange(Common::Serializer &s, HotSpotChange &change) {
	s.syncAsByte(change.roomNumber);
	s.syncAsByte(change.hotspotIndex);
	syncHotSpot(s, change.hotspot);
}

void syncResetEntry(Common::Serializer &s, ResetEntry &entry) {
	s.syncAsUint16LE(entry.room);
	s.syncAsUint16LE(entry.offset);
	s.syncAsByte(entry.dataSize);
	if (s.isLoading()) {
		entry.data = new byte[entry.dataSize];
	}
	for (int j = 0; j < entry.dataSize; ++j) {
		s.syncAsByte(entry.data[j]);
	}
}

bool syncGeneralData(Common::Serializer &s, SaveGameData *game) {
	// Byte
	s.syncAsByte(game->currentRoom);
	// Uint16
	s.syncAsUint16LE(game->alfredX);
	s.syncAsUint16LE(game->alfredY);
	s.syncAsByte(game->alfredDir);

	return !s.err();
}

bool syncGameStateData(Common::Serializer &s, GameStateData *gameState) {
	// GameState
	s.syncAsUint32LE((uint32 &)gameState->stateGame);

	// Inventory items
	uint16 inventorySize = (uint16)gameState->inventoryItems.size();
	s.syncAsUint16LE(inventorySize);
	if (s.isLoading()) {
		gameState->inventoryItems.resize(inventorySize);
	}
	for (uint16 i = 0; i < inventorySize; ++i) {
		s.syncAsByte(gameState->inventoryItems[i]);
	}
	// Selected inventory item
	s.syncAsSint16LE(gameState->selectedInventoryItem);

	// Room stickers
	uint16 stickersSize = (uint16)gameState->roomStickers.size();
	s.syncAsUint16LE(stickersSize);
	if (s.isSaving()) {
		for (const auto &pair : gameState->roomStickers) {
			byte roomNumber = pair._key;
			s.syncAsByte(roomNumber);
			const Common::Array<Sticker> &stickers = pair._value;
			uint16 numStickers = (uint16)stickers.size();
			s.syncAsUint16LE(numStickers);
			for (uint16 i = 0; i < numStickers; ++i) {
				Sticker sticker = stickers[i];
				syncSticker(s, sticker);
			}
		}
	} else {
		gameState->roomStickers.clear();
		for (uint16 idx = 0; idx < stickersSize; ++idx) {
			byte roomNumber;
			s.syncAsByte(roomNumber);
			uint16 numStickers;
			s.syncAsUint16LE(numStickers);
			Common::Array<Sticker> stickers;
			for (uint16 i = 0; i < numStickers; ++i) {
				int stickerIndex = 0;
				s.syncAsSint32LE(stickerIndex);
				stickers.push_back(g_engine->_res->getSticker(stickerIndex));
			}
			gameState->roomStickers[roomNumber] = stickers;
		}
	}

	// Room exit changes
	uint16 numExits = (uint16)gameState->roomExitChanges.size();
	s.syncAsUint16LE(numExits);
	if (s.isSaving()) {
		for (const auto &exitPair : gameState->roomExitChanges) {
			byte roomNumber = exitPair._key;
			s.syncAsByte(roomNumber);
			const Common::Array<ExitChange> &exits = exitPair._value;
			uint16 numExitsInRoom = (uint16)exits.size();
			s.syncAsUint16LE(numExitsInRoom);
			for (uint16 i = 0; i < numExitsInRoom; ++i) {
				ExitChange change = exits[i];
				syncExitChange(s, change);
			}
		}
	} else {
		gameState->roomExitChanges.clear();
		for (uint16 idx = 0; idx < numExits; ++idx) {
			byte roomNumber;
			s.syncAsByte(roomNumber);
			uint16 numExitsInRoom;
			s.syncAsUint16LE(numExitsInRoom);
			Common::Array<ExitChange> exits;
			for (uint16 i = 0; i < numExitsInRoom; ++i) {
				ExitChange change;
				syncExitChange(s, change);
				exits.push_back(change);
			}
			gameState->roomExitChanges[roomNumber] = exits;
		}
	}

	// Room walkbox changes
	uint16 numWalkBoxes = (uint16)gameState->roomWalkBoxChanges.size();
	s.syncAsUint16LE(numWalkBoxes);
	if (s.isSaving()) {
		for (const auto &walkBoxPair : gameState->roomWalkBoxChanges) {
			byte roomNumber = walkBoxPair._key;
			s.syncAsByte(roomNumber);
			const Common::Array<WalkBoxChange> &walkBoxes = walkBoxPair._value;
			uint16 numWalkBoxesInRoom = (uint16)walkBoxes.size();
			s.syncAsUint16LE(numWalkBoxesInRoom);
			for (uint16 i = 0; i < numWalkBoxesInRoom; ++i) {
				WalkBoxChange change = walkBoxes[i];
				syncWalkBoxChange(s, change);
			}
		}
	} else {
		gameState->roomWalkBoxChanges.clear();
		for (uint16 idx = 0; idx < numWalkBoxes; ++idx) {
			byte roomNumber;
			s.syncAsByte(roomNumber);
			uint16 numWalkBoxesInRoom;
			s.syncAsUint16LE(numWalkBoxesInRoom);
			Common::Array<WalkBoxChange> walkBoxes;
			for (uint16 i = 0; i < numWalkBoxesInRoom; ++i) {
				WalkBoxChange change;
				syncWalkBoxChange(s, change);
				walkBoxes.push_back(change);
			}
			gameState->roomWalkBoxChanges[roomNumber] = walkBoxes;
		}
	}

	// Room hotspot changes
	uint16 hotSpotChangesSize = (uint16)gameState->roomHotSpotChanges.size();
	s.syncAsUint16LE(hotSpotChangesSize);
	if (s.isSaving()) {
		for (const auto &hotSpotPair : gameState->roomHotSpotChanges) {
			byte roomNumber = hotSpotPair._key;
			s.syncAsByte(roomNumber);
			const Common::Array<HotSpotChange> &hotSpots = hotSpotPair._value;
			uint16 numHotSpots = (uint16)hotSpots.size();
			s.syncAsUint16LE(numHotSpots);
			for (uint16 i = 0; i < numHotSpots; ++i) {
				HotSpotChange change = hotSpots[i];
				syncHotSpotChange(s, change);
			}
		}
	} else {
		gameState->roomHotSpotChanges.clear();
		for (uint16 idx = 0; idx < hotSpotChangesSize; ++idx) {
			byte roomNumber;
			s.syncAsByte(roomNumber);
			uint16 numHotSpots;
			s.syncAsUint16LE(numHotSpots);
			Common::Array<HotSpotChange> hotSpots;
			for (uint16 i = 0; i < numHotSpots; ++i) {
				HotSpotChange change;
				syncHotSpotChange(s, change);
				hotSpots.push_back(change);
			}
			gameState->roomHotSpotChanges[roomNumber] = hotSpots;
		}
	}

	uint16 disabledSpritesSize = (uint16)gameState->disabledSprites.size();
	s.syncAsUint16LE(disabledSpritesSize);
	if (s.isSaving()) {
		for (const auto &spritePair : gameState->disabledSprites) {
			byte roomNumber = spritePair._key;
			s.syncAsByte(roomNumber);
			const Common::Array<int> &sprites = spritePair._value;
			uint16 numSprites = (uint16)sprites.size();
			s.syncAsUint16LE(numSprites);
			for (uint16 i = 0; i < numSprites; ++i) {
				int spriteIndex = sprites[i];
				s.syncAsSint32LE(spriteIndex);
			}
		}
	} else {
		gameState->disabledSprites.clear();
		for (uint16 idx = 0; idx < disabledSpritesSize; ++idx) {
			byte roomNumber;
			s.syncAsByte(roomNumber);
			uint16 numSprites;
			s.syncAsUint16LE(numSprites);
			Common::Array<int> sprites;
			for (uint16 i = 0; i < numSprites; ++i) {
				int spriteIndex;
				s.syncAsSint32LE(spriteIndex);
				sprites.push_back(spriteIndex);
			}
			gameState->disabledSprites[roomNumber] = sprites;
		}
	}

	// Disabled branches
	uint16 disabledBranchesSize = (uint16)gameState->disabledBranches.size();
	s.syncAsUint16LE(disabledBranchesSize);
	if (s.isSaving()) {
		for (const auto &branchPair : gameState->disabledBranches) {
			byte roomNumber = branchPair._key;
			s.syncAsByte(roomNumber);
			const Common::Array<ResetEntry> &branches = branchPair._value;
			uint16 numBranches = (uint16)branches.size();
			s.syncAsUint16LE(numBranches);
			for (uint16 i = 0; i < numBranches; ++i) {
				ResetEntry entry = branches[i];
				syncResetEntry(s, entry);
			}
		}
	} else {
		gameState->disabledBranches.clear();
		for (uint16 idx = 0; idx < disabledBranchesSize; ++idx) {
			byte roomNumber;
			s.syncAsByte(roomNumber);
			uint16 numBranches;
			s.syncAsUint16LE(numBranches);
			Common::Array<ResetEntry> branches;
			for (uint16 i = 0; i < numBranches; ++i) {
				ResetEntry entry;
				syncResetEntry(s, entry);
				branches.push_back(entry);
			}
			gameState->disabledBranches[roomNumber] = branches;
		}
	}

	// Conversation roots state
	s.syncBytes(gameState->conversationRootsState, 4 * 56);
	return !s.err();
}

Common::Error syncSaveData(Common::Serializer &s, SaveGameData *gameState) {
	if (!syncGeneralData(s, gameState))
		return Common::Error(Common::kUnknownError, "Failed to sync general save game data.");

	if (!syncGameStateData(s, gameState->gameState))
		return Common::Error(Common::kUnknownError, "Failed to sync game state data.");

	return Common::kNoError;
}

Common::Error PelrockEngine::syncGame(Common::Serializer &s) {
	Common::Error result;

	if (s.isLoading()) {
		SaveGameData saveGame;
		if (saveGame.gameState != nullptr)
			delete saveGame.gameState;
		saveGame.gameState = new GameStateData();
		result = syncSaveData(s, &(saveGame));
		loadGame(saveGame);
	} else {
		SaveGameData *saveGame = createSaveGameData();
		result = syncSaveData(s, saveGame);
	}
	return result;
}

void PelrockEngine::loadGame(SaveGameData &saveGame) {
	_alfredState.x = saveGame.alfredX;
	_alfredState.y = saveGame.alfredY;
	_alfredState.direction = (AlfredDirection)saveGame.alfredDir;
	_state = *(saveGame.gameState);

	setScreen(saveGame.currentRoom, _alfredState.direction);
	_state.stateGame = GAME;
}

SaveGameData *PelrockEngine::createSaveGameData() const {
	SaveGameData *saveGame = new SaveGameData();
	saveGame->gameState = &g_engine->_state;
	saveGame->currentRoom = _room->_currentRoomNumber;
	saveGame->alfredX = _alfredState.x;
	saveGame->alfredY = _alfredState.y;
	saveGame->alfredDir = _alfredState.direction;
	return saveGame;
}

} // End of namespace Pelrock
