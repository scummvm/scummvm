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

namespace Buried {

static const BuriedGameDescription gameDescriptions[] = {
	// English Windows 3.11 8BPP
	// Installed
	// v1.01
	{
		{
			"buried",
			"v1.01 8BPP",
			{
				{ "BIT816.EXE",  0, "57a14461c77d9c77534bd418043db1ec", 1163776 },
				{ "BIT8LIB.DLL", 0, "31bcd9e5cc32df00b09ce626e6d9106e", 2420480 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// English Windows 3.11 24BPP
	// Installed
	// v1.01
	{
		{
			"buried",
			"v1.01 24BPP",
			{
				{ "BIT2416.EXE",  0, "dcbfb3f2916ad902043942fc00d2017f", 1159680 },
				{ "BIT24LIB.DLL", 0, "74ac9dae92f415fea8cdbd220ba8795c", 5211648 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_TRUECOLOR,
			GUIO0()
		},
	},

	// Japanese Windows 3.11 8BPP
	// Installed
	// v1.051
	{
		{
			"buried",
			"v1.051 8BPP",
			{
				{ "BIT816.EXE",  0, "decbf9a7d91803525137ffd980d16708", 1163264 },
				{ "BIT8LIB.DLL", 0, "f5ccde0efccb95afe902627a35262568", 2418816 },
				{ 0, 0, 0, 0 },
			},
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// Japanese Windows 3.11 24BPP
	// Installed
	// v1.051
	{
		{
			"buried",
			"v1.051 24BPP",
			{
				{ "BIT2416.EXE",  0, "9435b9a40e3ac83e6fa1e83caaf57792", 1157632 },
				{ "BIT24LIB.DLL", 0, "4d55802259d9648b9aa396461bfd53a3", 6576896 },
				{ 0, 0, 0, 0 },
			},
			Common::JA_JPN,
			Common::kPlatformWindows,
			GF_TRUECOLOR,
			GUIO0()
		},
	},

	// English Windows 3.11 8BPP
	// Not Installed
	// v1.01
	{
		{
			"buried",
			"v1.01 8BPP",
			{
				{ "BIT816.EX_",  0, "166b44e53350c19bb25ef93d2c2b8f79", 364490 },
				{ "BIT8LIB.DL_", 0, "8a345993f60f6bed7c17fa9e7f2bc37d", 908854 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_COMPRESSED,
			GUIO0()
		},
	},

	// English Windows 3.11 24BPP
	// Not Installed
	// v1.01
	{
		{
			"buried",
			"v1.01 24BPP",
			{
				{ "BIT2416.EX_",  0, "a9ac76610ba614b59235a7d5e00e4a62", 361816 },
				{ "BIT24LIB.DL_", 0, "00e6eedbcef824988fbb01a87ca8f7fd", 2269314 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_COMPRESSED | GF_TRUECOLOR,
			GUIO0()
		},
	},

	// German Windows 3.11 8BPP
	// Installed
	// v1.05
	{
		{
			"buried",
			"v1.05 8BPP",
			{
				{ "BIT816.EXE",  0, "a039e9f1c569acc1cf80f6b549ce1e37", 1178112 },
				{ "BIT8LIB.DLL", 0, "6b22f0b47efb29e45e9b2a336185d924", 2420608 },
				{ 0, 0, 0, 0 },
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// German Windows 3.11 24BPP
	// Installed
	// v1.05
	{
		{
			"buried",
			"v1.05 24BPP",
			{
				{ "BIT2416.EXE",  0, "fbfd453cced2b14069fa32e3c8dd69e2", 1172480 },
				{ "BIT24LIB.DLL", 0, "30e56210d3150b5fa41c9bd2c90754fe", 6581376 },
				{ 0, 0, 0, 0 },
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			GF_TRUECOLOR,
			GUIO0()
		},
	},

	// French Windows 3.11 8BPP
	// Installed
	// v1.05
	{
		{
			"buried",
			"v1.05 8BPP",
			{
				{ "BIT816.EXE",  0, "edea5331dc7cb0f3da7322691e12a18a", 1182720 },
				{ "BIT8LIB.DLL", 0, "6b22f0b47efb29e45e9b2a336185d924", 2420608 },
				{ 0, 0, 0, 0 },
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// French Windows 3.11 24BPP
	// Installed
	// v1.05
	{
		{
			"buried",
			"v1.05 24BPP",
			{
				{ "BIT2416.EXE",  0, "0adea8e1ad6fddad3b861be8a7bab340", 1177088 },
				{ "BIT24LIB.DLL", 0, "30e56210d3150b5fa41c9bd2c90754fe", 6581376 },
				{ 0, 0, 0, 0 },
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			GF_TRUECOLOR,
			GUIO0()
		},
	},

	// Italian Windows 3.11 8BPP
	// Installed
	// v1.05
	{
		{
			"buried",
			"v1.05 8BPP",
			{
				{ "BIT816.EXE",  0, "fb3e5c9198503bbb45b79150b511af5e", 1175040 },
				{ "BIT8LIB.DLL", 0, "6b22f0b47efb29e45e9b2a336185d924", 2420608 },
				{ 0, 0, 0, 0 },
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// Italian Windows 3.11 24BPP
	// Installed
	// v1.05
	{
		{
			"buried",
			"v1.05 24BPP",
			{
				{ "BIT2416.EXE",  0, "56bdd481b063c91b95c21f02faa450bb", 1169408 },
				{ "BIT24LIB.DLL", 0, "30e56210d3150b5fa41c9bd2c90754fe", 6581376 },
				{ 0, 0, 0, 0 },
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			GF_TRUECOLOR,
			GUIO0()
		},
	},

	// Spanish Windows 3.11 8BPP
	// Installed
	// v1.05
	{
		{
			"buried",
			"v1.05 8BPP",
			{
				{ "BIT816.EXE",  0, "f08c96347fcb83d92ae57de1fb578234", 1174528 },
				{ "BIT8LIB.DLL", 0, "a80afdc20264e764e831ef5099cde623", 2420992 },
				{ 0, 0, 0, 0 },
			},
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
	},

	// Spanish Windows 3.11 24BPP
	// Installed
	// v1.05
	{
		{
			"buried",
			"v1.05 24BPP",
			{
				{ "BIT2416.EXE",  0, "d409b59f124babc9b423793e762b7e03", 1168896 },
				{ "BIT24LIB.DLL", 0, "c864bcd69d05532e0066b8db173a939b", 6582784 },
				{ 0, 0, 0, 0 },
			},
			Common::ES_ESP,
			Common::kPlatformWindows,
			GF_TRUECOLOR,
			GUIO0()
		},
	},

	// English Windows 95 8BPP
	// v1.1
	{
		{
			"buried",
			"v1.1 8BPP",
			{
				{ "BIT832.EXE",  0, "f4f8007f49197ba40ea633eb113c0b6d", 1262592 },
				{ "BIT8L32.DLL", 0, "addfef0420e1f41a7766ecc6baa58553", 2424832 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_WIN95,
			GUIO0()
		},
	},

	// English Windows 95 24BPP
	// v1.1
	{
		{
			"buried",
			"v1.1 24BPP",
			{
				{ "BIT2432.EXE",  0, "4086a8200938eac3e72d238a84f65618", 1257472 },
				{ "BIT24L32.DLL", 0, "198bfd476d5228c4a7a63c029cffadfc", 5216256 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_TRUECOLOR | GF_WIN95,
			GUIO0()
		},
	},

	// English Windows Demo 8BPP
	{
		{
			"buried",
			"Demo 8BPP",
			AD_ENTRY1("BIT816.EXE", "a5bca831dac0903a304c29c320f881c5"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOLAUNCHLOAD)
		},
	},

	// English Windows Demo 24BPP
	{
		{
			"buried",
			"Demo 24BPP",
			AD_ENTRY1("BIT2416.EXE", "9857e2d2b7a63b1304058dabc5098249"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | GF_TRUECOLOR,
			GUIO1(GUIO_NOLAUNCHLOAD)
		},
	},

	// English Windows 3.11 Trial 8BPP
	// v1.1
	{
		{
			"buried",
			"Trial 8BPP",
			{
				{ "BTV816.EXE",  0, "a3551483329816d8ddc8fa877113762c", 1170432 },
				{ "BIT8LIB.DLL", 0, "6b22f0b47efb29e45e9b2a336185d924", 2420608 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | GF_TRIAL,
			GUIO0()
		},
	},

	// English Windows 3.11 Trial 24BPP
	// v1.1
	{
		{
			"buried",
			"Trial 24BPP",
			{
				{ "BTV2416.EXE",  0, "e0783c5eda09176d414d3df4ada8fe89", 1164288 },
				{ "BIT24LIB.DLL", 0, "74ac9dae92f415fea8cdbd220ba8795c", 5211648 },
				{ 0, 0, 0, 0 },
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | GF_TRUECOLOR | GF_TRIAL,
			GUIO0()
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Buried
