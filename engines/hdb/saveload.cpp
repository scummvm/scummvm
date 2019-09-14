/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/thumbnail.h"

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/gfx.h"
#include "hdb/lua-script.h"
#include "hdb/map.h"
#include "hdb/sound.h"
#include "hdb/window.h"

namespace HDB {

bool HDBGame::canSaveGameStateCurrently() {
	return (_gameState == GAME_PLAY && !_ai->cinematicsActive());
}

Common::Error HDBGame::saveGameState(int slot, const Common::String &desc) {

	// If no map is loaded, don't try to save
	if (!g_hdb->_map->isLoaded())
		return Common::kCreatingFileFailed;

	// If it is autosave, push down all saves
	if (slot == 0) {
		Common::String nameFrom;
		Common::String nameTo;
		for (int i = kNumSaveSlots - 2; i >= 0; i--) {
			nameFrom = genSaveFileName(i, false);
			nameTo = genSaveFileName(i + 1, false);
			_saveFileMan->renameSavefile(nameFrom, nameTo);

			nameFrom = genSaveFileName(i, true);
			nameTo = genSaveFileName(i + 1, true);
			_saveFileMan->renameSavefile(nameFrom, nameTo);
		}
	}

	Common::OutSaveFile *out;

	Common::String saveFileName = genSaveFileName(slot, false);
	if (!(out = _saveFileMan->openForSaving(saveFileName)))
		error("Unable to open save file");

	Graphics::saveThumbnail(*out);

	_saveHeader.fileSlot = 0;
	Common::strlcpy(_saveHeader.saveID, saveFileName.c_str(), sizeof(_saveHeader.saveID));
	_saveHeader.seconds = _timeSeconds + (_timePlayed / 1000);
	Common::strlcpy(_saveHeader.mapName, _inMapName, sizeof(_saveHeader.mapName));

	// Actual Save Data
	saveGame(out);
	_lua->save(out);

	out->finalize();
	if (out->err())
		warning("Can't write file '%s'. (Disk full?)", saveFileName.c_str());

	delete out;

	return Common::kNoError;
}

bool HDBGame::canLoadGameStateCurrently() {
	return _gameState == GAME_PLAY;
}

Common::Error HDBGame::loadGameState(int slot) {
	Common::InSaveFile *in;

	Common::String saveFileName = genSaveFileName(slot, false);
	if (!(in = _saveFileMan->openForLoading(saveFileName))) {
		warning("missing savegame file %s", saveFileName.c_str());
		if (g_hdb->_map->isLoaded())
			g_hdb->setGameState(GAME_PLAY);
		return Common::kReadingFailed;
	}

	_window->closeAll();

	Graphics::skipThumbnail(*in);

	// Actual Save Data
	loadGame(in);

	_lua->loadLua(_currentLuaName); // load the Lua code FIRST! (if no file, it's ok)

	_lua->loadSaveFile(in);

	delete in;

	// center the player on the screen
	int x, y;
	_ai->getPlayerXY(&x, &y);
	_map->centerMapXY(x + 16, y + 16);

	if (!_ai->cinematicsActive())
		_gfx->turnOffFade();

	debug(7, "Action List Info:");
	for (int k = 0; k < 20; k++) {
		debug(7, "Action %d: entityName: %s", k, _ai->_actions[k].entityName);
		debug(7, "Action %d: x1: %d, y1: %d", k, _ai->_actions[k].x1, _ai->_actions[k].y1);
		debug(7, "Action %d: x2: %d, y2: %d", k, _ai->_actions[k].x2, _ai->_actions[k].y2);
		debug(7, "Action %d: luaFuncInit: %s, luaFuncUse: %s", k, _ai->_actions[k].luaFuncInit, _ai->_actions[k].luaFuncUse);
	}

	return Common::kNoError;
}

void HDBGame::saveGame(Common::OutSaveFile *out) {
	debug(1, "HDBGame::saveGame: start at %u", out->pos());

	// Save Map Name and Time
	out->writeUint32LE(_saveHeader.seconds);
	out->write(_inMapName, 32);

	debug(1, "HDBGame::saveGame: map at %u", out->pos());
	// Save Map Object Data
	_map->save(out);

	// Save Window Object Data
	debug(1, "HDBGame::saveGame: window at %u", out->pos());
	_window->save(out);

	// Save Gfx Object Data
	debug(1, "HDBGame::saveGame: gfx at %u", out->pos());
	_gfx->save(out);

	// Save Sound Object Data
	debug(1, "HDBGame::saveGame: sound at %u", out->pos());
	_sound->save(out);

	// Save Game Object Data
	debug(1, "HDBGame::saveGame: game object at %u", out->pos());
	save(out);

	// Save AI Object Data
	debug(1, "HDBGame::saveGame: ai at %u", out->pos());
	_ai->save(out);

	debug(1, "HDBGame::saveGame: end at %u", out->pos());
}

void HDBGame::loadGame(Common::InSaveFile *in) {
	debug(1, "HDBGame::loadGame: start at %u", in->pos());

	// Load Map Name and Time
	_timeSeconds = in->readUint32LE();
	_timePlayed = 0;
	in->read(_inMapName, 32);

	g_hdb->_sound->stopMusic();
	_saveHeader.seconds = _timeSeconds;
	Common::strlcpy(_saveHeader.mapName, _inMapName, sizeof(_saveHeader.mapName));

	// Load Map Object Data
	debug(1, "HDBGame::loadGame: map at %u", in->pos());
	_map->loadSaveFile(in);

	// Load Window Object Data
	debug(1, "HDBGame::loadGame: window at %u", in->pos());
	_window->loadSaveFile(in);

	// Load Gfx Object Data
	debug(1, "HDBGame::loadGame: gfx at %u", in->pos());
	_gfx->loadSaveFile(in);

	// Load Sound Object Data
	debug(1, "HDBGame::loadGame: sound at %u", in->pos());
	_sound->loadSaveFile(in);

	// Load Game Object Data
	debug(1, "HDBGame::loadGame: game object at %u", in->pos());
	loadSaveFile(in);

	// Load AI Object Data
	debug(1, "HDBGame::loadGame: ai at %u", in->pos());
	_ai->loadSaveFile(in);

	debug(1, "HDBGame::loadGame: end at %u", in->pos());

	_gfx->turnOffFade();
}

void HDBGame::save(Common::OutSaveFile *out) {
	out->write(_currentMapname, 64);
	out->write(_lastMapname, 64);
	out->write(_currentLuaName, 64);
	out->writeSint32LE(_actionMode);
	out->writeByte(_changeLevel);
	out->write(_changeMapname, 64);
	out->write(_inMapName, 32);
}

void HDBGame::loadSaveFile(Common::InSaveFile *in) {
	in->read(_currentMapname, 64);

	debug(0, "Loading map %s", _currentMapname);

	in->read(_lastMapname, 64);
	in->read(_currentLuaName, 64);
	_actionMode = in->readSint32LE();
	_changeLevel = in->readByte();
	in->read(_changeMapname, 64);
	in->read(_inMapName, 32);
}

void AIEntity::save(Common::OutSaveFile *out) {
	char funcString[32];
	const char *lookUp;


	// Write out 32-char names for the function ptrs we have in the entity struct
	lookUp = g_hdb->_ai->funcLookUp(aiAction);
	memset(&funcString, 0, 32);
	if (!lookUp && aiAction)
		error("AIEntity::save: No matching ACTION function for func-string for %s entity", AIType2Str(type));
	if (lookUp)
		strncpy(funcString, lookUp, 31);
	out->write(funcString, 32);

	lookUp = g_hdb->_ai->funcLookUp(aiUse);
	memset(&funcString, 0, 32);
	if (!lookUp && aiUse)
		error("AIEntity::save: No matching USE function for func-string for %s entity", AIType2Str(type));
	if (lookUp)
		strncpy(funcString, lookUp, 31);
	out->write(funcString, 32);

	lookUp = g_hdb->_ai->funcLookUp(aiInit);
	memset(&funcString, 0, 32);
	if (!lookUp && aiInit)
		error("AIEntity::save: No matching INIT function for func-string for %s entity", AIType2Str(type));
	if (lookUp)
		strncpy(funcString, lookUp, 31);
	out->write(funcString, 32);

	lookUp = g_hdb->_ai->funcLookUp(aiInit2);
	memset(&funcString, 0, 32);
	if (!lookUp && aiInit2)
		error("AIEntity::save: No matching INIT2 function for func-string for %s entity", AIType2Str(type));
	if (lookUp)
		strncpy(funcString, lookUp, 31);
	out->write(funcString, 32);

	lookUp = g_hdb->_ai->funcLookUp((FuncPtr)aiDraw);
	memset(&funcString, 0, 32);
	if (!lookUp && aiDraw)
		error("AIEntity::save: No matching DRAW function for func-string for %s entity", AIType2Str(type));
	if (lookUp)
		strncpy(funcString, lookUp, 31);
	out->write(funcString, 32);

	// Save AIEntity
	out->writeSint32LE((int)type);
	out->writeSint32LE((int)state);
	out->writeSint32LE((int)dir);
	out->write(luaFuncInit, 32);
	out->write(luaFuncAction, 32);
	out->write(luaFuncUse, 32);
	out->writeUint16LE(level);
	out->writeUint16LE(value1);
	out->writeUint16LE(value2);
	out->writeSint32LE((int)dir2);
	out->writeUint16LE(x);
	out->writeUint16LE(y);
	out->writeSint16LE(drawXOff);
	out->writeSint16LE(drawYOff);
	out->writeUint16LE(onScreen);
	out->writeUint16LE(moveSpeed);
	out->writeSint16LE(xVel);
	out->writeSint16LE(yVel);
	out->writeUint16LE(tileX);
	out->writeUint16LE(tileY);
	out->writeUint16LE(goalX);
	out->writeUint16LE(goalY);
	out->writeUint16LE(touchpX);
	out->writeUint16LE(touchpY);
	out->writeUint16LE(touchpTile);
	out->writeUint16LE(touchpWait);
	out->writeUint16LE(stunnedWait);
	out->writeSint16LE(sequence);
	out->write(entityName, 32);
	out->write(printedName, 32);
	out->writeUint16LE(animFrame);
	out->writeUint16LE(animDelay);
	out->writeUint16LE(animCycle);
}

void AIEntity::load(Common::InSaveFile *in) {
	char funcString[32];
	FuncPtr init, init2, use, action;
	EntFuncPtr drawf;

	action = init = init2 = use = NULL;
	drawf = NULL;

	// Read 32-char names for the function ptrs we have in entity struct
	in->read(funcString, 32);
	if (funcString[0])
		action = g_hdb->_ai->funcLookUp(funcString);

	in->read(funcString, 32);
	if (funcString[0])
		use = g_hdb->_ai->funcLookUp(funcString);

	in->read(funcString, 32);
	if (funcString[0])
		init = g_hdb->_ai->funcLookUp(funcString);

	in->read(funcString, 32);
	if (funcString[0])
		init2 = g_hdb->_ai->funcLookUp(funcString);

	in->read(funcString, 32);
	if (funcString[0])
		drawf = (EntFuncPtr)g_hdb->_ai->funcLookUp(funcString);

	// Load AIEntity
	type = (AIType)in->readSint32LE();
	state = (AIState)in->readSint32LE();
	dir = (AIDir)in->readSint32LE();
	in->read(luaFuncInit, 32);
	in->read(luaFuncAction, 32);
	in->read(luaFuncUse, 32);
	level = in->readUint16LE();
	value1 = in->readUint16LE();
	value2 = in->readUint16LE();
	dir2 = (AIDir)in->readSint32LE();
	x = in->readUint16LE();
	y = in->readUint16LE();
	drawXOff = in->readSint16LE();
	drawYOff = in->readSint16LE();
	onScreen = in->readUint16LE();
	moveSpeed = in->readUint16LE();
	xVel = in->readSint16LE();
	yVel = in->readSint16LE();
	tileX = in->readUint16LE();
	tileY = in->readUint16LE();
	goalX = in->readUint16LE();
	goalY = in->readUint16LE();
	touchpX = in->readUint16LE();
	touchpY = in->readUint16LE();
	touchpTile = in->readUint16LE();
	touchpWait = in->readUint16LE();
	stunnedWait = in->readUint16LE();
	sequence = in->readSint16LE();
	in->read(entityName, 32);
	in->read(printedName, 32);
	animFrame = in->readUint16LE();
	animDelay = in->readUint16LE();
	animCycle = in->readUint16LE();

	aiAction = action;
	aiInit = init;
	aiInit2 = init2;
	aiUse = use;
	aiDraw = drawf;
}

Common::String HDBGame::genSaveFileName(uint slot, bool lua) {
	if (!lua)
		return Common::String::format("%s.%03d", _targetName.c_str(), slot);

	return Common::String::format("%s.l.%03d", _targetName.c_str(), slot);
}


} // End of Namespace
