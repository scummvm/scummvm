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
#include "chewy/cursor.h"
#include "chewy/events.h"
#include "chewy/graphics.h"
#include "chewy/resource.h"
#include "chewy/scene.h"
#include "chewy/sound.h"
#include "chewy/text.h"

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
	delete _events;
	delete _text;
	delete _sound;
	delete _cursor;
	delete _scene;
	delete _graphics;
}

void ChewyEngine::initialize() {
	setDebugger(new Console(this));
	_cursor = new Cursor();
	_graphics = new Graphics(this);
	_scene = new Scene(this);
	_sound = new Sound(_mixer);
	_text = new Text();
	_events = new Events(this);

	_curCursor = 0;
	_elapsedFrames = 0;
	_videoNum = -1;
}

Common::Error ChewyEngine::run() {
	// Initialize backend
	//initGraphics(640, 480);
	initGraphics(320, 200);

	initialize();

	/*for (uint i = 0; i < 161; i++) {
		debug("Video %d", i);
		_graphics->playVideo(i);
	}*/

	//_graphics->playVideo(0);

	_scene->change(0);
	//_sound->playSpeech(1);
	//_sound->playSound(1);
	//_sound->playMusic(2);

	// Run a dummy loop
	while (!shouldQuit()) {
		_events->processEvents();

		// Cursor animation
		if (_elapsedFrames % 30 == 0)
			_cursor->animateCursor();

		if (_videoNum >= 0) {
			_graphics->playVideo(_videoNum);
			_scene->draw();
			_videoNum = -1;
		}

		g_system->updateScreen();
		g_system->delayMillis(10);

		_elapsedFrames++;
	}

	return Common::kNoError;
}

} // End of namespace Chewy
