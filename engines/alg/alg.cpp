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
#include "alg/detection.h"
#include "alg/game.h"
#include "alg/game_bountyhunter.h"
#include "alg/game_crimepatrol.h"
#include "alg/game_drugwars.h"
#include "alg/game_johnnyrock.h"
#include "alg/game_maddog.h"
#include "alg/game_maddog2.h"
#include "alg/game_spacepirates.h"

namespace Alg {

AlgEngine::AlgEngine(OSystem *syst, const AlgGameDescription *gd)
	: Engine(syst) {
	switch (gd->gameType) {
	case Alg::GType_CPATROL_SS_DOS:
	case Alg::GType_CPATROL_DS_DOS:
	case Alg::GType_CPATROL_DEMO_DOS: {
		GameCrimePatrol *game = new GameCrimePatrol(this, gd);
		_debugger = new DebuggerCrimePatrol(game);
		_game = game;
		break;
	}
	case Alg::GType_DWARS_SS_DOS:
	case Alg::GType_DWARS_DS_DOS:
	case Alg::GType_DWARS_DEMO_DOS: {
		GameDrugWars *game = new GameDrugWars(this, gd);
		_debugger = new DebuggerDrugWars(game);
		_game = game;
		break;
	}
	case Alg::GType_JOHNROC_SS_DOS:
	case Alg::GType_JOHNROC_DS_DOS: {
		GameJohnnyRock *game = new GameJohnnyRock(this, gd);
		_debugger = new DebuggerJohnnyRock(game);
		_game = game;
		break;
	}
	case Alg::GType_LBHUNTER_DOS:
	case Alg::GType_LBHUNTER_DEMO_DOS: {
		GameBountyHunter *game = new GameBountyHunter(this, gd);
		_debugger = new DebuggerBountyHunter(game);
		_game = game;
		break;
	}
	case Alg::GType_MADDOG_DOS: {
		GameMaddog *game = new GameMaddog(this, gd);
		_debugger = new DebuggerMaddog(game);
		_game = game;
		break;
	}
	case Alg::GType_MADDOG2_SS_DOS:
	case Alg::GType_MADDOG2_DS_DOS: {
		GameMaddog2 *game = new GameMaddog2(this, gd);
		_debugger = new DebuggerMaddog2(game);
		_game = game;
		break;
	}
	case Alg::GType_SPIRATES_SS_DOS:
	case Alg::GType_SPIRATES_DS_DOS:
	case Alg::GType_SPIRATES_DEMO_DOS: {
		GameSpacePirates *game = new GameSpacePirates(this, gd);
		_debugger = new DebuggerSpacePirates(game);
		_game = game;
		break;
	}
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
