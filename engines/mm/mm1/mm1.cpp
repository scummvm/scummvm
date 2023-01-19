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
#include "engines/util.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/gfx/gfx.h"
#include "mm/mm1/views/title_view.h"

namespace MM {
namespace MM1 {

MM1Engine *g_engine = nullptr;

MM1Engine::MM1Engine(OSystem *syst, const MightAndMagicGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _randomSource("MM1") {
	g_engine = this;
}

MM1Engine::~MM1Engine() {
	g_engine = nullptr;
}

Common::Error MM1Engine::run() {
	// Initialize graphics mode
	initGraphics(320, 200);
	Gfx::GFX::setEgaPalette(0);

	// Load globals
	if (!_globals.load())
		return Common::kNoError;

	// Run the game
	Views::TitleView screenView(this);
	runGame();
	return Common::kNoError;
}

} // End of namespace Xeen
} // End of namespace MM
