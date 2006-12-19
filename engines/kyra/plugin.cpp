/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "kyra/kyra.h"
#include "kyra/kyra2.h"
#include "kyra/kyra3.h"

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/advancedDetector.h"

#include "base/plugins.h"

using namespace Kyra;
using namespace Common;

enum {
	// We only compute MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

struct KYRAGameDescription {
	ADGameDescription desc;

	const char *id;
	GameFlags flags;
};

namespace {

#define ENTRY(f, x) { f, 0, x } 

const ADGameFileDescription kyra1EnglishFloppy1[] = {
	ENTRY("GEMCUT.EMC", "3c244298395520bb62b5edfe41688879")
};

const ADGameFileDescription kyra1EnglishFloppy2[] = {
	ENTRY("GEMCUT.EMC", "796e44863dd22fa635b042df1bf16673")
};

const ADGameFileDescription kyra1FrenchFloppy1[] = {
	ENTRY("GEMCUT.EMC", "abf8eb360e79a6c2a837751fbd4d3d24")
};

const ADGameFileDescription kyra1GermanFloppy1[] = {
	ENTRY("GEMCUT.EMC", "6018e1dfeaca7fe83f8d0b00eb0dd049")
};

const ADGameFileDescription kyra1GermanFloppy2[] = {
	ENTRY("GEMCUT.EMC", "f0b276781f47c130f423ec9679fe9ed9")
};

const ADGameFileDescription kyra1SpanishFloppy1[] = {
	ENTRY("GEMCUT.EMC", "8909b41596913b3f5deaf3c9f1017b01")
};

const ADGameFileDescription kyra1SpanishFloppy2[] = {
	ENTRY("GEMCUT.EMC", "747861d2a9c643c59fdab570df5b9093")
};

const ADGameFileDescription kyra1ItalianFloppy1[] = {
	ENTRY("GEMCUT.EMC", "ef08c8c237ee1473fd52578303fc36df")
};

const ADGameFileDescription kyra1EnglishCD1[] = {
	ENTRY("GEMCUT.PAK", "fac399fe62f98671e56a005c5e94e39f")
};

const ADGameFileDescription kyra1GermanCD1[] = {
	ENTRY("GEMCUT.PAK", "230f54e6afc007ab4117159181a1c722")
};

const ADGameFileDescription kyra1FrenchCD1[] = {
	ENTRY("GEMCUT.PAK", "b037c41768b652a040360ffa3556fd2a")
};

const ADGameFileDescription kyra1EnglishDemo1[] = {
	ENTRY("DEMO1.WSA", "fb722947d94897512b13b50cc84fd648")
};

const ADGameFileDescription kyra2UnknownUnknown1[] = {
	ENTRY("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032")
};

const ADGameFileDescription kyra3CD1[] = {
	ENTRY("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587")
};

#define FLAGS(x, y, z, w, id) { UNK_LANG, kPlatformUnknown, x, y, z, w, id }

#define KYRA1_FLOPPY_FLAGS FLAGS(false, false, false, false, GI_KYRA1)
#define KYRA1_CD_FLAGS FLAGS(false, true, false, true, GI_KYRA1)
#define KYRA1_DEMO_FLAGS FLAGS(true, false, false, false, GI_KYRA1)

#define KYRA2_UNK_FLAGS FLAGS(false, false, false, false, GI_KYRA2)

#define KYRA3_CD_FLAGS FLAGS(false, false, false, true, GI_KYRA3)

const KYRAGameDescription adGameDescs[] = {
	{ { "The Legend of Kyrandia", 0, ARRAYSIZE(kyra1EnglishFloppy1), kyra1EnglishFloppy1, EN_ANY, kPlatformPC }, "kyra1", KYRA1_FLOPPY_FLAGS },
	{ { "The Legend of Kyrandia", 0, ARRAYSIZE(kyra1EnglishFloppy2), kyra1EnglishFloppy2, EN_ANY, kPlatformPC }, "kyra1", KYRA1_FLOPPY_FLAGS },
	{ { "The Legend of Kyrandia", 0, ARRAYSIZE(kyra1FrenchFloppy1) , kyra1FrenchFloppy1 , FR_FRA, kPlatformPC }, "kyra1", KYRA1_FLOPPY_FLAGS },
	{ { "The Legend of Kyrandia", 0, ARRAYSIZE(kyra1GermanFloppy1) , kyra1GermanFloppy1 , DE_DEU, kPlatformPC }, "kyra1", KYRA1_FLOPPY_FLAGS },
	{ { "The Legend of Kyrandia", 0, ARRAYSIZE(kyra1GermanFloppy2) , kyra1GermanFloppy2 , DE_DEU, kPlatformPC }, "kyra1", KYRA1_FLOPPY_FLAGS }, // from Arne.F
	{ { "The Legend of Kyrandia", 0, ARRAYSIZE(kyra1SpanishFloppy1), kyra1SpanishFloppy1, ES_ESP, kPlatformPC }, "kyra1", KYRA1_FLOPPY_FLAGS }, // from VooD
	{ { "The Legend of Kyrandia", 0, ARRAYSIZE(kyra1SpanishFloppy2), kyra1SpanishFloppy2, ES_ESP, kPlatformPC }, "kyra1", KYRA1_FLOPPY_FLAGS }, // floppy 1.8 from clemmy
	{ { "The Legend of Kyrandia", 0, ARRAYSIZE(kyra1ItalianFloppy1), kyra1ItalianFloppy1, IT_ITA, kPlatformPC }, "kyra1", KYRA1_FLOPPY_FLAGS }, // from gourry

	{ { "The Legend of Kyrandia", "CD", ARRAYSIZE(kyra1EnglishCD1), kyra1EnglishCD1, EN_ANY, kPlatformPC }, "kyra1", KYRA1_CD_FLAGS },
	{ { "The Legend of Kyrandia", "CD", ARRAYSIZE(kyra1GermanCD1) , kyra1GermanCD1 , DE_DEU, kPlatformPC }, "kyra1", KYRA1_CD_FLAGS },
	{ { "The Legend of Kyrandia", "CD", ARRAYSIZE(kyra1FrenchCD1) , kyra1FrenchCD1 , FR_FRA, kPlatformPC }, "kyra1", KYRA1_CD_FLAGS },

	{ { "The Legend of Kyrandia", "Demo", ARRAYSIZE(kyra1EnglishDemo1), kyra1EnglishDemo1, EN_ANY, kPlatformPC }, "kyra1", KYRA1_DEMO_FLAGS },

	{ { "The Legend of Kyrandia: The Hand of Fate", 0, ARRAYSIZE(kyra2UnknownUnknown1), kyra2UnknownUnknown1, UNK_LANG, kPlatformPC }, "kyra2", KYRA2_UNK_FLAGS }, // check this! (cd version?)

	{ { "The Legend of Kyrandia: Malcolm's Revenge", 0, ARRAYSIZE(kyra3CD1), kyra3CD1, UNK_LANG, kPlatformPC }, "kyra3", KYRA3_CD_FLAGS },
	{ { NULL, NULL, 0, NULL, UNK_LANG, kPlatformUnknown }, NULL, KYRA2_UNK_FLAGS }
};

ADGameDescList getADDescList() {
	ADGameDescList gameDesc;

	for (int i = 0; i < ARRAYSIZE(adGameDescs) - 1; ++i) {
		gameDesc.push_back(&adGameDescs[i].desc);
	}

	return gameDesc;
}

ADList detectKyraGames(const FSList &fslist) {
	AdvancedDetector ad;

	ad.registerGameDescriptions(getADDescList());
	ad.setFileMD5Bytes(kMD5FileSizeLimit);

	ADList list = ad.detectGame(&fslist, Common::UNK_LANG, Common::kPlatformUnknown);
	return list;
}

bool setupGameFlags(const ADList &list, GameFlags &flags) {
	if (!list.size()) {
		// maybe add non md5 based detection again?
		return false;
	}

	int id = list[0];

	if (list.size() > 1) {
		int filesCount = 0;
		int curID = 0;
		// get's the entry which has most files to check (most specialized)
		for (ADList::const_iterator i = list.begin(); i != list.end(); ++i, ++curID) {
			if (filesCount < adGameDescs[*i].desc.filesCount) {
				filesCount = adGameDescs[*i].desc.filesCount;
				id = curID;
			}
		}
	}

	flags = adGameDescs[id].flags;

	Platform platform = parsePlatform(ConfMan.get("platform"));
	if (platform != kPlatformUnknown) {
		flags.platform = platform;
	}
	Language lang = parseLanguage(ConfMan.get("language"));
	if (lang != UNK_LANG && flags.lang != UNK_LANG) {
		flags.lang = lang;
	}

	if (flags.lang == UNK_LANG) {
		flags.lang = EN_ANY;
	}

	return true;
}

const PlainGameDescriptor gameList[] = {
	{ "kyra1", "The Legend of Kyrandia" },
	{ "kyra2", "The Legend of Kyrandia: The Hand of Fate" },
	{ "kyra3", "The Legend of Kyrandia: Malcolm's Revenge" },
	{ 0, 0 }
};

} // End of anonymous namespace

GameList Engine_KYRA_gameIDList() {
	return Common::real_ADVANCED_DETECTOR_GAMEID_LIST(gameList);
}

GameDescriptor Engine_KYRA_findGameID(const char *gameid) {
	return Common::real_ADVANCED_DETECTOR_FIND_GAMEID(gameid, gameList, 0);
}

DetectedGameList Engine_KYRA_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	ADList games = detectKyraGames(fslist);

