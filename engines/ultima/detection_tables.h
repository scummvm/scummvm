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

static const Ultima::UltimaGameDescription GAME_DESCRIPTIONS[] = {
#ifdef ENABLE_ULTIMA6
	// GOG Ultima 6
	{
		{
			"ultima6",
			nullptr,
			AD_ENTRY1s("converse.a", "5065716423ef1389e3f7b4946d815c26", 162615),
			Common::EN_ANY,
			Common::kPlatformDOS,
			0,
			GUIO0()
		},
		GAME_ULTIMA6
	},

	// GOG Ultima 6 - Enhanced
	{
		{
			"ultima6_enh",
			nullptr,
			AD_ENTRY1s("converse.a", "5065716423ef1389e3f7b4946d815c26", 162615),
			Common::EN_ANY,
			Common::kPlatformDOS,
			0,
			GUIO0()
		},
		GAME_ULTIMA6_ENHANCED
	},

#endif

#ifdef ENABLE_ULTIMA8
	{
		{
			"ultima8",
			"Gold Edition",
			AD_ENTRY1s("eusecode.flx", "c61f1dacde591cb39d452264e281f234", 1251108),
			Common::EN_ANY,
			Common::kPlatformDOS,
			0,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8
	},

	{
		{
			"ultima8",
			"Gold Edition",
			//AD_ENTRY1s("fusecode.flx", "4017eb8678ee24af0ce8c7647a05509b", 1300957),
			{
				{ "eusecode.flx", 0, "c61f1dacde591cb39d452264e281f234", 1251108 },
				{ "fusecode.flx", 0, "4017eb8678ee24af0ce8c7647a05509b", 1300957 },
				AD_LISTEND
			},			
			Common::FR_FRA,
			Common::kPlatformDOS,
			0,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8
	},

	{
		{
			"ultima8",
			"Gold Edition",
			//AD_ENTRY1s("gusecode.flx", "d69599a46870b66c1b7c02710ed185bdx", 0),
			{
				{"eusecode.flx", 0, "c61f1dacde591cb39d452264e281f234", 1251108},
				{"gusecode.flx", 0, "d69599a46870b66c1b7c02710ed185bd", 1378604},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			0,
			GUIO1(GUIO_NOMIDI)
		},
		GAME_ULTIMA8
	},
#endif

	{ AD_TABLE_END_MARKER, (GameId)0 }
};

} // End of namespace Ultima
