/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */

#define GAMEOPTION_OBJECT_LABELS GUIO_GAMEOPTIONS1

namespace Sword2 {

static const Sword2GameDescription gameDescriptions[] = {
	{
		{
			"sword2",
			"Demo",
			AD_ENTRY1s("general.clu", "11e824864a75195652610e8b397382a6", 8030769),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword2",
			"PC Gamer Demo",
			AD_ENTRY1s("general.clu", "522ecd261027f0b55315a32aaef97295", 4519015),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword2",
			"Demo",
			AD_ENTRY1s("players.clu", "5068815a62ba932afba7267bafc9786d", 9904289),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO | GF_SPANISHDEMO
	},

	{
		{
			"sword2",
			"Demo",
			AD_ENTRY1s("players.clu", "e8786804d399310bda3fcbf897bc44f7", 3085812),
			Common::EN_ANY,
			Common::kPlatformPSX,
			ADGF_DEMO,
			GUIO0()
		},
		GF_DEMO
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY3s("general.clu",  "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",     "9b344d976ca8d19a1cf5aa4413397f6b", 304968,
			           "speech1.clu",  "a403904a0e825356107d228f8f74092e", 176260048),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"1CD release",
			AD_ENTRY2s("general.clu",  "31db8564f9187538f24d9fda0677f666", 7059728,
			           "resource.tab", "ee4c0a8a2b8821ca113ea4176968b857", 16588),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"EU",
			AD_ENTRY3s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "9b344d976ca8d19a1cf5aa4413397f6b", 304968,
			           "speech1.clg", "d49a5f3683b734d1129cbf6a0f95ae83", 57935499),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "d0cafb4d2982613ca4cf0574a0e4e079", 418165),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "5771f52410745029d7f71af05072d3d6", 556961),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "56c1197e72249473538c30c912607d01", 418165),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "c141e9903e4a1f45252dd1500498b6e2", 488745),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"English speech",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "bc45e00cfb737ad61fada3ca6b1b2bfc", 279042),
			Common::CS_CZE,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"English speech",
			AD_ENTRY2s("general.clu", "11e824864a75195652610e8b397382a6", 8030769,
			           "text.clu",    "9867bb6dfc850bfa165812f0827a5508", 454229),
			Common::FI_FIN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"English speech",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "93ea23ccf78dc746ed9a027fcf66d58d", 248692),
			Common::HE_ISR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"English speech",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "82714fa70516486174cddc2754958cd4", 304968),
			Common::HU_HUN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"English speech",
			AD_ENTRY2s("general.clu", "11e824864a75195652610e8b397382a6", 8030769,
			           "text.clu",    "f1cf2aaa7e56d8bf6572c9b25267931e", 373704),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"English speech",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "cda6306bedfa63ac4386ff82977bfcd6", 410949),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"Fargus",
			AD_ENTRY2s("general.clu", "98e43a4fd93227b1d5d44e664eeede0c", 7320908,
			           "text.clu",    "33a2645498ef1f4e63c4f6a50da4a3e2", 288998),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"Mediahauz/English speech",
			AD_ENTRY2s("general.clu", "31db8564f9187538f24d9fda0677f666", 7059728,
			           "text.clu",    "e85c148037b8bfc02c968d4d22fda5e1", 315178),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
			           "text.clu",    "06691fc9f749f3f7ad0f622fbfe79467", 302756),
			Common::EN_USA,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
			           "text.clu",    "be8ad3f1d9d3ddd8881169b16aa23970", 838392),
			Common::EN_GRB,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
			           "text.clu",    "0920f1aec8bc9d02f8c94f73965c8006", 327668),
			Common::FR_FRA,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
			           "text.clu",    "5ce53dfc154b80d4ca64b60df808e411", 347456),
			Common::DE_DEU,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
			           "text.clu",    "03ffcd1eec48f74a3d16d1b7751cee0b", 316124),
			Common::ES_ESP,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{
		{
			"sword2",
			"",
			AD_ENTRY2s("general.clu", "7ef0353ca03338d59b4f4e3d01a38df1", 2095780,
			           "text.clu",    "298bd6eef464780bf6b0830805eef220", 334784),
			Common::IT_ITA,
			Common::kPlatformPSX,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		0
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Sword2
