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

#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/room.h"
#include "mutationofjb/script.h"
#include "mutationofjb/util.h"
#include "mutationofjb/commands/command.h"
#include "common/util.h"
#include "common/str.h"
#include "common/translation.h"

namespace MutationOfJB {

Game::Game(MutationOfJBEngine *vm)
: _vm(vm), _scriptExecCtx(*this) {
	_gameData = new GameData;
	loadGameData(false);

	EncryptedFile globalScriptFile;
	globalScriptFile.open("global.atn");
	_globalScript = new Script;
	_globalScript->loadFromStream(globalScriptFile);
	globalScriptFile.close();

	_localScript = nullptr;
	_room = new Room(_vm->getScreen());

	changeScene(13, false); // Initial scene.
}

GameData &Game::getGameData() {
	return *_gameData;
}

Script *Game::getGlobalScript() const {
	return _globalScript;
}

Script *Game::getLocalScript() const {
	return _localScript;
}

bool Game::loadGameData(bool partB) {
	EncryptedFile file;
	const char *fileName = !partB ? "startup.dat" : "startupb.dat";
	file.open(fileName);
	if (!file.isOpen()) {
		reportFileMissingError(fileName);
		return false;
	}

	_gameData->loadFromStream(file);

	file.close();

	return true;
}


void Game::changeScene(uint8 sceneId, bool partB) {
	_gameData->_lastScene = _gameData->_currentScene;
	_gameData->_currentScene = sceneId;
	_room->load(_gameData->_currentScene, partB);

	if (_localScript) {
		delete _localScript;
		_localScript = nullptr;
	}

	EncryptedFile scriptFile;
	Common::String fileName = Common::String::format("scrn%d%s.atn", sceneId, partB ? "b" : "");
	scriptFile.open(fileName);
	if (!scriptFile.isOpen()) {
		reportFileMissingError(fileName.c_str());
		return;
	}

	// TODO Actually parse this.
	Common::String dummy;
	dummy = scriptFile.readLine(); // Skip first line.
	scriptFile.seek(126, SEEK_CUR); // Skip 126 bytes.

	_localScript = new Script;
	_localScript->loadFromStream(scriptFile);
	scriptFile.close();
}


void Game::update() {
	_scriptExecCtx.runActiveCommand();
}

}
