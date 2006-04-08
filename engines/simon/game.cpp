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
	{"feeble", "The Feeble Files"},
	{"simon1", "Simon the Sorcerer 1"},
	{"simon2", "Simon the Sorcerer 2"},
	{NULL, NULL}
};

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
	return Simon::GAME_detectGames(fslist);
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

REGISTER_PLUGIN(SIMON, "Simon the Sorcerer");

namespace Simon {

#define FILE_MD5_BYTES 5000

static GameFileDescription SIMON1ACORNDEMO_GameFiles[] = {
	{ "data", 	GAME_GMEFILE,	"b4a7526ced425ba8ad0d548d0ec69900"},
	{ "gamebase", 	GAME_BASEFILE,	"425c7d1957699d35abca7e12a08c7422"},
	{ "icondata", 	GAME_ICONFILE, 	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped", 	GAME_STRFILE,	"d9de7542612d9f4e0819ad0df5eac56b"},
	{ "tbllist", 	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1ACORN_GameFiles[] = {
	{ "data", 	GAME_GMEFILE,	"64958b3a38afdcb85da1eeed85169806"},
	{ "gamebase", 	GAME_BASEFILE,	"28261b99cd9da1242189b4f6f2841bd6"},
	{ "icondata", 	GAME_ICONFILE, 	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped", 	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist", 	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1AMIGA_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"6c9ad2ff571d34a4cf0c696cf4e13500"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static GameFileDescription SIMON1AMIGA_FR_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"bd9828b9d4e5d89b50fe8c47a8e6bc07"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"2297baec985617d0d5612a0124bac359"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static GameFileDescription SIMON1AMIGA_DE_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"a2de9553f3b73064369948b5af38bb30"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static GameFileDescription SIMON1AMIGADEMO_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"a12b696170f14eca5ff75f1549829251"},  // Unpacked version
	{ "icon.pkd",		GAME_ICONFILE,	"ebc96af15bfaf75ba8210326b9260d2f"},
	{ "stripped.txt",	GAME_STRFILE,	"8edde5b9498dc9f31da1093028da467c"},
	{ "tbllist",		GAME_TBLFILE,	"1247e024e1f13ca54c1e354120c7519c"},
};

static GameFileDescription SIMON1CD32_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"bab7f19237cf7d7619b6c73631da1854"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"59be788020441e21861e284236fd08c1"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static GameFileDescription SIMON1CD32_2_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"ec5358680c117f29b128cbbb322111a4"},
	{ "icon.pkd",		GAME_ICONFILE,	"8ce5a46466a4f8f6d0f780b0ef00d5f5"},
	{ "stripped.txt",	GAME_STRFILE,	"59be788020441e21861e284236fd08c1"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static GameFileDescription SIMON1DOS_INF_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"9f93d27432ce44a787eef10adb640870"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1DOS_INF_RU_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"605fb866e03ec1c41b10c6a518ddfa49"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1DOS_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"c392e494dcabed797b98cbcfc687b33a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1DOS_RU_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"605fb866e03ec1c41b10c6a518ddfa49"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1DOS_FR_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"34759d0d4285a2f4b21b8e03b8fcefb3"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"aa01e7386057abc0c3e27dbaa9c4ba5b"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1DOS_DE_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"063015e6ce7d90b570dbc21fe0c667b1"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1DOS_IT_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"65c9b2dea57df84ef55d1eaf384ebd30"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1DOS_ES_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"5374fafdea2068134f33deab225feed3"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1DEMO_GameFiles[] = {
	{ "gdemo",		GAME_BASEFILE,	"2be4a21bc76e2fdc071867c130651439"},
	{ "icon.dat",		GAME_ICONFILE,	"55af3b4d93972bc58bfee38a86b76c3f"},
	{ "stripped.txt",	GAME_STRFILE,	"33a2e329b97b2a349858d6a093159eb7"},
	{ "tbllist",		GAME_TBLFILE,	"1247e024e1f13ca54c1e354120c7519c"},
};

static GameFileDescription SIMON1TALKIE_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"28261b99cd9da1242189b4f6f2841bd6"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"64958b3a38afdcb85da1eeed85169806"},
	{ "stripped.txt",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1TALKIE2_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"c0b948b6821d2140f8b977144f21027a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"64f73e94639b63af846ac4a8a94a23d8"},
	{ "stripped.txt",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1TALKIE_FR_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"3cfb9d1ff4ec725af9924140126cf69f"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"638049fa5d41b81fb6fb11671721b871"},
	{ "stripped.txt",	GAME_STRFILE,	"ef51ac74c946881ae4d7ca66cc7a0d1e"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1TALKIE_DE_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"48b1f3499e2e0d731047f4d481ff7817"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"7db9912acac4f1d965a64bdcfc370ba1"},
	{ "stripped.txt",	GAME_STRFILE,	"40d68bec54042ef930f084ad9a4342a1"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1TALKIE_HB_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"bc66e9c0b296e1b155a246917133f71a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"a34b2c8642f2e3676d7088b5c8b3e884"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1TALKIE_IT_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"8d3ca654e158c91b860c7eae31d65312"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"104efd83c8f3edf545982e07d87f66ac"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1TALKIE_ES_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"439f801ba52c02c9d1844600d1ce0f5e"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"eff2774a73890b9eac533db90cd1afa1"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1WIN_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"c7c12fea7f6d0bfd22af5cdbc8166862"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"b1b18d0731b64c0738c5cc4a2ee792fc"},
	{ "stripped.txt",	GAME_STRFILE,	"a27e87a9ba21212d769804b3df47bfb2"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON1WIN_DE_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"48b1f3499e2e0d731047f4d481ff7817"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"acd9cc438525b142d93b15c77a6f551b"},
	{ "stripped.txt",	GAME_STRFILE,	"40d68bec54042ef930f084ad9a4342a1"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static GameFileDescription SIMON2DOS_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"27c8e7feada80c75b70b9c2f6088d519"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"eefcc32b1f2c0482c1a59a963a146345"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2DOS_RU_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"7edfc633dd50f8caa719c478443db70b"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"eefcc32b1f2c0482c1a59a963a146345"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2DOS2_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"604d04315935e77624bd356ac926e068"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"aa6840420899a31874204f90bb214108"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2DOS2_RU_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"eb8bde3685842a8fd38f60bc476ef8e9"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"aa6840420899a31874204f90bb214108"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2DOS_DE_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"eb6e3e37fe52993f948d7e2d6b869828"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"5fa9d080b04c610f526bd685be1bf747"},
	{ "stripped.txt",	GAME_STRFILE,	"fd30df01cc248ecbaef302af855e0212"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2DOS_IT_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"3e11d400bea0638f360a724687005cd1"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"f306a397565d7f13bec7ecf14c723de7"},
	{ "stripped.txt",	GAME_STRFILE,	"bea6843fb9f3b2144fcb146d62db0b9a"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2DEMO_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"3794c15887539b8578bacab694ccf08a"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"f8c9e6df1e55923a749e115ba74210c4"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601"},
};

static GameFileDescription SIMON2TALKIE_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"8c301fb9c4fcf119d2730ccd2a565eb3"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"9c535d403966750ae98bdaf698375a38"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2TALKIE2_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"608e277904d87dd28725fa08eacc2c0d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"8d6dcc65577e285dbca03ff6d7d9323c"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601"},
};

static GameFileDescription SIMON2TALKIE_FR_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"43b3a04d2f0a0cbd1b024c814856561a"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"8af0e02c0c3344db64dffc12196eb59d"},
	{ "stripped.txt",	GAME_STRFILE,	"5ea27977b4d7dcfd50eb5074e162ebbf"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2TALKIE_DE_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"0d05c3f4c06c9a4ceb3d2f5bc0b18e11"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"6c5fdfdd0eab9038767c2d22858406b2"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2TALKIE_DE2_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"a76ea940076b5d9316796dea225a9b69"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"ec9f0f24fd895e7ea72e3c8e448c0240"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2TALKIE_HB_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"952a2b1be23c3c609ba8d988a9a1627d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"a2b249a82ea182af09789eb95fb6c5be"},
	{ "stripped.txt",	GAME_STRFILE,	"de9dbc24158660e153483fa0cf6c3172"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2TALKIE_IT_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"3e11d400bea0638f360a724687005cd1"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"344aca58e5ad5e25c517d5eb1d85c435"},
	{ "stripped.txt",	GAME_STRFILE,	"bea6843fb9f3b2144fcb146d62db0b9a"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2TALKIE_ES_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"268dc322aa73bcf27bb016b8e8ceb889"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"4f43bd06b6cc78dbd25a7475ca964eb1"},
	{ "stripped.txt",	GAME_STRFILE,	"d13753796bd81bf313a2449f34d8b112"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2WIN_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"608e277904d87dd28725fa08eacc2c0d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"e749c4c103d7e7d51b34620ed76c5a04"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2WIN_DE_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"a76ea940076b5d9316796dea225a9b69"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"9609a933c541fed2e00c6c3479d7c181"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2WIN_DE2_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"9e858b3bb189c134c3a5f34c3385a8d3"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"16d574da07e93bcae43cee353dab8c7e"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription SIMON2WIN_PL_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"657fd873f5d0637097ee02315b447e6f"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"7b9afcf82a94722707e0d025c0192be8"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static GameFileDescription FEEBLEFILES_AMI_DE_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"f550f7915c5ce3a68c9f870f507449c2"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameFileDescription FEEBLEFILES_AMI_UK_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"f550f7915c5ce3a68c9f870f507449c2"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameFileDescription FEEBLEFILES_MAC_DE_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameFileDescription FEEBLEFILES_MAC_FR_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"ba90b40a47726039671d9e91630dd7ed"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameFileDescription FEEBLEFILES_MAC_SP_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"71d7d2d5e479b053c5a9757f1702c9c3"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameFileDescription FEEBLEFILES_MAC_UK_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameFileDescription FEEBLEFILES_2CD_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameFileDescription FEEBLEFILES_4CD_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"a8746407a5b20a7da0da0a14c380af1c"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameFileDescription FEEBLEFILES_DE_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static GameDescription gameDescriptions[] = {
	// Simon the Sorcerer 1 - English Acorn CD Demo
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1ACORNDEMO,
		"CD Demo",
		ARRAYSIZE(SIMON1ACORNDEMO_GameFiles),
		SIMON1ACORNDEMO_GameFiles,
		GF_TALKIE,
		Common::EN_ANY,
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
		Common::EN_ANY,
		Common::kPlatformAcorn,
	},

	// Simon the Sorcerer 1 - English AGA Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1AMIGA,
		"AGA Floppy",
		ARRAYSIZE(SIMON1AMIGA_GameFiles),
		SIMON1AMIGA_GameFiles,
		GF_CRUNCHED | GF_OLD_BUNDLE,
		Common::EN_ANY,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - French AGA Floppy
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1AMIGA_FR,
		"AGA Floppy",
		ARRAYSIZE(SIMON1AMIGA_FR_GameFiles),
		SIMON1AMIGA_FR_GameFiles,
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
		ARRAYSIZE(SIMON1AMIGA_DE_GameFiles),
		SIMON1AMIGA_DE_GameFiles,
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
		ARRAYSIZE(SIMON1AMIGADEMO_GameFiles),
		SIMON1AMIGADEMO_GameFiles,
		GF_CRUNCHED | GF_OLD_BUNDLE,
		Common::EN_ANY,
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
		Common::EN_ANY,
		Common::kPlatformAmiga,
	},

