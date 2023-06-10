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

namespace MM {

static const MightAndMagicGameDescription GAME_DESCRIPTIONS[] = {
#ifdef ENABLE_MM1
	{
		// Might and Magic 1
		{
			"mm1",
			nullptr,
			AD_ENTRY1s("wallpix.dta", "86a7ef03fd5bf434d83012b07fa92680", 123059),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_TESTING,
			GUIO0()
		},
		GType_MightAndMagic1,
		0
	},

	{
		// Might and Magic 1 (Enhanced)
		{
			"mm1_enh",
			nullptr,
			AD_ENTRY1s("wallpix.dta", "86a7ef03fd5bf434d83012b07fa92680", 123059),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_TESTING,
			GUIO0()
		},
		GType_MightAndMagic1,
		GF_ENHANCED
	},

	{
		// Might and Magic 1 (Gfx Pack)
		// This isn't directly supported, because using it would allow playing
		// the game without actually having the original game files
		{
			"mm1",
			nullptr,
			AD_ENTRY1s("wallpix.dta", "64af4dad1f86aedf307e8184a9f12ba9", 164220),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_MightAndMagic1,
		GF_GFX_PACK
	},
#endif

#ifdef ENABLE_XEEN
	{
		// World of Xeen
		{
			"worldofxeen",
			nullptr,
			AD_ENTRY2s("xeen.cc", "0cffbab533d9afe140e69ec93096f43e", 13435646,
					   "dark.cc", "df194483ecea6abc0511637d712ced7c", 11217676),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_WorldOfXeen,
		0
	},

	{
		// World of Xeen (Monster Spawn Mod v1.0), Bugreport #12714
		{
			"worldofxeen",
			"Monster Spawn Mod v1.0",
			AD_ENTRY2s("xeen.cc", "37767811a52fb54a8f2be3b45acc91a9", 13536623,
					   "dark.cc", "a4dcb0731ad3818fdd908f17230ee773", 11222393),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
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
			AD_ENTRY2s("xeen.cc", "f4e4b3ddc43bd102dbe1637f480f1fa1", 13214150,
					   "dark.cc", "84a86bbbc5f2fe96c0b0325485ed8203", 11168312),
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
			AD_ENTRY2s("xeen.cc", "964078c53f649937ce9a1a3596ce3d9f", 13438429,
					   "dark.cc", "7f755ce39ea614fa6adb016f8bfc6e43", 11288403),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_WorldOfXeen,
		0
	},

	{
		// World of Xeen French- French untested
		// https://bugs.scummvm.org/ticket/12559
		{
			"worldofxeen",
			nullptr,
			AD_ENTRY2s("xeen.cc", "b9bb33bbd3783e7930ddedb189d7ba54", 13203565,
					   "dark.cc",  "437618b6b5e76f174719dcb529edbfbb", 11152923),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
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
			AD_ENTRY1s("xeen.cc", "0cffbab533d9afe140e69ec93096f43e", 13435646),
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
			AD_ENTRY1s("xeen.cc", "f4e4b3ddc43bd102dbe1637f480f1fa1", 13214150),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_Clouds,
		0
	},

	{
		// Clouds of Xeen, Russian fan-translation
		{
			"cloudsofxeen",
			nullptr,
			AD_ENTRY1s("xeen.cc", "1dead179f65bbb1ba11dd51d69768489", 13417817),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_Clouds,
		0
	},

	{
		// Clouds of Xeen (Chinese)
		{
			"cloudsofxeen",
			nullptr,
			AD_ENTRY1s("xeen.cc", "1fea0587fe139b13f4effa2b5791bd80", 13859999),
			Common::ZH_TWN,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_Clouds,
		0
	},

	{
		// Clouds of Xeen- French untested
		// https://bugs.scummvm.org/ticket/12557
		{
			"cloudsofxeen",
			nullptr,
			AD_ENTRY1s("xeen.cc", "b9bb33bbd3783e7930ddedb189d7ba54", 13203565),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_Clouds,
		0
	},

	{
		// Darkside of Xeen
		{
			"darksideofxeen",
			nullptr,
			AD_ENTRY1s("dark.cc", "df194483ecea6abc0511637d712ced7c", 11217676),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_DarkSide,
		0
	},

	{
		// Darkside of Xeen (GOG German)
		{
			"darksideofxeen",
			nullptr,
			AD_ENTRY1s("dark.cc", "84a86bbbc5f2fe96c0b0325485ed8203", 11168312),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_DarkSide,
		0
	},

	{
		// Darkside of Xeen (Bestseller Games Magazine #8 German)
		{
			"darksideofxeen",
			nullptr,
			AD_ENTRY2s("dark.cc",  "84a86bbbc5f2fe96c0b0325485ed8203", 11173657,
					   "intro.cc", "e47a7ab0223cf32b2d87eed91d024c35", 8899953),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_DarkSide,
		0
	},

	{
		// Darkside of Xeen (Chinese)
		{
			"darksideofxeen",
			nullptr,
			AD_ENTRY1s("dark.cc", "4dcbcdb0be885afebc2ab6c9e60c434e", 11137412),
			Common::ZH_TWN,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_DarkSide,
		0
	},

	{
		// Darkside of Xeen- French untested
		// https://bugs.scummvm.org/ticket/12558
		{
			"darksideofxeen",
			nullptr,
			AD_ENTRY1s("dark.cc", "437618b6b5e76f174719dcb529edbfbb", 11152923),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
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
			AD_ENTRY1s("swrd.cc", "0d51c3457070cc7d1a596da9241924a5", 13026924),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_SHOW_ITEM_COSTS, GAMEOPTION_DURABLE_ARMOR)
		},
		GType_Swords,
		0
	},
#endif

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace MM
