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
#include "common/md5.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

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
	{"elvira1", "Elvira"},
	{"elvira2", "Elvira 2"},
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

#define FILE_MD5_BYTES 5000

using Common::ADGameFileDescription;
using Common::ADGameDescription;

static ADGameFileDescription ELVIRA1AMIGADEMO_GameFiles[] = {
	{ "englishdemo",	GAME_BASEFILE,	"7bbc2dfe8619ef579004ff57674c6e92"},
	{ "icon.dat",		GAME_ICONFILE,	"68b329da9893e34099c7d8ad5cb9c940"},
};

static ADGameFileDescription ELVIRA1AMIGA_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"7bdaff4a118d8035047cf9b1393b3fa0"},
	{ "icon.dat",		GAME_ICONFILE,	"2db931e84f1ca01f0816dddfae3f49e1"},
};

static ADGameFileDescription ELVIRA1DOS_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"a49e132a1f18306dd5d1ec2fe435e178"},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45"},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149"},
};

static ADGameFileDescription ELVIRA1DOS2_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"9076d507d60cc454df662316438ec843"},
	{ "icon.dat",		GAME_ICONFILE,	"fda48c9da7f3e72d0313e2f5f760fc45"},
	{ "tbllist",		GAME_TBLFILE,	"319f6b227c7822a551f57d24e70f8149"},
};

static ADGameFileDescription ELVIRA2AMIGA_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"4aa163967f5d2bd319f8350d6af03186"},
	{ "icon.dat",		GAME_ICONFILE,	"a88b1c02e13ab04dd790ec30502c323d"},
	{ "stripped.txt",	GAME_STRFILE,	"41c975a9c1106cb5298a0bc3df0a266e"},
	{ "tbllist",		GAME_TBLFILE,	"177f5f2640e80ef92d1421d32de06a5e"},
};

static ADGameFileDescription ELVIRA2DOS_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"3313254722031b22d833a2cf45a91fd7"},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9"},
	{ "stripped.txt",	GAME_STRFILE,	"c2533277b7ff11f5495967d55355ea17"},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5"},
};

static ADGameFileDescription ELVIRA2DOS_FR_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"4bf28ab00f5324fd938e632595742382"},
	{ "icon.dat",		GAME_ICONFILE,	"83a7278bff55c82fbb3aef92981866c9"},
	{ "stripped.txt",	GAME_STRFILE,	"c3a8f644551a27c8a2fec0f8070b46b7"},
	{ "tbllist",		GAME_TBLFILE,	"8252660df0edbdbc3e6377e155bbd0c5"},
};

static ADGameFileDescription WAXWORKSAMIGA_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"e6207a69cf7356f13acbe8d14bdd844e"},  // Unpacked version
	{ "icon.pkd",		GAME_ICONFILE,	"4822a91c18b1b2005ac17fc617f7dcbe"},
	{ "stripped.txt",	GAME_STRFILE,	"6faaebff2786216900061eeb978f10af"},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0"},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f"},
};

static ADGameFileDescription WAXWORKSDOS_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"7751e9358e894e32ef40ef3b3bae0f2a"},
	{ "icon.dat",		GAME_ICONFILE,	"ef1b8ad3494cf103dc10a99fe152ef9a"},
	{ "roomslst",		GAME_RMSLFILE,	"e3758c46ab8f3c23a1ac012bd607108d"},
	{ "stripped.txt",	GAME_STRFILE,	"f259e3e07a1cde8d0404a767d815e12c"},
	{ "tbllist",		GAME_TBLFILE,	"95c44bfc380770a6b6dd0dfcc69e80a0"},
	{ "xtbllist",		GAME_XTBLFILE,	"6c7b3db345d46349a5226f695c03e20f"},
};

