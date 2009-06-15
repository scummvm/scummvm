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
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/file.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, Common::Language language)
    : Engine(system) {

    Common::File::addDefaultDirectory(_gameDataDir.getChild("Data"));
    Common::File::addDefaultDirectory(_gameDataDir.getChild("Vids"));
	Common::File::addDefaultDirectory(_gameDataDir.getChild("Music"));

    _eventMan->registerRandomSource(_rnd, "asylum");
}

AsylumEngine::~AsylumEngine() {
    //Common::clearAllDebugChannels();
	delete _mainMenu;
	delete _video;
	delete _sound;
	delete _screen;
}

Common::Error AsylumEngine::run() {
    Common::Error err;
    err = init();
    if (err != Common::kNoError)
            return err;
    return go();
}

// Will do the same as subroutine at address 0041A500
Common::Error AsylumEngine::init() {
	// initialize engine objects

	initGraphics(640, 480, true);

	_screen = new Screen(_system);
	_sound = new Sound(_mixer);
	_video = new Video(_mixer);
	_mainMenu = 0;
	_scene = 0;

    return Common::kNoError;
}

Common::Error AsylumEngine::go() {
    // initializing game
	// TODO: save dialogue key codes into sntrm_k.txt (need to figure out why they use such thing)
	// TODO: load startup configurations (address 0041A970)
	// TODO: init unknown game stuffs (address 0040F430)
	// TODO: if savegame exists on folder, than start NewGame()

	// Play intro movie
	// Disabled for quick testing
	//_video->playVideo(0);

	// Set up the game's main scene
    _scene = new Scene(_screen, _sound, 5);

	// Set up main menu
	_mainMenu = new MainMenu(_screen, _sound, _scene);

	// Enter first scene
	_scene->enterScene();

	while (!shouldQuit()) {
		checkForEvent(true);
		waitForTimer(60);
	}

    return Common::kNoError;
}

void AsylumEngine::waitForTimer(int msec_delay) {
	uint32 start_time = _system->getMillis();

	while (_system->getMillis() < start_time + msec_delay) {
		checkForEvent(false);
		_system->updateScreen();
		_system->delayMillis(10);
	}
}

void AsylumEngine::checkForEvent(bool doUpdate) {
	Common::Event ev;

	if (_system->getEventManager()->pollEvent(ev)) {
		if (ev.type == Common::EVENT_KEYDOWN) {
			if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
				/*
				// Push a quit event
				Common::Event event;
				event.type = Common::EVENT_QUIT;
				g_system->getEventManager()->pushEvent(event);
				*/
				// Toggle menu
				if (_mainMenu->isActive()) {
					_mainMenu->closeMenu();
					_scene->enterScene();
				} else {
					_mainMenu->openMenu();
				}

				return;
			}
		}
	}

	if (doUpdate) {
		// Copy background image
		_screen->copyBackBufferToScreen();
	}

	if (_mainMenu->isActive()) {
		// Main menu active, pass events to it
		_mainMenu->handleEvent(&ev, doUpdate);
	} else {
		// Pass events to the game
		_scene->handleEvent(&ev, doUpdate);
	}
}

} // namespace Asylum
