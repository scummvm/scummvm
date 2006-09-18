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
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/md5.h"

#include "base/plugins.h"

using namespace Kyra;

enum {
	// We only compute MD5 of the first megabyte of our data files.
	kMD5FileSizeLimit = 1024 * 1024
};

// Kyra MD5 detection brutally ripped from the Gobliins engine.
struct GameSettings {
	const char *gameid;
	const char *description;
	const char *md5sum;
	const char *checkFile;
	GameFlags flags;
};

struct Kyra1LanguageTable {
	const char *file;
	Common::Language language;
};

namespace {
const GameSettings kyra_games[] = {
	// floppy versions
	// english
	{ "kyra1", "The Legend of Kyrandia", "3c244298395520bb62b5edfe41688879", "GEMCUT.EMC",
		{ Common::EN_ANY, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA1 } },
	{ "kyra1", "The Legend of Kyrandia", "796e44863dd22fa635b042df1bf16673", "GEMCUT.EMC",
		{ Common::EN_ANY, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA1 } },
	// french
	{ "kyra1", "The Legend of Kyrandia", "abf8eb360e79a6c2a837751fbd4d3d24", "GEMCUT.EMC",
		{ Common::FR_FRA, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA1 } },
	// german
	{ "kyra1", "The Legend of Kyrandia", "6018e1dfeaca7fe83f8d0b00eb0dd049", "GEMCUT.EMC",
		{ Common::DE_DEU, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA1 } },
	{ "kyra1", "The Legend of Kyrandia", "f0b276781f47c130f423ec9679fe9ed9", "GEMCUT.EMC", // from Arne.F
		{ Common::DE_DEU, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA1 } },
	// spanish
	{ "kyra1", "The Legend of Kyrandia", "8909b41596913b3f5deaf3c9f1017b01", "GEMCUT.EMC", // from VooD
		{ Common::ES_ESP, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA1 } },
	{ "kyra1", "The Legend of Kyrandia", "747861d2a9c643c59fdab570df5b9093", "GEMCUT.EMC", // floppy 1.8 from clemmy
		{ Common::ES_ESP, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA1 } },
	// italian
	{ "kyra1", "The Legend of Kyrandia", "ef08c8c237ee1473fd52578303fc36df", "GEMCUT.EMC", // from gourry
		{ Common::IT_ITA, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA1 } },

	//{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_AMIGA | GF_FLOPPY | GF_ENGLISH,
	//									"2bd1da653eaefd691e050e4a9eb68a64", "GEMCUT.PAK" },
	
	// CD versions
	// english
	{ "kyra1", "The Legend of Kyrandia", "fac399fe62f98671e56a005c5e94e39f", "GEMCUT.PAK",
		{ Common::EN_ANY, Common::kPlatformPC, 0, 1, 0, 1, GI_KYRA1 } },
	// german
	{ "kyra1", "The Legend of Kyrandia", "230f54e6afc007ab4117159181a1c722", "GEMCUT.PAK",
		{ Common::DE_DEU, Common::kPlatformPC, 0, 1, 0, 1, GI_KYRA1 } },
	// french
	{ "kyra1", "The Legend of Kyrandia", "b037c41768b652a040360ffa3556fd2a", "GEMCUT.PAK",
		{ Common::FR_FRA, Common::kPlatformPC, 0, 1, 0, 1, GI_KYRA1 } },

	// demo versions
	// english
	{ "kyra1", "The Legend of Kyrandia Demo", "fb722947d94897512b13b50cc84fd648", "DEMO1.WSA",
		{ Common::EN_ANY, Common::kPlatformPC, 1, 0, 0, 0, GI_KYRA1 } },

	// kyra 2 games
	{ "kyra2", "The Legend of Kyrandia: The Hand of Fate", "28cbad1c5bf06b2d3825ae57d760d032", "FATE.PAK",
		{ Common::UNK_LANG, Common::kPlatformPC, 0, 0, 0, 0, GI_KYRA2 } }, // talkie version? fixed language version?
	
	// kyra 3 games
	{ "kyra3", "The Legend of Kyrandia: Malcolm's Revenge",	"3833ff312757b8e6147f464cca0a6587", "ONETIME.PAK",
		{ Common::UNK_LANG, Common::kPlatformPC, 0, 0, 0, 1, GI_KYRA3 } },

	{ 0, 0, 0, 0, { Common::UNK_LANG, Common::kPlatformUnknown, 0, 0, 0, 0, 0 }  }
};

// Keep list of different supported games
const PlainGameDescriptor kyra_list[] = {
	{ "kyra1", "The Legend of Kyrandia" },
	{ "kyra2", "The Legend of Kyrandia: The Hand of Fate" },
	{ "kyra3", "The Legend of Kyrandia: Malcolm's Revenge" },
	{ 0, 0 }
};

const Kyra1LanguageTable kyra1_languages[] = {
	{ "MAIN_FRE.CPS", Common::FR_FRA },
	{ "MAIN_GER.CPS", Common::DE_DEU },
	{ "MAIN_SPA.CPS", Common::ES_ESP },
	{ "MAIN_ITA.CPS", Common::IT_ITA },

	// default language
	{ "MAIN15.CPS", Common::EN_ANY },
	{ "MAIN_ENG.CPS", Common::EN_ANY },

	{ 0, Common::UNK_LANG }
};

const char *getKyraVersion(const GameFlags &flags) {
	if (flags.isTalkie) {
		return "CD";
	}

	return 0;
}
} // End of anonymous namespace

