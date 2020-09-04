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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "xeen/xeen.h"
#include "xeen/files.h"
#include "xeen/resources.h"

namespace Xeen {

XeenEngine *g_vm = nullptr;

XeenEngine::XeenEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: Engine(syst), _gameDescription(gameDesc), _randomSource("Xeen") {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");
	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound processing");

	_combat = nullptr;
	_debugger = nullptr;
	_events = nullptr;
	_files = nullptr;
	_interface = nullptr;
	_locations = nullptr;
	_map = nullptr;
	_party = nullptr;
	_patcher = nullptr;
	_resources = nullptr;
	_saves = nullptr;
	_screen = nullptr;
	_scripts = nullptr;
	_sound = nullptr;
	_spells = nullptr;
	_windows = nullptr;
	_noDirectionSense = false;
	_startupWindowActive = false;
	_gameMode = GMODE_STARTUP;
	_mode = MODE_STARTUP;
	_endingScore = 0;
	_loadSaveSlot = -1;
	_gameWon[0] = _gameWon[1] = _gameWon[2] = false;
	_finalScore = 0;
	g_vm = this;
}

XeenEngine::~XeenEngine() {
	delete _combat;
	//_debugger is deleted by Engine
	delete _events;
	delete _interface;
	delete _locations;
	delete _map;
	delete _party;
	delete _patcher;
	delete _saves;
	delete _screen;
	delete _scripts;
	delete _sound;
	delete _spells;
	delete _windows;
	delete _resources;
	delete _files;
	g_vm = nullptr;
}

bool XeenEngine::initialize() {
	// Create sub-objects of the engine
	_files = new FileManager(this);
	if (!_files->setup())
		return false;

	_resources = new Resources();
	_combat = new Combat(this);
	_debugger = new Debugger(this);
	setDebugger(_debugger);
	_events = new EventsManager(this);
	_interface = new Interface(this);
	_locations = new LocationManager();
	_map = new Map(this);
	_party = new Party(this);
	_patcher = new Patcher();
	_saves = new SavesManager(_targetName);
	_screen = new Screen(this);
	_scripts = new Scripts(this);
	_sound = new Sound(_mixer);
	_spells = new Spells(this);
	_windows = new Windows();

	// Set graphics mode
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Setup mixer
	syncSoundSettings();

	// Load settings
	loadSettings();

	return true;
}

void XeenEngine::loadSettings() {
	_gameWon[0] = ConfMan.hasKey("game_won") && ConfMan.getBool("game_won");
	_gameWon[1] = ConfMan.hasKey("game_won2") && ConfMan.getBool("game_won2");
	_gameWon[2] = ConfMan.hasKey("game_won3") && ConfMan.getBool("game_won3");
	_finalScore = ConfMan.hasKey("final_score") ? ConfMan.getInt("final_score") : 0;

	_extOptions._showItemCosts = ConfMan.hasKey("ShowItemCosts") && ConfMan.getBool("ShowItemCosts");
	_extOptions._durableArmor = ConfMan.hasKey("DurableArmor") && ConfMan.getBool("DurableArmor");

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			_loadSaveSlot = saveSlot;
	}
}

Common::Error XeenEngine::run() {
	if (initialize())
		outerGameLoop();

	return Common::kNoError;
}

void XeenEngine::outerGameLoop() {
	if (_loadSaveSlot != -1)
		// Loading savegame from launcher, so Skip menu and go straight to game
		_gameMode = GMODE_PLAY_GAME;

	while (!shouldQuit() && _gameMode != GMODE_QUIT) {
		GameMode mode = _gameMode;
		_gameMode = GMODE_NONE;
		assert(mode != GMODE_NONE);

		switch (mode) {
		case GMODE_STARTUP:
			showStartup();
			break;

		case GMODE_MENU:
			showMainMenu();
			break;

		case GMODE_PLAY_GAME:
			playGame();
			break;

		default:
			break;
		}
	}
}

int XeenEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

int XeenEngine::getRandomNumber(int minNumber, int maxNumber) {
	return getRandomNumber(maxNumber - minNumber) + minNumber;
}

Common::Error XeenEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	return _saves->saveGameState(slot, desc);
}

