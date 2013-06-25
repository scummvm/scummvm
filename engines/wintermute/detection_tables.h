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

namespace Wintermute {

#define GAMEOPTION_SHOW_FPS GUIO_GAMEOPTIONS1

static const PlainGameDescriptor wintermuteGames[] = {
	{"5ld",             "Five Lethal Demons"},
	{"5ma",             "Five Magical Amulets"},
	{"bthreshold",      "Beyond the Threshold"},
	{"actualdest",      "Actual Destination"},
	{"carolreed4",      "Carol Reed 4 - East Side Story"},
	{"carolreed5",      "Carol Reed 5 - The Colour of Murder"},
	{"carolreed6",      "Carol Reed 6 - Black Circle"},
	{"carolreed7",      "Carol Reed 7 - Blue Madonna"},
	{"carolreed8",      "Carol Reed 8 - Amber's Blood"},
	{"carolreed9",      "Carol Reed 9 - Cold Case Summer"},
	{"chivalry",        "Chivalry is Not Dead"},
	{"deadcity",        "Dead City"},
	{"dreaming",        "Des Reves Elastiques Avec Mille Insectes Nommes Georges"},
	{"dirtysplit",      "Dirty Split"},
	{"dreamscape",      "Dreamscape"},
	{"ghostsheet",      "Ghost in the Sheet"},
	{"hamlet",          "Hamlet or the last game without MMORPS features, shaders and product placement"},
	{"jamesperis",		"James Peris: No License Nor Control"},
	{"julia",           "J.U.L.I.A."},
	{"mirage",          "Mirage"},
	{"pigeons",         "Pigeons in the Park"},
	{"reversion1",      "Reversion: The Escape"},
	{"reversion2",      "Reversion: The Meeting"},
	{"rosemary",        "Rosemary"},
	{"thebox",          "The Box"},
	{"tradestory",		"The Trader of Stories"},
	{"twc",             "the white chamber"},
	{"wintermute",      "Wintermute engine game"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	// Five Lethal Demons
	{
		"5ld",
		"",
		AD_ENTRY1s("data.dcp", "1037a77cbd001e0644898addc022322c", 15407750),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Five Magical Amulets
	{
		"5ma",
		"",
		AD_ENTRY1s("data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Actual Destination
	{
		"actualdest",
		"",
		AD_ENTRY1s("data.dcp", "6926f44b26f21ceb1d840eaab9aeb510", 9081740),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Beyond the Threshold
	{
		"bthreshold",
		"",
		AD_ENTRY1s("data.dcp", "d49bf9ccb2e74507447c82d6ad3e2bc4", 12773712),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 4 - East Side Story (Demo)
	{
		"carolreed4",
		"Demo",
		AD_ENTRY1s("data.dcp", "b3f8b09bb4b05ee3e9d14697525257f9", 59296246),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Carol Reed 4 - East Side Story
	{
		"carolreed4",
		"",
		AD_ENTRY1s("data.dcp", "b26377797f060afc2d440d820100c1ce", 529320536),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Carol Reed 5 - The Colour of Murder
	{
		"carolreed5",
		"",
		AD_ENTRY1s("data.dcp", "3fcfca44209545d0e26774156427b494", 603660415),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 6 - Black Circle
	{
		"carolreed6",
		"",
		AD_ENTRY1s("data.dcp", "0e4c532beecf23d85012168753f41189", 456258147),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 7 - Blue Madonna
	{
		"carolreed7",
		"",
		AD_ENTRY1s("data.dcp", "24e3db3e2fabfc956713796d87a3efb0", 495471147),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 8 - Amber's Blood
	{
		"carolreed8",
		"",
		AD_ENTRY1s("data.dcp", "859d16b0d5b9b255e470cbded2c6cedc", 502714557),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 9 - Cold Case Summer
	{
		"carolreed9",
		"",
		AD_ENTRY1s("data.dcp", "2b343b48a7aee508d728a546b414a255", 620005266),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Chivalry is Not Dead
	{
		"chivalry",
		"",
		AD_ENTRY1s("data.dcp", "ebd0915d9a12df5224be22f53bb23eb6", 7278306),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},
	// Chivalry is Not Dead (Version from deirdrakai.com)
	{
		"chivalry",
		"",
		AD_ENTRY1s("data.dcp", "ae6d91b9517f4d2851a8ad94c96951c8", 7278302),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},
	// Dead City (English)
	{
		"deadcity",
		"",
		{
			{"english.dcp", 0, "c591046d6de7e381d76f70e0787b2b1f", 415935},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dead City (Italian)
	{
		"deadcity",
		"",
		{
			{"italian.dcp", 0, "92d8efb94436bec7bd1b7fe0b548192e", 454037},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dead City (Russian)
	{
		"deadcity",
		"",
		{
			{"russian.dcp", 0, "a0ae71e9e1185596fffb07ad2c951eb9", 653317},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (English)
	{
		"dirtysplit",
		"",
		AD_ENTRY1s("data.dcp", "8f3dae199361ece0f59fb20cfff6eed3", 88577621),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (German)
	{
		"dirtysplit",
		"",
		AD_ENTRY1s("data.dcp", "139d8a25579e969f8b37d20e6e3de5f9", 92668291),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Des Reves Elastiques Avec Mille Insectes Nommes Georges
	{
		"dreaming",
		"",
		AD_ENTRY1s("data.dcp", "4af26d97ea063fc1277ce30ae431de90", 8804073),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dreamscape
	{
		"dreamscape",
		"",
		AD_ENTRY1s("data.dcp",  "7a5752ed4446c862be9f02d7932acf54", 17034377),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Ghosts in the Sheet
	{
		"ghostsheet",
		"Demo",
		AD_ENTRY1s("data.dcp", "dc1f6595f412ac25a52eaf47dad4ab81", 169083),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Hamlet or the last game without MMORPS features, shaders and product placement
	{
		"hamlet",
		"",
		AD_ENTRY1s("data.dcp", "f624add957a77c9930529fb28cc2450f", 88183022),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// James Peris: No License Nor Control
	{
		"jamesperis",
		"Demo",
		AD_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507),
		Common::UNK_LANG, // No solution in place to select language
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// J.U.L.I.A. (English)
	{
		"julia",
		"",
		AD_ENTRY1s("data.dcp", "c2264b4f8fcd132d2913ff5b6076a24f", 10109741),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// J.U.L.I.A. (English) (Demo)
	{
		"julia",
		"Demo",
		AD_ENTRY1s("data.dcp", "f0bbc3394555a9811f6050dae428cab6", 7655237),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// J.U.L.I.A. (English) (Greenlight Demo)
	{
		"julia",
		"Greenlight Demo",
		AD_ENTRY1s("data.dcp", "4befd448d36b0dae9c3ab1aa7cb8b78d", 7271886),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Mirage
	{
		"mirage",
		"",
		AD_ENTRY1s("data.dcp", "d230b0b99c0aa77b9ecd094d8ee5573b", 17844056),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Pigeons in the Park
	{
		"pigeons",
		"",
		AD_ENTRY1s("data.dcp", "9143a5b6ff8206aefe3c4c643add3ec7", 2611100),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.0
	{
		"reversion1",
		"Version 1.0",
		AD_ENTRY1s("data.dcp", "cd616f98ebfd047e0c540b50b4b70761", 254384531),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (Chinese)
	{
		"reversion1",
		"Version 1.1",
		{
			{"chinese.dcp", 0, "cf97150739499a4c15f51dc534ff85a1", 6330561},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::ZH_CNA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (English)
	{
		"reversion1",
		"Version 1.1",
		{
			{"english.dcp", 0, "7b2f061d7c91365c5d04605f1de032b3", 5702699},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (French)
	{
		"reversion1",
		"Version 1.1",
		{
			{"french.dcp", 0, "214204b6022c5ed67fada44557690faf", 6327400},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (German)
	{
		"reversion1",
		"Version 1.1",
		{
			{"german.dcp", 0, "96677823b36d580a4a29e3659071071c", 6340699},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (Italian)
	{
		"reversion1",
		"Version 1.1",
		{
			{"italian.dcp", 0, "9ce80c1835108f10170a02969f71efe1", 6301836},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (Portuguese)
	{
		"reversion1",
		"Version 1.1",
		{
			{"portugues.dcp", 0, "8772501afa2c630a7c697eb99e9c7bda", 5053303},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (Chinese)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_nz.dcp", 0, "92c4065156e464211685bf799b3279fd", 5130600},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::ZH_CNA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (English)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_en.dcp", 0, "05845e1283920a6e4044f2a54f7a9519", 4818543},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (French)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_fr.dcp", 0, "441795490e9307eb2ed07830779881ac", 5425959},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (German)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_de.dcp", 0, "b588041015b93e54b4c246ca77d01e76", 5423798},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (Italian)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_it.dcp", 0, "a1f4199079b75ee10cded41f05b45d5f", 5386424},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (Portuguese)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_pt.dcp", 0, "3d653debd37e56756a79401e1004c4d2", 4149165},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Meeting (Chinese)
	{
		"reversion2",
		"",
		{
			{"xlanguage_nz.dcp", 0, "8c3709474a87a7876109025dff41ff3f", 8746015},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::ZH_CNA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Meeting (English)
	{
		"reversion2",
		"",
		{
			{"xlanguage_en.dcp", 0, "ca357d86618d1ab76a21c913f4403cbd", 8414976},
			{"data.dcp", 0, "f7938cbfdc48f07934550245a3286921", 255672016},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Meeting (Spanish)
	{
		"reversion2",
		"",
		AD_ENTRY1s("data.dcp", "f7938cbfdc48f07934550245a3286921", 255672016),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Rosemary
	{
		"rosemary",
		"",
		AD_ENTRY1s("data.dcp", "4f2631138bd4d27587d9043f8aeff3df", 29483643),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Box
	{
		"thebox",
		"",
		AD_ENTRY1s("data.dcp", "ec5f0c7e8174e307701447b53afe7e2f", 108372483),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Trader of Stories
	{
		"tradestory",
		"Demo",
		AD_ENTRY1s("data.dcp", "0a0b51191636cc8ead89b905281c3218", 40401902),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// the white chamber (multi-language)
	{
		"twc",
		"",
		AD_ENTRY1s("data.dcp", "0011d01142547c61e51ba24dc42b579e", 186451273),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Wintermute

