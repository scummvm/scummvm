/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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
#include "common/config-manager.h"
#include "common/file.h"

#include "cine/cine.h"

namespace Cine {
static DetectedGameList GAME_detectGames(const FSList &fslist);
}

using Common::File;

static const PlainGameDescriptor cineGames[] = {
	{"fw", "Future Wars"},
	{"os", "Operation Stealth"},
	{0, 0}
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(CINE, Cine::CineEngine, Cine::GAME_detectGames, cineGames, 0);

REGISTER_PLUGIN(CINE, "Cinematique evo 1 engine", "Future Wars & Operation Stealth (C) Delphine Software");

namespace Cine {

#define FILE_MD5_BYTES 5000

using Common::ADGameFileDescription;
using Common::ADGameDescription;

static const ADGameFileDescription FW_GameFiles[] = {
	{ "part01",		0,	"61d003202d301c29dd399acfb1354310"},
};

static const ADGameFileDescription FWALT_GameFiles[] = {
	{ "part01",		0,	"91d7271155520eae6915a9dd2dac120c"},
};

static const ADGameFileDescription FWDE_GameFiles[] = {
	{ "part01",		0,	"f5e98fcca3fb5e7afa284c81c39d8b14"},
};

static const ADGameFileDescription FWES_GameFiles[] = {
	{ "part01",		0,	"570109f965c7f53984b98c83d86eb206"},
};

static const ADGameFileDescription FWFR_GameFiles[] = {
	{ "part01",		0,	"5d1acb97abe9591f9008e00d07add95a"},
};

static const ADGameFileDescription FWAmiga_GameFiles[] = {
	{ "part01",		0,	"57afd280b598b4180fda6689fbedc4b8"},
};

static const ADGameFileDescription FWAmigaDE_GameFiles[] = {
	{ "part01",		0,	"3a87a913e0e33963a48a7f822ca0eb0e"},
};

static const ADGameFileDescription FWAmigaES_GameFiles[] = {
	{ "part01",		0,	"5ad0007ccd5f7b3dd6b15ea7f281f9e1"},
};

static const ADGameFileDescription FWAmigaFR_GameFiles[] = {
	{ "part01",		0,	"460f2da8793bc581a2d4b6fc19ccb5ae"},
};

static const ADGameFileDescription FWAmigaIT_GameFiles[] = {
	{ "part01",		0,	"1c8e5207743172134409ac58860021af"},
};

static const ADGameFileDescription FWAmigaDemo_GameFiles[] = {
	{ "demo",		0,	"0f50767cd964e302d3af0ba2528df8c4"},
	{ "demo.prc",		0,	"d2ac3a743d288359c63644ea7071edae"},
};

static const ADGameFileDescription FWST_GameFiles[] = {
	{ "part01",		0,	"36050db13af57e462ca1adc4df99de4e"},
};

static const ADGameFileDescription FWSTFR_GameFiles[] = {
	{ "part01",		0,	"ef245573b7dab0d4825ceb98e37cef4d"},
};

static const ADGameFileDescription OS_GameFiles[] = {
	{ "procs00",		0,	"d6752e7d25924cb866b61eb7cb0c8b56"},
};

static const ADGameFileDescription OSUS_GameFiles[] = {
	{ "procs1",		0,	"d8c3a9d05a63e4cfa801826a7063a126"},
};

static const ADGameFileDescription OSUS256_GameFiles[] = {
	{ "procs00",		0,	"862a75d76fb7fffec30e52be9ad1c474"},
};

static const ADGameFileDescription OSDE_GameFiles[] = {
	{ "procs1",		0,	"39b91ae35d1297ce0a76a1a803ca1593"},
};

static const ADGameFileDescription OSES_GameFiles[] = {
	{ "procs1",		0,	"74c2dabd9d212525fca8875a5f6d8994"},
};

static const ADGameFileDescription OSESCD_GameFiles[] = {
	{ "procs1",		0,	"74c2dabd9d212525fca8875a5f6d8994"},
	{ "sds1",		0,	"75443ba39cdc95667e07d7118e5c151c"},
};

static const ADGameFileDescription OSFR_GameFiles[] = {
	{ "procs00",		0,	"f143567f08cfd1a9b1c9a41c89eadfef"},
};

static const ADGameFileDescription OSIT_GameFiles[] = {
	{ "procs1",		0,	"da066e6b8dd93f2502c2a3755f08dc12"},
};

static const ADGameFileDescription OSAmiga_GameFiles[] = {
	{ "procs0",		0,	"a9da5531ead0ebf9ad387fa588c0cbb0"},
};

static const ADGameFileDescription OSAmigaAlt_GameFiles[] = {
	{ "procs0",		0,	"8a429ced2f4acff8a15ae125174042e8"},
};

static const ADGameFileDescription OSAmigaUS_GameFiles[] = {
	{ "procs0",		0,	"d5f27e33fc29c879f36f15b86ccfa58c"},
};

static const ADGameFileDescription OSAmigaDE_GameFiles[] = {
	{ "procs0",		0,	"8b7dce249821d3a62b314399c4334347"},
};

static const ADGameFileDescription OSAmigaES_GameFiles[] = {
	{ "procs0",		0,	"35fc295ddd0af9da932d256ba799a4b0"},
};

static const ADGameFileDescription OSAmigaFR_GameFiles[] = {
	{ "procs0",		0,	"d4ea4a97e01fa67ea066f9e785050ed2"},
};

static const ADGameFileDescription OSAmigaDemo_GameFiles[] = {
	{ "demo",		0,	"8d3a750d1c840b1b1071e42f9e6f6aa2"},
};

static const ADGameFileDescription OSST_GameFiles[] = {
	{ "procs0",		0,	"1501d5ae364b2814a33ed19347c3fcae"},
};

static const ADGameFileDescription OSSTFR_GameFiles[] = {
	{ "procs0",		0,	"2148d25de3219dd4a36580ca735d0afa"},
};


static const CINEGameDescription gameDescriptions[] = {
	{
		{
			"fw",
			"",
			ARRAYSIZE(FW_GameFiles),
			FW_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_FW,
		0,
	},

	{
		// This is the version included in the UK "Classic Collection"
		{
			"fw",
			"",
			ARRAYSIZE(FWALT_GameFiles),
			FWALT_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWDE_GameFiles),
			FWDE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},
		GType_FW,
		GF_ALT_FONT,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWES_GameFiles),
			FWES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformPC,
		},
		GType_FW,
		GF_ALT_FONT,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWFR_GameFiles),
			FWFR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformPC,
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWAmiga_GameFiles),
			FWAmiga_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWAmigaDE_GameFiles),
			FWAmigaDE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformAmiga,
		},
		GType_FW,
		GF_ALT_FONT,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWAmigaES_GameFiles),
			FWAmigaES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformAmiga,
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWAmigaFR_GameFiles),
			FWAmigaFR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformAmiga,
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWAmigaIT_GameFiles),
			FWAmigaIT_GameFiles,
			Common::IT_ITA,
			Common::kPlatformAmiga,
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"Demo",
			ARRAYSIZE(FWAmigaDemo_GameFiles),
			FWAmigaDemo_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWST_GameFiles),
			FWST_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAtariST,
		},
		GType_FW,
		0,
	},

	{
		{
			"fw",
			"",
			ARRAYSIZE(FWSTFR_GameFiles),
			FWSTFR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformAtariST,
		},
		GType_FW,
		0,
	},

	{
		{
			"os",
			"256 colors",
			ARRAYSIZE(OS_GameFiles),
			OS_GameFiles,
			Common::EN_GRB,
			Common::kPlatformPC,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSUS_GameFiles),
			OSUS_GameFiles,
			Common::EN_USA,
			Common::kPlatformPC,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"256 colors",
			ARRAYSIZE(OSUS256_GameFiles),
			OSUS256_GameFiles,
			Common::EN_USA,
			Common::kPlatformPC,
		},
		GType_OS,
		GF_CD,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSDE_GameFiles),
			OSDE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformPC,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSES_GameFiles),
			OSES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformPC,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"256 colors",
			ARRAYSIZE(OSESCD_GameFiles),
			OSESCD_GameFiles,
			Common::ES_ESP,
			Common::kPlatformPC,
		},
		GType_OS,
		GF_CD,
	},

	{
		{
			"os",
			"256 colors",
			ARRAYSIZE(OSFR_GameFiles),
			OSFR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformPC,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSIT_GameFiles),
			OSIT_GameFiles,
			Common::IT_ITA,
			Common::kPlatformPC,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSAmiga_GameFiles),
			OSAmiga_GameFiles,
			Common::EN_GRB,
			Common::kPlatformAmiga,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"alt",
			ARRAYSIZE(OSAmigaAlt_GameFiles),
			OSAmigaAlt_GameFiles,
			Common::EN_GRB,
			Common::kPlatformAmiga,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSAmigaUS_GameFiles),
			OSAmigaUS_GameFiles,
			Common::EN_USA,
			Common::kPlatformAmiga,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSAmigaDE_GameFiles),
			OSAmigaDE_GameFiles,
			Common::DE_DEU,
			Common::kPlatformAmiga,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSAmigaES_GameFiles),
			OSAmigaES_GameFiles,
			Common::ES_ESP,
			Common::kPlatformAmiga,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSAmigaFR_GameFiles),
			OSAmigaFR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformAmiga,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"Demo",
			ARRAYSIZE(OSAmigaDemo_GameFiles),
			OSAmigaDemo_GameFiles,
			Common::EN_GRB,
			Common::kPlatformAmiga,
		},
		GType_OS,
		GF_DEMO,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSST_GameFiles),
			OSST_GameFiles,
			Common::EN_GRB,
			Common::kPlatformAtariST,
		},
		GType_OS,
		0,
	},

	{
		{
			"os",
			"",
			ARRAYSIZE(OSSTFR_GameFiles),
			OSSTFR_GameFiles,
			Common::FR_FRA,
			Common::kPlatformAtariST,
		},
		GType_OS,
		0,
	},

};

bool CineEngine::initGame() {
	int i = Common::real_ADVANCED_DETECTOR_DETECT_INIT_GAME(
		(const byte *)gameDescriptions,
		sizeof(CINEGameDescription),
		ARRAYSIZE(gameDescriptions),
		FILE_MD5_BYTES,
		cineGames
		);
	_gameDescription = &gameDescriptions[i];
	return true;
}

DetectedGameList GAME_detectGames(const FSList &fslist) {
	return real_ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
		fslist,
		(const byte *)gameDescriptions,
		sizeof(CINEGameDescription),
		ARRAYSIZE(gameDescriptions),
		FILE_MD5_BYTES,
		cineGames
	);
}

} // End of namespace Cine
