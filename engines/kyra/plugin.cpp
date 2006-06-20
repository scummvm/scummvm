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

#include "backends/fs/fs.h"

#include "common/config-manager.h"
#include "common/file.h"
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
	byte id;
	uint32 features;
	const char *md5sum;
	const char *checkFile;
};

struct Kyra1LanguageTable {
	const char *file;
	uint32 language;
	Common::Language detLanguage;
};

namespace {
const GameSettings kyra1_games[] = {
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_ENGLISH | GF_FLOPPY, // english floppy 1.0 from Malice
										"3c244298395520bb62b5edfe41688879", "GEMCUT.EMC" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_ENGLISH | GF_FLOPPY, 
										"796e44863dd22fa635b042df1bf16673", "GEMCUT.EMC" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_FRENCH | GF_FLOPPY,
										"abf8eb360e79a6c2a837751fbd4d3d24", "GEMCUT.EMC" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_GERMAN | GF_FLOPPY, 
										"6018e1dfeaca7fe83f8d0b00eb0dd049", "GEMCUT.EMC"},
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_GERMAN | GF_FLOPPY, // from Arne.F 
										"f0b276781f47c130f423ec9679fe9ed9", "GEMCUT.EMC"},
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_SPANISH | GF_FLOPPY, // from VooD
										"8909b41596913b3f5deaf3c9f1017b01", "GEMCUT.EMC"},
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_SPANISH | GF_FLOPPY, // floppy 1.8 from clemmy
										"747861d2a9c643c59fdab570df5b9093", "GEMCUT.EMC"},
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_ITALIAN | GF_FLOPPY, // from gourry
										"ef08c8c237ee1473fd52578303fc36df", "GEMCUT.EMC" },
	//{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_AMIGA | GF_FLOPPY | GF_ENGLISH,
	//									"2bd1da653eaefd691e050e4a9eb68a64", "GEMCUT.PAK" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_ENGLISH | GF_TALKIE, 
										"fac399fe62f98671e56a005c5e94e39f", "GEMCUT.PAK" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_GERMAN | GF_TALKIE, 
										"230f54e6afc007ab4117159181a1c722", "GEMCUT.PAK" },
	{ "kyra1", "The Legend of Kyrandia",		GI_KYRA1, GF_FRENCH | GF_TALKIE, 
										"b037c41768b652a040360ffa3556fd2a", "GEMCUT.PAK" },
	{ "kyra1", "The Legend of Kyrandia Demo",	GI_KYRA1, GF_DEMO | GF_ENGLISH,
										"fb722947d94897512b13b50cc84fd648", "DEMO1.WSA" },
	{ 0, 0, 0, 0, 0, 0 }
};

#if 0
const GameSettings kyra2_games[] = {
	{ "kyra2", "The Hand of Fate",				GI_KYRA2, GF_ENGLISH,	// CD version? Floppy version?
										"28cbad1c5bf06b2d3825ae57d760d032", "FATE.PAK" },
	{ 0, 0, 0, 0, 0, 0 }
};

const GameSettings kyra3_games[] = {
	{ "kyra3", "The Legend of Kyrandia: Book Three",
	GI_KYRA3, GF_LNGUNK, "3833ff312757b8e6147f464cca0a6587", "ONETIME.PAK" },
	{ 0, 0, 0, 0, 0, 0 }
};
#endif

// Keep list of different supported games
const PlainGameDescriptor kyra_list[] = {
	{ "kyra1", "The Legend of Kyrandia" },
	//{ "kyra2", "The Hand of Fate" },
	//{ "kyra3", "The Legend of Kyrandia: Book Three" },
	{ 0, 0 }
};

const Kyra1LanguageTable kyra1_languages[] = {
	{ "MAIN_FRE.CPS", GF_FRENCH, Common::FR_FRA },
	{ "MAIN_GER.CPS", GF_GERMAN, Common::DE_DEU },
	{ "MAIN_SPA.CPS", GF_SPANISH, Common::ES_ESP },
	{ "MAIN_ITA.CPS", GF_ITALIAN, Common::IT_ITA },

	// default language
	{ "MAIN15.CPS", GF_ENGLISH, Common::EN_ANY },
	{ "MAIN_ENG.CPS", GF_ENGLISH, Common::EN_ANY },

	{ 0, 0, Common::UNK_LANG }
};

Common::Language convertKyraLang(uint32 features) {
	if (features & GF_ENGLISH) {
		return Common::EN_ANY;
	} else if (features & GF_FRENCH) {
		return Common::FR_FRA;
	} else if (features & GF_GERMAN) {
		return Common::DE_DEU;
	} else if (features & GF_SPANISH) {
		return Common::ES_ESP;
	}
	return Common::UNK_LANG;
}

