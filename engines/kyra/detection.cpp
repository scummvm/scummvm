/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
#include "kyra/kyra_v1.h"
#include "kyra/kyra_v2.h"
#include "kyra/kyra_v3.h"

#include "common/config-manager.h"
#include "common/advancedDetector.h"

#include "base/plugins.h"

struct KYRAGameDescription {
	Common::ADGameDescription desc;

	Kyra::GameFlags flags;
};

namespace {

#define FLAGS(x, y, z, a, id) { Common::UNK_LANG, Common::kPlatformUnknown, x, y, z, a, id }

#define KYRA1_FLOPPY_FLAGS FLAGS(false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_AMIGA_FLAGS FLAGS(false, false, false, false, Kyra::GI_KYRA1)
#define KYRA1_TOWNS_FLAGS FLAGS(false, true, false, false, Kyra::GI_KYRA1)
#define KYRA1_TOWNS_SJIS_FLAGS FLAGS(false, true, false, true, Kyra::GI_KYRA1)
#define KYRA1_CD_FLAGS FLAGS(false, true, true, false, Kyra::GI_KYRA1)
#define KYRA1_DEMO_FLAGS FLAGS(true, false, false, false, Kyra::GI_KYRA1)

#define KYRA2_CD_FLAGS FLAGS(false, false, false, false, Kyra::GI_KYRA2)
#define KYRA2_DEMO_FLAGS FLAGS(true, false, false, false, Kyra::GI_KYRA2)

#define KYRA3_CD_FLAGS FLAGS(false, false, true, false, Kyra::GI_KYRA3)

const KYRAGameDescription adGameDescs[] = {
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "3c244298395520bb62b5edfe41688879"), Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_FLOPPY_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "796e44863dd22fa635b042df1bf16673"), Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_FLOPPY_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "abf8eb360e79a6c2a837751fbd4d3d24"), Common::FR_FRA, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_FLOPPY_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "6018e1dfeaca7fe83f8d0b00eb0dd049"), Common::DE_DEU, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_FLOPPY_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "f0b276781f47c130f423ec9679fe9ed9"), Common::DE_DEU, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_FLOPPY_FLAGS }, // from Arne.F
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "8909b41596913b3f5deaf3c9f1017b01"), Common::ES_ESP, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_FLOPPY_FLAGS }, // from VooD
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "747861d2a9c643c59fdab570df5b9093"), Common::ES_ESP, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_FLOPPY_FLAGS }, // floppy 1.8 from clemmy
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "ef08c8c237ee1473fd52578303fc36df"), Common::IT_ITA, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_FLOPPY_FLAGS }, // from gourry

	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.PAK", "2bd1da653eaefd691e050e4a9eb68a64"), Common::EN_ANY, Common::kPlatformAmiga, Common::ADGF_NO_FLAGS }, KYRA1_AMIGA_FLAGS },

	{ { "kyra1", 0, AD_ENTRY1("TWMUSIC.PAK", "e53bca3a3e3fb49107d59463ec387a59"), Common::EN_ANY, Common::kPlatformFMTowns, Common::ADGF_NO_FLAGS }, KYRA1_TOWNS_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("TWMUSIC.PAK", "e53bca3a3e3fb49107d59463ec387a59"), Common::JA_JPN, Common::kPlatformFMTowns, Common::ADGF_NO_FLAGS }, KYRA1_TOWNS_SJIS_FLAGS },

	{ { "kyra1", "CD", AD_ENTRY1("GEMCUT.PAK", "fac399fe62f98671e56a005c5e94e39f"), Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_CD_FLAGS },
	{ { "kyra1", "CD", AD_ENTRY1("GEMCUT.PAK", "230f54e6afc007ab4117159181a1c722"), Common::DE_DEU, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_CD_FLAGS },
	{ { "kyra1", "CD", AD_ENTRY1("GEMCUT.PAK", "b037c41768b652a040360ffa3556fd2a"), Common::FR_FRA, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA1_CD_FLAGS },

	{ { "kyra1", "Demo", AD_ENTRY1("DEMO1.WSA", "fb722947d94897512b13b50cc84fd648"), Common::EN_ANY, Common::kPlatformPC, Common::ADGF_DEMO }, KYRA1_DEMO_FLAGS },

	{ { "kyra2", 0, AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"), Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA2_CD_FLAGS }, // CD version
	{ { "kyra2", 0, AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"), Common::DE_DEU, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA2_CD_FLAGS }, // CD version
	{ { "kyra2", 0, AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"), Common::FR_FRA, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA2_CD_FLAGS }, // CD version

	{ { "kyra2", "Demo", AD_ENTRY1("GENERAL.PAK", "35825783e5b60755fd520360079f9c15"), Common::EN_ANY, Common::kPlatformPC, Common::ADGF_DEMO }, KYRA2_DEMO_FLAGS },

	{ { "kyra3", 0, AD_ENTRY1("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587"), Common::EN_ANY, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA3_CD_FLAGS },
	{ { "kyra3", 0, AD_ENTRY1("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587"), Common::DE_DEU, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA3_CD_FLAGS },
	{ { "kyra3", 0, AD_ENTRY1("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587"), Common::FR_FRA, Common::kPlatformPC, Common::ADGF_NO_FLAGS }, KYRA3_CD_FLAGS },
	{ AD_TABLE_END_MARKER, FLAGS(0, 0, 0, 0, 0) }
};

const PlainGameDescriptor gameList[] = {
	{ "kyra1", "The Legend of Kyrandia" },
	{ "kyra2", "The Legend of Kyrandia: The Hand of Fate" },
	{ "kyra3", "The Legend of Kyrandia: Malcolm's Revenge" },
	{ 0, 0 }
};

const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)adGameDescs,
	// Size of that superset structure
	sizeof(KYRAGameDescription),
	// Number of bytes to compute MD5 sum for
	1024 * 1024,
	// List of all engine targets
	gameList,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	0,
	// List of files for file-based fallback detection (optional)
	0,
	// Fallback callback
	0,
	// Flags
	0
};

} // End of anonymous namespace

GameList Engine_KYRA_gameIDList() {
	return GameList(gameList);
}

GameDescriptor Engine_KYRA_findGameID(const char *gameid) {
	return Common::AdvancedDetector::findGameID(gameid, detectionParams);
}

GameList Engine_KYRA_detectGames(const FSList &fslist) {
	return Common::AdvancedDetector::detectAllGames(fslist, detectionParams);
}

PluginError Engine_KYRA_create(OSystem *syst, Engine **engine) {
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();
	
	const KYRAGameDescription *gd = (const KYRAGameDescription *)Common::AdvancedDetector::detectBestMatchingGame(detectionParams);
	if (gd == 0) {
		// maybe add non md5 based detection again?
		return kNoGameDataFoundError;
	}

	Kyra::GameFlags flags = gd->flags;
	
	flags.lang = gd->desc.language;
	flags.platform = gd->desc.platform;

	Common::Platform platform = Common::parsePlatform(ConfMan.get("platform"));
	if (platform != Common::kPlatformUnknown)
		flags.platform = platform;

	if (flags.lang == Common::UNK_LANG) {
		Common::Language lang = Common::parseLanguage(ConfMan.get("language"));
		if (lang != Common::UNK_LANG)
			flags.lang = lang;
		else
			flags.lang = Common::EN_ANY;
	}

	if (!scumm_stricmp("kyra1", gameid)) {
		*engine = new Kyra::KyraEngine_v1(syst, flags);
	} else if (!scumm_stricmp("kyra2", gameid)) {
		*engine = new Kyra::KyraEngine_v2(syst, flags);
	} else if (!scumm_stricmp("kyra3", gameid)) {
		*engine = new Kyra::KyraEngine_v3(syst, flags);
	} else
		error("Kyra engine created with invalid gameid ('%s')", gameid);

	return kNoError;
}

REGISTER_PLUGIN(KYRA, "Legend of Kyrandia Engine", "The Legend of Kyrandia (C) Westwood Studios");

