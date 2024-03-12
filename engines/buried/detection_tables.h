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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

namespace Buried {

// NOTE: If variants with new languages are added, the Mayan death god
// box puzzle will need to be updated. Check environ/mayan.cpp,
// DeathGodPuzzleBox::isPuzzleSolved()

static const ADGameDescription gameDescriptions[] = {
	// English Windows 3.11 8BPP
	// Installed
	// v1.00
	{
		"buried",
		"v1.00 8BPP",
		AD_ENTRY2s("BIT816.EXE", "da3c191bd4384950c17a19b9ea06cd7c", 1166336,
				   "BIT8LIB.DLL","31bcd9e5cc32df00b09ce626e6d9106e", 2420480),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 24BPP
	// Installed
	// v1.00
	{
		"buried",
		"v1.00 24BPP",
		AD_ENTRY2s("BIT2416.EXE", "e661f758d191150cbdf6781bb2565acb", 1162752,
				   "BIT24LIB.DLL","74ac9dae92f415fea8cdbd220ba8795c", 5211648),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 8BPP
	// Installed
	// v1.01
	{
		"buried",
		"v1.01 8BPP",
		AD_ENTRY2s("BIT816.EXE", "57a14461c77d9c77534bd418043db1ec", 1163776,
				   "BIT8LIB.DLL","31bcd9e5cc32df00b09ce626e6d9106e", 2420480),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 24BPP
	// Installed
	// v1.01
	{
		"buried",
		"v1.01 24BPP",
		AD_ENTRY2s("BIT2416.EXE", "dcbfb3f2916ad902043942fc00d2017f", 1159680,
				   "BIT24LIB.DLL","74ac9dae92f415fea8cdbd220ba8795c", 5211648),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// Japanese Windows 3.11 8BPP
	// Installed
	// v1.051
	{
		"buried",
		"v1.051 8BPP",
		AD_ENTRY2s("BIT816.EXE", "decbf9a7d91803525137ffd980d16708", 1163264,
				   "BIT8LIB.DLL","f5ccde0efccb95afe902627a35262568", 2418816),
		Common::JA_JPN,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_FULL_GAME
	},

	// Japanese Windows 3.11 24BPP
	// Installed
	// v1.051
	{
		"buried",
		"v1.051 24BPP",
		AD_ENTRY2s("BIT2416.EXE", "9435b9a40e3ac83e6fa1e83caaf57792", 1157632,
				   "BIT24LIB.DLL","4d55802259d9648b9aa396461bfd53a3", 6576896),
		Common::JA_JPN,
		Common::kPlatformWindows,
		GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 8BPP
	// Not Installed
	// v1.00
	{
		"buried",
		"v1.00 8BPP",
		AD_ENTRY2s("BIT816.EX_", "e385901182f4eafe0a8a157e4f24fc1f", 366069,
				   "BIT8LIB.DL_","8a345993f60f6bed7c17fa9e7f2bc37d", 900844),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_COMPRESSED,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 24BPP
	// Not Installed
	// v1.00
	{
		"buried",
		"v1.00 24BPP",
		AD_ENTRY2s("BIT2416.EX_", "144ad890ad0ded4262438c3ef37b88f4", 363601,
				   "BIT24LIB.DL_","00e6eedbcef824988fbb01a87ca8f7fd", 2272515),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_COMPRESSED | GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 8BPP
	// Not Installed
	// v1.01
	{
		"buried",
		"v1.01 8BPP",
		AD_ENTRY2s("BIT816.EX_", "166b44e53350c19bb25ef93d2c2b8f79", 364490,
				   "BIT8LIB.DL_","8a345993f60f6bed7c17fa9e7f2bc37d", 908854),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_COMPRESSED,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 24BPP
	// Not Installed
	// v1.01
	{
		"buried",
		"v1.01 24BPP",
		AD_ENTRY2s("BIT2416.EX_", "a9ac76610ba614b59235a7d5e00e4a62", 361816,
				   "BIT24LIB.DL_","00e6eedbcef824988fbb01a87ca8f7fd", 2269314),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_COMPRESSED | GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 8BPP
	// Not Installed
	// v1.04
	{
		"buried",
		"v1.04 8BPP",
		AD_ENTRY2s("BIT816.EXE", "9055335a574d3b9418b8ddb9a5539829", 1168384,
				   "BIT8LIB.DLL","31bcd9e5cc32df00b09ce626e6d9106e", 2420480),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 24BPP
	// Not Installed
	// v1.04
	{
		"buried",
		"v1.04 24BPP",
		AD_ENTRY2s("BIT2416.EXE", "782083ef765dcbe8e8ac11d025fba68d", 1163264,
				   "BIT24LIB.DLL","74ac9dae92f415fea8cdbd220ba8795c", 5211648),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// German Windows 3.11 8BPP
	// Installed
	// v1.05
	{
		"buried",
		"v1.05 8BPP",
		AD_ENTRY2s("BIT816.EXE", "a039e9f1c569acc1cf80f6b549ce1e37", 1178112,
				   "BIT8LIB.DLL","6b22f0b47efb29e45e9b2a336185d924", 2420608),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_FULL_GAME
	},

	// German Windows 3.11 24BPP
	// Installed
	// v1.05
	{
		"buried",
		"v1.05 24BPP",
		AD_ENTRY2s("BIT2416.EXE", "fbfd453cced2b14069fa32e3c8dd69e2", 1172480,
				   "BIT24LIB.DLL","30e56210d3150b5fa41c9bd2c90754fe", 6581376),
		Common::DE_DEU,
		Common::kPlatformWindows,
		GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// French Windows 3.11 8BPP
	// Installed
	// v1.05
	{
		"buried",
		"v1.05 8BPP",
		AD_ENTRY2s("BIT816.EXE", "edea5331dc7cb0f3da7322691e12a18a", 1182720,
				   "BIT8LIB.DLL","6b22f0b47efb29e45e9b2a336185d924", 2420608),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_FULL_GAME
	},

	// French Windows 3.11 24BPP
	// Installed
	// v1.05
	{
		"buried",
		"v1.05 24BPP",
		AD_ENTRY2s("BIT2416.EXE", "0adea8e1ad6fddad3b861be8a7bab340", 1177088,
				   "BIT24LIB.DLL","30e56210d3150b5fa41c9bd2c90754fe", 6581376),
		Common::FR_FRA,
		Common::kPlatformWindows,
		GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// Italian Windows 3.11 8BPP
	// Installed
	// v1.05
	{
		"buried",
		"v1.05 8BPP",
		AD_ENTRY2s("BIT816.EXE", "fb3e5c9198503bbb45b79150b511af5e", 1175040,
				   "BIT8LIB.DLL","6b22f0b47efb29e45e9b2a336185d924", 2420608),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_FULL_GAME
	},

	// Italian Windows 3.11 24BPP
	// Installed
	// v1.05
	{
		"buried",
		"v1.05 24BPP",
		AD_ENTRY2s("BIT2416.EXE", "56bdd481b063c91b95c21f02faa450bb", 1169408,
				   "BIT24LIB.DLL","30e56210d3150b5fa41c9bd2c90754fe", 6581376),
		Common::IT_ITA,
		Common::kPlatformWindows,
		GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// Spanish Windows 3.11 8BPP
	// Installed
	// v1.05
	{
		"buried",
		"v1.05 8BPP",
		AD_ENTRY2s("BIT816.EXE", "f08c96347fcb83d92ae57de1fb578234", 1174528,
				   "BIT8LIB.DLL","a80afdc20264e764e831ef5099cde623", 2420992),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_FULL_GAME
	},

	// Spanish Windows 3.11 24BPP
	// Installed
	// v1.05
	{
		"buried",
		"v1.05 24BPP",
		AD_ENTRY2s("BIT2416.EXE", "d409b59f124babc9b423793e762b7e03", 1168896,
				   "BIT24LIB.DLL","c864bcd69d05532e0066b8db173a939b", 6582784),
		Common::ES_ESP,
		Common::kPlatformWindows,
		GF_TRUECOLOR,
		GUIO_FULL_GAME
	},

	// English Windows 95 8BPP
	// v1.1
	{
		"buried",
		"v1.1 8BPP",
		AD_ENTRY2s("BIT832.EXE", "f4f8007f49197ba40ea633eb113c0b6d", 1262592,
				   "BIT8L32.DLL","addfef0420e1f41a7766ecc6baa58553", 2424832),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_WIN95,
		GUIO_FULL_GAME
	},

	// English Windows 95 24BPP
	// v1.1
	{
		"buried",
		"v1.1 24BPP",
		AD_ENTRY2s("BIT2432.EXE", "4086a8200938eac3e72d238a84f65618", 1257472,
				   "BIT24L32.DLL","198bfd476d5228c4a7a63c029cffadfc", 5216256),
		Common::EN_ANY,
		Common::kPlatformWindows,
		GF_TRUECOLOR | GF_WIN95,
		GUIO_FULL_GAME
	},

	// English Windows Demo 8BPP
	{
		"buried",
		"Demo 8BPP",
		AD_ENTRY1("BIT816.EXE", "a5bca831dac0903a304c29c320f881c5"),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO_GAME_DEMO
	},

	// English Windows Demo 24BPP
	{
		"buried",
		"Demo 24BPP",
		AD_ENTRY1("BIT2416.EXE", "9857e2d2b7a63b1304058dabc5098249"),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | GF_TRUECOLOR,
		GUIO_GAME_DEMO
	},

	// English Windows Demo 8BPP - US Gold (UK)
	{
		"buried",
		"Demo 8BPP",
		AD_ENTRY1s("BIT816.EXE", "5535fd50e504537ab08066a89df1b6de", 1259040),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO_GAME_DEMO
	},

	// English Windows Demo 24BPP - US Gold (UK)
	{
		"buried",
		"Demo 24BPP",
		AD_ENTRY1s("BIT2416.EXE", "53c520eb7c977fb1c2bb235e2a517b85", 2816256),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | GF_TRUECOLOR,
		GUIO_GAME_DEMO
	},

	// English Windows Demo 8BPP - North America
	{
		"buried",
		"Demo 8BPP",
		AD_ENTRY1s("BIT816.EXE", "30bbd89d7df8cb319e654a441efce877", 1259072),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO_GAME_DEMO
	},

	// English Windows Demo 24BPP - North America
	{
		"buried",
		"Demo 24BPP",
		AD_ENTRY1s("BIT2416.EXE", "69ba6611561c3eb5f7d8853ef913aa09", 2816256),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | GF_TRUECOLOR,
		GUIO_GAME_DEMO
	},

	// English Windows 3.11 Trial 8BPP
	// v1.1
	{
		"buried",
		"Trial 8BPP",
		AD_ENTRY2s("BTV816.EXE", "a3551483329816d8ddc8fa877113762c", 1170432,
				   "BIT8LIB.DLL","6b22f0b47efb29e45e9b2a336185d924", 2420608),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | GF_TRIAL,
		GUIO_FULL_GAME
	},

	// English Windows 3.11 Trial 24BPP
	// v1.1
	{
		"buried",
		"Trial 24BPP",
		AD_ENTRY2s("BTV2416.EXE", "e0783c5eda09176d414d3df4ada8fe89", 1164288,
				   "BIT24LIB.DLL","74ac9dae92f415fea8cdbd220ba8795c", 5211648),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | GF_TRUECOLOR | GF_TRIAL,
		GUIO_FULL_GAME
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Buried