Common::Platform convertKyraPlatform(uint32 features) {
	if (features & GF_AUDIOCD) {
		return Common::kPlatformFMTowns;
	} else if (features & GF_AMIGA) {
		return Common::kPlatformAmiga;
	}

	return Common::kPlatformPC;
}

const char *getKyraVersion(uint32 features) {
	if (features & GF_TALKIE) {
		return "CD";
	}

	return 0;
}
} // End of anonymous namespace

using namespace Kyra;

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

		// TODO: cleanup
		for (g = kyra1_games; g->gameid; g++) {
			if (scumm_stricmp(file->displayName().c_str(), g->checkFile) == 0)
				isFound = true;
		}

		if (isFound)
			break;

#if 0
		for (g = kyra2_games; g->gameid; g++) {
			if (scumm_stricmp(file->displayName().c_str(), g->checkFile) == 0)
				isFound = true;
		}

		if (isFound)
			break;
		
		for (g = kyra3_games; g->gameid; g++) {
			if (scumm_stricmp(file->displayName().c_str(), g->checkFile) == 0)
				isFound = true;
		}

		if (isFound)
			break;
#endif
	}

	if (file == fslist.end())
		return detectedGames;

	uint8 md5sum[16];
	char md5str[32 + 1];

	if (Common::md5_file(file->path().c_str(), md5sum, kMD5FileSizeLimit)) {
		for (int i = 0; i < 16; i++) {
			sprintf(md5str + i * 2, "%02x", (int)md5sum[i]);
		}

		// TODO: cleanup
		for (g = kyra1_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				DetectedGame dg(*g, convertKyraLang(g->features), convertKyraPlatform(g->features));
				dg.updateDesc(getKyraVersion(g->features));

				detectedGames.push_back(dg);
			}
		}

#if 0
		for (g = kyra2_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				DetectedGame dg(*g, convertKyraLang(g->features), convertKyraPlatform(g->features));
				dg.updateDesc(getKyraVersion(g->features));

				detectedGames.push_back(dg);
			}
		}
		
		for (g = kyra3_games; g->gameid; g++) {
			if (strcmp(g->md5sum, (char *)md5str) == 0) {
				DetectedGame dg(*g, convertKyraLang(g->features), convertKyraPlatform(g->features));
				dg.updateDesc(getKyraVersion(g->features));

				detectedGames.push_back(dg);
			}
		}
#endif

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
#if 0
	} else if (!scumm_stricmp("kyra2", gameid)) {
		*engine = new KyraEngine_v2(syst);
	} else if (!scumm_stricmp("kyra3", gameid)) {
		*engine = new KyraEngine_v3(syst);
#endif
	} else
		error("Kyra engine created with invalid gameid.");
	
	return kNoError;
}

REGISTER_PLUGIN(KYRA, "Legend of Kyrandia Engine");

#pragma mark -

int KyraEngine_v1::setupGameFlags() {
	// Detect game features based on MD5. Again brutally ripped from Gobliins.
	uint8 md5sum[16];
	char md5str[32 + 1];

	const GameSettings *g;
	bool versionFound = false;
	bool fileFound = false;

	_features = 0;
	memset(md5str, 0, sizeof(md5str));
	for (g = kyra1_games; g->gameid; g++) {
		Common::File temp;
		temp.open(g->checkFile);
		if (!temp.isOpen())
			continue;
		//if (!Common::File::exists(g->checkFile))
		//	continue;

		fileFound = true;
		
		if (Common::md5_file(g->checkFile, md5sum, kMD5FileSizeLimit)) {
			for (int j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
		} else
			continue;

		if (strcmp(g->md5sum, (char *)md5str) == 0) {
			_features = g->features;
			_game = g->id;

			if (g->description)
				g_system->setWindowCaption(g->description);

			versionFound = true;
			break;
		}
	}

	if (fileFound) {
		if (!versionFound) {
			printf("Unknown MD5 (%s)! Please report the details (language, platform, etc.) of this game to the ScummVM team\n", md5str);
			_features = 0;
			_game = GI_KYRA1;
			Common::File temp;
			temp.open(g->checkFile);
			if (temp.isOpen())
			//if (Common::File::exists("INTRO.VRM")) {
				_features |= GF_TALKIE;
			} else {
				_features |= GF_FLOPPY;
			}
			temp.close();
		
			// try to detect the language
			const Kyra1LanguageTable *lang = kyra1_languages;
			for (; lang->file; ++lang) {
				temp.open(g->checkFile);
				if (temp.isOpen())
				//if (Common::File::exists(lang->file)) {
					_features |= lang->language;
					versionFound = true;
					break;
				}
				temp.close();
			}
		
			if (!versionFound) {
				_features |= GF_LNGUNK;
			}
		}
	} else {
		GUIErrorMessage("No version of Kyrandia found in specified directory.");
		return -1;
	}

	return 0;
}