static ADGameFileDescription SIMON1ACORNDEMO_GameFiles[] = {
	{ "data", 	GAME_GMEFILE,	"b4a7526ced425ba8ad0d548d0ec69900"},
	{ "gamebase", 	GAME_BASEFILE,	"425c7d1957699d35abca7e12a08c7422"},
	{ "icondata", 	GAME_ICONFILE, 	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped", 	GAME_STRFILE,	"d9de7542612d9f4e0819ad0df5eac56b"},
	{ "tbllist", 	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1ACORN_GameFiles[] = {
	{ "data", 	GAME_GMEFILE,	"64958b3a38afdcb85da1eeed85169806"},
	{ "gamebase", 	GAME_BASEFILE,	"28261b99cd9da1242189b4f6f2841bd6"},
	{ "icondata", 	GAME_ICONFILE, 	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped", 	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist", 	GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1AMIGA_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"6c9ad2ff571d34a4cf0c696cf4e13500"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static ADGameFileDescription SIMON1AMIGA_FR_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"bd9828b9d4e5d89b50fe8c47a8e6bc07"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"2297baec985617d0d5612a0124bac359"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static ADGameFileDescription SIMON1AMIGA_DE_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"a2de9553f3b73064369948b5af38bb30"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"c649fcc0439766810e5097ee7e81d4c8"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static ADGameFileDescription SIMON1AMIGADEMO_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"a12b696170f14eca5ff75f1549829251"},  // Unpacked version
	{ "icon.pkd",		GAME_ICONFILE,	"ebc96af15bfaf75ba8210326b9260d2f"},
	{ "stripped.txt",	GAME_STRFILE,	"8edde5b9498dc9f31da1093028da467c"},
	{ "tbllist",		GAME_TBLFILE,	"1247e024e1f13ca54c1e354120c7519c"},
};

static ADGameFileDescription SIMON1CD32_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"bab7f19237cf7d7619b6c73631da1854"},
	{ "icon.pkd",		GAME_ICONFILE,	"565ef7a98dcc21ef526a2bb10b6f42ed"},
	{ "stripped.txt",	GAME_STRFILE,	"59be788020441e21861e284236fd08c1"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static ADGameFileDescription SIMON1CD32_2_GameFiles[] = {
	{ "gameamiga",		GAME_BASEFILE,	"ec5358680c117f29b128cbbb322111a4"},
	{ "icon.pkd",		GAME_ICONFILE,	"8ce5a46466a4f8f6d0f780b0ef00d5f5"},
	{ "stripped.txt",	GAME_STRFILE,	"59be788020441e21861e284236fd08c1"},
	{ "tbllist",		GAME_TBLFILE,	"f9d5bf2ce09f82289c791c3ca26e1e4b"},
};

static ADGameFileDescription SIMON1DOS_INF_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"9f93d27432ce44a787eef10adb640870"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1DOS_INF_RU_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"605fb866e03ec1c41b10c6a518ddfa49"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1DOS_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"c392e494dcabed797b98cbcfc687b33a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1DOS_RU_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"605fb866e03ec1c41b10c6a518ddfa49"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1DOS_FR_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"34759d0d4285a2f4b21b8e03b8fcefb3"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"aa01e7386057abc0c3e27dbaa9c4ba5b"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1DOS_DE_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"063015e6ce7d90b570dbc21fe0c667b1"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"c95a0a1ee973e19c2a1c5d12026c139f"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1DOS_IT_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"65c9b2dea57df84ef55d1eaf384ebd30"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1DOS_ES_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"5374fafdea2068134f33deab225feed3"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "stripped.txt",	GAME_STRFILE,	"2af9affc5981eec44b90d4c556145cb8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1DEMO_GameFiles[] = {
	{ "gdemo",		GAME_BASEFILE,	"2be4a21bc76e2fdc071867c130651439"},
	{ "icon.dat",		GAME_ICONFILE,	"55af3b4d93972bc58bfee38a86b76c3f"},
	{ "stripped.txt",	GAME_STRFILE,	"33a2e329b97b2a349858d6a093159eb7"},
	{ "tbllist",		GAME_TBLFILE,	"1247e024e1f13ca54c1e354120c7519c"},
};

static ADGameFileDescription SIMON1TALKIE_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"28261b99cd9da1242189b4f6f2841bd6"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"64958b3a38afdcb85da1eeed85169806"},
	{ "stripped.txt",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1TALKIE2_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"c0b948b6821d2140f8b977144f21027a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"64f73e94639b63af846ac4a8a94a23d8"},
	{ "stripped.txt",	GAME_STRFILE,	"f3b27a3fbb45dcd323a48159496e45e8"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1TALKIE_FR_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"3cfb9d1ff4ec725af9924140126cf69f"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"638049fa5d41b81fb6fb11671721b871"},
	{ "stripped.txt",	GAME_STRFILE,	"ef51ac74c946881ae4d7ca66cc7a0d1e"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1TALKIE_DE_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"48b1f3499e2e0d731047f4d481ff7817"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"7db9912acac4f1d965a64bdcfc370ba1"},
	{ "stripped.txt",	GAME_STRFILE,	"40d68bec54042ef930f084ad9a4342a1"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1TALKIE_HB_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"bc66e9c0b296e1b155a246917133f71a"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"a34b2c8642f2e3676d7088b5c8b3e884"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1TALKIE_IT_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"8d3ca654e158c91b860c7eae31d65312"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"52e315e0e02feca86d15cc82e3306b6c"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1TALKIE_IT2_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"8d3ca654e158c91b860c7eae31d65312"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"104efd83c8f3edf545982e07d87f66ac"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1TALKIE_ES_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"439f801ba52c02c9d1844600d1ce0f5e"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"eff2774a73890b9eac533db90cd1afa1"},
	{ "stripped.txt",	GAME_STRFILE,	"9d31bef42db1a8abe4e9f368014df1d5"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1WIN_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"c7c12fea7f6d0bfd22af5cdbc8166862"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"b1b18d0731b64c0738c5cc4a2ee792fc"},
	{ "stripped.txt",	GAME_STRFILE,	"a27e87a9ba21212d769804b3df47bfb2"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1WIN_RU_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"4536a706412b36d628f12142bfa97af0"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"b1b18d0731b64c0738c5cc4a2ee792fc"},
	{ "stripped.txt",	GAME_STRFILE,	"a27e87a9ba21212d769804b3df47bfb2"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON1WIN_DE_GameFiles[] = {
	{ "gamepc",		GAME_BASEFILE,	"48b1f3499e2e0d731047f4d481ff7817"},
	{ "icon.dat",		GAME_ICONFILE,	"22107c24dfb31b66ac503c28a6e20b19"},
	{ "simon.gme",		GAME_GMEFILE,	"acd9cc438525b142d93b15c77a6f551b"},
	{ "stripped.txt",	GAME_STRFILE,	"40d68bec54042ef930f084ad9a4342a1"},
	{ "tbllist",		GAME_TBLFILE,	"d198a80de2c59e4a0cd24b98814849e8"},
};

static ADGameFileDescription SIMON2DOS_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"27c8e7feada80c75b70b9c2f6088d519"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"eefcc32b1f2c0482c1a59a963a146345"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2DOS_RU_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"7edfc633dd50f8caa719c478443db70b"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"eefcc32b1f2c0482c1a59a963a146345"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2DOS2_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"604d04315935e77624bd356ac926e068"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"aa6840420899a31874204f90bb214108"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2DOS2_RU_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"eb8bde3685842a8fd38f60bc476ef8e9"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"aa6840420899a31874204f90bb214108"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2DOS_DE_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"eb6e3e37fe52993f948d7e2d6b869828"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"5fa9d080b04c610f526bd685be1bf747"},
	{ "stripped.txt",	GAME_STRFILE,	"fd30df01cc248ecbaef302af855e0212"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2DOS_IT_GameFiles[] = {
	{ "game32",		GAME_BASEFILE,	"3e11d400bea0638f360a724687005cd1"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"f306a397565d7f13bec7ecf14c723de7"},
	{ "stripped.txt",	GAME_STRFILE,	"bea6843fb9f3b2144fcb146d62db0b9a"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2DEMO_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"3794c15887539b8578bacab694ccf08a"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"f8c9e6df1e55923a749e115ba74210c4"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601"},
};

static ADGameFileDescription SIMON2TALKIE_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"8c301fb9c4fcf119d2730ccd2a565eb3"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"9c535d403966750ae98bdaf698375a38"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2TALKIE2_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"608e277904d87dd28725fa08eacc2c0d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"8d6dcc65577e285dbca03ff6d7d9323c"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"a0d5a494b5d3d209d1a1d76cc8d76601"},
};

static ADGameFileDescription SIMON2TALKIE_FR_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"43b3a04d2f0a0cbd1b024c814856561a"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"8af0e02c0c3344db64dffc12196eb59d"},
	{ "stripped.txt",	GAME_STRFILE,	"5ea27977b4d7dcfd50eb5074e162ebbf"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2TALKIE_DE_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"0d05c3f4c06c9a4ceb3d2f5bc0b18e11"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"6c5fdfdd0eab9038767c2d22858406b2"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2TALKIE_DE2_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"a76ea940076b5d9316796dea225a9b69"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"ec9f0f24fd895e7ea72e3c8e448c0240"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2TALKIE_HB_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"952a2b1be23c3c609ba8d988a9a1627d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"a2b249a82ea182af09789eb95fb6c5be"},
	{ "stripped.txt",	GAME_STRFILE,	"de9dbc24158660e153483fa0cf6c3172"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2TALKIE_IT_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"3e11d400bea0638f360a724687005cd1"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"344aca58e5ad5e25c517d5eb1d85c435"},
	{ "stripped.txt",	GAME_STRFILE,	"bea6843fb9f3b2144fcb146d62db0b9a"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2TALKIE_ES_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"268dc322aa73bcf27bb016b8e8ceb889"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"4f43bd06b6cc78dbd25a7475ca964eb1"},
	{ "stripped.txt",	GAME_STRFILE,	"d13753796bd81bf313a2449f34d8b112"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2WIN_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"608e277904d87dd28725fa08eacc2c0d"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"e749c4c103d7e7d51b34620ed76c5a04"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2WIN_DE_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"a76ea940076b5d9316796dea225a9b69"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"9609a933c541fed2e00c6c3479d7c181"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2WIN_DE2_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"9e858b3bb189c134c3a5f34c3385a8d3"},
	{ "icon.dat",		GAME_ICONFILE,	"ee92d1f84893195a60449f2430d07285"},
	{ "simon2.gme",		GAME_GMEFILE,	"16d574da07e93bcae43cee353dab8c7e"},
	{ "stripped.txt",	GAME_STRFILE,	"6de6292c9ac11bfb2e70fdb0f773ba85"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription SIMON2WIN_PL_GameFiles[] = {
	{ "gsptr30",		GAME_BASEFILE,	"657fd873f5d0637097ee02315b447e6f"},
	{ "icon.dat",		GAME_ICONFILE,	"72096a62d36e6034ea9fecc13b2dbdab"},
	{ "simon2.gme",		GAME_GMEFILE,	"7b9afcf82a94722707e0d025c0192be8"},
	{ "stripped.txt",	GAME_STRFILE,	"e229f84d46fa83f99b4a7115679f3fb6"},
	{ "tbllist",		GAME_TBLFILE,	"2082f8d02075e590300478853a91ffd9"},
};

static ADGameFileDescription FEEBLEFILES_AMI_DE_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "gfxindex.dat",	GAME_GFXIDXFILE,"f550f7915c5ce3a68c9f870f507449c2"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_AMI_UK_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "gfxindex.dat",	GAME_GFXIDXFILE,"f550f7915c5ce3a68c9f870f507449c2"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_MAC_DE_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_MAC_FR_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"ba90b40a47726039671d9e91630dd7ed"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_MAC_ES_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"71d7d2d5e479b053c5a9757f1702c9c3"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_MAC_UK_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "graphics.vga",	GAME_GFXIDXFILE,"11a4853cb35956846976e9473ee0e41e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_2CD_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"629762ea9ca9ee9ff85f4774d219f5c7"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_4CD_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"a8746407a5b20a7da0da0a14c380af1c"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_DE_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"bcd76ac080003eee3649df18db25b60e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_FR_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"ba90b40a47726039671d9e91630dd7ed"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_IT_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"80576f2e1ed4c912b63921fe77af313e"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription FEEBLEFILES_ES_GameFiles[] = {
	{ "game22",		GAME_BASEFILE,	"71d7d2d5e479b053c5a9757f1702c9c3"},
	{ "tbllist",		GAME_TBLFILE,	"0bbfee8e69739111eb36b0d138da8ddf"},
};

static ADGameFileDescription DIMP_GameFiles[] = {
	{ "Gdimp",		GAME_BASEFILE,	"0b1e89ae1dc2e012b7fa7a987b4ac42a"},
};

static ADGameFileDescription JUMBLE_GameFiles[] = {
	{ "Gjumble",		GAME_BASEFILE,	"d54cce46d339038d1a6b74ea213655bc"},
};

static ADGameFileDescription PUZZLE_GameFiles[] = {
	{ "Gpuzzle",		GAME_BASEFILE,	"3f80dac8e0d85401a1058a560fe49ab6"},
};

static ADGameFileDescription SWAMPY_GameFiles[] = {
	{ "Gswampy",		GAME_BASEFILE,	"3a6d4d7b2433e660f2483f9396cc87a2"},
};

static AGOSGameDescription gameDescriptions[] = {
	// Elvira - English Amiga Floppy
	{
		{
			"elvira1",
			"Floppy",
			ARRAYSIZE(ELVIRA1AMIGA_GameFiles),
			ELVIRA1AMIGA_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE | GF_CRUNCHED,
	},

	// Elvira - English Amiga Demo
	{
		{
			"elvira1",
			"Floppy",
			ARRAYSIZE(ELVIRA1AMIGADEMO_GameFiles),
			ELVIRA1AMIGADEMO_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_ELVIRA1,
		GID_ELVIRA1DEMO,
		GF_OLD_BUNDLE | GF_CRUNCHED,
	},

	// Elvira - English DOS Floppy
	{
		{
			"elvira1",
			"Floppy",
			ARRAYSIZE(ELVIRA1DOS_GameFiles),
			ELVIRA1DOS_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE,
	},

	// Elvira - English DOS Floppy Alternative?
	{
		{
			"elvira1",
			"Floppy",
			ARRAYSIZE(ELVIRA1DOS2_GameFiles),
			ELVIRA1DOS2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_ELVIRA1,
		GID_ELVIRA1,
		GF_OLD_BUNDLE,
	},

	// Elvira 2 - English Amiga Floppy
	{
		{
			"elvira2",
			"Floppy",
			ARRAYSIZE(ELVIRA2AMIGA_GameFiles),
			ELVIRA2AMIGA_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE | GF_CRUNCHED,
	},

	// Elvira 2 - English DOS Floppy
	{
		{
			"elvira2",
			"Floppy",
			ARRAYSIZE(ELVIRA2DOS_GameFiles),
			ELVIRA2DOS_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE,
	},

	// Elvira 2 - French DOS Floppy
	{
		{
			"elvira2",
			"Floppy",
			ARRAYSIZE(ELVIRA2DOS_FR_GameFiles),
			ELVIRA2DOS_FR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformPC,
		},

		GType_ELVIRA2,
		GID_ELVIRA2,
		GF_OLD_BUNDLE,
	},

	// Waxworks - English Amiga Floppy
	{
		{
			"waxworks",
			"Floppy",
			ARRAYSIZE(WAXWORKSAMIGA_GameFiles),
			WAXWORKSAMIGA_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE | GF_CRUNCHED,
	},

	// Waxworks - English DOS Floppy
	{
		{
			"waxworks",
			"Floppy",
			ARRAYSIZE(WAXWORKSDOS_GameFiles),
			WAXWORKSDOS_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_WW,
		GID_WAXWORKS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English Acorn CD Demo
	{
		{
			"simon1",
			"CD Demo",
			ARRAYSIZE(SIMON1ACORNDEMO_GameFiles),
			SIMON1ACORNDEMO_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAcorn,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - English Acorn CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1ACORN_GameFiles),
			SIMON1ACORN_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAcorn,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - English Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",
			ARRAYSIZE(SIMON1AMIGA_GameFiles),
			SIMON1AMIGA_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_SIMON1,
		GID_SIMON1AMIGA,
		GF_CRUNCHED | GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - French Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",
			ARRAYSIZE(SIMON1AMIGA_FR_GameFiles),
			SIMON1AMIGA_FR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformAmiga,
		},

		GType_SIMON1,
		GID_SIMON1AMIGA,
		GF_CRUNCHED | GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - German Amiga AGA Floppy
	{
		{
			"simon1",
			"AGA Floppy",
			ARRAYSIZE(SIMON1AMIGA_DE_GameFiles),
			SIMON1AMIGA_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformAmiga,
		},

		GType_SIMON1,
		GID_SIMON1AMIGA,
		GF_CRUNCHED | GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English Amiga ECS Demo
	{
		{
			"simon1",
			"ECS Demo",
			ARRAYSIZE(SIMON1AMIGADEMO_GameFiles),
			SIMON1AMIGADEMO_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_SIMON1,
		GID_SIMON1AMIGADEMO,
		GF_CRUNCHED | GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English Amiga CD32
	{
		{
			"simon1",
			"CD32",
			ARRAYSIZE(SIMON1CD32_GameFiles),
			SIMON1CD32_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_SIMON1,
		GID_SIMON1CD32,
		GF_TALKIE | GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English Amiga CD32 alternative?
	{
		{
			"simon1",
			"CD32",
			ARRAYSIZE(SIMON1CD32_2_GameFiles),
			SIMON1CD32_2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_SIMON1,
		GID_SIMON1CD32,
		GF_TALKIE | GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English DOS Floppy Demo
	{
		{
			"simon1",
			"Floppy Demo",
			ARRAYSIZE(SIMON1DEMO_GameFiles),
			SIMON1DEMO_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DEMO,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English DOS Floppy
	{
		{
			"simon1",
			"Floppy",
			ARRAYSIZE(SIMON1DOS_GameFiles),
			SIMON1DOS_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English DOS Floppy with Russian patch
	{
		{
			"simon1",
			"Floppy",
			ARRAYSIZE(SIMON1DOS_RU_GameFiles),
			SIMON1DOS_RU_GameFiles,
			Common::RU_RUS,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom)
	{
		{
			"simon1",
			"Floppy",
			ARRAYSIZE(SIMON1DOS_INF_GameFiles),
			SIMON1DOS_INF_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English DOS Floppy (Infocom) with Russian patch
	{
		{
			"simon1",
			"Floppy",
			ARRAYSIZE(SIMON1DOS_INF_RU_GameFiles),
			SIMON1DOS_INF_RU_GameFiles,
			Common::RU_RUS,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - French DOS Floppy
	{
		{
			"simon1",
			"Floppy",
			ARRAYSIZE(SIMON1DOS_FR_GameFiles),
			SIMON1DOS_FR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - German DOS Floppy
	{
		{
			"simon1",
			"Floppy",
			ARRAYSIZE(SIMON1DOS_DE_GameFiles),
			SIMON1DOS_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - Italian DOS Floppy
	{
		{
			"simon1",
			"Floppy",
			ARRAYSIZE(SIMON1DOS_IT_GameFiles),
			SIMON1DOS_IT_GameFiles,
			Common::IT_ITA,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - Spanish DOS Floppy
	{
		{
			"simon1",
			"Floppy",
			ARRAYSIZE(SIMON1DOS_ES_GameFiles),
			SIMON1DOS_ES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1DOS,
		GF_OLD_BUNDLE,
	},

	// Simon the Sorcerer 1 - English DOS CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1TALKIE_GameFiles),
			SIMON1TALKIE_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - English DOS CD alternate?
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1TALKIE2_GameFiles),
			SIMON1TALKIE2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - French DOS CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1TALKIE_FR_GameFiles),
			SIMON1TALKIE_FR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - German DOS CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1TALKIE_DE_GameFiles),
			SIMON1TALKIE_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - Hebrew DOS CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1TALKIE_HB_GameFiles),
			SIMON1TALKIE_HB_GameFiles,
			Common::HB_ISR,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - Italian DOS CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1TALKIE_IT_GameFiles),
			SIMON1TALKIE_IT_GameFiles,
			Common::IT_ITA,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - Italian DOS CD alternate
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1TALKIE_IT2_GameFiles),
			SIMON1TALKIE_IT2_GameFiles,
			// FIXME: DOS version which uses WAV format
			Common::IT_ITA,
			Common::kPlatformWindows,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - Spanish DOS CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1TALKIE_ES_GameFiles),
			SIMON1TALKIE_ES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformPC,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - English Windows CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1WIN_GameFiles),
			SIMON1WIN_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - English Windows CD with Russian patch
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1WIN_RU_GameFiles),
			SIMON1WIN_RU_GameFiles,
			Common::RU_RUS,
			Common::kPlatformWindows,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 1 - German Windows CD
	{
		{
			"simon1",
			"CD",
			ARRAYSIZE(SIMON1WIN_DE_GameFiles),
			SIMON1WIN_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformWindows,
		},

		GType_SIMON1,
		GID_SIMON1TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - English DOS Floppy
	{
		{
			"simon2",
			"Floppy",
			ARRAYSIZE(SIMON2DOS_GameFiles),
			SIMON2DOS_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0,
	},

	// Simon the Sorcerer 2 - English DOS Floppy with Russian patch
	{
		{
			"simon2",
			"Floppy",
			ARRAYSIZE(SIMON2DOS_RU_GameFiles),
			SIMON2DOS_RU_GameFiles,
			Common::RU_RUS,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0,
	},

	// Simon the Sorcerer 2 - English DOS Floppy alternate?
	{
		{
			"simon2",
			"Floppy",
			ARRAYSIZE(SIMON2DOS2_GameFiles),
			SIMON2DOS2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0,
	},

	// Simon the Sorcerer 2 - English DOS Floppy alternate? with Russian patch
	{
		{
			"simon2",
			"Floppy",
			ARRAYSIZE(SIMON2DOS2_RU_GameFiles),
			SIMON2DOS2_RU_GameFiles,
			Common::RU_RUS,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0,
	},

	// Simon the Sorcerer 2 - German DOS Floppy
	{
		{
			"simon2",
			"Floppy",
			ARRAYSIZE(SIMON2DOS_DE_GameFiles),
			SIMON2DOS_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0,
	},

	// Simon the Sorcerer 2 - Italian DOS Floppy
	{
		{
			"simon2",
			"Floppy",
			ARRAYSIZE(SIMON2DOS_IT_GameFiles),
			SIMON2DOS_IT_GameFiles,
			Common::IT_ITA,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2DOS,
		0,
	},

	// Simon the Sorcerer 2 - English DOS CD Demo
	{
		{
			"simon2",
			"CD Demo",
			ARRAYSIZE(SIMON2DEMO_GameFiles),
			SIMON2DEMO_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - English DOS CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2TALKIE_GameFiles),
			SIMON2TALKIE_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - English DOS CD alternate?
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2TALKIE2_GameFiles),
			SIMON2TALKIE2_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - French DOS CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2TALKIE_FR_GameFiles),
			SIMON2TALKIE_FR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - German DOS CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2TALKIE_DE_GameFiles),
			SIMON2TALKIE_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - German DOS CD alternate?
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2TALKIE_DE2_GameFiles),
			SIMON2TALKIE_DE2_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - Hebrew DOS CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2TALKIE_HB_GameFiles),
			SIMON2TALKIE_HB_GameFiles,
			Common::HB_ISR,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - Italian DOS CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2TALKIE_IT_GameFiles),
			SIMON2TALKIE_IT_GameFiles,
			// FIXME: DOS version which uses WAV format
			Common::IT_ITA,
			Common::kPlatformWindows,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - Spanish DOS CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2TALKIE_ES_GameFiles),
			SIMON2TALKIE_ES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformPC,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - English Windows CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2WIN_GameFiles),
			SIMON2WIN_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - German Windows CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2WIN_DE_GameFiles),
			SIMON2WIN_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformWindows,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - German Windows CD 1.1
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2WIN_DE2_GameFiles),
			SIMON2WIN_DE2_GameFiles,
			Common::DE_DEU,
			Common::kPlatformWindows,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// Simon the Sorcerer 2 - Polish Windows CD
	{
		{
			"simon2",
			"CD",
			ARRAYSIZE(SIMON2WIN_PL_GameFiles),
			SIMON2WIN_PL_GameFiles,
			Common::PL_POL,
			Common::kPlatformWindows,
		},

		GType_SIMON2,
		GID_SIMON2TALKIE,
		GF_TALKIE,
	},

	// The Feeble Files - English Amiga CD
	{
		{
			"feeble",
			"CD",
			ARRAYSIZE(FEEBLEFILES_AMI_UK_GameFiles),
			FEEBLEFILES_AMI_UK_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE,
	},

	// The Feeble Files - German Amiga CD
	{
		{
			"feeble",
			"CD",
			ARRAYSIZE(FEEBLEFILES_AMI_DE_GameFiles),
			FEEBLEFILES_AMI_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformAmiga,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE,
	},

	// The Feeble Files - English Macintosh CD
	{
		{
			"feeble",
			"CD",
			ARRAYSIZE(FEEBLEFILES_MAC_UK_GameFiles),
			FEEBLEFILES_MAC_UK_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE,
	},

	// The Feeble Files - French Macintosh CD
	{
		{
			"feeble",
			"CD",
			ARRAYSIZE(FEEBLEFILES_MAC_FR_GameFiles),
			FEEBLEFILES_MAC_FR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformMacintosh,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE,
	},

	// The Feeble Files - German Macintosh CD
	{
		{
			"feeble",
			"CD",
			ARRAYSIZE(FEEBLEFILES_MAC_DE_GameFiles),
			FEEBLEFILES_MAC_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformMacintosh,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE,
	},

	// The Feeble Files - Spanish Macintosh CD
	{
		{
			"feeble",
			"CD",
			ARRAYSIZE(FEEBLEFILES_MAC_ES_GameFiles),
			FEEBLEFILES_MAC_ES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformMacintosh,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_ZLIBCOMP | GF_TALKIE,
	},

	// The Feeble Files - English Windows 2CD
	{
		{
			"feeble",
			"2CD",
			ARRAYSIZE(FEEBLEFILES_2CD_GameFiles),
			FEEBLEFILES_2CD_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// The Feeble Files - English Windows 4CD
	{
		{
			"feeble",
			"4CD",
			ARRAYSIZE(FEEBLEFILES_4CD_GameFiles),
			FEEBLEFILES_4CD_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// The Feeble Files - French Windows 4CD
	{
		{
			"feeble",
			"4CD",
			ARRAYSIZE(FEEBLEFILES_FR_GameFiles),
			FEEBLEFILES_FR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformWindows,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// The Feeble Files - German Windows 4CD
	{
		{
			"feeble",
			"4CD",
			ARRAYSIZE(FEEBLEFILES_DE_GameFiles),
			FEEBLEFILES_DE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformWindows,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// The Feeble Files - Italian Windows 4CD
	{
		{
			"feeble",
			"4CD",
			ARRAYSIZE(FEEBLEFILES_IT_GameFiles),
			FEEBLEFILES_IT_GameFiles,
			Common::IT_ITA,
			Common::kPlatformWindows,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// The Feeble Files - Spanish Windows 4CD
	{
		{
			"feeble",
			"4CD",
			ARRAYSIZE(FEEBLEFILES_ES_GameFiles),
			FEEBLEFILES_ES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformWindows,
		},

		GType_FF,
		GID_FEEBLEFILES,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// Simon the Sorcerer's Puzzle Pack - Demon in my Pocket
	{
		{
			"dimp",
			"CD",
			ARRAYSIZE(DIMP_GameFiles),
			DIMP_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},

		GType_PP,
		GID_DIMP,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// Simon the Sorcerer's Puzzle Pack - Jumble
	{
		{
			"jumble",
			"CD",
			ARRAYSIZE(JUMBLE_GameFiles),
			JUMBLE_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},

		GType_PP,
		GID_JUMBLE,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// Simon the Sorcerer's Puzzle Pack - NoPatience
	{
		{
			"puzzle",
			"CD",
			ARRAYSIZE(PUZZLE_GameFiles),
			PUZZLE_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},

		GType_PP,
		GID_PUZZLE,
		GF_OLD_BUNDLE | GF_TALKIE,
	},

	// Simon the Sorcerer's Puzzle Pack - Swampy Adventures
	{
		{
			"swampy",
			"CD",
			ARRAYSIZE(SWAMPY_GameFiles),
			SWAMPY_GameFiles,
			Common::EN_ANY,
			Common::kPlatformWindows,
		},

		GType_PP,
		GID_SWAMPY,
		GF_OLD_BUNDLE | GF_TALKIE,
	},
};

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

	if (gameNumber >= gameCount || gameNumber == -1) {
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
