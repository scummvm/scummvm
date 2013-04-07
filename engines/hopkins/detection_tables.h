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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

namespace Hopkins {

static const HopkinsGameDescription gameDescriptions[] = {
	{
		// Hopkins FBI Linux Demo 1.00
		{
			"hopkins",
			"Linux Demo v1.00",
			{
				{"Hopkins-PDemo.bin", 0, "88b4d6e14b9b1407083cb3d1213c0fa7", 272027},
				{"RES_VAN.RES", 0, "29414c05be8f9fe794c61572a65def12", 16060544},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// Hopkins FBI Linux Demo 1.02
		{
			"hopkins",
			"Linux Demo v1.02",
			{
				{"Hopkins-PDemo.bin", 0, "f82f4e698f3a189419351be0de2b2f8e", 273760},
				{"RES_VAN.RES", 0, "29414c05be8f9fe794c61572a65def12", 16060544},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// Hopkins FBI OS/2, provided by Strangerke
		{
			"hopkins",
			0,
			{
				{"Hopkins.exe", 0, "63d45f882278e5a9fa1027066223e5d9", 292864},
				{"ENG_VOI.RES", 0, "fa5789d1d8c19d160bce44a33e742fdf", 66860711},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformOS2,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Win95 Demo, provided by Strangerke
		// CHECKME: No voice! a second file is required though... Also, it has multi-language support
		{
			"hopkins",
			"Win95 Demo",
			{
				{"Hopkins.exe", 0, "0c9ebfe371f4dcf84a49f333f04839a0", 376897},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Win95 Polish Demo, provided by Strangerke
		{
			"hopkins",
			"Win95 Demo",
			{
				{"Hopkins.exe", 0, "7595c0b9374739b212ee9f8f412ac716", 307200},
				{"RES_VAN.RES", 0, "8262cfba261c200af4451902689dffe0", 12233202},
				AD_LISTEND
			},
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Win95 Spanish
		{
			"hopkins",
				0,
			{
				{"Hopkins.exe", 0, "31c837378bb2e0b2573befea44956d3f", 421386},
				{"RES_VES.RES", 0, "77ee08896466ae88cc1af3bf1a0bf78c", 32882302},
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Win95, provided by Strangerke
		{
			"hopkins",
			0,
			{
				{"Hopkins.exe", 0, "277a5c144bf9ec7d8450ae37afb85090", 419281},
				{"RES_VAN.RES", 0, "f1693ac0b0859c8ecd8cb30ff43cf55f", 38296346},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Win95, provided by alexbevi
		// Dec 15  1998 hopkins.exe
		{
			"hopkins",
			0,
			{
				{"Hopkins.exe", 0, "a587762dd50d5933e1c89f9975180764", 378694},
				{"RES_VAN.RES", 0, "f1693ac0b0859c8ecd8cb30ff43cf55f", 38296346},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Linux, provided by Strangerke
		{
			"hopkins",
			0,
			{
				{"Hopkins.bin", 0, "71611380cb31744bf909b8319a65e6e6", 275844},
				{"RES_VFR.RES", 0, "0490d4d1aa71075ebf71cc79e5dc7894", 39817945},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformLinux,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},
	{
		// Hopkins FBI Linux, provided by Strangerke
		{
			"hopkins",
			0,
			{
				{"Hopkins.bin", 0, "71611380cb31744bf909b8319a65e6e6", 275844},
				{"RES_VAN.RES", 0, "29414c05be8f9fe794c61572a65def12", 38832455},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// Hopkins FBI BeOS, provided by Strangerke
		{
			"hopkins",
			0,
			{
				{"ENG_VOI.RES", 0, "fa5789d1d8c19d160bce44a33e742fdf", 66860711},
				{"Hopkins_ FBI", 0, "8940ce2e618c42691b66aad5d6c223b0", 757936},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformBeOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// Hopkins FBI BeOS, uninstalled, provided by eriktorbjorn
		{
			"hopkins",
			0,
			{
				{"ENG_VOI.RES", 0, "fa5789d1d8c19d160bce44a33e742fdf", 66860711},
				{"Hopkins.pkg", 0, "72f97806dd3d5fc0c0eb24196f180618", 285017},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformBeOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},

	},

	{
		// Hopkins FBI Win32, French uninstalled, provided by SylvainTV
		{
			"hopkins",
				0,
			{
				{"Hopkins.exe", 0, "277a5c144bf9ec7d8450ae37afb85090", 419281},
				{"RES_VFR.RES", 0, "b8a3849063c9eeefe80e82cfce1ad3cd", 39269361},
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},

	},


	{ AD_TABLE_END_MARKER }
};

} // End of namespace Hopkins
