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

#include "alg/alg.h"
#include "alg/game.h"
#include "alg/game_bountyhunter.h"
#include "alg/game_crimepatrol.h"
#include "alg/game_drugwars.h"
#include "alg/game_johnnyrock.h"
#include "alg/game_maddog.h"
#include "alg/game_maddog2.h"
#include "alg/game_spacepirates.h"

namespace Alg {

AlgEngine::AlgEngine(OSystem *syst, const ADGameDescription *desc)
	: Engine(syst) {
	if (scumm_stricmp(desc->gameId, "cpatrols") == 0 || scumm_stricmp(desc->gameId, "cpatrold") == 0 || scumm_stricmp(desc->gameId, "cpatroldemo") == 0) {
		GameCrimePatrol *game = new GameCrimePatrol(this, desc);
		_debugger = new DebuggerCrimePatrol(game);
		_game = game;
	} else if (scumm_stricmp(desc->gameId, "dwarss") == 0 || scumm_stricmp(desc->gameId, "dwarsd") == 0 || scumm_stricmp(desc->gameId, "dwarsdemo") == 0) {
		GameDrugWars *game = new GameDrugWars(this, desc);
		_debugger = new DebuggerDrugWars(game);
		_game = game;
	} else if (scumm_stricmp(desc->gameId, "johnrocs") == 0 || scumm_stricmp(desc->gameId, "johnrocd") == 0) {
		GameJohnnyRock *game = new GameJohnnyRock(this, desc);
		_debugger = new DebuggerJohnnyRock(game);
		_game = game;
	} else if (scumm_stricmp(desc->gameId, "lbhunter") == 0 || scumm_stricmp(desc->gameId, "lbhunterdemo") == 0) {
		GameBountyHunter *game = new GameBountyHunter(this, desc);
		_debugger = new DebuggerBountyHunter(game);
		_game = game;
	} else if (scumm_stricmp(desc->gameId, "maddog") == 0) {
		GameMaddog *game = new GameMaddog(this, desc);
		_debugger = new DebuggerMaddog(game);
		_game = game;
	} else if (scumm_stricmp(desc->gameId, "maddog2s") == 0 || scumm_stricmp(desc->gameId, "maddog2d") == 0) {
		GameMaddog2 *game = new GameMaddog2(this, desc);
		_debugger = new DebuggerMaddog2(game);
		_game = game;
	} else if (scumm_stricmp(desc->gameId, "spiratess") == 0 || scumm_stricmp(desc->gameId, "spiratesd") == 0 || scumm_stricmp(desc->gameId, "spiratesdemo") == 0) {
		GameSpacePirates *game = new GameSpacePirates(this, desc);
		_debugger = new DebuggerSpacePirates(game);
		_game = game;
	}
}

AlgEngine::~AlgEngine() {
	delete _game;
}

Common::Error AlgEngine::run() {
	initGraphics(320, 200);
	setDebugger(_debugger);
	return _game->run();
}

bool AlgEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime);
}

} // End of namespace Alg
