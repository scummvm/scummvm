/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Game detection, general game parameters

#include "saga/saga.h"

#include "common/file.h"
#include "common/fs.h"
#include "common/config-manager.h"
#include "common/advancedDetector.h"
#include "base/plugins.h"

#include "saga/rscfile.h"
#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/sagaresnames.h"


namespace Saga {
static DetectedGameList GAME_detectGames(const FSList &fslist);
}

static const PlainGameDescriptor saga_games[] = {
	{"ite", "Inherit the Earth: Quest for the Orb"},
	{"ihnm", "I Have No Mouth and I Must Scream"},
	{0, 0}
};

ADVANCED_DETECTOR_GAMEID_LIST(SAGA, saga_games);

ADVANCED_DETECTOR_FIND_GAMEID(SAGA, saga_games, NULL);

ADVANCED_DETECTOR_DETECT_GAMES(SAGA, Saga::GAME_detectGames);

ADVANCED_DETECTOR_ENGINE_CREATE(SAGA, Saga::SagaEngine, "SagaEngine", NULL);


REGISTER_PLUGIN(SAGA, "SAGA Engine", "Inherit the Earth (C) Wyrmkeep Entertainment");

namespace Saga {

using Common::ADGameFileDescription;
using Common::ADGameDescription;

#include "sagagame.cpp"

ADVANCED_DETECTOR_TO_DETECTED_GAME(saga_games);

bool SagaEngine::postInitGame() {
	_gameDisplayInfo = *_gameDescription->gameDisplayInfo;
	_displayClip.right = _gameDisplayInfo.logicalWidth;
	_displayClip.bottom = _gameDisplayInfo.logicalHeight;

	if (!_resource->createContexts()) {
		return false;
	}
	return true;
}

ADVANCED_DETECTOR_DETECT_INIT_GAME(SagaEngine::initGame, gameDescriptions, _gameDescription, postInitGame);

ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(GAME_detectGames, gameDescriptions);

} // End of namespace Saga
