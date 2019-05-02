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

#include "mutationofjb/commands/command.h"
#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/mutationofjb.h"
#include "mutationofjb/room.h"
#include "mutationofjb/script.h"
#include "mutationofjb/tasks/objectanimationtask.h"
#include "mutationofjb/util.h"

#include "common/str.h"
#include "common/util.h"

#include "engines/advancedDetector.h"

namespace MutationOfJB {

Game::Game(MutationOfJBEngine *vm)
	: _vm(vm),
	  _randomSource("mutationofjb"),
	  _delayedLocalScript(nullptr),
	  _runDelayedScriptStartup(false),
	  _gui(*this, _vm->getScreen()),
	  _scriptExecCtx(*this),
	  _taskManager(*this),
	  _assets(*this) {

	_gameData = new GameData;
	loadGameData(false);

	EncryptedFile globalScriptFile;
	globalScriptFile.open("global.atn");
	_globalScript = new Script;
	_globalScript->loadFromStream(globalScriptFile);
	globalScriptFile.close();

	_localScript = nullptr;
	_room = new Room(this, _vm->getScreen());

	_gui.init();

	_taskManager.startTask(TaskPtr(new ObjectAnimationTask));
}

MutationOfJBEngine &Game::getEngine() {
	return *_vm;
}

Common::RandomSource &Game::getRandomSource() {
	return _randomSource;
}

GameData &Game::getGameData() {
	return *_gameData;
}

Room &Game::getRoom() {
	return *_room;
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

	_gameData->loadInitialState(file);

	file.close();

	return true;
}

Script *Game::changeSceneLoadScript(uint8 sceneId, bool partB) {
	if (isCurrentSceneMap()) {
		_gui.markDirty();
	}

	_gameData->_lastScene = _gameData->_currentScene;
	_gameData->_currentScene = sceneId;
	_gameData->_partB = partB;

	_room->load(_gameData->_currentScene, partB);
	_gui.refreshAfterSceneChanged();

	EncryptedFile scriptFile;
	Common::String fileName = Common::String::format("scrn%d%s.atn", sceneId, partB ? "b" : "");
	scriptFile.open(fileName);
	if (!scriptFile.isOpen()) {
		reportFileMissingError(fileName.c_str());
		return nullptr;
	}

	// TODO Actually parse this.
	Common::String dummy;
	dummy = scriptFile.readLine(); // Skip first line.
	scriptFile.seek(126, SEEK_CUR); // Skip 126 bytes.

	Script *localScript = new Script;
	localScript->loadFromStream(scriptFile);
	scriptFile.close();

	return localScript;
}

void Game::changeScene(uint8 sceneId, bool partB) {
	if (_localScript) {
		delete _localScript;
		_localScript = nullptr;
	}

	_localScript = changeSceneLoadScript(sceneId, partB);
	if (_localScript) {
		_scriptExecCtx.startStartupSection();
	}
}

Script *Game::changeSceneDelayScript(uint8 sceneId, bool partB, bool runDelayedScriptStartup) {
	_delayedLocalScript = changeSceneLoadScript(sceneId, partB);
	_runDelayedScriptStartup = runDelayedScriptStartup;
	return _delayedLocalScript;
}

static Command *findActionInfoCommand(const ActionInfos &infos, const Common::String &entity1Name, const Common::String &entity2Name = Common::String()) {
	for (ActionInfos::const_iterator it = infos.begin(); it != infos.end(); ++it) {
		if (it->_entity1Name == entity1Name && it->_entity2Name == entity2Name) {
			return it->_command;
		}
	}
	return nullptr;
}

bool Game::startActionSection(ActionInfo::Action action, const Common::String &entity1Name, const Common::String &entity2Name) {
	Script *const localScript = getLocalScript();
	Script *const globalScript = getGlobalScript();

	Command *command = nullptr;
	if (localScript) {
		command = findActionInfoCommand(localScript->getActionInfos(action), entity1Name, entity2Name);
	}
	if (!command && globalScript) {
		command = findActionInfoCommand(globalScript->getActionInfos(action), entity1Name, entity2Name);
	}
	if (command) {
		_scriptExecCtx.startCommand(command);
		return true;
	}

	return false;
}

bool Game::isCurrentSceneMap() const {
	return _gameData->_currentScene == 12;
}

void Game::update() {
	Command::ExecuteResult res = _scriptExecCtx.runActiveCommand();
	if (res == Command::Finished && _delayedLocalScript) {
		delete _localScript;
		_localScript = _delayedLocalScript;

		if (_localScript && _runDelayedScriptStartup)
			_scriptExecCtx.startStartupSection();

		_delayedLocalScript = nullptr;
		_runDelayedScriptStartup = false;
	}

	_taskManager.update();
}

GameScreen &Game::getGameScreen() {
	return _gui;
}

uint8 Game::colorFromString(const char *colorStr) {
	struct {
		const char *str;
		uint8 color;
	} colors[] = {
		{"white", WHITE},
		{"darkgray", DARKGRAY},
		{"lightgray", LIGHTGRAY},
		{"green", GREEN},
		{"orange", ORANGE},
		{"darkblue", DARKBLUE},
		{"lightblue", LIGHTBLUE},
		{"brown", BROWN}
	};

	for (int i = 0; i < ARRAYSIZE(colors); ++i) {
		if (strcmp(colors[i].str, colorStr) == 0) {
			return colors[i].color;
		}
	}

	if (*colorStr == 'n') {
		return static_cast<uint8>(atoi(colorStr + 1));
	}

	warning("Color not found");
	return 0x00;
}

TaskManager &Game::getTaskManager() {
	return _taskManager;
}

Assets &Game::getAssets() {
	return _assets;
}

Graphics::Screen &Game::getScreen() {
	return *_vm->getScreen();
}

TaskPtr Game::getActiveSayTask() const {
	return _activeSayTask;
}

void Game::setActiveSayTask(const TaskPtr &sayTask) {
	_activeSayTask = sayTask;
}

bool Game::loadSaveAllowed() const {
	if (_scriptExecCtx.isCommandRunning())
		return false;

	if (isCurrentSceneMap())
		return false;

	return true;
}

Common::Language Game::getLanguage() const {
	return _vm->getGameDescription()->language;
}

void Game::switchToPartB() {
	getGameData().getInventory().removeAllItems();
	loadGameData(true);
	changeSceneDelayScript(3, true, true);
}

}
