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
#include "chewy/graphics.h"
#include "chewy/resource.h"

namespace Chewy {

ChewyEngine::ChewyEngine(OSystem *syst, const ChewyGameDescription *gameDesc)
	: Engine(syst),
	_gameDescription(gameDesc),
	_rnd("chewy") {

	_console = new Console(this);

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
}

Common::Error ChewyEngine::run() {
	// Initialize backend
	initGraphics(640, 480, true);

	initialize();

	Graphics *g = new Graphics();
	g->drawImage("comic.tgp", 0);
	delete g;

	// Run a dummy loop
	Common::Event event;

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			if ((event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) || event.type == Common::EVENT_LBUTTONUP)
				g_engine->quitGame();
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.flags & Common::KBD_CTRL && event.kbd.keycode == Common::KEYCODE_d)
				_console->attach();
		}

		_console->onFrame();
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

void ChewyEngine::initialize() {
}

} // End of namespace Chewy
