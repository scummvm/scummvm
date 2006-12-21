/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
#include "common/file.h"

#include "agi/agi.h"


namespace Agi {
static DetectedGameList GAME_detectGames(const FSList &fslist);
}

using Common::File;

static const PlainGameDescriptor agiGames[] = {
	{"agi", "Sierra AGI Engine" },

	{0, 0}
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(AGI, Agi::AgiEngine, Agi::GAME_detectGames, agiGames, 0);

REGISTER_PLUGIN(AGI, "AGI v2 + v3 Engine", "Sierra AGI Engine (C) Sierra On-Line Software");


namespace Agi {

#define FILE_MD5_BYTES 5000

using Common::ADGameFileDescription;
using Common::ADGameDescription;

using Common::File;

// Kings Quest I entries
static const ADGameFileDescription AGI_KQ1_PC_20F_GameFiles[] = {
	{ "logdir",		0,	"10ad66e2ecbd66951534a50aedcd0128"},
	{ NULL, 0, NULL}
};
static const ADGameFileDescription AGI_KQ1_2GS_10S_GameFiles[] = {
	{ "logdir",		0,	"f4277aa34b43d37382bc424c81627617"},
	{ NULL, 0, NULL}
};
static const ADGameFileDescription AGI_KQ1_MAC_20C_GameFiles[] = {
	{ "logdir",		0,	"d4c4739d4ac63f7dbd29255425077d48"},
	{ NULL, 0, NULL}
};

// Kings Quest II Entries
static const ADGameFileDescription AGI_KQ2_PC_22_GameFiles[] = {
	{ "logdir",		0,	"b944c4ff18fb8867362dc21cc688a283"},
	{ NULL, 0, NULL}
};
static const ADGameFileDescription AGI_KQ2_AMIGA_20F_GameFiles[] = {
	{ "logdir",		0,	"b866f0fab2fad91433a637a828cfa410"},
	{ NULL, 0, NULL}
};
static const ADGameFileDescription AGI_KQ2_MAC_20R_GameFiles[] = {
	{ "logdir",		0,	"cbdb0083317c8e7cfb7ac35da4bc7fdc"},
	{ NULL, 0, NULL}
};

// Kings Quest III
static const ADGameFileDescription AGI_KQ3_PC_101_GameFiles[] = {
	{ "logdir",		0,	"9c2b34e7ffaa89c8e2ecfeb3695d444b"},
	{ NULL, 0, NULL}
};
static const ADGameFileDescription AGI_KQ3_PC_200_GameFiles[] = {
	{ "logdir",		0,	"18aad8f7acaaff760720c5c6885b6bab"},
	{ NULL, 0, NULL}
};
static const ADGameFileDescription AGI_KQ3_PC_214A_GameFiles[] = {
	{ "logdir",		0,	"7650e659c7bc0f1e9f8a410b7a2e9de6"},
	{ NULL, 0, NULL}
};
static const ADGameFileDescription AGI_KQ3_PC_214B_GameFiles[] = {
	{ "logdir",		0,	"d3d17b77b3b3cd13246749231d9473cd"},
	{ NULL, 0, NULL}
};

// Kings Quest IV AGI v3
static const ADGameFileDescription AGI_KQ4_PC_20_GameFiles[] = {
	{ "kq4dir",		0,	"fe44655c42f16c6f81046fdf169b6337"},
	{ NULL, 0, NULL}
};
static const ADGameFileDescription AGI_KQ4_PC_22_GameFiles[] = {
	{ "kq4dir",		0,	"7470b3aeb49d867541fc66cc8454fb7d"},
	{ NULL, 0, NULL}
};

// Groza : AGDS
static const ADGameFileDescription AGI_Groza_PC_GameFiles[] = {
	{ "logdir",		0,	"421da3a18004122a966d64ab6bd86d2e"},
	{ NULL, 0, NULL}
};

// Donald Ducks Playground
static const ADGameFileDescription AGI_DDP_PC_10A_GameFiles[] = {
	{ "logdir",		0,	"64388812e25dbd75f7af1103bc348596"},
	{ NULL, 0, NULL}
};



static const AGIGameDescription gameDescriptions[] = {
	{
		// kings quest 1 : pc : 2.0F
		{
			"agi",
			"King's Quest 1 (PC) 2.0F [AGI 2.917]",
			AGI_KQ1_PC_20F_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},

	{
		// kings quest 1 : 2gs : 1.0S
		{
			"agi",
			"King's Quest 1 (IIgs) 1.0S-88223",
			AGI_KQ1_2GS_10S_GameFiles,
			Common::EN_ANY,
			Common::kPlatformApple2GS,
		},
		GType_V2,
		0,
		0x2272,
	},


	{
		// kings quest 1 : Mac : 2.0C
		{
			"agi",
			"King's Quest 1 (Mac) 2.0C",
			AGI_KQ1_MAC_20C_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// kings quest 2 : PC : 2.2
		{
			"agi",
			"King's Quest 2 (PC) 2.2 [AGI 2.426]",
			AGI_KQ2_PC_22_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},

	{
		// kings quest 2 : Amiga : 2.0J (not sure if this is broken version or not. need to test
		{
			"agi",
			"King's Quest 2 (Amiga) 2.0J",
			AGI_KQ2_AMIGA_20F_GameFiles,
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},
		GType_V2,
		AGI_AMIGA,
		0x2440,
	},

	{
		// kings quest 2 : Mac : 2.0R
		{
			"agi",
			"King's Quest 2 (Mac) 2.0R",
			AGI_KQ2_MAC_20R_GameFiles,
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_V2,
		0,
		0x2440,
	},



	{
		// kings quest 3 : PC : 1.01
		{
			"agi",
			"King's Quest 3 (PC) 1.01 11/08/86 [AGI 2.272]",
			AGI_KQ3_PC_101_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2272,
	},

	{
		// kings quest 3 : PC : 2.00
		{
			"agi",
			"King's Quest 3 (PC) 2.00 5/25/87 [AGI 2.435]",
			AGI_KQ3_PC_200_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},

	{
		// kings quest 3 : PC : 2.14-A
		{
			"agi",
			"King's Quest 3 (PC) 2.14 3/15/88 [AGI 2.936]",
			AGI_KQ3_PC_214A_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2936,
	},

	{
		// kings quest 3 : PC : 2.14-B
		{
			"agi",
			"King's Quest 3 (PC) 2.14 3/15/88 [AGI 2.936]",
			AGI_KQ3_PC_214B_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2936,
	},



	{
		// kings quest 4 : PC : 2.0
		{
			"agi",
			"King's Quest 4 (PC) 2.0 7/27/88 [AGI 3.002.086]",
			AGI_KQ4_PC_20_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3086,
	},

	{
		// kings quest 4 : PC : 2.0
		{
			"agi",
			"King's Quest 4 (PC) 2.2 9/27/88 [AGI 3.002.086]",
			AGI_KQ4_PC_22_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3086,
	},


	// AGDS : GROZA
	{
		// kings quest 4 : PC : 2.0
		{
			"agi",
			"Groza : Russian AGDS",
			AGI_Groza_PC_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		AGI_AGDS,
		0x2440,
	},


	{
		// Donald Ducks Playground : PC : 2.001 ** CURRENTLY UNSUPPORTED **
		{
			"agi",
			"Donald Ducks Playground 1.0A ** UNSUPPORTED **",
			AGI_DDP_PC_10A_GameFiles,
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2089,
	},


	{ { NULL, NULL, NULL, Common::UNK_LANG, Common::kPlatformUnknown }, 0, 0, 0 }
};

bool AgiEngine::initGame() {
	int i = Common::ADVANCED_DETECTOR_DETECT_INIT_GAME(
		(const byte *)gameDescriptions,
		sizeof(AGIGameDescription),
		FILE_MD5_BYTES,
		agiGames
		);
	_gameDescription = &gameDescriptions[i];
	return true;
}

DetectedGameList GAME_detectGames(const FSList &fslist) {
	return Common::ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
		fslist,
		(const byte *)gameDescriptions,
		sizeof(AGIGameDescription),
		FILE_MD5_BYTES,
		agiGames
	);
}

} // End of namespace Agi

