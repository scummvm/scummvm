/* ScummVM - Scumm Interpreter
 * Copyright (C) 2007 The ScummVM project
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

#include "common/advancedDetector.h"

#include "gob/gob.h"

namespace Gob {

struct GOBGameDescription {
	Common::ADGameDescription desc;

	uint32 features;
	const char *startTotBase;
};

static GameList GAME_detectGames(const FSList &fslist);
}

using namespace Common;

static const PlainGameDescriptor gobGames[] = {
	{"gob", "Gob engine game"},
	{"gob1", "Gobliiins"},
	{"gob2", "Gobliins 2"},
	{"gob3", "Goblins Quest 3"},
	{"bargon", "Bargon Attack"},
	{"ween", "Ween: The Prohpecy"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble"},
	{0, 0}
};

static const ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"gob1", "gob", kPlatformUnknown},
	{"gob2", "gob", kPlatformUnknown},
	{0, 0, kPlatformUnknown}
};

namespace Gob {

#define ENTRY(id,extra,flags,lang,platform,md5,tot) \
	{ \
		{ \
			id, \
			extra, \
			AD_ENTRY1("intro.stk", md5), \
			lang, \
			platform, \
		}, \
		flags, \
		tot \
	}

static const GOBGameDescription gameDescriptions[] = {
	// Supplied by Florian Zeitz on scummvm-devel
	ENTRY("gob1", "EGA", GF_GOB1 | GF_EGA, UNK_LANG, kPlatformPC, "c65e9cc8ba23a38456242e1f2b1caad4", "intro"),
	ENTRY("gob1", "EGA", GF_GOB1 | GF_EGA, RU_RUS, kPlatformPC, "f9233283a0be2464248d83e14b95f09c", "intro"),
	//{"gob1", "Gobliiins (Windows)", GF_GOB1, UNK_LANG, "8a5e850c49d7cacdba5f5eb1fcc77b89", "intro"},

	// Supplied by Theruler76 in bug report #1201233
	ENTRY("gob1", "VGA", GF_GOB1, UNK_LANG, kPlatformPC, "26a9118c0770fa5ac93a9626761600b2", "intro"),

	// CD 1.000 version. Multilingual
	ENTRY("gob1", "CD 1.000", GF_GOB1 | GF_CD, UNK_LANG, kPlatformPC, "2fbf4b5b82bbaee87eb45d4404c28998", "intro"),
	// CD 1.02 version. Multilingual
	ENTRY("gob1", "CD 1.02", GF_GOB1 | GF_CD, UNK_LANG, kPlatformPC, "8bd873137b6831c896ee8ad217a6a398", "intro"),

	ENTRY("gob1", "", GF_GOB1 | GF_AMIGA, UNK_LANG, kPlatformAmiga, "972f22c6ff8144a6636423f0354ca549", "intro"),

	ENTRY("gob1", "Interactive Demo", GF_GOB1, UNK_LANG, kPlatformPC, "e72bd1e3828c7dec4c8a3e58c48bdfdb", "intro"),
	
	ENTRY("gob1", "", GF_GOB1 | GF_MAC, UNK_LANG, kPlatformMacintosh, "00a42a7d2d22e6b6ab1b8c673c4ed267", "intro"),

	ENTRY("gob2", "", GF_GOB2, FR_FRA, kPlatformPC, "a13ecb4f6d8fd881ebbcc02e45cb5475", "intro"),
	ENTRY("gob2", "", GF_GOB2, EN_GRB, kPlatformPC, "b45b984ee8017efd6ea965b9becd4d66", "intro"),
	ENTRY("gob2", "", GF_GOB2, EN_USA, kPlatformPC, "dedb5d31d8c8050a8cf77abedcc53dae", "intro"),
	ENTRY("gob2", "", GF_GOB2, DE_DEU, kPlatformPC, "a13892cdf4badda85a6f6fb47603a128", "intro"),
	ENTRY("gob2", "", GF_GOB2, RU_RUS, kPlatformPC, "cd3e1df8b273636ee32e34b7064f50e8", "intro"),

	ENTRY("gob2", "", GF_GOB2 | GF_AMIGA, DE_DEU, kPlatformAmiga, "d28b9e9b41f31acfa58dcd12406c7b2c", "intro"),

	// Supplied by blackwhiteeagle in bug report #1605235
	ENTRY("gob2", "", GF_GOB2, DE_DEU, kPlatformPC, "3e4e7db0d201587dd2df4003b2993ef6", "intro"),

	// CD 1.000.
	ENTRY("gob2", "CD 1.000", GF_GOB2 | GF_CD, EN_USA, kPlatformPC, "9de5fbb41cf97182109e5fecc9d90347", "intro"),
	// CD 1.01
	ENTRY("gob2", "CD 1.02", GF_GOB2 | GF_CD, UNK_LANG, kPlatformPC, "24a6b32757752ccb1917ce92fd7c2a04", "intro"),

	ENTRY("gob2", "Demo", GF_GOB2, UNK_LANG, kPlatformPC, "8b1c98ff2ab2e14f47a1b891e9b92217", "usa"),
	ENTRY("gob2", "Interactive Demo", GF_GOB2, UNK_LANG, kPlatformPC, "cf1c95b2939bd8ff58a25c756cb6125e", "intro"),
	ENTRY("gob2", "Interactive Demo", GF_GOB2 | GF_AMIGA, UNK_LANG, kPlatformPC, "4b278c2678ea01383fd5ca114d947eea", "intro"),

	ENTRY("ween", "", GF_GOB2, UNK_LANG, kPlatformPC, "2bb8878a8042244dd2b96ff682381baa", "intro"),
	ENTRY("ween", "", GF_GOB2, UNK_LANG, kPlatformPC, "4b10525a3782aa7ecd9d833b5c1d308b", "intro"),
	ENTRY("ween", "Demo", GF_GOB2, UNK_LANG, kPlatformPC, "2e9c2898f6bf206ede801e3b2e7ee428", "intro"),

	ENTRY("bargon", "", GF_GOB2, UNK_LANG, kPlatformPC, "da3c54be18ab73fbdb32db24624a9c23", "intro"),

	ENTRY("gob3", "", GF_GOB2, UNK_LANG, kPlatformPC, "32b0f57f5ae79a9ae97e8011df38af42", "intro"),
	ENTRY("gob3", "", GF_GOB2, UNK_LANG, kPlatformPC, "1e2f64ec8dfa89f42ee49936a27e66e7", "intro"),
	ENTRY("gob3", "", GF_GOB2, FR_FRA, kPlatformPC, "e42a4f2337d6549487a80864d7826972", "intro"),
	ENTRY("gob3", "", GF_GOB2, RU_RUS, kPlatformPC, "4e3af248a48a2321364736afab868527", "intro"),
	ENTRY("gob3", "", GF_GOB2, UNK_LANG, kPlatformPC, "8d28ce1591b0e9cc79bf41cad0fc4c9c", "intro"),
	// CD 1.000
	ENTRY("gob3", "CD 1.000", GF_GOB2, UNK_LANG, kPlatformPC, "6f2c226c62dd7ab0ab6f850e89d3fc47", "intro"),
	// CD 1.02. Spanish "Computer Gaming World"* distribution in Spain
	ENTRY("gob3", "CD 1.02", GF_GOB2, UNK_LANG, kPlatformPC, "c3e9132ea9dc0fb866b6d60dcda10261", "intro"),

	ENTRY("gob3", "Interactive Demo", GF_GOB2, UNK_LANG, kPlatformPC, "7aebd94e49c2c5c518c9e7b74f25de9d", "intro"),
	ENTRY("gob3", "Demo", GF_GOB2, UNK_LANG, kPlatformPC, "b9b898fccebe02b69c086052d5024a55", "intro"),
	ENTRY("gob3", "Interactive Demo 2", GF_GOB2, UNK_LANG, kPlatformPC, "e5dcbc9f6658ebb1e8fe26bc4da0806d", "intro"),

	// CD 1.0
	ENTRY("woodruff", "1.0", GF_WOODRUFF, UNK_LANG, kPlatformPC, "dccf9d31cb720b34d75487408821b77e", "intro"),

	// CD 1.00, German release (INTRO.STRK seems to be multilingual, though?)
	ENTRY("woodruff", "1.00", GF_WOODRUFF, UNK_LANG, kPlatformPC, "5f5f4e0a72c33391e67a47674b120cc6", "intro"),

	{ { NULL, NULL, { { NULL, 0, NULL, 0 } }, UNK_LANG, kPlatformUnknown }, 0, NULL }
};

}

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Gob::gameDescriptions,
	// Size of that superset structure
	sizeof(Gob::GOBGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	gobGames,
	// Structure for autoupgrading obsolete targets
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	"gob",
	// Flags
	kADFlagComplexID
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(GOB, Gob::GobEngine, Gob::GAME_detectGames, detectionParams);

REGISTER_PLUGIN(GOB, "Gob Engine", "Goblins Games (C) Coktel Vision");


namespace Gob {

bool GobEngine::detectGame() {
	int i = AdvancedDetector::detectBestMatchingGame(detectionParams);

	if (gameDescriptions[i].startTotBase == 0) {
		_startTot = new char[10];
		_startTot0 = new char[11];
		strcpy(_startTot, "intro.tot");
		strcpy(_startTot0, "intro0.tot");
	} else {
		_startTot = new char[strlen(gameDescriptions[i].startTotBase) + 5];
		_startTot0 = new char[strlen(gameDescriptions[i].startTotBase) + 6];
		strcpy(_startTot, gameDescriptions[i].startTotBase);
		strcpy(_startTot0, gameDescriptions[i].startTotBase);
		strcat(_startTot, ".tot");
		strcat(_startTot0, "0.tot");
	}

	_features = gameDescriptions[i].features;
	_language = gameDescriptions[i].desc.language;

	return true;
}

GameList GAME_detectGames(const FSList &fslist) {
	GameList gl(AdvancedDetector::detectAllGames(fslist, detectionParams));

	if (gl.empty()) {
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (file->isDirectory()) continue;
			
			if (!scumm_stricmp(file->name().c_str(), "intro.stk")) {
				const PlainGameDescriptor *g = detectionParams.list;
				while (g->gameid) {
					if (0 == scumm_stricmp(detectionParams.singleid, g->gameid)) {
						gl.push_back(GameDescriptor(g->gameid, g->description));

						return gl;
					}

					g++;
				}
			}
		}
	}

	return gl;
}

} // End of namespace Parallaction
