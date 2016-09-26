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

#include "common/config-manager.h"
#include "common/error.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/palette.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "chewy/chewy.h"
#include "chewy/console.h"
#include "chewy/graphics.h"
#include "chewy/resource.h"
#include "chewy/sound.h"

namespace Chewy {

ChewyEngine::ChewyEngine(OSystem *syst, const ChewyGameDescription *gameDesc)
	: Engine(syst),
	_gameDescription(gameDesc),
	_rnd("chewy") {

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "back");
	SearchMan.addSubDirectoryMatching(gameDataDir, "cut");
	SearchMan.addSubDirectoryMatching(gameDataDir, "err");
	SearchMan.addSubDirectoryMatching(gameDataDir, "misc");
	SearchMan.addSubDirectoryMatching(gameDataDir, "room");
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");
	SearchMan.addSubDirectoryMatching(gameDataDir, "txt");
}

ChewyEngine::~ChewyEngine() {
	delete _console;
	delete _sound;
	delete _graphics;
}

void ChewyEngine::initialize() {
	_console = new Console(this);
	_graphics = new Graphics();
	_sound = new Sound();

	_curCursor = 0;
	_elapsedFrames = 0;
}

Common::Error ChewyEngine::run() {
	// Initialize backend
	//initGraphics(640, 480, true);
	initGraphics(320, 200, false);

	initialize();

	/*for (uint i = 0; i < 161; i++) {
		debug("Video %d", i);
		_graphics->playVideo(i);
	}*/

	//_graphics->playVideo(0);
	_graphics->drawImage("episode1.tgp", 0);
	_graphics->showCursor();
	_graphics->setCursor(0);
	//_sound->playSpeech(1);
	//_sound->playSound(1);
	//_sound->playMusic(2);

	// Run a dummy loop
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(_event)) {
			if (_event.type == Common::EVENT_KEYDOWN && _event.kbd.keycode == Common::KEYCODE_ESCAPE)
				g_engine->quitGame();
			if ((_event.type == Common::EVENT_KEYDOWN && _event.kbd.keycode == Common::KEYCODE_SPACE) || _event.type == Common::EVENT_RBUTTONUP)
				_graphics->nextCursor();
			if (_event.type == Common::EVENT_KEYDOWN && _event.kbd.flags & Common::KBD_CTRL && _event.kbd.keycode == Common::KEYCODE_d)
				_console->attach();
		}

		_console->onFrame();

		// Cursor animation
		if (_elapsedFrames % 30 == 0)
			_graphics->animateCursor();

		g_system->updateScreen();
		g_system->delayMillis(10);

		_elapsedFrames++;
	}

	return Common::kNoError;
}

} // End of namespace Chewy