	for (ADList::const_iterator pos = games.begin(); pos != games.end(); ++pos) {
		DetectedGame game(adGameDescs[*pos].id, adGameDescs[*pos].desc.name, adGameDescs[*pos].desc.language, adGameDescs[*pos].desc.platform);
		game.updateDesc(adGameDescs[*pos].desc.extra);
		detectedGames.push_back(game);
	}

	return detectedGames;
}

PluginError Engine_KYRA_create(OSystem *syst, Engine **engine) {
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		return kInvalidPathError;
	}

	GameFlags flags;
	ADList games = detectKyraGames(fslist);
	if (!setupGameFlags(games, flags)) {
		return kNoGameDataFoundError;
	}

	if (!scumm_stricmp("kyra1", gameid)) {
		*engine = new KyraEngine_v1(syst, flags);
	} else if (!scumm_stricmp("kyra2", gameid)) {
		*engine = new KyraEngine_v2(syst, flags);
	} else if (!scumm_stricmp("kyra3", gameid)) {
		*engine = new KyraEngine_v3(syst, flags);
	} else
		error("Kyra engine created with invalid gameid.");

	return kNoError;
}

REGISTER_PLUGIN(KYRA, "Legend of Kyrandia Engine", "The Legend of Kyrandia (C) Westwood Studios");
