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

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/system.h"
#include "engines/util.h"
#include "chewy/chewy.h"
#include "chewy/events.h"
#include "chewy/main.h"
#include "chewy/resource.h"
#include "chewy/sound.h"

namespace Chewy {

ChewyEngine *g_engine;

ChewyEngine::ChewyEngine(OSystem *syst, const ChewyGameDescription *gameDesc)
		: Engine(syst),
		_gameDescription(gameDesc),
		_rnd("chewy"),
		_events(nullptr),
		_screen(nullptr),
		_sound(nullptr) {

	g_engine = this;
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
	delete _screen;
	delete _sound;
	g_engine = nullptr;
}

void ChewyEngine::initialize() {
	_events = new EventsManager();
	_screen = new Graphics::Screen();
	_sound = new Sound(_mixer);
}

Common::Error ChewyEngine::run() {
	// Initialize backend
	//initGraphics(640, 480);
	initGraphics(320, 200);

	initialize();

	game_main();

	return Common::kNoError;
}

} // End of namespace Chewy
