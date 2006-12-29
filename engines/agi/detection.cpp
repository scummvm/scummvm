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

static const PlainGameDescriptor agiGames[] = {
	{"agi", "Sierra AGI Engine" },

	{0, 0}
};

ADVANCED_DETECTOR_DEFINE_PLUGIN(AGI, Agi::AgiEngine, Agi::GAME_detectGames, agiGames, 0);

REGISTER_PLUGIN(AGI, "AGI v2 + v3 Engine", "Sierra AGI Engine (C) Sierra On-Line Software");


namespace Agi {

#define FILE_MD5_BYTES 5000

static const AGIGameDescription gameDescriptions[] = {
	{
		// leisure suit larry 1 : pc : 1.00
		{
			"agi",
			"Leisure Suit Larry 1 (PC) 1.00 [AGI 2.440]",
			AD_ENTRY1("logdir", "1fe764e66857e7f305a5f03ca3f4971d"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},

	{
		// space quest 1 : pc : 2.2
		{
			"agi",
			"Space Quest 1 (PC) 2.2 [AGI 2.917]",
			AD_ENTRY1("logdir", "5d67630aba008ec5f7f9a6d0a00582f4"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},

	{
		// police quest 1 : pc : 2.0G
		{
			"agi",
			"Police Quest 1 (PC) 2.0G [AGI 2.917]",
			AD_ENTRY1("logdir", "231f3e28170d6e982fc0ced4c98c5c1c"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// kings quest 1 : pc : 2.0F
		{
			"agi",
			"King's Quest 1 (PC) 2.0F [AGI 2.917]",
			AD_ENTRY1("logdir", "10ad66e2ecbd66951534a50aedcd0128"),
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
			AD_ENTRY1("logdir", "f4277aa34b43d37382bc424c81627617"),
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
			AD_ENTRY1("logdir", "d4c4739d4ac63f7dbd29255425077d48"),
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
			AD_ENTRY1("logdir", "b944c4ff18fb8867362dc21cc688a283"),
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
			AD_ENTRY1("logdir", "b866f0fab2fad91433a637a828cfa410"),
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
			AD_ENTRY1("logdir", "cbdb0083317c8e7cfb7ac35da4bc7fdc"),
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
			AD_ENTRY1("logdir", "9c2b34e7ffaa89c8e2ecfeb3695d444b"),
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
			AD_ENTRY1("logdir", "18aad8f7acaaff760720c5c6885b6bab"),
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
			AD_ENTRY1("logdir", "7650e659c7bc0f1e9f8a410b7a2e9de6"),
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
			AD_ENTRY1("logdir", "d3d17b77b3b3cd13246749231d9473cd"),
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
			AD_ENTRY1("kq4dir", "fe44655c42f16c6f81046fdf169b6337"),
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
			AD_ENTRY1("kq4dir", "7470b3aeb49d867541fc66cc8454fb7d"),
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
			AD_ENTRY1("logdir", "421da3a18004122a966d64ab6bd86d2e"),
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
			AD_ENTRY1("logdir", "64388812e25dbd75f7af1103bc348596"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2089,
	},


	{ { NULL, NULL, {NULL, 0, NULL}, Common::UNK_LANG, Common::kPlatformUnknown }, 0, 0, 0 }
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

