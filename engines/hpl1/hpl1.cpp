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

#include "engine/engine.h"
#include "hpl1/hpl1.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "hpl1/console.h"
#include "hpl1/detection.h"

extern int hplMain(const hpl::tString &asCommandLine);

namespace Hpl1 {

Hpl1Engine *g_engine;

Hpl1Engine::Hpl1Engine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _randomSource("Hpl1") {
	g_engine = this;
}

Hpl1Engine::~Hpl1Engine() {
	delete _screen;
}

uint32 Hpl1Engine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String Hpl1Engine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error Hpl1Engine::run() {
	hplMain("");
	return Common::kNoError;
}

Common::Error Hpl1Engine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // namespace Hpl1
