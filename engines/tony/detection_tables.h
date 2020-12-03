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

namespace Tony {

static const TonyGameDescription gameDescriptions[] = {
	{
		// Tony Tough English not installed
		{
			"tony",
			0,
			// TODO: AdvancedDetector seems to have a problem where it thinks data1.cab is unrecognized.
			// Is it perhaps because the Agos engine also has detection entries for data1.cab?
			AD_ENTRY1s("data1.cab", "ce82907242166bfb594d97bdb68f96d2", 4350),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Czech not installed
		{
			"tony",
			0,
			// TODO: AdvancedDetector seems to have a problem where it thinks data1.cab is unrecognized.
			// Is it perhaps because the Agos engine also has detection entries for data1.cab?
			AD_ENTRY1s("data1.cab", "c6d5dd8f0c1241a6e3f7861b7f27bf7b", 4350),
			Common::CZ_CZE,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough English Demo
		{
			"tony",
			"Extracted Demo",
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 14972409,
					   "roasted.mpc", "1e247922ec869712bfd96625bc4d3c7c", 39211),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough English Demo (Compressed)
		{
			"tony",
			"Demo",
			AD_ENTRY1s("data1.cab", "7d8b6d308f96aee3968ad7910fb11e6d", 58660608),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | GF_COMPRESSED,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough French "Collection Aventure" provided by Strangerke
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "e890c6a41238827bdfa9874a65618b69", 374135),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough German "Shoe Box" provided by Strangerke
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "ccf7ab939a34de1b13df538596431684", 389554),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Italian provided by Fabio Barzagli
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "1dc896cdb945170d7408598f803411c1", 380001),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Italian provided by Giovanni Bajo
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "6202816f991b15af82aab84e3e4be011", 380183),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Polish provided by Fabio Barzagli
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "89733ea710669acc8e7900b115f4afef", 389625),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Polish provided by iGom bug #11546
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "c212a81e34edf92bc177a80f24780bd2", 380200),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough German "Gamestar" provided in bug #3566035
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "187de6f88f4083808cb66342ab55a7fd", 389904),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Czech provided in bug #3565765
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "a8283a101878f3ca105f1f83f07e2c40", 386491),
			Common::CZ_CZE,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough English Unpacked
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "57c4a3860cf899443c357e0078ea6f49", 366773),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough German "Shoe Box", reported in bug #3582420
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "bc3471f098e591dc509dcad401a8d8a5", 389554),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Tony Tough Russian, reported in bug #6589
		{
			"tony",
			0,
			AD_ENTRY2s("roasted.mpr", "06203dbbc85fdd1e6dc8fc211c1a6207", 135911071,
					   "roasted.mpc", "377d6e24adeedc6c5c09c31b92231218", 391536),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Tony
