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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "got/got.h"
#include "got/detection.h"
#include "got/console.h"
#include "got/utils/res_archive.h"

namespace Got {

GotEngine *g_engine;

GotEngine::GotEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Got") {
	g_engine = this;
}

GotEngine::~GotEngine() {
}

uint32 GotEngine::getFeatures() const {
	return _gameDescription->flags;
}

bool GotEngine::isDemo() const {
	return (_gameDescription->flags & ADGF_DEMO) != 0;
}

Common::String GotEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error GotEngine::run() {
	// Initialize 320x240 palleted graphics mode. Note that the original
	// main menu/dialogs ran at 320x200, but the game ran at 320x240.
	initGraphics(320, 240);

	// Set the engine's debugger console
	setDebugger(new Console());

	// Initialize resources and variables
	resInit();
	_vars.load();

	runGame();

	return Common::kNoError;
}

Common::Error GotEngine::syncGame(Common::Serializer &s) {
	byte version = 1;
	s.syncAsByte(version);

	_G(setup).sync(s);
	_G(thor_info).sync(s);
	_G(sd_data).sync(s);

	if (s.isLoading()) {
		// TODO
	}

	return Common::kNoError;
}

} // End of namespace Got
