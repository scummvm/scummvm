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
#include "gui/saveload.h"
#include "common/error.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/system.h"
#include "gui/message.h"

#include "tot/forest.h"
#include "tot/tot.h"
#include "tot/vars.h"

namespace Tot {

#define SAVEGAME_CURRENT_VERSION 1

bool syncGeneralData(Common::Serializer &s, SavedGame &game) {
	uint32 startBytes = s.bytesSynced();
	// Uint16
	s.syncAsUint16LE(game.roomCode);
	s.syncAsUint16LE(game.trajectoryLength);
	s.syncAsUint16LE(game.currentTrajectoryIndex);
	s.syncAsUint16LE(game.backpackObjectCode);
	s.syncAsUint16LE(game.rightSfxVol);
	s.syncAsUint16LE(game.leftSfxVol);
	s.syncAsUint16LE(game.musicVolRight);
	s.syncAsUint16LE(game.musicVolLeft);
	s.syncAsUint16LE(game.oldGridX);
	s.syncAsUint16LE(game.oldGridY);
	s.syncAsUint16LE(game.secAnimDepth);
	s.syncAsUint16LE(game.secAnimDir);
	s.syncAsUint16LE(game.secAnimX);
	s.syncAsUint16LE(game.secAnimY);
	s.syncAsUint16LE(game.secAnimIFrame);
	// Bytes
	s.syncAsByte(game.currentZone);
	s.syncAsByte(game.targetZone);
	s.syncAsByte(game.oldTargetZone);
	s.syncAsByte(game.inventoryPosition);
	s.syncAsByte(game.actionCode);
	s.syncAsByte(game.oldActionCode);
	s.syncAsByte(game.steps);
	s.syncAsByte(game.doorIndex);
	s.syncAsByte(game.characterFacingDir);
	s.syncAsByte(game.iframe);
	s.syncAsByte(game.gamePart);

	// Booleans
	s.syncAsByte(game.isSealRemoved);
	s.syncAsByte(game.obtainedList1);
	s.syncAsByte(game.obtainedList2);
	s.syncAsByte(game.list1Complete);
	s.syncAsByte(game.list2Complete);
	s.syncAsByte(game.isVasePlaced);
	s.syncAsByte(game.isScytheTaken);
	s.syncAsByte(game.isTridentTaken);
	s.syncAsByte(game.isPottersWheelDelivered);
	s.syncAsByte(game.isMudDelivered);
	s.syncAsByte(game.isGreenDevilDelivered);
	s.syncAsByte(game.isRedDevilCaptured);
	s.syncAsByte(game.isPottersManualDelivered);
	s.syncAsByte(game.isCupboardOpen);
	s.syncAsByte(game.isChestOpen);
	s.syncAsByte(game.isTVOn);
	s.syncAsByte(game.isTrapSet);

	for (int i = 0; i < kInventoryIconCount; i++) {
		s.syncAsUint16LE(game.mobj[i].bitmapIndex);
		s.syncAsUint16LE(game.mobj[i].code);
		s.syncString(game.mobj[i].objectName);
	}

	// integers
	s.syncAsSint32LE(game.element1);
	s.syncAsSint32LE(game.element2);
	s.syncAsSint32LE(game.characterPosX);
	s.syncAsSint32LE(game.characterPosY);
	s.syncAsSint32LE(game.xframe2);
	s.syncAsSint32LE(game.yframe2);

	// Strings
	s.syncString(game.oldInventoryObjectName);
	s.syncString(game.objetomoinventoryObjectNamehila);
	s.syncString(game.characterName);

	for (int i = 0; i < kRoutePointCount; i++) {
		s.syncAsSint16LE(game.mainRoute[i].x);
		s.syncAsSint16LE(game.mainRoute[i].y);
	}

	for (int i = 0; i < 300; i++) {
		s.syncAsSint16LE(game.trajectory[i].x);
		s.syncAsSint16LE(game.trajectory[i].y);
	}

	for (int indiaux = 0; indiaux < kCharacterCount; indiaux++) {
		// interleave them just to avoid creating many loops
		s.syncAsByte(game.firstTimeTopicA[indiaux]);
		s.syncAsByte(game.firstTimeTopicB[indiaux]);
		s.syncAsByte(game.firstTimeTopicC[indiaux]);
		s.syncAsByte(game.bookTopic[indiaux]);
		s.syncAsByte(game.mintTopic[indiaux]);
	}

	for (int indiaux = 0; indiaux < 5; indiaux++) {
		s.syncAsByte(game.caves[indiaux]);
		s.syncAsUint16LE(game.firstList[indiaux]);
		s.syncAsUint16LE(game.secondList[indiaux]);
	}

	for (int indiaux = 0; indiaux < 4; indiaux++) {
		s.syncAsUint16LE(game.niche[0][indiaux]);
		s.syncAsUint16LE(game.niche[1][indiaux]);
	}

	uint32 newBytes = s.bytesSynced();
	debug("return generalData totalBytes synced %d", s.bytesSynced());
	return true;
}

bool syncRoomData(Common::Serializer &s, Common::MemorySeekableReadWriteStream *roomStream) {
	uint32 startBytes = s.bytesSynced();
	if (s.isSaving()) {

		// Restore trajectory
		g_engine->setRoomTrajectories(g_engine->_secondaryAnimHeight, g_engine->_secondaryAnimWidth, RESTORE);
		// Make sure to save any unsaved changes in the room
		g_engine->saveRoomData(g_engine->_currentRoomData, g_engine->_rooms);

		// Do not fix screen grids, they will be fixed differently below
		g_engine->setRoomTrajectories(g_engine->_secondaryAnimHeight, g_engine->_secondaryAnimWidth, SET_WITH_ANIM);

		int size = roomStream->size();
		byte *roomBuf = (byte *)malloc(size);
		roomStream->seek(0, 0);
		roomStream->read(roomBuf, size);
		s.syncBytes(roomBuf, size);
		free(roomBuf);
		debug("return room totalBytes synced %d", s.bytesSynced());
	}
	uint32 newBytes = s.bytesSynced();
	if (s.isLoading()) {
		int size = g_engine->_rooms->size();
		delete (g_engine->_rooms);
		byte *roomBuf = (byte *)malloc(size);
		s.syncBytes(roomBuf, size);

		debug("Loading room data now");
		// TODO: Will roomBuf be automatically freed?
		g_engine->_rooms = new Common::MemorySeekableReadWriteStream(roomBuf, size, DisposeAfterUse::NO);
	}
	return true;
}

bool syncConversationData(Common::Serializer &s, Common::MemorySeekableReadWriteStream *conversations) {
	uint32 startBytes = s.bytesSynced();

	int size = conversations->size();
	if (s.isSaving()) {

		byte *convBuf = (byte *)malloc(size);
		conversations->seek(0, 0);
		conversations->read(convBuf, size);
		s.syncBytes(convBuf, size);
		free(convBuf);
		debug("return conversation totalBytes synced %d", s.bytesSynced());
	}
	if (s.isLoading()) {
		delete (g_engine->_conversationData);
		byte *convBuf = (byte *)malloc(size);
		s.syncBytes(convBuf, size);
		debug("Loading conversation data now");
		// TODO: Will objBuf be automatically freed?
		g_engine->_conversationData = new Common::MemorySeekableReadWriteStream(convBuf, size, DisposeAfterUse::NO);
	}
	return true;
}

bool syncItemData(Common::Serializer &s, Common::MemorySeekableReadWriteStream *items) {
	uint32 startBytes = s.bytesSynced();
	int size = items->size();
	if (s.isSaving()) {
		byte *objBuf = (byte *)malloc(size);
		items->seek(0, 0);
		items->read(objBuf, size);
		s.syncBytes(objBuf, size);
		free(objBuf);
		debug("return items totalBytes synced %d", s.bytesSynced());
	}
	uint32 newBytes = s.bytesSynced();
	if (s.isLoading()) {
		delete (g_engine->_invItemData);
		byte *objBuf = (byte *)malloc(size);
		s.syncBytes(objBuf, size);
		debug("Loading item data now");
		// TODO: Will objBuf be automatically freed?
		g_engine->_invItemData = new Common::MemorySeekableReadWriteStream(objBuf, size, DisposeAfterUse::NO);
	}
	return true;
}

Common::Error syncSaveData(Common::Serializer &ser, SavedGame &game) {
	if (!syncGeneralData(ser, game)) {
		warning("Error while synchronizing general data");
		return Common::kUnknownError;
	}
	if (!syncRoomData(ser, g_engine->_rooms)) {
		warning("Error while synchronizing room data");
		return Common::kUnknownError;
	}
	if (!syncItemData(ser, g_engine->_invItemData)) {
		warning("Error while syncrhonizing object data");
		return Common::kUnknownError;
	}
	if (!syncConversationData(ser, g_engine->_conversationData)) {
		warning("Error while syncrhonizing conversation data");
		return Common::kUnknownError;
	}
	return Common::kNoError;
}

Common::Error TotEngine::syncGame(Common::Serializer &s) {
	Common::Error result;

	if (s.isLoading()) {
		debug("Loading game!!");
		SavedGame loadedGame;
		// Means we are loading from before the game has started
		// if(rooms == nullptr) {
		g_engine->_graphics->clear();
		displayLoading();

		loadCharAnimation();
		loadObjects();

		g_engine->_graphics->loadPaletteFromFile("DEFAULT");
		loadScreenMemory();

		g_engine->_graphics->totalFadeOut(0);
		g_engine->_graphics->clear();
		displayLoading();
		initializeScreenFile();
		initializeObjectFile();
		readConversationFile(Common::String("CONVERSA.TRE"));
		// }
		result = syncSaveData(s, loadedGame);
		loadGame(loadedGame);
	} else {
		saveGameToRegister();
		result = syncSaveData(s, savedGame);
	}
	return result;
}

Common::Error TotEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	const byte version = SAVEGAME_CURRENT_VERSION;
	Common::Serializer s(nullptr, stream);
	s.setVersion(version);
	stream->writeByte(version);

	return syncGame(s);
}
Common::Error TotEngine::loadGameStream(Common::SeekableReadStream *stream) {
	byte version = stream->readByte();
	if (version > SAVEGAME_CURRENT_VERSION) {
		GUI::MessageDialog dialog(Common::String("Saved game was created with a newer version of ScummVM. Unable to load."));
		dialog.runModal();
		return Common::kUnknownError;
	}

	Common::Serializer s(stream, nullptr);
	s.setVersion(version);

	return syncGame(s);
}

bool TotEngine::canLoadGameStateCurrently(Common::U32String *msg) {
	return true;
}
bool TotEngine::canSaveGameStateCurrently(Common::U32String *msg) {
	return _inGame && saveAllowed;
}

} // End of namespace Tot
