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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/sherlock.h"
#include "sherlock/graphics.h"
#include "common/scummsys.h"
#include "common/debug-channels.h"
#include "engines/util.h"

namespace Sherlock {

SherlockEngine::SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Sherlock") {
	_animation = nullptr;
	_debugger = nullptr;
	_events = nullptr;
	_inventory = nullptr;
	_journal = nullptr;
	_people = nullptr;
	_res = nullptr;
	_scene = nullptr;
	_screen = nullptr;
	_scripts = nullptr;
	_sound = nullptr;
	_talk = nullptr;
	_ui = nullptr;
	_useEpilogue2 = false;
	_justLoaded = false;
	_loadingSavedGame = false;
	_onChessboard = false;
	_slowChess = false;
}

SherlockEngine::~SherlockEngine() {
	delete _animation;
	delete _debugger;
	delete _events;
	delete _journal;
	delete _people;
	delete _scene;
	delete _screen;
	delete _scripts;
	delete _sound;
	delete _talk;
	delete _ui;
	delete _inventory;
	delete _res;
}

void SherlockEngine::initialize() {
	initGraphics(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT, false);

	DebugMan.addDebugChannel(kDebugScript, "scripts", "Script debug level");

	ImageFile::setVm(this);
	Object::setVm(this);
	Sprite::setVm(this);
	_res = new Resources();
	_animation = new Animation(this);
	_debugger = new Debugger(this);
	_events = new Events(this);
	_inventory = new Inventory(this);
	_journal = new Journal();
	_people = new People(this);
	_scene = new Scene(this);
	_screen = new Screen(this);
	_scripts = new Scripts(this);
	_sound = new Sound(this);
	_talk = new Talk(this);
	_ui = new UserInterface(this);
}

Common::Error SherlockEngine::run() {
	initialize();

	// Temporarily disabled for now
//	showOpening();

	while (!shouldQuit()) {
		// Prepare for scene, and handle any game-specific scenes. This allows 
		// for game specific cutscenes or mini-games that aren't standard scenes
		startScene();
		if (shouldQuit())
			break;

		// Reset UI flags
		_ui->reset();

		// Reset the active characters to initially just Sherlock
		_people->reset();

		// Initialize and load the scene. 
		_scene->selectScene();

		// Scene handling loop
		sceneLoop();
	}

	return Common::kNoError;
}

void SherlockEngine::sceneLoop() {
	while (!shouldQuit() && _scene->_goToScene == -1) {
		// See if a script needs to be completed from either a goto room code,
		// or a script that was interrupted by another script
		if (_scripts->_scriptMoreFlag == 1 || _scripts->_scriptMoreFlag == 3)
			_talk->talkTo(_scripts->_scriptName);
		else
			_scripts->_scriptMoreFlag = 0;

		// Handle any input from the keyboard or mouse
		handleInput();

		if (_scene->_hsavedPos.x == -1)
			_scene->doBgAnim();
	}

	_scene->freeScene();
	_people->freeWalk();

}

/**
 * Handle all player input
 */
void SherlockEngine::handleInput() {
	_events->pollEventsAndWait();

	// See if a key or mouse button is pressed
	_events->setButtonState();

	_ui->handleInput();
}


/**
 * Read the state of a global flag
 */
bool SherlockEngine::readFlags(int flagNum) {
	bool value = _flags[ABS(flagNum)];
	if (flagNum < 0)
		value = !value;

	return value;
}

/**
 * Sets a global flag to either true or false depending on whether the specified
 * flag is positive or negative
 */
void SherlockEngine::setFlags(int flagNum) {
	_flags[ABS(flagNum)] = flagNum >= 0;

	_scene->checkSceneFlags(true);
}

void SherlockEngine::freeSaveGameList() {
	// TODO
}

} // End of namespace Comet