Common::Error XeenEngine::loadGameState(int slot) {
	_loadSaveSlot = slot;
	return Common::kNoError;
}

bool XeenEngine::canLoadGameStateCurrently() {
	return _mode != MODE_STARTUP;
}

bool XeenEngine::canSaveGameStateCurrently() {
	return _mode != MODE_COMBAT && _mode != MODE_STARTUP && _mode != MODE_SCRIPT_IN_PROGRESS
		&& (_map->mazeData()._mazeFlags & RESTRICTION_SAVE) == 0;
}

bool XeenEngine::canSaveAutosaveCurrently() {
	return canSaveGameStateCurrently() &&
		(_map && !(_map->mazeData()._mazeFlags & RESTRICTION_SAVE));
}

void XeenEngine::playGame() {
	_files->setGameCc(0);
	_sound->stopAllAudio();
	SpriteResource::setClippedBottom(140);

	play();
	_sound->stopAllAudio();
}

void XeenEngine::play() {
	_interface->setup();
	_screen->loadBackground("back.raw");
	_screen->loadPalette("mm4.pal");

	if (getGameID() == GType_DarkSide && !_map->_loadCcNum) {
		_map->_loadCcNum = 1;
		_party->_mazeId = 29;
		_party->_mazeDirection = DIR_NORTH;
		_party->_mazePosition.x = 25;
		_party->_mazePosition.y = 21;
	}

	_map->clearMaze();
	if (_loadSaveSlot >= 0) {
		_saves->newGame();
		_saves->loadGameState(_loadSaveSlot);
		_loadSaveSlot = -1;
	} else {
		_map->load(_party->_mazeId);
	}

	_interface->startup();
	if (_mode == MODE_STARTUP) {
//		_screen->fadeOut();
	}

	(*_windows)[0].update();
	_interface->mainIconsPrint();
	(*_windows)[0].update();
	_events->setCursor(0);

	_combat->_moveMonsters = true;
	if (_mode == MODE_STARTUP) {
		_mode = MODE_INTERACTIVE;
		_screen->fadeIn();
	}

	_combat->_moveMonsters = true;

	gameLoop();

	if (_party->_dead)
		death();

	_mode = MODE_STARTUP;
	_gameMode = GMODE_MENU;
}

void XeenEngine::gameLoop() {
	// Main game loop
	while (isLoadPending() || !shouldExit()) {
		if (isLoadPending()) {
			// Load any pending savegame
			int saveSlot = _loadSaveSlot;
			_loadSaveSlot = -1;
			(void)_saves->loadGameState(saveSlot);
			_interface->drawParty(true);
		}

		_map->cellFlagLookup(_party->_mazePosition);
		if (_map->_currentIsEvent) {
			_gameMode = (GameMode)_scripts->checkEvents();
			if (isLoadPending())
				continue;
			if (shouldExit())
				return;
		}
		_party->giveTreasure();

		// Main user interface handler for waiting for and processing user input
		_interface->perform();

		if (_party->_dead)
			break;
	}
}

Common::String XeenEngine::printMil(uint value) {
	return (value >= 1000000) ? Common::String::format("%u mil", value / 1000000) :
		Common::String::format("%u", value);
}

Common::String XeenEngine::printK(uint value) {
	return (value > 9999) ? Common::String::format("%uk", value / 1000) :
		Common::String::format("%u", value);
}

Common::String XeenEngine::printK2(uint value) {
	return (value > 999) ? Common::String::format("%uk", value / 1000) :
		Common::String::format("%u", value);
}

void XeenEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	if (_sound)
		_sound->updateSoundSettings();
}

void XeenEngine::saveSettings() {
	if (_gameWon[0])
		ConfMan.setBool("game_won", true);
	if (_gameWon[1])
		ConfMan.setBool("game_won2", true);
	if (_gameWon[2])
		ConfMan.setBool("game_won3", true);

	ConfMan.setInt("final_score", _finalScore);
	ConfMan.flushToDisk();
}

void XeenEngine::GUIError(const Common::U32String &msg) {
	GUIErrorMessage(msg);
}

} // End of namespace Xeen
