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

namespace Xeen {

static const XeenGameDescription gameDescriptions[] = {
	{
		// World of Xeen
		{
			"worldofxeen",
			nullptr,
			{
				{ "xeen.cc", 0, "0cffbab533d9afe140e69ec93096f43e", 13435646 },
				{ "dark.cc", 0, "df194483ecea6abc0511637d712ced7c", 11217676 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_WorldOfXeen,
		0
	},

	{
		// World of Xeen (Bestseller Games Magazine #6 + #8 German)
		{
			"worldofxeen",
			nullptr,
			{
				{"xeen.cc", 0, "f4e4b3ddc43bd102dbe1637f480f1fa1", 13214150},
				{"dark.cc", 0, "84a86bbbc5f2fe96c0b0325485ed8203", 11173657},
				{"intro.cc", 0, "e47a7ab0223cf32b2d87eed91d024c35", 8899953},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_WorldOfXeen,
		0
	},

	{
		// World of Xeen (GOG German)
		{
			"worldofxeen",
			nullptr,
			{
				{ "xeen.cc", 0, "f4e4b3ddc43bd102dbe1637f480f1fa1", 13214150 },
				{ "dark.cc", 0, "84a86bbbc5f2fe96c0b0325485ed8203", 11168312 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_WorldOfXeen,
		0
	},

	{
		// World of Xeen (2 CD talkie version)
		{
			"worldofxeen",
			"CD",
			{
				{"xeen.cc", 0, "964078c53f649937ce9a1a3596ce3d9f", 13438429},
				{"dark.cc", 0, "7f755ce39ea614fa6adb016f8bfc6e43", 11288403},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_WorldOfXeen,
		0
	},

	{
		// Clouds of Xeen
		{
			"cloudsofxeen",
			nullptr,
			{
				{ "xeen.cc", 0, "0cffbab533d9afe140e69ec93096f43e", 13435646 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_Clouds,
		0
	},

	{
		// Clouds of Xeen (GOG, Bestseller Games Magazine #6 German)
		{
			"cloudsofxeen",
			nullptr,
			{
				{ "xeen.cc", 0, "f4e4b3ddc43bd102dbe1637f480f1fa1", 13214150 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_Clouds,
		0
	},

	{
		// Dark Side of Xeen
		{
			"darksideofxeen",
			nullptr,
			{
				{ "dark.cc", 0, "df194483ecea6abc0511637d712ced7c", 11217676 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_DarkSide,
		0
	},

	{
		// Dark Side of Xeen (GOG German)
		{
			"darksideofxeen",
			nullptr,
			{
				{ "dark.cc", 0, "84a86bbbc5f2fe96c0b0325485ed8203", 11168312 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_DarkSide,
		0
	},

	{
		// Dark Side of Xeen (Bestseller Games Magazine #8 German)
		{
			"darksideofxeen",
			nullptr,
			{
				{ "dark.cc", 0, "84a86bbbc5f2fe96c0b0325485ed8203", 11173657},
				{ "intro.cc", 0, "e47a7ab0223cf32b2d87eed91d024c35", 8899953},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_DarkSide,
		0
	},

	{
		// Swords of Xeen
		{
			"swordsofxeen",
			nullptr,
			{
				{"swrd.cc", 0, "0d51c3457070cc7d1a596da9241924a5", 13026924 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_Swords,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace Xeen
