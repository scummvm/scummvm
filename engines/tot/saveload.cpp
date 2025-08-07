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
#include "tot/playanim.h"
#include "tot/routines.h"
#include "tot/tot.h"

namespace Tot {

#define SAVEGAME_CURRENT_VERSION 1

bool syncGeneralData(Common::Serializer &s, SavedGame &game) {
	uint32 startBytes = s.bytesSynced();
	// Uint16
	s.syncAsUint16LE(game.numeropantalla);
	s.syncAsUint16LE(game.longtray);
	s.syncAsUint16LE(game.indicetray);
	s.syncAsUint16LE(game.codigoobjmochila);
	s.syncAsUint16LE(game.volumenfxderecho);
	s.syncAsUint16LE(game.volumenfxizquierdo);
	s.syncAsUint16LE(game.volumenmelodiaderecho);
	s.syncAsUint16LE(game.volumenmelodiaizquierdo);
	s.syncAsUint16LE(game.oldxrejilla);
	s.syncAsUint16LE(game.oldyrejilla);
	s.syncAsUint16LE(game.animadoprofundidad);
	s.syncAsUint16LE(game.animadodir);
	s.syncAsUint16LE(game.animadoposx);
	s.syncAsUint16LE(game.animadoposy);
	s.syncAsUint16LE(game.animadoiframe2);
	// Bytes
	s.syncAsByte(game.zonaactual);
	s.syncAsByte(game.zonadestino);
	s.syncAsByte(game.oldzonadestino);
	s.syncAsByte(game.posicioninv);
	s.syncAsByte(game.numeroaccion);
	s.syncAsByte(game.oldnumeroacc);
	s.syncAsByte(game.pasos);
	s.syncAsByte(game.indicepuertas);
	s.syncAsByte(game.direccionmovimiento);
	s.syncAsByte(game.iframe);
	s.syncAsByte(game.parte_del_juego);

	// Booleans
	s.syncAsByte(game.sello_quitado);
	s.syncAsByte(game.lista1);
	s.syncAsByte(game.lista2);
	s.syncAsByte(game.completadalista1);
	s.syncAsByte(game.completadalista2);
	s.syncAsByte(game.vasijapuesta);
	s.syncAsByte(game.guadagna);
	s.syncAsByte(game.tridente);
	s.syncAsByte(game.torno);
	s.syncAsByte(game.barro);
	s.syncAsByte(game.diablillo_verde);
	s.syncAsByte(game.rojo_capturado);
	s.syncAsByte(game.manual_torno);
	s.syncAsByte(game.alacena_abierta);
	s.syncAsByte(game.baul_abierto);
	s.syncAsByte(game.teleencendida);
	s.syncAsByte(game.trampa_puesta);

	for (int i = 0; i < inventoryIconCount; i++) {
		s.syncAsUint16LE(game.mobj[i].bitmapIndex);
		s.syncAsUint16LE(game.mobj[i].code);
		s.syncString(game.mobj[i].objectName);
	}

	// integers
	s.syncAsSint32LE(game.elemento1);
	s.syncAsSint32LE(game.elemento2);
	s.syncAsSint32LE(game.xframe);
	s.syncAsSint32LE(game.yframe);
	s.syncAsSint32LE(game.xframe2);
	s.syncAsSint32LE(game.yframe2);

	// Strings
	s.syncString(game.oldobjmochila);
	s.syncString(game.objetomochila);
	s.syncString(game.nombrepersonaje);

	for (int i = 0; i < routePointCount; i++) {
		s.syncAsSint16LE(game.mainRoute[i].x);
		s.syncAsSint16LE(game.mainRoute[i].y);
	}

	for (int i = 0; i < 300; i++) {
		s.syncAsSint16LE(game.trayec[i].x);
		s.syncAsSint16LE(game.trayec[i].y);
	}

	for (int indiaux = 0; indiaux < characterCount; indiaux++) {
		// interleave them just to avoid creating many loops
		s.syncAsByte(game.primera[indiaux]);
		s.syncAsByte(game.lprimera[indiaux]);
		s.syncAsByte(game.cprimera[indiaux]);
		s.syncAsByte(game.libro[indiaux]);
		s.syncAsByte(game.caramelos[indiaux]);
	}

	for (int indiaux = 0; indiaux < 5; indiaux++) {
		s.syncAsByte(game.cavernas[indiaux]);
		s.syncAsUint16LE(game.firstList[indiaux]);
		s.syncAsUint16LE(game.secondList[indiaux]);
	}

	for (int indiaux = 0; indiaux < 4; indiaux++) {
		s.syncAsUint16LE(game.hornacina[0][indiaux]);
		s.syncAsUint16LE(game.hornacina[1][indiaux]);
	}

	uint32 newBytes = s.bytesSynced();
	debug("return generalData totalBytes synced %d", s.bytesSynced());
	return true;
}

bool syncRoomData(Common::Serializer &s, Common::MemorySeekableReadWriteStream *roomStream) {
	uint32 startBytes = s.bytesSynced();
	if (s.isSaving()) {

		// Restore trajectory
		setRoomTrajectories(altoanimado, anchoanimado, RESTORE);
		// Make sure to save any unsaved changes in the room
		saveRoomData(currentRoomData, rooms);

		// Do not fix screen grids, they will be fixed differently below
		setRoomTrajectories(altoanimado, anchoanimado, SET_WITH_ANIM);

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
		int size = rooms->size();
		delete (rooms);
		byte *roomBuf = (byte *)malloc(size);
		s.syncBytes(roomBuf, size);

		debug("Loading room data now");
		// TODO: Will roomBuf be automatically freed?
		rooms = new Common::MemorySeekableReadWriteStream(roomBuf, size, DisposeAfterUse::NO);
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
		delete (conversationData);
		byte *convBuf = (byte *)malloc(size);
		s.syncBytes(convBuf, size);
		debug("Loading conversation data now");
		// TODO: Will objBuf be automatically freed?
		conversationData = new Common::MemorySeekableReadWriteStream(convBuf, size, DisposeAfterUse::NO);
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
		delete (invItemData);
		byte *objBuf = (byte *)malloc(size);
		s.syncBytes(objBuf, size);
		debug("Loading item data now");
		// TODO: Will objBuf be automatically freed?
		invItemData = new Common::MemorySeekableReadWriteStream(objBuf, size, DisposeAfterUse::NO);
	}
	return true;
}

Common::Error syncSaveData(Common::Serializer &ser, SavedGame &game) {
	if (!syncGeneralData(ser, game)) {
		warning("Error while synchronizing general data");
		return Common::kUnknownError;
	}
	if (!syncRoomData(ser, rooms)) {
		warning("Error while synchronizing room data");
		return Common::kUnknownError;
	}
	if (!syncItemData(ser, invItemData)) {
		warning("Error while syncrhonizing object data");
		return Common::kUnknownError;
	}
	if (!syncConversationData(ser, conversationData)) {
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
		processingActive();

		loadCharAnimation();
		loadObjects();

		loadPalette("DEFAULT");
		loadScreenMemory();

		totalFadeOut(0);
		g_engine->_graphics->clear();
		processingActive();
		initializeScreenFile();
		initializeObjectFile();
		readConversationFile(Common::String("CONVERSA.TRE"));
		// }
		result = syncSaveData(s, loadedGame);
		loadGame(loadedGame);
	} else {
		saveGameToRegister();
		result = syncSaveData(s, regpartida);
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
	return inGame && saveAllowed;
}

} // End of namespace Tot
