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
#include "graphics/palette.h"
#include "m4/adv_r/adv_been.h"
#include "m4/m4.h"
#include "m4/detection.h"
#include "m4/console.h"
#include "m4/param.h"

namespace M4 {

M4Engine *g_engine;

M4Engine::M4Engine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("M4") {
	g_engine = this;
}

M4Engine::~M4Engine() {
	delete _screen;
}

uint32 M4Engine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String M4Engine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error M4Engine::run() {
	Globals globals;

	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	param_init();
	parse_all_flags();

	if (!_G(system_shutting_down)) {
		game_systems_initialize(INSTALL_ALL);



		// TODO
	}

	param_shutdown();
	return Common::kNoError;
}

void M4Engine::game_systems_initialize(byte flags) {
	_G(term).init(_G(kernel).use_debug_monitor, _G(kernel).use_log_file);

	size_t totalMem = _G(kernel).mem_avail();
	if (_G(kernel).suppress_cache == CACHE_NOT_OVERRIDE_BY_FLAG_PARSE)
		_G(kernel).suppress_cache = totalMem < 8000000;

	debugC(kDebugCore, _G(kernel).suppress_cache ?
		"Cache System Disabled" : "Cache System Enabled");
	debugC(kDebugCore, "Available memory: %ld", totalMem);

	if (flags & INSTALL_PLAYER_BEEN_INIT)
		player_been_init(180);
}

Common::Error M4Engine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace M4
