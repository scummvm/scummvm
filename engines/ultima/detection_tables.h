/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

namespace Ultima {

#define GUI_OPTIONS_ULTIMA1	GUIO0()
#define GUI_OPTIONS_ULTIMA4	GUIO1(GUIO_NOSPEECH)
#define GUI_OPTIONS_ULTIMA6	GUIO0()
#define GUI_OPTIONS_MARTIAN_DREAMS GUIO0()
#define GUI_OPTIONS_SAVAGE_EMPIRE  GUIO0()

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
			GUI_OPTIONS_ULTIMA1
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
			GUI_OPTIONS_ULTIMA1
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
			GUI_OPTIONS_ULTIMA1
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
			GUI_OPTIONS_ULTIMA4
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
			GUI_OPTIONS_ULTIMA4
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
			GUI_OPTIONS_ULTIMA6
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
			GUI_OPTIONS_ULTIMA6
		},
		GAME_ULTIMA6,
		GF_VGA_ENHANCED
	},

	// Ultima VI - French patch by Docwise Dragon
	// https://sirjohn.de/en/ultima6/ultima-vi-french-translation-patch/
	// Note: Not all user interface elements are translated in ScummVM
	{
		{
			"ultima6",
			0,
			AD_ENTRY1s("converse.a", "35c95d56737d957db7e72193e810053b", 182937),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_ULTIMA6
		},
		GAME_ULTIMA6,
		0
	},

	// Ultima VI - Enhanced - French patch by Docwise Dragon
	// https://sirjohn.de/en/ultima6/ultima-vi-french-translation-patch/
	// Note: Not all user interface elements are translated in ScummVM
	{
		{
			"ultima6_enh",
			0,
			AD_ENTRY1s("converse.a", "35c95d56737d957db7e72193e810053b", 182937),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_ULTIMA6
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
			GUI_OPTIONS_ULTIMA6
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
			GUI_OPTIONS_ULTIMA6
		},
		GAME_ULTIMA6,
		GF_VGA_ENHANCED
	},

	// Ultima VI - German Patch 1.6 https://sirjohn.de/ultima-6/
	// Note: Not all user interface elements are translated in ScummVM
	{
		{
			"ultima6",
			0,
			AD_ENTRY1s("converse.a", "5242f0228bbc9c3a60c7aa6071499688", 198797),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_ULTIMA6
		},
		GAME_ULTIMA6,
		0
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
			GUI_OPTIONS_ULTIMA6
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
			GUI_OPTIONS_ULTIMA6
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
			GUI_OPTIONS_ULTIMA6
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
			GUI_OPTIONS_ULTIMA6
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
			GUI_OPTIONS_ULTIMA6
		},
		GAME_ULTIMA6,
		GF_VGA_ENHANCED
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
			GUI_OPTIONS_MARTIAN_DREAMS
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
			GUI_OPTIONS_MARTIAN_DREAMS
		},
		GAME_MARTIAN_DREAMS,
		GF_VGA_ENHANCED
	},


	// The Savage Empire v1.6
	{
		{
			"thesavageempire",
			0,
			AD_ENTRY1s("talk.lzc", "bef60fbc3cc478b2a2e8f0883652b2f3", 160784),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUI_OPTIONS_SAVAGE_EMPIRE
		},
		GAME_SAVAGE_EMPIRE,
		0
	},

	// The Savage Empire v1.6
	{
		{
			"thesavageempire_enh",
			0,
			AD_ENTRY1s("talk.lzc", "bef60fbc3cc478b2a2e8f0883652b2f3", 160784),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUI_OPTIONS_SAVAGE_EMPIRE
		},
		GAME_SAVAGE_EMPIRE,
		GF_VGA_ENHANCED
	},

	// The Savage Empire v2.1
	{
		{
			"thesavageempire",
			0,
			AD_ENTRY1s("talk.lzc", "1bbb5a425e1d7e2e3aa9b887e511ffc6", 160931),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUI_OPTIONS_SAVAGE_EMPIRE
		},
		GAME_SAVAGE_EMPIRE,
		0
	},

	// The Savage Empire v2.1
	{
		{
			"thesavageempire_enh",
			0,
			AD_ENTRY1s("talk.lzc", "1bbb5a425e1d7e2e3aa9b887e511ffc6", 160931),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUI_OPTIONS_SAVAGE_EMPIRE
		},
		GAME_SAVAGE_EMPIRE,
		GF_VGA_ENHANCED
	},

	// The Savage Empire v2.1
	{
		{
			"thesavageempire",
			0,
			AD_ENTRY1s("talk.lzc", "1bbb5a425e1d7e2e3aa9b887e511ffc6", 160931),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_SAVAGE_EMPIRE,
		0
	},

	// The Savage Empire v2.1
	{
		{
			"thesavageempire_enh",
			0,
			AD_ENTRY1s("talk.lzc", "1bbb5a425e1d7e2e3aa9b887e511ffc6", 160931),
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
