/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"

#include "agos/agos.h"

namespace AGOS {
static DetectedGameList GAME_detectGames(const FSList &fslist);
}

using Common::File;

struct ObsoleteGameID {
	const char *from;
	const char *to;
	Common::Platform platform;
};

/**
 * Conversion table mapping old obsolete target names to the
 * corresponding new target and platform combination.
 *
 */
static const ObsoleteGameID obsoleteGameIDsTable[] = {
	{"simon1acorn", "simon1", Common::kPlatformAcorn},
	{"simon1amiga", "simon1", Common::kPlatformAmiga},
	{"simon1cd32", "simon1", Common::kPlatformAmiga},
	{"simon1demo", "simon1", Common::kPlatformPC},
	{"simon1dos", "simon1", Common::kPlatformPC},
	{"simon1talkie", "simon1", Common::kPlatformPC},
	{"simon1win", "simon1", Common::kPlatformWindows},
	{"simon2dos", "simon2",  Common::kPlatformPC},
	{"simon2talkie", "simon2", Common::kPlatformPC},
	{"simon2mac", "simon2", Common::kPlatformMacintosh},
	{"simon2win", "simon2",  Common::kPlatformWindows},
	{NULL, NULL, Common::kPlatformUnknown}
};

static const PlainGameDescriptor simonGames[] = {
	{"elvira1", "Elvira - Mistress of the Dark"},
	{"elvira2", "Elvira II - The Jaws of Cerberus"},
	{"waxworks", "Waxworks"},
	{"simon1", "Simon the Sorcerer 1"},
	{"simon2", "Simon the Sorcerer 2"},
	{"feeble", "The Feeble Files"},
	{"dimp", "Demon in my Pocket"},
	{"jumble", "Jumble"},
	{"puzzle", "NoPatience"},
	{"swampy", "Swampy Adventures"},
	{NULL, NULL}
};

GameList Engine_AGOS_gameIDList() {
	GameList games;
	const PlainGameDescriptor *g = simonGames;
	while (g->gameid) {
		games.push_back(*g);
		g++;
	}

	return games;
}

GameDescriptor Engine_AGOS_findGameID(const char *gameid) {
	// First search the list of supported game IDs.
	const PlainGameDescriptor *g = simonGames;
	while (g->gameid) {
		if (!scumm_stricmp(gameid, g->gameid))
			return *g;
		g++;
	}

	// If we didn't find the gameid in the main list, check if it
	// is an obsolete game id.
	GameDescriptor gs;
	const ObsoleteGameID *o = obsoleteGameIDsTable;
	while (o->from) {
		if (0 == scumm_stricmp(gameid, o->from)) {
			gs.gameid = gameid;
			gs.description = "Obsolete game ID";
			return gs;
		}
		o++;
	}
	return gs;
}

DetectedGameList Engine_AGOS_detectGames(const FSList &fslist) {
	return AGOS::GAME_detectGames(fslist);
}

PluginError Engine_AGOS_create(OSystem *syst, Engine **engine) {
	assert(syst);
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();

	for (const ObsoleteGameID *o = obsoleteGameIDsTable; o->from; ++o) {
		if (!scumm_stricmp(gameid, o->from)) {
			// Match found, perform upgrade
			gameid = o->to;
			ConfMan.set("gameid", o->to);

			if (o->platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(o->platform));

			warning("Target upgraded from %s to %s", o->from, o->to);
			ConfMan.flushToDisk();
			break;
		}
	}

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		warning("AGOSEngine: invalid game path '%s'", dir.path().c_str());
		return kInvalidPathError;
	}

	// Invoke the detector
	DetectedGameList detectedGames = Engine_AGOS_detectGames(fslist);

	for (uint i = 0; i < detectedGames.size(); i++) {
		if (detectedGames[i].gameid == gameid) {
			*engine = new AGOS::AGOSEngine(syst);
			return kNoError;
		}
	}

	warning("AGOSEngine: Unable to locate game data at path '%s'", dir.path().c_str());
	return kNoGameDataFoundError;
}

REGISTER_PLUGIN(AGOS, "AGOS", "AGOS (C) Adventure Soft");

namespace AGOS {

using Common::ADGameFileDescription;
using Common::ADGameDescription;

#include "agosgame.cpp"

DetectedGame toDetectedGame(const ADGameDescription &g) {
	const char *title = 0;

	const PlainGameDescriptor *sg = simonGames;
	while (sg->gameid) {
		if (!scumm_stricmp(g.name, sg->gameid))
			title = sg->description;
		sg++;
	}

	DetectedGame dg(g.name, title, g.language, g.platform);
	dg.updateDesc(g.extra);
	return dg;
}

bool AGOSEngine::initGame() {
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

	Common::String gameid = ConfMan.get("gameid");

	// At this point, Engine_AGOS_create() has already verified that the
	// desired game is in the specified directory. But we've already
	// forgotten which particular version it was, so we have to do it all
	// over again...

	for (int i = 0; i < ARRAYSIZE(gameDescriptions); i++)
		descList.push_back((ADGameDescription *)&gameDescriptions[i]);

	AdvDetector.registerGameDescriptions(descList);
	AdvDetector.setFileMD5Bytes(FILE_MD5_BYTES);

	matches = AdvDetector.detectGame(NULL, language, platform);

	for (uint i = 0; i < matches.size(); i++) {
		if (toDetectedGame(gameDescriptions[matches[i]].desc).gameid == gameid) {
			gameNumber = matches[i];
			break;
		}
	}

	//delete &matches;

	if (gameNumber >= ARRAYSIZE(gameDescriptions) || gameNumber == -1) {
		error("AGOSEngine::loadGame wrong gameNumber");
	}

	debug(2, "Running %s", toDetectedGame(gameDescriptions[gameNumber].desc).description.c_str());

	_gameDescription = &gameDescriptions[gameNumber];

	return true;
}

DetectedGameList GAME_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	Common::AdvancedDetector AdvDetector;
	Common::ADList matches;
	Common::ADGameDescList descList;

	for (int i = 0; i < ARRAYSIZE(gameDescriptions); i++)
		descList.push_back((ADGameDescription *)&gameDescriptions[i]);

	AdvDetector.registerGameDescriptions(descList);
	AdvDetector.setFileMD5Bytes(FILE_MD5_BYTES);

	matches = AdvDetector.detectGame(&fslist, Common::UNK_LANG, Common::kPlatformUnknown);

	for (uint i = 0; i < matches.size(); i++)
		detectedGames.push_back(toDetectedGame(gameDescriptions[matches[i]].desc));
	
	//delete &matches;
	return detectedGames;
}

} // End of namespace AGOS
