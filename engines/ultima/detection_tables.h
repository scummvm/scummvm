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
 */

namespace Ultima {

static const UltimaGameDescription GAME_DESCRIPTIONS[] = {
#ifndef RELEASE_BUILD
	{
		// Ultima I - The First Age of Darkness
		{
			"ultima1",
			"VGA Enhanced",
			{
				{ "maptiles.vga", 0, "d4b67e17affe64c0ddb48511bfe4cf37", 47199 },
				{ "objtiles.vga", 0, "1a1446970d095aeb03bcf6dcec40d6e2", 289344 },
				{ "map.bin", 0, "f99633a0110ccf90837ab161be56cf1c", 13104 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_ULTIMA1,
		GF_VGA_ENHANCED
	},

	{
		// Ultima I - The First Age of Darkness, PC98 version
		{
			"ultima1",
			0,
			AD_ENTRY2s("egctown.bin",	"4f7de68f6689cf9617aa1ea03240137e", 4896,
					   "map.bin",		"f99633a0110ccf90837ab161be56cf1c", 13104),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_ULTIMA1,
		0
	},

	{
		// Ultima I - The First Age of Darkness
		{
			"ultima1",
			0,
			AD_ENTRY1s("map.bin", "f99633a0110ccf90837ab161be56cf1c", 13104),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_ULTIMA1,
		0
	},
#endif

	{
		// Ultima IV - Quest of the Avatar
		{
			"ultima4",
			0,
			AD_ENTRY1s("britain.ult", "304fe52ce5f34b9181052363d74d7505", 1280),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GAME_ULTIMA4,
		0
	},

	{
		// Ultima IV - Quest of the Avatar
		{
			"ultima4_enh",
			0,
			AD_ENTRY1s("britain.ult", "304fe52ce5f34b9181052363d74d7505", 1280),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GAME_ULTIMA4,
		GF_VGA_ENHANCED
	},

	// GOG Ultima VI
	{
		{
			"ultima6",
			0,
			AD_ENTRY1s("converse.a", "5065716423ef1389e3f7b4946d815c26", 162615),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GAME_ULTIMA6,
		0
	},

	// GOG Ultima VI - Enhanced
	{
		{
			"ultima6_enh",
			0,
			AD_ENTRY1s("converse.a", "5065716423ef1389e3f7b4946d815c26", 162615),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GAME_ULTIMA6,
		GF_VGA_ENHANCED
	},

	// Ultima VI - German Patch https://sirjohn.de/ultima-6/
	// Note: Not all user interface elements are translated in ScummVM
	{
		{
			"ultima6",
			0,
			AD_ENTRY1s("converse.a", "ae979230b97f8813bdf8f82698847435", 198627),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GAME_ULTIMA6,
		0
	},

	// Ultima VI - German Patch https://sirjohn.de/ultima-6/
	// Note: Not all user interface elements are translated in ScummVM
	{
		{
			"ultima6_enh",
			0,
			AD_ENTRY1s("converse.a", "ae979230b97f8813bdf8f82698847435", 198627),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GAME_ULTIMA6,
		GF_VGA_ENHANCED
	},

	// PC98 Ultima 6
	{
		{
			"ultima6",
			0,
			AD_ENTRY1s("converse.a", "99975e79e7cae3ee24a8e33982f60fe4", 190920),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_ULTIMA6,
		0
	},

	// Ultima VI - Nitpickers Delight older version
	{
		{
			"ultima6",
			0,
			AD_ENTRY1s("converse.a", "5c15ba2a75fb921b715a1a0bf0152bac", 165874),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GAME_ULTIMA6,
		0
	},
	{
		{
			"ultima6_enh",
			0,
			AD_ENTRY1s("converse.a","5c15ba2a75fb921b715a1a0bf0152bac", 165874),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GAME_ULTIMA6,
		GF_VGA_ENHANCED
	},

	// Ultima VI - Nitpickers Delight newer version
	{
		{
			"ultima6",
			0,
			AD_ENTRY1s("converse.a", "9f77c84a03efc77df2d53544d1275da8", 167604),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GAME_ULTIMA6,
		0
	},
	{
		{
			"ultima6_enh",
			0,
			AD_ENTRY1s("converse.a", "9f77c84a03efc77df2d53544d1275da8", 167604),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GAME_ULTIMA6,
		GF_VGA_ENHANCED
	},

	// Ultima VIII - CD (provided by ddeluca1com, bug #11944)
	{
		{
			"ultima8",
			"",
			AD_ENTRY1s("eusecode.flx", "5494165cbf4b07be04a465e28350e086", 1209018),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	// Ultima VIII - Ultima Collection 1998
	{
		{
			"ultima8",
			"",
			AD_ENTRY1s("eusecode.flx", "87c8b584e2947e5e4d99bd8bff6cea2e", 1251108),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	// GOG Ultima VIII
	{
		{
			"ultima8",
			"Gold Edition",
			AD_ENTRY1s("eusecode.flx", "c61f1dacde591cb39d452264e281f234", 1251108),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	{
		{
			"ultima8",
			"Gold Edition",
			AD_ENTRY1s("fusecode.flx", "4017eb8678ee24af0ce8c7647a05509b", 1300957),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	{
		{
			"ultima8",
			"Gold Edition",
			AD_ENTRY1s("gusecode.flx", "d69599a46870b66c1b7c02710ed185bd", 1378604),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	// German CD version
	{
		{
			"ultima8",
			"",
			AD_ENTRY1s("gusecode.flx", "dc981f82c6303548ad1c967cdef1a0ea", 1335445),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	{
		{
			"ultima8",
			"",
			AD_ENTRY1s("eusecode.flx", "1abad7a58e052ff3d9664df1ab2ddb86", 1136206),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	{
		{
			"ultima8",
			"",
			AD_ENTRY1s("jusecode.flx", "1793bb252b805bf8d59300690987c605", 1208003),
			Common::JA_JPN,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	// Crusader games use basically the same engine as ultima8, but still
	// need a lot of work.  All are unstable, some simply crash on startup.

	// GOG Crusader - No Remorse (V1.21)
	{
		{
			"remorse",
			"",
			AD_ENTRY1s("eusecode.flx", "0a0f64507adc4f280129c735ee9cad42", 556613),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REM,
		0
	},

	// Crusader - No Remorse provided by andy155
	{
		{
			"remorse",
			"",
			AD_ENTRY1s("eusecode.flx", "3fb211f4adfd80595078afc85bdfe7b4", 433143),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REM,
		0
	},

	// Crusader - No Remorse (Spanish) provided by Wesker
	{
		{
			"remorse",
			"",
			AD_ENTRY1s("eusecode.flx", "36a16d70c97d0379f1133cc743c31313", 558493),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REM,
		0
	},

	// Crusader - No Remorse (Japanese) provided by Dominus
	{
		{
			"remorse",
			"",
			AD_ENTRY1s("jusecode.flx", "088105959be4f2de1cb9e796e71c5f2d", 554522),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REM,
		0
	},

	// Crusader - No Remorse - Demo V1.12D
	{
		{
			"remorse",
			"Demo",
			AD_ENTRY1s("eusecode.flx", "41cdca35b62f4b2a7bb4c3b1ec782423", 556613),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REM,
		0
	},

	// GOG Crusader - No Regret (V1.06)
	{
		{
			"regret",
			"",
			AD_ENTRY1s("eusecode.flx", "1bb360156b7240a1f05eb9bda01c54db", 481652),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REG,
		0
	},

	// Crusader - No Regret - German V1.07NV
	{
		{
			"regret",
			"",
			AD_ENTRY1s("eusecode.flx", "1824d9725de45a8b49f058c12c6cf5c3", 484445),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REG,
		0
	},

	// Crusader - No Regret - Demo V1.08
	{
		{
			"regret",
			"Demo",
			AD_ENTRY1s("eusecode.flx", "c6416e4716f3c008dba113a2a460367e", 483174),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REG,
		0
	},

	// Crusader - No Regret (Spanish) provided by Wesker
	{
		{
			"regret",
			"",
			AD_ENTRY1s("eusecode.flx", "f5906654047ed1dab75760da6426ecfa", 478125),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_CRUSADER_REG,
		0
	},

	// GOG Martian Dreams
	{
		{
			"martiandreams",
			0,
			AD_ENTRY1s("talk.lzc", "6efafc030cb552028c564897e40d87b5", 409705),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_MARTIAN_DREAMS,
		0
	},

	// GOG Martian Dreams - Enhanced
	{
		{
			"martiandreams_enh",
			0,
			AD_ENTRY1s("talk.lzc", "6efafc030cb552028c564897e40d87b5", 409705),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_MARTIAN_DREAMS,
		GF_VGA_ENHANCED
	},


	// GOG The Savage Empire
	{
		{
			"thesavageempire",
			0,
			AD_ENTRY1s("talk.lzc", "bef60fbc3cc478b2a2e8f0883652b2f3", 160784),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_SAVAGE_EMPIRE,
		0
	},

	// GOG The Savage Empire
	{
		{
			"thesavageempire_enh",
			0,
			AD_ENTRY1s("talk.lzc", "bef60fbc3cc478b2a2e8f0883652b2f3", 160784),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_SAVAGE_EMPIRE,
		GF_VGA_ENHANCED
	},

	{ AD_TABLE_END_MARKER, (GameId)0, 0 }
};

} // End of namespace Ultima