	// Simon the Sorcerer 1 - English Amiga CD32 alternative?
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1CD32_2,
		"CD32",
		ARRAYSIZE(SIMON1CD32_2_GameFiles),
		SIMON1CD32_2_GameFiles,
		GF_TALKIE | GF_OLD_BUNDLE,
		Common::EN_ANY,
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
		Common::EN_ANY,
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
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS Floppy with Russian patch
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_RU,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_RU_GameFiles),
		SIMON1DOS_RU_GameFiles,
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
		ARRAYSIZE(SIMON1DOS_INF_GameFiles),
		SIMON1DOS_INF_GameFiles,
		GF_OLD_BUNDLE,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom) with Russian patch
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1DOS_INF_RU,
		"Floppy",
		ARRAYSIZE(SIMON1DOS_INF_RU_GameFiles),
		SIMON1DOS_INF_RU_GameFiles,
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
		ARRAYSIZE(SIMON1DOS_FR_GameFiles),
		SIMON1DOS_FR_GameFiles,
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
		ARRAYSIZE(SIMON1DOS_DE_GameFiles),
		SIMON1DOS_DE_GameFiles,
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
		ARRAYSIZE(SIMON1DOS_IT_GameFiles),
		SIMON1DOS_IT_GameFiles,
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
		ARRAYSIZE(SIMON1DOS_ES_GameFiles),
		SIMON1DOS_ES_GameFiles,
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
		ARRAYSIZE(SIMON1TALKIE_GameFiles),
		SIMON1TALKIE_GameFiles,
		GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - English DOS CD alternate?
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE2,
		"CD",
		ARRAYSIZE(SIMON1TALKIE2_GameFiles),
		SIMON1TALKIE2_GameFiles,
		GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 1 - French DOS CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1TALKIE_FR,
		"CD",
		ARRAYSIZE(SIMON1TALKIE_FR_GameFiles),
		SIMON1TALKIE_FR_GameFiles,
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
		ARRAYSIZE(SIMON1TALKIE_DE_GameFiles),
		SIMON1TALKIE_DE_GameFiles,
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
		ARRAYSIZE(SIMON1TALKIE_HB_GameFiles),
		SIMON1TALKIE_HB_GameFiles,
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
		ARRAYSIZE(SIMON1TALKIE_IT_GameFiles),
		SIMON1TALKIE_IT_GameFiles,
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
		ARRAYSIZE(SIMON1TALKIE_ES_GameFiles),
		SIMON1TALKIE_ES_GameFiles,
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
		ARRAYSIZE(SIMON1WIN_GameFiles),
		SIMON1WIN_GameFiles,
		GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 1 - German Windows CD
	{
		"simon1",
		GType_SIMON1,
		GID_SIMON1WIN_DE,
		"CD",
		ARRAYSIZE(SIMON1WIN_DE_GameFiles),
		SIMON1WIN_DE_GameFiles,
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
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS Floppy with Russian patch
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS_RU,
		"Floppy",
		ARRAYSIZE(SIMON2DOS_RU_GameFiles),
		SIMON2DOS_RU_GameFiles,
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
		ARRAYSIZE(SIMON2DOS2_GameFiles),
		SIMON2DOS2_GameFiles,
		0,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS Floppy alternate? with Russian patch
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2DOS2_RU,
		"Floppy",
		ARRAYSIZE(SIMON2DOS2_RU_GameFiles),
		SIMON2DOS2_RU_GameFiles,
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
		ARRAYSIZE(SIMON2DOS_DE_GameFiles),
		SIMON2DOS_DE_GameFiles,
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
		ARRAYSIZE(SIMON2DOS_IT_GameFiles),
		SIMON2DOS_IT_GameFiles,
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
		ARRAYSIZE(SIMON2DEMO_GameFiles),
		SIMON2DEMO_GameFiles,
		GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - English DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE,
		"CD",
		ARRAYSIZE(SIMON2TALKIE_GameFiles),
		SIMON2TALKIE_GameFiles,
		GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformPC,
	},


	// Simon the Sorcerer 2 - English DOS CD alternate?
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE2,
		"CD",
		ARRAYSIZE(SIMON2TALKIE2_GameFiles),
		SIMON2TALKIE2_GameFiles,
		GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Simon the Sorcerer 2 - French DOS CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2TALKIE_FR,
		"CD",
		ARRAYSIZE(SIMON2TALKIE_FR_GameFiles),
		SIMON2TALKIE_FR_GameFiles,
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
		ARRAYSIZE(SIMON2TALKIE_DE_GameFiles),
		SIMON2TALKIE_DE_GameFiles,
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
		ARRAYSIZE(SIMON2TALKIE_DE2_GameFiles),
		SIMON2TALKIE_DE2_GameFiles,
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
		ARRAYSIZE(SIMON2TALKIE_HB_GameFiles),
		SIMON2TALKIE_HB_GameFiles,
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
		ARRAYSIZE(SIMON2TALKIE_IT_GameFiles),
		SIMON2TALKIE_IT_GameFiles,
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
		ARRAYSIZE(SIMON2TALKIE_ES_GameFiles),
		SIMON2TALKIE_ES_GameFiles,
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
		ARRAYSIZE(SIMON2WIN_GameFiles),
		SIMON2WIN_GameFiles,
		GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformWindows,
	},

	// Simon the Sorcerer 2 - German Windows CD
	{
		"simon2",
		GType_SIMON2,
		GID_SIMON2WIN_DE,
		"CD",
		ARRAYSIZE(SIMON2WIN_DE_GameFiles),
		SIMON2WIN_DE_GameFiles,
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
		ARRAYSIZE(SIMON2WIN_DE2_GameFiles),
		SIMON2WIN_DE2_GameFiles,
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
		ARRAYSIZE(SIMON2WIN_PL_GameFiles),
		SIMON2WIN_PL_GameFiles,
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
		ARRAYSIZE(FEEBLEFILES_AMI_UK_GameFiles),
		FEEBLEFILES_AMI_UK_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformAmiga,
	},

	// The Feeble Files - German Amiga CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_AMI_DE,
		"CD",
		ARRAYSIZE(FEEBLEFILES_AMI_DE_GameFiles),
		FEEBLEFILES_AMI_DE_GameFiles,
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
		ARRAYSIZE(FEEBLEFILES_MAC_UK_GameFiles),
		FEEBLEFILES_MAC_UK_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformMacintosh,
	},

	// The Feeble Files - French Macintosh CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_MAC_FR,
		"CD",
		ARRAYSIZE(FEEBLEFILES_MAC_FR_GameFiles),
		FEEBLEFILES_MAC_FR_GameFiles,
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
		ARRAYSIZE(FEEBLEFILES_MAC_DE_GameFiles),
		FEEBLEFILES_MAC_DE_GameFiles,
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
		ARRAYSIZE(FEEBLEFILES_MAC_SP_GameFiles),
		FEEBLEFILES_MAC_SP_GameFiles,
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
		ARRAYSIZE(FEEBLEFILES_2CD_GameFiles),
		FEEBLEFILES_2CD_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformWindows,
	},

	// The Feeble Files - English Windows 4CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_4CD,
		"4CD",
		ARRAYSIZE(FEEBLEFILES_4CD_GameFiles),
		FEEBLEFILES_4CD_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::EN_ANY,
		Common::kPlatformWindows,
	},

	// The Feeble Files - German Windows 4CD
	{
		"feeble",
		GType_FF,
		GID_FEEBLEFILES_DE,
		"4CD",
		ARRAYSIZE(FEEBLEFILES_DE_GameFiles),
		FEEBLEFILES_DE_GameFiles,
		GF_OLD_BUNDLE | GF_TALKIE,
		Common::DE_DEU,
		Common::kPlatformWindows,
	},
};

