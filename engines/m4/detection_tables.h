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
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
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
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_Burger,
		kFeaturesCD
	},
	{
		{
			"burger",
			nullptr,
			AD_ENTRY1s("burger.has", "795c98a74e351ec437a396bb29897daf", 730771),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
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
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_Burger,
		kFeaturesDemo
	},
	{
		{
			"burger",
			"Demo",
			AD_ENTRY1s("overview.has", "57aa43a3ef88a934a43e9b1890ef5e17", 10519),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_Burger,
		kFeaturesDemo
	},
	{
		{
			"riddle",
			nullptr,
			AD_ENTRY1s("ripley.has", "056d517360c89eb4c297a319f21a7071", 700469),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_Riddle,
		kFeaturesCD
	},
	{
		{
			"riddle",
			nullptr,
			AD_ENTRY1s("ripley.has", "d073582c9011d44dd0d7e2ede317a86d", 700469),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
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
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
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
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
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
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_Riddle,
		kFeaturesCD
	},
	{	// Demo
		{
			"riddle",
			"Demo",
			AD_ENTRY1s("ripley.has", "3a90dd7052860b6e246ec7e0aaf202f6", 104527),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_Riddle,
		kFeaturesDemo
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // End of namespace M4
