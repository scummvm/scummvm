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
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "common/system.h"
#include "engines/util.h"

#include "cryo/cryo.h"
#include "cryo/eden.h"

namespace Cryo {

CryoEngine *g_ed = nullptr;

CryoEngine::CryoEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_rnd = new Common::RandomSource("cryo");

	_game = nullptr;
	_screenView = nullptr;

	_showHotspots = false;
	_timerTicks = 0;

	_mouseButton = 0;

	g_ed = this;
}

CryoEngine::~CryoEngine() {
	delete _rnd;
	delete _game;
	delete _screenView;
}

Common::Error CryoEngine::run() {
	_game = new EdenGame(this);
	_screenView = new View(320, 200);
	setDebugger(new Debugger(this));

	///// CLTimer
	_timerTicks = 0;   // incremented in realtime

	// Initialize graphics using following:
	initGraphics(320, 200);
	_screen.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	_game->run();

	return Common::kNoError;
}

bool CryoEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

} // End of namespace Cryo
