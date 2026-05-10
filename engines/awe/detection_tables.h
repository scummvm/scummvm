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

namespace Awe {

const PlainGameDescriptor aweGames[] = {
	{ "anotherworld", "Another World - Out of this World" },
	{ nullptr, nullptr }
};

const AweGameDescription gameDescriptions[] = {

	// DOS Floppy (1991) French/English
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY1s("memlist.bin", "f2bf61fe20c98108b2256e96d57d3fe0", 2940),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_RENDEREGA, GUIO_RENDERVGA, GAMEOPTION_COPY_PROTECTION)
		},
		DT_DOS
	},
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY1s("memlist.bin", "f2bf61fe20c98108b2256e96d57d3fe0", 2940),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_RENDEREGA, GUIO_RENDERVGA, GAMEOPTION_COPY_PROTECTION)
		},
		DT_DOS
	},

	// DOS Floppy - ANOTHER WORLD PC (19/3/92)
	// This version is also included on the Delphine Classic Collection CD
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY1s("memlist.bin", "63675fdb53dd4a2b8d72182bbee47592", 2940),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_RENDEREGA, GUIO_RENDERVGA, GAMEOPTION_COPY_PROTECTION)
		},
		DT_DOS
	},

	// Out of This World, Valueware and Interplay's 10 Year Anthology
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY1s("memlist.bin", "4605962431175a5b961f6db4041adcff", 2940),
			Common::EN_USA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_RENDEREGA, GUIO_RENDERVGA, GAMEOPTION_COPY_PROTECTION)
		},
		DT_DOS
	},

	// Out of This World (alternate)
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY1s("memlist.bin", "ee9e67fcaace0300fec4e619299e6e0e", 2940),
			Common::EN_USA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_RENDEREGA, GUIO_RENDERVGA, GAMEOPTION_COPY_PROTECTION)
		},
		DT_DOS
	},

	{
		{
			"anotherworld",
			"Demo",
			AD_ENTRY1s("memlist.bin", "11e0cc58aeb47ad1dfc1c4dae8dcd9ee", 2940),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO3(GUIO_RENDEREGA, GUIO_RENDERVGA, GAMEOPTION_COPY_PROTECTION)
		},
		DT_DOS
	},

	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY1s("bank", "30fb99cb4cbd812273d0b54b7b4a18ca", 987462),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
		DT_WIN31
	},

	{
		{
			"anotherworld",
			"20th Anniversary",
			AD_ENTRY1s("game/dat/file017.dat", "3c3e73a77d2cab00324c1affd3f2ef8c", 25108),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		DT_20TH_EDITION
	},

	// European version
	// Also included as a bonus on both GOG and Steam.
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY2s("bank01", "8cec5badf5bea89bff3a550daff79861", 244868,
					   "bank03", "2ef3440fd6205634b257d56b0bc3ea51", 127846),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_COPY_PROTECTION)
		},
		DT_AMIGA
	},
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY2s("bank01", "8cec5badf5bea89bff3a550daff79861", 244868,
					   "bank03", "2ef3440fd6205634b257d56b0bc3ea51", 127846),
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_COPY_PROTECTION)
		},
		DT_AMIGA
	},
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY2s("bank01", "8cec5badf5bea89bff3a550daff79861", 244868,
					   "bank03", "2ef3440fd6205634b257d56b0bc3ea51", 127846),
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_COPY_PROTECTION)
		},
		DT_AMIGA
	},

	// Out of this World (USA)
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY2s("bank01", "8cec5badf5bea89bff3a550daff79861", 244822,
					   "bank03", "b8cdbcd57e9953423a21dadeeca3cdfa", 128040),
			Common::EN_USA,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_COPY_PROTECTION)
		},
		DT_AMIGA
	},

	// Delphine Classic Collection (Kixx)
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY2s("bank01", "8cec5badf5bea89bff3a550daff79861", 244674,
					   "bank03", "c2594927418291119c2996eabcf6d0f1", 128038),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_COPY_PROTECTION)
		},
		DT_AMIGA
	},
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY2s("bank01", "8cec5badf5bea89bff3a550daff79861", 244674,
					   "bank03", "c2594927418291119c2996eabcf6d0f1", 128038),
			Common::FR_FRA,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_COPY_PROTECTION)
		},
		DT_AMIGA
	},
	{
		{
			"anotherworld",
			nullptr,
			AD_ENTRY2s("bank01", "8cec5badf5bea89bff3a550daff79861", 244674,
					   "bank03", "c2594927418291119c2996eabcf6d0f1", 128038),
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_COPY_PROTECTION)
		},
		DT_AMIGA
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Awe
