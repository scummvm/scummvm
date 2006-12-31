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
		// Sarien Name == Groza
		{
			"agi",
			"Groza (russian) [AGDS sample]",
			AD_ENTRY1("logdir", "421da3a18004122a966d64ab6bd86d2e"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		AGI_AGDS,
		0x2440,
	},


	{
		// Sarien Name == Black Cauldron (Apple IIgs) 1.0O 2/24/89 (CE)
		{
			"agi",
			"Black Cauldron (Apple IIgs) 1.0O 2/24/89 (CE)",
			AD_ENTRY1("bcdir", "dc09d30b147242692f4f85b9811962db"),
			Common::EN_ANY,
			Common::kPlatformApple2GS,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == Black Cauldron (PC) 2.00 6/14/87 [AGI 2.439]
		{
			"agi",
			"Black Cauldron (IBM) 2.00 6/14/87",
			AD_ENTRY1("logdir", "7f598d4712319b09d7bd5b3be10a2e4a"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// Sarien Name == Black Cauldron (PC 5.25") 2.10 11/10/88 [AGI 3.002.098]
		{
			"agi",
			"Black Cauldron (IBM 5.25\") 2.10 11/10/88",
			AD_ENTRY1("bcdir", "0c5a9acbcc7e51127c34818e75806df6"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == Donald Duck's Playground (ST) 1.0A 8/8/86
		{
			"agi",
			"Donald Duck's Playground (ST) 1.0A 8/8/86",
			AD_ENTRY1("logdir", "64388812e25dbd75f7af1103bc348596"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2272,
	},


	{
		// Sarien Name == AGI Demo 1 (PC) 05/87 [AGI 2.425]
		{
			"agi",
			"AGI Demo 1 (IBM) 05/87",
			AD_ENTRY1("logdir", "9c4a5b09cc3564bc48b4766e679ea332"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// Sarien Name == AGI Demo 2 (IIgs) 1.0C (Censored)
		{
			"agi",
			"AGI Demo 2 (Apple IIgs) 1.0C",
			AD_ENTRY1("logdir", "580ffdc569ff158f56fb92761604f70e"),
			Common::EN_ANY,
			Common::kPlatformApple2GS,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == AGI Demo 2 (PC 3.5") 11/87 [AGI 2.915]
		{
			"agi",
			"AGI Demo 2 (IBM 3.5\") 11/87",
			AD_ENTRY1("logdir", "e8ebeb0bbe978172fe166f91f51598c7"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == AGI Demo 2 (PC 5.25") 11/87 [v1] [AGI 2.915]
		{
			"agi",
			"AGI Demo 2 (IBM 5.25\") 11/87 [version 1]",
			AD_ENTRY1("logdir", "852ac303a374df62571642ca1e2d1f0a"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == AGI Demo 2 (PC 5.25") 01/88 [v2] [AGI 2.917]
		{
			"agi",
			"AGI Demo 2 (IBM 5.25\") 01/88 [version 2]",
			AD_ENTRY1("logdir", "1503f02086ea9f388e7e041c039eaa69"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == AGI Demo 3 (PC) 09/88 [AGI 3.002.102]
		{
			"agi",
			"AGI Demo 3 (IBM) 09/88",
			AD_ENTRY1("dmdir", "289c7a2c881f1d973661e961ced77d74"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == King's Quest 4 demo (PC) [AGI 3.002.102]
		{
			"agi",
			"King's Quest 4 demo (IBM)",
			AD_ENTRY1("dmdir", "a3332d70170a878469d870b14863d0bf"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == Xmas Card 1986 (PC) [AGI 2.272]
		{
			"agi",
			"Xmas Card 1986 (IBM) [version 1]",
			AD_ENTRY1("logdir", "3067b8d5957e2861e069c3c0011bd43d"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2272,
	},


	{
		// Sarien Name == Gold Rush! (PC 5.25") 2.01 12/22/88 [AGI 3.002.149]
		{
			"agi",
			"Gold Rush (IBM 5.25\") 2.01 12/22/88",
			AD_ENTRY1("grdir", "db733d199238d4009a9e95f11ece34e9"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == Gold Rush! (PC 5.25") 2.01 12/22/88 [AGI 3.002.149] (Cracked)
		{
			"agi",
			"Gold Rush (IBM 5.25\") 2.01 12/22/88 [DIR hacked]",
			AD_ENTRY1("grdir", "3ae052117feb483f01a9017025fbb366"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == King's Quest 1 (IIgs) 1.0S-88223
		{
			"agi",
			"King's Quest 1 (Apple IIgs) 1.0S-88223",
			AD_ENTRY1("logdir", "f4277aa34b43d37382bc424c81627617"),
			Common::EN_ANY,
			Common::kPlatformApple2GS,
		},
		GType_V2,
		0,
		0x2272,
	},


	{
		// Sarien Name == King's Quest 1 (Mac) 2.0C
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
		// Sarien Name == King's Quest 1 (PC 5.25"/3.5") 2.0F [AGI 2.917]
		{
			"agi",
			"King's Quest 1 (IBM 5.25\"/3.5\") 2.0F",
			AD_ENTRY1("logdir", "10ad66e2ecbd66951534a50aedcd0128"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == King's Quest 2 (Amiga) 2.0J (Broken)
		{
			"agi",
			"King's Quest 2 (Amiga) 2.0J [OBJECT decrypted]",
			AD_ENTRY1("logdir", "b866f0fab2fad91433a637a828cfa410"),
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},
		GType_V2,
		AGI_AMIGA,
		0x2440,
	},


	{
		// Sarien Name == King's Quest 2 (Mac) 2.0R
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
		// Sarien Name == King's Quest 2 (PC 5.25"/3.5") 2.2 [AGI 2.426]
		{
			"agi",
			"King's Quest 2 (IBM 5.25\"/3.5\") 2.2",
			AD_ENTRY1("logdir", "b944c4ff18fb8867362dc21cc688a283"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == King's Quest 3 (PC) 1.01 11/08/86 [AGI 2.272]
		{
			"agi",
			"King's Quest 3 (IBM) 1.01 11/08/86",
			AD_ENTRY1("logdir", "9c2b34e7ffaa89c8e2ecfeb3695d444b"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2272,
	},


	{
		// Sarien Name == King's Quest 3 (PC 5.25") 2.00 5/25/87 [AGI 2.435]
		{
			"agi",
			"King's Quest 3 (IBM 5.25\") 2.00 5/25/87",
			AD_ENTRY1("logdir", "18aad8f7acaaff760720c5c6885b6bab"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// Sarien Name == King's Quest 3 (Mac) 2.14 3/15/88
		{
			"agi",
			"King's Quest 3 (IBM 5.25\") 2.14 3/15/88",
			AD_ENTRY1("logdir", "7650e659c7bc0f1e9f8a410b7a2e9de6"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// Sarien Name == King's Quest 3 (PC 3.5") 2.14 3/15/88 [AGI 2.936]
		{
			"agi",
			"King's Quest 3 (IBM 3.5\") 2.14 3/15/88",
			AD_ENTRY1("logdir", "d3d17b77b3b3cd13246749231d9473cd"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2936,
	},


	{
		// Sarien Name == King's Quest 4 (PC 3.5") 2.0 7/27/88 [AGI 3.002.086]
		{
			"agi",
			"King's Quest 4 (IBM 3.5\") 2.0 7/27/88",
			AD_ENTRY1("kq4dir", "fe44655c42f16c6f81046fdf169b6337"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3086,
	},


	{
		// Sarien Name == King's Quest 4 (PC 3.5") 2.2 9/27/88 [AGI 3.002.086]
		{
			"agi",
			"King's Quest 4 (IBM 3.5\") 2.2 9/27/88",
			AD_ENTRY1("kq4dir", "7470b3aeb49d867541fc66cc8454fb7d"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3086,
	},


	{
		// Sarien Name == Leisure Suit Larry 1 (Mac) 1.05 6/26/87
		{
			"agi",
			"Leisure Suit Larry 1 (Mac) 1.05 6/26/87",
			AD_ENTRY1("logdir", "8a0076429890531832f0dc113285e31e"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// Sarien Name == Leisure Suit Larry 1 (PC 5.25"/3.5") 1.00 6/1/87 [AGI 2.440]
		{
			"agi",
			"Leisure Suit Larry 1 (IBM 5.25\"/3.5\") 1.00 6/1/87",
			AD_ENTRY1("logdir", "1fe764e66857e7f305a5f03ca3f4971d"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// Sarien Name == Mixed Up Mother Goose (PC) [AGI 2.915] (Broken)
		{
			"agi",
			"Mixed-Up Mother Goose (IBM) [corrupt/OBJECT from disk 1]",
			AD_ENTRY1("logdir", "e524655abf9b96a3b179ffcd1d0f79af"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == Mixed-Up Mother Goose (PC) [AGI 2.915]
		{
			"agi",
			"Mixed-Up Mother Goose (IBM)",
			AD_ENTRY1("logdir", "e524655abf9b96a3b179ffcd1d0f79af"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == Manhunter NY (IIgs) 2.0E 10/05/88 (CE)
		{
			"agi",
			"Manhunter 1: NY (Apple IIgs) 2.0E 10/05/88 (CE)",
			AD_ENTRY1("mhdir", "2f1509f76f24e6e7d213f2dadebbf156"),
			Common::EN_ANY,
			Common::kPlatformApple2GS,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == Manhunter NY (Amiga) 1.06 3/18/89
		{
			"agi",
			"Manhunter 1: NY (Amiga) 1.06 3/18/89",
			AD_ENTRY1("mhdir", "92c6183042d1c2bb76236236a7d7a847"),
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},
		GType_V3,
		AGI_AMIGA,
		0x3149,
	},


	{
		// Sarien Name == Manhunter NY (PC 5.25") 1.22 8/31/88 [AGI 3.002.107]
		{
			"agi",
			"Manhunter 1: NY (IBM 5.25\") 1.22 8/31/88 [VOL.1->LOGIC.159 hacked]",
			AD_ENTRY1("mhdir", "0c7b86f05fe02c2e26cff1b07450b82a"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == Manhunter SF (PC 3.5") 3.02 7/26/89 [AGI 3.002.149]
		{
			"agi",
			"Manhunter 2: SF (IBM 3.5\") 3.02 7/26/89",
			AD_ENTRY1("mh2dir", "6fb6f0ee2437704c409cf17e081ba152"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V3,
		0,
		0x3149,
	},


	{
		// Sarien Name == Police Quest 1 (IIgs) 2.0A-88318
		{
			"agi",
			"Police Quest 1 (Apple IIgs) 2.0A-88318",
			AD_ENTRY1("logdir", "8994e39d0901de3d07cecfb954075bb5"),
			Common::EN_ANY,
			Common::kPlatformApple2GS,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == Police Quest 1 (PC) 2.0A 10/23/87 [AGI 2.903/2.911]
		{
			"agi",
			"Police Quest 1 (IBM) 2.0A 10/23/87",
			AD_ENTRY1("logdir", "b9dbb305092851da5e34d6a9f00240b1"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == Police Quest 1 (Mac) 2.0G 12/3/87
		{
			"agi",
			"Police Quest 1 (IBM 5.25\"/ST) 2.0G 12/03/87",
			AD_ENTRY1("logdir", "231f3e28170d6e982fc0ced4c98c5c1c"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// Sarien Name == Space Quest 1 (PC) 1.0X [AGI 2.089]
		{
			"agi",
			"Space Quest 1 (IBM) 1.0X",
			AD_ENTRY1("logdir", "af93941b6c51460790a9efa0e8cb7122"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2089,
	},


	{
		// Sarien Name == Space Quest 1 (PC) 1.1A [AGI 2.272]
		{
			"agi",
			"Space Quest 1 (IBM) 1.1A",
			AD_ENTRY1("logdir", "8d8c20ab9f4b6e4817698637174a1cb6"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2272,
	},


	{
		// Sarien Name == Space Quest 1 (PC 5.25"/3.5") 2.2 [AGI 2.426/2.917]
		{
			"agi",
			"Space Quest 1 (IBM 5.25\"/3.5\") 2.2",
			AD_ENTRY1("logdir", "5d67630aba008ec5f7f9a6d0a00582f4"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2440,
	},


	{
		// Sarien Name == Space Quest 2 (Amiga) 2.0F
		{
			"agi",
			"Space Quest 2 (Amiga) 2.0F [VOL.2->PICTURE.16 broken]",
			AD_ENTRY1("logdir", "28add5125484302d213911df60d2aded"),
			Common::EN_ANY,
			Common::kPlatformAmiga,
		},
		GType_V2,
		AGI_AMIGA,
		0x2936,
	},


	{
		// Sarien Name == Space Quest 2 (Mac) 2.0D
		{
			"agi",
			"Space Quest 2 (Mac) 2.0D",
			AD_ENTRY1("logdir", "bfbebe0b59d83f931f2e1c62ce9484a7"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
		},
		GType_V2,
		0,
		0x2936,
	},


	{
		// Sarien Name == Space Quest 2 (PC 5.25"/ST) 2.0C/A [AGI 2.915]
		{
			"agi",
			"Space Quest 2 (IBM 5.25\"/ST) 2.0C [A]",
			AD_ENTRY1("logdir", "bd71fe54869e86945041700f1804a651"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2917,
	},


	{
		// Sarien Name == Space Quest 2 (PC 3.5") 2.0F [AGI 2.936]
		{
			"agi",
			"Space Quest 2 (IBM 3.5\") 2.0F",
			AD_ENTRY1("logdir", "28add5125484302d213911df60d2aded"),
			Common::EN_ANY,
			Common::kPlatformPC,
		},
		GType_V2,
		0,
		0x2936,
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

