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

#include "engines/util.h"
#include "ultima/ultima0/sdw.h"
#include "ultima/ultima0/ultima0.h"
#include "ultima/ultima0/akalabeth.h"
#include "ultima/ultima0/sdw.h"

namespace Ultima {
namespace Ultima0 {

Ultima0Engine *g_engine;

Ultima0Engine::Ultima0Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc), _randomSource("Ultima0") {
	g_engine = this;
}

Ultima0Engine::~Ultima0Engine() {
}

Common::Error Ultima0Engine::run() {
	initGraphics(DEFAULT_SCX, DEFAULT_SCY);
	Display = new Graphics::Screen();
	GameSpeed = 120;

	// Call the real main program
	MAINStart();

	delete Display;

	return Common::kNoError;
}

} // namespace Ultima0
} // namespace Ultima
