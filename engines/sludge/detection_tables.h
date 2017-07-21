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

namespace Sludge {

static const SludgeGameDescription gameDescriptions[] = {
	{
		{
			"welcome",
			"",
			AD_ENTRY1s("Welcome.slg", "50445503761cf6684fe3270d0860a4c3", 51736),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"verbcoin",
			"",
			AD_ENTRY1s("Verb Coin.slg", "e39ec315dcbf3a1137481f0a5fe1617d", 980270),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"verbcoin",
			"",
			AD_ENTRY1s("Verb Coin.slg", "e39ec315dcbf3a1137481f0a5fe1617d", 980270),
			Common::DE_DEU,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		1
	},

	{
		{
			"robinsrescue",
			"",
			AD_ENTRY1s("robins_rescue.slg", "16cbf2bf916ed89f9c1b14fab133cf96", 14413769),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"outoforder",
			"",
			AD_ENTRY1s("gamedata", "4d72dbad0ff170169cd7e4e7e389a90d", 21122647),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"frasse",
			"",
			AD_ENTRY1s("Gamedata.slg", "e4eb4eca6117bb9b77870bb74af453b4", 88582819),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"interview",
			"",
			AD_ENTRY1s("gamedata", "6ca8f6e44f30d09bd68e008be4c20e8d", 2570140),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"life",
			"",
			AD_ENTRY1s("LifeFlashesBy.slg", "a471759e071e5d2c0e8e6887607df778", 163794266),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	// TODO: the games down here are windows-only and can't be successfully run by
	// sludge engine nor scummvm, need to solve it
	{
		{
			"tgttpoacs",
			"",
			AD_ENTRY1s("gamedata", "d5ec4d7d8440f7744335d25d25e1e943", 40368),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"mandy",
			"",
			AD_ENTRY1s("gamedata", "bb51eea418d87071c98f4e050ccf6387", 589),
			Common::EN_ANY,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"cubert",
			"",
			AD_ENTRY1s("gamedata", "0078eb54f63cc0a22e50f17d904fcfde", 26799),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Wage
