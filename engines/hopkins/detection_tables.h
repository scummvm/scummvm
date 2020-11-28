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

namespace Hopkins {

#define GAMEOPTION_GORE_DEFAULT_ON  GUIO_GAMEOPTIONS1
#define GAMEOPTION_GORE_DEFAULT_OFF GUIO_GAMEOPTIONS2

static const HopkinsGameDescription gameDescriptions[] = {
	{
		// Hopkins FBI Linux Demo UK 1.00 and 1.02
		{
			"hopkins",
			"Linux Demo",
			AD_ENTRY1s("RES_VAN.RES", "29414c05be8f9fe794c61572a65def12", 16060544),
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_DEMO,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI OS/2, provided by Strangerke
		{
			"hopkins",
			0,
			AD_ENTRY2s("ENG_VOI.RES",	"fa5789d1d8c19d160bce44a33e742fdf", 66860711,
					   "CREAN.TXT",		"e13aa69d9e043f066776e1d0ef98fdf5", 1871),
			Common::EN_ANY,
			Common::kPlatformOS2,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI BeOS, provided by Strangerke & Eriktorbjorn
		{
			"hopkins",
			0,
			AD_ENTRY1s("ENG_VOI.RES", "fa5789d1d8c19d160bce44a33e742fdf", 66860711),
			Common::EN_ANY,
			Common::kPlatformBeOS,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI Win95 Spanish
		{
			"hopkins",
			0,
			AD_ENTRY1s("RES_VES.RES", "77ee08896466ae88cc1af3bf1a0bf78c", 32882302),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI Win95 UK, provided by Strangerke, alexbevi, greencis
		{
			"hopkins",
			0,
			AD_ENTRY1s("RES_VAN.RES", "f1693ac0b0859c8ecd8cb30ff43cf55f", 38296346),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_OFF, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI Win95 RU, provided by greencis in bug #3613068
		{
			"hopkins",
			0,
			AD_ENTRY1s("res_van.res", "bf17c710e184a25a6c8e9d1d9503c38e", 32197685),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI Linux, provided by Strangerke
		{
			"hopkins",
			0,
			AD_ENTRY1s("RES_VFR.RES", "0490d4d1aa71075ebf71cc79e5dc7894", 39817945),
			Common::FR_FRA,
			Common::kPlatformLinux,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI Linux, provided by Strangerke
		{
			"hopkins",
			0,
			AD_ENTRY1s("RES_VAN.RES", "29414c05be8f9fe794c61572a65def12", 38832455),
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI Win95, French, provided by SylvainTV
		{
			"hopkins",
			0,
			AD_ENTRY1s("RES_VFR.RES", "b8a3849063c9eeefe80e82cfce1ad3cd", 39269361),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},

	},
	{
		// Hopkins FBI Win95 Polish, provided by Paput in bug #6511
		{
			"hopkins",
			0,
			AD_ENTRY1s("RES_VAN.RES", "f2fec5172e4a7a9d35cb2a5f948ef6a9", 39400865),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_GORE_DEFAULT_OFF, GUIO_NOMIDI)
		},
	},

	{
		// Hopkins FBI Win95 Demo, provided by Strangerke
		// CHECKME: No voice! a second file is required though... Also, it has multi-language support
		{
			"hopkins",
			"Win95 Demo",
			AD_ENTRY1s("Hopkins.exe", "0c9ebfe371f4dcf84a49f333f04839a0", 376897),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO2(GAMEOPTION_GORE_DEFAULT_ON, GUIO_NOMIDI)
		},
	},
	{
		// Hopkins FBI Win95 Polish Demo, provided by Strangerke
		{
			"hopkins",
			"Win95 Demo",
			AD_ENTRY1s("RES_VAN.RES", "8262cfba261c200af4451902689dffe0", 12233202),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO2(GAMEOPTION_GORE_DEFAULT_OFF, GUIO_NOMIDI)
		},
	},
	{ AD_TABLE_END_MARKER }
};

} // End of namespace Hopkins
