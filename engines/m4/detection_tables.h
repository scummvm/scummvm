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

namespace M4 {

static const PlainGameDescriptor m4Games[] = {
	{ "m4", "MADS/M4 engine game" },
	{ "riddle", "Ripley's Believe It or Not!: The Riddle of Master Lu" },
	{ "burger", "Orion Burger" },
	{ 0, 0 }
};

static const M4GameDescription gameDescriptions[] = {
	{
		{
			"burger",
			nullptr,
			AD_ENTRY1s("burger.has", "10c8064e9c771072122f50737ac97245", 730771),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Burger,
		kFeaturesCD
	},
	{
		{
			"burger",
			nullptr,
			AD_ENTRY1s("burger.has", "3887c30f0d2f93fed7549120748c07fd", 724755),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Burger,
		kFeaturesCD
	},
	{
		{
			"burger",
			nullptr,
			AD_ENTRY1s("burger.has", "55be8693a4c36e7efcdca0f0c77758ae", 724191),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Burger,
		kFeaturesCD
	},
	// TODO: The production of the Russian version is totally screwed up.
	// In the German version, the vocab field for each hotspot loaded in adv_hotspot.cpp
	// has the English version, and this is separate from the text shown on-screen
	// which is German in the German version, and English in the English version.
	// But for the Russian they changed this text to be Russian, as well as changed
	// all rooms' pre_parser/parser methods to use it. As such, to support it we'd need to:
	// 1) Add the Russian text to all the room methods. Hell no.
	// 2) Or prepare a data file that strips the vocab for each room,
	// and uses it when the Russian version is being played.
	{
		{
			"burger",
			nullptr,
			AD_ENTRY1s("burger.has", "795c98a74e351ec437a396bb29897daf", 730771),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Burger,
		kFeaturesCD
	},
	{
		{
			"burger",
			"Demo",
			AD_ENTRY1s("burger.has", "fc3f363b6153240a448bd3b7be9318da", 62159),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Burger,
		kFeaturesDemo
	},
	{
		{
			"burger",
			"Demo",
			AD_ENTRY1s("burger.has", "eee638f804c53b640218fbbb8c681fdc", 88137),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Burger,
		kFeaturesDemo
	},
	{
		{
			"burger",
			"Non-Interactive Demo",
			AD_ENTRY1s("overview.has", "57aa43a3ef88a934a43e9b1890ef5e17", 10519),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Burger,
		kFeaturesNonInteractiveDemo
	},

	{	// v1.08 English
		{
			"riddle",
			nullptr,
			AD_ENTRY1s("ripley.has", "056d517360c89eb4c297a319f21a7071", 700469),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesCD
	},
	{	// v2.05 English Kixx rerelease
		{
			"riddle",
			nullptr,
			AD_ENTRY1s("ripley.has", "d073582c9011d44dd0d7e2ede317a86d", 700469),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			nullptr,
			AD_ENTRY1s("ripley.has", "d9e9f8befec432a047b1047fb2bc7aea", 710997),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			nullptr,
			AD_ENTRY1s("ripley.has", "3d48c5700785d11e6a5bc832b95be918", 701973),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			nullptr,
			AD_ENTRY1s("ripley.has", "5ee011cff7178dae3ddf6f9b7d4102ac", 701691),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			nullptr,
			AD_ENTRY1s("ripley.has", "30ad07f5c52af0cb72020df042674aa8", 704511),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesCD
	},
	{	// Game Version DELTOID - June 8, 1995
		{
			"riddle",
			"Demo",
			AD_ENTRY1s("ripley.has", "3a90dd7052860b6e246ec7e0aaf202f6", 104527),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesDemo
	},
	{	// Game Version DELTOID - June 8, 1995
		{
			"riddle",
			"Demo",
			AD_ENTRY1s("option1/ripley.has", "3a90dd7052860b6e246ec7e0aaf202f6", 104527),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesDemo
	},
	{	// Game Version NYUK -- July 13, 1995
		{
			"riddle",
			"Demo2",
			// Small/Medium/Vast installation
			AD_ENTRY1s("ripley.has", "2c5e80441c4ace6ff4ffe0ecd4099a58", 149057),
			// Large/Huge installation
			// AD_ENTRY1s("ripley.has", "daaff94287aec861e27b1e7b37811229", 164319),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesDemo
	},
	{	// Game Version OBVIATE -- Aug 2, 1995
		{
			"riddle",
			"Demo",
			AD_ENTRY1s("ripley.has", "df74ee9ceefb006638839572f0144256", 74353),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesDemo
	},
	{	// Game Version German version 1.1 -- Apr 2, 1996
		{
			"riddle",
			"Demo",
			AD_ENTRY1s("RIPLEY.HAS", "810b27cf5e2c119ad2ec0a9e550eb32c", 119097),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOMUSIC)
		},
		GType_Riddle,
		kFeaturesDemo
	},
	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace M4
