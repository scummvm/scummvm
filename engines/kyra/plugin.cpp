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

struct KYRAGameDescription {
	Common::ADGameDescription desc;

	GameFlags flags;
};

namespace {

#define FLAGS(x, y, z, w, id) { UNK_LANG, kPlatformUnknown, x, y, z, w, id }

#define KYRA1_FLOPPY_FLAGS FLAGS(false, false, false, false, GI_KYRA1)
#define KYRA1_CD_FLAGS FLAGS(false, true, false, true, GI_KYRA1)
#define KYRA1_DEMO_FLAGS FLAGS(true, false, false, false, GI_KYRA1)

#define KYRA2_UNK_FLAGS FLAGS(false, false, false, false, GI_KYRA2)

#define KYRA3_CD_FLAGS FLAGS(false, false, false, true, GI_KYRA3)

static const KYRAGameDescription adGameDescs[] = {
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "3c244298395520bb62b5edfe41688879"), EN_ANY, kPlatformPC }, KYRA1_FLOPPY_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "796e44863dd22fa635b042df1bf16673"), EN_ANY, kPlatformPC }, KYRA1_FLOPPY_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "abf8eb360e79a6c2a837751fbd4d3d24"), FR_FRA, kPlatformPC }, KYRA1_FLOPPY_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "6018e1dfeaca7fe83f8d0b00eb0dd049"), DE_DEU, kPlatformPC }, KYRA1_FLOPPY_FLAGS },
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "f0b276781f47c130f423ec9679fe9ed9"), DE_DEU, kPlatformPC }, KYRA1_FLOPPY_FLAGS }, // from Arne.F
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "8909b41596913b3f5deaf3c9f1017b01"), ES_ESP, kPlatformPC }, KYRA1_FLOPPY_FLAGS }, // from VooD
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "747861d2a9c643c59fdab570df5b9093"), ES_ESP, kPlatformPC }, KYRA1_FLOPPY_FLAGS }, // floppy 1.8 from clemmy
	{ { "kyra1", 0, AD_ENTRY1("GEMCUT.EMC", "ef08c8c237ee1473fd52578303fc36df"), IT_ITA, kPlatformPC }, KYRA1_FLOPPY_FLAGS }, // from gourry

	{ { "kyra1", "CD", AD_ENTRY1("GEMCUT.PAK", "fac399fe62f98671e56a005c5e94e39f"), EN_ANY, kPlatformPC }, KYRA1_CD_FLAGS },
	{ { "kyra1", "CD", AD_ENTRY1("GEMCUT.PAK", "230f54e6afc007ab4117159181a1c722"), DE_DEU, kPlatformPC }, KYRA1_CD_FLAGS },
	{ { "kyra1", "CD", AD_ENTRY1("GEMCUT.PAK", "b037c41768b652a040360ffa3556fd2a"), FR_FRA, kPlatformPC }, KYRA1_CD_FLAGS },

	{ { "kyra1", "Demo", AD_ENTRY1("DEMO1.WSA", "fb722947d94897512b13b50cc84fd648"), EN_ANY, kPlatformPC }, KYRA1_DEMO_FLAGS },

	{ { "kyra2", 0, AD_ENTRY1("FATE.PAK", "28cbad1c5bf06b2d3825ae57d760d032"), UNK_LANG, kPlatformPC }, KYRA2_UNK_FLAGS }, // check this! (cd version?)

	{ { "kyra3", 0, AD_ENTRY1("ONETIME.PAK", "3833ff312757b8e6147f464cca0a6587"), UNK_LANG, kPlatformPC }, KYRA3_CD_FLAGS },
	{ { NULL, NULL, {NULL, 0, NULL, 0}, UNK_LANG, kPlatformUnknown }, KYRA2_UNK_FLAGS }
};

const PlainGameDescriptor gameList[] = {
	{ "kyra1", "The Legend of Kyrandia" },
	{ "kyra2", "The Legend of Kyrandia: The Hand of Fate" },
	{ "kyra3", "The Legend of Kyrandia: Malcolm's Revenge" },
	{ 0, 0 }
};

static const Common::ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)adGameDescs,
	// Size of that superset structure
	sizeof(KYRAGameDescription),
	// Number of bytes to compute MD5 sum for
	1024 * 1024,
	// List of all engine targets
	gameList,
	// Structure for autoupgrading obsolete targets
	0
};

} // End of anonymous namespace

GameList Engine_KYRA_gameIDList() {
	return GameList(gameList);
}

GameDescriptor Engine_KYRA_findGameID(const char *gameid) {
	return Common::ADVANCED_DETECTOR_FIND_GAMEID(gameid, detectionParams);
}

GameList Engine_KYRA_detectGames(const FSList &fslist) {
	return Common::ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(fslist, detectionParams);
}

PluginError Engine_KYRA_create(OSystem *syst, Engine **engine) {
	assert(engine);
	const char *gameid = ConfMan.get("gameid").c_str();
	
	int id = ADVANCED_DETECTOR_DETECT_INIT_GAME(detectionParams);
	if (id == -1) {
		// FIXME: This case currently can never happen, as we simply error out
		// inside ADVANCED_DETECTOR_DETECT_INIT_GAME.
		
		// maybe add non md5 based detection again?
		return kNoGameDataFoundError;
	}

	GameFlags flags = adGameDescs[id].flags;

	if (!scumm_stricmp("kyra1", gameid)) {
		*engine = new KyraEngine_v1(syst, flags);
	} else if (!scumm_stricmp("kyra2", gameid)) {
		*engine = new KyraEngine_v2(syst, flags);
	} else if (!scumm_stricmp("kyra3", gameid)) {
		*engine = new KyraEngine_v3(syst, flags);
	} else
		error("Kyra engine created with invalid gameid");

	return kNoError;
}

REGISTER_PLUGIN(KYRA, "Legend of Kyrandia Engine", "The Legend of Kyrandia (C) Westwood Studios");
