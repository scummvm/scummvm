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
#include "engines/util.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/hodjnpodj/mazedoom/game_exe.h"

namespace Bagel {
namespace HodjNPodj {

HodjNPodjEngine *g_engine;

HodjNPodjEngine::HodjNPodjEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		BagelEngine(syst, gameDesc) {
	g_engine = this;
}

HodjNPodjEngine::~HodjNPodjEngine() {
	g_engine = nullptr;
}

Common::Error HodjNPodjEngine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	Common::String minigame = ConfMan.get("minigame");
	if (minigame == "mazedoom")
		MazeDoom::CTheApp::run();
	else
		warning("TODO: entire game");

	// TODO: overall game

	return Common::kNoError;
}

} // namespace HodjNPodj
} // namespace Bagel
