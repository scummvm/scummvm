/* ScummVM - Scumm Interpreter
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

#include "backends/fs/fs.h"

#include "base/gameDetector.h"
#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/file.h"
#include "common/md5.h"

#include "simon/simon.h"
#include "simon/intern.h"


namespace Simon {
static DetectedGameList GAME_ProbeGame(const FSList &fslist, int **matches = NULL);
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
	// Simon the Sorcerer 1 & 2
	{"feeble", "The Feeble Files"},
	{"simon1", "Simon the Sorcerer 1"},
	{"simon2", "Simon the Sorcerer 2"},

	{NULL, NULL}
};

static const char *findDescriptionFromGameID(const char *gameid) {
	const PlainGameDescriptor *g = simonGames;
	while (g->gameid) {
		if (!scumm_stricmp(g->gameid, gameid)) {
			return g->description;
		}
		g++;
	}
	error("Unknown gameid encountered in findDescriptionFromGameID");
}

GameList Engine_SIMON_gameIDList() {
	GameList games;
	const PlainGameDescriptor *g = simonGames;
	while (g->gameid) {
		games.push_back(*g);
		g++;
	}

	return games;
}

GameDescriptor Engine_SIMON_findGameID(const char *gameid) {
	// First search the list of supported game IDs.
	const PlainGameDescriptor *g = simonGames;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
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

DetectedGameList Engine_SIMON_detectGames(const FSList &fslist) {
	return Simon::GAME_ProbeGame(fslist);
}

Engine *Engine_SIMON_create(GameDetector *detector, OSystem *syst) {
	const ObsoleteGameID *o = obsoleteGameIDsTable;
	while (o->from) {
		if (!scumm_stricmp(detector->_gameid.c_str(), o->from)) {
			detector->_gameid = o->to;

			ConfMan.set("gameid", o->to);

			if (o->platform != Common::kPlatformUnknown)
				ConfMan.set("platform", Common::getPlatformCode(o->platform));

			warning("Target upgraded from %s to %s", o->from, o->to);
			ConfMan.flushToDisk();
			break;
		}
		o++;
	}

	return new Simon::SimonEngine(syst);
}

REGISTER_PLUGIN(SIMON, "Simon the Sorcerer")

namespace Simon {

static int detectGame(const FSList &fslist, bool mode = false, int start = -1);

struct GameMD5 {
	GameIds id;
	const char *md5;
	const char *filename;
	bool caseSensitive;
};

#define FILE_MD5_BYTES 5000

static GameMD5 gameMD5[] = {
	{ GID_SIMON1ACORNDEMO, "b4a7526ced425ba8ad0d548d0ec69900", "data", false },
	{ GID_SIMON1ACORNDEMO, "425c7d1957699d35abca7e12a08c7422", "gamebase", false },
	{ GID_SIMON1ACORNDEMO, "22107c24dfb31b66ac503c28a6e20b19", "icondata", false},
	{ GID_SIMON1ACORNDEMO, "d9de7542612d9f4e0819ad0df5eac56b", "stripped", false},
	{ GID_SIMON1ACORNDEMO, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1ACORN,     "64958b3a38afdcb85da1eeed85169806", "data", false },
	{ GID_SIMON1ACORN,     "28261b99cd9da1242189b4f6f2841bd6", "gamebase", false },
	{ GID_SIMON1ACORN,     "22107c24dfb31b66ac503c28a6e20b19", "icondata", false},
	{ GID_SIMON1ACORN,     "f3b27a3fbb45dcd323a48159496e45e8", "stripped", false},
	{ GID_SIMON1ACORN,     "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1AMIGA,     "6c9ad2ff571d34a4cf0c696cf4e13500", "gameamiga", true },
	{ GID_SIMON1AMIGA,     "565ef7a98dcc21ef526a2bb10b6f42ed", "icon.pkd", true },
	{ GID_SIMON1AMIGA,     "c649fcc0439766810e5097ee7e81d4c8", "stripped.txt", true},
	{ GID_SIMON1AMIGA,     "f9d5bf2ce09f82289c791c3ca26e1e4b", "tbllist", true},

	{ GID_SIMON1AMIGA_FR,  "bd9828b9d4e5d89b50fe8c47a8e6bc07", "gameamiga", true },
	{ GID_SIMON1AMIGA_FR,  "565ef7a98dcc21ef526a2bb10b6f42ed", "icon.pkd", true },
	{ GID_SIMON1AMIGA_FR,  "2297baec985617d0d5612a0124bac359", "stripped.txt", true},
	{ GID_SIMON1AMIGA_FR,  "f9d5bf2ce09f82289c791c3ca26e1e4b", "tbllist", true},

	{ GID_SIMON1AMIGA_DE,  "a2de9553f3b73064369948b5af38bb30", "gameamiga", true },
	{ GID_SIMON1AMIGA_DE,  "565ef7a98dcc21ef526a2bb10b6f42ed", "icon.pkd", true },
	{ GID_SIMON1AMIGA_DE,  "c649fcc0439766810e5097ee7e81d4c8", "stripped.txt", true},
	{ GID_SIMON1AMIGA_DE,  "f9d5bf2ce09f82289c791c3ca26e1e4b", "tbllist", true},

	{ GID_SIMON1AMIGADEMO, "a12b696170f14eca5ff75f1549829251", "gameamiga", true },  // Unpacked version
	{ GID_SIMON1AMIGADEMO, "ebc96af15bfaf75ba8210326b9260d2f", "icon.pkd", true },
	{ GID_SIMON1AMIGADEMO, "8edde5b9498dc9f31da1093028da467c", "stripped.txt", true},
	{ GID_SIMON1AMIGADEMO, "1247e024e1f13ca54c1e354120c7519c", "tbllist", true},

	{ GID_SIMON1CD32,      "bab7f19237cf7d7619b6c73631da1854", "gameamiga", true },
	{ GID_SIMON1CD32,      "565ef7a98dcc21ef526a2bb10b6f42ed", "icon.pkd", true },
	{ GID_SIMON1CD32,      "59be788020441e21861e284236fd08c1", "stripped.txt", true},
	{ GID_SIMON1CD32,      "f9d5bf2ce09f82289c791c3ca26e1e4b", "tbllist", true},

	{ GID_SIMON1CD32_2,    "ec5358680c117f29b128cbbb322111a4", "gameamiga", true },
	{ GID_SIMON1CD32_2,    "8ce5a46466a4f8f6d0f780b0ef00d5f5", "icon.pkd", true },
	{ GID_SIMON1CD32_2,    "59be788020441e21861e284236fd08c1", "stripped.txt", true},
	{ GID_SIMON1CD32_2,    "f9d5bf2ce09f82289c791c3ca26e1e4b", "tbllist", true},

	{ GID_SIMON1DOS_INF,  "9f93d27432ce44a787eef10adb640870", "gamepc", false },
	{ GID_SIMON1DOS_INF,  "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS_INF,  "2af9affc5981eec44b90d4c556145cb8", "stripped.txt", false},
	{ GID_SIMON1DOS_INF,  "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DOS_INF_RU,"605fb866e03ec1c41b10c6a518ddfa49", "gamepc", false },
	{ GID_SIMON1DOS_INF_RU,"22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS_INF_RU,"2af9affc5981eec44b90d4c556145cb8", "stripped.txt", false},
	{ GID_SIMON1DOS_INF_RU,"d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DOS,       "c392e494dcabed797b98cbcfc687b33a", "gamepc", false },
	{ GID_SIMON1DOS,       "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS,       "c95a0a1ee973e19c2a1c5d12026c139f", "stripped.txt", false},
	{ GID_SIMON1DOS,       "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DOS_RU,    "605fb866e03ec1c41b10c6a518ddfa49", "gamepc", false },
	{ GID_SIMON1DOS_RU,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS_RU,    "c95a0a1ee973e19c2a1c5d12026c139f", "stripped.txt", false},
	{ GID_SIMON1DOS_RU,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DOS_FR,    "34759d0d4285a2f4b21b8e03b8fcefb3", "gamepc", false },
	{ GID_SIMON1DOS_FR,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS_FR,    "aa01e7386057abc0c3e27dbaa9c4ba5b", "stripped.txt", false},
	{ GID_SIMON1DOS_FR,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DOS_DE,    "063015e6ce7d90b570dbc21fe0c667b1", "gamepc", false },
	{ GID_SIMON1DOS_DE,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS_DE,    "c95a0a1ee973e19c2a1c5d12026c139f", "stripped.txt", false},
	{ GID_SIMON1DOS_DE,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DOS_IT,    "65c9b2dea57df84ef55d1eaf384ebd30", "gamepc", false },
	{ GID_SIMON1DOS_IT,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS_IT,    "2af9affc5981eec44b90d4c556145cb8", "stripped.txt", false},
	{ GID_SIMON1DOS_IT,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DOS_ES,    "5374fafdea2068134f33deab225feed3", "gamepc", false },
	{ GID_SIMON1DOS_ES,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1DOS_ES,    "2af9affc5981eec44b90d4c556145cb8", "stripped.txt", false},
	{ GID_SIMON1DOS_ES,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1DEMO,      "2be4a21bc76e2fdc071867c130651439", "gdemo", false },
	{ GID_SIMON1DEMO,      "55af3b4d93972bc58bfee38a86b76c3f", "icon.dat", false},
	{ GID_SIMON1DEMO,      "33a2e329b97b2a349858d6a093159eb7", "stripped.txt", false},
	{ GID_SIMON1DEMO,      "1247e024e1f13ca54c1e354120c7519c", "tbllist", false},

	{ GID_SIMON1TALKIE,    "28261b99cd9da1242189b4f6f2841bd6", "gamepc", false },
	{ GID_SIMON1TALKIE,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE,    "64958b3a38afdcb85da1eeed85169806", "simon.gme", false },
	{ GID_SIMON1TALKIE,    "f3b27a3fbb45dcd323a48159496e45e8", "stripped.txt", false},
	{ GID_SIMON1TALKIE,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1TALKIE2,    "c0b948b6821d2140f8b977144f21027a", "gamepc", false },
	{ GID_SIMON1TALKIE2,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE2,    "64f73e94639b63af846ac4a8a94a23d8", "simon.gme", false },
	{ GID_SIMON1TALKIE2,    "f3b27a3fbb45dcd323a48159496e45e8", "stripped.txt", false},
	{ GID_SIMON1TALKIE2,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1TALKIE_FR, "3cfb9d1ff4ec725af9924140126cf69f", "gamepc", false },
	{ GID_SIMON1TALKIE_FR, "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE_FR, "638049fa5d41b81fb6fb11671721b871", "simon.gme", false },
	{ GID_SIMON1TALKIE_FR, "ef51ac74c946881ae4d7ca66cc7a0d1e", "stripped.txt", false},
	{ GID_SIMON1TALKIE_FR, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1TALKIE_DE, "48b1f3499e2e0d731047f4d481ff7817", "gamepc", false },
	{ GID_SIMON1TALKIE_DE, "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE_DE, "7db9912acac4f1d965a64bdcfc370ba1", "simon.gme", false },
	{ GID_SIMON1TALKIE_DE, "40d68bec54042ef930f084ad9a4342a1", "stripped.txt", false},
	{ GID_SIMON1TALKIE_DE, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1TALKIE_HB, "bc66e9c0b296e1b155a246917133f71a", "gamepc", false },
	{ GID_SIMON1TALKIE_HB, "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE_HB, "a34b2c8642f2e3676d7088b5c8b3e884", "simon.gme", false },
	{ GID_SIMON1TALKIE_HB, "9d31bef42db1a8abe4e9f368014df1d5", "stripped.txt", false},
	{ GID_SIMON1TALKIE_HB, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1TALKIE_IT, "8d3ca654e158c91b860c7eae31d65312", "gamepc", false },
	{ GID_SIMON1TALKIE_IT, "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE_IT, "104efd83c8f3edf545982e07d87f66ac", "simon.gme", false },
	{ GID_SIMON1TALKIE_IT, "9d31bef42db1a8abe4e9f368014df1d5", "stripped.txt", false},
	{ GID_SIMON1TALKIE_IT, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1TALKIE_ES, "439f801ba52c02c9d1844600d1ce0f5e", "gamepc", false },
	{ GID_SIMON1TALKIE_ES, "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1TALKIE_ES, "eff2774a73890b9eac533db90cd1afa1", "simon.gme", false },
	{ GID_SIMON1TALKIE_ES, "9d31bef42db1a8abe4e9f368014df1d5", "stripped.txt", false},
	{ GID_SIMON1TALKIE_ES, "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1WIN,       "c7c12fea7f6d0bfd22af5cdbc8166862", "gamepc", false },
	{ GID_SIMON1WIN,       "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1WIN,       "b1b18d0731b64c0738c5cc4a2ee792fc", "simon.gme", false },
	{ GID_SIMON1WIN,       "a27e87a9ba21212d769804b3df47bfb2", "stripped.txt", false},
	{ GID_SIMON1WIN,       "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON1WIN_DE,    "48b1f3499e2e0d731047f4d481ff7817", "gamepc", false },
	{ GID_SIMON1WIN_DE,    "22107c24dfb31b66ac503c28a6e20b19", "icon.dat", false},
	{ GID_SIMON1WIN_DE,    "acd9cc438525b142d93b15c77a6f551b", "simon.gme", false },
	{ GID_SIMON1WIN_DE,    "40d68bec54042ef930f084ad9a4342a1", "stripped.txt", false},
	{ GID_SIMON1WIN_DE,    "d198a80de2c59e4a0cd24b98814849e8", "tbllist", false},

	{ GID_SIMON2DOS,       "27c8e7feada80c75b70b9c2f6088d519", "game32", false },
	{ GID_SIMON2DOS,       "ee92d1f84893195a60449f2430d07285", "icon.dat", false},
	{ GID_SIMON2DOS,       "eefcc32b1f2c0482c1a59a963a146345", "simon2.gme", false},
	{ GID_SIMON2DOS,       "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2DOS,       "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2DOS_RU,    "7edfc633dd50f8caa719c478443db70b", "game32", false },
	{ GID_SIMON2DOS_RU,    "ee92d1f84893195a60449f2430d07285", "icon.dat", false},
	{ GID_SIMON2DOS_RU,    "eefcc32b1f2c0482c1a59a963a146345", "simon2.gme", false},
	{ GID_SIMON2DOS_RU,    "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2DOS_RU,    "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2DOS2,      "604d04315935e77624bd356ac926e068", "game32", false },
	{ GID_SIMON2DOS2,      "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2DOS2,      "aa6840420899a31874204f90bb214108", "simon2.gme", false},
	{ GID_SIMON2DOS2,      "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2DOS2,      "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2DOS2_RU,   "eb8bde3685842a8fd38f60bc476ef8e9", "game32", false },
	{ GID_SIMON2DOS2_RU,   "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2DOS2_RU,   "aa6840420899a31874204f90bb214108", "simon2.gme", false},
	{ GID_SIMON2DOS2_RU,   "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2DOS2_RU,   "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2DOS_DE,    "eb6e3e37fe52993f948d7e2d6b869828", "game32", false },
	{ GID_SIMON2DOS_DE,    "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2DOS_DE,    "5fa9d080b04c610f526bd685be1bf747", "simon2.gme", false},
	{ GID_SIMON2DOS_DE,    "fd30df01cc248ecbaef302af855e0212", "stripped.txt", false},
	{ GID_SIMON2DOS_DE,    "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2DOS_IT,    "3e11d400bea0638f360a724687005cd1", "game32", false },
	{ GID_SIMON2DOS_IT,    "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2DOS_IT,    "f306a397565d7f13bec7ecf14c723de7", "simon2.gme", false},
	{ GID_SIMON2DOS_IT,    "bea6843fb9f3b2144fcb146d62db0b9a", "stripped.txt", false},
	{ GID_SIMON2DOS_IT,    "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2DEMO,      "3794c15887539b8578bacab694ccf08a", "gsptr30", false },
	{ GID_SIMON2DEMO,      "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2DEMO,      "f8c9e6df1e55923a749e115ba74210c4", "simon2.gme", false},
	{ GID_SIMON2DEMO,      "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2DEMO,      "a0d5a494b5d3d209d1a1d76cc8d76601", "tbllist", false},

	{ GID_SIMON2TALKIE,    "8c301fb9c4fcf119d2730ccd2a565eb3", "gsptr30", false },
	{ GID_SIMON2TALKIE,    "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE,    "9c535d403966750ae98bdaf698375a38", "simon2.gme", false },
	{ GID_SIMON2TALKIE,    "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2TALKIE,    "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE2,   "608e277904d87dd28725fa08eacc2c0d", "gsptr30", false },
	{ GID_SIMON2TALKIE2,   "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE2,   "8d6dcc65577e285dbca03ff6d7d9323c", "simon2.gme", false },
	{ GID_SIMON2TALKIE2,   "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2TALKIE2,   "a0d5a494b5d3d209d1a1d76cc8d76601", "tbllist", false},

	{ GID_SIMON2TALKIE_FR, "43b3a04d2f0a0cbd1b024c814856561a", "gsptr30", false },
	{ GID_SIMON2TALKIE_FR, "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE_FR, "8af0e02c0c3344db64dffc12196eb59d", "simon2.gme", false },
	{ GID_SIMON2TALKIE_FR, "5ea27977b4d7dcfd50eb5074e162ebbf", "stripped.txt", false},
	{ GID_SIMON2TALKIE_FR, "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE_DE, "0d05c3f4c06c9a4ceb3d2f5bc0b18e11", "gsptr30", false },
	{ GID_SIMON2TALKIE_DE, "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE_DE, "6c5fdfdd0eab9038767c2d22858406b2", "simon2.gme", false },
	{ GID_SIMON2TALKIE_DE, "6de6292c9ac11bfb2e70fdb0f773ba85", "stripped.txt", false},
	{ GID_SIMON2TALKIE_DE, "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE_DE2,"a76ea940076b5d9316796dea225a9b69", "gsptr30", false },
	{ GID_SIMON2TALKIE_DE2,"72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE_DE2,"ec9f0f24fd895e7ea72e3c8e448c0240", "simon2.gme", false },
	{ GID_SIMON2TALKIE_DE2,"6de6292c9ac11bfb2e70fdb0f773ba85", "stripped.txt", false},
	{ GID_SIMON2TALKIE_DE2,"2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE_HB, "952a2b1be23c3c609ba8d988a9a1627d", "gsptr30", false },
	{ GID_SIMON2TALKIE_HB, "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE_HB, "a2b249a82ea182af09789eb95fb6c5be", "simon2.gme", false },
	{ GID_SIMON2TALKIE_HB, "de9dbc24158660e153483fa0cf6c3172", "stripped.txt", false},
	{ GID_SIMON2TALKIE_HB, "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE_IT, "3e11d400bea0638f360a724687005cd1", "gsptr30", false },
	{ GID_SIMON2TALKIE_IT, "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE_IT, "344aca58e5ad5e25c517d5eb1d85c435", "simon2.gme", false },
	{ GID_SIMON2TALKIE_IT, "bea6843fb9f3b2144fcb146d62db0b9a", "stripped.txt", false},
	{ GID_SIMON2TALKIE_IT, "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2TALKIE_ES, "268dc322aa73bcf27bb016b8e8ceb889", "gsptr30", false },
	{ GID_SIMON2TALKIE_ES, "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2TALKIE_ES, "4f43bd06b6cc78dbd25a7475ca964eb1", "simon2.gme", false },
	{ GID_SIMON2TALKIE_ES, "d13753796bd81bf313a2449f34d8b112", "stripped.txt", false},
	{ GID_SIMON2TALKIE_ES, "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2WIN,       "608e277904d87dd28725fa08eacc2c0d", "gsptr30", false },
	{ GID_SIMON2WIN,       "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2WIN,       "e749c4c103d7e7d51b34620ed76c5a04", "simon2.gme", false },
	{ GID_SIMON2WIN,       "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2WIN,       "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2WIN_DE,    "a76ea940076b5d9316796dea225a9b69", "gsptr30", false },
	{ GID_SIMON2WIN_DE,    "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2WIN_DE,    "9609a933c541fed2e00c6c3479d7c181", "simon2.gme", false },
	{ GID_SIMON2WIN_DE,    "6de6292c9ac11bfb2e70fdb0f773ba85", "stripped.txt", false},
	{ GID_SIMON2WIN_DE,    "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2WIN_DE2,   "9e858b3bb189c134c3a5f34c3385a8d3", "gsptr30", false },
	{ GID_SIMON2WIN_DE2,   "ee92d1f84893195a60449f2430d07285", "icon.dat", false},
	{ GID_SIMON2WIN_DE2,   "16d574da07e93bcae43cee353dab8c7e", "simon2.gme", false },
	{ GID_SIMON2WIN_DE2,   "6de6292c9ac11bfb2e70fdb0f773ba85", "stripped.txt", false},
	{ GID_SIMON2WIN_DE2,   "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_SIMON2WIN_PL,    "657fd873f5d0637097ee02315b447e6f", "gsptr30", false },
	{ GID_SIMON2WIN_PL,    "72096a62d36e6034ea9fecc13b2dbdab", "icon.dat", false},
	{ GID_SIMON2WIN_PL,    "7b9afcf82a94722707e0d025c0192be8", "simon2.gme", false },
	{ GID_SIMON2WIN_PL,    "e229f84d46fa83f99b4a7115679f3fb6", "stripped.txt", false},
	{ GID_SIMON2WIN_PL,    "2082f8d02075e590300478853a91ffd9", "tbllist", false},

	{ GID_FEEBLEFILES_AMI_DE, "bcd76ac080003eee3649df18db25b60e", "game22", false },
	{ GID_FEEBLEFILES_AMI_DE, "f550f7915c5ce3a68c9f870f507449c2", "gfxindex.dat", false},
	{ GID_FEEBLEFILES_AMI_DE, "bdb4cf3eacfeffc3aed4f90bf87c5690", "sfxindex.dat", false},
	{ GID_FEEBLEFILES_AMI_DE, "e0f9e91213e4b6f684e2a9a67d7a4660", "spindex.dat", false},
	{ GID_FEEBLEFILES_AMI_DE, "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},

	{ GID_FEEBLEFILES_AMI_UK, "629762ea9ca9ee9ff85f4774d219f5c7", "game22", false },
	{ GID_FEEBLEFILES_AMI_UK, "f550f7915c5ce3a68c9f870f507449c2", "gfxindex.dat", false},
	{ GID_FEEBLEFILES_AMI_UK, "bdb4cf3eacfeffc3aed4f90bf87c5690", "sfxindex.dat", false},
	{ GID_FEEBLEFILES_AMI_UK, "e0f9e91213e4b6f684e2a9a67d7a4660", "spindex.dat", false},
	{ GID_FEEBLEFILES_AMI_UK, "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},

	{ GID_FEEBLEFILES_MAC_DE, "bcd76ac080003eee3649df18db25b60e", "game22", false },
	{ GID_FEEBLEFILES_MAC_DE, "11a4853cb35956846976e9473ee0e41e", "graphics.vga", false},
	{ GID_FEEBLEFILES_MAC_DE, "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},

	{ GID_FEEBLEFILES_MAC_FR, "ba90b40a47726039671d9e91630dd7ed", "game22", false },
	{ GID_FEEBLEFILES_MAC_FR, "11a4853cb35956846976e9473ee0e41e", "graphics.vga", false},
	{ GID_FEEBLEFILES_MAC_FR, "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},

	{ GID_FEEBLEFILES_MAC_SP, "71d7d2d5e479b053c5a9757f1702c9c3", "game22", false },
	{ GID_FEEBLEFILES_MAC_SP, "11a4853cb35956846976e9473ee0e41e", "graphics.vga", false},
	{ GID_FEEBLEFILES_MAC_SP, "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},

	{ GID_FEEBLEFILES_MAC_UK, "629762ea9ca9ee9ff85f4774d219f5c7", "game22", false },
	{ GID_FEEBLEFILES_MAC_UK, "11a4853cb35956846976e9473ee0e41e", "graphics.vga", false},
	{ GID_FEEBLEFILES_MAC_UK, "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},

	{ GID_FEEBLEFILES_2CD,    "629762ea9ca9ee9ff85f4774d219f5c7", "game22", false },
	{ GID_FEEBLEFILES_2CD,    "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},

	{ GID_FEEBLEFILES_4CD,    "a8746407a5b20a7da0da0a14c380af1c", "game22", false },
	{ GID_FEEBLEFILES_4CD,    "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},

	{ GID_FEEBLEFILES_DE,     "bcd76ac080003eee3649df18db25b60e", "game22", false },
	{ GID_FEEBLEFILES_DE,     "0bbfee8e69739111eb36b0d138da8ddf", "tbllist", false},
};

// Simon the Sorcerer 1
static GameFileDescription SIMON1CD32_GameFiles[] = {
	{"gameamiga", GAME_BASEFILE},
	{"icon.pkd", GAME_ICONFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON1ACORN_GameFiles[] = {
	{"data", GAME_GMEFILE},
	{"gamebase", GAME_BASEFILE},
	{"icondata", GAME_ICONFILE},
	{"stripped", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON1DEMO_GameFiles[] = {
	{"gdemo", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON1DOS_GameFiles[] = {
	{"gamepc", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON1_GameFiles[] = {
	{"gamepc", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"simon.gme", GAME_GMEFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

// Simon the Sorcerer 2
static GameFileDescription SIMON2DOS_GameFiles[] = {
	{"game32", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"simon2.gme", GAME_GMEFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription SIMON2_GameFiles[] = {
	{"gsptr30", GAME_BASEFILE},
	{"icon.dat", GAME_ICONFILE},
	{"simon2.gme", GAME_GMEFILE},
	{"stripped.txt", GAME_STRFILE},
	{"tbllist", GAME_TBLFILE},
};

// The Feeble Files
static GameFileDescription FEEBLEFILES_GameFiles[] = {
	{"game22", GAME_BASEFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription FEEBLEFILES_AMI_GameFiles[] = {
	{"game22", GAME_BASEFILE},
	{"gfxindex.dat", GAME_GFXIDXFILE},
	{"sfxindex.dat", GAME_SFXIDXFILE},
	{"spindex.dat", GAME_SPIDXFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameFileDescription FEEBLEFILES_MAC_GameFiles[] = {
	{"game22", GAME_BASEFILE},
	{"graphics.vga", GAME_GFXIDXFILE},
	{"tbllist", GAME_TBLFILE},
};

static GameDescription gameDescriptions[] = {
	// Simon the Sorcerer 1 - English Acorn CD Demo
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1ACORNDEMO,
		"CD Demo",
		ARRAYSIZE(SIMON1ACORN_GameFiles),
		SIMON1ACORN_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformAcorn,
	},

	// Simon the Sorcerer 1 - English Acorn CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1ACORN,
		"CD",
		ARRAYSIZE(SIMON1ACORN_GameFiles),
		SIMON1ACORN_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformAcorn,
	},

	// Simon the Sorcerer 1 - English AGA Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1AMIGA,
		"AGA Floppy",
		ARRAYSIZE(SIMON1CD32_GameFiles),
		SIMON1CD32_GameFiles,
		GF_CRUNCHED | GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - French AGA Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1AMIGA_FR,
		"AGA Floppy",
		ARRAYSIZE(SIMON1CD32_GameFiles),
		SIMON1CD32_GameFiles,
		GF_CRUNCHED | GF_OLD_BUNDLE,
		Common::FR_FRA,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - German AGA Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1AMIGA_DE,
		"AGA Floppy",
		ARRAYSIZE(SIMON1CD32_GameFiles),
		SIMON1CD32_GameFiles,
		GF_CRUNCHED | GF_OLD_BUNDLE,
		Common::DE_DEU,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - English Amiga ECS Demo
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1AMIGADEMO,
		"ECS Demo",
		ARRAYSIZE(SIMON1CD32_GameFiles),
		SIMON1CD32_GameFiles,
		GF_CRUNCHED | GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - English Amiga CD32
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1CD32,
		"CD32",
		ARRAYSIZE(SIMON1CD32_GameFiles),
		SIMON1CD32_GameFiles,
		GF_TALKIE | GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - English Amiga CD32 alternative?
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1CD32_2,
		"CD32",
		ARRAYSIZE(SIMON1CD32_GameFiles),
		SIMON1CD32_GameFiles,
		GF_TALKIE | GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - English DOS Floppy Demo
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DEMO,
		"Floppy Demo",
		ARRAYSIZE(SIMON1DEMO_GameFiles),
		SIMON1DEMO_GameFiles,
		GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS Floppy with Russian patch
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_RU,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::RU_RUS,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom)
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_INF,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom) with Russian patch
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_INF_RU,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::RU_RUS,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - French DOS Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_FR,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::FR_FRA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - German DOS Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_DE,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - Italian DOS Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_IT,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::IT_ITA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - Spanish DOS Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_ES,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_GameFiles),
		SIMON1DOS_GameFiles,
		GF_OLD_BUNDLE,
		Common::ES_ESP,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS CD alternate?
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE2,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - French DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_FR,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::FR_FRA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - German DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_DE,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - Hebrew DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_HB,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::HB_ISR,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - Italian DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_IT,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::IT_ITA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - Spanish DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_ES,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::ES_ESP,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English Windows CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1WIN,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 1 - German Windows CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1WIN_DE,
		"CD",
		ARRAYSIZE(SIMON1_GameFiles),
		SIMON1_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 2 - English DOS Floppy
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS,
		"Floppy",
		ARRAYSIZE(SIMON2DOS_GameFiles),
		SIMON2DOS_GameFiles,
		0,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS Floppy with Russian patch
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS_RU,
		"Floppy",
		ARRAYSIZE(SIMON2DOS_GameFiles),
		SIMON2DOS_GameFiles,
		0,
		Common::RU_RUS,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS Floppy alternate?
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS2,
		"Floppy",
		ARRAYSIZE(SIMON2DOS_GameFiles),
		SIMON2DOS_GameFiles,
		0,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS Floppy alternate? with Russian patch
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS2_RU,
		"Floppy",
		ARRAYSIZE(SIMON2DOS_GameFiles),
		SIMON2DOS_GameFiles,
		0,
		Common::RU_RUS,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - German DOS Floppy
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS_DE,
		"Floppy",
		ARRAYSIZE(SIMON2DOS_GameFiles),
		SIMON2DOS_GameFiles,
		0,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - Italian DOS Floppy
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS_IT,
		"Floppy",
		ARRAYSIZE(SIMON2DOS_GameFiles),
		SIMON2DOS_GameFiles,
		0,
		Common::IT_ITA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS CD Demo
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DEMO,
		"CD Demo",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformPC,
	},


	// Simon the Sorcerer 2 - English DOS CD alternate?
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE2,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - French DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_FR,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::FR_FRA,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - German DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_DE,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - German DOS CD alternate?
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_DE2,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - Hebrew DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_HB,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::HB_ISR,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - Italian DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_IT,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::IT_ITA,
		// FIXME: DOS version which uses WAV format
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 2 - Spanish DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_ES,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::ES_ESP,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English Windows CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2WIN,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 2 - German Windows CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2WIN_DE,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 2 - German Windows CD 1.1
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2WIN_DE2,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 2 - Polish Windows CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2WIN_PL,
		"CD",
		ARRAYSIZE(SIMON2_GameFiles),
		SIMON2_GameFiles,
		GF_TALKIE,
		Common::PL_POL,
		Common::kPlatformWindows,
	},

	// The Feeble Files - English Amiga CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_AMI_UK,
		"CD",
		ARRAYSIZE(FEEBLEFILES_AMI_GameFiles),
		FEEBLEFILES_AMI_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformAmiga,
	},

	// The Feeble Files - German Amiga CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_AMI_DE,
		"CD",
		ARRAYSIZE(FEEBLEFILES_AMI_GameFiles),
		FEEBLEFILES_AMI_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformAmiga,
	},

	// The Feeble Files - English Macintosh CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_MAC_UK,
		"CD",
		ARRAYSIZE(FEEBLEFILES_MAC_GameFiles),
		FEEBLEFILES_MAC_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformMacintosh,
	},

	// The Feeble Files - French Macintosh CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_MAC_FR,
		"CD",
		ARRAYSIZE(FEEBLEFILES_MAC_GameFiles),
		FEEBLEFILES_MAC_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::FR_FRA,
		Common::kPlatformMacintosh,
	},

	// The Feeble Files - German Macintosh CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_MAC_DE,
		"CD",
		ARRAYSIZE(FEEBLEFILES_MAC_GameFiles),
		FEEBLEFILES_MAC_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformMacintosh,
	},

	// The Feeble Files - Spanish Macintosh CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_MAC_SP,
		"CD",
		ARRAYSIZE(FEEBLEFILES_MAC_GameFiles),
		FEEBLEFILES_MAC_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::ES_ESP,
		Common::kPlatformMacintosh,
	},

	// The Feeble Files - English Windows 2CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_2CD,
		"2CD",
		ARRAYSIZE(FEEBLEFILES_GameFiles),
		FEEBLEFILES_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// The Feeble Files - English Windows 4CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_4CD,
		"4CD",
		ARRAYSIZE(FEEBLEFILES_GameFiles),
		FEEBLEFILES_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// The Feeble Files - German Windows 4CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_DE,
		"4CD",
		ARRAYSIZE(FEEBLEFILES_GameFiles),
		FEEBLEFILES_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformWindows,
	},
};

static DetectedGame toDetectedGame(const GameDescription &g) {
	const char *title = 0;
	if (g.gameType == GType_SIMON1)
		title = "Simon the Sorcerer 1";
	if (g.gameType == GType_SIMON2)
		title = "Simon the Sorcerer 2";
	if (g.gameType == GType_FF)
		title = "The Feeble Files";
	DetectedGame dg(g.name, title, g.language, g.platform);
	dg.updateDesc(g.extra);
	return dg;
}

bool SimonEngine::initGame(void) {
	int gameNumber;
	FSList dummy;

	if ((gameNumber = detectGame(dummy)) == -1) {
		warning("No valid games were found in the specified directory.");
		return false;
	}

	debug(2, "Running %s", toDetectedGame(gameDescriptions[gameNumber]).description.c_str());

	_gameDescription = &gameDescriptions[gameNumber];

	return true;
}

DetectedGameList GAME_ProbeGame(const FSList &fslist, int **retmatches) {
	DetectedGameList detectedGames;
	int game_n;
	int index = 0, i, j;
	int matches[ARRAYSIZE(gameDescriptions)];
	bool mode = retmatches ? false : true;

	game_n = -1;
	for (i = 0; i < ARRAYSIZE(gameDescriptions); i++)
		matches[i] = -1;

	while (1) {
		game_n = detectGame(fslist, mode, game_n);
		if (game_n == -1)
			break;
		matches[index++] = game_n;
	}

	// We have some resource sets which are superpositions of other
	// Particularly it is ite-demo-linux vs ite-demo-win
	// Now remove lesser set if bigger matches too

	if (index > 1) {
		// Search max number
		int maxcount = 0;
		for (i = 0; i < index; i++) {
			int count = 0;
			for (j = 0; j < ARRAYSIZE(gameMD5); j++)
				if (gameMD5[j].id == gameDescriptions[matches[i]].gameId)
					count++;
			maxcount = MAX(maxcount, count);
		}

		// Now purge targets with number of files lesser than max
		for (i = 0; i < index; i++) {
			int count = 0;
			for (j = 0; j < ARRAYSIZE(gameMD5); j++)
				if (gameMD5[j].id == gameDescriptions[matches[i]].gameId)
					count++;
			if (count < maxcount) {
				debug(2, "Purged: %s", toDetectedGame(gameDescriptions[matches[i]]).description.c_str());
				matches[i] = -1;
			}
		}

	}

	// and now push them into list of detected games
	for (i = 0; i < index; i++)
		if (matches[i] != -1) {
			GameDescription &g = gameDescriptions[matches[i]];
			DetectedGame dg(g.name, findDescriptionFromGameID(g.name), g.language, g.platform);
			dg.updateDesc(g.extra);
			detectedGames.push_back(dg);
		}
		
	if (retmatches) {
		*retmatches = (int *)calloc(ARRAYSIZE(gameDescriptions), sizeof(int));
		for (i = 0; i < ARRAYSIZE(gameDescriptions); i++)
			(*retmatches)[i] = matches[i];
	}

	return detectedGames;
}

int detectGame(const FSList &fslist, bool mode, int start) {
	int game_count = ARRAYSIZE(gameDescriptions);
	int game_n = -1;
	typedef Common::Map<Common::String, Common::String> StringMap;
	StringMap filesMD5;

	typedef Common::Map<Common::String, bool> StringSet;
	StringSet filesList;

	uint16 file_count;
	uint16 file_n;
	Common::File test_file;
	bool file_missing;

	Common::String tstr, tstr1;
	char md5str[32+1];
	uint8 md5sum[16];

	// First we compose list of files which we need MD5s for
	for (int i = 0; i < ARRAYSIZE(gameMD5); i++) {
		tstr = Common::String(gameMD5[i].filename);
		tstr.toLowercase();

		if (gameMD5[i].caseSensitive && !mode)
			filesList[Common::String(gameMD5[i].filename)] = true;
		else
			filesList[tstr] = true;
	}

	if (mode) {
		// Now count MD5s for required files
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				tstr = file->displayName();
				// FIXME: there is a bug in String class. tstr1 = tstr; tstr.toLowercase()
				// makes tstr1 lowercase as well
				tstr1 = Common::String(file->displayName().c_str());
				tstr.toLowercase();

				if (filesList.contains(tstr) || filesList.contains(tstr1)) {
					if (Common::md5_file(file->path().c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
						for (int j = 0; j < 16; j++) {
							sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
						}
						filesMD5[tstr] = Common::String(md5str);
						filesMD5[tstr1] = Common::String(md5str);
					}
				}
			}
		}
	} else {
		Common::File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			if (testFile.open(file->_key.c_str())) {
				testFile.close();
				if (Common::md5_file(file->_key.c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
					for (int j = 0; j < 16; j++) {
						sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
					}
					filesMD5[file->_key] = Common::String(md5str);
				}
			}
		}
	}

	for (game_n = start + 1; game_n < game_count; game_n++) {
		file_count = gameDescriptions[game_n].filesCount;
		file_missing = false;

		// Try to open all files for this game
		for (file_n = 0; file_n < file_count; file_n++) {
			tstr = gameDescriptions[game_n].filesDescriptions[file_n].fileName;

			if (!filesMD5.contains(tstr)) {
				file_missing = true;
				break;
			}
		}

		// Try the next game, couldn't find all files for the current
		// game
		if (file_missing) {
			continue;
		} else {
			bool match = true;

			debug(2, "Probing game: %s", toDetectedGame(gameDescriptions[game_n]).description.c_str());

			for (int i = 0; i < ARRAYSIZE(gameMD5); i++) {
				if (gameMD5[i].id == gameDescriptions[game_n].gameId) {
					tstr = gameMD5[i].filename;

					if (strcmp(gameMD5[i].md5, filesMD5[tstr].c_str())) {
						match = false;
						break;
					}
				}
			}
			if (!match)
				continue;

			debug(2, "Found game: %s", toDetectedGame(gameDescriptions[game_n]).description.c_str());

			return game_n;
		}
	}

	if (!filesMD5.isEmpty() && start == -1) {
		printf("MD5s of your game version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your game name and version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());
	}

	return -1;
}

} // End of namespace Simon