DetectedGame GameDescription::toDetectedGame() {
	const char *title;
	title = simonGames[gameType].description;
	DetectedGame dg(name, title, language, platform);
	dg.updateDesc(extra);
	return dg;
}

static int detectGame(const FSList *fslist, Common::Language language, Common::Platform platform, int*& returnMatches) {
	int gamesCount = ARRAYSIZE(gameDescriptions);
	int filesCount;

	typedef Common::HashMap<Common::String, bool> StringSet;
	StringSet filesList;

	typedef Common::HashMap<Common::String, Common::String> StringMap;
	StringMap filesMD5;

	Common::String tstr, tstr2;
	
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
			tstr2 = tstr + ".";
			filesList[tstr] = true;
			filesList[tstr2] = true;
		}
	}
	
	if (fslist != NULL) {
		for (FSList::const_iterator file = fslist->begin(); file != fslist->end(); ++file) {
			if (file->isDirectory()) continue;
			tstr = file->displayName();
			tstr.toLowercase();
			tstr2 = tstr + ".";

			if (!filesList.contains(tstr) && !filesList.contains(tstr2)) continue;

			if (!Common::md5_file(file->path().c_str(), md5sum, NULL, FILE_MD5_BYTES)) continue;
			for (j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
			filesMD5[tstr] = Common::String(md5str);
			filesMD5[tstr2] = Common::String(md5str);
		}
	} else {
		Common::File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			tstr = file->_key;
			tstr.toLowercase();

			if(!filesMD5.contains(tstr)) {
				if (testFile.open(file->_key.c_str())) {
					testFile.close();

					if (Common::md5_file(file->_key.c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
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
			tstr2 = tstr + ".";

			if (!filesMD5.contains(tstr) && !filesMD5.contains(tstr2)) {
				fileMissing = true;
				break;
			}
			if (strcmp(fileDesc->md5, filesMD5[tstr].c_str()) && strcmp(fileDesc->md5, filesMD5[tstr2].c_str())) {
				fileMissing = true;
				break;
			}
		}
		if (!fileMissing) {
			debug(2, "Found game: %s", gameDescriptions[i].toDetectedGame().description.c_str());
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
				debug(2, "Purged %s", gameDescriptions[matched[i]].toDetectedGame().description.c_str());
				matched[i] = -1;
				continue;
			}

			if (gameDescriptions[matched[i]].filesCount < maxcount) {
				debug(2, "Purged: %s", gameDescriptions[matched[i]].toDetectedGame().description.c_str());
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

bool SimonEngine::initGame() {
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
		error("SimonEngine::loadGame wrong gameNumber");
	}

	debug(2, "Running %s", gameDescriptions[gameNumber].toDetectedGame().description.c_str());

	_gameDescription = &gameDescriptions[gameNumber];

	return true;
}

DetectedGameList GAME_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	int count;
	int* matches;
	count = detectGame(&fslist, Common::UNK_LANG, Common::kPlatformUnknown, matches);

	for (int i = 0; i < count; i++)
		detectedGames.push_back(gameDescriptions[matches[i]].toDetectedGame());
	free(matches);
	return detectedGames;
}

} // End of namespace Simon