GameList Engine_KYRA_gameIDList() {
	GameList games;
	const PlainGameDescriptor *g = kyra_list;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}
	return games;
}

GameDescriptor Engine_KYRA_findGameID(const char *gameid) {
	const PlainGameDescriptor *g = kyra_list;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return *g;
}

DetectedGameList Engine_KYRA_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	const GameSettings *g;
	FSList::const_iterator file;

	// Iterate over all files in the given directory
	bool isFound = false;
	for (file = fslist.begin(); file != fslist.end(); file++) {
		if (file->isDirectory())
			continue;

		for (g = kyra_games; g->gameid; g++) {
			if (scumm_stricmp(file->name().c_str(), g->checkFile) == 0)
				isFound = true;
		}

		if (isFound)
			break;
	}

	if (file == fslist.end())
		return detectedGames;

	uint8 md5sum[16];
	char md5str[32 + 1];

	if (Common::md5_file(*file, md5sum, kMD5FileSizeLimit)) {
		for (int i = 0; i < 16; i++) {
			sprintf(md5str + i * 2, "%02x", (int)md5sum[i]);
		}

		for (g = kyra_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				DetectedGame dg(*g, g->flags.lang, g->flags.platform);
				dg.updateDesc(getKyraVersion(g->flags));

				detectedGames.push_back(dg);
			}
		}

		if (detectedGames.empty()) {
			printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);

			const PlainGameDescriptor *g1 = kyra_list;
			while (g1->gameid) {
				detectedGames.push_back(*g1);
				g1++;
			}
		}
	}
	return detectedGames;
}

PluginError Engine_KYRA_create(OSystem *syst, Engine **engine) {
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();

	if (!scumm_stricmp("kyra1", gameid)) {
		*engine = new KyraEngine_v1(syst);
	} else if (!scumm_stricmp("kyra2", gameid)) {
		*engine = new KyraEngine_v2(syst);
	} else if (!scumm_stricmp("kyra3", gameid)) {
		*engine = new KyraEngine_v3(syst);
	} else
		error("Kyra engine created with invalid gameid.");

	FSList fslist;
	FilesystemNode dir(ConfMan.get("path"));
	if (!dir.listDir(fslist, FilesystemNode::kListFilesOnly)) {
		warning("KyraEngine: invalid game path '%s'", dir.path().c_str());
		return kInvalidPathError;
	}

	if (*engine) {
		if (((KyraEngine*)(*engine))->setupGameFlags()) {
			warning("KyraEngine: unable to locate game data at path '%s'", dir.path().c_str());
			delete *engine;
			return kNoGameDataFoundError;
		}
	}
	
	return kNoError;
}

REGISTER_PLUGIN(KYRA, "Legend of Kyrandia Engine", "The Legend of Kyrandia (C) Westwood Studios");

#pragma mark -

int KyraEngine_v1::setupGameFlags() {
	// Detect game features based on MD5. Again brutally ripped from Gobliins.
	uint8 md5sum[16];
	char md5str[32 + 1];

	const GameSettings *g;
	bool versionFound = false;
	bool fileFound = false;

	memset(md5str, 0, sizeof(md5str));
	for (g = kyra_games; g->gameid; g++) {
		if (scumm_stricmp(g->gameid, "kyra1"))
			continue;
		if (!Common::File::exists(g->checkFile))
			continue;

		fileFound = true;
		
		if (Common::md5_file(g->checkFile, md5sum, kMD5FileSizeLimit)) {
			for (int j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
		} else
			continue;

		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			_flags = g->flags;

			if (g->description)
				g_system->setWindowCaption(g->description);

			versionFound = true;
			break;
		}
	}

	if (fileFound) {
		if (!versionFound) {
			printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);
			_flags.gameID = GI_KYRA1;
			if (Common::File::exists("INTRO.VRM")) {
				_flags.isTalkie = 1;
				_flags.useAltShapeHeader = 1;
			}
		
			// try to detect the language
			const Kyra1LanguageTable *lang = kyra1_languages;
			for (; lang->file; ++lang) {
				if (Common::File::exists(lang->file)) {
					_flags.lang = lang->language;
					versionFound = true;
					break;
				}
			}
		
			if (!versionFound) {
				_flags.lang = Common::UNK_LANG;
			}
		}
	} else {
		GUIErrorMessage("No version of Kyrandia found in specified directory.");
		return -1;
	}


	// if the user says we got a macintosh version, then we assume to have one
	// since it's currently not possible to detect the macintosh version
	// because of limitations of the current detector code
	if (Common::parsePlatform(ConfMan.get("platform")) == Common::kPlatformMacintosh) {
		_flags.platform = Common::kPlatformMacintosh;
	}

	return 0;
}

