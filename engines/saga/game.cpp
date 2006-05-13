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
#include "common/md5.h"
#include "common/hashmap.h"
#include "common/config-manager.h"
#include "base/plugins.h"
#include "backends/fs/fs.h"

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
	assert(engine);
	*engine = new Saga::SagaEngine(syst);
	return kNoError;
}

REGISTER_PLUGIN(SAGA, "SAGA Engine");

namespace Saga {
#include "sagagame.cpp"

DetectedGame toDetectedGame(const GameDescription &g) {
	const char *title;
	title = saga_games[g.gameType].description;
	DetectedGame dg(g.name, title, g.language, g.platform);
	dg.updateDesc(g.extra);
	return dg;
}

static int detectGame(const FSList *fslist, Common::Language language, Common::Platform platform, int*& returnMatches) {
	int gamesCount = ARRAYSIZE(gameDescriptions);
	int filesCount;

	typedef Common::HashMap<Common::String, bool> StringSet;
	StringSet filesList;

	typedef Common::HashMap<Common::String, Common::String> StringMap;
	StringMap filesMD5;

	Common::String tstr;
	
	int i, j;
	char md5str[32+1];
	uint8 md5sum[16];

	int matched[ARRAYSIZE(gameDescriptions)];
	int matchedCount = 0;
	bool fileMissing;
	GameFileDescription *fileDesc;

	// First we compose list of files which we need MD5s for
	for (i = 0; i < gamesCount; i++) {
		for (j = 0; j < gameDescriptions[i].filesCount; j++) {
			tstr = Common::String(gameDescriptions[i].filesDescriptions[j].fileName);
			tstr.toLowercase();
			filesList[tstr] = true;
		}
	}
	
	if (fslist != NULL) {
		for (FSList::const_iterator file = fslist->begin(); file != fslist->end(); ++file) {
			if (file->isDirectory()) continue;
			tstr = file->displayName();
			tstr.toLowercase();

			if (!filesList.contains(tstr)) continue;

			if (!Common::md5_file(file->path().c_str(), md5sum, FILE_MD5_BYTES)) continue;
			for (j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
			filesMD5[tstr] = Common::String(md5str);
		}
	} else {
		Common::File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			tstr = file->_key;
			tstr.toLowercase();

			if(!filesMD5.contains(tstr)) {
				if (testFile.open(file->_key)) {
					testFile.close();

					if (Common::md5_file(file->_key.c_str(), md5sum, FILE_MD5_BYTES)) {
						for (j = 0; j < 16; j++) {
							sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
						}
						filesMD5[tstr] = Common::String(md5str);
					}
				}
			}
		}
	}

	for (i = 0; i < gamesCount; i++) {
		filesCount = gameDescriptions[i].filesCount;		
		fileMissing = false;

		// Try to open all files for this game
		for (j = 0; j < filesCount; j++) {
			fileDesc = &gameDescriptions[i].filesDescriptions[j];
			tstr = fileDesc->fileName;
			tstr.toLowercase();

			if (!filesMD5.contains(tstr)) {

				if ((fileDesc->fileType & (GAME_SOUNDFILE | GAME_VOICEFILE | GAME_MUSICFILE)) != 0) {
					//TODO: find recompressed files
				}
				fileMissing = true;
				break;
			}
			if (strcmp(fileDesc->md5, filesMD5[tstr].c_str())) {
				fileMissing = true;
				break;
			}
		}
		if (!fileMissing) {
			debug(2, "Found game: %s", toDetectedGame(gameDescriptions[i]).description.c_str());
			matched[matchedCount++] = i;
		}
	}

	if (!filesMD5.empty() && (matchedCount == 0)) {
		printf("MD5s of your game version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your game name and version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());
	}

	// We have some resource sets which are superpositions of other
	// Particularly it is ite-demo-linux vs ite-demo-win
	// Now remove lesser set if bigger matches too

	if (matchedCount > 1) {
		// Search max number
		int maxcount = 0;
		for (i = 0; i < matchedCount; i++) {
			maxcount = MAX(gameDescriptions[matched[i]].filesCount, maxcount);
		}

		// Now purge targets with number of files lesser than max
		for (i = 0; i < matchedCount; i++) {
			if ((gameDescriptions[matched[i]].language != language && language != Common::UNK_LANG) ||
				(gameDescriptions[matched[i]].platform != platform && platform != Common::kPlatformUnknown)) {
				debug(2, "Purged %s", toDetectedGame(gameDescriptions[matched[i]]).description.c_str());
				matched[i] = -1;
				continue;
			}

			if (gameDescriptions[matched[i]].filesCount < maxcount) {
				debug(2, "Purged: %s", toDetectedGame(gameDescriptions[matched[i]]).description.c_str());
				matched[i] = -1;
			}
		}
	}


	returnMatches = (int *)malloc(matchedCount * sizeof(int));
	j = 0;
	for (i = 0; i < matchedCount; i++)
		if (matched[i] != -1)
			returnMatches[j++] = matched[i];
	return j;
}

bool SagaEngine::initGame() {
	uint16 gameCount = ARRAYSIZE(gameDescriptions);
	int gameNumber = -1;
	
	DetectedGameList detectedGames;
	int count;
	int* matches;
	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;

	if (ConfMan.hasKey("language"))
		language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		platform = Common::parsePlatform(ConfMan.get("platform"));


	count = detectGame(NULL, language, platform, matches);

	if (count == 0) {
		warning("No valid games were found in the specified directory.");
		return false;
	}

	if (count != 1)
		warning("Conflicting targets detected (%d)", count);

	gameNumber = matches[0];

	free(matches);

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
	int count;
	int* matches;
	count = detectGame(&fslist, Common::UNK_LANG, Common::kPlatformUnknown, matches);

	for (int i = 0; i < count; i++)
		detectedGames.push_back(toDetectedGame(gameDescriptions[matches[i]]));
	free(matches);
	return detectedGames;
}

} // End of namespace Saga
