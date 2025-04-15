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

#include "alg/alg.h"
#include "alg/detection.h"
#include "alg/game.h"
#include "alg/logic/game_bountyhunter.h"
#include "alg/logic/game_crimepatrol.h"
#include "alg/logic/game_drugwars.h"
#include "alg/logic/game_johnnyrock.h"
#include "alg/logic/game_maddog.h"
#include "alg/logic/game_maddog2.h"
#include "alg/logic/game_spacepirates.h"

namespace Alg {

AlgEngine::AlgEngine(OSystem *syst, const AlgGameDescription *gd)
	: Engine(syst), _gameDescription(gd) {
	switch (gd->gameType) {
	case GType_CRIME_PATROL: {
		GameCrimePatrol *game = new GameCrimePatrol(this, gd);
		_debugger = new DebuggerCrimePatrol(game);
		_game = game;
		break;
	}
	case GType_DRUG_WARS: {
		GameDrugWars *game = new GameDrugWars(this, gd);
		_debugger = new DebuggerDrugWars(game);
		_game = game;
		break;
	}
	case GType_WSJR: {
		GameJohnnyRock *game = new GameJohnnyRock(this, gd);
		_debugger = new DebuggerJohnnyRock(game);
		_game = game;
		break;
	}
	case GType_LAST_BOUNTY_HUNTER: {
		GameBountyHunter *game = new GameBountyHunter(this, gd);
		_debugger = new DebuggerBountyHunter(game);
		_game = game;
		break;
	}
	case GType_MADDOG: {
		GameMaddog *game = new GameMaddog(this, gd);
		_debugger = new DebuggerMaddog(game);
		_game = game;
		break;
	}
	case GType_MADDOG2: {
		GameMaddog2 *game = new GameMaddog2(this, gd);
		_debugger = new DebuggerMaddog2(game);
		_game = game;
		break;
	}
	case GType_SPACE_PIRATES: {
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
	if (ConfMan.hasKey("single_speed_videos")) {
		_useSingleSpeedVideos = ConfMan.getBool("single_speed_videos");
	}
	return _game->run();
}

bool AlgEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) ||
		   (f == kSupportsLoadingDuringRuntime) ||
		   (f == kSupportsSavingDuringRuntime);
}

} // End of namespace Alg
