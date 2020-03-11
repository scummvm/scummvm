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

	// GOG Ultima VI
	{
		{
			"ultima6",
			0,
			AD_ENTRY1s("converse.a", "5065716423ef1389e3f7b4946d815c26", 162615),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
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
			ADGF_UNSTABLE,
			GUIO0()
		},
		GAME_ULTIMA6,
		GF_VGA_ENHANCED
	},

	// Ultima VIII - Ultima Collection 1998
	{
		{
			"ultima8",
			"",
			AD_ENTRY1s("eusecode.flx", "87c8b584e2947e5e4d99bd8bff6cea2e", 1251108),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
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
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	{
		{
			"ultima8",
			"Gold Edition",
			{
				{ "eusecode.flx", 0, "c61f1dacde591cb39d452264e281f234", 1251108 },
				{ "fusecode.flx", 0, "4017eb8678ee24af0ce8c7647a05509b", 1300957 },
				AD_LISTEND
			},			
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
		0
	},

	{
		{
			"ultima8",
			"Gold Edition",
			{
				{"eusecode.flx", 0, "c61f1dacde591cb39d452264e281f234", 1251108},
				{"gusecode.flx", 0, "d69599a46870b66c1b7c02710ed185bd", 1378604},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8,
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
