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

GameList Engine_SAGA_gameIDList() {
	GameList games;
	const PlainGameDescriptor *g = saga_games;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}

	return games;
}

GameDescriptor Engine_SAGA_findGameID(const char *gameid) {
	const PlainGameDescriptor *g = saga_games;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return *g;
}

DetectedGameList Engine_SAGA_detectGames(const FSList &fslist) {
	return Saga::GAME_detectGames(fslist);
}

PluginError Engine_SAGA_create(OSystem *syst, Engine **engine) {
	assert(syst);
	assert(engine);

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		warning("SagaEngine: invalid game path '%s'", dir.path().c_str());
		return kInvalidPathError;
	}

	// Invoke the detector
	Common::String gameid = ConfMan.get("gameid");
	DetectedGameList detectedGames = Engine_SAGA_detectGames(fslist);

	for (uint i = 0; i < detectedGames.size(); i++) {
		if (detectedGames[i].gameid == gameid) {
			*engine = new Saga::SagaEngine(syst);
			return kNoError;
		}
	}

	warning("SagaEngine: Unable to locate game data at path '%s'", dir.path().c_str());
	return kNoGameDataFoundError;
}

REGISTER_PLUGIN(SAGA, "SAGA Engine", "Inherit the Earth (C) Wyrmkeep Entertainment");

namespace Saga {

using Common::ADGameFileDescription;
using Common::ADGameDescription;

#include "sagagame.cpp"

DetectedGame toDetectedGame(const SAGAGameDescription &g) {
	const char *title;
	title = saga_games[g.gameType].description;
	DetectedGame dg(g.desc.name, title, g.desc.language, g.desc.platform);
	dg.updateDesc(g.desc.extra);
	return dg;
}

bool SagaEngine::initGame() {
	uint16 gameCount = ARRAYSIZE(gameDescriptions);
	int gameNumber = -1;
	
	DetectedGameList detectedGames;
	Common::AdvancedDetector AdvDetector;
	Common::ADList matches;
	Common::ADGameDescList descList;

	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;

	if (ConfMan.hasKey("language"))
		language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		platform = Common::parsePlatform(ConfMan.get("platform"));


	for (int i = 0; i < ARRAYSIZE(gameDescriptions); i++)
		descList.push_back((const ADGameDescription *)&gameDescriptions[i]);

	AdvDetector.registerGameDescriptions(descList);
	AdvDetector.setFileMD5Bytes(FILE_MD5_BYTES);

	matches = AdvDetector.detectGame(NULL, language, platform);

	if (matches.size() == 0) {
		warning("No valid games were found in the specified directory.");
		return false;
	}

	if (matches.size() != 1)
		warning("Conflicting targets detected (%d)", matches.size());

	gameNumber = matches[0];

	//delete matches;

	if (gameNumber >= gameCount || gameNumber == -1) {
		error("SagaEngine::loadGame wrong gameNumber");
	}

	_gameTitle = toDetectedGame(gameDescriptions[gameNumber]).description;
	debug(2, "Running %s", _gameTitle.c_str());

	_gameNumber = gameNumber;
	_gameDescription = &gameDescriptions[gameNumber];
	_gameDisplayInfo = *_gameDescription->gameDisplayInfo;
	_displayClip.right = _gameDisplayInfo.logicalWidth;
	_displayClip.bottom = _gameDisplayInfo.logicalHeight;

	if (!_resource->createContexts()) {
		return false;
	}
	return true;
}

DetectedGameList GAME_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	Common::AdvancedDetector AdvDetector;
	Common::ADList matches;
	Common::ADGameDescList descList;

	for (int i = 0; i < ARRAYSIZE(gameDescriptions); i++)
		descList.push_back((const ADGameDescription *)&gameDescriptions[i]);

	AdvDetector.registerGameDescriptions(descList);
	AdvDetector.setFileMD5Bytes(FILE_MD5_BYTES);

	matches = AdvDetector.detectGame(&fslist, Common::UNK_LANG, Common::kPlatformUnknown);

	for (uint i = 0; i < matches.size(); i++)
		detectedGames.push_back(toDetectedGame(gameDescriptions[matches[i]]));
	//delete matches;

	return detectedGames;
}

} // End of namespace Saga
