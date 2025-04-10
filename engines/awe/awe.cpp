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

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "engines/util.h"
#include "awe/awe.h"
#include "awe/engine.h"
#include "awe/systemstub.h"

namespace Awe {

AweEngine::AweEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc) {
}

Common::Error AweEngine::run() {
	// Setup mixer
	if (!_mixer->isReady()) {
		::warning("Sound initialization failed.");
	}

	// Initialize backend
	initGraphics(320, 200);

	// Run the game
	SystemStub *stub = SystemStub_SDL_create();
	Awe::Engine *e = new Awe::Engine(stub, nullptr, nullptr);
	e->run();
	delete e;
	delete stub;

	return Common::kNoError;
}

} // namespace Awe
