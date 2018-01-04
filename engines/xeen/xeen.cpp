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
	_resources = nullptr;
	_saves = nullptr;
	_screen = nullptr;
	_scripts = nullptr;
	_sound = nullptr;
	_spells = nullptr;
	_windows = nullptr;
	_eventData = nullptr;
	_noDirectionSense = false;
	_startupWindowActive = false;
	_quitMode = 0;
	_mode = MODE_0;
	_endingScore = 0;
	_loadSaveSlot = -1;
	g_vm = this;
}

XeenEngine::~XeenEngine() {
	delete _combat;
	delete _debugger;
	delete _events;
	delete _interface;
	delete _locations;
	delete _map;
	delete _party;
	delete _saves;
	delete _screen;
	delete _scripts;
	delete _sound;
	delete _spells;
	delete _windows;
	delete _eventData;
	delete _resources;
	delete _files;
	g_vm = nullptr;
}

void XeenEngine::initialize() {
	// Create sub-objects of the engine
	_files = new FileManager(this);
	_resources = Resources::init(this);
	_combat = new Combat(this);
	_debugger = new Debugger(this);
	_events = new EventsManager(this);
	_interface = new Interface(this);
	_locations = new LocationManager();
	_map = new Map(this);
	_party = new Party(this);
	_saves = new SavesManager(_targetName);
	_screen = new Screen(this);
	_scripts = new Scripts(this);
	_sound = new Sound(this, _mixer);
	_spells = new Spells(this);
	_windows = new Windows();

	File f("029.obj", 1);
	_eventData = f.readStream(f.size());

	// Set graphics mode
	initGraphics(320, 200);

	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			_loadSaveSlot = saveSlot;
	}
}

Common::Error XeenEngine::run() {
	initialize();

	outerGameLoop();

	return Common::kNoError;
}

int XeenEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

int XeenEngine::getRandomNumber(int minNumber, int maxNumber) {
	return getRandomNumber(maxNumber - minNumber) + minNumber;
}

Common::Error XeenEngine::saveGameState(int slot, const Common::String &desc) {
	return _saves->saveGameState(slot, desc);
}

Common::Error XeenEngine::loadGameState(int slot) {
	return _saves->loadGameState(slot);
}

bool XeenEngine::canLoadGameStateCurrently() {
	return true;
}

bool XeenEngine::canSaveGameStateCurrently() {
	return true;
}

void XeenEngine::playGame() {
	_files->setGameCc(0);
	_sound->stopAllAudio();

	play();
}

void XeenEngine::play() {
	// TODO: Init variables
	_quitMode = 0;

	_interface->setup();
	_screen->loadBackground("back.raw");
	_screen->loadPalette("mm4.pal");

	if (getGameID() != GType_WorldOfXeen && !_map->_loadDarkSide) {
		_map->_loadDarkSide = true;
		_party->_mazeId = 29;
		_party->_mazeDirection = DIR_NORTH;
		_party->_mazePosition.x = 25;
		_party->_mazePosition.y = 21;
	}

	_map->load(_party->_mazeId);

	_interface->startup();
	if (_mode == MODE_0) {
//		_screen->fadeOut();
	}

	(*_windows)[0].update();
	_interface->mainIconsPrint();
	(*_windows)[0].update();
	_events->setCursor(0);

	_combat->_moveMonsters = true;
	if (_mode == MODE_0) {
		_mode = MODE_1;
		_screen->fadeIn();
	}

	_combat->_moveMonsters = true;

	gameLoop();
}

void XeenEngine::gameLoop() {
	// Main game loop
	while (!shouldQuit()) {
		_map->cellFlagLookup(_party->_mazePosition);
		if (_map->_currentIsEvent) {
			_quitMode = _scripts->checkEvents();
			if (shouldQuit() || _quitMode)
				return;
		}
		_party->giveTreasure();

		// Main user interface handler for waiting for and processing user input
		_interface->perform();
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

} // End of namespace Xeen
